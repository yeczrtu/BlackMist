#include "BlackMistSceneViewExtension.h"

#include "BlackMistPasses.h"
#include "Engine/EngineTypes.h"
#include "HAL/IConsoleManager.h"
#include "PostProcess/PostProcessMaterialInputs.h"
#include "SceneView.h"

namespace
{
static TAutoConsoleVariable<int32> CVarBlackMistEnable(
	TEXT("r.BlackMist.Enable"),
	1,
	TEXT("Globally enables Black Mist rendering."),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarBlackMistDebug(
	TEXT("r.BlackMist.Debug"),
	-1,
	TEXT("Black Mist debug mode. -1: use subsystem setting, 0: final, 1: mask, 2: D1, 3: D2, 4: D3, 5: D4, 6: accumulated halo, 7: core-only, 8: halo-only."),
	ECVF_RenderThreadSafe);

static TAutoConsoleVariable<int32> CVarBlackMistForcePassLocation(
	TEXT("r.BlackMist.ForcePassLocation"),
	0,
	TEXT("Development override for Black Mist callback location. 0: MotionBlur/BeforeBloom, 1: AfterDOF."),
	ECVF_RenderThreadSafe);
}

FBlackMistSceneViewExtension::FBlackMistSceneViewExtension(const FAutoRegister& AutoRegister, UWorld* InWorld)
	: FWorldSceneViewExtension(AutoRegister, InWorld)
{
}

void FBlackMistSceneViewExtension::SubscribeToPostProcessingPass(
	EPostProcessingPass Pass,
	const FSceneView& InView,
	FPostProcessingPassDelegateArray& InOutPassCallbacks,
	bool bIsPassEnabled)
{
	if (Pass != GetConfiguredPass_RenderThread())
	{
		return;
	}

	if (!ShouldSubscribeForView_RenderThread(InView))
	{
		return;
	}

	InOutPassCallbacks.Add(FPostProcessingPassDelegate::CreateRaw(this, &FBlackMistSceneViewExtension::PostProcessPass_RenderThread));
}

void FBlackMistSceneViewExtension::SetRenderSettings_RenderThread(const FBlackMistRenderSettings& InSettings)
{
	RenderSettings = InSettings;
}

FScreenPassTexture FBlackMistSceneViewExtension::PostProcessPass_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const FPostProcessMaterialInputs& Inputs)
{
	if (!ShouldSubscribeForView_RenderThread(View))
	{
		return Inputs.ReturnUntouchedSceneColorForPostProcessing(GraphBuilder);
	}

	const FScreenPassTextureSlice SceneColorSlice = Inputs.GetInput(EPostProcessMaterialInput::SceneColor);
	if (!SceneColorSlice.IsValid())
	{
		return Inputs.ReturnUntouchedSceneColorForPostProcessing(GraphBuilder);
	}

	FBlackMistRenderSettings Settings = RenderSettings;
	const int32 DebugOverride = CVarBlackMistDebug.GetValueOnRenderThread();
	if (DebugOverride >= 0)
	{
		Settings.DebugMode = FMath::Clamp(static_cast<uint32>(DebugOverride), 0u, 8u);
	}

	FBlackMistPassInputs PassInputs;
	PassInputs.SceneColor = FScreenPassTexture::CopyFromSlice(GraphBuilder, SceneColorSlice);
	PassInputs.OverrideOutput = Inputs.OverrideOutput;
	PassInputs.Settings = Settings;

	return AddBlackMistPasses(GraphBuilder, View, PassInputs);
}

bool FBlackMistSceneViewExtension::ShouldSubscribeForView_RenderThread(const FSceneView& View) const
{
	if (!RenderSettings.IsEffectActive())
	{
		return false;
	}

	if (CVarBlackMistEnable.GetValueOnRenderThread() == 0)
	{
		return false;
	}

	if (View.FeatureLevel < ERHIFeatureLevel::SM5)
	{
		return false;
	}

	if (!View.Family || !View.Family->Scene)
	{
		return false;
	}

	if (View.bIsSceneCapture && !RenderSettings.bAffectSceneCaptures)
	{
		return false;
	}

	if (View.bIsReflectionCapture || View.bIsPlanarReflection || View.bIsVirtualTexture)
	{
		return false;
	}

	if (!RenderSettings.bAffectEditorViewports && !View.bIsGameView)
	{
		return false;
	}

	if (View.Family->EngineShowFlags.PathTracing)
	{
		return false;
	}

	return true;
}

ISceneViewExtension::EPostProcessingPass FBlackMistSceneViewExtension::GetConfiguredPass_RenderThread() const
{
	return CVarBlackMistForcePassLocation.GetValueOnRenderThread() == 1
		? EPostProcessingPass::AfterDOF
		: EPostProcessingPass::MotionBlur;
}
