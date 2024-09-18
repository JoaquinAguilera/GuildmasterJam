// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <MaterialEditor/MaterialEditorInstanceConstant.h>
#include <Guganana/Engine.h>
#include <Materials/MaterialInstanceConstant.h>
#include <MaterialEditorUtilities.h>
#include <Materials/MaterialInstance.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include <Blueprint/UserWidget.h>
#include <MaterialEditingLibrary.h>

#include <ExpressiveText/Public/Asset/ExpressiveTextMaterial.h>

#include "MaterialParameterEditor.generated.h"

UENUM(BlueprintType)
enum class EExText_MaterialParameterType : uint8
{
    Scalar,
    Vector,
    Texture,
    StaticSwitch,
    MappingOptions
};

USTRUCT(BlueprintType)
struct FExText_MaterialParameter
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Material Parameters")
    EExText_MaterialParameterType Type;
    
    UPROPERTY(BlueprintReadOnly, Category = "Material Parameters")
    FMaterialParameterInfo ParameterInfo;

    int32 SortPriority;

    FExText_MaterialParameter()
        : Type(EExText_MaterialParameterType::Scalar )
        , ParameterInfo()
        , SortPriority( -1 )
    {
    }
};


USTRUCT(BlueprintType)
struct FExText_MaterialParameterGroup
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Material Parameters")
    TArray< FExText_MaterialParameter > Parameters;
    
    UPROPERTY(BlueprintReadOnly, Category = "Material Parameters")
    FName Name;

    int32 SortPriority;
};

USTRUCT(BlueprintType)
struct FExText_MaterialParameterLayerGrouping
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Material Parameters")
    FName LayerName;

    UPROPERTY(BlueprintReadOnly, Category = "Material Parameters")
    TArray<FExText_MaterialParameterGroup> Groups;
};


