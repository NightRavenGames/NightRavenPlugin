// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhotoModeComponent.generated.h"

class APhotoModeCameraActor;
class UInputMappingContext;

DECLARE_MULTICAST_DELEGATE(FOnPhotoModePreCapture);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPhotoModePostCapture, const FString&); // SavedPhoto

UCLASS(ClassGroup=(PhotoMode), meta=(BlueprintSpawnableComponent))
class BACKROOMSCAMERA_API UPhotoModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPhotoModeComponent();

	UFUNCTION(BlueprintCallable, Category = "PhotoMode")
	void EnterPhotoMode();

	UFUNCTION(BlueprintCallable, Category = "PhotoMode")
	void ExitPhotoMode();

	UFUNCTION(BlueprintCallable, Category = "PhotoMode")
	void TakePhoto();

	UFUNCTION(BlueprintPure, Category = "PhotoMode")
	bool IsInPhotoMode() const { return bIsInPhotoMode; }

	FOnPhotoModePreCapture OnPreCapture;
	FOnPhotoModePostCapture OnPostCapture;
	
protected:

	UPROPERTY(Editdefaultsonly, Category = "PhotoMode")
	TSubclassOf<APhotoModeCameraActor> CameraActorClass;

	UPROPERTY(Editdefaultsonly, Category = "PhotoMode")
	UInputMappingContext* PhotoModeMappingContext;

	UPROPERTY()
	APhotoModeCameraActor* SpawnedCameraActor;
	
	bool bIsInPhotoMode = false;
	FDelegateHandle ScreenshotDelegateHandle;

	void OnScreenshotCaptured(int32 Width,  int32 Height, const TArray<FColor>& Bitmap);

	FString GetPhotosDirectory() const;
};
