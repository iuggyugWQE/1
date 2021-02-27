// Copyright 2020 June Rhodes. All Rights Reserved.

#include "ExampleCPPSubsystem.h"

#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "ExampleCPPConfigLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UExampleCPPSubsystem::PostInitProperties()
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
        IOnlinePresencePtr Presence = Subsystem->GetPresenceInterface();
        Party->AddOnPartyInvitesChangedDelegate_Handle(
            FOnPartyInvitesChangedDelegate::CreateUObject(this, &UExampleCPPSubsystem::OnPartyInvitesChanged));
        Party->AddOnPartyDataReceivedDelegate_Handle(
            FOnPartyDataReceivedDelegate::CreateUObject(this, &UExampleCPPSubsystem::OnPartyDataReceived));
        Session->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::OnSessionUserInviteAccepted));
        Session->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionCompleteDelegate::CreateUObject(this, &UExampleCPPSubsystem::OnSessionJoinedViaOverlay));
        Presence->AddOnPresenceReceivedDelegate_Handle(
            FOnPresenceReceivedDelegate::CreateUObject(this, &UExampleCPPSubsystem::OnPresenceReceived));
    }
}

void UExampleCPPSubsystem::OnPartyDataReceived(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &Namespace,
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
                    &UExampleCPPSubsystem::OnFindSessionForLeaderFollow,
                    SessionFollowString));
        }
    }
}

void UExampleCPPSubsystem::OnFindSessionForLeaderFollow(
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
        this->JoinSessionForLeaderFollowDelegateHandle =
            Session->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionComplete::FDelegate::CreateUObject(
                this,
                &UExampleCPPSubsystem::OnJoinSessionForLeaderFollow));
        if (!Session->JoinSession(0, FName(SessionFollowString), SearchResult))
        {
            UE_LOG(LogTemp, Error, TEXT("Could not call JoinSession after following party leader!"));
        }
    }
}

void UExampleCPPSubsystem::OnJoinSessionForLeaderFollow(
    FName SessionName,
    EOnJoinSessionCompleteResult::Type JoinResult)
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

void UExampleCPPSubsystem::OnSessionUserInviteAccepted(
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
        FOnJoinSessionComplete::FDelegate::CreateUObject(this, &UExampleCPPSubsystem::OnJoinSessionForInviteFollow));
    if (!Session->JoinSession(0, FName(*InviteResult.GetSessionIdStr()), InviteResult))
    {
        UE_LOG(LogTemp, Error, TEXT("Could not call JoinSession after accepting invite!"));
    }
}

void UExampleCPPSubsystem::OnJoinSessionForInviteFollow(
    FName SessionName,
    EOnJoinSessionCompleteResult::Type JoinResult)
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

void UExampleCPPSubsystem::OnSessionJoinedViaOverlay(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
    if (JoinResult == EOnJoinSessionCompleteResult::Success &&
        SessionName.ToString().StartsWith(TEXT("JoinViaOverlay_")))
    {
        this->SessionJoinedMapTravelRequired.Broadcast(SessionName);
    }
}

void UExampleCPPSubsystem::OnPartyInvitesChanged(const FUniqueNetId &LocalUserId)
{
    this->OnInvitationsChanged.Broadcast();
}

bool UExampleCPPSubsystem::IsSignedIn(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return false;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

    return Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn;
}

FString UExampleCPPSubsystem::GetLoggedInUsername(const UObject *WorldContextObject)
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

FString UExampleCPPSubsystem::GetLoggedInProductUserId(const UObject *WorldContextObject)
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

FString UExampleCPPSubsystem::GetLoggedInEpicAccountId(const UObject *WorldContextObject)
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

FString UExampleCPPSubsystem::GetLoggedInAuthAttribute(const UObject *WorldContextObject, const FString &InAuthAttrName)
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
                if (Account->GetAuthAttribute(InAuthAttrName, OutAttr))
                {
                    return OutAttr;
                }
            }
        }
    }

    return TEXT("");
}

bool UExampleCPPSubsystem::CanLinkToEpicGamesAccount(const UObject *WorldContextObject)
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

