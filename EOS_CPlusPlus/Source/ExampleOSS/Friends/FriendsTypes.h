// Copyright June Rhodes. MIT Licensed.

#pragma once

#include "Containers/Set.h"
#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "FriendsTypes.generated.h"

UCLASS(BlueprintType)
class EXAMPLEOSS_API UExampleCPPFriend : public UObject
{
    GENERATED_BODY()

public:
    TSharedPtr<FOnlineFriend> Friend;

    UFUNCTION(BlueprintPure)
    FUniqueNetIdRepl GetUserId()
    {
        return FUniqueNetIdRepl(*Friend->GetUserId());
    }

    UFUNCTION(BlueprintPure)
    FString GetUserIdString()
    {
        return Friend->GetUserId()->ToString();
    }

    UFUNCTION(BlueprintPure)
    FName GetUserIdType()
    {
        return Friend->GetUserId()->GetType();
    }

    UFUNCTION(BlueprintPure)
    FString GetDisplayName()
    {
        return Friend->GetDisplayName();
    }

    UFUNCTION(BlueprintPure)
    FString GetRealName()
    {
        return Friend->GetRealName();
    }

    UFUNCTION(BlueprintPure)
    FString GetWrappedFriendsSummary()
    {
        TArray<FString> Summary;
        FString SubsystemNamesStr;
        if (Friend->GetUserAttribute(TEXT("eosSynthetic.subsystemNames"), SubsystemNamesStr))
        {
            TArray<FString> SubsystemNames;
            SubsystemNamesStr.ParseIntoArray(SubsystemNames, TEXT(","));
            for (const auto &SubsystemName : SubsystemNames)
            {
                FString Id, RealName, DisplayName;
                Friend->GetUserAttribute(FString::Printf(TEXT("eosSynthetic.friend.%s.id"), *SubsystemName), Id);
                Friend->GetUserAttribute(
                    FString::Printf(TEXT("eosSynthetic.friend.%s.realName"), *SubsystemName),
                    RealName);
                Friend->GetUserAttribute(
                    FString::Printf(TEXT("eosSynthetic.friend.%s.displayName"), *SubsystemName),
                    DisplayName);
                Summary.Add(FString::Printf(
                    TEXT("subsystem: %s id: %s real name: %s display name: %s"),
                    *SubsystemName,
                    *Id,
                    *RealName,
                    *DisplayName));
            }
        }
        return FString::Join(Summary, TEXT("\n"));
    }

    UFUNCTION(BlueprintCallable)
    FString ToString()
    {
        if (Friend == nullptr)
        {
            return TEXT("");
        }

        return Friend->GetDisplayName();
    }

    UFUNCTION(BlueprintPure)
    bool CanAcceptInvite()
    {
        if (Friend == nullptr)
        {
            return false;
        }

        return Friend->GetInviteStatus() == EInviteStatus::PendingInbound;
    }

    UFUNCTION(BlueprintPure)
    bool CanDeleteFriend()
    {
        if (Friend == nullptr)
        {
            return false;
        }

        return Friend->GetInviteStatus() == EInviteStatus::PendingOutbound ||
               Friend->GetInviteStatus() == EInviteStatus::Accepted;
    }

    UFUNCTION(BlueprintPure)
    bool CanRejectInvite()
    {
        if (Friend == nullptr)
        {
            return false;
        }

        return Friend->GetInviteStatus() == EInviteStatus::PendingInbound;
    }

    UFUNCTION(BlueprintPure)
    FString GetInviteStatus()
    {
        if (Friend == nullptr)
        {
            return TEXT("Unknown");
        }

        switch (Friend->GetInviteStatus())
        {
        case EInviteStatus::Accepted:
            return TEXT("Accepted");
        case EInviteStatus::Blocked:
            return TEXT("Blocked");
        case EInviteStatus::PendingInbound:
            return TEXT("Pending Inbound");
        case EInviteStatus::PendingOutbound:
            return TEXT("Pending Outbound");
        case EInviteStatus::Suggested:
            return TEXT("Suggested");
        default:
            return TEXT("Unknown");
        }
    }

    UFUNCTION(BlueprintPure)
    FString GetPresenceLastOnline()
    {
        if (Friend == nullptr)
        {
            return TEXT("");
        }

        return Friend->GetPresence().LastOnline.ToString();
    }

    UFUNCTION(BlueprintPure)
    FString GetPresenceSessionID()
    {
        if (Friend == nullptr || !Friend->GetPresence().SessionId.IsValid())
        {
            return TEXT("");
        }

        return Friend->GetPresence().SessionId->ToString();
    }

    UFUNCTION(BlueprintPure)
    FString GetPresenceStatusProperties()
    {
        if (Friend == nullptr)
        {
            return TEXT("");
        }

        TArray<FString> Results;
        for (const auto &Prop : Friend->GetPresence().Status.Properties)
        {
            Results.Add(FString::Printf(TEXT("%s=%s"), *Prop.Key, *Prop.Value.ToString()));
        }
        return FString::Join(Results, TEXT("\n"));
    }

    UFUNCTION(BlueprintPure)
    FString GetPresenceStatusState()
    {
        if (Friend == nullptr)
        {
            return TEXT("");
        }

        switch (Friend->GetPresence().Status.State)
        {
        case EOnlinePresenceState::Away:
            return TEXT("Away");
        case EOnlinePresenceState::Chat:
            return TEXT("Chat");
        case EOnlinePresenceState::DoNotDisturb:
            return TEXT("Do Not Disturb");
        case EOnlinePresenceState::ExtendedAway:
            return TEXT("Extended Away");
        case EOnlinePresenceState::Offline:
            return TEXT("Offline");
        case EOnlinePresenceState::Online:
            return TEXT("Online");
        default:
            return TEXT("Unknown");
        }
    }

    UFUNCTION(BlueprintPure)
    FString GetPresenceStatusStatus()
    {
        if (Friend == nullptr)
        {
            return TEXT("");
        }

        return Friend->GetPresence().Status.StatusStr;
    }

    UFUNCTION(BlueprintPure)
    bool HasVoiceSupport()
    {
        if (Friend == nullptr)
        {
            return false;
        }

        return Friend->GetPresence().bHasVoiceSupport;
    }

    UFUNCTION(BlueprintPure)
    bool IsJoinable()
    {
        if (Friend == nullptr)
        {
            return false;
        }

        return Friend->GetPresence().bIsJoinable;
    }

    UFUNCTION(BlueprintPure)
    bool IsPresenceOnline()
    {
        if (Friend == nullptr)
        {
            return false;
        }

        return Friend->GetPresence().bIsOnline;
    }

    UFUNCTION(BlueprintPure)
    bool IsPresencePlayingThisGame()
    {
        if (Friend == nullptr)
        {
            return false;
        }

        return Friend->GetPresence().bIsPlayingThisGame;
    }

    UFUNCTION(BlueprintPure)
    bool IsPresencePlaying()
    {
        if (Friend == nullptr)
        {
            return false;
        }

        return Friend->GetPresence().bIsPlaying;
    }
};

USTRUCT(BlueprintType)
struct EXAMPLEOSS_API FExampleCPPSimpleUser
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    FString UserId;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
    FString DisplayName;
};