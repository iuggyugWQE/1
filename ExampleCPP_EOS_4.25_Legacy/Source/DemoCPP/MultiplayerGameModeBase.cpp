// Copyright 2020 June Rhodes. All Rights Reserved.

#include "MultiplayerGameModeBase.h"

void AMultiplayerGameModeBase::PreLogout(APlayerController *Exiting)
{
    this->K2_OnPreLogout(Exiting);
}