UCLASS()
class UExpressiveTextMaterialParameterEditorFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

    template <typename Func>
    static void PopulateLayerGrouping (
        TArray<FExText_MaterialParameterLayerGrouping>& LayerGroupings,
        UMaterialInstance* MaterialInstance,
        const TArray<FMaterialParameterInfo>& VisibleParameterInfos,
        const FMaterialLayersFunctions& LayerFunctions, 
        const EExText_MaterialParameterType Type, 
        const Func GetParametersFunction, 
        const TFunction<bool( const FMaterialParameterInfo&, const FName&)> Filter = [](auto Info, auto GroupName) { return true; }
    )
    {
        TArray<FMaterialParameterInfo> OutParameterInfo;
        TArray<FGuid> OutParameterIds;

        GetParametersFunction(OutParameterInfo, OutParameterIds);
        for (const auto& Info : OutParameterInfo)
        {
            if (!VisibleParameterInfos.Contains(Info))
            {
                // Don't show non-active parameters
                continue;
            }

            if (Info.Association == EMaterialParameterAssociation::LayerParameter)
            {
                const auto& LayersEditorData = Guganana::Engine::GetEditorOnlyData(LayerFunctions);
                const FName LayerName = FName(*LayersEditorData.LayerNames[Info.Index].ToString());

                FName GroupName;
                int32 ParamSortPriority = 1000;
                MaterialInstance->GetGroupName(Info, GroupName);
                MaterialInstance->GetParameterSortPriority(Info, ParamSortPriority);

                // Don't display internal parameters
                if(GroupName == FName("Internal") || !Filter(Info,GroupName))
                {
                    continue;
                }

                FExText_MaterialParameterLayerGrouping* FoundExistingLayer = LayerGroupings.FindByPredicate([&LayerName](const auto& LayerGrouping) { return LayerGrouping.LayerName == LayerName; });
                if (!FoundExistingLayer)
                {
                    FoundExistingLayer = &LayerGroupings.Emplace_GetRef();
                    FoundExistingLayer->LayerName = LayerName;
                }

                FExText_MaterialParameterGroup* FoundGroup = FoundExistingLayer->Groups.FindByPredicate([&GroupName](const auto& Group) { return Group.Name == GroupName; });
                if (!FoundGroup)
                {
                    FoundGroup = &FoundExistingLayer->Groups.Emplace_GetRef();
                    FoundGroup->Name = GroupName;
                    MaterialInstance->GetGroupSortPriority(GroupName.ToString(), FoundGroup->SortPriority);
                }

                FExText_MaterialParameter Parameter;
                Parameter.ParameterInfo = Info;
                Parameter.Type = Type;
                Parameter.SortPriority = ParamSortPriority;

                FoundGroup->Parameters.Add(Parameter);
            }
        }
    }

    UFUNCTION(BlueprintCallable, Category = "Expressive Text Editor")
    static void GetParameterLayerGroupings(UMaterialInstance* MaterialInstance, TArray<FExText_MaterialParameterLayerGrouping>& LayerGroupings)
    {
#if WITH_EDITORONLY_DATA
        if (!MaterialInstance)
        {
            return;
        }

        TArray<FMaterialParameterInfo> VisibleParameterInfos;
        FMaterialEditorUtilities::GetVisibleMaterialParameters(MaterialInstance->GetMaterial(), MaterialInstance, VisibleParameterInfos);

        FMaterialLayersFunctions LayerFunctions;

#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
        FStaticParameterSet StaticParameters;
        MaterialInstance->GetStaticParameterValues(StaticParameters);
        if (StaticParameters.MaterialLayersParameters.Num() > 0)
        {
            LayerFunctions = StaticParameters.MaterialLayersParameters[0].Value;
        }
#else
        MaterialInstance->GetMaterialLayers(LayerFunctions);
#endif

        if (LayerFunctions.Layers.Num() == 0)
        {
            return;
        }

        PopulateLayerGrouping( LayerGroupings, MaterialInstance, VisibleParameterInfos, LayerFunctions,
        EExText_MaterialParameterType::Scalar,
            [MaterialInstance](auto& ParameterInfos, auto& ParameterIds)
            {
                MaterialInstance->GetAllScalarParameterInfo(ParameterInfos, ParameterIds);
            }
        );

        PopulateLayerGrouping( LayerGroupings, MaterialInstance, VisibleParameterInfos, LayerFunctions,
        EExText_MaterialParameterType::Vector,
            [MaterialInstance](auto& ParameterInfos, auto& ParameterIds)
            {
                MaterialInstance->GetAllVectorParameterInfo(ParameterInfos, ParameterIds);
            }
        );

        PopulateLayerGrouping( LayerGroupings, MaterialInstance, VisibleParameterInfos, LayerFunctions,
        EExText_MaterialParameterType::Texture,
            [MaterialInstance](auto& ParameterInfos, auto& ParameterIds)
            {
                MaterialInstance->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);
            }
        );

        PopulateLayerGrouping( LayerGroupings, MaterialInstance, VisibleParameterInfos, LayerFunctions,
        EExText_MaterialParameterType::StaticSwitch,
            [MaterialInstance](auto& ParameterInfos, auto& ParameterIds)
            {
                MaterialInstance->GetAllStaticSwitchParameterInfo(ParameterInfos, ParameterIds);
            },
            [](const FMaterialParameterInfo& Info, const FName& GroupName)
            {
                return GroupName != FName("MappingOptions");
            }
        );

        PopulateLayerGrouping( LayerGroupings, MaterialInstance, VisibleParameterInfos, LayerFunctions,
        EExText_MaterialParameterType::MappingOptions,
            [MaterialInstance](auto& ParameterInfos, auto& ParameterIds)
            {
                MaterialInstance->GetAllStaticSwitchParameterInfo(ParameterInfos, ParameterIds);
            },
            [](const FMaterialParameterInfo& Info, const FName& GroupName)
            {
                return GroupName == FName("MappingOptions");
            }
        );
        
        // Sort groups and parameters
        const auto GroupSorter = [](const auto& A, const auto& B) { 
                return A.Name.FastLess(B.Name);
        };
        const auto ParamSorter = [](const auto& A, const auto& B) { 
            return A.SortPriority < B.SortPriority; 
        };

        for(auto& Layer : LayerGroupings )
        {
            for (auto& Group : Layer.Groups)
            {
                Group.Parameters.Sort(ParamSorter);
            }

            Layer.Groups.Sort(GroupSorter);
        }
#endif
    }

   
};

