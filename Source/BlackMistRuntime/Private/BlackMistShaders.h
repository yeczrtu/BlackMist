#pragma once

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "ScreenPass.h"

class FBlackMistPrefilterPS final : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FBlackMistPrefilterPS);
	SHADER_USE_PARAMETER_STRUCT(FBlackMistPrefilterPS, FGlobalShader);

public:
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureInput, Input)
		SHADER_PARAMETER(FScreenTransform, SvPositionToInputTextureUV)
		SHADER_PARAMETER(float, Threshold)
		SHADER_PARAMETER(float, SoftKnee)
		SHADER_PARAMETER(float, ScatterAmount)
		SHADER_PARAMETER(float, MaxScatterRadiance)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FBlackMistDownsamplePS final : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FBlackMistDownsamplePS);
	SHADER_USE_PARAMETER_STRUCT(FBlackMistDownsamplePS, FGlobalShader);

public:
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureInput, Input)
		SHADER_PARAMETER(FScreenTransform, SvPositionToInputTextureUV)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FBlackMistUpsamplePS final : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FBlackMistUpsamplePS);
	SHADER_USE_PARAMETER_STRUCT(FBlackMistUpsamplePS, FGlobalShader);

public:
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureInput, LowInput)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureInput, BaseInput)
		SHADER_PARAMETER(FScreenTransform, SvPositionToLowTextureUV)
		SHADER_PARAMETER(FScreenTransform, SvPositionToBaseTextureUV)
		SHADER_PARAMETER(float, BaseWeight)
		SHADER_PARAMETER(float, LowWeight)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};

class FBlackMistCompositePS final : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FBlackMistCompositePS);
	SHADER_USE_PARAMETER_STRUCT(FBlackMistCompositePS, FGlobalShader);

public:
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureInput, SceneColorInput)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureInput, HaloInput)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureInput, DebugD1Input)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureInput, DebugD2Input)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureInput, DebugD3Input)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureInput, DebugD4Input)
		SHADER_PARAMETER(FScreenTransform, SvPositionToSceneColorTextureUV)
		SHADER_PARAMETER(FScreenTransform, SvPositionToHaloTextureUV)
		SHADER_PARAMETER(FScreenTransform, SvPositionToDebugD1TextureUV)
		SHADER_PARAMETER(FScreenTransform, SvPositionToDebugD2TextureUV)
		SHADER_PARAMETER(FScreenTransform, SvPositionToDebugD3TextureUV)
		SHADER_PARAMETER(FScreenTransform, SvPositionToDebugD4TextureUV)
		SHADER_PARAMETER(float, Threshold)
		SHADER_PARAMETER(float, SoftKnee)
		SHADER_PARAMETER(float, Intensity)
		SHADER_PARAMETER(float, HaloStrength)
		SHADER_PARAMETER(float, CoreLoss)
		SHADER_PARAMETER(float, Contrast)
		SHADER_PARAMETER(float, ContrastPivot)
		SHADER_PARAMETER(float, ShadowLift)
		SHADER_PARAMETER(float, ShadowStart)
		SHADER_PARAMETER(float, ShadowEnd)
		SHADER_PARAMETER(FVector3f, HaloTint)
		SHADER_PARAMETER(uint32, DebugMode)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters);
};
