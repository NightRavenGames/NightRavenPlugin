#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/Scene.h"
#include "Templates/SubclassOf.h"
#include "IconCreatorTypes.generated.h"

class UTexture2D;
class ULightComponent;
class UMaterialInterface;
class UDecalComponent;
class UStaticMesh;

UENUM(BlueprintType)
enum class EIconBackgroundMode : uint8
{
	Transparent,
	FlatColor,
	Gradient,
	BackgroundImage
};

UENUM(BlueprintType)
enum class EIconGradientShape : uint8
{
	Linear,
	Radial,
	Diamond,
	Conical
};

UENUM(BlueprintType)
enum class EIconBillboardBlend : uint8
{
	Normal,
	Additive,
	Multiply,
	Screen,
	Overlay
};

UENUM(BlueprintType)
enum class EIconStudioLightType : uint8
{
	Directional,
	Point,
	Spot,
	Rect
};

USTRUCT(BlueprintType)
struct FIconStudioLight
{
	GENERATED_BODY()

	// lights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light")
	EIconStudioLightType Type = EIconStudioLightType::Directional;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light", meta = (ClampMin = "0.0"))
	float Intensity = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light")
	bool bCastShadows = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light")
	FRotator Direction = FRotator(-45.f, 135.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light")
	FVector RelativePosition = FVector(1.2f, -1.2f, 0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light", meta = (ClampMin = "0.1"))
	float AttenuationRadiusScale = 6.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light")
	bool bInverseSquaredFalloff = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float InnerConeAngle = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light", meta = (ClampMin = "1.0", ClampMax = "89.0"))
	float OuterConeAngle = 40.f;

	// the light component class captured from the level (Spot, Point, Rect or Directional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light")
	TSubclassOf<ULightComponent> ComponentClass;

	//This is what makes all the light options carry into the icon, not just a hand-picked few (taken by capture from the level)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Light")
	TMap<FString, FString> PropertyValues;
};

USTRUCT(BlueprintType)
struct FIconStudioDecal
{
	GENERATED_BODY()

	// Decals
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Decal")
	TObjectPtr<UMaterialInterface> DecalMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Decal")
	FVector RelativePosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Decal")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Decal")
	FVector DecalSizeScale = FVector(1.f, 1.f, 1.f);
};


USTRUCT(BlueprintType)
struct FIconStudioProp
{
	GENERATED_BODY()

	//Secondary meshes that can be used as a prop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Prop")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Prop")
	FVector RelativePosition = FVector(0.f, 0.f, -1.2f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Prop")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Prop")
	FVector PropScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Prop")
	bool bCastShadow = true;

	// when ON this prop is rendered without effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Prop")
	bool bIgnoreEffects = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Prop")
	TArray<TObjectPtr<UMaterialInterface>> MaterialOverrides;
};

USTRUCT(BlueprintType)
struct FIconStudioBillboard
{
	GENERATED_BODY()

