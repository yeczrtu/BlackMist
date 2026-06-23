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

UENUM(BlueprintType)
enum class EBlackMistScatterMetric : uint8
{
	Perceptual = 0 UMETA(DisplayName = "Perceptual Luminance"),
	Peak = 1 UMETA(DisplayName = "Peak Channel"),
	Hybrid = 2 UMETA(DisplayName = "Hybrid")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.5", ClampMax = "3.0", UIMin = "0.5", UIMax = "3.0"))
	float DiffusionRadius = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float WideTail = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", ClampMax = "0.05", UIMin = "0.0", UIMax = "0.05"))
	float BaseScatter = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	EBlackMistScatterMetric ScatterMetric = EBlackMistScatterMetric::Hybrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float ChromaSensitivity = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float LocalVeilingStrength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	EBlackMistDebugMode DebugMode = EBlackMistDebugMode::Final;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	bool bAffectSceneCaptures = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	bool bAffectEditorViewports = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Black Mist")
	bool bAffectPathTracing = true;
};

struct BLACKMISTRUNTIME_API FBlackMistRenderSettings
{
	bool bEnabled = true;
	bool bAffectSceneCaptures = false;
	bool bAffectEditorViewports = true;
	bool bAffectPathTracing = true;
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
	float DiffusionRadius = 1.0f;
	float WideTail = 0.4f;
	float BaseScatter = 0.0f;
	float ChromaSensitivity = 0.25f;
	float LocalVeilingStrength = 1.0f;
	uint32 ScatterMetric = 2;
	uint32 DebugMode = 0;

	static FBlackMistRenderSettings FromSettings(const FBlackMistSettings& InSettings)
	{
		const FBlackMistSettings Defaults;
		FBlackMistRenderSettings Out;
		Out.bEnabled = InSettings.bEnabled;
		Out.bAffectSceneCaptures = InSettings.bAffectSceneCaptures;
		Out.bAffectEditorViewports = InSettings.bAffectEditorViewports;
		Out.bAffectPathTracing = InSettings.bAffectPathTracing;
		Out.Intensity = FMath::Clamp(FiniteOr(InSettings.Intensity, Defaults.Intensity), 0.0f, 2.0f);
		Out.Threshold = FMath::Max(FiniteOr(InSettings.Threshold, Defaults.Threshold), 0.0f);
		Out.SoftKnee = FMath::Clamp(FiniteOr(InSettings.SoftKnee, Defaults.SoftKnee), 0.0f, 1.0f);
		Out.ScatterAmount = FMath::Clamp(FiniteOr(InSettings.ScatterAmount, Defaults.ScatterAmount), 0.0f, 1.0f);
		Out.HaloStrength = FMath::Clamp(FiniteOr(InSettings.HaloStrength, Defaults.HaloStrength), 0.0f, 4.0f);
		Out.CoreLoss = FMath::Clamp(FiniteOr(InSettings.CoreLoss, Defaults.CoreLoss), 0.0f, 1.0f);
		Out.Contrast = FMath::Clamp(FiniteOr(InSettings.Contrast, Defaults.Contrast), 0.75f, 1.05f);
		Out.ContrastPivot = FMath::Max(FiniteOr(InSettings.ContrastPivot, Defaults.ContrastPivot), 0.0f);
		Out.ShadowLift = FMath::Clamp(FiniteOr(InSettings.ShadowLift, Defaults.ShadowLift), 0.0f, 0.1f);
		Out.ShadowStart = FMath::Max(FiniteOr(InSettings.ShadowStart, Defaults.ShadowStart), 0.0f);
		Out.ShadowEnd = FMath::Max(FiniteOr(InSettings.ShadowEnd, Defaults.ShadowEnd), Out.ShadowStart + 0.001f);
		Out.MaxScatterRadiance = FMath::Max(FiniteOr(InSettings.MaxScatterRadiance, Defaults.MaxScatterRadiance), 0.0f);
		Out.HaloTint = FVector3f(
			FMath::Max(FiniteOr(InSettings.HaloTint.R, Defaults.HaloTint.R), 0.0f),
			FMath::Max(FiniteOr(InSettings.HaloTint.G, Defaults.HaloTint.G), 0.0f),
			FMath::Max(FiniteOr(InSettings.HaloTint.B, Defaults.HaloTint.B), 0.0f));
		Out.DiffusionRadius = FMath::Clamp(FiniteOr(InSettings.DiffusionRadius, Defaults.DiffusionRadius), 0.5f, 3.0f);
		Out.WideTail = FMath::Clamp(FiniteOr(InSettings.WideTail, Defaults.WideTail), 0.0f, 1.0f);
		Out.BaseScatter = FMath::Clamp(FiniteOr(InSettings.BaseScatter, Defaults.BaseScatter), 0.0f, 0.05f);
		Out.ChromaSensitivity = FMath::Clamp(FiniteOr(InSettings.ChromaSensitivity, Defaults.ChromaSensitivity), 0.0f, 1.0f);
		Out.LocalVeilingStrength = FMath::Clamp(FiniteOr(InSettings.LocalVeilingStrength, Defaults.LocalVeilingStrength), 0.0f, 1.0f);
		Out.ScatterMetric = FMath::Clamp(static_cast<uint32>(InSettings.ScatterMetric), 0u, 2u);
		Out.ScaleWeights = DeriveScaleWeights(InSettings, Defaults, Out.WideTail);
		Out.DebugMode = FMath::Clamp(static_cast<uint32>(InSettings.DebugMode), 0u, 8u);
		return Out;
	}

