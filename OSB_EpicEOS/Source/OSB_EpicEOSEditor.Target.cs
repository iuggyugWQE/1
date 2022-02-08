using UnrealBuildTool;
using System.Collections.Generic;

public class OSB_EpicEOSEditorTarget : TargetRules
{
	public OSB_EpicEOSEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "OSB_EpicEOS" } );
	}
}
