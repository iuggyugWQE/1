using UnrealBuildTool;
using System.Collections.Generic;

public class ExampleOSSTarget : TargetRules
{
    public ExampleOSSTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange(new string[] { "ExampleOSS", "ExampleOSSEarlyConfig" });
    }
}
