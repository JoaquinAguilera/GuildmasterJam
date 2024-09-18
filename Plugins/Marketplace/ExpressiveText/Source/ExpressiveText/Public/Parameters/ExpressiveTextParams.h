// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "ExpressiveTextParameterValue.h"
#include "ExTextClearDirection.h"
#include "Asset/ExpressiveTextFont.h"
#include "Asset/ExpressiveTextMaterial.h"
#include "Materials/MaterialGeneration.h"
#include "ExpressiveTextParamsCategories.h"

#include <Blueprint/UserWidget.h>
#include <UObject/SoftObjectPtr.h>
#include <Guganana/Core.h>
#include <Misc/EngineVersionComparison.h>

#if !UE_VERSION_OLDER_THAN( 5, 0, 0 )
#include <UObject/ObjectSaveContext.h>
#endif

#include "ExpressiveTextParams.generated.h"

//-----------------------------------------------------------------------

#if WITH_EDITOR
#define CATEGORY(CategoryName) \
	virtual TSubclassOf<UExTextParameterCategory> GetParameterCategory() const override \
	{ \
		return UExTextParameterCategory_##CategoryName::StaticClass(); \
	}
#else
	#define CATEGORY(CategoryName) 
#endif

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Font"))
class EXPRESSIVETEXT_API UExTextValue_Font : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(Font)
	using ValueType = UExpressiveTextFont*;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Font, meta = (Tooltip = "Font to display") )
	UExpressiveTextFont* Value;

#if WITH_EDITORONLY_DATA
void NewFontDownloaded( FName FontName, UExpressiveTextFont* Font )
{
	if( FontName == StoredFontName )
	{
		Value = Font;
	}
}

#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
    virtual void PreSave( const class ITargetPlatform * TargetPlatform ) override
	{
		Super::PreSave(TargetPlatform);
		OnPreSave();
	}
#else
	virtual void PreSave(FObjectPreSaveContext SaveContext)
	{
		Super::PreSave(SaveContext);
		OnPreSave();
	}
#endif

	void OnPreSave()
	{
		if( IsValid(Value) )
		{
			StoredFontName = IsValid(Value) ? FName( *Value->GetName() ) : FName();
		}
	}
	
	virtual void PostLoad() override;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Font )
	FName StoredFontName;
#endif

#if WITH_EDITOR
	void EnsureFontIsSet() const;	
	virtual void ConfirmUsage() const override
	{
		EnsureFontIsSet();
	}
#endif
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Font Size"))
class EXPRESSIVETEXT_API UExTextValue_FontSize : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:

	CATEGORY(Font)
	using ValueType = int32;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = FontSize, meta = (ClampMin = "1", UIMin = "1", UIMax = "300", Tooltip = "Text size when using this style") )
	int32 Value = 24;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Letter Spacing"))
class EXPRESSIVETEXT_API UExTextValue_LetterSpacing : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:

	CATEGORY(Font)
	using ValueType = int32;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = LetterSpacing, meta = (UIMin = "-1000", UIMax = "1000", Tooltip = "Tweaks space between letters") )
	int32 Value = 0;

#if WITH_EDITOR
	virtual FString MoreInfoTooltip() const override
	{
		return TEXT("[icon,gold](warning) Letter Spacing is experimental and currently does not support Reveal Rate ");
	}
#endif
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Typeface"))
class EXPRESSIVETEXT_API UExTextValue_Typeface : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:

	CATEGORY(Font)
	using ValueType = FName;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Typeface, meta = (Tooltip = "Which typeface should be displayed (i.e Italic, Bold, BoldItalic, etc...) " ) )
	FName Value;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Font Color"))
class EXPRESSIVETEXT_API UExTextValue_FontColor : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:

	CATEGORY(Font)
	using ValueType = FLinearColor;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Font Color", meta = (Tooltip = "Color applied to the text" ) )
	FLinearColor Value = FLinearColor::White;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Reveal Rate"))
class EXPRESSIVETEXT_API UExTextValue_RevealRate : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(RevealAnimation)
	using ValueType = float;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reveal Rate", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "500.0", Tooltip = "How many characters should be revealed in each second that passes" ) )
	float Value;
};

//-----------------------------------------------------------------------

class UExpressiveTextAnimation;

USTRUCT(BlueprintType)
struct FExText_GlyphAnimation
{
	GENERATED_BODY()

	FExText_GlyphAnimation()
		: Animation(nullptr)
	{}

	FExText_GlyphAnimation( UExpressiveTextAnimation* Anim)
		: Animation(Anim)
	{}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (Tooltip = "Glyph animation we're going to play" ) )
	UExpressiveTextAnimation* Animation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, meta = (Tooltip = "Should the animation be played in reverse?" ))
	bool Reverse = false;
};

UCLASS(meta = (DisplayName = "Reveal Animation"))
class EXPRESSIVETEXT_API UExTextValue_RevealAnimation : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(RevealAnimation)
	using ValueType = FExText_GlyphAnimation;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reveal Animation", meta = (Tooltip = "Select which reveal animation should be played" ))
	FExText_GlyphAnimation Value;
};

