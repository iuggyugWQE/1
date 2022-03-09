// Copyright June Rhodes. MIT Licensed.

#include "../ExampleCPPSubsystem.h"

#include "../ExampleCPPConfigLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UExampleCPPSubsystem::OnFriendsChange()
{
    this->OnFriendsChanged.Broadcast();
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

void UExampleCPPSubsystem::StartSetFriendAlias(
    const UObject *WorldContextObject,
    FUniqueNetIdRepl UserId,
    const FString &Alias,
    FExampleCPPSubsystemSetFriendAliasComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    Friends->SetFriendAlias(
        0,
        *UserId.GetUniqueNetId(),
        TEXT(""),
        Alias,
        FOnSetFriendAliasComplete::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleSetFriendAliasComplete,
            WorldContextObject,
            OnDone));
}

void UExampleCPPSubsystem::HandleSetFriendAliasComplete(
    int32 LocalUserNum,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FOnlineError &Error,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemSetFriendAliasComplete OnDone)
{
    OnDone.ExecuteIfBound(Error.bSucceeded);
}

void UExampleCPPSubsystem::StartDeleteFriendAlias(
    const UObject *WorldContextObject,
    FUniqueNetIdRepl UserId,
    FExampleCPPSubsystemDeleteFriendAliasComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    Friends->DeleteFriendAlias(
        0,
        *UserId.GetUniqueNetId(),
        TEXT(""),
        FOnDeleteFriendAliasComplete::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleDeleteFriendAliasComplete,
            WorldContextObject,
            OnDone));
}

void UExampleCPPSubsystem::HandleDeleteFriendAliasComplete(
    int32 LocalUserNum,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FOnlineError &Error,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemDeleteFriendAliasComplete OnDone)
{
    OnDone.ExecuteIfBound(Error.bSucceeded);
}