void UExampleCPPSubsystem::StartLogin(
    const UObject *WorldContextObject,
    int32 LocalUserNum,
    FExampleCPPSubsystemLoginComplete OnDone)
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
            &UExampleCPPSubsystem::HandleLoginComplete,
            WorldContextObject,
            OnDone));
    if (!Identity->AutoLogin(LocalUserNum))
    {
        OnDone.ExecuteIfBound(false, TEXT("Call didn't start"));
    }
}

void UExampleCPPSubsystem::HandleLoginComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &UserId,
    const FString &Error,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemLoginComplete OnDone)
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

void UExampleCPPSubsystem::StartLogout(const UObject *WorldContextObject, FExampleCPPSubsystemLogoutComplete OnDone)
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
            &UExampleCPPSubsystem::HandleLogoutComplete,
            WorldContextObject,
            OnDone));
    if (!Identity->Logout(0))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleLogoutComplete(
    int32 _UnusedLocalUserNum,
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemLogoutComplete OnDone)
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

void UExampleCPPSubsystem::StartCreateSession(
    const UObject *WorldContextObject,
    FExampleCPPSubsystemCreateSessionComplete OnDone)
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
            &UExampleCPPSubsystem::HandleCreateSessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->CreateSession(0, FName(TEXT("MyLocalSessionName")), *SessionSettings))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleCreateSessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemCreateSessionComplete OnDone)
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

void UExampleCPPSubsystem::StartFindSessions(
    const UObject *WorldContextObject,
    FExampleCPPSubsystemFindSessionsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<UExampleCPPSessionSearchResult *>());
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    // note: This example code only supports running one FindSessions process at once; if you want to support
    // multiple in flight you probably need to make a latent blueprint node.
    if (this->FindSessionsDelegateHandle.IsValid())
    {
        OnDone.ExecuteIfBound(false, TArray<UExampleCPPSessionSearchResult *>());
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
            &UExampleCPPSubsystem::HandleFindSessionsComplete,
            WorldContextObject,
            OnDone,
            Search));
    if (!Session->FindSessions(0, Search))
    {
        OnDone.ExecuteIfBound(false, TArray<UExampleCPPSessionSearchResult *>());
    }
}

void UExampleCPPSubsystem::HandleFindSessionsComplete(
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemFindSessionsComplete OnDone,
    TSharedRef<FOnlineSessionSearch> Search)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    check(Subsystem != nullptr);
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
    check(Session != nullptr);

    TArray<UExampleCPPSessionSearchResult *> Results;
    if (bWasSuccessful)
    {
        for (auto RawResult : Search->SearchResults)
        {
            if (RawResult.IsValid())
            {
                auto Result = NewObject<UExampleCPPSessionSearchResult>(this);
                Result->Result = RawResult;
                Result->SessionId = RawResult.GetSessionIdStr();
                Session->GetResolvedConnectString(RawResult, NAME_GamePort, Result->ConnectionString);
                Results.Add(Result);
            }
        }
    }

    OnDone.ExecuteIfBound(bWasSuccessful, Results);

    Session->ClearOnFindSessionsCompleteDelegate_Handle(this->FindSessionsDelegateHandle);
    this->FindSessionsDelegateHandle.Reset();
}

FString UExampleCPPSubsystem::GetCurrentSessionId(const UObject *WorldContextObject)
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

TArray<FString> UExampleCPPSubsystem::GetPlayersInSession(const UObject *WorldContextObject)
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

void UExampleCPPSubsystem::RegisterExistingPlayers(const UObject *WorldContextObject)
{
    for (auto It = WorldContextObject->GetWorld()->GetPlayerControllerIterator(); It; --It)
    {
        this->RegisterPlayer(It->Get());
    }
}

void UExampleCPPSubsystem::RegisterPlayer(APlayerController *InPlayerController)
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

void UExampleCPPSubsystem::UnregisterPlayer(APlayerController *InPlayerController)
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

