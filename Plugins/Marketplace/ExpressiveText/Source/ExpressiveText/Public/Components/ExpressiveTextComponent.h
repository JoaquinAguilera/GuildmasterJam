// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Blueprint/UserWidget.h>
#include <Components/StaticMeshComponent.h>
#include <Components/WidgetComponent.h>
#include <Engine/StaticMesh.h>
#include <Materials/MaterialInstanceDynamic.h>

#include "Components/ExpressiveTextLightMode.h"
#include "ExpressiveTextSettings.h"
#include "Handles/ExpressiveTextSelector.h"
#include "Widgets/ExpressiveTextWidget.h"

#include "ExpressiveTextComponent.generated.h"

UCLASS()
class EXPRESSIVETEXT_API UExpressiveTextWidgetComponent : public UWidgetComponent
{
    GENERATED_BODY()
public:
    UExpressiveTextWidgetComponent();
        
    void EnsureWidgetCreated();

    virtual void BeginPlay() override;

    virtual void OnRegister() override;

    virtual void OnUnregister() override;

    void SetTimingPolicy(EWidgetTimingPolicy InTimingPolicy)
    {
        TimingPolicy = InTimingPolicy;
    }
    
    UPROPERTY(Transient)
    UExpressiveTextWidget* UserWidget;
};

UINTERFACE()
class UExpressiveTextWidgetInterface : public UInterface
{
    GENERATED_BODY()
};

class IExpressiveTextWidgetInterface
{
    GENERATED_BODY()
public:

    UFUNCTION(BlueprintImplementableEvent, Category = ExpressiveTextWidgetInterface)
    void SetText( const FExpressiveTextSelector& Text );
};

UCLASS( Blueprintable, meta = (BlueprintSpawnableComponent) )
class EXPRESSIVETEXT_API UExpressiveTextComponent : public USceneComponent
{
    GENERATED_BODY()
public:

