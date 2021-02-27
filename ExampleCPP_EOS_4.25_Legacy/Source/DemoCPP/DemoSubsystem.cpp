// Copyright 2020 June Rhodes. All Rights Reserved.

#include "DemoSubsystem.h"

#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UDemoSubsystem::PostInitProperties()
{
    Super::PostInitProperties();

    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        return;
    }

    check(this->GetWorld() != nullptr);

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    if (Subsystem != nullptr && Subsystem->GetSubsystemName() == FName(TEXT("EOS")))
    {
        IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
        IOnlinePartyPtr Party = Subsystem->GetPartyInterface();
        IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
        Party->AddOnPartyInvitesChangedDelegate_Handle(
            FOnPartyInvitesChangedDelegate::CreateUObject(this, &UDemoSubsystem::OnPartyInvitesChanged));
        Party->AddOnPartyDataReceivedDelegate_Handle(
            FOnPartyDataReceivedConstDelegate::CreateUObject(this, &UDemoSubsystem::OnPartyDataReceived));
        Session->AddOnSessionUserInviteAcceptedDelegate_Handle(
            FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UDemoSubsystem::OnSessionUserInviteAccepted));
        Session->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionCompleteDelegate::CreateUObject(this, &UDemoSubsystem::OnSessionJoinedViaOverlay));
    }
}

void UDemoSubsystem::OnPartyDataReceived(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FOnlinePartyData &PartyData)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr PartySystem = Subsystem->GetPartyInterface();

    auto Party = PartySystem->GetParty(LocalUserId, PartyId);
    if (!Party.IsValid() || *Party->LeaderId == LocalUserId)
    {
        // Ignore this request because we're either not in the party (?) or
        // we're the leader.
        return;
    }

    FVariantData Data;
    if (PartyData.GetAttribute(TEXT("SessionFollowString"), Data))
    {
        if (Data.GetType() == EOnlineKeyValuePairDataType::String)
        {
            // This party has a SessionFollowString, parse it.
            FString SessionFollowString;
            Data.GetValue(SessionFollowString);
            TArray<FString> SessionFollowComponents;
            SessionFollowString.ParseIntoArray(SessionFollowComponents, TEXT("|"));
            FString RequestId = SessionFollowComponents[0];
            FString SessionId = SessionFollowComponents[1];

            if (this->SessionFollowConsumedIds.Contains(RequestId))
            {
                // We have already acted upon this before.
                return;
            }

            this->SessionFollowConsumedIds.Add(RequestId);

            // Try to find the session.
            Session->FindSessionById(
                *Identity->GetUniquePlayerId(0),
                *Session->CreateSessionIdFromString(SessionId),
                *Identity->GetUniquePlayerId(0),
                FOnSingleSessionResultCompleteDelegate::CreateUObject(
                    this,
                    &UDemoSubsystem::OnFindSessionForLeaderFollow,
                    SessionFollowString));
        }
    }
}

void UDemoSubsystem::OnFindSessionForLeaderFollow(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FOnlineSessionSearchResult &SearchResult,
    FString SessionFollowString)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    if (bWasSuccessful)
    {
        // Join the session.
        this->JoinSessionForLeaderSessionName = FName(SessionFollowString);
        this->JoinSessionForLeaderFollowDelegateHandle = Session->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionComplete::FDelegate::CreateUObject(this, &UDemoSubsystem::OnJoinSessionForLeaderFollow));
        if (!Session->JoinSession(0, FName(SessionFollowString), SearchResult))
        {
            UE_LOG(LogTemp, Error, TEXT("Could not call JoinSession after following party leader!"));
        }
    }
}

void UDemoSubsystem::OnJoinSessionForLeaderFollow(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    if (JoinResult == EOnJoinSessionCompleteResult::Success && SessionName.IsEqual(JoinSessionForLeaderSessionName))
    {
        this->PartyLeaderWasFollowedToSession.Broadcast(SessionName);

        Session->ClearOnJoinSessionCompleteDelegate_Handle(this->JoinSessionForLeaderFollowDelegateHandle);
    }
}

void UDemoSubsystem::OnSessionUserInviteAccepted(
    const bool bWasSuccessful,
    const int32 LocalUserNum,
    TSharedPtr<const FUniqueNetId> UserId,
    const FOnlineSessionSearchResult &InviteResult)
{
    check(bWasSuccessful); // Should always be true in EOS subsystem.

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    this->JoinSessionForInviteSessionName = FName(*InviteResult.GetSessionIdStr());
    this->JoinSessionForInviteDelegateHandle = Session->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionComplete::FDelegate::CreateUObject(this, &UDemoSubsystem::OnJoinSessionForInviteFollow));
    if (!Session->JoinSession(0, FName(*InviteResult.GetSessionIdStr()), InviteResult))
    {
        UE_LOG(LogTemp, Error, TEXT("Could not call JoinSession after accepting invite!"));
    }
}

void UDemoSubsystem::OnJoinSessionForInviteFollow(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    if (JoinResult == EOnJoinSessionCompleteResult::Success && SessionName.IsEqual(JoinSessionForInviteSessionName))
    {
        this->SessionJoinedMapTravelRequired.Broadcast(SessionName);

        Session->ClearOnJoinSessionCompleteDelegate_Handle(this->JoinSessionForInviteDelegateHandle);
    }
}

void UDemoSubsystem::OnSessionJoinedViaOverlay(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
    if (JoinResult == EOnJoinSessionCompleteResult::Success &&
        SessionName.ToString().StartsWith(TEXT("JoinViaOverlay_")))
    {
        this->SessionJoinedMapTravelRequired.Broadcast(SessionName);
    }
}

void UDemoSubsystem::OnPartyInvitesChanged(const FUniqueNetId &LocalUserId)
{
    this->OnInvitationsChanged.Broadcast();
}

bool UDemoSubsystem::IsSignedIn(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return false;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

    return Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn;
}

