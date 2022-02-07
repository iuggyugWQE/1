// Copyright June Rhodes. MIT Licensed.

using UnrealBuildTool;
using System.Collections.Generic;

public class ClangTidyExampleEditorTarget : TargetRules
{
	public ClangTidyExampleEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "ClangTidyExample" } );

		bUsePCHFiles = false;
		DisableUnityBuildForModules = new string[]
		{
			"ClangTidyExample"
		};
	}
}