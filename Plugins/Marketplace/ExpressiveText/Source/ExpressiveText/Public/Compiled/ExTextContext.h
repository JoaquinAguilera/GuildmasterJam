// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Actions/ExpressiveTextActionInterface.h"

#include "ExTextContext.generated.h"

USTRUCT(BlueprintType)
struct FExpressiveTextContext
{
    GENERATED_BODY()

    FExpressiveTextContext()
        : ActionMap()
        , SearchOuter()
    {}

    IExText_ActionInterface* SearchAction( const FName& ActionName ) const
    {
        if( auto* Result = ActionMap.Find( ActionName ) )
        {
            return static_cast<IExText_ActionInterface*>( Result->GetInterface() );
        }

        if( SearchOuter )
        {
            return SearchOuter( ActionName );
        }

        return nullptr;
    }

    void SetSearchOuter( TFunction< IExText_ActionInterface*( const FName& ) > InFunction )
    {
        SearchOuter = InFunction;
    }

    void AddAction( const FName& Name, TScriptInterface<IExText_ActionInterface> Action )
    {
        ActionMap.Add( Name, Action );
    }

private:
    TMap< FName, TScriptInterface<IExText_ActionInterface>> ActionMap;
    TFunction< IExText_ActionInterface*( const FName& ) > SearchOuter; 
};