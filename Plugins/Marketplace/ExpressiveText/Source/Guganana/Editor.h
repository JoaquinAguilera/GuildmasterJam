// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Misc/EngineVersionComparison.h>

#if UE_VERSION_OLDER_THAN(5,1,0)
#include <EditorStyleSet.h>
#else
#include <Styling/AppStyle.h>
#endif

namespace Guganana
{
    class Editor
    {
public:
        static const FSlateBrush* GetBrush(FName BrushName)
        {
#if UE_VERSION_OLDER_THAN(5,1,0)
            return FEditorStyle::GetBrush(BrushName);
#else  
            return FAppStyle::GetBrush(BrushName);
#endif
        }

        static const ISlateStyle& GetStyle()
        {
#if UE_VERSION_OLDER_THAN(5,1,0)
            return FEditorStyle::Get();
#else  
            return FAppStyle::Get();
#endif
        }
    };
}