// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Debug/ExpressiveTextDebugger.h"

struct FInterjectionVisitor
{
    FInterjectionVisitor( const TWeakPtr<FExpressiveTextExtraction>& InExtraction )
        : OwnerExtraction( InExtraction )
        , CurrentInterjectionIndex( 0 )
    {}

    void VisitInterjectionsAt( int32 Index, TFunction<void( const TSharedRef<FExText_Interjection> )> Visitor )
    {
        if( auto* RawExtraction = OwnerExtraction.Pin().Get() )
        {
            if( RawExtraction->Interjections.IsValidIndex( CurrentInterjectionIndex )  )
            {
                const auto& ParsedInterjection = RawExtraction->Interjections[CurrentInterjectionIndex];
                
                if( Index >= ParsedInterjection.Index  || IsIndexAtEndOfStringInterjection(Index,ParsedInterjection) )
                {
                    Visitor( ParsedInterjection.Interjection.ToSharedRef() );
                    CurrentInterjectionIndex++;

                    // Recurse in case there's more interjections at this index
                    VisitInterjectionsAt( Index, Visitor );
                }
            }
        }
    } 


    //Info: Interjections at the end of sentences need to be accounted and are triggered by the last glyph reveal
    bool IsIndexAtEndOfStringInterjection(int32 Index, const FExText_ParsedInterjection& Interjection)
    {
        if (auto* RawExtraction = OwnerExtraction.Pin().Get())
        {
            if (RawExtraction->ExtractionTree->Range.EndIndex == Interjection.Index && Interjection.Index == Index + 1)
            {
                return true;
            }
        }

        return false;
    }


private:
    TWeakPtr<FExpressiveTextExtraction> OwnerExtraction;
    int32 CurrentInterjectionIndex;
};

struct FInterjectionOutput
{
    FInterjectionOutput()
        : PauseDuration( 0.f )
    {}

    float PauseDuration;
};

class FExText_Interjection
{

public:

    virtual ~FExText_Interjection()
    { 
    }

    void ProcessModifiers( FInterjectionOutput& Out ) const
    {
        if (IsCorrectlyParsed)
        {
            OnProcessModifiers( Out );
        }
    }

    void DoExecute(UWorld& World) const
    {
        if (IsCorrectlyParsed)
        {
            OnExecute(World);
        }
        else
        {
            EXTEXT_LOG( Error, TEXT("Interjection is not correctly parsed") );
        }
    }

protected:
    virtual void OnProcessModifiers(FInterjectionOutput& Out) const {};
    virtual void OnExecute(UWorld& World) const {};
    bool IsCorrectlyParsed;
};