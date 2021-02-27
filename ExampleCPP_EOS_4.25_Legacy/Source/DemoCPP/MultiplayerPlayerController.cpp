// Copyright 2020 June Rhodes. All Rights Reserved.

#include "MultiplayerPlayerController.h"

#include "MultiplayerGameModeBase.h"

void AMultiplayerPlayerController::OnNetCleanup(UNetConnection *Connection)
{
    if (GetLocalRole() == ROLE_Authority && PlayerState != NULL)
    {
        AMultiplayerGameModeBase *GameMode = Cast<AMultiplayerGameModeBase>(GetWorld()->GetAuthGameMode());
        if (GameMode)
        {
            GameMode->PreLogout(this);
        }
    }

    Super::OnNetCleanup(Connection);
}