FString UDemoSubsystem::GetLoggedInUsername(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TEXT("");
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

    if (Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        TSharedPtr<const FUniqueNetId> Id = Identity->GetUniquePlayerId(0);
        if (Id.IsValid())
        {
            TSharedPtr<FUserOnlineAccount> Account = Identity->GetUserAccount(*Id);
            if (Account.IsValid())
            {
                if (Account->GetDisplayName().IsEmpty())
                {
                    return TEXT("(No Username Set, but you are Logged In)");
                }
                else
                {
                    return Account->GetDisplayName();
                }
            }
        }
    }

    return TEXT("Not Logged In");
}

FString UDemoSubsystem::GetLoggedInProductUserId(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TEXT("");
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

    if (Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        TSharedPtr<const FUniqueNetId> Id = Identity->GetUniquePlayerId(0);
        if (Id.IsValid())
        {
            TSharedPtr<FUserOnlineAccount> Account = Identity->GetUserAccount(*Id);
            if (Account.IsValid())
            {
                FString OutAttr;
                if (Account->GetUserAttribute(TEXT("productUserId"), OutAttr))
                {
                    return OutAttr;
                }
            }
        }
    }

    return TEXT("");
}

FString UDemoSubsystem::GetLoggedInEpicAccountId(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TEXT("");
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

    if (Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        TSharedPtr<const FUniqueNetId> Id = Identity->GetUniquePlayerId(0);
        if (Id.IsValid())
        {
            TSharedPtr<FUserOnlineAccount> Account = Identity->GetUserAccount(*Id);
            if (Account.IsValid())
            {
                FString OutAttr;
                if (Account->GetUserAttribute(TEXT("epicAccountId"), OutAttr))
                {
                    return OutAttr;
                }
            }
        }
    }

    return TEXT("");
}

bool UDemoSubsystem::CanLinkToEpicGamesAccount(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return false;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

    if (Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        TSharedPtr<const FUniqueNetId> Id = Identity->GetUniquePlayerId(0);
        if (Id.IsValid())
        {
            TSharedPtr<FUserOnlineAccount> Account = Identity->GetUserAccount(*Id);
            if (Account.IsValid())
            {
                FString OutVal;
                if (Account->GetAuthAttribute(TEXT("CanLinkToEpicGamesAccount"), OutVal))
                {
                    return OutVal == TEXT("true");
                }
            }
        }
    }
    return false;
}

void UDemoSubsystem::StartLogin(const UObject *WorldContextObject, int32 LocalUserNum, FDemoBPLLoginComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TEXT("Call didn't start"));
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

    // note: This example code only supports running one Login process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->LoginDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false, TEXT("Call didn't start"));
        return;
    }
    this->LoginDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(
        LocalUserNum,
        FOnLoginComplete::FDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleLoginComplete,
            WorldContextObject,
            OnDone));
    if (!Identity->AutoLogin(LocalUserNum))
    {
        OnDone.ExecuteIfBound(false, TEXT("Call didn't start"));
    }
}

void UDemoSubsystem::HandleLoginComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &UserId,
    const FString &Error,
    const UObject *WorldContextObject,
    FDemoBPLLoginComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful, Error);

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    if (Identity == nullptr)
    {
        return;
    }

    Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, this->LoginDelegateHandle);
    this->LoginDelegateHandle.Reset();
}

void UDemoSubsystem::StartLogout(const UObject *WorldContextObject, FDemoBPLLogoutComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

    // note: This example code only supports running one Logout process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->LogoutDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    this->LogoutDelegateHandle = Identity->AddOnLogoutCompleteDelegate_Handle(
        0,
        FOnLogoutComplete::FDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleLogoutComplete,
            WorldContextObject,
            OnDone));
    if (!Identity->Logout(0))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UDemoSubsystem::HandleLogoutComplete(
    int32 _UnusedLocalUserNum,
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FDemoBPLLogoutComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    if (Identity == nullptr)
    {
        return;
    }

    Identity->ClearOnLogoutCompleteDelegate_Handle(0, this->LogoutDelegateHandle);
    this->LogoutDelegateHandle.Reset();
}

void UDemoSubsystem::StartCreateSession(const UObject *WorldContextObject, FDemoSubsystemCreateSessionComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one CreateSession process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->CreateSessionDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    TSharedRef<FOnlineSessionSettings> SessionSettings = MakeShared<FOnlineSessionSettings>();
    SessionSettings->NumPublicConnections = 4;
    SessionSettings->bShouldAdvertise = true;
    SessionSettings->bUsesPresence = true;
    SessionSettings->Settings.Add(
        FName(TEXT("SessionSetting")),
        FOnlineSessionSetting(FString(TEXT("SettingValue")), EOnlineDataAdvertisementType::ViaOnlineService));

    this->CreateSessionDelegateHandle =
        Session->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionComplete::FDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleCreateSessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->CreateSession(0, FName(TEXT("MyLocalSessionName")), *SessionSettings))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UDemoSubsystem::HandleCreateSessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FDemoSubsystemCreateSessionComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return;
    }

    Session->ClearOnCreateSessionCompleteDelegate_Handle(this->CreateSessionDelegateHandle);
    this->CreateSessionDelegateHandle.Reset();
}

void UDemoSubsystem::StartFindSessions(const UObject *WorldContextObject, FDemoSubsystemFindSessionsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<UDemoSessionSearchResult *>());
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one FindSessions process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->FindSessionsDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false, TArray<UDemoSessionSearchResult *>());
        return;
    }

    TSharedRef<FOnlineSessionSearch> Search = MakeShared<FOnlineSessionSearch>();
    // Remove the default search parameters that FOnlineSessionSearch sets up.
    Search->QuerySettings.SearchParams.Empty();
    Search->QuerySettings.Set(
        FName(TEXT("SessionSetting")),
        FString(TEXT("SettingValue")),
        EOnlineComparisonOp::Equals);

    this->FindSessionsDelegateHandle =
        Session->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsComplete::FDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleFindSessionsComplete,
            WorldContextObject,
            OnDone,
            Search));
    if (!Session->FindSessions(0, Search))
    {
        OnDone.ExecuteIfBound(false, TArray<UDemoSessionSearchResult *>());
    }
}

