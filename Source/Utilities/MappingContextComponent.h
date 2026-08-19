// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MappingContextComponent.generated.h"

class UInputMappingContext;
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class UTILITIES_API UMappingContextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UInputMappingContext*> MappingContexts;
	
	UMappingContextComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	UFUNCTION()
	void AddMappingContext(APlayerController* PlayerController, UInputMappingContext* MappingContext);
	
	UFUNCTION()
	void RemoveMappingContext(APlayerController* PlayerController, UInputMappingContext* MappingContext);
	
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
	void SetupMappingContext();
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
