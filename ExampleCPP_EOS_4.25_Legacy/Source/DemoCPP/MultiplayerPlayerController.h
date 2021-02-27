// Copyright 2020 June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerController.h"

#include "MultiplayerPlayerController.generated.h"

/**
 * The OnLogout function on GameModeBase fires after the player controller has already set it's
 * Player and NetConnection to nullptr, which means we can't pull the FUniqueNetIdRepl out during
 * UnregisterPlayer.
 *
 * This player controller, in conjunction with UMultiplayerGameModeBase, allows us to deregister
 * players just before that happens.
 */
UCLASS(BlueprintType)
class DEMOCPP_API AMultiplayerPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void OnNetCleanup(UNetConnection *Connection) override;
};