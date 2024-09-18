// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#if WITH_EDITOR
#include <Editor/CurveAssetEditor/Public/CurveAssetEditorModule.h> // needs to be moved to editor only module
#endif

#include <Curves/CurveVector.h>
#include <Curves/CurveLinearColor.h>
#include <Curves/CurveFloat.h>
#include <Engine/DataAsset.h>
#include <Fonts/SlateFontInfo.h>
#include <GenericPlatform/GenericPlatformMath.h>
#include <Misc/EngineVersionComparison.h>
#include <Misc/FrameValue.h>
#include <Modules/ModuleManager.h>
#include <Slate/WidgetTransform.h>

#include "ExpressiveTextAnimation.generated.h"


struct FExTextAnimationPayload {

    FExTextAnimationPayload()
        : WidgetTransform( FVector2D::ZeroVector, FVector2D(1.f, 1.f), FVector2D::ZeroVector, 0.f )
        , FontColor( FLinearColor::White )
        , DropShadowColor( FLinearColor::Transparent )
        , DropShadowOffset( FVector2D::ZeroVector )
        , Pivot( 0.5f, 0.5f )
        , ClipAmount( FVector2D::ZeroVector )
        , OutlineRenderSize( 1.f, 1.f )
        , Opacity( 1.f )
        , BlurAmount( 0.f )
        , BackgroundBlurAmount( 0.f )
        , Scale( 1.f )
        , MaxDisplayedScale( 1.f )
        , OutlineSize( 0.f )
        , LetterSpacing( 0 )
        , ShouldClip( false )
        , ShouldBlur( false )
        , ShouldBackgroundBlur( false )
    {}

    FWidgetTransform WidgetTransform;
    FLinearColor FontColor;
    FLinearColor DropShadowColor;
    FVector2D DropShadowOffset;
    FVector2D Pivot;
    FVector2D ClipAmount;
    FVector2D OutlineRenderSize;
    float Opacity;
    float BlurAmount;
    float BackgroundBlurAmount;
    float Scale;
    float MaxDisplayedScale;
    float OutlineSize;
    int LetterSpacing;
    bool ShouldClip;
    bool ShouldBlur;
    bool ShouldBackgroundBlur;
};

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EExTextAnimationFlags : int32
{
    PositionIsPercentageOfSize = 1
};

