// Copyright 2022 Guganana. All Rights Reserved.
#include "ExpressiveText/Public/Debug/ExpressiveTextDebugger.h"

#include "ExpressiveTextSettings.h"
#include "Resources/ExpressiveTextResources.h"
#include "Subsystems/ExpressiveTextSubsystem.h"

#include <HAL/IConsoleManager.h>

DEFINE_LOG_CATEGORY(LogExpressiveTextDebugger);

FAutoConsoleCommandWithWorldAndArgs AExpressiveTextDebugger::CmdToggleDebugger(
        TEXT("ExpressiveText.ToggleDebugger"),
        TEXT("Spawns/despawns the expressive text debugger"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
                if (World)
                {
                    if (Instance)
                    {
                        Instance->Destroy();
                    }
                    else
                    {
                        Instance = World->SpawnActor<AExpressiveTextDebugger>(GetDefault<UExpressiveTextSettings>()->DebuggerClass.LoadSynchronous());
                    }
                }
            })
       );

TSoftObjectPtr<AExpressiveTextDebugger> AExpressiveTextDebugger::Instance = TSoftObjectPtr<AExpressiveTextDebugger>();

void AExpressiveTextDebugger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AllStylesStatus.Empty();
    AllFontsStatus.Empty();

    if (GEngine)
    {
        if( auto* Subsystem = GEngine->GetEngineSubsystem<UExpressiveTextSubsystem>() )
        {
            const auto CreateMessageForSoftPtr = []( auto& SoftPtr )
            {
                const bool IsLoaded = SoftPtr.ResolveObject() != nullptr;
                FString AssetName = SoftPtr.GetAssetName(); 
                FString LoadedStr = IsLoaded ? TEXT(" : loaded") : TEXT(" : unloaded");

                FExText_ColoredMessage Message;       
                Message.Message = AssetName + LoadedStr;
                Message.Color = IsLoaded ? FColor::Green : FColor::Cyan;
                return Message;
            };
            
            for( const auto& Style: Subsystem->GetAllResourcesByType<ExpressiveTextResource::Style>() )
            {
                AllStylesStatus.Emplace( CreateMessageForSoftPtr( Style ) );
            }         

            for (const auto& Font : Subsystem->GetAllResourcesByType<ExpressiveTextResource::Font>())
            {
                AllFontsStatus.Emplace( CreateMessageForSoftPtr( Font ) );
            }
        }
    }
}