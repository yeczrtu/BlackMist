#pragma once

#include "BlackMistSettings.h"
#include "SceneViewExtension.h"

class FBlackMistSceneViewExtension final : public FWorldSceneViewExtension
{
public:
	FBlackMistSceneViewExtension(const FAutoRegister& AutoRegister, UWorld* InWorld);

	virtual void SubscribeToPostProcessingPass(
		EPostProcessingPass Pass,
		const FSceneView& InView,
		FPostProcessingPassDelegateArray& InOutPassCallbacks,
		bool bIsPassEnabled) override;

	void SetRenderSettings_RenderThread(const FBlackMistRenderSettings& InSettings);

private:
	FScreenPassTexture PostProcessPass_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FSceneView& View,
		const FPostProcessMaterialInputs& Inputs);

	bool ShouldSubscribeForView_RenderThread(const FSceneView& View) const;
	ISceneViewExtension::EPostProcessingPass GetConfiguredPass_RenderThread() const;

	FBlackMistRenderSettings RenderSettings;
};
