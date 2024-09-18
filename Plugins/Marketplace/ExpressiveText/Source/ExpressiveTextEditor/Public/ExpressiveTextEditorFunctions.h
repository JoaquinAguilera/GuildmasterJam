// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "ExpressiveTextEditorWidgetInterface.h"

#include <EditorUtilityWidget.h>
#include <EditorUtilityWidgetBlueprint.h>
#include <Editor.h>
#include <EditorUtilitySubsystem.h>
#include <EditorUtilityWidgetBlueprint.h>
#include <Engine/GameEngine.h>
#include <Engine/UserInterfaceSettings.h>
#include <ExpressiveText/Public/Parameters/ExpressiveTextParameterValue.h>
#include <Framework/Application/SlateApplication.h>
#include <Guganana/Engine.h>
#include <HAL/PlatformApplicationMisc.h>
#include <IBlutilityModule.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include <LevelEditor.h>
#include <LevelEditorViewport.h>
#include <Misc/EngineVersionComparison.h>
#include <Modules/ModuleManager.h>
#include <WidgetBlueprint.h>
#include <Widgets/Layout/SDPIScaler.h>
#include <Widgets/SOverlay.h>
#include <Widgets/SViewport.h>
#include <Widgets/SWidget.h>
#include <Templates/SharedPointer.h>

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 24
#include <ILevelViewport.h>
#else
#include <IAssetViewport.h>
#endif

#if UE_VERSION_OLDER_THAN( 5, 1, 0 )
#include <AssetRegistryModule.h>
#include <AssetData.h>
#else
#include <AssetRegistry/AssetRegistryModule.h>
#include <AssetRegistry/AssetData.h>
#endif

#include "ExpressiveTextEditorFunctions.generated.h"

USTRUCT( BlueprintType )
struct FOverlayWidgetHandle
{    
    GENERATED_BODY()
    
    FOverlayWidgetHandle()
        : Widget()
    {}

    TSharedPtr<SWidget> Widget;
};

