#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BlackMistSettings.h"
#include "BlackMistSubsystem.generated.h"

class FBlackMistSceneViewExtension;

UCLASS(BlueprintType)
class BLACKMISTRUNTIME_API UBlackMistSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Black Mist")
	void SetSettings(const FBlackMistSettings& InSettings);

	UFUNCTION(BlueprintPure, Category = "Black Mist")
	FBlackMistSettings GetSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Black Mist")
	void SetEnabled(bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category = "Black Mist")
	void SetIntensity(float InIntensity);

	UFUNCTION(BlueprintCallable, Category = "Black Mist")
	void SetDebugMode(EBlackMistDebugMode InDebugMode);

	UFUNCTION(BlueprintCallable, Category = "Black Mist")
	void ResetToProjectDefaultSettings();

	UFUNCTION(BlueprintCallable, Category = "Black Mist")
	void ResetToPluginDefaultSettings();

private:
	void PushSettingsToRenderThread(bool bForce = false);

	UPROPERTY(EditAnywhere, Category = "Black Mist")
	FBlackMistSettings Settings;

	FBlackMistRenderSettings LastPushedSettings;
	bool bHasPushedSettings = false;
	TSharedPtr<FBlackMistSceneViewExtension, ESPMode::ThreadSafe> SceneViewExtension;
};
