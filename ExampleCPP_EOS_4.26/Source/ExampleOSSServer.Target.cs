using UnrealBuildTool;
using System.Collections.Generic;

public class ExampleOSSServerTarget : TargetRules
{
    public ExampleOSSServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange(new string[] { "ExampleOSS", "ExampleOSSEarlyConfig" });
    }
}