UCLASS(BlueprintType)
class EXPRESSIVETEXT_API UExpressiveTextAnimation : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

    void Evaluate( const FVector2D& BlockSize, float TimePassed, float AnimationDuration, FExTextAnimationPayload& OutAnimPayload, FSlateFontInfo& FontInfo, float FontSizeCompensation, bool IsReverse)
    {
		float Time = ReinterpretTime(TimePassed, AnimationDuration);
        
        if(IsReverse)
        {
            Time = GetAnimationDuration() - Time;
        }


		if (Scale)
		{
			float Min;
			Scale->FloatCurve.GetValueRange(Min, OutAnimPayload.MaxDisplayedScale);
            OutAnimPayload.MaxDisplayedScale = FMath::Max(OutAnimPayload.MaxDisplayedScale, 0.0001f);
            OutAnimPayload.Scale = Scale->GetFloatValue(Time);
		}
		if (Position)
		{
            FVector2D EvaluatedPos = FVector2D(Position->GetVectorValue(Time));
            if( AnimationFlags & (int32)EExTextAnimationFlags::PositionIsPercentageOfSize )
            {
                OutAnimPayload.WidgetTransform.Translation = BlockSize * EvaluatedPos * OutAnimPayload.Scale;
		    }
            else
            {
			    OutAnimPayload.WidgetTransform.Translation = EvaluatedPos * FontSizeCompensation;
            }
        }
		if (Shear)
		{
            OutAnimPayload.WidgetTransform.Shear = FVector2D(Shear->GetVectorValue(Time));
		}
		if (Size)
		{
            OutAnimPayload.WidgetTransform.Scale = FVector2D(Size->GetVectorValue(Time));
		}
		if (Angle)
		{
            OutAnimPayload.WidgetTransform.Angle = Angle->GetFloatValue(Time);
		}
        if (Pivot)
        {
            OutAnimPayload.Pivot = FVector2D(Pivot->GetVectorValue(Time));
        }
		if (Color)
		{
            OutAnimPayload.FontColor = Color->GetLinearColorValue(Time);
		}
		if (OutlineColor)
		{
            FontInfo.OutlineSettings.OutlineColor = OutlineColor->GetLinearColorValue(Time);
		}        
		if (OutlineSize)
		{
            OutAnimPayload.OutlineSize = OutlineSize->GetFloatValue(Time) * FontSizeCompensation;
            FontInfo.OutlineSettings.OutlineSize = OutAnimPayload.OutlineSize;
		}
        if( OutlineRenderSize )
        {
            OutAnimPayload.OutlineRenderSize = FVector2D(OutlineRenderSize->GetVectorValue(Time));
        }
		if (ShadowColor)
		{
            OutAnimPayload.DropShadowColor = ShadowColor->GetLinearColorValue(Time);
		}
        if (ShadowOffset)
        {
            OutAnimPayload.DropShadowOffset = FVector2D(ShadowOffset->GetVectorValue(Time)) * FontSizeCompensation;
        }
		if (ClippingAmount)
		{
            OutAnimPayload.ShouldClip = true;
            OutAnimPayload.ClipAmount = FVector2D(ClippingAmount->GetVectorValue(Time));
		}
        if (BlurAmount)
        {
            OutAnimPayload.ShouldBlur = true;
            OutAnimPayload.BlurAmount = BlurAmount->GetFloatValue(Time) * FontSizeCompensation;
        }
        
        if (BackgroundBlurAmount)
        {
            OutAnimPayload.ShouldBackgroundBlur = true;
            OutAnimPayload.BackgroundBlurAmount = BackgroundBlurAmount->GetFloatValue(Time) * FontSizeCompensation;
        }
        if (Opacity)
        {
            OutAnimPayload.Opacity = Opacity->GetFloatValue(Time);
            OutAnimPayload.FontColor.A = FMath::Min(OutAnimPayload.FontColor.A, OutAnimPayload.Opacity);
            OutAnimPayload.DropShadowColor.A = FMath::Min(OutAnimPayload.DropShadowColor.A, OutAnimPayload.Opacity);
            FontInfo.OutlineSettings.OutlineColor.A = FMath::Min(FontInfo.OutlineSettings.OutlineColor.A, OutAnimPayload.Opacity);
        }
        if (LetterSpacing)
        {
            OutAnimPayload.LetterSpacing = LetterSpacing->GetFloatValue(Time);
        }

    }


    float ReinterpretTime(float CurrentTime, float DesiredAnimationDuration)
    {
        if (DesiredAnimationDuration <= 0.f)
        {
            return 0.f;
        }

        const float NormalizedTime = FMath::Clamp( CurrentTime / DesiredAnimationDuration, 0.f, 1.f );

        return NormalizedTime * GetAnimationDuration();
    }

    float GetAnimationDuration()
    {
        if (AnimationDurationCache.IsSet())
        {
            return AnimationDurationCache.GetValue();
        }

        float Duration = 0.f;

        Duration = FMath::Max(Duration, GetLongestAxisDuration(Opacity, 1));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(Scale, 1));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(Size, 2));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(Angle, 1));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(Pivot, 3));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(LetterSpacing, 1));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(Shear, 2));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(Color, 4));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(Position, 3));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(OutlineColor, 4));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(OutlineSize, 1));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(OutlineRenderSize, 2));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(ShadowOffset, 3));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(ShadowColor, 4));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(ClippingAmount, 3));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(BlurAmount, 1));
        Duration = FMath::Max(Duration, GetLongestAxisDuration(BackgroundBlurAmount, 1));

        AnimationDurationCache = Duration;
        return Duration;
    }

    template< typename CurveType >
    float GetLongestAxisDuration(const CurveType* Curve, uint8 NumAxis)
    {
        float LongestDuration = 0.f;

        if (Curve)
        {
            for (uint8 i = 0; i < NumAxis; i++)
            {
                const auto& CurveForAxis = GetCurveForAxis(Curve, i);
                if (!CurveForAxis.IsEmpty())
                {
                    LongestDuration = FMath::Max(LongestDuration, CurveForAxis.GetLastKey().Time);
                }
            }
        }

        return LongestDuration;
    }

    template< typename CurveType >
    const FRichCurve& GetCurveForAxis(const CurveType* Curve, uint8 AxisIndex)
    {
        return Curve->FloatCurves[AxisIndex];
    }

    
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = AnimationFlags, Meta = (Bitmask, BitmaskEnum = "/Script/ExpressiveText.EExTextAnimationFlags") )
	int32 AnimationFlags;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Opacity)
	UCurveFloat* Opacity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Scale)
    UCurveFloat* Scale;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Size)
    UCurveVector* Size;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = Position)
    UCurveVector* Position;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = LetterSpacing)
    UCurveFloat* LetterSpacing;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Angle)
    UCurveFloat* Angle;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Shear)
    UCurveVector* Shear;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Pivot)
    UCurveVector* Pivot;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Color)
    UCurveLinearColor* Color;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = OutlineColor)
    UCurveLinearColor* OutlineColor;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = OutlineSize)
    UCurveFloat* OutlineSize;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = OutlineRenderSize)
    UCurveVector* OutlineRenderSize;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ShadowOffset)
    UCurveVector* ShadowOffset;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ShadowColor)
    UCurveLinearColor* ShadowColor;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = ClippingAmount)
    UCurveVector* ClippingAmount;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = BlurAmount)
    UCurveFloat* BlurAmount;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = BackgroundBlurAmount)
    UCurveFloat* BackgroundBlurAmount;

