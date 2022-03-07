// Copyright June Rhodes. MIT Licensed.

#include "../ExampleCPPSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "OnlineSubsystemUtils.h"

void UExampleCPPSubsystem::ShowFriendsUI(const UObject *WorldContextObject)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface();

    if (ExternalUI.IsValid())
    {
        ExternalUI->ShowFriendsUI(0);
    }
}

void UExampleCPPSubsystem::ShowInviteUI(const UObject *WorldContextObject, FName SessionName)
{
    IOnlineSubsystem *Subsystem = Online::GetSubsystem(this->GetWorld());
    check(Subsystem != nullptr);
    IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface();

    if (ExternalUI.IsValid())
    {
        ExternalUI->ShowInviteUI(0, SessionName);
    }
}