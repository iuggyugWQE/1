// Copyright June Rhodes. MIT Licensed.

using UnrealBuildTool;
using System.Collections.Generic;

public class ClangTidyExampleTarget : TargetRules
{
	public ClangTidyExampleTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "ClangTidyExample" } );
	}
}