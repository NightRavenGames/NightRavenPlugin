// Fill out your copyright notice in the Description page of Project Settings.


#include "GASPlayerState.h"

#include "BaseAttributeSet.h"



AGASPlayerState::AGASPlayerState()
{
	AbilitySystemComponent= CreateDefaultSubobject<UBaseAbilitySystemComponent>("AbilitySystemComponent");
}

void AGASPlayerState::BeginPlay()
{
	TArray<TSubclassOf<UAttributeSet>> AttributeSetsArray;
	TArray<UDataTable*> DataTables;
	AttributeSets.GetKeys(AttributeSetsArray);
	
	if (AttributeSets.Num()>0)
		for (int i=0;i<AttributeSets.Num();i++)
		{
			UAttributeSet* CreatedAttributeSet = NewObject<UAttributeSet>(this, AttributeSetsArray[i]);
			
			if (AttributeSets.Find(AttributeSetsArray[i]) != nullptr)
				CreatedAttributeSet->InitFromMetaDataTable(*AttributeSets.Find(AttributeSetsArray[i]));
			
			AbilitySystemComponent->AddSpawnedAttribute(CreatedAttributeSet);
		}
	
	
	Super::BeginPlay();
			
		
			
}