void UExampleCPPSubsystem::StartDestroySession(
    const UObject *WorldContextObject,
    FName SessionName,
    FExampleCPPSubsystemDestroySessionComplete OnDone)
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
            &UExampleCPPSubsystem::HandleDestroySessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->DestroySession(SessionName))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleDestroySessionComplete(
    FName SessionName,
    bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemDestroySessionComplete OnDone)
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

void UExampleCPPSubsystem::StartJoinSession(
    const UObject *WorldContextObject,
    UExampleCPPSessionSearchResult *SearchResult,
    FExampleCPPSubsystemJoinSessionComplete OnDone)
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
            &UExampleCPPSubsystem::HandleJoinSessionComplete,
            WorldContextObject,
            OnDone));
    if (!Session->JoinSession(0, FName(TEXT("MyLocalSessionName")), SearchResult->Result))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleJoinSessionComplete(
    FName SessionName,
    EOnJoinSessionCompleteResult::Type JoinResult,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemJoinSessionComplete OnDone)
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

void UExampleCPPSubsystem::SendBeaconPingToSearchResult(
    const UObject *WorldContextObject,
    UExampleCPPSessionSearchResult *SearchResult,
    FExampleCPPSubsystemBeaconPingComplete OnDone)
{
    if (!UExampleCPPConfigLibrary::GetAreBeaconsSupported(WorldContextObject))
    {
        OnDone.ExecuteIfBound(false, TEXT("Beacons are not supported on the legacy networking stack"));
        return;
    }

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TEXT("No online subsystem"));
        return;
    }
    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    FString ConnectInfo;
    // The port name is set to FName(TEXT("12345")) as per the comment in DemoBeacon.cpp.
    if (!Session->GetResolvedConnectString(SearchResult->Result, FName(TEXT("12345")), ConnectInfo))
    {
        OnDone.ExecuteIfBound(false, TEXT("Connect info not found"));
        return;
    }

    auto Beacon = WorldContextObject->GetWorld()->SpawnActor<AOnlineBeacon>(AExampleCPPBeaconClient::StaticClass());
    if (Beacon)
    {
        Cast<AExampleCPPBeaconClient>(Beacon)->OnComplete = OnDone;
        FURL OldURL;
        FURL URL(&OldURL, *ConnectInfo, ETravelType::TRAVEL_Absolute);
        Cast<AExampleCPPBeaconClient>(Beacon)->InitClient(URL);
    }
}

FString UExampleCPPSubsystem::GetResolvedConnectString(const UObject *WorldContextObject)
{
    return GetResolvedConnectStringForSession(WorldContextObject, FName(TEXT("MyLocalSessionName")));
}

FString UExampleCPPSubsystem::GetResolvedConnectStringForSession(const UObject *WorldContextObject, FName SessionName)
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

void UExampleCPPSubsystem::StartUpdatePresence(
    const UObject *WorldContextObject,
    const FString &NewPresenceString,
    FExampleCPPSubsystemUpdatePresenceComplete OnDone)
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
            &UExampleCPPSubsystem::HandleUpdatePresenceComplete,
            WorldContextObject,
            OnDone));
}

void UExampleCPPSubsystem::HandleUpdatePresenceComplete(
    const class FUniqueNetId &UserId,
    const bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemUpdatePresenceComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);
}

TArray<UExampleCPPPartyId *> UExampleCPPSubsystem::GetJoinedParties(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TArray<UExampleCPPPartyId *>();
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    TArray<TSharedRef<const FOnlinePartyId>> Result;
    if (!Party->GetJoinedParties(*Identity->GetUniquePlayerId(0).Get(), Result))
    {
        return TArray<UExampleCPPPartyId *>();
    }

    TArray<UExampleCPPPartyId *> ResultIds;
    for (auto Entry : Result)
    {
        UExampleCPPPartyId *Id = NewObject<UExampleCPPPartyId>(this);
        Id->PartyId = Entry;
        ResultIds.Add(Id);
    }
    return ResultIds;
}

