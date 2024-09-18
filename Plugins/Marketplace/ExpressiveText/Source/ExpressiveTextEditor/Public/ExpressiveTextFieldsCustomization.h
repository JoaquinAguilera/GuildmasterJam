// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include <ExpressiveText/Public/Asset/ExpressiveTextFields.h>
#include <ExpressiveText/Public/Extractions/TreeExtraction.h>
#include <ExpressiveText/Public/ExpressiveTextProcessor.h>

#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"

class FExText_ExpressiveTextFieldsCustomization : public IPropertyTypeCustomization
{
public:

    FExText_ExpressiveTextFieldsCustomization()
        : StructPropertyHandle()
    {}

    static TSharedRef<IPropertyTypeCustomization> MakeInstance()
    {
        return MakeShareable(new FExText_ExpressiveTextFieldsCustomization);
    }

    virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& InHeaderRow, IPropertyTypeCustomizationUtils& InStructCustomizationUtils) override
    {
        InHeaderRow
            .NameContent()
            [
                InStructPropertyHandle->CreatePropertyNameWidget()
            ]
            .ValueContent()
            [
                InStructPropertyHandle->CreatePropertyValueWidget()
            ];
    }

    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override
    {
        StructPropertyHandle = InStructPropertyHandle;

        FSimpleDelegate OnEditDelegate = FSimpleDelegate::CreateSP(this, &FExText_ExpressiveTextFieldsCustomization::OnEdit);
        InStructPropertyHandle->SetOnPropertyValueChanged(OnEditDelegate);
        InStructPropertyHandle->SetOnChildPropertyValueChanged(OnEditDelegate);

        AddPropertiesRecursive(InStructPropertyHandle, StructBuilder);
    }

    void OnEdit()
    {
        for (FExpressiveTextFields* Fields : GetStructsBeingEdited())
        {
            if (Fields)
            {   
                UObject* ValidOuter = nullptr;

                TArray<UObject*> OuterObjects;
                StructPropertyHandle->GetOuterObjects(OuterObjects);

                for( UObject* Object : OuterObjects )
                {
                    if (Object)
                    {
                        ValidOuter = Object;
                        break;
                    }
                }

                FExpressiveText Text;
                Text.SetFields(*Fields);
                Text.SetWorldContext(ValidOuter);

                // Populate ReferencedResources
                UExpressiveTextProcessor::CompileText(Text).Next(
                    [Fields, FieldsHandle = Fields->GetAliveHandle()](const FCompiledExpressiveText& InCompiledText)
                    {
                        if (FieldsHandle.IsValid())
                        {
                            Fields->ReferencedResources = InCompiledText.HarvestedResources;
                        }
                    }
                );
            }
        }

    }

    TArray<FExpressiveTextFields*> GetStructsBeingEdited() const
    {
        TArray<FExpressiveTextFields*> Output;

        if (StructPropertyHandle->IsValidHandle())
        {
            TArray<void*> StructPtrs;
            StructPropertyHandle->AccessRawData(StructPtrs);
            Output.Reserve(StructPtrs.Num());

            for (auto It = StructPtrs.CreateConstIterator(); It; ++It)
            {
                void* RawPtr = *It;
                if (RawPtr)
                {
                    FExpressiveTextFields* const Struct = reinterpret_cast<FExpressiveTextFields*>(RawPtr);
                    Output.Add(Struct);
                }
            }
        }

        return Output;
    }

    void AddPropertiesRecursive(TSharedRef<IPropertyHandle> Property, IDetailChildrenBuilder& ChildBuilder)
    {
        uint32 NumChildren = 0;
        Property->GetNumChildren(NumChildren);

        for (uint32 Index = 0; Index < NumChildren; ++Index)
        {
            TSharedRef<IPropertyHandle> Child = Property->GetChildHandle(Index).ToSharedRef();
            if (Child->GetProperty())
            {
                ChildBuilder.AddProperty(Child);
            }
            else
            {
                AddPropertiesRecursive(Child, ChildBuilder);
            }
        }
    }

    TSharedPtr<IPropertyHandle> StructPropertyHandle;

};
