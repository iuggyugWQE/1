// Copyright June Rhodes. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Diagnostics;

public class RedpointEOSAuthSteam : ModuleRules
{
    public RedpointEOSAuthSteam(ReadOnlyTargetRules Target) : base(Target)
    {
        DefaultBuildSettings = BuildSettingsVersion.V2;
        bUsePrecompiled = true;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "OnlineSubsystem",
                "OnlineSubsystemRedpointEOS",
                "OnlineSubsystemUtils",
                "RedpointEOSSDK",
            }
        );

        if (Target.Type != TargetType.Server)
        {
            PrivateDependencyModuleNames.AddRange(
               new string[]
               {
                    "UMG",
               }
            );
        }

    }
}
