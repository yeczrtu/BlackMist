#include "BlackMistSubsystem.h"

#include "BlackMistProjectSettings.h"
#include "BlackMistSceneViewExtension.h"
#include "SceneViewExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlackMistSubsystem)

void UBlackMistSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UWorld* World = GetWorld())
	{
		Settings = GetDefault<UBlackMistProjectSettings>()->DefaultSettings;
		SceneViewExtension = FSceneViewExtensions::NewExtension<FBlackMistSceneViewExtension>(World);
		PushSettingsToRenderThread(true);
	}
}

void UBlackMistSubsystem::Deinitialize()
{
	if (SceneViewExtension.IsValid())
	{
		FBlackMistRenderSettings DisabledSettings = LastPushedSettings;
		DisabledSettings.bEnabled = false;
		TSharedPtr<FBlackMistSceneViewExtension, ESPMode::ThreadSafe> Extension = SceneViewExtension;

		ENQUEUE_RENDER_COMMAND(BlackMistDisableExtension)(
			[Extension, DisabledSettings](FRHICommandListImmediate&)
			{
				if (Extension.IsValid())
				{
					Extension->SetRenderSettings_RenderThread(DisabledSettings);
				}
			});
	}

	SceneViewExtension.Reset();
	Super::Deinitialize();
}

void UBlackMistSubsystem::SetSettings(const FBlackMistSettings& InSettings)
{
	Settings = InSettings;
	PushSettingsToRenderThread();
}

FBlackMistSettings UBlackMistSubsystem::GetSettings() const
{
	return Settings;
}

void UBlackMistSubsystem::SetEnabled(bool bInEnabled)
{
	Settings.bEnabled = bInEnabled;
	PushSettingsToRenderThread();
}

void UBlackMistSubsystem::SetIntensity(float InIntensity)
{
	Settings.Intensity = InIntensity;
	PushSettingsToRenderThread();
}

void UBlackMistSubsystem::SetDebugMode(EBlackMistDebugMode InDebugMode)
{
	Settings.DebugMode = InDebugMode;
	PushSettingsToRenderThread();
}

void UBlackMistSubsystem::ResetToProjectDefaultSettings()
{
	SetSettings(GetDefault<UBlackMistProjectSettings>()->DefaultSettings);
}

void UBlackMistSubsystem::ResetToPluginDefaultSettings()
{
	SetSettings(UBlackMistProjectSettings::GetPluginDefaultSettings());
}

void UBlackMistSubsystem::PushSettingsToRenderThread(bool bForce)
{
	if (!SceneViewExtension.IsValid())
	{
		return;
	}

	const FBlackMistRenderSettings Snapshot = FBlackMistRenderSettings::FromSettings(Settings);
	if (!bForce && bHasPushedSettings && Snapshot == LastPushedSettings)
	{
		return;
	}

	LastPushedSettings = Snapshot;
	bHasPushedSettings = true;

	TSharedPtr<FBlackMistSceneViewExtension, ESPMode::ThreadSafe> Extension = SceneViewExtension;
	ENQUEUE_RENDER_COMMAND(BlackMistUpdateSettings)(
		[Extension, Snapshot](FRHICommandListImmediate&)
		{
			if (Extension.IsValid())
			{
				Extension->SetRenderSettings_RenderThread(Snapshot);
			}
		});
}
