// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Misc/FileHelper.h>
#include <Misc/Paths.h>
#include "Plugin.h"

namespace Guganana
{
    class Id
    {
    public:
        static FGuid Get()
        {
            static FGuid Id = FGuid();

            if( Id == FGuid() )
            {
                if( !TryReadFromFile(Id) )
                {
                    Id = FGuid::NewGuid();
                    SaveGuid(Id);
                }                
            }

            return Id;
        }

    private:

        static FString GetIdFilePath()
        {
            return FPaths::Combine( FPaths::EngineVersionAgnosticUserDir(), Guganana::PluginInformation::PluginName(), TEXT("Id") );
        }

        static bool TryReadFromFile( FGuid& OutGuid )
        {
            FString FileData;
            if( FFileHelper::LoadFileToString(FileData, *GetIdFilePath() ) )
            {
                FGuid Result;
                if( FGuid::Parse(FileData, Result) )
                {
                    OutGuid = Result;
                    return true;
                }
            }

            return false;
        }
        
        static void SaveGuid( const FGuid& Guid )
        {
            FFileHelper::SaveStringToFile(Guid.ToString(), *GetIdFilePath() );
        }
    };
}