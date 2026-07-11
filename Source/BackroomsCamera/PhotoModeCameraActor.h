// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhotoModeCameraActor.generated.h"

class UCameraComponent;

UCLASS()
class BACKROOMSCAMERA_API APhotoModeCameraActor : public AActor
{
	GENERATED_BODY()

public:
	APhotoModeCameraActor();

	UPROPERTY(VisibleAnywhere, Category = "PhotoMode")
	UCameraComponent* CameraComponent;

	void ApplyMovementInput(const FVector& MoveDelta, const FRotator& LookDelta);
protected:
	virtual void BeginPlay() override;
};