void UDemoSubsystem::HandleFindSessionsComplete(
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FDemoSubsystemFindSessionsComplete OnDone,
    TSharedRef<FOnlineSessionSearch> Search)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    check(Session != nullptr);

    TArray<UDemoSessionSearchResult *> Results;
    if (bWasSuccessful)
    {
        for (auto RawResult : Search->SearchResults)
        {
            if (RawResult.IsValid())
            {
                auto Result = NewObject<UDemoSessionSearchResult>(this);
                Result->Result = RawResult;
                Result->SessionId = RawResult.GetSessionIdStr();
                Session->GetResolvedConnectString(RawResult, FName(TEXT("")), Result->ConnectionString);
                Results.Add(Result);
            }
        }
    }

    OnDone.ExecuteIfBound(bWasSuccessful, Results);

    Session->ClearOnFindSessionsCompleteDelegate_Handle(this->FindSessionsDelegateHandle);
    this->FindSessionsDelegateHandle.Reset();
}

FString UDemoSubsystem::GetCurrentSessionId(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TEXT("");
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return TEXT("");
    }

    FNamedOnlineSession *NamedSession = Session->GetNamedSession(FName(TEXT("MyLocalSessionName")));
    if (NamedSession == nullptr || !NamedSession->SessionInfo.IsValid() || !NamedSession->SessionInfo->IsValid())
    {
        return TEXT("");
    }

    return NamedSession->GetSessionIdStr();
}

TArray<FString> UDemoSubsystem::GetPlayersInSession(const UObject *WorldContextObject)
{
    TArray<FString> Results;

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return Results;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return Results;
    }

    FNamedOnlineSession *NamedSession = Session->GetNamedSession(FName(TEXT("MyLocalSessionName")));
    if (NamedSession == nullptr || !NamedSession->SessionInfo.IsValid() || !NamedSession->SessionInfo->IsValid())
    {
        return Results;
    }

    for (TSharedRef<const FUniqueNetId> PlayerId : NamedSession->RegisteredPlayers)
    {
        if (NamedSession->LocalOwnerId != nullptr && *PlayerId == *NamedSession->LocalOwnerId)
        {
            Results.Add(FString::Printf(TEXT("(host): %s"), *PlayerId->ToString()));
        }
        else
        {
            Results.Add(FString::Printf(TEXT("(client): %s"), *PlayerId->ToString()));
        }
    }

    return Results;
}

void UDemoSubsystem::RegisterPlayer(APlayerController *InPlayerController)
{
    check(IsValid(InPlayerController));

    FUniqueNetIdRepl UniqueNetIdRepl;
    if (InPlayerController->IsLocalPlayerController())
    {
        ULocalPlayer *LocalPlayer = InPlayerController->GetLocalPlayer();
        if (IsValid(LocalPlayer))
        {
            UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
        }
        else
        {
            UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
            check(IsValid(RemoteNetConnection));
            UniqueNetIdRepl = RemoteNetConnection->PlayerId;
        }
    }
    else
    {
        UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
        check(IsValid(RemoteNetConnection));
        UniqueNetIdRepl = RemoteNetConnection->PlayerId;
    }

    TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
    if (!UniqueNetId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("No unique net ID assocated with connection, can not register player"));
        return;
    }

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    check(Session != nullptr);

    verify(Session->RegisterPlayer(FName(TEXT("MyLocalSessionName")), *UniqueNetId, false));
}

void UDemoSubsystem::UnregisterPlayer(APlayerController *InPlayerController)
{
    check(IsValid(InPlayerController));

    FUniqueNetIdRepl UniqueNetIdRepl;
    if (InPlayerController->IsLocalPlayerController())
    {
        ULocalPlayer *LocalPlayer = InPlayerController->GetLocalPlayer();
        if (IsValid(LocalPlayer))
        {
            UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
        }
        else
        {
            UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
            check(IsValid(RemoteNetConnection));
            UniqueNetIdRepl = RemoteNetConnection->PlayerId;
        }
    }
    else
    {
        UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
        check(IsValid(RemoteNetConnection));
        UniqueNetIdRepl = RemoteNetConnection->PlayerId;
    }

    TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
    if (!UniqueNetId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("No unique net ID assocated with connection, can not unregister player"));
        return;
    }

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    check(Session != nullptr);

    verify(Session->UnregisterPlayer(FName(TEXT("MyLocalSessionName")), *UniqueNetId));
}

void UDemoSubsystem::StartDestroySession(
    const UObject *WorldContextObject,
    FName SessionName,
    FDemoSubsystemDestroySessionComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one DestroySession process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->DestroySessionDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    this->DestroySessionDelegateHandle =
        Session->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionComplete::FDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleDestroySessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->DestroySession(SessionName))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UDemoSubsystem::HandleDestroySessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FDemoSubsystemDestroySessionComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return;
    }

    Session->ClearOnDestroySessionCompleteDelegate_Handle(this->DestroySessionDelegateHandle);
    this->DestroySessionDelegateHandle.Reset();
}

