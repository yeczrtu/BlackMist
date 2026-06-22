#include "BlackMistBlueprintLibrary.h"

#include "BlackMistSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlackMistBlueprintLibrary)

namespace
{
UBlackMistSubsystem* GetBlackMistSubsystem(const UObject* WorldContextObject)
{
	if (!GEngine)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	return World ? World->GetSubsystem<UBlackMistSubsystem>() : nullptr;
}
}

void UBlackMistBlueprintLibrary::SetBlackMistSettings(const UObject* WorldContextObject, const FBlackMistSettings& Settings)
{
	if (UBlackMistSubsystem* Subsystem = GetBlackMistSubsystem(WorldContextObject))
	{
		Subsystem->SetSettings(Settings);
	}
}

FBlackMistSettings UBlackMistBlueprintLibrary::GetBlackMistSettings(const UObject* WorldContextObject)
{
	if (const UBlackMistSubsystem* Subsystem = GetBlackMistSubsystem(WorldContextObject))
	{
		return Subsystem->GetSettings();
	}

	return FBlackMistSettings();
}

void UBlackMistBlueprintLibrary::SetBlackMistEnabled(const UObject* WorldContextObject, bool bEnabled)
{
	if (UBlackMistSubsystem* Subsystem = GetBlackMistSubsystem(WorldContextObject))
	{
		Subsystem->SetEnabled(bEnabled);
	}
}
