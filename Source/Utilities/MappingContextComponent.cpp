// Fill out your copyright notice in the Description page of Project Settings.


#include "EnhancedInputSubsystems.h"
#include "MappingContextComponent.h"



// Sets default values for this component's properties
UMappingContextComponent::UMappingContextComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMappingContextComponent::BeginPlay()
{
	Super::BeginPlay();
	SetupMappingContext();
}


// Called every frame
void UMappingContextComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMappingContextComponent::SetupMappingContext()
{
	if (Cast<APawn>(GetOwner()) && IsValid(Cast<APawn>(GetOwner())->Controller))
	{
		APawn* Owner = Cast<APawn>(GetOwner());
		
		for (auto MappingContext : MappingContexts)
			AddMappingContext(Cast<APlayerController>(Owner->Controller), MappingContext);
	}
	else
		GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Red,"Owner is not a Pawn, mapping contexts will not be added");
		
	
}

void UMappingContextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (auto MappingContext : MappingContexts)
		RemoveMappingContext(Cast<APlayerController>(Cast<APawn>(GetOwner())->Controller), MappingContext);
	Super::EndPlay(EndPlayReason);
}

void UMappingContextComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	if (!IsValid(Pawn) || !IsValid(NewController)) return;
	for (auto MappingContext : MappingContexts)
		AddMappingContext(Cast<APlayerController>(NewController), MappingContext);
	
}

void UMappingContextComponent::AddMappingContext(APlayerController* PlayerController, UInputMappingContext* MappingContext)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (Subsystem->HasMappingContext(MappingContext)) return;
	
	Subsystem->AddMappingContext(MappingContext,0);
	GEngine->AddOnScreenDebugMessage(-1,5.0,FColor::Green, "Mapping context added :)");
}

void UMappingContextComponent::RemoveMappingContext(APlayerController* PlayerController, UInputMappingContext* MappingContext)
{
	
	if (!IsValid (PlayerController)) return;
	if (!IsValid(PlayerController->GetLocalPlayer())) return;
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!Subsystem->HasMappingContext(MappingContext)) return;
	
	Subsystem->RemoveMappingContext(MappingContext);
	GEngine->AddOnScreenDebugMessage(-1,5.0,FColor::Green, "Mapping context removed :)");
}

