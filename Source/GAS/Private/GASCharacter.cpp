// Fill out your copyright notice in the Description page of Project Settings.


#include "GASCharacter.h"


// Sets default values
AGASCharacter::AGASCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UBaseAbilitySystemComponent>("AbilitySystemComponent");
}



