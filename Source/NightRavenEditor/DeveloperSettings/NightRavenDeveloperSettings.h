// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/SoftObjectPath.h"
#include "NightRavenDeveloperSettings.generated.h"

/**
 
 */
UCLASS(Config=EditorPerProjectUserSettings)
class NIGHTRAVENEDITOR_API UNightRavenDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UNightRavenDeveloperSettings();

	//Interface for settings
	virtual FName GetCategoryName() const override;
	//End of interface

#if WITH_EDITORONLY_DATA
	/**
	 *A List of maps to access through the toolbar
	 **/
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = Maps, meta=(AllowedClasses="/Script/Engine.World"))
	TArray<FSoftObjectPath> EditorMaps;

	UPROPERTY(Config, EditAnywhere, Category="Data Validation")
	bool bSkipDataValidators = false;
	
#endif
};
