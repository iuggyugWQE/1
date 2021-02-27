// Copyright Redpoint Games 2018, All Rights Reserved.

using UnrealBuildTool;

public class DemoCPPEarlyConfig : ModuleRules
{
    public DemoCPPEarlyConfig(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
        });
    }
}