#if WITH_EDITOR
    UFUNCTION(BlueprintCallable, Category = ExTextAnimation)
    void OpenCurveAsset(const FName& CurveName )
    {

#if UE_VERSION_OLDER_THAN( 4, 25, 0 )
        auto* CurveProp = FindField<UObjectProperty>(GetClass(), CurveName);
#else
        auto* CurveProp = FindFProperty<FObjectProperty>(GetClass(), CurveName);
#endif
        if (CurveProp)
        {
            if (auto* CastedCurve = Cast<UCurveBase>(CurveProp->GetObjectPropertyValue_InContainer(this)) )
            {
                FCurveAssetEditorModule& CurveAssetEditorModule = FModuleManager::LoadModuleChecked<FCurveAssetEditorModule>("CurveAssetEditor");
                CurveAssetEditorModule.CreateCurveAssetEditor(EToolkitMode::Standalone, nullptr, CastedCurve );
            }
        }
    }

    UFUNCTION(BlueprintCallable, Category = ExTextAnimation)
    UCurveBase* GetCurveAsset(const FName& CurveName)
    {
#if UE_VERSION_OLDER_THAN( 4, 25, 0 )
        auto* CurveProp = FindField<UObjectProperty>(GetClass(), CurveName);
#else
        auto* CurveProp = FindFProperty<FObjectProperty>(GetClass(), CurveName);
#endif
        if (CurveProp)
        {
            return Cast<UCurveBase>(CurveProp->GetObjectPropertyValue_InContainer(this));
        }        
        
        return nullptr;
    }

    UFUNCTION(BlueprintCallable, Category = ExTextAnimation)
    void SetCurveAsset(const FName& CurveName, bool CreateNew)
    {
#if UE_VERSION_OLDER_THAN( 4, 25, 0 )
        auto* CurveProp = FindField<UObjectProperty>(GetClass(), CurveName);
#else
        auto* CurveProp = FindFProperty<FObjectProperty>(GetClass(), CurveName);
#endif
        if (CurveProp)
        {
            UCurveBase* DesiredValue = nullptr;

            if(CreateNew)
            {
                const FString NewCurveName = FString::Printf(TEXT("%s - %s"), *GetNameSafe(this), *CurveName.ToString());
                DesiredValue = NewObject<UCurveBase>(this, CurveProp->PropertyClass, FName(*NewCurveName));
            }

            CurveProp->SetObjectPropertyValue_InContainer(this, DesiredValue);
        }
    }

    UFUNCTION(BlueprintCallable, Category = ExTextAnimation)
    static void GetAllAnimationCurves(TArray<FName>& OutCurveNames)
    {
        OutCurveNames.Empty();

        if (UClass* Class = ThisClass::StaticClass())
        {

#if UE_VERSION_OLDER_THAN( 4, 26, 0 )
            TFieldIterator<UProperty> It(Class);
#else
            TFieldIterator<FProperty> It(Class);
#endif

            for (; It; ++It)
            {
                if (auto* Prop = *It)
                {

#if UE_VERSION_OLDER_THAN( 4, 26, 0 )
                    if (auto* ObjProp = Cast<UObjectProperty>(Prop))
#else
                    if (auto* ObjProp = CastField<FObjectProperty>(Prop))
#endif
                    {
                        if (ObjProp->PropertyClass && ObjProp->PropertyClass->IsChildOf(UCurveBase::StaticClass() ) )
                        {
                            OutCurveNames.Add(ObjProp->GetFName());
                        }
                    }
                }
            }
        }
    }
#endif

    TFrameValue<float> AnimationDurationCache;
};


template<>
inline const FRichCurve& UExpressiveTextAnimation::GetCurveForAxis<UCurveFloat>(const UCurveFloat* Curve, uint8 AxisIndex)
{
    return Curve->FloatCurve;
}