void UExampleCPPSubsystem::RequestPartyMembersFollowLeader(const UObject *WorldContextObject, FName SessionName)
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
                if (!PartySystem->UpdatePartyData(*LocalUser, *PartyId, NAME_Default, *PartyDataUpdate))
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

void UExampleCPPSubsystem::StartCreateParty(
    const UObject *WorldContextObject,
    int PartyTypeId,
    FExampleCPPSubsystemCreatePartyComplete OnDone)
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
    // You must set InvitePermissions to ::Anyone in order for synthetic parties to work.
    Config->InvitePermissions = PartySystemPermissions::EPermissionType::Anyone;

    if (!Party->CreateParty(
            *Identity->GetUniquePlayerId(0).Get(),
            (FOnlinePartyTypeId)PartyTypeId,
            *Config,
            FOnCreatePartyComplete::CreateUObject(
                this,
                &UExampleCPPSubsystem::HandleCreatePartyComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleCreatePartyComplete(
    const FUniqueNetId &LocalUserId,
    const TSharedPtr<const FOnlinePartyId> &PartyId,
    const ECreatePartyCompletionResult Result,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemCreatePartyComplete OnDone)
{
    OnDone.ExecuteIfBound(Result == ECreatePartyCompletionResult::Succeeded);
}

TArray<UExampleCPPPartyInvite *> UExampleCPPSubsystem::GetInvitations(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TArray<UExampleCPPPartyInvite *>();
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    TArray<IOnlinePartyJoinInfoConstRef> PendingInvites;
    if (!Party->GetPendingInvites(*Identity->GetUniquePlayerId(0).Get(), PendingInvites))
    {
        return TArray<UExampleCPPPartyInvite *>();
    }

    TArray<UExampleCPPPartyInvite *> Results;
    for (auto Entry : PendingInvites)
    {
        UExampleCPPPartyInvite *Id = NewObject<UExampleCPPPartyInvite>(this);
        Id->PartyInvite = Entry;
        Results.Add(Id);
    }
    return Results;
}

TArray<UExampleCPPPartyMemberId *> UExampleCPPSubsystem::GetPartyMembers(
    const UObject *WorldContextObject,
    UExampleCPPPartyId *PartyId)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TArray<UExampleCPPPartyMemberId *>();
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlinePartyPtr Party = Subsystem->GetPartyInterface();

    TArray<FOnlinePartyMemberConstRef> PartyMembers;
    if (!Party->GetPartyMembers(*Identity->GetUniquePlayerId(0).Get(), *PartyId->PartyId.Get(), PartyMembers))
    {
        return TArray<UExampleCPPPartyMemberId *>();
    }

    TArray<UExampleCPPPartyMemberId *> Results;
    for (auto Entry : PartyMembers)
    {
        UExampleCPPPartyMemberId *Id = NewObject<UExampleCPPPartyMemberId>(this);
        Id->PartyMember = Entry;
        Results.Add(Id);
    }
    return Results;
}

bool UExampleCPPSubsystem::IsPartyLeader(const UObject *WorldContextObject, UExampleCPPPartyId *PartyId)
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

void UExampleCPPSubsystem::StartJoinParty(
    const UObject *WorldContextObject,
    UExampleCPPPartyInvite *Invite,
    FExampleCPPSubsystemJoinPartyComplete OnDone)
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
                &UExampleCPPSubsystem::HandleJoinPartyComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleJoinPartyComplete(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const EJoinPartyCompletionResult Result,
    const int32 NotApprovedReason,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemJoinPartyComplete OnDone)
{
    OnDone.ExecuteIfBound(Result == EJoinPartyCompletionResult::Succeeded);
}

void UExampleCPPSubsystem::StartLeaveParty(
    const UObject *WorldContextObject,
    UExampleCPPPartyId *PartyId,
    FExampleCPPSubsystemLeavePartyComplete OnDone)
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
                &UExampleCPPSubsystem::HandleLeavePartyComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleLeavePartyComplete(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const ELeavePartyCompletionResult Result,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemLeavePartyComplete OnDone)
{
    OnDone.ExecuteIfBound(Result == ELeavePartyCompletionResult::Succeeded);
}

void UExampleCPPSubsystem::StartKickMember(
    const UObject *WorldContextObject,
    UExampleCPPPartyId *PartyId,
    UExampleCPPPartyMemberId *MemberId,
    FExampleCPPSubsystemKickMemberComplete OnDone)
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
                &UExampleCPPSubsystem::HandleKickMemberComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleKickMemberComplete(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const EKickMemberCompletionResult Result,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemKickMemberComplete OnDone)
{
    OnDone.ExecuteIfBound(Result == EKickMemberCompletionResult::Succeeded);
}

TArray<UExampleCPPFriend *> UExampleCPPSubsystem::GetFriends(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return TArray<UExampleCPPFriend *>();
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    TArray<TSharedRef<FOnlineFriend>> FriendsRaw;
    if (!Friends->GetFriendsList(0, TEXT(""), FriendsRaw))
    {
        return TArray<UExampleCPPFriend *>();
    }

    TArray<UExampleCPPFriend *> Results;
    for (auto Friend : FriendsRaw)
    {
        UExampleCPPFriend *FriendResult = NewObject<UExampleCPPFriend>(this);
        FriendResult->Friend = Friend;
        Results.Add(FriendResult);
    }
    return Results;
}

void UExampleCPPSubsystem::StartReadFriends(
    const UObject *WorldContextObject,
    FExampleCPPSubsystemReadFriendsComplete OnDone)
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
                &UExampleCPPSubsystem::HandleReadFriendsComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleReadFriendsComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FString &ListName,
    const FString &ErrorStr,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemReadFriendsComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);
}

