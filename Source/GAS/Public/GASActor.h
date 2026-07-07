// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "BaseAbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "GASActor.generated.h"

UCLASS()
class GAS_API AGASActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGASActor();

protected:
	
	UPROPERTY(BlueprintReadOnly)
	UBaseAbilitySystemComponent* AbilitySystemComponent;

public:
	// Called every frame
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override {return AbilitySystemComponent;};
	
};
