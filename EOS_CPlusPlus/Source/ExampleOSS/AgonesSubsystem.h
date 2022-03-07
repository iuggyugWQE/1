// Copyright June Rhodes. MIT Licensed.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"

#include "AgonesSubsystem.generated.h"

#if defined(UE_5_0_OR_LATER)
typedef FTSTicker FUTicker;
typedef FTSTicker::FDelegateHandle FUTickerDelegateHandle;
#else
typedef FTicker FUTicker;
typedef FDelegateHandle FUTickerDelegateHandle;
#endif // #if defined(UE_5_0_OR_LATER)

DECLARE_DYNAMIC_DELEGATE_ThreeParams(
    FAgonesPortsCallback,
    bool,
    HasPortOverride,
    int32,
    OutGamePort,
    int32,
    OutBeaconPort);

UCLASS(BlueprintType)
class EXAMPLEOSS_API UAgonesSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    bool bHasSentReady = false;
    FString AgonesHttpPort;
    FUTickerDelegateHandle HealthCheckTimer;

private:
    void HandleHealthCheck(FHttpRequestPtr HttpRequest, const FHttpResponsePtr HttpResponse, const bool bSucceeded);
    void HandleReady(FHttpRequestPtr HttpRequest, const FHttpResponsePtr HttpResponse, const bool bSucceeded);
    void HandleGameServer(
        FHttpRequestPtr HttpRequest,
        const FHttpResponsePtr HttpResponse,
        const bool bSucceeded,
        FAgonesPortsCallback Callback);

public:
    virtual void Initialize(FSubsystemCollectionBase &Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable)
    virtual void StartGetPortsFromAgones(FAgonesPortsCallback Callback);
    virtual bool RetryGetGamePorts(float DeltaSeconds, FAgonesPortsCallback Callback);

    virtual bool SendHealthCheck(float DeltaSeconds);
    virtual void SendReady();
};