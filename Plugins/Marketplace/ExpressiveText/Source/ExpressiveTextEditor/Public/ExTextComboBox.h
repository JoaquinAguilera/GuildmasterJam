// Copyright 2022 Guganana. All Rights Reserved.
#pragma once 

#include <CoreMinimal.h>

#include <Components/Widget.h>
#include <DetailLayoutBuilder.h>
#include <ExpressiveText/Public/Parameters/ExpressiveTextParameterValue.h>
#include <SSearchableComboBox.h>
#include <Templates/SubclassOf.h>
#include <Templates/SharedPointer.h>

#include "ExTextComboBox.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnSelectionChanged);

struct FExTextComboBoxWrapper : public TSharedFromThis<FExTextComboBoxWrapper>
{ 
    FExTextComboBoxWrapper()
        : SearchableComboBox()
        , Options()
    {}

	TSharedRef<SWidget> RebuildWidget()
    {
        SearchableComboBox = SNew(SSearchableComboBox)
		.OptionsSource(&Options)
		.OnGenerateWidget(this, &FExTextComboBoxWrapper::MakeRetargetSourceComboWidget)
		.OnSelectionChanged(this, &FExTextComboBoxWrapper::OnSelectionChanged)
		.Content()
		[
			SNew( STextBlock )
			.Text(FText::FromString(TEXT("Add a style parameter...")) )
			.Font( IDetailLayoutBuilder::GetDetailFont() )
			.ToolTipText(FText::FromString(TEXT("Customize this style by changing text parameters")))
		];

        return SearchableComboBox.ToSharedRef();
    }
	// End of UWidget interface
    
	TSharedRef<SWidget> MakeRetargetSourceComboWidget( TSharedPtr<FString> InItem )
    {
	    return SNew(STextBlock).Text( FText::FromString(*InItem) ).Font( IDetailLayoutBuilder::GetDetailFont() );
    }

	void OnSelectionChanged( TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo  )
    {
        SelectionChanged.ExecuteIfBound();
        SearchableComboBox->ClearSelection();
    }

    void UpdateOptions( const TArray<TSubclassOf<UExpressiveTextParameterValue>>& NewOptions )
    {
		Options.Empty();
		OptionsMap.Empty();

        for( const auto& ValueClass : NewOptions)
        {            
            if( ValueClass && !ValueClass->HasAnyClassFlags(CLASS_Abstract) )
            {
                TSharedPtr<FString> Name = nullptr;
                
                const FString& NameFromMetadata = ValueClass->GetMetaData(TEXT("DisplayName"));

                if  (!NameFromMetadata.IsEmpty())
                {
                    Name = MakeShareable(new FString(NameFromMetadata) );
                }
                else
                {
                    Name = MakeShareable(new FString(ValueClass->GetName()));
                    Name->ReplaceInline(TEXT("ExTextValue_"), TEXT(""));
                }                

                Options.Emplace( Name );
                OptionsMap.Emplace( Name, ValueClass);
            }
        }

        Options.Sort([](const TSharedPtr<FString>& A, const TSharedPtr<FString>& B) {
            return *A.Get() < *B.Get();
        });
        
        SearchableComboBox->RefreshOptions();
    }

    TSubclassOf<UExpressiveTextParameterValue> GetSelectedOption() const
    {
        if( auto* Found = OptionsMap.Find( SearchableComboBox->GetSelectedItem() ) )
        {
            return *Found;
        }

        return nullptr;
    }

    FOnSelectionChanged SelectionChanged;

private:

    TSharedPtr<SSearchableComboBox> SearchableComboBox;
	TArray< TSharedPtr< FString > >	Options;
    TMap< TSharedPtr<FString>, TSubclassOf<UExpressiveTextParameterValue> > OptionsMap;
};

UCLASS( BlueprintType )
class UExTextComboBox : public UWidget
{
    GENERATED_BODY()

public:

    UExTextComboBox()
        : Super()
        , ComboBoxWrapper( MakeShareable( new FExTextComboBoxWrapper ) )
    {}

    #if WITH_EDITOR
	virtual const FText GetPaletteCategory() override
    {
        return FText::FromString(TEXT("Expressive Text Editor"));
    }
    #endif

    // UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override
    {
        return ComboBoxWrapper->RebuildWidget();
    }

    virtual void ReleaseSlateResources(bool bReleaseChildren) override
    {
        ComboBoxWrapper.Reset();

        Super::ReleaseSlateResources(bReleaseChildren);
    }
	// End of UWidget interface
    
    UFUNCTION( BlueprintCallable, Category = ExpressiveTextEditor )
    void BindOnSelectionChanged( FOnSelectionChanged OnSelectionChanged )
    {
        ComboBoxWrapper->SelectionChanged = OnSelectionChanged;
    }

    UFUNCTION( BlueprintCallable, Category = ExpressiveTextEditor )
    TSubclassOf<UExpressiveTextParameterValue> GetSelectedOption() const
    {
        return ComboBoxWrapper->GetSelectedOption();
    }

    UFUNCTION( BlueprintCallable, Category = ExpressiveTextEditor )
    void UpdateOptions( const TArray<TSubclassOf<UExpressiveTextParameterValue>>& Options )
    {
        ComboBoxWrapper->UpdateOptions( Options );
    }

private:
    TSharedPtr<FExTextComboBoxWrapper> ComboBoxWrapper;
};
