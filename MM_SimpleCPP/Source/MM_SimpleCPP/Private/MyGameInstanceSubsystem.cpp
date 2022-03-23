// Copyright June Rhodes. MIT Licensed.

#include "MyGameInstanceSubsystem.h"

#include "./OnlineLobbyInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemUtils.h"

void UMyGameInstanceSubsystem::OnHandleProgress(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    FMatchmakingEngineProgressInfo ProgressInfo)
{
    this->OnProgress.Broadcast(
        ProgressInfo.StepName,
        ProgressInfo.CurrentStatus,
        ProgressInfo.CurrentDetail,
        ProgressInfo.CurrentProgress,
        ProgressInfo.EstimatedTimeOfCompletion);
}

void UMyGameInstanceSubsystem::OnHandleComplete(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    FMatchmakingEngineResponse Response)
{
    TArray<FString> Results;
    for (int32 t = 0; t < Response.Teams.Num(); t++)
    {
        if (Results.Num() != 0)
        {
            Results.Add(TEXT(""));
        }
        Results.Add(FString::Printf(TEXT("== Team %d =="), t));
        for (int32 s = 0; s < Response.Teams[t].Slots.Num(); s++)
        {
            switch (Response.Teams[t].Slots[s].Type)
            {
            case EMatchmakingEngineResponseTeamSlotType::Empty:
                Results.Add(FString::Printf(TEXT("Slot %d: (none)"), s));
                break;
            case EMatchmakingEngineResponseTeamSlotType::AI:
                Results.Add(FString::Printf(TEXT("Slot %d: (AI)"), s));
                break;
            case EMatchmakingEngineResponseTeamSlotType::User:
                Results.Add(FString::Printf(TEXT("Slot %d: %s"), s, *Response.Teams[t].Slots[s].UserId->ToString()));
                break;
            }
        }
    }

    this->OnComplete.Broadcast(FString::Join(Results, TEXT("\n")));
}

void UMyGameInstanceSubsystem::OnHandleResultsReady(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    FMatchmakingEngineResponse Response)
{
    // We don't actually use this event in the example, but this is where you would
    // store the matchmaking results if you wanted to use team information in
    // e.g. "ChoosePlayerStart". The OnComplete event doesn't fire until *after*
    // the listen server has spawned the first player controller, so this lets
    // you get the results earlier.
}

void UMyGameInstanceSubsystem::OnHandleCancelled(const FString &TaskId, const struct FMatchmakingEngineRequest &Request)
{
    this->OnCancelled.Broadcast();
}

void UMyGameInstanceSubsystem::OnHandleError(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    const FOnlineError &Error)
{
    this->OnError.Broadcast();
}

void UMyGameInstanceSubsystem::StartMatchmaking()
{
    IOnlineSubsystem *OSS = Online::GetSubsystem(this->GetWorld());

    FMatchmakingEngineRequest Request = {};
    Request.Identity = OSS->GetIdentityInterface();
    Request.Lobby = Online::GetLobbyInterface(OSS);
    Request.PartySystem = nullptr;
    Request.Session = OSS->GetSessionInterface();
    Request.WorldContextHandle = GEngine->GetWorldContextFromWorldChecked(this->GetWorld()).ContextHandle;
    Request.UserId = OSS->GetIdentityInterface()->GetUniquePlayerId(0).ToSharedRef();
    Request.PartyId = nullptr;

#if WITH_EDITOR
    // Make request ID extra unique in editor.
    int64 RequestId = (FDateTime::UtcNow().GetTicks() / 100000) * 100000;
    FWorldContext &WorldContext = GEngine->GetWorldContextFromWorldChecked(this->GetWorld());
    RequestId += ((int64)WorldContext.PIEInstance % 100) * 1000;
    RequestId += FMath::RandRange(0, 999);
#else
    int64 RequestId = FDateTime::UtcNow().GetTicks();
#endif

    auto RequestHostConfiguration = MakeShared<FMatchmakingEngineRequestHostConfiguration>();
    RequestHostConfiguration->RequestId = RequestId;

    // ======== Main settings you care about =========
    RequestHostConfiguration->QueueName = TEXT("Default");
    RequestHostConfiguration->TeamCapacities.Add(1);
    RequestHostConfiguration->TeamCapacities.Add(1);
    Request.CompletionBehaviour.Type = EMatchmakingEngineCompletionBehaviourType::StartListenServerWithMap;
    Request.CompletionBehaviour.bConnectViaSessionID = true; // Set to false for Steam.
    Request.CompletionBehaviour.MapName = FName(TEXT("/Game/Matchmaking/MultiplayerMap"));
    // ======== Timing and balance settings not available in blueprints =========
    RequestHostConfiguration->OnNoCandidates = EMatchmakingBehaviourOnNoCandidates::CompletePartiallyFilled;
    RequestHostConfiguration->MinimumWaitSecondsBaseline = 60;
    RequestHostConfiguration->MinimumWaitSecondsPerEmptySlot = 5;
    RequestHostConfiguration->BalanceMode = EMatchmakingBalanceMode::MaximizeTeamFill;
    // =============================================

    Request.HostConfiguration = RequestHostConfiguration;
    Request.CompletionBehaviour.OnResultsReady =
        FMatchmakingEngineRequestComplete::CreateUObject(this, &UMyGameInstanceSubsystem::OnHandleResultsReady);
    Request.CompletionBehaviour.OnComplete =
        FMatchmakingEngineRequestComplete::CreateUObject(this, &UMyGameInstanceSubsystem::OnHandleComplete);
    Request.OnProgress =
        FMatchmakingEngineRequestProgress::CreateUObject(this, &UMyGameInstanceSubsystem::OnHandleProgress);
    Request.OnCancelled =
        FMatchmakingEngineRequestCancelled::CreateUObject(this, &UMyGameInstanceSubsystem::OnHandleCancelled);
    Request.OnError = FMatchmakingEngineRequestError::CreateUObject(this, &UMyGameInstanceSubsystem::OnHandleError);

    this->Handle = IMatchmakingEngine::Get()->Enqueue(Request);
}

void UMyGameInstanceSubsystem::CancelMatchmaking()
{
    IMatchmakingEngine::Get()->Cancel(this->Handle);
}