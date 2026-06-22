#pragma once

#include "CoreMinimal.h"
#include "BlackMistSettings.generated.h"

UENUM(BlueprintType)
enum class EBlackMistDebugMode : uint8
{
	Final = 0 UMETA(DisplayName = "Final"),
	ScatterMask = 1 UMETA(DisplayName = "Scatter Mask"),
	PrefilterHalf = 2 UMETA(DisplayName = "D1 Half Prefilter"),
	DownsampleQuarter = 3 UMETA(DisplayName = "D2 Quarter"),
	DownsampleEighth = 4 UMETA(DisplayName = "D3 Eighth"),
	DownsampleSixteenth = 5 UMETA(DisplayName = "D4 Sixteenth"),
	AccumulatedHalo = 6 UMETA(DisplayName = "Accumulated Halo"),
	CoreLossOnly = 7 UMETA(DisplayName = "Core Loss Only"),
	HaloOnly = 8 UMETA(DisplayName = "Halo Only")
};

USTRUCT(BlueprintType)
struct BLACKMISTRUNTIME_API FBlackMistSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "1.0"))
	float Intensity = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "8.0"))
	float Threshold = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float SoftKnee = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float ScatterAmount = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", ClampMax = "4.0", UIMin = "0.0", UIMax = "2.0"))
	float HaloStrength = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float CoreLoss = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.75", ClampMax = "1.05", UIMin = "0.85", UIMax = "1.0"))
	float Contrast = 0.96f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float ContrastPivot = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", ClampMax = "0.1", UIMin = "0.0", UIMax = "0.02"))
	float ShadowLift = 0.004f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "0.2"))
	float ShadowStart = 0.03f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0"))
	float ShadowEnd = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	FLinearColor HaloTint = FLinearColor(1.0f, 0.965f, 0.90f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", UIMin = "1.0", UIMax = "256.0"))
	float MaxScatterRadiance = 64.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	FLinearColor ScaleWeights = FLinearColor(0.34f, 0.30f, 0.22f, 0.14f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	EBlackMistDebugMode DebugMode = EBlackMistDebugMode::Final;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	bool bAffectSceneCaptures = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	bool bAffectEditorViewports = true;
};

struct BLACKMISTRUNTIME_API FBlackMistRenderSettings
{
	bool bEnabled = true;
	bool bAffectSceneCaptures = false;
	bool bAffectEditorViewports = true;
	float Intensity = 0.45f;
	float Threshold = 1.0f;
	float SoftKnee = 0.65f;
	float ScatterAmount = 0.12f;
	float HaloStrength = 0.75f;
	float CoreLoss = 0.30f;
	float Contrast = 0.96f;
	float ContrastPivot = 0.18f;
	float ShadowLift = 0.004f;
	float ShadowStart = 0.03f;
	float ShadowEnd = 0.30f;
	float MaxScatterRadiance = 64.0f;
	FVector3f HaloTint = FVector3f(1.0f, 0.965f, 0.90f);
	FVector4f ScaleWeights = FVector4f(0.34f, 0.30f, 0.22f, 0.14f);
	uint32 DebugMode = 0;

	static FBlackMistRenderSettings FromSettings(const FBlackMistSettings& InSettings)
	{
		FBlackMistRenderSettings Out;
		Out.bEnabled = InSettings.bEnabled;
		Out.bAffectSceneCaptures = InSettings.bAffectSceneCaptures;
		Out.bAffectEditorViewports = InSettings.bAffectEditorViewports;
		Out.Intensity = FMath::Clamp(InSettings.Intensity, 0.0f, 2.0f);
		Out.Threshold = FMath::Max(InSettings.Threshold, 0.0f);
		Out.SoftKnee = FMath::Clamp(InSettings.SoftKnee, 0.0f, 1.0f);
		Out.ScatterAmount = FMath::Clamp(InSettings.ScatterAmount, 0.0f, 1.0f);
		Out.HaloStrength = FMath::Clamp(InSettings.HaloStrength, 0.0f, 4.0f);
		Out.CoreLoss = FMath::Clamp(InSettings.CoreLoss, 0.0f, 1.0f);
		Out.Contrast = FMath::Clamp(InSettings.Contrast, 0.75f, 1.05f);
		Out.ContrastPivot = FMath::Max(InSettings.ContrastPivot, 0.0f);
		Out.ShadowLift = FMath::Clamp(InSettings.ShadowLift, 0.0f, 0.1f);
		Out.ShadowStart = FMath::Max(InSettings.ShadowStart, 0.0f);
		Out.ShadowEnd = FMath::Max(InSettings.ShadowEnd, Out.ShadowStart + 0.001f);
		Out.MaxScatterRadiance = FMath::Max(InSettings.MaxScatterRadiance, 0.0f);
		Out.HaloTint = FVector3f(
			FMath::Max(InSettings.HaloTint.R, 0.0f),
			FMath::Max(InSettings.HaloTint.G, 0.0f),
			FMath::Max(InSettings.HaloTint.B, 0.0f));
		Out.ScaleWeights = FVector4f(
			FMath::Max(InSettings.ScaleWeights.R, 0.0f),
			FMath::Max(InSettings.ScaleWeights.G, 0.0f),
			FMath::Max(InSettings.ScaleWeights.B, 0.0f),
			FMath::Max(InSettings.ScaleWeights.A, 0.0f));

		const float WeightSum = Out.ScaleWeights.X + Out.ScaleWeights.Y + Out.ScaleWeights.Z + Out.ScaleWeights.W;
		Out.ScaleWeights = WeightSum > UE_SMALL_NUMBER ? Out.ScaleWeights / WeightSum : FVector4f(0.34f, 0.30f, 0.22f, 0.14f);
		Out.DebugMode = FMath::Clamp(static_cast<uint32>(InSettings.DebugMode), 0u, 8u);
		return Out;
	}

	bool IsEffectActive() const
	{
		return bEnabled && Intensity > 0.0001f;
	}

	bool operator==(const FBlackMistRenderSettings& Other) const
	{
		return bEnabled == Other.bEnabled
			&& bAffectSceneCaptures == Other.bAffectSceneCaptures
			&& bAffectEditorViewports == Other.bAffectEditorViewports
			&& Intensity == Other.Intensity
			&& Threshold == Other.Threshold
			&& SoftKnee == Other.SoftKnee
			&& ScatterAmount == Other.ScatterAmount
			&& HaloStrength == Other.HaloStrength
			&& CoreLoss == Other.CoreLoss
			&& Contrast == Other.Contrast
			&& ContrastPivot == Other.ContrastPivot
			&& ShadowLift == Other.ShadowLift
			&& ShadowStart == Other.ShadowStart
			&& ShadowEnd == Other.ShadowEnd
			&& MaxScatterRadiance == Other.MaxScatterRadiance
			&& HaloTint == Other.HaloTint
			&& ScaleWeights == Other.ScaleWeights
			&& DebugMode == Other.DebugMode;
	}

	bool operator!=(const FBlackMistRenderSettings& Other) const
	{
		return !(*this == Other);
	}
};
