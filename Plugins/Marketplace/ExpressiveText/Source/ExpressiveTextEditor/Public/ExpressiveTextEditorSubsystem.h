// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <EditorSubsystem.h>
#include <ExpressiveText/Public/Resources/ExpressiveTextResources.h>
#include <HAL/IConsoleManager.h>
#include <JsonObjectConverter.h>
#include <Interfaces/IPluginManager.h>

#include "ExpressiveTextEditorSubsystem.generated.h"

UCLASS()
class EXPRESSIVETEXTEDITOR_API UExpressiveTextEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UExpressiveTextEditorSubsystem();


protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	void OnEngineInitialized();
	void GenerateAssetManagerEntries();

	template <typename Resource>
	bool AddAssetManagerEntryForResource(const TArray<FString>& Paths)
	{
		return AddAssetManagerEntryImpl( Resource::AssetType(), Resource::Class::StaticClass(), Paths);
	}
	bool AddAssetManagerEntryImpl(const FPrimaryAssetType& Type, UClass* BaseClass, const TArray<FString>& Paths);
	
    static FAutoConsoleCommand CmdUpdateViewportCameraPosititioning;

	UPROPERTY()
	TArray<UClass*> KeepLoadedClasses;
};