UCLASS()
class UExTextMaterialParameterEditWidgetBase : public UUserWidget
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Expressive Text Editor")
    void Setup(UMaterialInstanceConstant* Material, const TArray<FMaterialParameterInfo>& InParameterInfos)
    {
        ParameterInfos = InParameterInfos;

        if (Material && ParameterInfos.Num() > 0)
        {
            Setup_Impl(Material);
            Setup_Blueprint();
        }
    }

    UFUNCTION(BlueprintCallable, Category = "Expressive Text Editor")
    void ResetToDefault()
    {
        ResetToDefaultImpl();
        Setup_Blueprint();
    }

    UFUNCTION(BlueprintCallable, Category = "Expressive Text Editor")
    virtual bool HasChanged() const
    {
        return false;
    }

    virtual bool IsDirty() const
    {
        return false;
    }

    virtual void Setup_Impl(UMaterialInstanceConstant* Material)
    {
    }

    virtual void ResetToDefaultImpl()
    {
    }

    UFUNCTION(BlueprintImplementableEvent, Category = "Expressive Text Editor")
    void Setup_Blueprint();
    
    void UpdateMaterial(UMaterialInstanceConstant* Material)
    {
        if (Material && ParameterInfos.Num() > 0)
        {
            UpdateMaterial_Impl(Material);
        }
    }

    virtual void UpdateMaterial_Impl(UMaterialInstanceConstant* Material)
    {
    }


    UFUNCTION(BlueprintCallable, Category = "Expressive Text Editor")
    static void UpdateMaterialWithDirtyParameters(UMaterialInstanceConstant* Material, const TArray<UExTextMaterialParameterEditWidgetBase*>& Parameters)
    {
        bool AnyDirty = false;
        for (auto& Parameter : Parameters)
        {
            if (Parameter->IsDirty())
            {
                AnyDirty = true;
                Parameter->UpdateMaterial(Material);
            }
        }

        if (AnyDirty)
        {
            FStaticParameterSet NewStaticParameters;
            Material->GetStaticParameterValues(NewStaticParameters);
            Material->UpdateStaticPermutation(NewStaticParameters);
            //Material->InitStaticPermutation();

#if WITH_EDITOR
            Material->Modify();
            Material->PostEditChange();
            //UMaterialEditingLibrary::RecompileMaterial(Material->GetMaterial());
#endif
        }
    }

    UPROPERTY(BlueprintReadWrite, Category = "Material Parameters")
    TArray<FMaterialParameterInfo> ParameterInfos;

};

UCLASS()
class UExTextMaterialParameterEditWidget_Float : public UExTextMaterialParameterEditWidgetBase
{
    GENERATED_BODY()

public:

    virtual void Setup_Impl(UMaterialInstanceConstant* Material) override
    {
        Material->GetScalarParameterValue(ParameterInfos[0], Value);
        Material->GetScalarParameterDefaultValue(ParameterInfos[0], Default);
        Material->GetScalarParameterSliderMinMax(ParameterInfos[0], Min, Max);
    }

    virtual void ResetToDefaultImpl() override
    {
        Value = Default;
    }

    virtual bool HasChanged() const override
    {
        return Value != Default;
    }

    virtual bool IsDirty() const override
    {
        return Value != LastValue;
    }

    virtual void UpdateMaterial_Impl(UMaterialInstanceConstant* Material) override
    {
        Material->SetScalarParameterValueEditorOnly(ParameterInfos[0], Value);
        LastValue = Value;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    float Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    float Value;
    float LastValue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    float Min;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    float Max;
};

UCLASS()
class UExTextMaterialParameterEditWidget_Vector4 : public UExTextMaterialParameterEditWidgetBase
{
    GENERATED_BODY()

public:

    virtual void ResetToDefaultImpl() override
    {
        Value = Default;
    }

    virtual bool HasChanged() const override
    {
        return Value != Default;
    }

    virtual bool IsDirty() const override
    {
        return Value != LastValue;
    }

    virtual void Setup_Impl(UMaterialInstanceConstant* Material) override
    {
        if (Material)
        {
            Material->GetVectorParameterValue(ParameterInfos[0], Value);
            Material->GetVectorParameterDefaultValue(ParameterInfos[0], Default);
        }
    }

    virtual void UpdateMaterial_Impl(UMaterialInstanceConstant* Material) override
    {
        if (Material)
        {
            Material->SetVectorParameterValueEditorOnly(ParameterInfos[0], Value);
        }

        LastValue = Value;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    FLinearColor Value;
    FLinearColor LastValue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    FLinearColor Default;
};

UCLASS()
class UExTextMaterialParameterEditWidget_Texture : public UExTextMaterialParameterEditWidgetBase
{
    GENERATED_BODY()

public:

    virtual void ResetToDefaultImpl() override
    {
        Value = Default;
    }

    virtual bool HasChanged() const override
    {
        return Value != Default;
    }

    virtual bool IsDirty() const override
    {
        return Value != LastValue;
    }

    virtual void Setup_Impl(UMaterialInstanceConstant* Material) override
    {
        Material->GetTextureParameterValue(ParameterInfos[0], Value);
        Material->GetTextureParameterDefaultValue(ParameterInfos[0], Default);
    }


    virtual void UpdateMaterial_Impl(UMaterialInstanceConstant* Material) override
    {
        Material->SetTextureParameterValueEditorOnly(ParameterInfos[0], Value);
        LastValue = Value;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    UTexture* Value;
    UTexture* LastValue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    UTexture* Default;
};

UCLASS()
class UExTextMaterialParameterEditWidget_StaticSwitch : public UExTextMaterialParameterEditWidgetBase
{
    GENERATED_BODY()

public:

