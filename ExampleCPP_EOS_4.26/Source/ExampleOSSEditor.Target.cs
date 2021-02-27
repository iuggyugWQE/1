using UnrealBuildTool;
using System.Collections.Generic;

public class ExampleOSSEditorTarget : TargetRules
{
    public ExampleOSSEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.AddRange(new string[] { "ExampleOSS" });
    }
}
