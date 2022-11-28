// Copyright June Rhodes. MIT Licensed.

using UnrealBuildTool;

public class ExampleOSSEditor : ModuleRules
{
    public ExampleOSSEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

#if UE_5_1_OR_LATER
        PublicDefinitions.Add("UE_5_1_OR_LATER=1");
#endif
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
            "AssetRegistry", 
            "OnlineSubsystemRedpointEOS",
            "BlueprintGraph",
            "UnrealEd",
            "SourceControl",
        });
    }
}