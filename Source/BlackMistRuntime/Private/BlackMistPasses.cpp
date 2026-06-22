#include "BlackMistPasses.h"

#include "BlackMistShaders.h"

#include "HAL/IConsoleManager.h"
#include "PixelShaderUtils.h"
#include "RHIStaticStates.h"

namespace
{
static TAutoConsoleVariable<int32> CVarBlackMistIntermediateFormat(
	TEXT("r.BlackMist.IntermediateFormat"),
	1,
	TEXT("Black Mist intermediate format. 0: Auto, 1: RGBA16F, 2: R11G11B10."),
	ECVF_RenderThreadSafe);

FIntPoint DivideAndRoundUp(const FIntPoint& Size, int32 Divisor)
{
	return FIntPoint(
		FMath::Max(1, FMath::DivideAndRoundUp(Size.X, Divisor)),
		FMath::Max(1, FMath::DivideAndRoundUp(Size.Y, Divisor)));
}

EPixelFormat GetIntermediateFormat()
{
	const int32 FormatMode = CVarBlackMistIntermediateFormat.GetValueOnRenderThread();
	return FormatMode == 2 ? PF_FloatR11G11B10 : PF_FloatRGBA;
}

FScreenPassTexture CreatePyramidTexture(FRDGBuilder& GraphBuilder, const FIntPoint& Extent, const TCHAR* Name)
{
	const FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
		Extent,
		GetIntermediateFormat(),
		FClearValueBinding::Black,
		TexCreate_ShaderResource | TexCreate_RenderTargetable);

	FScreenPassTexture Output(GraphBuilder.CreateTexture(Desc, Name), FIntRect(FIntPoint::ZeroValue, Extent));
	Output.UpdateVisualizeTextureExtent();
	return Output;
}

FScreenTransform MakeSvPositionToTextureUV(const FScreenPassTexture& Output, const FScreenPassTexture& Input)
{
	return (
		FScreenTransform::ChangeTextureBasisFromTo(FScreenPassTextureViewport(Output), FScreenTransform::ETextureBasis::TexelPosition, FScreenTransform::ETextureBasis::ViewportUV) *
		FScreenTransform::ChangeTextureBasisFromTo(FScreenPassTextureViewport(Input), FScreenTransform::ETextureBasis::ViewportUV, FScreenTransform::ETextureBasis::TextureUV));
}

FScreenPassTextureInput MakeInput(FScreenPassTexture Texture, FRHISamplerState* Sampler)
{
	return GetScreenPassTextureInput(Texture, Sampler);
}

void AddPrefilterPass(FRDGBuilder& GraphBuilder, const FSceneView& View, FScreenPassTexture Input, FScreenPassTexture Output, const FBlackMistRenderSettings& Settings)
{
	FRHISamplerState* BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	FBlackMistPrefilterPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FBlackMistPrefilterPS::FParameters>();
	PassParameters->View = View.ViewUniformBuffer;
	PassParameters->Input = MakeInput(Input, BilinearClampSampler);
	PassParameters->SvPositionToInputTextureUV = MakeSvPositionToTextureUV(Output, Input);
	PassParameters->Threshold = Settings.Threshold;
	PassParameters->SoftKnee = Settings.SoftKnee;
	PassParameters->ScatterAmount = Settings.ScatterAmount;
	PassParameters->MaxScatterRadiance = Settings.MaxScatterRadiance;
	PassParameters->RenderTargets[0] = FScreenPassRenderTarget(Output.Texture, Output.ViewRect, ERenderTargetLoadAction::ENoAction).GetRenderTargetBinding();

	TShaderMapRef<FBlackMistPrefilterPS> PixelShader(GetGlobalShaderMap(View.FeatureLevel));
	FPixelShaderUtils::AddFullscreenPass(
		GraphBuilder,
		GetGlobalShaderMap(View.FeatureLevel),
		RDG_EVENT_NAME("BlackMist.PrefilterHalf"),
		PixelShader,
		PassParameters,
		Output.ViewRect);
}

