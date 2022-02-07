// Copyright June Rhodes. MIT Licensed.

#include "ClangTidyExampleGameModeBase.h"

#include "Engine/DemoNetDriver.h"

void ABadDestroyTest::BeginDestroy()
{
    // This will warn about a missing call to Super::BeginDestroy().
}

// This will warn because there's no Super::GetLifetimeReplicatedProps.
void AClangTidyExampleGameModeBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
}

// This will warn you that MyStr could be 'const FString&' instead to prevent unnecessary copies.
void AClangTidyExampleGameModeBase::SomeBasicFunction(FString MyStr)
{
    if (MyStr.IsEmpty())
    {
    }
}

void AClangTidyExampleGameModeBase::MyMoveFunction(FString MyStr)
{
    // This will warn you that you can use MoveTemp(MyStr) to move it instead of doing an unnecessary copy.
    FString UsuallyThisPointerStore = MyStr;

    TArray<FString> Arr;
    // This will warn because you're copying a non-trivially copyable type on each iteration of the for-range loop.
    for (auto Val : Arr)
    {
        Arr.Remove(Val);
    }
}

void AClangTidyExampleGameModeBase::TestFunction()
{
    TArray<FString> Arr;
    for (const auto &Val : Arr)
    {
        // This will warn because you're passing a reference from an array directly into one of the array's mutating
        // functions like Remove.
        Arr.Remove(Val);
    }

    // This will warn about using IOnlineSubsystem::Get(), which isn't safe in the editor. You should use
    // Online::GetSubsystem(this->GetWorld()) instead.
    IOnlineSubsystem::Get();

    IMyInter *Inter = nullptr;
    Inter->ReactToTrigger();
}

void ABadActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABadActor, A);
    DOREPLIFETIME(ABadActor, D);
}
