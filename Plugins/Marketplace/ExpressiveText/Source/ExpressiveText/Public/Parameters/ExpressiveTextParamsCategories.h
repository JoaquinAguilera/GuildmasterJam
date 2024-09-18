// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Templates/SubclassOf.h>

#include "ExpressiveTextParamsCategories.generated.h"


UCLASS(BlueprintType)
class UExTextParameterCategory : public UObject
{
	GENERATED_BODY()
public:

#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadOnly, Category = "Expressive Text Category")
	FLinearColor Color;
	
	UPROPERTY(BlueprintReadOnly, Category = "Expressive Text Category")
	FName Name;

	UPROPERTY(BlueprintReadOnly, Category = "Expressive Text Category")
	int32 SortPriority;
#endif

	UFUNCTION( BlueprintCallable, Category = "Expressive Text Editor")
	static TArray<TSubclassOf<UExTextParameterCategory>> SortCategories(TArray<TSubclassOf<UExTextParameterCategory>> Categories)
	{
#if WITH_EDITORONLY_DATA
		Categories.Sort(
			[](const TSubclassOf<UExTextParameterCategory> A, const TSubclassOf<UExTextParameterCategory> B)
			{
				if (!A.Get() || !B.Get())
				{
					return false;
				}

				return A.GetDefaultObject()->SortPriority < B.GetDefaultObject()->SortPriority;
			}
		);
#endif

		return Categories;
	}
};

#if WITH_EDITORONLY_DATA
#define CATEGORY_COLOR(name, hex) static FColor name(){ return FColor::FromHex(#hex); }
class ExTextCategoryColors
{
public:
	CATEGORY_COLOR(Red, #FF4A1C)
	CATEGORY_COLOR(LightBlue, #87BCDE)
	CATEGORY_COLOR(Yellow, #F7EC59)
	CATEGORY_COLOR(Green, #99D19C)
	CATEGORY_COLOR(Blue, #00A8E8)
	CATEGORY_COLOR(Purple, #A89FFF)
	CATEGORY_COLOR(Pink, #FC60A8)
	CATEGORY_COLOR(White, #FFFFFF)
};
#undef CATEGORY_COLOR
#endif

#if WITH_EDITORONLY_DATA
#define CATEGORY_CONSTRUCT( CategoryName, CategoryColor, PriorityValue ) \
	Color = ExTextCategoryColors::CategoryColor(); \
	Name = TEXT(#CategoryName); \
	SortPriority = PriorityValue;
#else
#define CATEGORY_CONSTRUCT( CategoryName, CategoryColor, PriorityValue ) 
#endif

UCLASS()
class UExTextParameterCategory_Font : public UExTextParameterCategory
{
	GENERATED_BODY()
	UExTextParameterCategory_Font() : Super() { CATEGORY_CONSTRUCT(Font, Red, 1) }
};

UCLASS()
class UExTextParameterCategory_Material : public UExTextParameterCategory
{
	GENERATED_BODY()
	UExTextParameterCategory_Material() : Super(){ CATEGORY_CONSTRUCT(Material, Green, 4) }
};

UCLASS()
class UExTextParameterCategory_Shadow : public UExTextParameterCategory
{
	GENERATED_BODY()
	UExTextParameterCategory_Shadow() : Super() { CATEGORY_CONSTRUCT(Shadow, Blue, 6) }
};

UCLASS()
class UExTextParameterCategory_Outline : public UExTextParameterCategory
{
	GENERATED_BODY()
	UExTextParameterCategory_Outline() : Super() { CATEGORY_CONSTRUCT(Outline, LightBlue, 5) }
};

UCLASS()
class UExTextParameterCategory_RevealAnimation : public UExTextParameterCategory
{
	GENERATED_BODY()
	UExTextParameterCategory_RevealAnimation() : Super() { CATEGORY_CONSTRUCT(Reveal Animation, Yellow,2) }
};

UCLASS()
class UExTextParameterCategory_ClearAnimation : public UExTextParameterCategory
{
	GENERATED_BODY()
	UExTextParameterCategory_ClearAnimation() : Super() { CATEGORY_CONSTRUCT(Clear Animation, Purple, 3) }
};

UCLASS()
class UExTextParameterCategory_Other : public UExTextParameterCategory
{
	GENERATED_BODY()
	UExTextParameterCategory_Other() : Super() { CATEGORY_CONSTRUCT(Other, Pink, 7) }
};

UCLASS()
class UExTextParameterCategory_Settings : public UExTextParameterCategory
{
	GENERATED_BODY()
	UExTextParameterCategory_Settings() : Super() { CATEGORY_CONSTRUCT(Settings, White, 100) }
};

#undef CATEGORY_CONSTRUCT
