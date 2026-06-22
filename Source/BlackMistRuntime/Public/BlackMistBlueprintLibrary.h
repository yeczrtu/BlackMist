#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlackMistSettings.h"
#include "BlackMistBlueprintLibrary.generated.h"

UCLASS()
class BLACKMISTRUNTIME_API UBlackMistBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Black Mist", meta = (WorldContext = "WorldContextObject"))
	static void SetBlackMistSettings(const UObject* WorldContextObject, const FBlackMistSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "Black Mist", meta = (WorldContext = "WorldContextObject"))
	static FBlackMistSettings GetBlackMistSettings(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Black Mist", meta = (WorldContext = "WorldContextObject"))
	static void SetBlackMistEnabled(const UObject* WorldContextObject, bool bEnabled);
};
