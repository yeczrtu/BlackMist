#include "BlackMistProjectSettings.h"

#include "BlackMistSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlackMistProjectSettings)

UBlackMistProjectSettings::UBlackMistProjectSettings()
{
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
#endif
