// Fill out your copyright notice in the Description page of Project Settings.


#include "GASPlayerState.h"


AGASPlayerState::AGASPlayerState()
{
	AbilitySystemComponent= CreateDefaultSubobject<UBaseAbilitySystemComponent>("AbilitySystemComponent");
}

void AGASPlayerState::BeginPlay()
{
	Super::BeginPlay();
	if (AttributeSets.Num()>0)
		for (auto Set : AttributeSets)
		{
			UAttributeSet* CreatedAttributeSet = NewObject<UAttributeSet>(AbilitySystemComponent, Set);
			AbilitySystemComponent->AddSpawnedAttribute(CreatedAttributeSet);
		}
			
		
			
}