//-----------------------------------------------------------------------

UCLASS(Abstract)
class EXPRESSIVETEXT_API UExTextValue_MaterialBase : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	using ValueType = TArray<UExpressiveTextMaterial*>;

	UExTextValue_MaterialBase()
		: Super()
		, CombinedMaterial()
		, MaterialsChecksum(0)
	{
	}


	virtual void GetMaterials(TArray<UExpressiveTextMaterial*>& OutMaterials) const { check(false); }

#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override
	{
		Super::PostEditChangeProperty(Event);

		TArray<UExpressiveTextMaterial*> Materials;
		GetMaterials(Materials);

		MaterialGeneration::ReconstructCombinedMaterial(this, CombinedMaterial, Materials);
		
		MaterialsChecksum = 0;
		for (int32 i = 0; i < Materials.Num(); i++)
		{
			const auto& Material = Materials[i];
			MaterialsChecksum = HashCombine(GetTypeHash(Material), MaterialsChecksum);
			MaterialsChecksum = HashCombine(i, MaterialsChecksum);
		}
	}

	virtual TSubclassOf<UUserWidget> DisplayExtraWidget() const override
	{
		return TSoftClassPtr<UUserWidget>(FSoftObjectPath(TEXT("/ExpressiveText/Core/Editor/ParameterExtras/BPExTextEditor_ParameterExtra_Materials.BPExTextEditor_ParameterExtra_Materials_C"))).LoadSynchronous();
	}
#endif

	UPROPERTY(BlueprintReadOnly, Category = MaterialBase)
	UMaterialInstanceConstant* CombinedMaterial;

	UPROPERTY(BlueprintReadOnly, Category = MaterialBase)
	int64 MaterialsChecksum;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Material"))
class EXPRESSIVETEXT_API UExTextValue_Material : public UExTextValue_MaterialBase
{
	GENERATED_BODY()

public:

	CATEGORY(Material)
	using ValueType = TArray<UExpressiveTextMaterial*>;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (Tooltip = "Which material that we're going to apply to the text outline - useful for shader animations"))
	TArray<UExpressiveTextMaterial*> Value;

	virtual void GetMaterials(TArray<UExpressiveTextMaterial*>& OutMaterials) const override
	{
		OutMaterials = Value;
	}
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Shadow Color"))
class EXPRESSIVETEXT_API UExTextValue_ShadowColor : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(Shadow)
	using ValueType = FLinearColor;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Shadow Color", meta = (Tooltip = "Color applied to the text's drop shadow - invisible by default" ))
	FLinearColor Value = FLinearColor( 0.f, 0.f, 0.f, 0.f );
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Shadow Offset"))
class EXPRESSIVETEXT_API UExTextValue_ShadowOffset : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(Shadow)
	using ValueType = FVector2D;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Shadow Offset", meta = (Tooltip = "The amount of offset we're going to add to the drop shadow" ) )
	FVector2D Value = FVector2D( 1.f, 1.f );
};

//-----------------------------------------------------------------------

UCLASS( meta=(DisplayName = "Reveal Animation Duration") )
class EXPRESSIVETEXT_API UExTextValue_AnimationDuration : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(RevealAnimation)
	using ValueType = float;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reveal Animation Duration", meta = (ClampMin = "0.0001", UIMin = "0.0001", UIMax = "8.0", Tooltip = "How long should the reveal animation take (in seconds)" ) )
	float Value = 1.0f;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Reveal Animation Loop Period"))
class EXPRESSIVETEXT_API UExTextValue_AnimationLoopPeriod : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(RevealAnimation)
	using ValueType = float;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reveal Animation Loop Period", meta = (UIMin = "0.0001", UIMax = "120.0", Tooltip = "After how long should the animation loop and start again (in seconds) - negative values mean it does not loop" ) )
	float Value = -1.f;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Per Character Action"))
class EXPRESSIVETEXT_API UExTextValue_PerCharacterAction : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	using ValueType = class UExText_ActionBase*;

	UPROPERTY( Instanced, EditAnywhere, BlueprintReadWrite, Category = "Per Character Action", meta = (Tooltip = "Action to fire each time a character is revealed") )
	class UExText_ActionBase* Value;

#if WITH_EDITOR
	virtual FString MoreInfoTooltip() const override
	{
		return TEXT(" [icon,gold](warning) Per Character Action only works when Reveal Rate is greater than 0");
	}
#endif
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Clear Timer"))
class EXPRESSIVETEXT_API UExTextValue_ClearTimer : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(ClearAnimation)
	using ValueType = float;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Clear Timer", meta = (ClampMin = "-1.0", UIMin = "-1.0", UIMax = "120.0", Tooltip = "How long to wait before we start clearing the text (in seconds). Timer only starts counting after the entire text is revealed. \nNegative values mean text is not cleared") )
	float Value = -1.f;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Clear Direction"))
class EXPRESSIVETEXT_API UExTextValue_ClearDirection: public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(ClearAnimation)
	using ValueType = EExText_ClearDirection;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Clear Direction", meta = (Tooltip = "Direction we should clear the text (Backwards/Forwards)" ) )
	EExText_ClearDirection Value = EExText_ClearDirection::Forwards;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Clear Rate"))
