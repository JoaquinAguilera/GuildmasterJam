// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include "Parameters/ExpressiveTextParameterLookup.h"
#include "Compiled/ExTextContext.h"

#include "TreeExtraction.generated.h"

UENUM(BlueprintType)
enum class EExtractionSectionType : uint8
{
    TagsDelimiterStart,
    Tags,
    TagsDelimiterEnd,
    ContentDelimiterStart,
    Content,
    ContentDelimiterEnd,
};

USTRUCT( BlueprintType )
struct FExpressiveTextExtractionSection
{
    GENERATED_BODY()

    FExpressiveTextExtractionSection( int32 InSectionIndex, int32 Start, int32 End, EExtractionSectionType InType )
        : SectionIndex( InSectionIndex )
        , Type( InType )
        , BeginIndex( Start )
        , EndIndex( End )
    {}

    FExpressiveTextExtractionSection()
        : SectionIndex(-1)
        , Type(EExtractionSectionType::Content)
        , BeginIndex(-1)
        , EndIndex(-1)
    {}

    UPROPERTY( BlueprintReadWrite, Category = ExpressiveText )
    int32 SectionIndex;

    UPROPERTY( BlueprintReadWrite, Category = ExpressiveText )
    EExtractionSectionType Type;

    UPROPERTY( BlueprintReadWrite, Category = ExpressiveText )
    int32 BeginIndex;
    
    UPROPERTY( BlueprintReadWrite, Category = ExpressiveText )
    int32 EndIndex;
};

USTRUCT(BlueprintType)
struct FExpressiveLineExtractionsInfo
{
    GENERATED_BODY()

    TSharedPtr< struct FExpressiveTextExtraction > Extraction;

    UPROPERTY( BlueprintReadOnly, Category = ExpressiveText )
    FString UnprocessedLine;
};

struct FTreeExtraction 
{

    void AddChild( const TSharedRef<FTreeExtraction>& Child )
    {
        check( Child->Range.BeginIndex >= Range.BeginIndex );

        Children.Emplace( Child );
    }

    void TrawlSections( const TFunction< void ( const FTreeExtraction&, const FTextRange& ) >& Visitor ) const
    {
        TrawlSectionsImpl( 0, Visitor );
    }
    void TrawlOriginalTextSections( const TFunction< void(const FExpressiveTextExtractionSection&) >& Visitor) const
    {
        int32 ChildCounter = 0;
        TrawlOriginalTextSectionsImpl( 0, ChildCounter, Visitor );
    }

    FTextRange Range;
    FTextRange OriginalRange;
    FTextRange TagsRange;
    FTextRange ContentRange;
    TSharedPtr<FExpressiveTextParameterLookup> ParameterLookup;

    TArray<FString> TagsList;
    FString Content;

    TSharedPtr<FTreeExtraction> Parent;
    TArray< TSharedRef< FTreeExtraction> > Children;

private:
    void TrawlSectionsImpl( int32 CurrentIndex, const TFunction< void ( const FTreeExtraction&, const FTextRange& ) >& Visitor ) const
    {
        int32 CurrentChild = 0;        
        while( CurrentIndex <= Range.EndIndex )
        {
            if( CurrentChild < Children.Num() )
            {
                const FTreeExtraction& Child = Children[ CurrentChild ].Get();

                // Catch-up to next child
                if( CurrentIndex < Child.Range.BeginIndex )
                {
                    Visitor( *this, FTextRange( CurrentIndex, Child.Range.BeginIndex ) );
                    CurrentIndex = Child.Range.BeginIndex;
                }

                Child.TrawlSectionsImpl( CurrentIndex, Visitor );
                CurrentIndex = Child.Range.EndIndex;
                CurrentChild++;
            }
            else
            {
                int32 EndIndex = Range.EndIndex;
                Visitor(*this, FTextRange(CurrentIndex, EndIndex));
                CurrentIndex = EndIndex + 1;
            }
        }
    }

    void TrawlOriginalTextSectionsImpl(int32 CurrentIndex, int32& ChildCounter, const TFunction< void(const FExpressiveTextExtractionSection&) >& Visitor) const
    {
        int32 ThisIndex = ChildCounter++;
        int32 CurrentChild = 0;

        while (CurrentIndex <= OriginalRange.EndIndex)
        {
            if (TagsRange.BeginIndex >= 0 && CurrentIndex < ContentRange.BeginIndex )
            {
                Visitor(FExpressiveTextExtractionSection( ThisIndex, TagsRange.BeginIndex, TagsRange.BeginIndex, EExtractionSectionType::TagsDelimiterStart ));
                Visitor(FExpressiveTextExtractionSection( ThisIndex, TagsRange.BeginIndex + 1, TagsRange.EndIndex - 1, EExtractionSectionType::Tags ));
                Visitor(FExpressiveTextExtractionSection( ThisIndex, TagsRange.EndIndex, TagsRange.EndIndex, EExtractionSectionType::TagsDelimiterEnd ));
                Visitor(FExpressiveTextExtractionSection( ThisIndex, ContentRange.BeginIndex, ContentRange.BeginIndex, EExtractionSectionType::ContentDelimiterStart ));
                CurrentIndex = ContentRange.BeginIndex + 1;
            }

            if (CurrentChild < Children.Num())
            {
                const FTreeExtraction& Child = Children[CurrentChild].Get();

                // Catch-up to next child tags
                if (CurrentIndex < Child.OriginalRange.BeginIndex)
                {
                    Visitor(FExpressiveTextExtractionSection( ThisIndex, CurrentIndex, Child.OriginalRange.BeginIndex, EExtractionSectionType::Content ));
                    CurrentIndex = Child.OriginalRange.BeginIndex;
                }

                Child.TrawlOriginalTextSectionsImpl(CurrentIndex, ChildCounter, Visitor);
                CurrentIndex = Child.OriginalRange.EndIndex + 1;
                CurrentChild++;
            }
            else
            {
                if (CurrentIndex < OriginalRange.EndIndex)
                {
                    Visitor(FExpressiveTextExtractionSection( ThisIndex, CurrentIndex, OriginalRange.EndIndex, EExtractionSectionType::Content ));
                }
                if (ContentRange.EndIndex >= 0)
                {
                    Visitor(FExpressiveTextExtractionSection(ThisIndex, ContentRange.EndIndex, ContentRange.EndIndex, EExtractionSectionType::ContentDelimiterEnd));
                }
                CurrentIndex = OriginalRange.EndIndex + 1;
            }
        }
    }

};