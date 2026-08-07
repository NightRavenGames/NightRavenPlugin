// Fill out your copyright notice in the Description page of Project Settings.


#include "NightRavenUtilities.h"

FVector UNightRavenUtilities::GetRandomPositionInRadius2D(FVector Origin, float Radius)
{
	float RandomPoint = FMath::RandRange(0,360);
	float PointX = FMath::Cos(RandomPoint) *  Radius;
	float PointY = FMath::Sin(RandomPoint) *  Radius;
	FVector Point = FVector(Origin.X+PointX,Origin.Y+PointY,Origin.Z);
	
	return Point;
}