	//billboards
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billboard")
	TObjectPtr<UTexture2D> Texture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billboard")
	FVector2D Position = FVector2D(0.5f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billboard")
	FVector2D Size = FVector2D(0.35f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billboard", meta = (ClampMin = "-360.0", ClampMax = "360.0"))
	float RotationDegrees = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billboard")
	FLinearColor Tint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billboard")
	bool bBehindMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billboard")
	EIconBillboardBlend BlendMode = EIconBillboardBlend::Normal;
};

USTRUCT(BlueprintType)
struct FIconOutlineSettings
{
	GENERATED_BODY()

	//outline
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline")
	bool bEnable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline", meta = (EditCondition = "bEnable"))
	FLinearColor Color = FLinearColor(1.f, 0.35f, 0.05, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline", meta = (ClampMin = "0.0", ClampMax = "128.0", EditCondition = "bEnable"))
	float WidthPixels = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnable"))
	float Softness = 0.6f;

	//when this is ON threat is as a light placed to the side, so only the faces turned towards its glow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline", meta = (EditCondition = "bEnable", DisplayName = "Directional(Rim Lights)"))
	bool bDirectional = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline", meta = (ClampMin = "1.0", ClampMax ="100.0", EditCondition = "bEnable && bDirectional", DisplayName = "Light Cone" ))
	float LightConeWidthPercent = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnable && bDirectional", DisplayName = "Light Edge" ))
	float LightEdgeSoftness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline", meta = (ClampMin = "0.0", ClampMax ="360.0", EditCondition = "bEnable && bDirectional", DisplayName = "Light Direction" ))
	float LightDirectionDegrees = 45.f;
	
};

USTRUCT(BlueprintType)
struct FIconEffectsSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Effects")
	TArray<FIconStudioLight> Lights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Effects", meta = (ClampMin = "1.0", DisplayName = "Stage Light Reference Radius"))
	float LightReferenceRadius = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Effects")
	bool bHasPostProcess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Effects")
	FPostProcessSettings PostProcess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Effects")
	TArray<FIconStudioDecal> Decals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Effects")
	TArray<FIconStudioBillboard> Billboards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Effects")
	TArray<FIconStudioProp> Props;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Effects")
	FIconOutlineSettings Outline;
};

USTRUCT(BlueprintType)
struct FIconExposureSettings
{
	GENERATED_BODY()

	//exposure
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Exposure")
	bool bAutoExpose = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Exposure", meta = (ClampMin = "-10.0", ClampMax = "10.0"))
	float ExposureCompensation = 0.0f;
};

USTRUCT(BlueprintType)
struct FIconOutputSettings
{
	GENERATED_BODY()

	//output
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OutputFolder = TEXT("/Game/GeneratedIcons");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NamePrefix = TEXT("T_Icon_");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Strip SM_ Prefix"))
	bool bStripSMPrefix = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOverwriteExisting = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSkipIfUnchanged = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Also Export PNG"))
	bool bAlsoExportPNG = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bAlsoExportPNG", DisplayName = "PNG Export Directory"))
	FString PNGExportDirectory;
};

USTRUCT(BlueprintType)
struct FIconGenSettings
{
	GENERATED_BODY()
	// framing & resolution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator", meta = (ClampMin = "16", ClampMax = "4096"))
	int32 Resolution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator")
	bool bOrthographic = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator", meta = (ClampMin = "5", ClampMax = "120", EditCondition = "!bOrthographic"))
	float FOV = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator")
	FRotator CameraOrbit = FRotator(-30.f, 45.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator")
	FRotator MeshRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator", meta = (DisplayName = "Lock Mesh Scale Axes"))
	bool bUniformMeshScale = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator", meta = (ClampMin = "0.01", ClampMax = "10.0", EditCondition = "bUniformMeshScale", DisplayName = "Mesh Scale"))
	float MeshScaleUniform = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator", meta = (EditCondition = "!bUniformMeshScale"))
	FVector MeshScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Padding = 0.12f;

	// materials override
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Materials")
	TArray<TObjectPtr<UMaterialInterface>> MaterialOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Background")
	EIconBackgroundMode BackgroundMode = EIconBackgroundMode::Transparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Background", meta = (EditCondition = "BackgroundMode == EIconBackgroundMode::FlatColor"))
	FLinearColor FlatBackgroundColor = FLinearColor(0.03f, 0.03f, 0.05, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Background", meta =(EditCondition = "BackgroundMode == EIconBackgroundMode::Gradient", DisplayName = "Gradient")) 
	FLinearColor GradientColorB = FLinearColor(0.25f, 0.05f, 0.35f, 1.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Background", meta = (EditCondition = "BackgroundMode == EIconBackgroundMode::Gradient"))
	EIconGradientShape GradientShape = EIconGradientShape::Linear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Background", meta = (ClampMin = "0.0", ClampMax = "360.0", EditCondition = "BackgroundMode == EIconBackgroundMode::Gradient"))
	float GradientAngle = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Background", meta = (ClampMin = "0.0", ClampMax = "1.0" , EditCondition = "BackgroundMode == EIconBackgroundMode::Gradient"))
	float GradientMidpoint = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Background", meta = (ClampMin = "0.01", ClampMax = "1.0", EditCondition = "BackgroundMode == EIconBackgroundMode::Gradient"))
	float GradientSoftness = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Background", meta = (EditCondition = "BackgroundMode == EIconBackgroundMode::BackgroundImage"))
	TObjectPtr<UTexture2D> BackgroundTexture = nullptr; // The image to sit behind the mesh. TObjectPtr is UE5's GC-tracked pointer wrapper

	//
	//Effects (lights, shadows, post process, decals, billboards, outline)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Effects")
	bool bMeshCastShadows = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Effects", meta = (ShowOnlyInnerProperties))
	FIconEffectsSettings Effects;

	UPROPERTY(BlueprintReadWrite)
	FIconOutlineSettings Output;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Icon Creator|Exposure", meta = (ShowOnlyInnerProperties))
	FIconExposureSettings Exposure;
	
};


UCLASS(config = Editor, DefaultConfig, meta = (DisplayName = "Icon Creator"))
class ICONCREATOR_API UIconCreatorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	// override the category so this page appears under "Plugins" in Project Settings rather than a default heading
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); };

	// a full settings struct; this is what makes it persist to the .ini file between editor sessions
	UPROPERTY(Config, EditAnywhere, Category = "Icon Creator")
	FIconGenSettings DefaultSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Icon Creator|Presets")
	TMap<FString, FIconGenSettings> Presets;
};

//persistent data attached to each GENERATED icon texture so it saves inside the .uasset and * survives editor restarts
// this also allows it to compare "What made this icon last time" to "What would make it now" and skip the work when they match

UCLASS()
class ICONCREATOR_API UIconCreatorIconTag : public UAssetUserData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString SourceHash;
};