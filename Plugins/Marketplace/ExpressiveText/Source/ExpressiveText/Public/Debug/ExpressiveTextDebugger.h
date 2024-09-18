// Copyright 2022 Guganana. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include <Engine/Engine.h>
#include <HAL/IConsoleManager.h>
#include <EngineUtils.h>

#include "ExpressiveTextDebugger.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogExpressiveTextDebugger, Log, All );

USTRUCT( BlueprintType )
struct FExText_ColoredMessage
{
    GENERATED_BODY()

    FExText_ColoredMessage()
        : Message()
        , Color( FColor::Cyan )
    {}

    FExText_ColoredMessage( const FString& InMessage, const FColor& InColor )
        : Message(InMessage)
        , Color(InColor)
    {}

    UPROPERTY( BlueprintReadOnly, Category = ExpressiveText )
    FString Message;

    UPROPERTY( BlueprintReadOnly, Category = ExpressiveText )
    FColor Color;
};

UCLASS(Abstract)
class AExpressiveTextDebugger : public AActor
{
    GENERATED_BODY()

public:
    AExpressiveTextDebugger() 
        : Super()
    {
        PrimaryActorTick.bCanEverTick = true;
        PrimaryActorTick.bStartWithTickEnabled = true;
    }

    static AExpressiveTextDebugger* Get()
    {
        return Instance.Get();
    }

    void Log(ELogVerbosity::Type Verbosity, const FString& Message)
    {
        FString FinalMessage = FString::Printf(TEXT("%s: %s"), *VerbosityToString(Verbosity), *Message);
        if (GEngine)
        {
            Logs.Emplace(FinalMessage, VerbosityToColor( Verbosity ) );
        }
    }

    FColor VerbosityToColor( ELogVerbosity::Type Verbosity )
    {
        switch( Verbosity )
        {
        case ELogVerbosity::Type::Error:
            return FColor::Red;
        case ELogVerbosity::Type::Warning:
            return FColor::Yellow;
        case ELogVerbosity::Type::Log:
            return FColor::White;
        }
        
        return FColor::White;
    }

    FString VerbosityToString( ELogVerbosity::Type Verbosity )
    {
        switch( Verbosity )
        {
        case ELogVerbosity::Type::Error:
            return TEXT("Error");
        case ELogVerbosity::Type::Warning:
            return TEXT("Warning");
        case ELogVerbosity::Type::Log:
            return TEXT("Info");
        }
        return TEXT("");
    }

    UPROPERTY( BlueprintReadOnly, Category = ExpressiveText )
    TArray<FExText_ColoredMessage> AllStylesStatus;

    UPROPERTY( BlueprintReadOnly, Category = ExpressiveText )
    TArray<FExText_ColoredMessage> AllFontsStatus;

    UPROPERTY( BlueprintReadOnly, Category = ExpressiveText )
    TArray<FExText_ColoredMessage> Logs;

protected:

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
    }

    virtual void Tick(float DeltaTime) override;
private:

    static FAutoConsoleCommandWithWorldAndArgs CmdToggleDebugger;
    static TSoftObjectPtr<AExpressiveTextDebugger> Instance;
};

#ifndef EXTEXT_LOG
#define EXTEXT_LOG(Verbosity, Format, ...) \
	UE_LOG(LogExpressiveTextDebugger, Verbosity, Format, ##__VA_ARGS__); \
    //ExTextLogger::Log( ELogVerbosity::Verbosity, FString::Printf(Format, ##__VA_ARGS__) );
#endif