void UDemoSubsystem::StartJoinSession(
    const UObject *WorldContextObject,
    UDemoSessionSearchResult *SearchResult,
    FDemoSubsystemJoinSessionComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one JoinSession process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->JoinSessionDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    check(IsValid(SearchResult));

    this->JoinSessionDelegateHandle =
        Session->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionComplete::FDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleJoinSessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->JoinSession(0, FName(TEXT("MyLocalSessionName")), SearchResult->Result))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UDemoSubsystem::HandleJoinSessionComplete(
    FName SessionName,
    EOnJoinSessionCompleteResult::Type JoinResult,
    const UObject *WorldContextObject,
    FDemoSubsystemJoinSessionComplete OnDone)
{
    if (!SessionName.IsEqual(FName(TEXT("MyLocalSessionName"))))
    {
        // Since we can conflict with the party leader following stuff... ugh the sessions API...
        return;
    }

    OnDone.ExecuteIfBound(
        JoinResult == EOnJoinSessionCompleteResult::Success ||
        JoinResult == EOnJoinSessionCompleteResult::AlreadyInSession);

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    if (Session == nullptr)
    {
        return;
    }

    Session->ClearOnJoinSessionCompleteDelegate_Handle(this->JoinSessionDelegateHandle);
    this->JoinSessionDelegateHandle.Reset();
}

FString UDemoSubsystem::GetResolvedConnectString(const UObject *WorldContextObject)
{
    return GetResolvedConnectStringForSession(WorldContextObject, FName(TEXT("MyLocalSessionName")));
}

FString UDemoSubsystem::GetResolvedConnectStringForSession(const UObject *WorldContextObject, FName SessionName)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TEXT("");
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    FString ConnectInfo;
    if (Session->GetResolvedConnectString(SessionName, ConnectInfo))
    {
        return ConnectInfo;
    }

    return TEXT("");
}

void UDemoSubsystem::StartUpdatePresence(
    const UObject *WorldContextObject,
    const FString &NewPresenceString,
    FDemoSubsystemUpdatePresenceComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePresencePtr Presence = Subsystem->GetPresenceInterface();

    FOnlineUserPresenceStatus Status;
    Status.State = EOnlinePresenceState::Online;
    Status.StatusStr = NewPresenceString;

    Presence->SetPresence(
        *Identity->GetUniquePlayerId(0).Get(),
        Status,
        IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleUpdatePresenceComplete,
            WorldContextObject,
            OnDone));
}

void UDemoSubsystem::HandleUpdatePresenceComplete(
    const class FUniqueNetId &UserId,
    const bool bWasSuccessful,
    const UObject *WorldContextObject,
    FDemoSubsystemUpdatePresenceComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);
}

TArray<UDemoPartyId *> UDemoSubsystem::GetJoinedParties(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TArray<UDemoPartyId *>();
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    TArray<TSharedRef<const FOnlinePartyId>> Result;
    if (!Party->GetJoinedParties(*Identity->GetUniquePlayerId(0).Get(), Result))
    {
        return TArray<UDemoPartyId *>();
    }

    TArray<UDemoPartyId *> ResultIds;
    for (auto Entry : Result)
    {
        UDemoPartyId *Id = NewObject<UDemoPartyId>(this);
        Id->PartyId = Entry;
        ResultIds.Add(Id);
    }
    return ResultIds;
}

void UDemoSubsystem::RequestPartyMembersFollowLeader(const UObject *WorldContextObject, FName SessionName)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr PartySystem = Subsystem->GetPartyInterface();
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    auto LocalUser = Identity->GetUniquePlayerId(0).Get();

    TArray<TSharedRef<const FOnlinePartyId>> Result;
    if (!PartySystem->GetJoinedParties(*LocalUser, Result))
    {
        return;
    }

    auto SessionData = Session->GetNamedSession(SessionName);
    FString SessionFollowString = FString::Printf(TEXT("%d|%s"), FMath::Rand(), *SessionData->GetSessionIdStr());

    for (auto PartyId : Result)
    {
        auto Party = PartySystem->GetParty(*LocalUser, *PartyId);
        if (Party.IsValid())
        {
            if (*Party->LeaderId == *LocalUser)
            {
                // We are the leader, so we can make other members follow.
                auto PartyDataUpdate = MakeShared<FOnlinePartyData>();
                PartyDataUpdate->SetAttribute(TEXT("SessionFollowString"), SessionFollowString);
                if (!PartySystem->UpdatePartyData(*LocalUser, *PartyId, *PartyDataUpdate))
                {
                    UE_LOG(
                        LogTemp,
                        Error,
                        TEXT("Could not request other members to follow because UpdatePartyData failed."));
                }
            }
        }
    }
}

