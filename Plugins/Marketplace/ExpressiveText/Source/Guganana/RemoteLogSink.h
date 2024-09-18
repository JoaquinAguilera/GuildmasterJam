// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <HttpModule.h>
#include <Interfaces/IHttpRequest.h>
#include <Interfaces/IHttpResponse.h>
#include <JsonObjectConverter.h>
#include <Tickable.h>

#include "Plugin.h"
#include "Id.h"
#include <Unlog/UnlogImplementation.h>

namespace Guganana 
{
    class RemoteLog : public FTickableGameObject
    {
    public:

        RemoteLog()
            : PendingLogEvents()
            , TimeBetweenSendsInSeconds( 2.f )
            , AccumulatedTime( 0.f )
        {}

        void SetTimeBetweenSends(float InTimeBetweenSendsInSeconds)
        {
            TimeBetweenSendsInSeconds = InTimeBetweenSendsInSeconds;
        }
        void Log(ELogVerbosity::Type Verbosity, const FString& Message)
        {
            PendingLogEvents.Emplace(Verbosity, Message);
        }
        virtual bool IsTickable() const 
        { 
            return PendingLogEvents.Num() > 0;
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
            RETURN_QUICK_DECLARE_CYCLE_STAT(RemoteLog, STATGROUP_Tickables);
        }

        virtual void Tick(float DeltaTime) override
        {
            AccumulatedTime += DeltaTime;

            if( AccumulatedTime > TimeBetweenSendsInSeconds )
            {
                SendLogEvents();
                PendingLogEvents.Empty();
                AccumulatedTime = 0.f;
            }
        }
    private:
        struct LogEvent
        {
            LogEvent( ELogVerbosity::Type InVerbosity, const FString& InMessage )
                : Time( FDateTime::UtcNow().ToUnixTimestamp() )
                , Message( InMessage )
                , Verbosity( VerbosityToString(InVerbosity) )
                , Category( TEXT("All"))
            {}
            
            int64 Time; 
            FString Message;
            FString Verbosity;
            FString Category;
        };

        void SendLogEvents()
        {
            TSharedRef<FJsonObject> ResultJsonObject = MakeShareable(new FJsonObject);

            ResultJsonObject->SetStringField("appId", Id::Get().ToString() );
            ResultJsonObject->SetStringField("product", PluginInformation::PluginName());
            ResultJsonObject->SetStringField("versionFriendly", PluginInformation::Version::Friendly());
            ResultJsonObject->SetStringField("versionSHA", PluginInformation::Version::SHA());
            ResultJsonObject->SetStringField("unrealVersion", FEngineVersion::Current().ToString());

            TArray< TSharedPtr<FJsonValue> > EventsJson;
            for (const auto& Event : PendingLogEvents)
            {
                TSharedPtr<FJsonObject> Data = MakeShareable(new FJsonObject);
                Data->SetNumberField(TEXT("timestamp"), (double)Event.Time);
                Data->SetStringField(TEXT("category"), Event.Category);
                Data->SetStringField(TEXT("verbosity"), Event.Verbosity);
                Data->SetStringField(TEXT("message"), Event.Message);
                EventsJson.Add(MakeShareable(new FJsonValueObject(Data)));
            }

            ResultJsonObject->SetArrayField("events", EventsJson);

            FString ResultJsonObjString;
            TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&ResultJsonObjString);
            FJsonSerializer::Serialize(ResultJsonObject, Writer);

            auto Request = FHttpModule::Get().CreateRequest();
            Request->SetURL(TEXT("https://logs.guganana.com"));
            Request->SetVerb("POST");
            Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
            Request->SetContentAsString(ResultJsonObjString);
            Request->ProcessRequest();            
        }

        static FString VerbosityToString(ELogVerbosity::Type Verbosity)
        {
            switch (Verbosity)
            {
            case ELogVerbosity::Type::Error:
                return TEXT("Error");
            case ELogVerbosity::Type::Warning:
                return TEXT("Warning");
            case ELogVerbosity::Type::Log:
                return TEXT("Log");
            }
            return TEXT("None");
        }

        TArray<LogEvent> PendingLogEvents;
        float TimeBetweenSendsInSeconds;
        float AccumulatedTime;         
    };

}

template< typename TConcrete >
struct TUnlogRemoteSink
{
    static void Call(const UnlogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FString& Message)
    {
        if (TConcrete::ShouldLog())
        {
            static Guganana::RemoteLog RemoteLogger;
            RemoteLogger.Log(Verbosity, Message);
        }
    }


};


