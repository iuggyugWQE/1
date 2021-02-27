// Copyright 2020 June Rhodes. All Rights Reserved.

#include "DemoConfigLibrary.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

FString UDemoConfigLibrary::GetAuthMode(const UObject *WorldContextObject)
{
    FString Value = TEXT("EASRequired");
    GConfig->GetString(TEXT("EpicOnlineServices"), TEXT("AuthenticationBehaviour"), Value, GEngineIni);
    return Value;
}

void UDemoConfigLibrary::SetAuthMode(const UObject *WorldContextObject, FString AuthMode)
{
    GConfig->SetString(TEXT("EpicOnlineServices"), TEXT("AuthenticationBehaviour"), *AuthMode, GEngineIni);
    GConfig->Flush(false, GEngineIni);
}

bool UDemoConfigLibrary::GetPersistentLoginEnabled(const UObject *WorldContextObject)
{
    bool Value = true;
    GConfig->GetBool(TEXT("EpicOnlineServices"), TEXT("DisablePersistentLogin"), Value, GEngineIni);
    return !Value;
}

void UDemoConfigLibrary::SetPersistentLoginEnabled(const UObject *WorldContextObject, bool EnablePersistentLogin)
{
    GConfig->SetBool(TEXT("EpicOnlineServices"), TEXT("DisablePersistentLogin"), !EnablePersistentLogin, GEngineIni);
    GConfig->Flush(false, GEngineIni);
}

bool UDemoConfigLibrary::GetDeviceIdEnabled(const UObject *WorldContextObject)
{
    bool Value = true;
    GConfig->GetBool(TEXT("EpicOnlineServices"), TEXT("AllowDeviceIdAccounts"), Value, GEngineIni);
    return Value;
}

void UDemoConfigLibrary::SetDeviceIdEnabled(const UObject *WorldContextObject, bool EnableDeviceIds)
{
    GConfig->SetBool(TEXT("EpicOnlineServices"), TEXT("AllowDeviceIdAccounts"), EnableDeviceIds, GEngineIni);
    GConfig->Flush(false, GEngineIni);
}

bool UDemoConfigLibrary::GetDeleteDeviceIdEnabled(const UObject *WorldContextObject)
{
    bool Value = true;
    GConfig->GetBool(TEXT("EpicOnlineServices"), TEXT("DeleteDeviceIdOnLogout"), Value, GEngineIni);
    return Value;
}

void UDemoConfigLibrary::SetDeleteDeviceIdEnabled(const UObject *WorldContextObject, bool EnableDeleteDeviceId)
{
    GConfig->SetBool(TEXT("EpicOnlineServices"), TEXT("DeleteDeviceIdOnLogout"), EnableDeleteDeviceId, GEngineIni);
    GConfig->Flush(false, GEngineIni);
}

bool UDemoConfigLibrary::GetIsSteamEnabled(const UObject *WorldContextObject)
{
    bool Value = true;
    GConfig->GetBool(TEXT("OnlineSubsystemSteam"), TEXT("bEnabled"), Value, GEngineIni);
    return Value;
}

void UDemoConfigLibrary::SetIsSteamEnabled(const UObject *WorldContextObject, bool EnableSteam)
{
    GConfig->SetBool(TEXT("OnlineSubsystemSteam"), TEXT("bEnabled"), EnableSteam, GEngineIni);
    GConfig->Flush(false, GEngineIni);
}

FString UDemoConfigLibrary::GetSteamAppId(const UObject *WorldContextObject)
{
    FString Value = TEXT("");
    GConfig->GetString(TEXT("OnlineSubsystemSteam"), TEXT("SteamDevAppId"), Value, GEngineIni);
    return Value;
}

void UDemoConfigLibrary::SetSteamAppId(const UObject *WorldContextObject, FString InAppId)
{
    GConfig->SetString(TEXT("OnlineSubsystemSteam"), TEXT("SteamDevAppId"), *InAppId, GEngineIni);
    GConfig->Flush(false, GEngineIni);
}

TArray<FString> UDemoConfigLibrary::GetDelegatedSubsystems(const UObject *WorldContextObject)
{
    FString Value = TEXT("");
    GConfig->GetString(TEXT("EpicOnlineServices"), TEXT("DelegatedSubsystems"), Value, GEngineIni);

    TArray<FString> Results;
    Value.ParseIntoArray(Results, TEXT(","), true);

    return Results;
}

void UDemoConfigLibrary::SetDelegatedSubsystems(const UObject *WorldContextObject, TArray<FString> InSubsystemNames)
{
    FString NewValue = FString::Join(InSubsystemNames, TEXT(","));
    GConfig->SetString(TEXT("EpicOnlineServices"), TEXT("DelegatedSubsystems"), *NewValue, GEngineIni);
    GConfig->Flush(false, GEngineIni);
}