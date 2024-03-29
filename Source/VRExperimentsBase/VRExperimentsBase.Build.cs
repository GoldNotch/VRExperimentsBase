// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VRExperimentsBase : ModuleRules
{
	public VRExperimentsBase(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				
			}
			);

		PublicDefinitions.Add("_USE_SCIVI_CONNECTION_");
		PrivateIncludePaths.AddRange(
			new string[] {
				"ThirdPartyLibs",
				"../../SRanipal/Source/SRanipal/Public/Eye"
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "Projects",
				"CoreUObject", "Engine", "InputCore",
				"OpenSSL", "Json", "JsonUtilities",
				"UMG", "Slate", "SlateCore",
				"SRanipal", "SRanipalEye","HeadMountedDisplay",
				"AudioCapture", "SignalProcessing",
				"BlueprintJson", "MediaAssets"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