void UExampleCPPSubsystem::StartReadBlockedUsers(
    const UObject *WorldContextObject,
    FExampleCPPSubsystemReadUsersComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    this->QueryBlockedPlayersHandle =
        Friends->AddOnQueryBlockedPlayersCompleteDelegate_Handle(FOnQueryBlockedPlayersCompleteDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleReadBlockedUsersComplete,
            WorldContextObject,
            OnDone));

    if (!Friends->QueryBlockedPlayers(*Identity->GetUniquePlayerId(0)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

TArray<FExampleCPPSimpleUser> UExampleCPPSubsystem::GetBlockedUsers(const UObject *WorldContextObject)
{
    TArray<FExampleCPPSimpleUser> Results;

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return Results;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    TArray<TSharedRef<FOnlineBlockedPlayer>> BlockedUsers;
    if (!Friends->GetBlockedPlayers(*Identity->GetUniquePlayerId(0), BlockedUsers))
    {
        return Results;
    }
    for (const auto &BlockedUser : BlockedUsers)
    {
        FExampleCPPSimpleUser SimpleUser;
        SimpleUser.UserId = BlockedUser->GetUserId()->ToString();
        SimpleUser.DisplayName = BlockedUser->GetDisplayName();
        Results.Add(SimpleUser);
    }
    return Results;
}

void UExampleCPPSubsystem::HandleReadBlockedUsersComplete(
    const FUniqueNetId &UserId,
    bool bWasSuccessful,
    const FString &Error,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemReadUsersComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    Friends->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(this->QueryBlockedPlayersHandle);

    OnDone.ExecuteIfBound(bWasSuccessful);
}

TArray<FExampleCPPSimpleUser> UExampleCPPSubsystem::GetRecentPlayers(const UObject *WorldContextObject)
{
    TArray<FExampleCPPSimpleUser> Results;

    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        return Results;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    TArray<TSharedRef<FOnlineRecentPlayer>> RecentPlayers;
    if (!Friends->GetRecentPlayers(*Identity->GetUniquePlayerId(0), TEXT(""), RecentPlayers))
    {
        return Results;
    }
    for (const auto &RecentPlayer : RecentPlayers)
    {
        FExampleCPPSimpleUser SimpleUser;
        SimpleUser.UserId = RecentPlayer->GetUserId()->ToString();
        SimpleUser.DisplayName = RecentPlayer->GetDisplayName();
        Results.Add(SimpleUser);
    }
    return Results;
}

void UExampleCPPSubsystem::StartReadRecentPlayers(
    const UObject *WorldContextObject,
    FExampleCPPSubsystemReadUsersComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    this->QueryRecentPlayersHandle =
        Friends->AddOnQueryRecentPlayersCompleteDelegate_Handle(FOnQueryRecentPlayersCompleteDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleReadRecentPlayersComplete,
            WorldContextObject,
            OnDone));

    if (!Friends->QueryRecentPlayers(*Identity->GetUniquePlayerId(0), TEXT("")))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleReadRecentPlayersComplete(
    const FUniqueNetId &UserId,
    const FString &Namespace,
    bool bWasSuccessful,
    const FString &Error,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemReadUsersComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    Friends->ClearOnQueryRecentPlayersCompleteDelegate_Handle(this->QueryRecentPlayersHandle);

    OnDone.ExecuteIfBound(bWasSuccessful);
}

void UExampleCPPSubsystem::AcceptFriendInvite(
    const UObject *WorldContextObject,
    const FString &TargetUserId,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    if (!Friends->AcceptInvite(
            0,
            *Identity->CreateUniquePlayerId(TargetUserId),
            TEXT(""),
            FOnAcceptInviteComplete::CreateUObject(
                this,
                &UExampleCPPSubsystem::HandleAcceptFriendInviteComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleAcceptFriendInviteComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FString &ErrorStr,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);
}

void UExampleCPPSubsystem::BlockPlayer(
    const UObject *WorldContextObject,
    const FString &TargetUserId,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    this->BlockPlayerCompleteHandle = Friends->AddOnBlockedPlayerCompleteDelegate_Handle(
        0,
        FOnBlockedPlayerCompleteDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleBlockPlayerComplete,
            WorldContextObject,
            OnDone));

    if (!Friends->BlockPlayer(0, *Identity->CreateUniquePlayerId(TargetUserId)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleBlockPlayerComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &UniqueID,
    const FString &ListName,
    const FString &ErrorStr,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    Friends->ClearOnBlockedPlayerCompleteDelegate_Handle(0, this->BlockPlayerCompleteHandle);

    OnDone.ExecuteIfBound(bWasSuccessful);
}

void UExampleCPPSubsystem::DeleteFriend(
    const UObject *WorldContextObject,
    const FString &TargetUserId,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    this->DeleteFriendCompleteHandle = Friends->AddOnDeleteFriendCompleteDelegate_Handle(
        0,
        FOnDeleteFriendCompleteDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleDeleteFriendComplete,
            WorldContextObject,
            OnDone));

    if (!Friends->DeleteFriend(0, *Identity->CreateUniquePlayerId(TargetUserId), TEXT("")))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleDeleteFriendComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FString &ErrorStr,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    Friends->ClearOnDeleteFriendCompleteDelegate_Handle(0, this->DeleteFriendCompleteHandle);

    OnDone.ExecuteIfBound(bWasSuccessful);
}

void UExampleCPPSubsystem::RejectFriendInvite(
    const UObject *WorldContextObject,
    const FString &TargetUserId,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    this->RejectFriendInviteCompleteHandle = Friends->AddOnRejectInviteCompleteDelegate_Handle(
        0,
        FOnRejectInviteCompleteDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleRejectFriendInvite,
            WorldContextObject,
            OnDone));

    if (!Friends->RejectInvite(0, *Identity->CreateUniquePlayerId(TargetUserId), TEXT("")))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleRejectFriendInvite(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FString &ErrorStr,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    Friends->ClearOnRejectInviteCompleteDelegate_Handle(0, this->RejectFriendInviteCompleteHandle);

    OnDone.ExecuteIfBound(bWasSuccessful);
}

void UExampleCPPSubsystem::SendFriendInvite(
    const UObject *WorldContextObject,
    const FString &TargetUserId,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    if (!Friends->SendInvite(
            0,
            *Identity->CreateUniquePlayerId(TargetUserId),
            TEXT(""),
            FOnSendInviteComplete::CreateUObject(
                this,
                &UExampleCPPSubsystem::HandleSendFriendInviteComplete,
                WorldContextObject,
                OnDone)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleSendFriendInviteComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FString &ErrorStr,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    OnDone.ExecuteIfBound(bWasSuccessful);
}

void UExampleCPPSubsystem::UnblockPlayer(
    const UObject *WorldContextObject,
    const FString &TargetUserId,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    this->UnblockPlayerCompleteHandle = Friends->AddOnUnblockedPlayerCompleteDelegate_Handle(
        0,
        FOnUnblockedPlayerCompleteDelegate::CreateUObject(
            this,
            &UExampleCPPSubsystem::HandleUnblockPlayerComplete,
            WorldContextObject,
            OnDone));

    if (!Friends->UnblockPlayer(0, *Identity->CreateUniquePlayerId(TargetUserId)))
    {
        OnDone.ExecuteIfBound(false);
    }
}

void UExampleCPPSubsystem::HandleUnblockPlayerComplete(
    int32 LocalUserNum,
    bool bWasSuccessful,
    const FUniqueNetId &UniqueID,
    const FString &ListName,
    const FString &ErrorStr,
    const UObject *WorldContextObject,
    FExampleCPPSubsystemFriendOperationComplete OnDone)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
    if (Subsystem == nullptr)
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
    IOnlineFriendsPtr Friends = Subsystem->GetFriendsInterface();

    Friends->ClearOnUnblockedPlayerCompleteDelegate_Handle(0, this->UnblockPlayerCompleteHandle);

    OnDone.ExecuteIfBound(bWasSuccessful);
}

// StartInviteFriend is inside ExampleCPPSubsystem.Parties.cpp.