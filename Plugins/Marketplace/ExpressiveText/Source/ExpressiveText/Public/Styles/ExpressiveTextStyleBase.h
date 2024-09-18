// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Parameters/ExpressiveTextParameterLookup.h"
#include "Parameters/ExpressiveTextParams.h"
#include "Parameters/ExpressiveTextStyleInterface.h"
#include "Parameters/Extractors/ExpressiveTextCacheParameterExtractor.h"

#include <Engine/DataAsset.h>
#include <Framework/Notifications/NotificationManager.h>
#include <Widgets/Notifications/SNotificationList.h>

#include "ExpressiveTextStyleBase.generated.h"

UCLASS( Abstract, BlueprintType )
class EXPRESSIVETEXT_API UExpressiveTextStyleBase
	: public UPrimaryDataAsset
	, public IExpressiveTextStyleInterface
{
	GENERATED_BODY()

public:

	UExpressiveTextStyleBase()
		: Super( )
		, Parameters()
		, Cache( MakeShareable( new FExpressiveTextParameterCache ) )
	{}

#if WITH_EDITOR	
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UExpressiveTextStyleBase, Parameters))
		{
			RebuildCache();
		}

		SanitizeInheritedStyles();
		OnPostEditChangeCalled.Broadcast();
	}
#endif

	void SanitizeInheritedStyles()
	{
		UExpressiveTextStyleBase* Culprit;

		while( CheckForCircularStyleReferences(*this, Culprit) )
		{
			InheritedStyles.Remove( Culprit );

			FString Message = FString::Printf( TEXT("Inherting style '%s' would cause a circular style reference - removed entry"), *Culprit->GetDescriptor().ToString() );
			FNotificationInfo Info( FText::FromString( Message ) );
			Info.ExpireDuration = 5.0f;
			Info.bFireAndForget = true;
			Info.bUseLargeFont = false;
			Info.bUseThrobber = false;
			Info.bUseSuccessFailIcons = false;
			FSlateNotificationManager::Get().AddNotification( Info );
		}
	}

	virtual TSharedPtr<FExpressiveTextParameterLookup> GetParameterLookup() const override
	{		
		auto WeakCache = TWeakPtr< FExpressiveTextParameterCache >(Cache);
		TSharedPtr<IExpressiveTextParameterExtractor> Extractor = MakeShareable(new FExpressiveTextCacheParameterExtractor( WeakCache ));
		TSharedPtr<FExpressiveTextParameterLookup> Root = MakeShareable(new FExpressiveTextParameterLookup( GetDescriptor(), Extractor ));

		for( int32 i = InheritedStyles.Num() - 1; i >= 0; i-- )
		{
			auto* InheritedStyle = InheritedStyles[i];

			if( InheritedStyle )
			{
				auto InheritedStyleLookup = InheritedStyle->GetParameterLookup();
				Root->SetNext(InheritedStyleLookup);
			}
		}

		return Root;
	}


	virtual void PostLoad() override
	{
		Super::PostLoad();
		RebuildCache();
		SanitizeInheritedStyles();
	}

	virtual FName GetDescriptor() const
	{
		return FName();
	}

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
	TMap< TSubclassOf<UExpressiveTextParameterValue>, UExpressiveTextParameterValue*> Parameters;

	UPROPERTY(EditAnywhere, Category = InheritedStyles)
	TArray<UExpressiveTextStyleBase*> InheritedStyles;

#if WITH_EDITOR
	UFUNCTION( BlueprintCallable, Category = ExpressiveText )
	void RebuildCache_EditorOnly()
	{
		RebuildCache();
	}

	UFUNCTION(BlueprintCallable, Category = ExpressiveText)
	UExpressiveTextParameterValue* ConstructParameter(UClass* Type)
	{
		return NewObject<UExpressiveTextParameterValue>(this, Type, NAME_None, RF_Public);
	}

	UFUNCTION( BlueprintPure, Category = ExpressiveText )
	bool HasInheritedStyles()
	{
		return InheritedStyles.Num() > 0;
	}
#endif

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPostEditChangeCalled);
	UPROPERTY(BlueprintAssignable, Category ="Expressive Text")
	FOnPostEditChangeCalled OnPostEditChangeCalled;

private:

	bool CheckForCircularStyleReferences( const UExpressiveTextStyleBase& StyleToCheck, UExpressiveTextStyleBase*& CulpritInheritedStyle )
	{
		for( auto* InheritedStyle : InheritedStyles )
		{
			if( InheritedStyle )
			{
				UExpressiveTextStyleBase* Unused;
				if( InheritedStyle == &StyleToCheck || InheritedStyle->CheckForCircularStyleReferences( StyleToCheck, Unused ) )
				{
					CulpritInheritedStyle = InheritedStyle;
					return true;
				}
			}
		}

		return false;
	}

	void RebuildCache()
	{
		Cache->Empty();

		for (auto Iter = Parameters.CreateConstIterator(); Iter; ++Iter)
		{
			if ( auto* Value = Iter.Value() )
			{
				Cache->Add( *Value );
			}
		}
	}

	TSharedRef<FExpressiveTextParameterCache> Cache;
};