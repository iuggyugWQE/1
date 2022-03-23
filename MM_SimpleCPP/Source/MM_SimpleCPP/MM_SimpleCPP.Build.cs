// Copyright June Rhodes. MIT Licensed.

using UnrealBuildTool;

public class MM_SimpleCPP : ModuleRules
{
    public MM_SimpleCPP(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

        PrivateDependencyModuleNames.AddRange(new string[] { "Matchmaking", "OnlineSubsystem", "OnlineSubsystemUtils" });
    }
}
