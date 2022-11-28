#pragma once

#include "Commandlets/Commandlet.h"

#include "EraseTasksCommandlet.generated.h"

UCLASS()
class UEraseTasksCommandlet : public UCommandlet
{
    GENERATED_BODY()

    virtual int32 Main(const FString &Params) override;

private:
    bool SavePackageHelper(UPackage *Package);
    void MovePinsAwayToAnotherPin(class UEdGraphPin *CurrentPin, class UEdGraphPin *NewPin);
};