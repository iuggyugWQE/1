// Copyright June Rhodes. MIT Licensed.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OnlineSubsystem.h"

#include "ClangTidyExampleGameModeBase.generated.h"
 
UCLASS(BlueprintType)
class ABadDestroyTest : public AActor
{
	GENERATED_BODY()

public:
    virtual void BeginDestroy() override;
}; 

UCLASS()
class AClangTidyExampleGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

private:
    // This will warn that it's unsafe to store shared pointers to the online subsystem
    // in class or struct fields, because their lifetime must be controlled by the online
    // subsystem.
    IOnlineSubsystemPtr BadPtr;
    IOnlineIdentityPtr AlsoBadPtr;
	
public:
    void SomeBasicFunction(FString MyStr);
    void MyMoveFunction(FString MyStr);

	void TestFunction();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    // This will warn because there's no DOREPLIFETIME call in GetLifetimeReplicatedProps.
    UPROPERTY(Replicated)
    int HelloWorld;

    UPROPERTY(Category = "Test", BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Hello", BlueprintBaseOnly))
    int SomethingElse;

    // This will warn that you forgot to add UPROPERTY().
    AActor *UntrackedObject;
};

UCLASS()
class ABadActor : public AActor
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    UPROPERTY(Replicated)
    int A;

    // This will warn because there's no DOREPLIFETIME call in GetLifetimeReplicatedProps.
    UPROPERTY(Replicated)
    int B;

    // This will warn because there's no DOREPLIFETIME call in GetLifetimeReplicatedProps.
    UPROPERTY(Replicated)
    int C;

    UPROPERTY(Replicated)
    int D;
};

UINTERFACE(MinimalAPI, Blueprintable)
class UMyInter : public UInterface
{
    GENERATED_BODY()
};

class IMyInter
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    bool ReactToTrigger();
};