    UExpressiveTextComponent()
        : Super()
        , WidgetComponent()
        , Text()
        , Space( EWidgetSpace::World )
        , LightMode( EExpressiveTextLightMode::Lit )
        , EmissiveIntensity( 1.f )
        , TimingPolicy(EWidgetTimingPolicy::RealTime )
        , Resolution( 500.f, 500.f )
        , ShouldRenderWhenOffscren( false )
        , CastShadow(true)
        , LightModeMaterialsAsset()
        #if WITH_EDITORONLY_DATA
        , ShowPreviewBackground( true )
        , SoftPlaneMesh( FSoftObjectPath(TEXT("/ExpressiveText/Core/Meshes/Plane.Plane")) )
        , PreviewPlaneMesh( nullptr )
        #endif
    {
        PrimaryComponentTick.bTickEvenWhenPaused = true;
        PrimaryComponentTick.bCanEverTick = true;
    }

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        EnsureWidgetCreated();
    }

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override
    {
        Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
        UpdateWidget();
        UpdatePlane();

#if WITH_EDITOR
        if (PreviewPlaneMesh)
        {
            if( auto* World = GetWorld() )
            {
                ForceUniformScalling();

#if UE_VERSION_OLDER_THAN(4,26,0)
                PreviewPlaneMesh->SetScalarParameterValueOnMaterials("LastUpdate", FApp::GetCurrentTime() - GStartTime);
#else
                PreviewPlaneMesh->SetScalarParameterValueOnMaterials("LastUpdate", World->GetUnpausedTimeSeconds());
#endif
            }
        }
#endif
    }

    void UpdateWidget()
    {
        if (WidgetComponent)
        {
            if( auto* Material = WidgetComponent->GetMaterialInstance() )
            {
                Material->SetScalarParameterValue( TEXT("EmissiveIntensity"), EmissiveIntensity);
            }

            if (auto* Widget = WidgetComponent->UserWidget)
            {
                IExpressiveTextWidgetInterface::Execute_SetText(Widget, Text);
            }
        }
    }

    void UpdatePlane()
    {
#if WITH_EDITOR
        if (PreviewPlaneMesh)
        {
            PreviewPlaneMesh->SetVisibility(ShowPreviewBackground);

            FVector UpdatedScale(Resolution.X / 100.f, Resolution.Y / 100.f, 1.f);
            PreviewPlaneMesh->SetRelativeScale3D(UpdatedScale);
        }
#endif
    }

    virtual void OnRegister() override
    {
        Super::OnRegister();
        CreatePlaneVisualization();
    }

    virtual void OnUnregister() override
    {
        Super::OnUnregister();

        if( WidgetComponent != nullptr )
        {
            WidgetComponent->UnregisterComponent();
            WidgetComponent->ConditionalBeginDestroy();
            WidgetComponent = nullptr;
        }

#if WITH_EDITOR
        if (PreviewPlaneMesh)
        {
            PreviewPlaneMesh->UnregisterComponent();
            PreviewPlaneMesh->ConditionalBeginDestroy();
            PreviewPlaneMesh = nullptr;
        }
#endif
    }

    void EnsureWidgetCreated()
    {        
        if (WidgetComponent == nullptr)
        {
            static int32 GlobalCounter = 0;
            
            FString ObjectName = FString::Printf(TEXT("ExpressiveTextWidgetComponent_%d"), GlobalCounter++);
            WidgetComponent = NewObject<UExpressiveTextWidgetComponent>(this, *ObjectName);
            WidgetComponent->SetWidgetSpace(Space);
            WidgetComponent->SetTimingPolicy(TimingPolicy);
            WidgetComponent->SetDrawSize(FVector2D(Resolution.X, Resolution.Y));
            WidgetComponent->SetTickWhenOffscreen( ShouldRenderWhenOffscren );
            WidgetComponent->SetCastShadow(CastShadow);
            WidgetComponent->SetVisibility(GetVisibleFlag(), true);
            WidgetComponent->SetHiddenInGame(bHiddenInGame, true);

            WidgetComponent->RegisterComponent();
            WidgetComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
            WidgetComponent->SetCollisionEnabled( ECollisionEnabled::NoCollision );

            if (auto* Material = FetchMaterialForLightMode(LightMode))
            {
                WidgetComponent->SetMaterial(0, Material);
            }

            // Screen space widgets do not work in preview
            if (Space != EWidgetSpace::Screen)
            {
                WidgetComponent->EnsureWidgetCreated();
                UpdateWidget();
            }
        }
    }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
        //ForceUniformScalling();
    }
#endif

    UPROPERTY( Transient )
    UExpressiveTextWidgetComponent* WidgetComponent;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Expressive Text" )
    FExpressiveTextSelector Text;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Widget Settings" )
    EWidgetSpace Space;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Widget Settings" )
    EExpressiveTextLightMode LightMode;
    
    UPROPERTY( EditAnywhere, Interp, BlueprintReadWrite, Category = "Widget Settings", meta = ( EditCondition = "LightMode == EExpressiveTextLightMode::Emissive || LightMode == EExpressiveTextLightMode::Additive" ) )
    float EmissiveIntensity;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Widget Settings" )
    EWidgetTimingPolicy TimingPolicy;
    
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Widget Settings" )
    FIntPoint Resolution;
    
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Widget Settings" )
    bool ShouldRenderWhenOffscren;
        
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Widget Settings" )
    bool CastShadow;

    UFUNCTION(BlueprintCallable, Category = "Expressive Text Component")
    void SkipReveal() const
    {
        if( WidgetComponent )
        {
            if( auto* Widget = WidgetComponent->UserWidget )
            {
                Widget->SkipReveal();
            }
        }
    }

    UFUNCTION(BlueprintCallable, Category = "Expressive Text Component")
    void SetCastShadows(bool Value)
    {
        CastShadow = Value;
        if (WidgetComponent)
        {
            WidgetComponent->SetCastShadow(CastShadow);
        }
    }

    UFUNCTION(BlueprintCallable, Category = "Expressive Text Component")
    void SetTextVisibility(bool Value)
    {
        SetVisibility(Value);
        if (WidgetComponent)
        {
            WidgetComponent->SetVisibility(Value,true);
        }
    }

    UFUNCTION(BlueprintCallable, Category = "Expressive Text Component")
    void SetTextHiddenInGame(bool Value)
    {
        SetHiddenInGame(Value);
        if (WidgetComponent)
        {
            WidgetComponent->SetHiddenInGame(bHiddenInGame, true);
        }
    }

    UFUNCTION(BlueprintPure, Category = "Expressive Text Component")
	void GetChronos( FExpressiveTextChronos& OutChronos ) const
	{
        if( WidgetComponent )
        {
            if( auto* Widget = WidgetComponent->UserWidget )
            {
		        Widget->GetChronos(OutChronos);
            }
        }
	}

    UFUNCTION( BlueprintPure, Category = "Expressive Text Component" )
    UTextureRenderTarget2D* GetRenderTarget() const
    {
        if( WidgetComponent )
        {
            return WidgetComponent->GetRenderTarget();
        }
        return nullptr;
    }

    UFUNCTION(BlueprintPure, Category = "Expressive Text Component")
    UExpressiveTextWidgetComponent* GetWidgetComponent() const
    {
        return WidgetComponent;
    }

