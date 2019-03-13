using UnrealBuildTool;
using System.IO;

public class SteamVRInputDevice : ModuleRules
{
	public SteamVRInputDevice(TargetInfo Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivatePCHHeaderFile = "Public/ISteamVRInputDeviceModule.h";

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PrivateIncludePathModuleNames.AddRange(new string[]
         {
            "TargetPlatform",
            "SteamVRInput"
        });

        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "InputCore",
				"InputDevice",
                "Json",
                "JsonUtilities"
			}
			);

        PublicDependencyModuleNames.AddRange(
    new string[]
    {
                "OpenVRSDK",
                "SteamVRInput"
    }
    );

        if (UEBuildConfiguration.bBuildEditor == true)
        {
            DynamicallyLoadedModuleNames.Add("UnrealEd");
        }

        //AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenVR");

        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Win32 || (Target.Platform == UnrealTargetPlatform.Linux && Target.Architecture.StartsWith("x86_64")))
        {
            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenGL");
            PrivateDependencyModuleNames.Add("OpenGLDrv");
        }
    }
}