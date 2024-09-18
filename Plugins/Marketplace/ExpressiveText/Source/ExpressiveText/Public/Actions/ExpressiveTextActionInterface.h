// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <UObject/Interface.h>

#include "ExpressiveTextActionInterface.generated.h"

UINTERFACE(BlueprintType)
class UExText_ActionInterface : public UInterface 
{
  GENERATED_BODY()
};

class IExText_ActionInterface 
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintNativeEvent, Category = ExpressiveText )
  void Run();

  virtual void SetWorld( UWorld* World ) {}
};
