// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "BaseAbilitySystemComponent.h"
#include "GASPawn.generated.h"

UCLASS()
class GAS_API AGASPawn : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGASPawn();

protected:
	
	UPROPERTY(BlueprintReadOnly)
	UBaseAbilitySystemComponent* AbilitySystemComponent;

public:
	// Called every frame
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override {return AbilitySystemComponent;};	
	
};
