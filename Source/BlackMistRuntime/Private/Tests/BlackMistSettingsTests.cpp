#include "BlackMistSettings.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include <limits>

namespace
{
bool IsNearlyEqual(float Actual, float Expected, float Tolerance = 0.0001f)
{
	return FMath::Abs(Actual - Expected) <= Tolerance;
}

float WeightSum(const FVector4f& Weights)
{
	return Weights.X + Weights.Y + Weights.Z + Weights.W;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBlackMistRenderSettingsSanitizationTest,
	"Plugins.BlackMist.Settings.Sanitization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBlackMistRenderSettingsSanitizationTest::RunTest(const FString& Parameters)
{
	const FBlackMistSettings Defaults;
	const float NaN = std::numeric_limits<float>::quiet_NaN();
	const float Inf = std::numeric_limits<float>::infinity();

	FBlackMistSettings InvalidSettings;
	InvalidSettings.Intensity = NaN;
	InvalidSettings.Threshold = -5.0f;
	InvalidSettings.SoftKnee = Inf;
	InvalidSettings.ScatterAmount = -1.0f;
	InvalidSettings.HaloStrength = 99.0f;
	InvalidSettings.CoreLoss = Inf;
	InvalidSettings.Contrast = NaN;
	InvalidSettings.ContrastPivot = -1.0f;
	InvalidSettings.ShadowLift = Inf;
	InvalidSettings.ShadowStart = -2.0f;
	InvalidSettings.ShadowEnd = -4.0f;
	InvalidSettings.MaxScatterRadiance = NaN;
	InvalidSettings.HaloTint = FLinearColor(-1.0f, NaN, Inf, 1.0f);
	InvalidSettings.DiffusionRadius = Inf;
	InvalidSettings.WideTail = -1.0f;
	InvalidSettings.BaseScatter = Inf;
	InvalidSettings.ChromaSensitivity = NaN;
	InvalidSettings.LocalVeilingStrength = Inf;
	InvalidSettings.ScaleWeights = FLinearColor(-1.0f, -2.0f, -3.0f, -4.0f);

	const FBlackMistRenderSettings Sanitized = FBlackMistRenderSettings::FromSettings(InvalidSettings);
	TestEqual(TEXT("NaN Intensity falls back to plugin default"), Sanitized.Intensity, Defaults.Intensity);
	TestEqual(TEXT("Negative Threshold clamps to zero"), Sanitized.Threshold, 0.0f);
	TestEqual(TEXT("Inf SoftKnee falls back to plugin default"), Sanitized.SoftKnee, Defaults.SoftKnee);
	TestEqual(TEXT("Negative ScatterAmount clamps to zero"), Sanitized.ScatterAmount, 0.0f);
	TestEqual(TEXT("HaloStrength clamps to maximum"), Sanitized.HaloStrength, 4.0f);
	TestEqual(TEXT("Inf CoreLoss falls back to plugin default"), Sanitized.CoreLoss, Defaults.CoreLoss);
	TestEqual(TEXT("NaN Contrast falls back to plugin default"), Sanitized.Contrast, Defaults.Contrast);
	TestEqual(TEXT("Negative ContrastPivot clamps to zero"), Sanitized.ContrastPivot, 0.0f);
	TestEqual(TEXT("Inf ShadowLift falls back to plugin default"), Sanitized.ShadowLift, Defaults.ShadowLift);
	TestEqual(TEXT("ShadowStart clamps to zero"), Sanitized.ShadowStart, 0.0f);
	TestEqual(TEXT("ShadowEnd remains greater than ShadowStart"), Sanitized.ShadowEnd, Sanitized.ShadowStart + 0.001f);
	TestEqual(TEXT("NaN MaxScatterRadiance falls back to plugin default"), Sanitized.MaxScatterRadiance, Defaults.MaxScatterRadiance);
	TestEqual(TEXT("Negative tint clamps to zero"), Sanitized.HaloTint.X, 0.0f);
	TestEqual(TEXT("NaN tint falls back to plugin default"), Sanitized.HaloTint.Y, Defaults.HaloTint.G);
	TestEqual(TEXT("Inf tint falls back to plugin default"), Sanitized.HaloTint.Z, Defaults.HaloTint.B);
	TestEqual(TEXT("Inf DiffusionRadius falls back to plugin default"), Sanitized.DiffusionRadius, Defaults.DiffusionRadius);
	TestEqual(TEXT("Negative WideTail clamps to zero"), Sanitized.WideTail, 0.0f);
	TestEqual(TEXT("Inf BaseScatter falls back to plugin default"), Sanitized.BaseScatter, Defaults.BaseScatter);
	TestEqual(TEXT("NaN ChromaSensitivity falls back to plugin default"), Sanitized.ChromaSensitivity, Defaults.ChromaSensitivity);
	TestEqual(TEXT("Inf LocalVeilingStrength falls back to plugin default"), Sanitized.LocalVeilingStrength, Defaults.LocalVeilingStrength);
	TestTrue(TEXT("All-negative ScaleWeights fall back to default weights"), IsNearlyEqual(Sanitized.ScaleWeights.X, Defaults.ScaleWeights.R));
	TestTrue(TEXT("Fallback ScaleWeights stay normalized"), IsNearlyEqual(WeightSum(Sanitized.ScaleWeights), 1.0f));

	FBlackMistSettings CustomWeightsSettings;
	CustomWeightsSettings.ScaleWeights = FLinearColor(2.0f, 1.0f, 1.0f, 0.0f);
	const FBlackMistRenderSettings CustomWeights = FBlackMistRenderSettings::FromSettings(CustomWeightsSettings);
	TestTrue(TEXT("Custom scale weights are normalized"), IsNearlyEqual(WeightSum(CustomWeights.ScaleWeights), 1.0f));
	TestTrue(TEXT("Custom X scale weight is preserved proportionally"), IsNearlyEqual(CustomWeights.ScaleWeights.X, 0.5f));
	TestTrue(TEXT("Custom Y scale weight is preserved proportionally"), IsNearlyEqual(CustomWeights.ScaleWeights.Y, 0.25f));
	TestTrue(TEXT("Custom Z scale weight is preserved proportionally"), IsNearlyEqual(CustomWeights.ScaleWeights.Z, 0.25f));
	TestTrue(TEXT("Custom W scale weight is preserved proportionally"), IsNearlyEqual(CustomWeights.ScaleWeights.W, 0.0f));

	FBlackMistSettings WideTailSettings;
	WideTailSettings.WideTail = 1.0f;
	const FBlackMistRenderSettings WideTail = FBlackMistRenderSettings::FromSettings(WideTailSettings);
	TestTrue(TEXT("Derived wide-tail weights are normalized"), IsNearlyEqual(WeightSum(WideTail.ScaleWeights), 1.0f));
	TestTrue(TEXT("WideTail derives the far-level contribution"), WideTail.ScaleWeights.W > Defaults.ScaleWeights.A);

	return true;
}

#endif
