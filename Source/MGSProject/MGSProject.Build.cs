// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MGSProject : ModuleRules
{
	public MGSProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"PoseSearch",
			"Chooser",
			"AnimationWarpingRuntime",
			"MotionWarping"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MGSProject",
			"MGSProject/Variant_Platforming",
			"MGSProject/Variant_Platforming/Animation",
			"MGSProject/Variant_Combat",
			"MGSProject/Variant_Combat/AI",
			"MGSProject/Variant_Combat/Animation",
			"MGSProject/Variant_Combat/Gameplay",
			"MGSProject/Variant_Combat/Interfaces",
			"MGSProject/Variant_Combat/UI",
			"MGSProject/Variant_SideScrolling",
			"MGSProject/Variant_SideScrolling/AI",
			"MGSProject/Variant_SideScrolling/Gameplay",
			"MGSProject/Variant_SideScrolling/Interfaces",
			"MGSProject/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
