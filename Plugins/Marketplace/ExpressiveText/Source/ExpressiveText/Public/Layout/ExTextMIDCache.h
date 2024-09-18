// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Materials/MaterialInstanceDynamic.h>
#include <Tickable.h>

#include "Asset/ExpressiveTextMaterial.h"

struct FExTextMIDRequest
{
    FExTextMIDRequest( UMaterialInterface& InMat )
        : BaseMaterial( InMat )
        , ScalarParams()
        , VectorParams()
    {}

    void AddScalar( const FMaterialParameterInfo& Param, float Value )
    {
        ScalarParams.Add( Param, Value );
    }

    void AddVector( const FMaterialParameterInfo& Param, const FLinearColor& Value )
    {
        VectorParams.Add( Param, Value );
    }

    UMaterialInterface& BaseMaterial;
    TMap<FMaterialParameterInfo, float> ScalarParams;
    TMap<FMaterialParameterInfo, FLinearColor> VectorParams;
};


struct FExTextMIDChecksum
{
    FExTextMIDChecksum( const FExTextMIDRequest& Request )
      : Checksum( 0 )
    {
        Checksum = GetTypeHash( &Request.BaseMaterial );

        for ( const auto& ScalarParam : Request.ScalarParams )
        {
            Checksum = HashCombine( Checksum, GetTypeHash( ScalarParam.Key ) );
            Checksum = HashCombine( Checksum, GetTypeHash( ScalarParam.Value ) );
        }

        for ( const auto& VectorParam : Request.VectorParams )
        {
            Checksum =  HashCombine( Checksum, GetTypeHash( VectorParam.Key ) );
            Checksum =  HashCombine( Checksum, GetTypeHash( VectorParam.Value ) );
        }
    }

    friend uint32 GetTypeHash( const FExTextMIDChecksum& MID )
    {
        return MID.Checksum;
    }

    friend bool operator==(const FExTextMIDChecksum& Lhs, const FExTextMIDChecksum& Rhs)
    {
        return Lhs.Checksum == Rhs.Checksum;
    }
    
private:
    uint32 Checksum;
};


struct FExTextMID
{    
    UMaterialInstanceDynamic* GetMIDAndConfirmUsage() const
    {
        LastConfirmedUsageTimestamp = FPlatformTime::Seconds();
        return MID.Get();
    }

    void ResetMID()
    {
        MID = nullptr;
    }
    
    mutable float LastConfirmedUsageTimestamp;
    TStrongObjectPtr<UMaterialInstanceDynamic> MID;
};

class FExTextMIDCache : public FTickableGameObject
{
public:

    FExTextMIDCache()
        : MIDCache()
        , LastPurgeTimestamp( 0.0f )
    {}

    const TSharedPtr<FExTextMID> RequestMID( const FExTextMIDRequest& Request )
    {
        const FExTextMIDChecksum Checksum( Request );

        if(TSharedPtr<FExTextMID>* Result = MIDCache.Find( Checksum ) )
        {
            auto ResultSharedPtr = *Result;
            if (ResultSharedPtr)
            {
                ResultSharedPtr->LastConfirmedUsageTimestamp = FPlatformTime::Seconds();
                return ResultSharedPtr;
            }
        }
            
        TSharedPtr<FExTextMID> NewMID = MakeShareable(new FExTextMID);
        NewMID->MID = TStrongObjectPtr<UMaterialInstanceDynamic>( UMaterialInstanceDynamic::Create( &Request.BaseMaterial, nullptr ) );
        NewMID->LastConfirmedUsageTimestamp = FPlatformTime::Seconds();


        NewMID->MID->SetScalarParameterValue("InstanceRandom", FMath::Rand() );

        for (const auto& ScalarParam : Request.ScalarParams)
        {
            NewMID->MID->SetScalarParameterValueByInfo(ScalarParam.Key, ScalarParam.Value);
        }

        for (const auto& VectorParam : Request.VectorParams)
        {
            NewMID->MID->SetVectorParameterValueByInfo(VectorParam.Key, VectorParam.Value);
        }

        return MIDCache.Add( Checksum, NewMID );
    }

    void TryPurgeCache()
    {
        static constexpr float MIDCachePurgeInterval = 5.0f;
        static constexpr float MIDTimeToLive = 6.0f;
        
        const float CurrentTime = FPlatformTime::Seconds();
        if( CurrentTime - LastPurgeTimestamp < MIDCachePurgeInterval )
        {
            return;
        }

        for( auto It = MIDCache.CreateIterator(); It; ++It )
        {
            if (auto* ExTextMID = It.Value().Get())
                {
                if (CurrentTime - ExTextMID->LastConfirmedUsageTimestamp > MIDTimeToLive)
                {
#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
                    ExTextMID->MID->MarkPendingKill();
#else
                    ExTextMID->MID->MarkAsGarbage();
#endif
                    ExTextMID->ResetMID();
                    It.RemoveCurrent();
            }
            }
        }
    }

    virtual bool IsTickableWhenPaused() const override
	{
		return true;
	}

	virtual bool IsTickableInEditor() const override
	{
		return true;
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FExTextMIDCache, STATGROUP_Tickables);
	}

	virtual void Tick(float DeltaTime) override
	{
		TryPurgeCache();
	}

private:
    TMap<FExTextMIDChecksum, TSharedPtr<FExTextMID> > MIDCache;
    double LastPurgeTimestamp;
};

