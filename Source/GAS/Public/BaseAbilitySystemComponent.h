// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "BaseAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UBaseAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBaseAbilitySystemComponent();
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;
	
	virtual void BeginPlay() override;
};
