// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "ExTextClearDirection.generated.h"

UENUM(BlueprintType)
enum class EExText_ClearDirection: uint8
{
	// Clear each character one by one in forward direction (left-to-right in latin based languages).
	Forwards,
	
	// Clear each character one by one in backwards direction (right-to-left in latin based languages).
	Backwards
};