	static float FiniteOr(float Value, float Fallback)
	{
		return FMath::IsFinite(Value) ? Value : Fallback;
	}

	static FVector4f NormalizeWeights(const FVector4f& Weights, const FVector4f& Fallback)
	{
		const FVector4f NonNegative(
			FMath::Max(Weights.X, 0.0f),
			FMath::Max(Weights.Y, 0.0f),
			FMath::Max(Weights.Z, 0.0f),
			FMath::Max(Weights.W, 0.0f));

		const float WeightSum = NonNegative.X + NonNegative.Y + NonNegative.Z + NonNegative.W;
		return WeightSum > UE_SMALL_NUMBER ? NonNegative / WeightSum : Fallback;
	}

	static FVector4f DeriveScaleWeights(const FBlackMistSettings& InSettings, const FBlackMistSettings& Defaults, float WideTail)
	{
		const FVector4f DefaultWeights(Defaults.ScaleWeights.R, Defaults.ScaleWeights.G, Defaults.ScaleWeights.B, Defaults.ScaleWeights.A);
		const bool bHasCustomScaleWeights =
			!FMath::IsNearlyEqual(InSettings.ScaleWeights.R, Defaults.ScaleWeights.R) ||
			!FMath::IsNearlyEqual(InSettings.ScaleWeights.G, Defaults.ScaleWeights.G) ||
			!FMath::IsNearlyEqual(InSettings.ScaleWeights.B, Defaults.ScaleWeights.B) ||
			!FMath::IsNearlyEqual(InSettings.ScaleWeights.A, Defaults.ScaleWeights.A);

		if (bHasCustomScaleWeights)
		{
			const FVector4f CustomWeights(
				FiniteOr(InSettings.ScaleWeights.R, Defaults.ScaleWeights.R),
				FiniteOr(InSettings.ScaleWeights.G, Defaults.ScaleWeights.G),
				FiniteOr(InSettings.ScaleWeights.B, Defaults.ScaleWeights.B),
				FiniteOr(InSettings.ScaleWeights.A, Defaults.ScaleWeights.A));
			return NormalizeWeights(CustomWeights, DefaultWeights);
		}

		const FVector4f NarrowWeights(0.42f, 0.31f, 0.18f, 0.09f);
		const FVector4f WideWeights(0.22f, 0.285f, 0.28f, 0.215f);
		const FVector4f DerivedWeights(
			FMath::Lerp(NarrowWeights.X, WideWeights.X, WideTail),
			FMath::Lerp(NarrowWeights.Y, WideWeights.Y, WideTail),
			FMath::Lerp(NarrowWeights.Z, WideWeights.Z, WideTail),
			FMath::Lerp(NarrowWeights.W, WideWeights.W, WideTail));
		return NormalizeWeights(DerivedWeights, DefaultWeights);
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
			&& bAffectPathTracing == Other.bAffectPathTracing
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
			&& DiffusionRadius == Other.DiffusionRadius
			&& WideTail == Other.WideTail
			&& BaseScatter == Other.BaseScatter
			&& ChromaSensitivity == Other.ChromaSensitivity
			&& LocalVeilingStrength == Other.LocalVeilingStrength
			&& ScatterMetric == Other.ScatterMetric
			&& DebugMode == Other.DebugMode;
	}

	bool operator!=(const FBlackMistRenderSettings& Other) const
	{
		return !(*this == Other);
	}
};
