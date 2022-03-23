// Copyright June Rhodes. MIT Licensed.

#pragma once

#include "CoreMinimal.h"
#include "MatchmakingEngine.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "MyGameInstanceSubsystem.generated.h"

// We just make blueprint visible events so we can pass the
// information up to our UMG widgets, but you don't have to (e.g.
// if you're using Slate).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
    FOnGameMatchmakingProgress,
    FName,
    StepName,
    FText,
    Status,
    FText,
    Detail,
    float,
    Progress,
    FDateTime,
    ETA);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameMatchmakingComplete, FString, MatchResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameMatchmakingCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameMatchmakingError);

UCLASS()
class UMyGameInstanceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

private:
    FMatchmakingEngineRequestHandle Handle;

    void OnHandleProgress(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        FMatchmakingEngineProgressInfo ProgressInfo);
    void OnHandleResultsReady(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        FMatchmakingEngineResponse Response);
    void OnHandleComplete(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        FMatchmakingEngineResponse Response);
    void OnHandleCancelled(const FString &TaskId, const struct FMatchmakingEngineRequest &Request);
    void OnHandleError(
        const FString &TaskId,
        const struct FMatchmakingEngineRequest &Request,
        const FOnlineError &Error);

public:
    UPROPERTY(BlueprintAssignable)
    FOnGameMatchmakingProgress OnProgress;

    UPROPERTY(BlueprintAssignable)
    FOnGameMatchmakingComplete OnComplete;

    UPROPERTY(BlueprintAssignable)
    FOnGameMatchmakingCancelled OnCancelled;

    UPROPERTY(BlueprintAssignable)
    FOnGameMatchmakingError OnError;

    UFUNCTION(BlueprintCallable)
    void StartMatchmaking();

    UFUNCTION(BlueprintCallable)
    void CancelMatchmaking();
};
