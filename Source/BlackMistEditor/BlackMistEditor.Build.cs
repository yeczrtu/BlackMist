using UnrealBuildTool;

public class BlackMistEditor : ModuleRules
{
	public BlackMistEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"BlackMistRuntime",
				"Core",
				"CoreUObject",
				"Engine",
				"PropertyEditor",
				"Slate",
				"SlateCore",
				"UnrealEd"
			});
	}
}
