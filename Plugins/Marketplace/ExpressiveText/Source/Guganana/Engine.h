// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Misc/EngineVersionComparison.h>

#if UE_VERSION_OLDER_THAN( 4, 27, 0 )
#include <AssetData.h>
#else
#include <AssetRegistry/AssetData.h>
#endif

#if UE_VERSION_OLDER_THAN( 5, 1, 0 )
#include <IAssetRegistry.h>
#else
#include <AssetRegistry/IAssetRegistry.h>
#endif

namespace Guganana
{
    class Engine
	{
	public:
		static bool IsExitRequested()
		{
		#if UE_VERSION_OLDER_THAN( 4, 24, 0 ) 
			return GIsRequestingExit;
		#else
			return IsEngineExitRequested();
		#endif
		}

		template< typename T >
		static bool GetAssetsByClass(IAssetRegistry& AssetReg, TArray<FAssetData>& OutAssetData, bool bSearchSubClasses = false)
		{
			return GetAssetsByClass( T::StaticClass(), AssetReg, OutAssetData, bSearchSubClasses );
		}

		static bool GetAssetsByClass(UClass* Class, IAssetRegistry& AssetReg, TArray<FAssetData>& OutAssetData, bool bSearchSubClasses = false)
		{
		#if UE_VERSION_OLDER_THAN(5,1,0)
			return AssetReg.GetAssetsByClass(Class->GetFName(), OutAssetData, bSearchSubClasses);
		#else  
			return AssetReg.GetAssetsByClass(FTopLevelAssetPath(Class), OutAssetData, bSearchSubClasses);
		#endif
		}

		template< typename T >
		static bool IsAssetOfClass(const FAssetData& AssetData )
		{
			return AssetData.GetClass() == T::StaticClass();
		}

		template< typename A >
		static auto& GetRuntime(A& In)
		{
#if UE_VERSION_OLDER_THAN( 5, 1, 0 )
			return In;
#else
			return In.GetRuntime();
#endif
		}
				
	#if WITH_EDITOR
		template< typename A >
		static auto& GetEditorOnlyData(A& In)
		{
		#if UE_VERSION_OLDER_THAN( 5, 1, 0 )
			return In;
		#else
			return In.EditorOnly;
		#endif
		}
	#endif

#if WITH_EDITOR
		template< typename A >
		static auto& GetStaticSwitchParameters(A& In)
		{
#if UE_VERSION_OLDER_THAN( 5, 1, 0 )
			return In.StaticSwitchParameters;
#elif UE_VERSION_OLDER_THAN( 5, 2, 0 )
			// StaticSwitchParameters only lived in Editor Info during 5.1
			return GetEditorOnlyData(In).StaticSwitchParameters;
#else
			return GetRuntime(In).StaticSwitchParameters;
#endif

		}
#endif

    };
}
