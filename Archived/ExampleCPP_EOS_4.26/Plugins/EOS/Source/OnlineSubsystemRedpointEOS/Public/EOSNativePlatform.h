// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "UObject/Interface.h"

#include "EOSNativePlatform.generated.h"

EOS_ENABLE_STRICT_WARNINGS

/**************************************************************
 *
 * ----- DEPRECATION WARNING -----
 *
 * This classes in this header are DEPRECATED and only exist to support the old Monolithic authentication graph. DO NOT
 * implement these classes in new code.
 *
 * If you are attempting to implement an alternate way of authenticating, implement your own authentication graph and
 * register it (refer to the documentation).
 *
 * If you are attempting to support a platform that is not otherwise supported by EOS, implement IEOSRuntimePlatform and
 * set it as the runtime platform in your module startup.
 *
 * ----- DEPRECATION WARNING -----
 *
 **************************************************************/

#ifndef EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH
#define EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH TEXT("authenticatedWith")
#endif

struct ONLINESUBSYSTEMREDPOINTEOS_API FExternalAccountIdInfoLegacy
{
public:
    int32_t AccountIdType;
    FString AccountId;
};

typedef TMap<FString, FString> FUserAuthAttributes;

DECLARE_DELEGATE_ThreeParams(
    FGetCredentialsComplete,
    bool /* bWasSuccessful */,
    FOnlineAccountCredentials /* Creds */,
    FUserAuthAttributes /* UserAuthAttributes */);
DECLARE_DELEGATE_TwoParams(
    FGetFriendsComplete,
    bool /* bWasSuccessful */,
    TArray<TSharedRef<FOnlineFriend>> /* Friends */);
DECLARE_DELEGATE_TwoParams(FGetAvatarComplete, bool /* bWasSuccessful */, UTexture2D * /* Friends */);

UINTERFACE(MinimalAPI) class UEOSNativePlatform : public UInterface
{
    GENERATED_BODY()
};

class ONLINESUBSYSTEMREDPOINTEOS_API IEOSNativePlatform
{
    GENERATED_BODY()

public:
    virtual void LoadModules(){};

    virtual FText GetDisplayName() = 0;

    virtual bool HasCredentials(int32 LocalUserNum) = 0;
    virtual void GetCredentials(int32 LocalUserNum, FGetCredentialsComplete OnComplete) = 0;

    virtual bool HasAvatar(const FUniqueNetId &UserId)
    {
        return false;
    }
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    virtual void GetAvatar(const FUniqueNetId &UserId, FGetAvatarComplete OnComplete)
    {
        OnComplete.ExecuteIfBound(false, nullptr);
    }

    virtual bool CanProvideExternalAccountId(const FUniqueNetId &UserId)
    {
        return false;
    }
    virtual FExternalAccountIdInfoLegacy GetExternalAccountId(const FUniqueNetId &UserId)
    {
        return FExternalAccountIdInfoLegacy();
    }

    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    virtual bool CanRefreshCredentials(FOnlineAccountCredentials PreviousCredentials)
    {
        // If you implement this method, you *MUST* ensure that PreviousCredentials.Type is checked
        // to match the kind of credentials you hand out. CanRefreshCredentials gets called on
        // *all* native platforms when any kind of credential needs to be refreshed.
        return false;
    }
    virtual void GetRefreshCredentials(
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        FOnlineAccountCredentials PreviousCredentials,
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        FGetCredentialsComplete OnComplete)
    {
        OnComplete.ExecuteIfBound(false, FOnlineAccountCredentials(), TMap<FString, FString>());
    }
};

EOS_DISABLE_STRICT_WARNINGS
