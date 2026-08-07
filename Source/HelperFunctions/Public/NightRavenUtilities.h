// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NightRavenUtilities.generated.h"

/**
 * 
 */
UCLASS()
class HELPERFUNCTIONS_API UNightRavenUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure,BlueprintCallable)
	static FVector GetRandomPositionInRadius2D(FVector Origin, float Range);
	
};
