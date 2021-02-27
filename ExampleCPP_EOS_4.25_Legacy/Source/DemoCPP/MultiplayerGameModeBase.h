// Copyright 2020 June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "MultiplayerGameModeBase.generated.h"

/**
 * The OnLogout function on GameModeBase fires after the player controller has already set it's
 * Player and NetConnection to nullptr, which means we can't pull the FUniqueNetIdRepl out during
 * UnregisterPlayer.
 *
 * This game mode base, in conjunction with AMultiplayerPlayerController, allows us to deregister
 * players just before that happens.
 */
UCLASS(BlueprintType)
class DEMOCPP_API AMultiplayerGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    virtual void PreLogout(APlayerController *Exiting);

    UFUNCTION(
        BlueprintImplementableEvent,
        Category = Game,
        meta = (DisplayName = "OnPreLogout", ScriptName = "OnPreLogout"))
    void K2_OnPreLogout(APlayerController *ExitingController);
};