private:

#if WITH_EDITOR
    void ForceUniformScalling()
    {
        FVector OriginalScale = GetComponentScale();
        float ChangedScaleValue = GetChangedAxisValue(OriginalScale);
        SetWorldScale3D( FVector(ChangedScaleValue) );
    }
#endif

    UMaterialInterface* FetchMaterialForLightMode( EExpressiveTextLightMode InLightMode )
    {
        if( !IsValid( LightModeMaterialsAsset ) )
        {
            if (auto* LoadedLightModeMaterialsAsset = GetDefault<UExpressiveTextSettings>()->LightModeMaterialsAsset.LoadSynchronous())
            {
                LightModeMaterialsAsset = LoadedLightModeMaterialsAsset;
            }
        }

        if( !IsValid(LightModeMaterialsAsset) )
        {
            ensure(false); //! Unable to load light mode materials
            return nullptr;
        }

        if( auto* Material = LightModeMaterialsAsset->LightModeMaterials.Find(InLightMode) )
        {
            return *Material;
        }
        
        //! Failed to find material for Light Mode - ensure map is correctly populated
        ensure(false);     
        return nullptr;
    }

    UPROPERTY( Transient )
    UExpressiveTextLightModeMaterialsAsset* LightModeMaterialsAsset;

#if WITH_EDITORONLY_DATA
    UPROPERTY( EditAnywhere, Category = "Widget Settings", meta=( DisplayPriority = "1") )
    bool ShowPreviewBackground;

    TSoftObjectPtr<UStaticMesh> SoftPlaneMesh;
    
    UPROPERTY(Transient)
    UStaticMeshComponent* PreviewPlaneMesh;    
#endif

    void CreatePlaneVisualization()
    {
#if WITH_EDITOR
        if( UWorld* World = GetWorld() )
        {
            if( PreviewPlaneMesh )
            {
                return;
            }

            if( World->WorldType != EWorldType::Editor )
            {
                return;
            }

            if (Space == EWidgetSpace::World)
            {
                if( auto* LoadedMesh = SoftPlaneMesh.LoadSynchronous() )
                {
                    PreviewPlaneMesh = NewObject<UStaticMeshComponent>(this, TEXT("PlaneVisualization"));
                    PreviewPlaneMesh->SetStaticMesh(LoadedMesh);
                    PreviewPlaneMesh->RegisterComponent();
                    PreviewPlaneMesh->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
                    PreviewPlaneMesh->AddLocalOffset(FVector(-1.f, 0.f, 0.f));
                    PreviewPlaneMesh->SetRelativeRotation(FRotator(180.f, 90.f, -90.f));
                    PreviewPlaneMesh->SetHiddenInGame(true);
                    PreviewPlaneMesh->SetCollisionEnabled( ECollisionEnabled::NoCollision );

                    UpdatePlane();
                }
            }
        }
#endif
    }

    static float GetChangedAxisValue( FVector Vector )
    {
        if ( Vector.X == Vector.Y )
        {
            return Vector.Z;
        }

        if( Vector.X == Vector.Z )
        {
            return Vector.Y;
        }

        return Vector.X;
    }
};
