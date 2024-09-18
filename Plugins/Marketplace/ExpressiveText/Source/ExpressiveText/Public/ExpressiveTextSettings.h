// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>
#include <Engine/Engine.h>
#include "Styles/ExpressiveTextDefaultStyle.h"

#include "ExpressiveTextSettings.generated.h"

class UExpressiveTextLightModeMaterialsAsset;

UCLASS(config = ExpressiveText, DefaultConfig, meta = (DisplayName = "Expressive Text") )
class EXPRESSIVETEXT_API UExpressiveTextSettings : public UDeveloperSettings
{
	GENERATED_BODY()

	UExpressiveTextSettings()
		: Super()
		, CustomDownloadedFontsDirectory()
		, ExpressiveTextWidgetClass( FSoftObjectPath( TEXT("/ExpressiveText/BP_ExpressiveTextWidget.BP_ExpressiveTextWidget_C") ) )
		, DefaultStyleAsset( FSoftObjectPath( TEXT("/ExpressiveText/Core/Internal/ExpressiveTextDefaultStyle.ExpressiveTextDefaultStyle") ) )
		, LightModeMaterialsAsset( FSoftObjectPath(TEXT("/ExpressiveText/Core/LightModeMaterialsAsset.LightModeMaterialsAsset")) )
		, BaseTextLayer( FSoftObjectPath(TEXT("/ExpressiveText/Core/Internal/BaseTextLayer.BaseTextLayer")) )
		, BaseTextLayerBlend( FSoftObjectPath(TEXT("/ExpressiveText/Core/Internal/BaseTextLayerBlend.BaseTextLayerBlend")) )
		, BaseTextMaterial( FSoftObjectPath(TEXT("/ExpressiveText/Core/Internal/BaseTextMaterial.BaseTextMaterial")) )
		, DebuggerClass(FSoftObjectPath(TEXT("/ExpressiveText/Core/Debug/BP_ExpressiveTextDebugger.BP_ExpressiveTextDebugger_C")))
		, TagHighlightingColors()
		, StopShaderPatchPrompts(false)
	{
		TagHighlightingColors = {
			FColor( 240, 128, 128 ),
			FColor( 155, 209, 229 ),
			FColor( 149, 191, 116 ),
			FColor( 255, 186, 73),
			FColor( 60, 47, 165 )
		};
	}

	virtual FName GetCategoryName() const override
	{
		return FName(TEXT("Plugins"));
	}
	virtual void PostInitProperties() override
	{
		Super::PostInitProperties();
		
	#if WITH_EDITOR
		// Ensure ini file is always up to date since it's used for cook purposes
		SaveConfig(); 
	#endif
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override
	{
		Super::PostEditChangeProperty(Event);

		if (RevertShaderPatch)
		{
			RevertShaderPatch = false;
			GEngine->Exec(nullptr, TEXT("ExpressiveText.RevertSlateShadersPatch"));
		}

		SaveConfig();
	}
#endif

public:

	UPROPERTY( Config, EditDefaultsOnly, Category = ExpressiveText )
	FDirectoryPath CustomDownloadedFontsDirectory;

	UPROPERTY( Config, EditDefaultsOnly, Category = ExpressiveText )
	TSoftClassPtr<class UUserWidget> ExpressiveTextWidgetClass;

	UPROPERTY( Config, EditDefaultsOnly, Category = ExpressiveText )
	TSoftObjectPtr<UExpressiveTextDefaultStyle> DefaultStyleAsset;
	
	UPROPERTY( Config, EditDefaultsOnly, Category = ExpressiveText )
    TSoftObjectPtr<UExpressiveTextLightModeMaterialsAsset> LightModeMaterialsAsset;

	UPROPERTY( Config, VisibleAnywhere, Category = ExpressiveText )
    TSoftObjectPtr<UMaterialFunctionMaterialLayer> BaseTextLayer;

	UPROPERTY( Config, VisibleAnywhere, Category = ExpressiveText )
    TSoftObjectPtr<UMaterialFunctionInterface> BaseTextLayerBlend;
	
	UPROPERTY( Config, VisibleAnywhere, Category = ExpressiveText )
    TSoftObjectPtr<UMaterial> BaseTextMaterial;

	UPROPERTY( Config, EditDefaultsOnly, Category = ExpressiveText )
	TSoftClassPtr<class AExpressiveTextDebugger> DebuggerClass;

	UPROPERTY( Config, EditDefaultsOnly, BlueprintReadOnly, Category = ExpressiveText )
	TArray<FColor> TagHighlightingColors;

	UPROPERTY( Config, EditDefaultsOnly, BlueprintReadOnly, Category = ExpressiveText )
	bool StopShaderPatchPrompts;

	UPROPERTY( Config, EditDefaultsOnly, BlueprintReadOnly, Category = ExpressiveText )
	bool RevertShaderPatch;

	const UExpressiveTextDefaultStyle* GetDefaultStyle() const
	{
		return DefaultStyleAsset.LoadSynchronous();
	}

	UFUNCTION( BlueprintPure, Category = ExpressiveText )
	static UExpressiveTextSettings* GetExpressiveTextSettings()
	{
		return GetMutableDefault<UExpressiveTextSettings>();
	}

};