void AddDownsamplePass(FRDGBuilder& GraphBuilder, const FSceneView& View, const TCHAR* EventName, FScreenPassTexture Input, FScreenPassTexture Output)
{
	FRHISamplerState* BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	FBlackMistDownsamplePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FBlackMistDownsamplePS::FParameters>();
	PassParameters->Input = MakeInput(Input, BilinearClampSampler);
	PassParameters->SvPositionToInputTextureUV = MakeSvPositionToTextureUV(Output, Input);
	PassParameters->RenderTargets[0] = FScreenPassRenderTarget(Output.Texture, Output.ViewRect, ERenderTargetLoadAction::ENoAction).GetRenderTargetBinding();

	TShaderMapRef<FBlackMistDownsamplePS> PixelShader(GetGlobalShaderMap(View.FeatureLevel));
	FPixelShaderUtils::AddFullscreenPass(
		GraphBuilder,
		GetGlobalShaderMap(View.FeatureLevel),
		RDG_EVENT_NAME("%s", EventName),
		PixelShader,
		PassParameters,
		Output.ViewRect);
}

void AddUpsamplePass(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const TCHAR* EventName,
	FScreenPassTexture LowInput,
	FScreenPassTexture BaseInput,
	FScreenPassTexture Output,
	float BaseWeight,
	float LowWeight)
{
	FRHISamplerState* BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	FBlackMistUpsamplePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FBlackMistUpsamplePS::FParameters>();
	PassParameters->LowInput = MakeInput(LowInput, BilinearClampSampler);
	PassParameters->BaseInput = MakeInput(BaseInput, BilinearClampSampler);
	PassParameters->SvPositionToLowTextureUV = MakeSvPositionToTextureUV(Output, LowInput);
	PassParameters->SvPositionToBaseTextureUV = MakeSvPositionToTextureUV(Output, BaseInput);
	PassParameters->BaseWeight = BaseWeight;
	PassParameters->LowWeight = LowWeight;
	PassParameters->RenderTargets[0] = FScreenPassRenderTarget(Output.Texture, Output.ViewRect, ERenderTargetLoadAction::ENoAction).GetRenderTargetBinding();

	TShaderMapRef<FBlackMistUpsamplePS> PixelShader(GetGlobalShaderMap(View.FeatureLevel));
	FPixelShaderUtils::AddFullscreenPass(
		GraphBuilder,
		GetGlobalShaderMap(View.FeatureLevel),
		RDG_EVENT_NAME("%s", EventName),
		PixelShader,
		PassParameters,
		Output.ViewRect);
}

void AddCompositePass(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	FScreenPassTexture SceneColor,
	FScreenPassTexture Halo,
	FScreenPassTexture D1,
	FScreenPassTexture D2,
	FScreenPassTexture D3,
	FScreenPassTexture D4,
	FScreenPassRenderTarget Output,
	const FBlackMistRenderSettings& Settings)
{
	FRHISamplerState* BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	FBlackMistCompositePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FBlackMistCompositePS::FParameters>();
	PassParameters->View = View.ViewUniformBuffer;
	PassParameters->SceneColorInput = MakeInput(SceneColor, BilinearClampSampler);
	PassParameters->HaloInput = MakeInput(Halo, BilinearClampSampler);
	PassParameters->DebugD1Input = MakeInput(D1, BilinearClampSampler);
	PassParameters->DebugD2Input = MakeInput(D2, BilinearClampSampler);
	PassParameters->DebugD3Input = MakeInput(D3, BilinearClampSampler);
	PassParameters->DebugD4Input = MakeInput(D4, BilinearClampSampler);
	PassParameters->SvPositionToSceneColorTextureUV = MakeSvPositionToTextureUV(Output, SceneColor);
	PassParameters->SvPositionToHaloTextureUV = MakeSvPositionToTextureUV(Output, Halo);
	PassParameters->SvPositionToDebugD1TextureUV = MakeSvPositionToTextureUV(Output, D1);
	PassParameters->SvPositionToDebugD2TextureUV = MakeSvPositionToTextureUV(Output, D2);
	PassParameters->SvPositionToDebugD3TextureUV = MakeSvPositionToTextureUV(Output, D3);
	PassParameters->SvPositionToDebugD4TextureUV = MakeSvPositionToTextureUV(Output, D4);
	PassParameters->Threshold = Settings.Threshold;
	PassParameters->SoftKnee = Settings.SoftKnee;
	PassParameters->Intensity = Settings.Intensity;
	PassParameters->HaloStrength = Settings.HaloStrength;
	PassParameters->CoreLoss = Settings.CoreLoss;
	PassParameters->Contrast = Settings.Contrast;
	PassParameters->ContrastPivot = Settings.ContrastPivot;
	PassParameters->ShadowLift = Settings.ShadowLift;
	PassParameters->ShadowStart = Settings.ShadowStart;
	PassParameters->ShadowEnd = Settings.ShadowEnd;
	PassParameters->HaloTint = Settings.HaloTint;
	PassParameters->DebugMode = Settings.DebugMode;
	PassParameters->RenderTargets[0] = Output.GetRenderTargetBinding();

	TShaderMapRef<FBlackMistCompositePS> PixelShader(GetGlobalShaderMap(View.FeatureLevel));
	FPixelShaderUtils::AddFullscreenPass(
		GraphBuilder,
		GetGlobalShaderMap(View.FeatureLevel),
		RDG_EVENT_NAME("BlackMist.Composite"),
		PixelShader,
		PassParameters,
		Output.ViewRect);
}
}

