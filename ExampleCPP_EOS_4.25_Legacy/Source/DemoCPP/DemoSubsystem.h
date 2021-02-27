// Copyright 2020 June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Set.h"
#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DemoSubsystem.generated.h"

/**
 * This demo project exposes some blueprint nodes so they're accessible from UMG. It's not meant to be an example of how
 * you can access the online subsystem from blueprints.
 *
 * If you're interested in using the online subsystem from blueprints, see the "DemoBlueprint" project instead.
 */

UCLASS(BlueprintType)
class DEMOCPP_API UDemoSessionSearchResult : public UObject
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
class DEMOCPP_API UDemoPartyId : public UObject
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
class DEMOCPP_API UDemoPartyMemberId : public UObject
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
class DEMOCPP_API UDemoPartyInvite : public UObject
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
class DEMOCPP_API UDemoFriend : public UObject
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
struct DEMOCPP_API FDemoStat
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString Id;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int CurrentValue;
};

USTRUCT(BlueprintType)
struct DEMOCPP_API FDemoAchievement
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
struct DEMOCPP_API FDemoLeaderboardEntry
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

DECLARE_DYNAMIC_DELEGATE_TwoParams(FDemoBPLLoginComplete, bool, WasSuccessful, FString, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoBPLLogoutComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_TwoParams(
    FDemoSubsystemFindSessionsComplete,
    bool,
    WasSuccessful,
    const TArray<UDemoSessionSearchResult *> &,
    Results);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemCreateSessionComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemDestroySessionComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemJoinSessionComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemUpdatePresenceComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemCreatePartyComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemJoinPartyComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemLeavePartyComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemKickMemberComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemReadFriendsComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemInviteFriendComplete, bool, WasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDemoSubsystemInvitationsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDemoSubsystemPartyLeaderWasFollowedToSession, FName, SessionName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDemoSubsystemSessionJoinedMapTravelRequired, FName, SessionName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemQueryUserInfoComplete, FString, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemQueryUserIdMappingComplete, FString, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemQueryExternalIdMappingsComplete, FString, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(
    FDemoSubsystemQueryStatsComplete,
    bool,
    bWasSuccessful,
    const TArray<FDemoStat> &,
    QueriedStats);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDemoSubsystemIngestStatComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_DELEGATE_TwoParams(
    FDemoSubsystemQueryAchievementsComplete,
    bool,
    bWasSuccessful,
    const TArray<FDemoAchievement> &,
    Achievements);
DECLARE_DYNAMIC_DELEGATE_TwoParams(
    FDemoSubsystemQueryLeaderboardsComplete,
    bool,
    bWasSuccessful,
    const TArray<FDemoLeaderboardEntry> &,
    LeaderboardEntries);

UCLASS(BlueprintType)
class DEMOCPP_API UDemoSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    TSet<FString> SessionFollowConsumedIds;

public:
    virtual void PostInitProperties() override;

    void OnPartyDataReceived(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
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
    FDemoSubsystemPartyLeaderWasFollowedToSession PartyLeaderWasFollowedToSession;

    UPROPERTY(BlueprintAssignable)
    FDemoSubsystemSessionJoinedMapTravelRequired SessionJoinedMapTravelRequired;

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    bool IsSignedIn(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetLoggedInUsername(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetLoggedInProductUserId(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetLoggedInEpicAccountId(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartLogin(const UObject *WorldContextObject, int32 LocalUserNum, FDemoBPLLoginComplete OnDone);

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
        FDemoBPLLoginComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartLogout(const UObject *WorldContextObject, FDemoBPLLogoutComplete OnDone);

private:
    FDelegateHandle LogoutDelegateHandle;
    void HandleLogoutComplete(
        int32 _UnusedLocalUserNum,
        bool bWasSuccessful,
        const UObject *WorldContextObject,
        FDemoBPLLogoutComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartCreateSession(const UObject *WorldContextObject, FDemoSubsystemCreateSessionComplete OnDone);

private:
    FDelegateHandle CreateSessionDelegateHandle;
    void HandleCreateSessionComplete(
        FName SessionName,
        bool bWasSuccessful,
        const UObject *WorldContextObject,
        FDemoSubsystemCreateSessionComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartFindSessions(const UObject *WorldContextObject, FDemoSubsystemFindSessionsComplete OnDone);

private:
    FDelegateHandle FindSessionsDelegateHandle;
    void HandleFindSessionsComplete(
        bool bWasSuccessful,
        const UObject *WorldContextObject,
        FDemoSubsystemFindSessionsComplete OnDone,
        TSharedRef<FOnlineSessionSearch> Search);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetCurrentSessionId(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    TArray<FString> GetPlayersInSession(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void RegisterPlayer(APlayerController *InPlayerController);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void UnregisterPlayer(APlayerController *InPlayerController);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartDestroySession(
        const UObject *WorldContextObject,
        FName SessionName,
        FDemoSubsystemDestroySessionComplete OnDone);

private:
    FDelegateHandle DestroySessionDelegateHandle;
    void HandleDestroySessionComplete(
        FName SessionName,
        bool bWasSuccessful,
        const UObject *WorldContextObject,
        FDemoSubsystemDestroySessionComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartJoinSession(
        const UObject *WorldContextObject,
        UDemoSessionSearchResult *SearchResult,
        FDemoSubsystemJoinSessionComplete OnDone);

private:
    FDelegateHandle JoinSessionDelegateHandle;
    void HandleJoinSessionComplete(
        FName SessionName,
        EOnJoinSessionCompleteResult::Type JoinResult,
        const UObject *WorldContextObject,
        FDemoSubsystemJoinSessionComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetResolvedConnectString(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    FString GetResolvedConnectStringForSession(const UObject *WorldContextObject, FName SessionName);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartUpdatePresence(
        const UObject *WorldContextObject,
        const FString &NewPresenceString,
        FDemoSubsystemUpdatePresenceComplete OnDone);

private:
    void HandleUpdatePresenceComplete(
        const class FUniqueNetId &UserId,
        const bool bWasSuccessful,
        const UObject *WorldContextObject,
        FDemoSubsystemUpdatePresenceComplete OnDone);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    TArray<UDemoPartyId *> GetJoinedParties(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void RequestPartyMembersFollowLeader(const UObject *WorldContextObject, FName SessionName);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartCreateParty(const UObject *WorldContextObject, int PartyTypeId, FDemoSubsystemCreatePartyComplete OnDone);

private:
    void HandleCreatePartyComplete(
        const FUniqueNetId &LocalUserId,
        const TSharedPtr<const FOnlinePartyId> &PartyId,
        const ECreatePartyCompletionResult Result,
        const UObject *WorldContextObject,
        FDemoSubsystemCreatePartyComplete OnDone);

public:
    UPROPERTY(BlueprintAssignable)
    FDemoSubsystemInvitationsChanged OnInvitationsChanged;

private:
    void OnPartyInvitesChanged(const FUniqueNetId &LocalUserId);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    TArray<UDemoPartyInvite *> GetInvitations(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    TArray<UDemoPartyMemberId *> GetPartyMembers(const UObject *WorldContextObject, UDemoPartyId *PartyId);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    bool IsPartyLeader(const UObject *WorldContextObject, UDemoPartyId *PartyId);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartJoinParty(
        const UObject *WorldContextObject,
        UDemoPartyInvite *Invite,
        FDemoSubsystemJoinPartyComplete OnDone);

private:
    void HandleJoinPartyComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const EJoinPartyCompletionResult Result,
        const int32 NotApprovedReason,
        const UObject *WorldContextObject,
        FDemoSubsystemJoinPartyComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartLeaveParty(
        const UObject *WorldContextObject,
        UDemoPartyId *PartyId,
        FDemoSubsystemLeavePartyComplete OnDone);

private:
    void HandleLeavePartyComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const ELeavePartyCompletionResult Result,
        const UObject *WorldContextObject,
        FDemoSubsystemLeavePartyComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartKickMember(
        const UObject *WorldContextObject,
        UDemoPartyId *PartyId,
        UDemoPartyMemberId *MemberId,
        FDemoSubsystemKickMemberComplete OnDone);

private:
    void HandleKickMemberComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const EKickMemberCompletionResult Result,
        const UObject *WorldContextObject,
        FDemoSubsystemKickMemberComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    TArray<UDemoFriend *> GetFriends(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartReadFriends(const UObject *WorldContextObject, FDemoSubsystemReadFriendsComplete OnDone);

private:
    void HandleReadFriendsComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FString &ListName,
        const FString &ErrorStr,
        const UObject *WorldContextObject,
        FDemoSubsystemReadFriendsComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void StartInviteFriend(
        const UObject *WorldContextObject,
        UDemoPartyId *PartyId,
        UDemoFriend *Friend,
        FDemoSubsystemInviteFriendComplete OnDone);

private:
    void HandleInviteFriendComplete(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &RecipientId,
        const ESendPartyInvitationCompletionResult Result,
        const UObject *WorldContextObject,
        FDemoSubsystemInviteFriendComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryUserInfo(
        const UObject *WorldContextObject,
        FString ProductUserIdInput,
        FString EpicAccountIdInput,
        FDemoSubsystemQueryUserInfoComplete OnDone);

private:
    FDelegateHandle QueryUserInfoDelegateHandle;
    void HandleQueryUserInfoComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const TArray<TSharedRef<const FUniqueNetId>> &UserIds,
        const FString &ErrorStr,
        const UObject *WorldContextObject,
        FDemoSubsystemQueryUserInfoComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryUserIdMapping(
        const UObject *WorldContextObject,
        FString DisplayNameInput,
        FDemoSubsystemQueryUserIdMappingComplete OnDone);

private:
    void HandleQueryUserMappingComplete(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &DisplayNameOrEmail,
        const FUniqueNetId &FoundUserId,
        const FString &Error,
        const UObject *WorldContextObject,
        FDemoSubsystemQueryUserIdMappingComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryExternalIdMappings(
        const UObject *WorldContextObject,
        FString PlatformType,
        FString ExternalIds,
        FDemoSubsystemQueryExternalIdMappingsComplete OnDone);

private:
    void HandleQueryExternalIdMappings(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FExternalIdQueryOptions &QueryOptions,
        const TArray<FString> &ExternalIds,
        const FString &Error,
        const UObject *WorldContextObject,
        FDemoSubsystemQueryExternalIdMappingsComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryStats(const UObject *WorldContextObject, FDemoSubsystemQueryStatsComplete OnDone);

private:
    void HandleQueryStats(
        const FOnlineError &ResultState,
        const TArray<TSharedRef<const FOnlineStatsUserStats>> &UsersStatsResult,
        const UObject *WorldContextObject,
        FDemoSubsystemQueryStatsComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void IngestStat(
        const UObject *WorldContextObject,
        FString StatName,
        int32 IngestAmount,
        FDemoSubsystemIngestStatComplete OnDone);

private:
    void HandleIngestStat(
        const FOnlineError &ResultState,
        const UObject *WorldContextObject,
        FDemoSubsystemIngestStatComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryAchievements(const UObject *WorldContextObject, FDemoSubsystemQueryAchievementsComplete OnDone);

private:
    void HandleQueryAchievementDescriptions(
        const FUniqueNetId &UserId,
        const bool bWasSuccessful,
        const UObject *WorldContextObject,
        FDemoSubsystemQueryAchievementsComplete OnDone);
    void HandleQueryAchievements(
        const FUniqueNetId &UserId,
        const bool bWasSuccessful,
        const UObject *WorldContextObject,
        FDemoSubsystemQueryAchievementsComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryFriendsLeaderboards(const UObject *WorldContextObject, FDemoSubsystemQueryLeaderboardsComplete OnDone);
    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    void QueryGlobalLeaderboards(
        const UObject *WorldContextObject,
        const FString &LeaderboardId,
        FDemoSubsystemQueryLeaderboardsComplete OnDone);

private:
    FDelegateHandle QueryLeaderboardsDelegateHandle;
    void HandleLeaderboardResult(
        const bool bWasSuccessful,
        FOnlineLeaderboardReadRef LeaderboardRef,
        const UObject *WorldContextObject,
        FDemoSubsystemQueryLeaderboardsComplete OnDone);

public:
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "WorldContextObject"))
    void SeamlessTravel(const UObject *WorldContextObject);
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, meta = (WorldContext = "WorldContextObject"))
    void NonSeamlessTravel(const UObject *WorldContextObject);
};
