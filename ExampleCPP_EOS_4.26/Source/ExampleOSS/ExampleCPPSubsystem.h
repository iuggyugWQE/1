// Copyright 2020 June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Set.h"
#include "CoreMinimal.h"
#include "ExampleCPPBeacon.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ExampleCPPSubsystem.generated.h"

/**
 * This demo project exposes some blueprint nodes so they're accessible from UMG. It's not meant to be an example of how
 * you can access the online subsystem from blueprints.
 *
 * If you're interested in using the online subsystem from blueprints, see the "DemoBlueprint" project instead.
 */

UCLASS(BlueprintType)
class EXAMPLEOSS_API UExampleCPPSessionSearchResult : public UObject
{
    GENERATED_BODY()

public:
    FOnlineSessionSearchResult Result;

    UPROPERTY(BlueprintReadOnly)
    FString SessionId;

    UPROPERTY(BlueprintReadOnly)
    FString ConnectionString;
};

UCLASS(BlueprintType)
class EXAMPLEOSS_API UExampleCPPPartyId : public UObject
{
    GENERATED_BODY()

public:
    TSharedPtr<const FOnlinePartyId> PartyId;

    UFUNCTION(BlueprintCallable)
    FString ToString()
    {
        if (PartyId == nullptr)
        {
            return TEXT("");
        }

        return PartyId->ToString();
    }
};

UCLASS(BlueprintType)
class EXAMPLEOSS_API UExampleCPPPartyMemberId : public UObject
{
    GENERATED_BODY()

public:
    FOnlinePartyMemberConstPtr PartyMember;

    UFUNCTION(BlueprintCallable)
    FString ToString()
    {
        if (PartyMember == nullptr)
        {
            return TEXT("");
        }

        return PartyMember->GetUserId()->ToString();
    }
};

UCLASS(BlueprintType)
class EXAMPLEOSS_API UExampleCPPPartyInvite : public UObject
{
    GENERATED_BODY()

public:
    IOnlinePartyJoinInfoConstPtr PartyInvite;

    UFUNCTION(BlueprintCallable)
    FString ToString()
    {
        if (PartyInvite == nullptr)
        {
            return TEXT("");
        }

        return PartyInvite->GetPartyId()->ToString();
    }
};

UCLASS(BlueprintType)
class EXAMPLEOSS_API UExampleCPPFriend : public UObject
{
    GENERATED_BODY()

public:
    TSharedPtr<FOnlineFriend> Friend;

    UFUNCTION(BlueprintCallable)
    FString ToString()
    {
        if (Friend == nullptr)
        {
            return TEXT("");
        }

        return Friend->GetDisplayName();
    }
};

USTRUCT(BlueprintType)
struct EXAMPLEOSS_API FExampleCPPStat
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString Id;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int CurrentValue;
};

USTRUCT(BlueprintType)
struct EXAMPLEOSS_API FExampleCPPAchievement
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString Id;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FText Description;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bUnlocked;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Progress;
};

USTRUCT(BlueprintType)
struct EXAMPLEOSS_API FExampleCPPLeaderboardEntry
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int Rank;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString PlayerId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString PlayerNickname;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int Score;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FExampleCPPSubsystemLoginComplete, bool, WasSuccessful, FString, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemLogoutComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_TwoParams(
    FExampleCPPSubsystemFindSessionsComplete,
    bool,
    WasSuccessful,
    const TArray<UExampleCPPSessionSearchResult *> &,
    Results);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemCreateSessionComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemDestroySessionComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemJoinSessionComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemUpdatePresenceComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemCreatePartyComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemJoinPartyComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemLeavePartyComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemKickMemberComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemReadFriendsComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemInviteFriendComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExampleCPPSubsystemInvitationsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExampleCPPSubsystemPartyLeaderWasFollowedToSession, FName, SessionName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExampleCPPSubsystemSessionJoinedMapTravelRequired, FName, SessionName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemQueryUserInfoComplete, FString, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemQueryUserIdMappingComplete, FString, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemQueryExternalIdMappingsComplete, FString, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(
    FExampleCPPSubsystemQueryStatsComplete,
    bool,
    bWasSuccessful,
    const TArray<FExampleCPPStat> &,
    QueriedStats);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemIngestStatComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_DELEGATE_TwoParams(
    FExampleCPPSubsystemQueryAchievementsComplete,
    bool,
    bWasSuccessful,
    const TArray<FExampleCPPAchievement> &,
    Achievements);
