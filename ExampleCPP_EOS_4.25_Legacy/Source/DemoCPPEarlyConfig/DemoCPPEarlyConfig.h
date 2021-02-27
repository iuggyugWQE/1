// Copyright Redpoint Games 2018, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDynamicConfig, All, All);

class FDemoCPPEarlyConfigModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;

    virtual bool IsGameModule() const override
    {
        return true;
    }
};