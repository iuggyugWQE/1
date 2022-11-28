// Copyright June Rhodes. MIT Licensed.

using UnrealBuildTool;
using System.Collections.Generic;

public class ExampleOSSTarget : TargetRules
{
    public ExampleOSSTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange(new string[] { "ExampleOSS", "ExampleOSSEarlyConfig" });
#if UE_5_1_OR_LATER
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
#endif

        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            ExtraModuleNames.AddRange(new string[] { "ExampleOSSDeveloper" });
        }

        ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_STEAM=1");
        ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_DISCORD=1");
    }
}