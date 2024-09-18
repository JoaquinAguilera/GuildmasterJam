// Copyright 2022 Guganana. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include <ExpressiveText/Public/Parameters/ExpressiveTextParams.h>

#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include <Widgets/Input/SComboBox.h>
#include <Widgets/SWidget.h>
//
//class FExText_GlyphAnimationCustomization : public IPropertyTypeCustomization
//{
//public:
//	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
//    {
//        return MakeShareable(new FExText_GlyphAnimationCustomization());
//    } 
//
//
//	/** IPropertyTypeCustomization interface */
//	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override
//	{
//        StructPropertyHandle = InStructPropertyHandle;
//
//        static const FName AnimationNamePropertyName = GET_MEMBER_NAME_CHECKED(FExText_GlyphAnimation, AnimationName);
//        AnimationNameProperty = StructPropertyHandle->GetChildHandle(AnimationNamePropertyName);
//        check(AnimationNameProperty.IsValid());
//
//        InHeaderRow
//        .NameContent()
//        [
//            InStructPropertyHandle->CreatePropertyNameWidget()
//        ]
//        .ValueContent()
//        .MinDesiredWidth(0)
//        .MaxDesiredWidth(0)
//        [
//            InStructPropertyHandle->CreatePropertyValueWidget()
//        ];
//    }
//    
//    virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& InStructBuilder, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override
//    {
//        IDetailPropertyRow& AnimationNameRow = InStructBuilder.AddProperty(AnimationNameProperty.ToSharedRef());
//        
//        AnimationNameRow.CustomWidget()
//        .NameContent()
//        [
//            AnimationNameProperty->CreatePropertyNameWidget()
//        ]
//        .ValueContent()
//        [
//            SAssignNew(AnimationCombo, SComboBox<TSharedPtr<FName>>)
//            .OptionsSource(&AnimationComboData)
//            .IsEnabled(this, &FExText_GlyphAnimationCustomization::IsAnimationComboEnabled)
//            .OnComboBoxOpening(this, &FExText_GlyphAnimationCustomization::OnAnimationComboOpening)
//            .OnSelectionChanged(this, &FExText_GlyphAnimationCustomization::OnAnimationSelectionChanged)
//            .OnGenerateWidget(this, &FExText_GlyphAnimationCustomization::MakeAnimationNameWidget)
//            [
//                SNew(STextBlock)
//                .Text(this, &FExText_GlyphAnimationCustomization::GetAnimationComboText)
//                .Font(IDetailLayoutBuilder::GetDetailFont())
//            ]
//        ];
//
//
//        static const FName ReversePropertyName = GET_MEMBER_NAME_CHECKED(FExText_GlyphAnimation, Reverse);
//        auto ReverseProp = InStructPropertyHandle->GetChildHandle(ReversePropertyName);
//        if (ReverseProp.IsValid())
//        {
//            InStructBuilder.AddProperty(ReverseProp.ToSharedRef());
//        }
//    }
//
//protected:
//	bool IsAnimationComboEnabled() const
//    {
//        return true;
//    }
//
//	void OnAnimationComboOpening()
//    {
//	    AnimationComboData.Empty();
//        AnimationComboData.Add( MakeShareable( new FName(NAME_None) ) );
//
//        if( auto* GlyphClass = GetDefault<UExpressiveTextSettings>()->GlyphWidgetClass.LoadSynchronous() )
//        {
//			if( auto* GlyphCDO = GlyphClass->GetDefaultObject<UExpressiveTextGlyphWidget>() )
//            {
//                TArray<FName> OutAnimations;
//                GlyphCDO->GetAllAnimationNames(OutAnimations);
//                for( FName Animation : OutAnimations )
//                {
//                    AnimationComboData.Add(MakeShareable( new FName(Animation) ));
//                }
//            }
//        }
//
//        AnimationCombo->ClearSelection();
//        AnimationCombo->RefreshOptions();
//    }
//
//	void OnAnimationSelectionChanged(TSharedPtr<FName> InNewSelection, ESelectInfo::Type)
//    {
//        if(InNewSelection.IsValid())
//        {
//            AnimationNameProperty->SetValue(*InNewSelection);
//        }
//    }
//
//	TSharedRef<SWidget> MakeAnimationNameWidget(TSharedPtr<FName> AnimationEntry)
//    {
//        return
//            SNew(STextBlock)
//            .Text(FText::FromName(*AnimationEntry))
//            .Font(FEditorStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")));
//    }
//
//	FText GetAnimationComboText() const
//    {
//	    return FText::FromName(GetActiveAnimationEntry());
//    }
//
//	TArray<FExText_GlyphAnimation*> GetAnimationStructsBeingEdited() const
//    {
//        TArray<FExText_GlyphAnimation*> Output;
//
//        if(StructPropertyHandle->IsValidHandle())
//        {
//            TArray<void*> StructPtrs;
//            StructPropertyHandle->AccessRawData(StructPtrs);
//            Output.Reserve(StructPtrs.Num());
//
//            for(auto It = StructPtrs.CreateConstIterator(); It; ++It)
//            {
//                void* RawPtr = *It;
//                if(RawPtr)
//                {
//                    FExText_GlyphAnimation* const AnimationStruct = reinterpret_cast<FExText_GlyphAnimation*>(RawPtr);
//                    Output.Add(AnimationStruct);
//                }
//            }
//        }
//
//        return Output;
//    }
//
//    FName GetActiveAnimationEntry() const
//    {
//        auto AnimationStructs = GetAnimationStructsBeingEdited();
//        if(AnimationStructs.Num() > 0)
//        {
//            return AnimationStructs[0]->AnimationName;
//        }
//
//        return NAME_None;
//    }
//
//	TSharedPtr<IPropertyHandle> StructPropertyHandle;
//	TSharedPtr<IPropertyHandle> AnimationNameProperty;
//
//	TSharedPtr<SComboBox<TSharedPtr<FName>>> AnimationCombo;
//	TArray<TSharedPtr<FName>> AnimationComboData;
//};
