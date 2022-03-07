// Copyright June Rhodes. MIT Licensed.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"

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

#if defined(UE_5_0_OR_LATER)
typedef FTSTicker FUTicker;
typedef FTSTicker::FDelegateHandle FUTickerDelegateHandle;
#else
typedef FTicker FUTicker;
typedef FDelegateHandle FUTickerDelegateHandle;
#endif // #if defined(UE_5_0_OR_LATER)