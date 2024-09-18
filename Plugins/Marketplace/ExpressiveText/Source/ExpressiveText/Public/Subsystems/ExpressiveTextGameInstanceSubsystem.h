// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "ExpressiveTextGameInstanceSubsystem.generated.h"


UCLASS()
class EXPRESSIVETEXT_API UExpressiveTextGameInstanceSubsystem : public UGameInstanceSubsystem  
{
	GENERATED_BODY()

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
	UPROPERTY(Transient)
	AActor* Debugger;
};


