// Fill out your copyright notice in the Description page of Project Settings.


#include "PhotoModeComponent.h"
#include "PhotoModeCameraActor.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"


UPhotoModeComponent::UPhotoModeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FString UPhotoModeComponent::GetPhotosDirectory() const
{
	return FPaths::ProjectSavedDir() / TEXT("Photos");
}

void UPhotoModeComponent::EnterPhotoMode()
{
	if (bIsInPhotoMode)
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());

	// Only ever run this locally — never for a remote/simulated proxy's component
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	if (!SpawnedCameraActor && CameraActorClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = OwnerPawn;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SpawnedCameraActor = GetWorld()->SpawnActor<APhotoModeCameraActor>(
			CameraActorClass,
			OwnerPawn->GetActorLocation() + OwnerPawn->GetActorForwardVector() * 100.f,
			OwnerPawn->GetActorRotation(),
			Params);
	}

	if (SpawnedCameraActor)
	{
		// Client-side view change only — no RPC, no possession, no server involvement
		PC->SetViewTargetWithBlend(SpawnedCameraActor, 0.25f);
	}

	if (PhotoModeMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(PhotoModeMappingContext, 10);
		}
	}

	// Deliberately no SetGamePaused, no CustomTimeDilation change.
	// Other players on the listen server keep simulating normally.

	bIsInPhotoMode = true;
}

void UPhotoModeComponent::ExitPhotoMode()
{
	if (!bIsInPhotoMode)
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	APlayerController* PC = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;

	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	PC->SetViewTargetWithBlend(OwnerPawn, 0.25f);

	if (PhotoModeMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(PhotoModeMappingContext);
		}
	}

	if (SpawnedCameraActor)
	{
		SpawnedCameraActor->Destroy();
		SpawnedCameraActor = nullptr;
	}

	bIsInPhotoMode = false;
}

void UPhotoModeComponent::TakePhoto()
{
	if (!bIsInPhotoMode)
	{
		return;
	}

	if (!GEngine ||!GEngine->GameViewport)
	{
		return;
	}

	// Give Listener a chance to hide retice/prompt widgets right before capture
	OnPreCapture.Broadcast();

	ScreenshotDelegateHandle = GEngine->GameViewport->OnScreenshotCaptured().AddUObject(
		this, &UPhotoModeComponent::OnScreenshotCaptured);

	FScreenshotRequest::RequestScreenshot(false /* bShowUI here*/);
}

void UPhotoModeComponent::OnScreenshotCaptured(int32 Width, int32 Height, const TArray<FColor>& Bitmap)
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->OnScreenshotCaptured().Remove(ScreenshotDelegateHandle);
	}

	TArray<uint8> CompressedPNG;
	FImageUtils::ThumbnailCompressImageArray(Width, Height, Bitmap, CompressedPNG);

	IFileManager& FileManager = IFileManager::Get();
	const FString PhotosDir = GetPhotosDirectory();

	// Explicit existence check, then create if missing
	if (!FileManager.DirectoryExists(*PhotosDir))
	{
		const bool bCreated = FileManager.MakeDirectory(*PhotosDir, /*Tree=*/true);
		if (!bCreated)
		{
			UE_LOG(LogTemp, Error, TEXT("PhotoMode: Failed to create directory: %s"), *PhotosDir);
			return;
		}
	}

	const FString Filename = FString::Printf(TEXT("Photo_%s.png"),
		*FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H-%M-%S")));
	const FString FullPath = PhotosDir / Filename;

	if (FFileHelper::SaveArrayToFile(CompressedPNG, *FullPath))
	{
		OnPostCapture.Broadcast(FullPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PhotoMode: Failed to save screenshot to: %s"), *FullPath);
	}
}


