// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Modules/ModuleManager.h>
#include <Stats/Stats.h>

DECLARE_STATS_GROUP(TEXT("ExpressiveText"), STATGROUP_ExpressiveText, STATCAT_Advanced)

class FExpressiveTextModule : public IModuleInterface
{
public: 
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