class EXPRESSIVETEXT_API UExTextValue_ClearRate : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(ClearAnimation)
	using ValueType = float;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Clear Rate", meta = (Tooltip = "How fast to clear the text (how many characters per second)" ) )
	float Value = 0.f;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Clear Animation"))
class EXPRESSIVETEXT_API UExTextValue_ClearAnimation : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(ClearAnimation)
	using ValueType = FExText_GlyphAnimation;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Clear Animation", meta = (Tooltip = "Select which clear animation should be played" ) )
	FExText_GlyphAnimation Value;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Clear Animation Duration"))
class EXPRESSIVETEXT_API UExTextValue_ClearAnimationDuration : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(ClearAnimation)
	using ValueType = float;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ClearAnimationDuration, meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "20.0", Tooltip = "How long the clear animation should play for (in seconds)" ) )
	float Value = 1.f;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Offset"))
class EXPRESSIVETEXT_API UExTextValue_Offset : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	using ValueType = FVector2D;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Offset )
	FVector2D Value = FVector2D::ZeroVector;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Outline Color"))
class EXPRESSIVETEXT_API UExTextValue_OutlineColor : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(Outline)
	using ValueType = FLinearColor;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = OutlineColor )
	FLinearColor Value;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Outline Material"))
class EXPRESSIVETEXT_API UExTextValue_OutlineMaterial : public UExTextValue_MaterialBase
{
	GENERATED_BODY()
public:

	CATEGORY(Outline)
	using ValueType = TArray<UExpressiveTextMaterial*>;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Outline Material", meta = (Tooltip = "Which material that we're going to apply to the text outline - useful for shader animations"))
	TArray<UExpressiveTextMaterial*> Value;

	virtual void GetMaterials(TArray<UExpressiveTextMaterial*>& OutMaterials) const override
	{
		OutMaterials = Value;
	}

};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Outline Size"))
class EXPRESSIVETEXT_API UExTextValue_OutlineSize : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(Outline)
	using ValueType = int32;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = OutlineSize )
	int32 Value;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Outline on Drop Shadows"))
class EXPRESSIVETEXT_API UExTextValue_OutlineOnDropShadows : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(Outline)
	using ValueType = bool;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = OutlineOnDropShadows )
	bool Value;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Draw Outline as Separate Layer"))
class EXPRESSIVETEXT_API UExTextValue_DrawOutlineAsSeparateLayer : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(Outline)
	using ValueType = bool;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DrawOutlineAsSeparateLayer )
	bool Value;
};

//-----------------------------------------------------------------------
// This enum should match ESlateDrawEffect 1-to-1
UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EExTextDrawBlendEffect : int32
{
	/** No effect applied */
	None					= 0,
	/** Advanced: Draw the element with no blending */
	NoBlending			= 1 << 0,
	/** Advanced: Blend using pre-multiplied alpha. Ignored if NoBlending is set. */
	PreMultipliedAlpha	= 1 << 1,
	/** Advanced: No gamma correction should be done */
	NoGamma				= 1 << 2,
	/** Advanced: Change the alpha value to 1 - Alpha. */
	InvertAlpha			= 1 << 3,
	
	/** Disables pixel snapping */
	NoPixelSnapping		= 1 << 4,
	/** Draw the element with a disabled effect */
	DisabledEffect		= 1 << 5,
	/** Advanced: Don't read from texture alpha channel */
	IgnoreTextureAlpha	= 1 << 6,

	/** Advanced: Existing Gamma correction should be reversed */
	ReverseGamma			= 1 << 7
};

UCLASS(meta = (DisplayName = "Draw Blend Effect"))
class EXPRESSIVETEXT_API UExTextValue_DrawBlendEffect : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	using ValueType = int32;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = DrawBlendEffect, Meta = (Bitmask, BitmaskEnum = "/Script/ExpressiveText.EExTextDrawBlendEffect") )
	int32 Value;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Outline Blur Amount"))
class EXPRESSIVETEXT_API UExTextValue_OutlineBlurAmount : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	CATEGORY(Outline)
	using ValueType = float;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = OutlineBlurAmount )
	float Value;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Force Full Text Shaping Method"))
class EXPRESSIVETEXT_API UExTextValue_ForceFullTextShapingMethod : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	using ValueType = bool;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ForceFullTextShapingMethod )
	bool Value;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Force Draw Each Glyph Separately"))
class EXPRESSIVETEXT_API UExTextValue_ForceDrawEachGlyphSeparately : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	using ValueType = bool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ForceDrawEachGlyphSeparately)
	bool Value;
};

//-----------------------------------------------------------------------

UCLASS(meta = (DisplayName = "Percentage Offset"))
class EXPRESSIVETEXT_API UExTextValue_PercentageOffset : public UExpressiveTextParameterValue
{
	GENERATED_BODY()

public:
	using ValueType = FVector2D;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = PercentageOffset )
	FVector2D Value = FVector2D::ZeroVector;
};

//-----------------------------------------------------------------------

#undef CATEGORY