UCLASS()
class UExpressiveTextEditorFunctions : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public: 

    UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static void RemoveViewportOverlay( UPARAM(ref) FOverlayWidgetHandle& WidgetHandle )
    {
        if( WidgetHandle.Widget )
        {
            FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
            auto ActiveLevelViewport = LevelEditor.GetFirstActiveViewport();

            if ( ActiveLevelViewport.IsValid() )
            {
                ActiveLevelViewport->RemoveOverlayWidget( WidgetHandle.Widget.ToSharedRef() );
            }

            WidgetHandle.Widget.Reset();
        }
    }

    UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static FOverlayWidgetHandle AddViewportOverlay( UUserWidget* Widget )
    {
        FOverlayWidgetHandle Result;

        if( Widget )
        {
            FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
            auto ActiveLevelViewport = LevelEditor.GetFirstActiveViewport();

            if (ActiveLevelViewport.IsValid())
            {
                auto Content = Widget->TakeWidget();

                TSharedRef<SDPIScaler> Wrapper = SNew(SDPIScaler)
                    .DPIScale_Lambda([] { return UExpressiveTextEditorFunctions::GetDPIScalingBasedOnSize(); } )
                    [
                        Content
                    ];

                Result.Widget = Wrapper;
                ActiveLevelViewport->AddOverlayWidget(Wrapper);
            }
        }

        return Result;
    }

	UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static float GetPlatformDPIScaling()
    {
		return FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(0, 0);
	}

	UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static void GetAllParameterValueClasses( TArray<TSubclassOf<UExpressiveTextParameterValue>>& OutClasses )
    {
		OutClasses.Empty();
        
	    TArray<UClass*> Results;
		GetDerivedClasses(UExpressiveTextParameterValue::StaticClass(), Results, true);

        for( UClass* Class : Results )
        {
            if( Class )
            {
                OutClasses.Emplace( Class );
            }
        }
	}

	UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static FVector2D GetEditorViewportSize()
    {
		FVector2D ViewSize = GCurrentLevelEditingViewportClient->Viewport->GetSizeXY();
		return ViewSize * ( 1.f/ GetPlatformDPIScaling() );
    }

    UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static FVector2D GetEditorViewportSizeScaledByDPI()
    {
        float Scaling = GetDPIScalingBasedOnSize();
        check(Scaling > 0.f);
        return GetEditorViewportSize() / Scaling;
    }

    UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static float GetDPIScalingBasedOnSize()
    {
        FVector2D ViewSize = GCurrentLevelEditingViewportClient->Viewport->GetSizeXY();
		return GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(FIntPoint(ViewSize.X, ViewSize.Y)) * (1.f / UExpressiveTextEditorFunctions::GetPlatformDPIScaling());
    }

	UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
	static FVector2D GetMousePosition( const FGeometry& ViewportGeometry )
	{
		FVector2D MousePosition = FSlateApplication::Get().GetCursorPos();
		return ViewportGeometry.AbsoluteToLocal(MousePosition);
	}

    UFUNCTION( BlueprintCallable, Category = ExpressiveTextEditor )
    static void StartWidget(UWidgetBlueprint* Blueprint)
    {
        if (Blueprint->GeneratedClass->IsChildOf(UEditorUtilityWidget::StaticClass()))
        {
            const UEditorUtilityWidget* CDO = Cast< UEditorUtilityWidget >( Blueprint->GeneratedClass->GetDefaultObject() );
            if (CDO->ShouldAutoRunDefaultAction())
            {
                // This is an instant-run blueprint, just execute it
                UEditorUtilityWidget* Instance = NewObject<UEditorUtilityWidget>(nullptr, Blueprint->GeneratedClass);
                Instance->ExecuteDefaultAction();
            }
            else
            {
                FName RegistrationName = FName(*(Blueprint->GetPathName() + TEXT("_ActiveTab")));
                FText DisplayName = FText::FromString(Blueprint->GetName());
                FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
                auto LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
                
#if UE_VERSION_OLDER_THAN( 4, 26, 0 )
                auto NewDockTab = LevelEditorTabManager->InvokeTab(RegistrationName);
#else
                auto NewDockTab = LevelEditorTabManager->TryInvokeTab(RegistrationName);
#endif
            }
        }
    }

    UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static UWorld* GetPIEWorld()
    {
        for ( const FWorldContext& Context : GEngine->GetWorldContexts() )
        {
            if( Context.WorldType == EWorldType::PIE )
            {
                return Context.World();
            }
        }

        return nullptr;
    }
    
    UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static UWorld* FindGameWorld()
    {
        for ( const FWorldContext& Context : GEngine->GetWorldContexts() )
        {
            if ( UWorld* World = Context.World())
            {
                if( World->IsGameWorld() )
                {
                    return Context.World();
                }
            }
        }

        return nullptr;
    }

    UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static void SetRealtimeOverride(bool Value)
    {
        if (GCurrentLevelEditingViewportClient)
        {
#if UE_VERSION_OLDER_THAN( 4, 25, 0 )
            GCurrentLevelEditingViewportClient->SetRealtime(Value, true);
#else
            GCurrentLevelEditingViewportClient->AddRealtimeOverride(Value, FText::FromString(TEXT("ExpressiveText")) );
#endif
        }
    }

    UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static void RestoreRealtimeValue()
    {
        if (GCurrentLevelEditingViewportClient)
        {
#if UE_VERSION_OLDER_THAN( 4, 25, 0 )
            GCurrentLevelEditingViewportClient->RestoreRealtime(true);
#else
            GCurrentLevelEditingViewportClient->RemoveRealtimeOverride(FText::FromString(TEXT("ExpressiveText")));
#endif
        }
    }
    
    UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    static void GetAssetsByClass( TSubclassOf<UObject> Class, bool bSearchSubclasses, TArray<FAssetData>& OutAssetData )
    {
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        auto& AssetReg = AssetRegistryModule.Get();
        Guganana::Engine::GetAssetsByClass( *Class, AssetReg, OutAssetData, bSearchSubclasses);
    }

    static void StartCustomEditor( UObject* Asset, const FSoftObjectPath& Path )
    {
        TSoftObjectPtr<UWidgetBlueprint> BlueprintSoft = TSoftObjectPtr<UWidgetBlueprint>( Path );
        UWidgetBlueprint* Blueprint = BlueprintSoft.LoadSynchronous();

        auto* EditorWidget = Blueprint;//Cast<UEditorUtilityWidgetBlueprint>(Blueprint);
        if (EditorWidget)
        {
            UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();

            // UEditorUtilityWidgetBlueprint is not being exported to the API in 4.23
            // so there's no way to do a unreal dynamic cast
            auto* Widget = EditorUtilitySubsystem->SpawnAndRegisterTab((UEditorUtilityWidgetBlueprint*)EditorWidget);
            if( Widget->GetClass()->ImplementsInterface(UExpressiveTextEditorWidgetInterface::StaticClass() ) )
            {
                IExpressiveTextEditorWidgetInterface::Execute_SetAsset(Widget, Asset);
            }
        }
    }

    static void StartCustomEditor( UObject& Asset, const FSoftObjectPath& Path )
    {
        StartCustomEditor(&Asset, Path);
    }

	static void StartCustomEditor(const FSoftObjectPath& Path)
	{
		StartCustomEditor(nullptr, Path);
	}

    static void GetViewportCameraInfo(FVector& CameraLocation, FRotator& CameraRotation)
    {
        CameraLocation = FVector::ZeroVector;
        CameraRotation = FRotator::ZeroRotator;

        for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients())
        {
            if (LevelVC && LevelVC->IsPerspective())
            {
                CameraLocation = LevelVC->GetViewLocation();
                CameraRotation = LevelVC->GetViewRotation();
                return;
            }
        }
    }
};
