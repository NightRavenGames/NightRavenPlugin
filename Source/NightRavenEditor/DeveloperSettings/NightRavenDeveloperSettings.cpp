// Fill out your copyright notice in the Description page of Project Settings.


#include "NightRavenDeveloperSettings.h"
#include "Misc/App.h"

UNightRavenDeveloperSettings::UNightRavenDeveloperSettings()
{
}

FName UNightRavenDeveloperSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}