void UExampleCPPSubsystem::StartInviteFriend(
    const UObject *WorldContextObject,
    UExampleCPPPartyId *PartyId,
    UExampleCPPFriend *Friend,
    FExampleCPPSubsystemInviteFriendComplete OnDone)
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
                &UExampleCPPSubsystem::HandleInviteFriendComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleInviteFriendComplete(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &RecipientId,
    const ESendPartyInvitationCompletionResult Result,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemInviteFriendComplete OnDone)
{
    OnDone.ExecuteIfBound(Result == ESendPartyInvitationCompletionResult::Succeeded);
}

void UExampleCPPSubsystem::QueryUserInfo(
    const UObject *WorldContextObject,
    FString ProductUserIdInput,
    FString EpicAccountIdInput,
    FExampleCPPSubsystemQueryUserInfoComplete OnDone)
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
            &UExampleCPPSubsystem::HandleQueryUserInfoComplete,
            WorldContextObject,
            OnDone));

    TArray<TSharedRef<const FUniqueNetId>> UserIds;
    UserIds.Add(UniqueNetId.ToSharedRef());
    User->QueryUserInfo(0, UserIds);
}

void UExampleCPPSubsystem::HandleQueryUserInfoComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const TArray<TSharedRef<const FUniqueNetId>> &UserIds,
    const FString &ErrorStr,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemQueryUserInfoComplete OnDone)
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

void UExampleCPPSubsystem::QueryUserIdMapping(
    const UObject *WorldContextObject,
    FString DisplayNameInput,
    FExampleCPPSubsystemQueryUserIdMappingComplete OnDone)
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
            &UExampleCPPSubsystem::HandleQueryUserMappingComplete,
            WorldContextObject,
            OnDone));
}

void UExampleCPPSubsystem::HandleQueryUserMappingComplete(
    bool bWasSuccessful,
    const FUniqueNetId &UserId,
    const FString &DisplayNameOrEmail,
    const FUniqueNetId &FoundUserId,
    const FString &Error,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemQueryUserIdMappingComplete OnDone)
{
    if (!bWasSuccessful)
    {
        OnDone.ExecuteIfBound(FString::Printf(TEXT("Error: %s"), *Error));
        return;
    }

    OnDone.ExecuteIfBound(FString::Printf(TEXT("Found user: %s"), *FoundUserId.ToString()));
}