void UDemoSubsystem::StartCreateParty(
    const UObject *WorldContextObject,
    int PartyTypeId,
    FDemoSubsystemCreatePartyComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    TSharedRef<FPartyConfiguration> Config = MakeShared<FPartyConfiguration>();
    Config->bIsAcceptingMembers = true;
    Config->MaxMembers = 4;

    if (!Party->CreateParty(
            *Identity->GetUniquePlayerId(0).Get(),
            (FOnlinePartyTypeId)PartyTypeId,
            *Config,
            FOnCreatePartyComplete::CreateUObject(
                this,
                &UDemoSubsystem::HandleCreatePartyComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UDemoSubsystem::HandleCreatePartyComplete(
    const FUniqueNetId &LocalUserId,
    const TSharedPtr<const FOnlinePartyId> &PartyId,
    const ECreatePartyCompletionResult Result,
    const UObject *WorldContextObject,
    FDemoSubsystemCreatePartyComplete OnDone)
{
    OnDone.ExecuteIfBound(Result == ECreatePartyCompletionResult::Succeeded);
}

TArray<UDemoPartyInvite *> UDemoSubsystem::GetInvitations(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TArray<UDemoPartyInvite *>();
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    TArray<IOnlinePartyJoinInfoConstRef> PendingInvites;
    if (!Party->GetPendingInvites(*Identity->GetUniquePlayerId(0).Get(), PendingInvites))
    {
        return TArray<UDemoPartyInvite *>();
    }

    TArray<UDemoPartyInvite *> Results;
    for (auto Entry : PendingInvites)
    {
        UDemoPartyInvite *Id = NewObject<UDemoPartyInvite>(this);
        Id->PartyInvite = Entry;
        Results.Add(Id);
    }
    return Results;
}

TArray<UDemoPartyMemberId *> UDemoSubsystem::GetPartyMembers(const UObject *WorldContextObject, UDemoPartyId *PartyId)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TArray<UDemoPartyMemberId *>();
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    TArray<FOnlinePartyMemberConstRef> PartyMembers;
    if (!Party->GetPartyMembers(*Identity->GetUniquePlayerId(0).Get(), *PartyId->PartyId.Get(), PartyMembers))
    {
        return TArray<UDemoPartyMemberId *>();
    }

    TArray<UDemoPartyMemberId *> Results;
    for (auto Entry : PartyMembers)
    {
        UDemoPartyMemberId *Id = NewObject<UDemoPartyMemberId>(this);
        Id->PartyMember = Entry;
        Results.Add(Id);
    }
    return Results;
}

bool UDemoSubsystem::IsPartyLeader(const UObject *WorldContextObject, UDemoPartyId *PartyId)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return false;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    return Party->IsMemberLeader(
        *Identity->GetUniquePlayerId(0).Get(),
        *PartyId->PartyId.Get(),
        *Identity->GetUniquePlayerId(0).Get());
}

void UDemoSubsystem::StartJoinParty(
    const UObject *WorldContextObject,
    UDemoPartyInvite *Invite,
    FDemoSubsystemJoinPartyComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    if (!Party->JoinParty(
            *Identity->GetUniquePlayerId(0).Get(),
            *Invite->PartyInvite,
            FOnJoinPartyComplete::CreateUObject(
                this,
                &UDemoSubsystem::HandleJoinPartyComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UDemoSubsystem::HandleJoinPartyComplete(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const EJoinPartyCompletionResult Result,
    const int32 NotApprovedReason,
    const UObject *WorldContextObject,
    FDemoSubsystemJoinPartyComplete OnDone)
{
    OnDone.ExecuteIfBound(Result == EJoinPartyCompletionResult::Succeeded);
}

void UDemoSubsystem::StartLeaveParty(
    const UObject *WorldContextObject,
    UDemoPartyId *PartyId,
    FDemoSubsystemLeavePartyComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    if (!Party->LeaveParty(
            *Identity->GetUniquePlayerId(0).Get(),
            *PartyId->PartyId,
            FOnLeavePartyComplete::CreateUObject(
                this,
                &UDemoSubsystem::HandleLeavePartyComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UDemoSubsystem::HandleLeavePartyComplete(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const ELeavePartyCompletionResult Result,
    const UObject *WorldContextObject,
    FDemoSubsystemLeavePartyComplete OnDone)
{
    OnDone.ExecuteIfBound(Result == ELeavePartyCompletionResult::Succeeded);
}

void UDemoSubsystem::StartKickMember(
    const UObject *WorldContextObject,
    UDemoPartyId *PartyId,
    UDemoPartyMemberId *MemberId,
    FDemoSubsystemKickMemberComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    if (!Party->KickMember(
            *Identity->GetUniquePlayerId(0).Get(),
            *PartyId->PartyId,
            *MemberId->PartyMember->GetUserId(),
            FOnKickPartyMemberComplete::CreateUObject(
                this,
                &UDemoSubsystem::HandleKickMemberComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UDemoSubsystem::HandleKickMemberComplete(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const EKickMemberCompletionResult Result,
    const UObject *WorldContextObject,
    FDemoSubsystemKickMemberComplete OnDone)
{
    OnDone.ExecuteIfBound(Result == EKickMemberCompletionResult::Succeeded);
}

TArray<UDemoFriend *> UDemoSubsystem::GetFriends(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TArray<UDemoFriend *>();
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    TArray<TSharedRef<FOnlineFriend>> FriendsRaw;
    if (!Friends->GetFriendsList(0, TEXT(""), FriendsRaw))
    {
        return TArray<UDemoFriend *>();
    }

    TArray<UDemoFriend *> Results;
    for (auto Friend : FriendsRaw)
    {
        UDemoFriend *FriendResult = NewObject<UDemoFriend>(this);
        FriendResult->Friend = Friend;
        Results.Add(FriendResult);
    }
    return Results;
}

void UDemoSubsystem::StartReadFriends(const UObject *WorldContextObject, FDemoSubsystemReadFriendsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    if (!Friends->ReadFriendsList(
            0,
            TEXT(""),
            FOnReadFriendsListComplete::CreateUObject(
                this,
                &UDemoSubsystem::HandleReadFriendsComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UDemoSubsystem::HandleReadFriendsComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FString &ListName,
    const FString &ErrorStr,
    const UObject *WorldContextObject,
    FDemoSubsystemReadFriendsComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);
}

void UDemoSubsystem::StartInviteFriend(
    const UObject *WorldContextObject,
    UDemoPartyId *PartyId,
    UDemoFriend *Friend,
    FDemoSubsystemInviteFriendComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    FPartyInvitationRecipient Recipient = FPartyInvitationRecipient(Friend->Friend->GetUserId());

    if (!Party->SendInvitation(
            *Identity->GetUniquePlayerId(0).Get(),
            *PartyId->PartyId,
            Recipient,
            FOnSendPartyInvitationComplete::CreateUObject(
                this,
                &UDemoSubsystem::HandleInviteFriendComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UDemoSubsystem::HandleInviteFriendComplete(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &RecipientId,
    const ESendPartyInvitationCompletionResult Result,
    const UObject *WorldContextObject,
    FDemoSubsystemInviteFriendComplete OnDone)
{
    OnDone.ExecuteIfBound(Result == ESendPartyInvitationCompletionResult::Succeeded);
}

void UDemoSubsystem::QueryUserInfo(
    const UObject *WorldContextObject,
    FString ProductUserIdInput,
    FString EpicAccountIdInput,
    FDemoSubsystemQueryUserInfoComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(TEXT("Subsystem not available."));
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineUserPtr User = Subsystem->GetUserInterface();

    TSharedPtr<const FUniqueNetId> UniqueNetId =
        Identity->CreateUniquePlayerId(FString::Printf(TEXT("%s|%s"), *ProductUserIdInput, *EpicAccountIdInput));
    if (UniqueNetId == nullptr)
    {
        OnDone.ExecuteIfBound(TEXT("The IDs you entered were invalid."));
        return;
    }

    // note: This example code only supports running one QueryUserInfo process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->QueryUserInfoDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(TEXT("Already a QueryUserInfo running."));
        return;
    }

    this->QueryUserInfoDelegateHandle = User->AddOnQueryUserInfoCompleteDelegate_Handle(
        0,
        FOnQueryUserInfoComplete::FDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleQueryUserInfoComplete,
            WorldContextObject,
            OnDone));

    TArray<TSharedRef<const FUniqueNetId>> UserIds;
    UserIds.Add(UniqueNetId.ToSharedRef());
    User->QueryUserInfo(0, UserIds);
}

void UDemoSubsystem::HandleQueryUserInfoComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const TArray<TSharedRef<const FUniqueNetId>> &UserIds,
    const FString &ErrorStr,
    const UObject *WorldContextObject,
    FDemoSubsystemQueryUserInfoComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(TEXT("Subsystem not available."));
        return;
    }
    IOnlineUserPtr User = Subsystem->GetUserInterface();
    if (User == nullptr)
    {
        OnDone.ExecuteIfBound(TEXT("Interface not available."));
        return;
    }

    if (!bWasSuccessful)
    {
        OnDone.ExecuteIfBound(FString::Printf(TEXT("Error: %s"), *ErrorStr));
    }
    else
    {
        if (UserIds.Num() == 0)
        {
            OnDone.ExecuteIfBound(TEXT("No such user was found."));
        }
        else if (UserIds.Num() == 1)
        {
            auto Id = UserIds[0];
            TSharedPtr<FOnlineUser> UserAcc = User->GetUserInfo(0, *Id);

            if (UserAcc.IsValid())
            {
                OnDone.ExecuteIfBound(FString::Printf(TEXT("%s -> %s"), *Id->ToString(), *UserAcc->GetDisplayName()));
            }
            else
            {
                OnDone.ExecuteIfBound(FString::Printf(TEXT("%s -> (bug!!)"), *Id->ToString()));
            }
        }
        else
        {
            OnDone.ExecuteIfBound(TEXT("Bug: Too many user IDs returned!"));
        }
    }

    User->ClearOnQueryUserInfoCompleteDelegate_Handle(0, this->QueryUserInfoDelegateHandle);
    this->QueryUserInfoDelegateHandle.Reset();
}

void UDemoSubsystem::QueryUserIdMapping(
    const UObject *WorldContextObject,
    FString DisplayNameInput,
    FDemoSubsystemQueryUserIdMappingComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(TEXT("Subsystem not available."));
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineUserPtr User = Subsystem->GetUserInterface();

    User->QueryUserIdMapping(
        *Identity->GetUniquePlayerId(0),
        DisplayNameInput,
        IOnlineUser::FOnQueryUserMappingComplete::CreateUObject(
            this,
            &UDemoSubsystem::HandleQueryUserMappingComplete,
            WorldContextObject,
            OnDone));
}

void UDemoSubsystem::HandleQueryUserMappingComplete(
    bool bWasSuccessful,
    const FUniqueNetId &UserId,
    const FString &DisplayNameOrEmail,
    const FUniqueNetId &FoundUserId,
    const FString &Error,
    const UObject *WorldContextObject,
    FDemoSubsystemQueryUserIdMappingComplete OnDone)
{
    if (!bWasSuccessful)
    {
        OnDone.ExecuteIfBound(FString::Printf(TEXT("Error: %s"), *Error));
        return;
    }

    OnDone.ExecuteIfBound(FString::Printf(TEXT("Found user: %s"), *FoundUserId.ToString()));
}

void UDemoSubsystem::QueryExternalIdMappings(
    const UObject *WorldContextObject,
    FString PlatformType,
    FString ExternalIdsStr,
    FDemoSubsystemQueryExternalIdMappingsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(TEXT("Subsystem not available."));
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineUserPtr User = Subsystem->GetUserInterface();

    TArray<FString> ExternalIdsTemp, ExternalIds;
    ExternalIdsStr.ParseIntoArray(ExternalIdsTemp, TEXT("\n"));
    for (auto ExternalIdTemp : ExternalIdsTemp)
    {
        ExternalIds.Add(ExternalIdTemp.TrimStartAndEnd());
    }

    FExternalIdQueryOptions Opts = {};
    Opts.AuthType = PlatformType;
    Opts.bLookupByDisplayName = false;

    User->QueryExternalIdMappings(
        *Identity->GetUniquePlayerId(0),
        Opts,
        ExternalIds,
        IOnlineUser::FOnQueryExternalIdMappingsComplete::CreateUObject(
            this,
            &UDemoSubsystem::HandleQueryExternalIdMappings,
            WorldContextObject,
            OnDone));
}

void UDemoSubsystem::HandleQueryExternalIdMappings(
    bool bWasSuccessful,
    const FUniqueNetId &UserId,
    const FExternalIdQueryOptions &QueryOptions,
    const TArray<FString> &ExternalIds,
    const FString &Error,
    const UObject *WorldContextObject,
    FDemoSubsystemQueryExternalIdMappingsComplete OnDone)
{
    if (!bWasSuccessful)
    {
        OnDone.ExecuteIfBound(FString::Printf(TEXT("Error: %s"), *Error));
        return;
    }

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(TEXT("Subsystem not available."));
        return;
    }
    IOnlineUserPtr User = Subsystem->GetUserInterface();
    if (User == nullptr)
    {
        OnDone.ExecuteIfBound(TEXT("Interface not available."));
        return;
    }

    TArray<FString> Lines;
    for (auto ExternalId : ExternalIds)
    {
        auto EUID = User->GetExternalIdMapping(QueryOptions, ExternalId);
        if (EUID.IsValid())
        {
            Lines.Add(FString::Printf(TEXT("%s = %s"), *ExternalId, *EUID->ToString()));
        }
        else
        {
            Lines.Add(FString::Printf(TEXT("%s = not found"), *ExternalId));
        }
    }
    OnDone.ExecuteIfBound(FString::Join(Lines, TEXT("\n")));
}

void UDemoSubsystem::QueryStats(const UObject *WorldContextObject, FDemoSubsystemQueryStatsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FDemoStat>());
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineStatsPtr Stats = Subsystem->GetStatsInterface();

    TArray<TSharedRef<const FUniqueNetId>> Users;
    Users.Add(Identity->GetUniquePlayerId(0).ToSharedRef());

    TArray<FString> StatNames;
    StatNames.Add(TEXT("TestLatest"));
    StatNames.Add(TEXT("TestScore"));

    Stats->QueryStats(
        Identity->GetUniquePlayerId(0).ToSharedRef(),
        Users,
        StatNames,
        FOnlineStatsQueryUsersStatsComplete::CreateUObject(
            this,
            &UDemoSubsystem::HandleQueryStats,
            WorldContextObject,
            OnDone));
}

void UDemoSubsystem::HandleQueryStats(
    const FOnlineError &ResultState,
    const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
    const UObject *WorldContextObject,
    FDemoSubsystemQueryStatsComplete OnDone)
{
    auto QueriedStats = UsersStatsResult[0];

    if (!ResultState.bSucceeded)
    {
        OnDone.ExecuteIfBound(false, TArray<FDemoStat>());
        return;
    }

    TArray<FDemoStat> Results;
    for (auto KV : QueriedStats->Stats)
    {
        FDemoStat Stat;
        Stat.Id = KV.Key;
        KV.Value.GetValue(Stat.CurrentValue);
        Results.Add(Stat);
    }

    OnDone.ExecuteIfBound(true, Results);
}

void UDemoSubsystem::IngestStat(
    const UObject *WorldContextObject,
    FString StatName,
    int32 IngestAmount,
    FDemoSubsystemIngestStatComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineStatsPtr StatsInterface = Subsystem->GetStatsInterface();

    FOnlineStatsUserUpdatedStats Stat = FOnlineStatsUserUpdatedStats(Identity->GetUniquePlayerId(0).ToSharedRef());
    Stat.Stats.Add(StatName, FOnlineStatUpdate(IngestAmount, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));

    TArray<FOnlineStatsUserUpdatedStats> Stats;
    Stats.Add(Stat);

    StatsInterface->UpdateStats(
        Identity->GetUniquePlayerId(0).ToSharedRef(),
        Stats,
        FOnlineStatsUpdateStatsComplete::CreateUObject(
            this,
            &UDemoSubsystem::HandleIngestStat,
            WorldContextObject,
            OnDone));
}

void UDemoSubsystem::HandleIngestStat(
    const FOnlineError &ResultState,
    const UObject *WorldContextObject,
    FDemoSubsystemIngestStatComplete OnDone)
{
    OnDone.ExecuteIfBound(ResultState.bSucceeded);
}

void UDemoSubsystem::QueryAchievements(
    const UObject *WorldContextObject,
    FDemoSubsystemQueryAchievementsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FDemoAchievement>());
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineAchievementsPtr AchievementsInterface = Subsystem->GetAchievementsInterface();

    AchievementsInterface->QueryAchievementDescriptions(
        *Identity->GetUniquePlayerId(0),
        FOnQueryAchievementsCompleteDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleQueryAchievementDescriptions,
            WorldContextObject,
            OnDone));
}

void UDemoSubsystem::HandleQueryAchievementDescriptions(
    const FUniqueNetId &UserId,
    const bool bWasSuccessful,
    const UObject *WorldContextObject,
    FDemoSubsystemQueryAchievementsComplete OnDone)
{
    if (!bWasSuccessful)
    {
        OnDone.ExecuteIfBound(false, TArray<FDemoAchievement>());
        return;
    }

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FDemoAchievement>());
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineAchievementsPtr AchievementsInterface = Subsystem->GetAchievementsInterface();

    AchievementsInterface->QueryAchievements(
        *Identity->GetUniquePlayerId(0),
        FOnQueryAchievementsCompleteDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleQueryAchievements,
            WorldContextObject,
            OnDone));
}

void UDemoSubsystem::HandleQueryAchievements(
    const FUniqueNetId &UserId,
    const bool bWasSuccessful,
    const UObject *WorldContextObject,
    FDemoSubsystemQueryAchievementsComplete OnDone)
{
    if (!bWasSuccessful)
    {
        OnDone.ExecuteIfBound(false, TArray<FDemoAchievement>());
        return;
    }

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FDemoAchievement>());
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineAchievementsPtr AchievementsInterface = Subsystem->GetAchievementsInterface();

    TArray<FOnlineAchievement> AchievementsData;
    verify(
        AchievementsInterface->GetCachedAchievements(*Identity->GetUniquePlayerId(0), AchievementsData) ==
        EOnlineCachedResult::Success);

    TArray<FDemoAchievement> Results;
    for (auto Data : AchievementsData)
    {
        FDemoAchievement Achievement;
        Achievement.Id = Data.Id;
        Achievement.Progress = Data.Progress;
        Achievement.bUnlocked = Data.Progress >= 100.0f;

        FOnlineAchievementDesc AchievementDesc;
        verify(
            AchievementsInterface->GetCachedAchievementDescription(Data.Id, AchievementDesc) ==
            EOnlineCachedResult::Success);
        Achievement.DisplayName = AchievementDesc.Title;
        Achievement.Description = Achievement.bUnlocked ? AchievementDesc.UnlockedDesc : AchievementDesc.LockedDesc;

        Results.Add(Achievement);
    }

    OnDone.ExecuteIfBound(true, Results);
}

void UDemoSubsystem::QueryFriendsLeaderboards(
    const UObject *WorldContextObject,
    FDemoSubsystemQueryLeaderboardsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FDemoLeaderboardEntry>());
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineLeaderboardsPtr LeaderboardsInterface = Subsystem->GetLeaderboardsInterface();

    FOnlineLeaderboardReadRef ReadRef = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
    ReadRef->ColumnMetadata.Add(FColumnMetaData(FName(TEXT("TestScore")), EOnlineKeyValuePairDataType::Int32));

    // note: This example code only supports running one leaderboard process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->QueryLeaderboardsDelegateHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Already a leaderboard operation running."));
        OnDone.ExecuteIfBound(false, TArray<FDemoLeaderboardEntry>());
        return;
    }

    this->QueryLeaderboardsDelegateHandle = LeaderboardsInterface->AddOnLeaderboardReadCompleteDelegate_Handle(
        FOnLeaderboardReadComplete::FDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleLeaderboardResult,
            ReadRef,
            WorldContextObject,
            OnDone));
    if (!LeaderboardsInterface->ReadLeaderboardsForFriends(0, ReadRef))
    {
        LeaderboardsInterface->ClearOnLeaderboardReadCompleteDelegate_Handle(this->QueryLeaderboardsDelegateHandle);
        this->QueryLeaderboardsDelegateHandle.Reset();
        OnDone.ExecuteIfBound(false, TArray<FDemoLeaderboardEntry>());
    }
}

void UDemoSubsystem::QueryGlobalLeaderboards(
    const UObject *WorldContextObject,
    const FString &LeaderboardId,
    FDemoSubsystemQueryLeaderboardsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FDemoLeaderboardEntry>());
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineLeaderboardsPtr LeaderboardsInterface = Subsystem->GetLeaderboardsInterface();

    FOnlineLeaderboardReadRef ReadRef = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
    ReadRef->LeaderboardName = FName(LeaderboardId);

    // note: This example code only supports running one leaderboard process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->QueryLeaderboardsDelegateHandle.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Already a leaderboard operation running."));
        OnDone.ExecuteIfBound(false, TArray<FDemoLeaderboardEntry>());
        return;
    }

    this->QueryLeaderboardsDelegateHandle = LeaderboardsInterface->AddOnLeaderboardReadCompleteDelegate_Handle(
        FOnLeaderboardReadComplete::FDelegate::CreateUObject(
            this,
            &UDemoSubsystem::HandleLeaderboardResult,
            ReadRef,
            WorldContextObject,
            OnDone));
    if (!LeaderboardsInterface->ReadLeaderboardsAroundRank(0, 100, ReadRef))
    {
        LeaderboardsInterface->ClearOnLeaderboardReadCompleteDelegate_Handle(this->QueryLeaderboardsDelegateHandle);
        this->QueryLeaderboardsDelegateHandle.Reset();
        OnDone.ExecuteIfBound(false, TArray<FDemoLeaderboardEntry>());
    }
}

void UDemoSubsystem::HandleLeaderboardResult(
    const bool bWasSuccessful,
    FOnlineLeaderboardReadRef LeaderboardRef,
    const UObject *WorldContextObject,
    FDemoSubsystemQueryLeaderboardsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    check(Subsystem != nullptr);
    IOnlineLeaderboardsPtr LeaderboardsInterface = Subsystem->GetLeaderboardsInterface();
    LeaderboardsInterface->ClearOnLeaderboardReadCompleteDelegate_Handle(this->QueryLeaderboardsDelegateHandle);
    this->QueryLeaderboardsDelegateHandle.Reset();

    if (!bWasSuccessful)
    {
        OnDone.ExecuteIfBound(false, TArray<FDemoLeaderboardEntry>());
        return;
    }

    TArray<FDemoLeaderboardEntry> Results;
    for (auto Row : LeaderboardRef->Rows)
    {
        FDemoLeaderboardEntry Entry;
        Entry.PlayerId = Row.PlayerId->ToString();
        Entry.PlayerNickname = Row.NickName;
        Entry.Rank = Row.Rank;
        if (Row.Columns.Contains(TEXT("Score")))
        {
            // For global leaderboards, column is always called "Score".
            Row.Columns[TEXT("Score")].GetValue(Entry.Score);
        }
        else
        {
            // For friend leaderboards, we requested the "TestScore" stat.
            Row.Columns[TEXT("TestScore")].GetValue(Entry.Score);
        }
        Results.Add(Entry);
    }

    OnDone.ExecuteIfBound(true, Results);
}

void UDemoSubsystem::SeamlessTravel(const UObject *WorldContextObject)
{
    AGameModeBase *GameModeBase = WorldContextObject->GetWorld()->GetAuthGameMode<AGameModeBase>();
    if (GameModeBase == nullptr)
    {
        return;
    }

    GameModeBase->bUseSeamlessTravel = true;
    if (WorldContextObject->GetWorld()->GetMapName().Contains("MultiplayerMap2"))
    {
        WorldContextObject->GetWorld()->ServerTravel(TEXT("/Game/MultiplayerMap"), true);
    }
    else
    {
        WorldContextObject->GetWorld()->ServerTravel(TEXT("/Game/MultiplayerMap2"), true);
    }
}

void UDemoSubsystem::NonSeamlessTravel(const UObject *WorldContextObject)
{
    AGameModeBase *GameModeBase = WorldContextObject->GetWorld()->GetAuthGameMode<AGameModeBase>();
    if (GameModeBase == nullptr)
    {
        return;
    }

    GameModeBase->bUseSeamlessTravel = false;
    if (WorldContextObject->GetWorld()->GetMapName().Contains("MultiplayerMap2"))
    {
        WorldContextObject->GetWorld()->ServerTravel(TEXT("/Game/MultiplayerMap"), true);
    }
    else
    {
        WorldContextObject->GetWorld()->ServerTravel(TEXT("/Game/MultiplayerMap2"), true);
    }
}