FScreenPassTexture AddBlackMistPasses(FRDGBuilder& GraphBuilder, const FSceneView& View, const FBlackMistPassInputs& Inputs)
{
	check(Inputs.SceneColor.IsValid());

	const FIntPoint FullSize = Inputs.SceneColor.ViewRect.Size();
	const FIntPoint HalfSize = DivideAndRoundUp(FullSize, 2);
	const FIntPoint QuarterSize = DivideAndRoundUp(FullSize, 4);
	const FIntPoint EighthSize = DivideAndRoundUp(FullSize, 8);
	const FIntPoint SixteenthSize = DivideAndRoundUp(FullSize, 16);

	FScreenPassTexture D1 = CreatePyramidTexture(GraphBuilder, HalfSize, TEXT("BlackMist.D1.Half"));
	FScreenPassTexture D2 = CreatePyramidTexture(GraphBuilder, QuarterSize, TEXT("BlackMist.D2.Quarter"));
	FScreenPassTexture D3 = CreatePyramidTexture(GraphBuilder, EighthSize, TEXT("BlackMist.D3.Eighth"));
	FScreenPassTexture D4 = CreatePyramidTexture(GraphBuilder, SixteenthSize, TEXT("BlackMist.D4.Sixteenth"));
	FScreenPassTexture U3 = CreatePyramidTexture(GraphBuilder, EighthSize, TEXT("BlackMist.U3.Eighth"));
	FScreenPassTexture U2 = CreatePyramidTexture(GraphBuilder, QuarterSize, TEXT("BlackMist.U2.Quarter"));
	FScreenPassTexture U1 = CreatePyramidTexture(GraphBuilder, HalfSize, TEXT("BlackMist.U1.Half"));

	FScreenPassRenderTarget Output = Inputs.OverrideOutput;
	if (!Output.IsValid())
	{
		Output = FScreenPassRenderTarget::CreateFromInput(GraphBuilder, Inputs.SceneColor, View.GetOverwriteLoadAction(), TEXT("BlackMist.Output"));
	}

	AddPrefilterPass(GraphBuilder, View, Inputs.SceneColor, D1, Inputs.Settings);
	AddDownsamplePass(GraphBuilder, View, TEXT("BlackMist.DownsampleQuarter"), D1, D2);
	AddDownsamplePass(GraphBuilder, View, TEXT("BlackMist.DownsampleEighth"), D2, D3);
	AddDownsamplePass(GraphBuilder, View, TEXT("BlackMist.DownsampleSixteenth"), D3, D4);
	AddUpsamplePass(GraphBuilder, View, TEXT("BlackMist.UpsampleEighth"), D4, D3, U3, Inputs.Settings.ScaleWeights.Z, Inputs.Settings.ScaleWeights.W);
	AddUpsamplePass(GraphBuilder, View, TEXT("BlackMist.UpsampleQuarter"), U3, D2, U2, Inputs.Settings.ScaleWeights.Y, 1.0f);
	AddUpsamplePass(GraphBuilder, View, TEXT("BlackMist.UpsampleHalf"), U2, D1, U1, Inputs.Settings.ScaleWeights.X, 1.0f);
	AddCompositePass(GraphBuilder, View, Inputs.SceneColor, U1, D1, D2, D3, D4, Output, Inputs.Settings);

	return MoveTemp(Output);
}