void UExampleCPPSubsystem::QueryExternalIdMappings(
    const UObject *WorldContextObject,
    FString PlatformType,
    FString ExternalIdsStr,
    FExampleCPPSubsystemQueryExternalIdMappingsComplete OnDone)
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
            &UExampleCPPSubsystem::HandleQueryExternalIdMappings,
            WorldContextObject,
            OnDone));
}

void UExampleCPPSubsystem::HandleQueryExternalIdMappings(
    bool bWasSuccessful,
    const FUniqueNetId &UserId,
    const FExternalIdQueryOptions &QueryOptions,
    const TArray<FString> &ExternalIds,
    const FString &Error,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemQueryExternalIdMappingsComplete OnDone)
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

void UExampleCPPSubsystem::QueryStats(const UObject *WorldContextObject, FExampleCPPSubsystemQueryStatsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPStat>());
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
            &UExampleCPPSubsystem::HandleQueryStats,
            WorldContextObject,
            OnDone));
}

void UExampleCPPSubsystem::HandleQueryStats(
    const FOnlineError &ResultState,
    const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemQueryStatsComplete OnDone)
{
    auto QueriedStats = UsersStatsResult[0];

    if (!ResultState.bSucceeded)
    {
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPStat>());
        return;
    }

    TArray<FExampleCPPStat> Results;
    for (auto KV : QueriedStats->Stats)
    {
        FExampleCPPStat Stat;
        Stat.Id = KV.Key;
        KV.Value.GetValue(Stat.CurrentValue);
        Results.Add(Stat);
    }

    OnDone.ExecuteIfBound(true, Results);
}

void UExampleCPPSubsystem::IngestStat(
    const UObject *WorldContextObject,
    FString StatName,
    int32 IngestAmount,
    FExampleCPPSubsystemIngestStatComplete OnDone)
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
            &UExampleCPPSubsystem::HandleIngestStat,
            WorldContextObject,
            OnDone));
}

void UExampleCPPSubsystem::HandleIngestStat(
    const FOnlineError &ResultState,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemIngestStatComplete OnDone)
{
    OnDone.ExecuteIfBound(ResultState.bSucceeded);
}

void UExampleCPPSubsystem::QueryAchievements(
    const UObject *WorldContextObject,
    FExampleCPPSubsystemQueryAchievementsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPAchievement>());
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineAchievementsPtr AchievementsInterface = Subsystem->GetAchievementsInterface();

    AchievementsInterface->QueryAchievementDescriptions(
        *Identity->GetUniquePlayerId(0),
        FOnQueryAchievementsCompleteDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleQueryAchievementDescriptions,
            WorldContextObject,
            OnDone));
}

void UExampleCPPSubsystem::HandleQueryAchievementDescriptions(
    const FUniqueNetId &UserId,
    const bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemQueryAchievementsComplete OnDone)
{
    if (!bWasSuccessful)
    {
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPAchievement>());
        return;
    }

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPAchievement>());
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineAchievementsPtr AchievementsInterface = Subsystem->GetAchievementsInterface();

    AchievementsInterface->QueryAchievements(
        *Identity->GetUniquePlayerId(0),
        FOnQueryAchievementsCompleteDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleQueryAchievements,
            WorldContextObject,
            OnDone));
}

void UExampleCPPSubsystem::HandleQueryAchievements(
    const FUniqueNetId &UserId,
    const bool bWasSuccessful,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemQueryAchievementsComplete OnDone)
{
    if (!bWasSuccessful)
    {
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPAchievement>());
        return;
    }

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPAchievement>());
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineAchievementsPtr AchievementsInterface = Subsystem->GetAchievementsInterface();

    TArray<FOnlineAchievement> AchievementsData;
    verify(
        AchievementsInterface->GetCachedAchievements(*Identity->GetUniquePlayerId(0), AchievementsData) ==
        EOnlineCachedResult::Success);

    TArray<FExampleCPPAchievement> Results;
    for (auto Data : AchievementsData)
    {
        FExampleCPPAchievement Achievement;
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

void UExampleCPPSubsystem::QueryFriendsLeaderboards(
    const UObject *WorldContextObject,
    FExampleCPPSubsystemQueryLeaderboardsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPLeaderboardEntry>());
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
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPLeaderboardEntry>());
        return;
    }

    this->QueryLeaderboardsDelegateHandle = LeaderboardsInterface->AddOnLeaderboardReadCompleteDelegate_Handle(
        FOnLeaderboardReadComplete::FDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleLeaderboardResult,
            ReadRef,
            WorldContextObject,
            OnDone));
    if (!LeaderboardsInterface->ReadLeaderboardsForFriends(0, ReadRef))
    {
        LeaderboardsInterface->ClearOnLeaderboardReadCompleteDelegate_Handle(this->QueryLeaderboardsDelegateHandle);
        this->QueryLeaderboardsDelegateHandle.Reset();
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPLeaderboardEntry>());
    }
}

