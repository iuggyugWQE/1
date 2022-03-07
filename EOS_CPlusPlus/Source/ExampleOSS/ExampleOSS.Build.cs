// Copyright June Rhodes. MIT Licensed.

using UnrealBuildTool;

public class ExampleOSS : ModuleRules
{
    public ExampleOSS(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

#if UE_5_0_OR_LATER
        PublicDefinitions.Add("UE_5_0_OR_LATER=1");
#endif

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystemUtils", "DeveloperSettings", "VoiceChat" });

        PrivateDependencyModuleNames.AddRange(new string[] { 
            "OnlineSubsystem", 
            "ExampleOSSEarlyConfig",
#if UE_5_0_OR_LATER
            "HTTP",
#else
            "Http",
#endif
            "Json"
        });
    }
}