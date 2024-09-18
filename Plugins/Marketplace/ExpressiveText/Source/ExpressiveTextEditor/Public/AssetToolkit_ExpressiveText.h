// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include "EditorUndoClient.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/GCObject.h"

#include "ExpressiveText.h"

class FSpawnTabArgs;
class ISlateStyle;
class IToolkitHost;
class SDockTab;
class UTextAsset;


/**
 * Implements an Editor toolkit for textures.
 */
class FAssetToolkit_ExpressiveText
	: public FAssetEditorToolkit
//	, public FEditorUndoClient
	, public FGCObject
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InStyle The style set to use.
	 */
	FAssetToolkit_ExpressiveText(const TSharedRef<ISlateStyle>& InStyle)
        : AssetBeingEdited(nullptr)
        , Style(InStyle)
    { }


	/** Virtual destructor. */
	//virtual ~FAssetToolkit_ExpressiveText();

	void Initialize(UExpressiveTextAsset* InAsset, const EToolkitMode::Type InMode, const TSharedPtr<IToolkitHost>& InToolkitHost)
    {
        AssetBeingEdited = InAsset;
    }

	//~ FAssetEditorToolkit interface
	virtual FString GetDocumentationLink() const override{ return FString(); }
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override{}
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override{}

	//~ IToolkit interface
	virtual FText GetBaseToolkitName() const override{ return NSLOCTEXT("ExpressiveTextEditor", "AssetToolkit_ExpressiveText", "Expressive Text Editor");}
	virtual FName GetToolkitFName() const override{ return TEXT("ExpressiveTextEditor"); }
	virtual FLinearColor GetWorldCentricTabColorScale() const override{ return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f); }
	virtual FString GetWorldCentricTabPrefix() const override { return TEXT("Expressive Text"); }

	//~ FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override{ Collector.AddReferencedObject(AssetBeingEdited);}
	
protected:

	// //~ FEditorUndoClient interface
	// virtual void PostUndo(bool bSuccess) override;
	// virtual void PostRedo(bool bSuccess) override;

private:

	/** Callback for spawning the Properties tab. */
	//TSharedRef<SDockTab> HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier);

private:

	/** The text asset being edited. */
	UExpressiveTextAsset* AssetBeingEdited;

	/** Pointer to the style set to use for toolkits. */
	TSharedRef<ISlateStyle> Style;
};