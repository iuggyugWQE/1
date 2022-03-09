// Copyright June Rhodes. MIT Licensed.

using UnrealBuildTool;

public class ExampleOSSDeveloper : ModuleRules
{
    public ExampleOSSDeveloper(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

#if UE_5_0_OR_LATER
        PublicDefinitions.Add("UE_5_0_OR_LATER=1");
#endif

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystemUtils" });

        PrivateDependencyModuleNames.AddRange(new string[] { 
            "OnlineSubsystem", 
            "ExampleOSSEarlyConfig",
#if UE_5_0_OR_LATER
            "HTTP",
#else
            "Http",
#endif
            "Json", 
            "Gauntlet",
            "AutomationController", 
            "AssetRegistry", 
            "OnlineSubsystemRedpointEOS" 
        });
    }
}