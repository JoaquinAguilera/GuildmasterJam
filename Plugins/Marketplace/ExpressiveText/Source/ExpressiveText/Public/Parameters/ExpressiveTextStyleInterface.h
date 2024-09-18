// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

class IExpressiveTextStyleInterface 
{
public:

  virtual TSharedPtr<FExpressiveTextParameterLookup> GetParameterLookup() const = 0;  
};