void UExampleCPPSubsystem::QueryGlobalLeaderboards(
    const UObject *WorldContextObject,
    const FString &LeaderboardId,
    FExampleCPPSubsystemQueryLeaderboardsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPLeaderboardEntry>());
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
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPLeaderboardEntry>());
        return;
    }

    this->QueryLeaderboardsDelegateHandle = LeaderboardsInterface->AddOnLeaderboardReadCompleteDelegate_Handle(
        FOnLeaderboardReadComplete::FDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleLeaderboardResult,
            ReadRef,
            WorldContextObject,
            OnDone));
    if (!LeaderboardsInterface->ReadLeaderboardsAroundRank(0, 100, ReadRef))
    {
        LeaderboardsInterface->ClearOnLeaderboardReadCompleteDelegate_Handle(this->QueryLeaderboardsDelegateHandle);
        this->QueryLeaderboardsDelegateHandle.Reset();
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPLeaderboardEntry>());
    }
}

void UExampleCPPSubsystem::HandleLeaderboardResult(
    const bool bWasSuccessful,
    FOnlineLeaderboardReadRef LeaderboardRef,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemQueryLeaderboardsComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    check(Subsystem != nullptr);
    IOnlineLeaderboardsPtr LeaderboardsInterface = Subsystem->GetLeaderboardsInterface();
    LeaderboardsInterface->ClearOnLeaderboardReadCompleteDelegate_Handle(this->QueryLeaderboardsDelegateHandle);
    this->QueryLeaderboardsDelegateHandle.Reset();

    if (!bWasSuccessful)
    {
        OnDone.ExecuteIfBound(false, TArray<FExampleCPPLeaderboardEntry>());
        return;
    }

    TArray<FExampleCPPLeaderboardEntry> Results;
    for (auto Row : LeaderboardRef->Rows)
    {
        FExampleCPPLeaderboardEntry Entry;
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

void UExampleCPPSubsystem::SeamlessTravel(const UObject *WorldContextObject)
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

void UExampleCPPSubsystem::NonSeamlessTravel(const UObject *WorldContextObject)
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

void UExampleCPPSubsystem::QueryPresence(
    const UObject *WorldContextObject,
    FExampleCPPSubsystemQueryPresenceComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    check(Subsystem != nullptr);
    IOnlinePresencePtr PresenceInterface = Subsystem->GetPresenceInterface();
    check(PresenceInterface != nullptr);
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    check(Identity != nullptr);

    PresenceInterface->QueryPresence(
        *Identity->GetUniquePlayerId(0),
        IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleQueryPresenceComplete));
}

void UExampleCPPSubsystem::HandleQueryPresenceComplete(const class FUniqueNetId &UserId, const bool bWasSuccessful)
{
    UE_LOG(LogTemp, Verbose, TEXT("Query presence result: %s"), bWasSuccessful ? TEXT("true") : TEXT("false"));
}

void UExampleCPPSubsystem::OnPresenceReceived(
    const class FUniqueNetId &UserId,
    const TSharedRef<FOnlineUserPresence> &Presence)
{
    PresenceUpdated.Broadcast(FUniqueNetIdRepl(UserId).ToString(), Presence->Status.ToDebugString());
}