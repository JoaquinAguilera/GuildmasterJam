// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <UObject/Interface.h>

#include "ExpressiveTextEditorWidgetInterface.generated.h"

UINTERFACE(BlueprintType)
class UExpressiveTextEditorWidgetInterface : public UInterface 
{
  GENERATED_BODY()
};

class IExpressiveTextEditorWidgetInterface 
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintNativeEvent, Category = ExpressiveTextEditor )
  bool SetAsset( UObject* Asset );
};