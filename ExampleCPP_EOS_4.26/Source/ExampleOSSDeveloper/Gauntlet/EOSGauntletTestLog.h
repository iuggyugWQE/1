// Copyright 2020 June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEOSGauntlet, All, All);

// Workaround Verbose log level not being emitted from stdout on macOS.
#if PLATFORM_MAC
#define GauntletLogLevel Log
#else
#define GauntletLogLevel Verbose
#endif

void EmitTestStart(FString TestName);
void EmitTestFail(FString TestName);
void EmitTestPass(FString TestName);