using UnrealBuildTool;

public class BlackMistRuntime : ModuleRules
{
	public BlackMistRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"DeveloperSettings",
				"Engine"
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"RenderCore",
				"Renderer",
				"RHI"
			});
	}
}
