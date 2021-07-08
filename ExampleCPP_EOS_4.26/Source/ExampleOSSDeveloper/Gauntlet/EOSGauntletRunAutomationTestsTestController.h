// Copyright 2020 June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GauntletTestController.h"
#include "IAutomationControllerManager.h"

#include "EOSGauntletRunAutomationTestsTestController.generated.h"

enum class EEOSGauntletTestStage : uint8
{
    Init,
    AutomationInit,
    AutomationFindWorkers,
    AutomationRequestTests,
    AutomationMonitorTests,
    End,
};

UCLASS(transient)
class UEOSGauntletRunAutomationTestsTestController : public UGauntletTestController
{
    GENERATED_BODY()

public:
    UEOSGauntletRunAutomationTestsTestController();
    virtual ~UEOSGauntletRunAutomationTestsTestController(){};

private:
    EEOSGauntletTestStage Stage;

    IAutomationControllerManagerPtr AutomationController;

    float FindWorkersTimeout;

    FDelegateHandle OnTestsRefreshedDelegate;
    void OnTestsRefreshed();

protected:
    void OnTick(float TimeDelta) override;
};