// Fill out your copyright notice in the Description page of Project Settings.


#include "PhotoModeCameraActor.h"
#include "Camera/CameraComponent.h"

// Sets default values
APhotoModeCameraActor::APhotoModeCameraActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	RootComponent = CameraComponent;
}

void APhotoModeCameraActor::ApplyMovementInput(const FVector& MoveDelta, const FRotator& LookDelta)
{
	AddActorWorldOffset(MoveDelta, true);
	AddActorLocalRotation(LookDelta); 
}

void APhotoModeCameraActor::BeginPlay()
{
	Super::BeginPlay();
	
}
