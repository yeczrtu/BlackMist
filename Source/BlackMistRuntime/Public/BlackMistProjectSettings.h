#pragma once

#include "CoreMinimal.h"
#include "BlackMistSettings.h"
#include "Engine/DeveloperSettings.h"
#include "BlackMistProjectSettings.generated.h"

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Black Mist"))
class BLACKMISTRUNTIME_API UBlackMistProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBlackMistProjectSettings();

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Black Mist")
	FBlackMistSettings DefaultSettings;

	void ResetToPluginDefaults();

	static FBlackMistSettings GetPluginDefaultSettings();

	virtual FName GetCategoryName() const override;

#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void ApplyToExistingWorlds() const;
};
