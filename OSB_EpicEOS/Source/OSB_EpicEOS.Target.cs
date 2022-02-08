using UnrealBuildTool;
using System.Collections.Generic;

public class OSB_EpicEOSTarget : TargetRules
{
	public OSB_EpicEOSTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "OSB_EpicEOS" } );
	}
}
