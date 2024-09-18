// Copyright 2022 Guganana. All Rights Reserved.
#include "Subsystems/ExpressiveTextGameInstanceSubsystem.h"

#include "ExpressiveText/Public/Debug/ExpressiveTextDebugger.h"
#include "ExpressiveText/Public/ExpressiveTextSettings.h"

void UExpressiveTextGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    //if( auto* Settings = GetDefault<UExpressiveTextSettings>() )
    //{
    //    if( UWorld* World = GetWorld() )
    //    {
    //        if( Settings->ActivateExpressiveTextDebugger )
    //        {
    //            Debugger = World->SpawnActor<AExpressiveTextDebugger>(AExpressiveTextDebugger::StaticClass());
    //        }
    //    }
    //}
}

void UExpressiveTextGameInstanceSubsystem::Deinitialize()
{
    //if( Debugger )
    //{
    //    Debugger->Destroy();
    //    Debugger = nullptr;
    //}

    //Super::Deinitialize();
}