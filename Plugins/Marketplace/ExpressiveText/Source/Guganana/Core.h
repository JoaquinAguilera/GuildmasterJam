// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Misc/EngineVersionComparison.h>

namespace Guganana
{
    class Core
    {
    public:
        static UPackage* GetPackage( UObject* Object )
        {            
            if( Object == nullptr )
            {
                return nullptr;
            }

            return GetPackage( *Object );     
        }

        static UPackage* GetPackage( UObject& Object )
        {            
        #if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 24
            return Object.GetOutermost();
        #else
            return Object.GetPackage();
        #endif            
        }

        static UPackage* MakePackage( const FString& PackageName )
        {
        #if UE_VERSION_OLDER_THAN( 4, 26, 0 )
		    return CreatePackage(nullptr, *PackageName);
        #else
		    return CreatePackage(*PackageName);
        #endif
        }
        
    };
}
