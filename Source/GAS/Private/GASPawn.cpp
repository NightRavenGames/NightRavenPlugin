// Fill out your copyright notice in the Description page of Project Settings.


#include "GASPawn.h"


// Sets default values
AGASPawn::AGASPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UBaseAbilitySystemComponent>("AbilitySystemComponent");

}

// Called when the game starts or when spawned


