// Copyright June Rhodes. MIT Licensed.

using UnrealBuildTool;
using System.Collections.Generic;

public class ExampleOSSEditorTarget : TargetRules
{
    public ExampleOSSEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange(new string[] { "ExampleOSS", "ExampleOSSDeveloper", "ExampleOSSEditor", "ExampleOSSEarlyConfig" });
#if UE_5_1_OR_LATER
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
#endif

        ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_STEAM=1");
        ProjectDefinitions.Add("ONLINE_SUBSYSTEM_EOS_ENABLE_DISCORD=1");
    }
}