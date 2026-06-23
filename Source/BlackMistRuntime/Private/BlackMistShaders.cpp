#include "BlackMistShaders.h"

#include "DataDrivenShaderPlatformInfo.h"

bool FBlackMistPrefilterPS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
}

bool FBlackMistDownsamplePS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
}

bool FBlackMistUpsamplePS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
}

bool FBlackMistCompositePS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
}

bool FBlackMistDebugCompositePS::ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
}

IMPLEMENT_GLOBAL_SHADER(FBlackMistPrefilterPS, "/Plugin/BlackMist/Private/BlackMist.usf", "BlackMistPrefilterPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FBlackMistDownsamplePS, "/Plugin/BlackMist/Private/BlackMist.usf", "BlackMistDownsamplePS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FBlackMistUpsamplePS, "/Plugin/BlackMist/Private/BlackMist.usf", "BlackMistUpsamplePS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FBlackMistCompositePS, "/Plugin/BlackMist/Private/BlackMist.usf", "BlackMistCompositePS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FBlackMistDebugCompositePS, "/Plugin/BlackMist/Private/BlackMist.usf", "BlackMistDebugCompositePS", SF_Pixel);
