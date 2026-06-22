#pragma once

#include "BlackMistSettings.h"
#include "ScreenPass.h"

struct FBlackMistPassInputs
{
	FScreenPassTexture SceneColor;
	FScreenPassRenderTarget OverrideOutput;
	FBlackMistRenderSettings Settings;
};

FScreenPassTexture AddBlackMistPasses(FRDGBuilder& GraphBuilder, const FSceneView& View, const FBlackMistPassInputs& Inputs);
