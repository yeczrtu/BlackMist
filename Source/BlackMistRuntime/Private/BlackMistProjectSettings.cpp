#include "BlackMistProjectSettings.h"

#include "BlackMistSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlackMistProjectSettings)

UBlackMistProjectSettings::UBlackMistProjectSettings()
{
}

void UBlackMistProjectSettings::ResetToPluginDefaults()
{
	DefaultSettings = GetPluginDefaultSettings();
	SaveConfig();
	ApplyToExistingWorlds();
}

FBlackMistSettings UBlackMistProjectSettings::GetPluginDefaultSettings()
{
	return FBlackMistSettings();
}

FName UBlackMistProjectSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

#if WITH_EDITOR
FText UBlackMistProjectSettings::GetSectionText() const
{
	return NSLOCTEXT("BlackMistProjectSettings", "SectionText", "Black Mist");
}

FText UBlackMistProjectSettings::GetSectionDescription() const
{
	return NSLOCTEXT("BlackMistProjectSettings", "SectionDescription", "Default settings for the Black Mist post effect.");
}

void UBlackMistProjectSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplyToExistingWorlds();
}
#endif

void UBlackMistProjectSettings::ApplyToExistingWorlds() const
{
	if (!GEngine)
	{
		return;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* World = WorldContext.World();
		if (!World)
		{
			continue;
		}

		if (UBlackMistSubsystem* Subsystem = World->GetSubsystem<UBlackMistSubsystem>())
		{
			Subsystem->SetSettings(DefaultSettings);
		}
	}
}
