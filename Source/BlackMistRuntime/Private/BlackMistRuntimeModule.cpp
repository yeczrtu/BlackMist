#include "Modules/ModuleManager.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

DEFINE_LOG_CATEGORY_STATIC(LogBlackMist, Log, All);

class FBlackMistRuntimeModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("BlackMist"));
		if (!Plugin.IsValid())
		{
			UE_LOG(LogBlackMist, Error, TEXT("BlackMist plugin descriptor was not found."));
			return;
		}

		const FString PluginShaderDir = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/BlackMist"), PluginShaderDir);
	}

	virtual void ShutdownModule() override
	{
	}
};

IMPLEMENT_MODULE(FBlackMistRuntimeModule, BlackMistRuntime)
