// Copyright 2022 Guganana. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "Styles/ExpressiveTextStyle.h"
#include "Layout/ExTextMIDCache.h"
#include "Resources/ExpressiveTextResources.h"

#include <Engine/AssetManager.h>
#include <Subsystems/EngineSubsystem.h>

#if UE_VERSION_OLDER_THAN( 5, 1, 0 )
#include <AssetData.h>
#else
#include <AssetRegistry/AssetData.h>
#endif

#include "ExpressiveTextSubsystem.generated.h"

class UExpressiveTextAnimation;

UCLASS()
class EXPRESSIVETEXT_API UExpressiveTextSubsystem 
	: public UEngineSubsystem 
{
	GENERATED_BODY()
public:

	TOptional<TFuture<UExpressiveTextFont*>> FetchFont(const FName& Tag, TOptional<FString> OnMissingFontMessage = TOptional<FString>()) const;
	const FColor* FetchColorForTag( const FName& Tag ) const;


	template< typename ResourceType, typename T = typename ResourceType::Class >
	TOptional<TFuture<T*>> FetchResource(const FName& Tag) const
	{
		auto& Manager = UAssetManager::Get();
		FPrimaryAssetId AssetId(ResourceType::AssetType(), Tag);

		FAssetData Data;
		if (!Manager.GetPrimaryAssetData(AssetId, Data))
		{
			return TOptional<TFuture<T*>>();
		}

		TArray<FName> Unused;
		auto LoadHandle = Manager.PreloadPrimaryAssets({ AssetId }, Unused, false);

		TSharedRef<TPromise<T*>> Result = MakeShareable(new TPromise<T*>());
		const auto OnLoaded = [Result, LoadHandle]()
		{
			Result->SetValue(Cast<T>(LoadHandle->GetLoadedAsset()));
		};

		if (LoadHandle->HasLoadCompleted())
		{
			OnLoaded();
		}
		else
		{
			FStreamableDelegate Delegate;
			Delegate.BindLambda(OnLoaded);
			LoadHandle->BindCompleteDelegate(Delegate);
		}

		return Result->GetFuture();
	}


	template< typename ResourceType, typename T = typename ResourceType::Class >
	TArray<FSoftObjectPath> GetAllResourcesByType()
	{
		TArray<FSoftObjectPath> Assets;

		auto& Manager = UAssetManager::Get();
		Manager.GetPrimaryAssetPathList(ResourceType::AssetType(), Assets);

		return Assets;
	}
	
	DECLARE_EVENT_TwoParams( UExpressiveTextSubsystem, FExpressiveTextMissingFont, FName, TOptional<FString>)
	FExpressiveTextMissingFont& OnMissingFont()
	{
		return ExpressiveTextMissingFont;
	}

	const TSharedPtr<FExTextMID> RequestMID( const FExTextMIDRequest& Request )
	{
		return MIDCache.RequestMID( Request );
	}

	
#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadOnly, Category = ExpressiveText)
	FVector ViewportCameraLoc;
		
	UPROPERTY(BlueprintReadOnly, Category = ExpressiveText)
	FRotator ViewportCameraRot;
#endif

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:

	FExpressiveTextMissingFont ExpressiveTextMissingFont;
	TMap<FName, FColor> ColorMap;
	FExTextMIDCache MIDCache;

	//populates color map based on CSS/HTML color names
	void PopulateColorMap();
};
