 // Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#define DECLARE_STATIC(Type, Name, Value) \
    static Type& Name()\
    {\
        static Type Name_Val = Value;\
        return Name_Val;\
    }

struct ExpressiveTextPlugin
{
    DECLARE_STATIC(FString, PluginName, TEXT("ExpressiveText"))

    struct Version
    {
        DECLARE_STATIC(FString, Friendly, TEXT("0.5"))
        DECLARE_STATIC(FString, SHA, TEXT("37bceb8"))
        DECLARE_STATIC(FDateTime, Date, FDateTime(2024, 6, 13))
    };

    struct Endpoints
    {
        DECLARE_STATIC(FString, ConfirmUsage, TEXT("https://api.expressivetext.guganana.com/api/usage"))
        DECLARE_STATIC(FString, JoinDiscord, TEXT("https://api.expressivetext.guganana.com/api/joinDiscord?id="))
        DECLARE_STATIC(FString, GithubOFLFonts, TEXT("https://api.github.com/repos/google/fonts/git/trees/beddc937d245e8c0a6011e8e66d0b294ca35d68a"))
    };

    struct DocsPages
    {
        DECLARE_STATIC(FString, ReportABug, TEXT("https://expressivetext.com/docs/30440dfb-5d4d-4fa4-a033-ae9be80fe2db"))
    };
};  

using ExportedPluginInformation = ExpressiveTextPlugin;

#undef DECLARE_STATIC 