    virtual void ResetToDefaultImpl() override
    {
        Value = Default;
    }

    virtual bool HasChanged() const override
    {
        return Value != Default;
    }

    virtual bool IsDirty() const override
    {
        return Value != LastValue;
    }

    virtual void Setup_Impl(UMaterialInstanceConstant* Material) override
    {
        FGuid Id;
        Material->GetStaticSwitchParameterValue(ParameterInfos[0], Value, Id);
        Material->GetStaticSwitchParameterDefaultValue(ParameterInfos[0], Default, Id);
    }

    virtual void UpdateMaterial_Impl(UMaterialInstanceConstant* Material) override
    {
        FStaticParameterSet Parameters;
        Material->GetStaticParameterValues(Parameters);
        auto& StaticSwitchParameters = Guganana::Engine::GetStaticSwitchParameters(Parameters);
        if (auto* FoundSwitch = StaticSwitchParameters.FindByPredicate([this](const auto& Switch) { return Switch.ParameterInfo == ParameterInfos[0]; }))
        {
            if (FoundSwitch->Value != Value)
            {
                FoundSwitch->Value = Value;
                FoundSwitch->bOverride = true;
                Material->UpdateStaticPermutation(Parameters);
                Material->InitStaticPermutation();
            }
        }

        LastValue = Value;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    bool Value;
    bool LastValue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    bool Default;
};



UENUM(BlueprintType)
enum class EExText_MaterialMappingOptions : uint8
{
    PerCharacterOnX,
    PerCharacterOnY,
    PerCharacterStretch,
    PerRunOnX,
    PerRunOnY,
    PerRunStretch,
    ScreenOnX,
    ScreenOnY,
    ScreenStretch
};

UCLASS()
class UExTextMaterialParameterEditWidget_MappingOptions : public UExTextMaterialParameterEditWidgetBase
{
    GENERATED_BODY()

public:

    EExText_MaterialMappingOptions NameToMappingOption(FName Name)
    {
        FString OptionSanitizedName = Name.ToString().Replace(TEXT("MappingOption_"),TEXT(""));
        return static_cast<EExText_MaterialMappingOptions>( StaticEnum<EExText_MaterialMappingOptions>()->GetIndexByNameString(OptionSanitizedName) );
    }

    virtual bool IsDirty() const override
    {
        return Value != LastValue;
    }

    virtual void Setup_Impl(UMaterialInstanceConstant* Material) override
    {
        FGuid Id;

        for (const auto& ParameterInfo : ParameterInfos)
        {
            bool IsSwitchedOn = false;
            Material->GetStaticSwitchParameterValue(ParameterInfo, IsSwitchedOn, Id);

            if (IsSwitchedOn)
            {
                Value = NameToMappingOption(ParameterInfo.Name);
            }
        }
    }


    virtual void UpdateMaterial_Impl(UMaterialInstanceConstant* Material) override
    {
        FString ValueName = StaticEnum<EExText_MaterialMappingOptions>()->GetNameStringByIndex(static_cast<int32>(Value));

        FStaticParameterSet Parameters;
        Material->GetStaticParameterValues(Parameters);

        const int32 WorkingLayerIndex = ParameterInfos[0].Index;

        auto& StaticSwitchParameters = Guganana::Engine::GetStaticSwitchParameters(Parameters);
        if (auto* FoundSwitch = StaticSwitchParameters.FindByPredicate(
            [WorkingLayerIndex,ValueName](const auto& Switch)
            { 
                return Switch.ParameterInfo.Index == WorkingLayerIndex && Switch.ParameterInfo.ToString().Contains(ValueName);
            }))
        {
            if (FoundSwitch->Value == false)
            {
                // turn off all overrides
                for (auto& StaticSwitch : StaticSwitchParameters)
                {
                    // only mutate switches belonging to the same layer
                    if (StaticSwitch.ParameterInfo.Index == WorkingLayerIndex)
                    {
                        if (StaticSwitch.ParameterInfo.Name.ToString().StartsWith("MappingOption"))
                        {
                            StaticSwitch.Value = false;
                            StaticSwitch.bOverride = true;
                        }
                    }
                }

                FoundSwitch->Value = true;
                FoundSwitch->bOverride = true;
                Material->UpdateStaticPermutation(Parameters);
                Material->InitStaticPermutation();
            }
        }

        LastValue = Value;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Parameters")
    EExText_MaterialMappingOptions Value = EExText_MaterialMappingOptions::PerRunStretch;
    EExText_MaterialMappingOptions LastValue;
};
