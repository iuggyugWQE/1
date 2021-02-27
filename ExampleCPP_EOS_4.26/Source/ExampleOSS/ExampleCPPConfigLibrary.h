// Copyright 2020 June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "ExampleCPPConfigLibrary.generated.h"

UCLASS(Config = Game, meta = (DisplayName = "Example Project"))
class EXAMPLEOSS_API UExampleCPPConfig : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /**
     * If true and you are using the unified example project, the example will prefer blueprint tasks over the C++
     * tasks.
     */
    UPROPERTY(EditAnywhere, Config, Category = "Example")
    bool bPreferBlueprintsImplementation;
};

UCLASS(BlueprintType)
class EXAMPLEOSS_API UExampleCPPConfigLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    static bool GetPreferBlueprintsImplementation();

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static FString GetAuthMode(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static void SetAuthMode(const UObject *WorldContextObject, FString AuthMode);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static bool GetPersistentLoginEnabled(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static void SetPersistentLoginEnabled(const UObject *WorldContextObject, bool EnablePersistentLogin);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static bool GetDeviceIdEnabled(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static void SetDeviceIdEnabled(const UObject *WorldContextObject, bool EnableDeviceIds);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static bool GetDeleteDeviceIdEnabled(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static void SetDeleteDeviceIdEnabled(const UObject *WorldContextObject, bool EnableDeleteDeviceId);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static bool GetIsSteamEnabled(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static void SetIsSteamEnabled(const UObject *WorldContextObject, bool EnableSteam);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static FString GetSteamAppId(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static void SetSteamAppId(const UObject *WorldContextObject, FString InAppId);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static TArray<FString> GetDelegatedSubsystems(const UObject *WorldContextObject);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static void SetDelegatedSubsystems(const UObject *WorldContextObject, TArray<FString> InSubsystemNames);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
    static bool GetAreBeaconsSupported(const UObject *WorldContextObject);
};