DECLARE_DYNAMIC_DELEGATE_TwoParams(
    FExampleCPPSubsystemQueryLeaderboardsComplete,
    bool,
    bWasSuccessful,
    const TArray<FExampleCPPLeaderboardEntry> &,
    LeaderboardEntries);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExampleCPPSubsystemQueryPresenceComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FExampleCPPSubsystemPresenceUpdated, FString, UserId, FString, Status);

UCLASS(BlueprintType)
class EXAMPLEOSS_API UExampleCPPSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    TSet<FString> SessionFollowConsumedIds;

public:
    virtual void PostInitProperties() override;

    void OnPartyDataReceived(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData);
    void OnFindSessionForLeaderFollow(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FOnlineSessionSearchResult &SearchResult,
        FString SessionFollowString);
    FName JoinSessionForLeaderSessionName;
    FDelegateHandle JoinSessionForLeaderFollowDelegateHandle;
    void OnJoinSessionForLeaderFollow(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

    void OnSessionUserInviteAccepted(
        const bool bWasSuccessful,
        const int32 LocalUserNum,
        TSharedPtr<const FUniqueNetId> UserId,
        const FOnlineSessionSearchResult &InviteResult);
    void OnSessionJoinedViaOverlay(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);
    FName JoinSessionForInviteSessionName;
    FDelegateHandle JoinSessionForInviteDelegateHandle;
    void OnJoinSessionForInviteFollow(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

    UPROPERTY(BlueprintAssignable)
    FExampleCPPSubsystemPartyLeaderWasFollowedToSession PartyLeaderWasFollowedToSession;

    UPROPERTY(BlueprintAssignable)
    FExampleCPPSubsystemSessionJoinedMapTravelRequired SessionJoinedMapTravelRequired;

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    bool IsSignedIn(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetLoggedInUsername(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetLoggedInProductUserId(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetLoggedInEpicAccountId(const UObject *WorldContextObject);

    UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
    FString GetLoggedInAuthAttribute(const UObject *WorldContextObject, const FString &InAuthAttrName);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartLogin(const UObject *WorldContextObject, int32 LocalUserNum, FExampleCPPSubsystemLoginComplete OnDone);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    bool CanLinkToEpicGamesAccount(const UObject *WorldContextObject);

private:
    FDelegateHandle LoginDelegateHandle;
    void HandleLoginComplete(
        int32 _UnusedLocalUserNum,
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &Error,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemLoginComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartLogout(const UObject *WorldContextObject, FExampleCPPSubsystemLogoutComplete OnDone);

private:
    FDelegateHandle LogoutDelegateHandle;
    void HandleLogoutComplete(
        int32 _UnusedLocalUserNum,
        bool bWasSuccessful,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemLogoutComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartCreateSession(const UObject *WorldContextObject, FExampleCPPSubsystemCreateSessionComplete OnDone);

private:
    FDelegateHandle CreateSessionDelegateHandle;
    void HandleCreateSessionComplete(
        FName SessionName,
        bool bWasSuccessful,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemCreateSessionComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartFindSessions(const UObject *WorldContextObject, FExampleCPPSubsystemFindSessionsComplete OnDone);

private:
    FDelegateHandle FindSessionsDelegateHandle;
    void HandleFindSessionsComplete(
        bool bWasSuccessful,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemFindSessionsComplete OnDone,
        TSharedRef<FOnlineSessionSearch> Search);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetCurrentSessionId(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    TArray<FString> GetPlayersInSession(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void RegisterExistingPlayers(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void RegisterPlayer(APlayerController *InPlayerController);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void UnregisterPlayer(APlayerController *InPlayerController);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartDestroySession(
        const UObject *WorldContextObject,
        FName SessionName,
        FExampleCPPSubsystemDestroySessionComplete OnDone);

private:
    FDelegateHandle DestroySessionDelegateHandle;
    void HandleDestroySessionComplete(
        FName SessionName,
        bool bWasSuccessful,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemDestroySessionComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartJoinSession(
        const UObject *WorldContextObject,
        UExampleCPPSessionSearchResult *SearchResult,
        FExampleCPPSubsystemJoinSessionComplete OnDone);

private:
    FDelegateHandle JoinSessionDelegateHandle;
    void HandleJoinSessionComplete(
        FName SessionName,
        EOnJoinSessionCompleteResult::Type JoinResult,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemJoinSessionComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void SendBeaconPingToSearchResult(
        const UObject *WorldContextObject,
        UExampleCPPSessionSearchResult *SearchResult,
        FExampleCPPSubsystemBeaconPingComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetResolvedConnectString(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetResolvedConnectStringForSession(const UObject *WorldContextObject, FName SessionName);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartUpdatePresence(
        const UObject *WorldContextObject,
        const FString &NewPresenceString,
        FExampleCPPSubsystemUpdatePresenceComplete OnDone);

private:
    void HandleUpdatePresenceComplete(
        const class FUniqueNetId &UserId,
        const bool bWasSuccessful,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemUpdatePresenceComplete OnDone);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    TArray<UExampleCPPPartyId *> GetJoinedParties(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void RequestPartyMembersFollowLeader(const UObject *WorldContextObject, FName SessionName);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartCreateParty(
        const UObject *WorldContextObject,
        int PartyTypeId,
        FExampleCPPSubsystemCreatePartyComplete OnDone);

private:
    void HandleCreatePartyComplete(
        const FUniqueNetId &LocalUserId,
        const TSharedPtr<const FOnlinePartyId> &PartyId,
        const ECreatePartyCompletionResult Result,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemCreatePartyComplete OnDone);

public:
    UPROPERTY(BlueprintAssignable)
    FExampleCPPSubsystemInvitationsChanged OnInvitationsChanged;

private:
    void OnPartyInvitesChanged(const FUniqueNetId &LocalUserId);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    TArray<UExampleCPPPartyInvite *> GetInvitations(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    TArray<UExampleCPPPartyMemberId *> GetPartyMembers(const UObject *WorldContextObject, UExampleCPPPartyId *PartyId);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    bool IsPartyLeader(const UObject *WorldContextObject, UExampleCPPPartyId *PartyId);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartJoinParty(
        const UObject *WorldContextObject,
        UExampleCPPPartyInvite *Invite,
        FExampleCPPSubsystemJoinPartyComplete OnDone);

private:
    void HandleJoinPartyComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const EJoinPartyCompletionResult Result,
        const int32 NotApprovedReason,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemJoinPartyComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartLeaveParty(
        const UObject *WorldContextObject,
        UExampleCPPPartyId *PartyId,
        FExampleCPPSubsystemLeavePartyComplete OnDone);

private:
    void HandleLeavePartyComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const ELeavePartyCompletionResult Result,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemLeavePartyComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartKickMember(
        const UObject *WorldContextObject,
        UExampleCPPPartyId *PartyId,
        UExampleCPPPartyMemberId *MemberId,
        FExampleCPPSubsystemKickMemberComplete OnDone);

private:
    void HandleKickMemberComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const EKickMemberCompletionResult Result,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemKickMemberComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    TArray<UExampleCPPFriend *> GetFriends(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartReadFriends(const UObject *WorldContextObject, FExampleCPPSubsystemReadFriendsComplete OnDone);

private:
    void HandleReadFriendsComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FString &ListName,
        const FString &ErrorStr,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemReadFriendsComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartInviteFriend(
        const UObject *WorldContextObject,
        UExampleCPPPartyId *PartyId,
        UExampleCPPFriend *Friend,
        FExampleCPPSubsystemInviteFriendComplete OnDone);

private:
    void HandleInviteFriendComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &RecipientId,
        const ESendPartyInvitationCompletionResult Result,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemInviteFriendComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryUserInfo(
        const UObject *WorldContextObject,
        FString ProductUserIdInput,
        FString EpicAccountIdInput,
        FExampleCPPSubsystemQueryUserInfoComplete OnDone);

private:
    FDelegateHandle QueryUserInfoDelegateHandle;
    void HandleQueryUserInfoComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const TArray<TSharedRef<const FUniqueNetId>> &UserIds,
        const FString &ErrorStr,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemQueryUserInfoComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryUserIdMapping(
        const UObject *WorldContextObject,
        FString DisplayNameInput,
        FExampleCPPSubsystemQueryUserIdMappingComplete OnDone);

private:
    void HandleQueryUserMappingComplete(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &DisplayNameOrEmail,
        const FUniqueNetId &FoundUserId,
        const FString &Error,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemQueryUserIdMappingComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryExternalIdMappings(
        const UObject *WorldContextObject,
        FString PlatformType,
        FString ExternalIds,
        FExampleCPPSubsystemQueryExternalIdMappingsComplete OnDone);

private:
    void HandleQueryExternalIdMappings(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FExternalIdQueryOptions &QueryOptions,
        const TArray<FString> &ExternalIds,
        const FString &Error,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemQueryExternalIdMappingsComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryStats(const UObject *WorldContextObject, FExampleCPPSubsystemQueryStatsComplete OnDone);

private:
    void HandleQueryStats(
        const FOnlineError &ResultState,
        const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemQueryStatsComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void IngestStat(
        const UObject *WorldContextObject,
        FString StatName,
        int32 IngestAmount,
        FExampleCPPSubsystemIngestStatComplete OnDone);

private:
    void HandleIngestStat(
        const FOnlineError &ResultState,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemIngestStatComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryAchievements(const UObject *WorldContextObject, FExampleCPPSubsystemQueryAchievementsComplete OnDone);

private:
    void HandleQueryAchievementDescriptions(
        const FUniqueNetId &UserId,
        const bool bWasSuccessful,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemQueryAchievementsComplete OnDone);
    void HandleQueryAchievements(
        const FUniqueNetId &UserId,
        const bool bWasSuccessful,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemQueryAchievementsComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryFriendsLeaderboards(
        const UObject *WorldContextObject,
        FExampleCPPSubsystemQueryLeaderboardsComplete OnDone);
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryGlobalLeaderboards(
        const UObject *WorldContextObject,
        const FString &LeaderboardId,
        FExampleCPPSubsystemQueryLeaderboardsComplete OnDone);

private:
    FDelegateHandle QueryLeaderboardsDelegateHandle;
    void HandleLeaderboardResult(
        const bool bWasSuccessful,
        FOnlineLeaderboardReadRef LeaderboardRef,
        const UObject *WorldContextObject,
        FExampleCPPSubsystemQueryLeaderboardsComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "WorldContextObject"))
    void SeamlessTravel(const UObject *WorldContextObject);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "WorldContextObject"))
    void NonSeamlessTravel(const UObject *WorldContextObject);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryPresence(const UObject *WorldContextObject, FExampleCPPSubsystemQueryPresenceComplete OnDone);

    UPROPERTY(BlueprintAssignable)
    FExampleCPPSubsystemPresenceUpdated PresenceUpdated;

private:
    void HandleQueryPresenceComplete(const class FUniqueNetId &UserId, const bool bWasSuccessful);

    void OnPresenceReceived(const class FUniqueNetId &UserId, const TSharedRef<FOnlineUserPresence> &Presence);
};
