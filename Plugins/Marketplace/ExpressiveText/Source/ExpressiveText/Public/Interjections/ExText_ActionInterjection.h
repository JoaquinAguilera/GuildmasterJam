// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Engine/Engine.h>

#include "ExText_Interjection.h"

class FExText_ActionInterjection : public FExText_Interjection
{
public:
    FExText_ActionInterjection( const FString& Parameter, const FExpressiveTextContext& Context )
        : Action( )
    {
        if( auto* SearchResult = Context.SearchAction( FName( *Parameter ) ) )
        {
#if UE_VERSION_OLDER_THAN( 4, 27, 0 )
            Action = TWeakInterfacePtr< IExText_ActionInterface >(*SearchResult);
#else
            Action = TWeakInterfacePtr< IExText_ActionInterface >(SearchResult);
#endif
            IsCorrectlyParsed = true;
        }
    }
protected:
    virtual void OnExecute( UWorld& World ) const override
    {
        if( Action.IsValid() )
        {
            Action->SetWorld( &World );
            IExText_ActionInterface::Execute_Run( Action.GetObject() );
        }
    }
private:

    static UWorld* GetWorld()
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

    TWeakInterfacePtr< IExText_ActionInterface > Action;
};