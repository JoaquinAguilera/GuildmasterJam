// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include "Plugin.h"
#include "Id.h"

#include <FileHelpers.h>
#include <Framework/Notifications/NotificationManager.h>
#include <HttpModule.h>
#include <Interfaces/IHttpRequest.h>
#include <Interfaces/IHttpResponse.h>
#include <JsonObjectConverter.h>
#include <Misc/EngineVersion.h>
#include <Widgets/Notifications/SNotificationList.h>

namespace Guganana
{
    class HTTP
    {
    public:
        template< typename Plugin = PluginInformation >
        static void ConfirmAppUsage(TFunction<void(TSharedPtr<FJsonObject>)> Callback = nullptr)
        {
            if (IsRunningCommandlet())
            {
                return;
            }

            // Analytics for confirming app is used to estimate the number of users.
            // Sends a unique generated key for this plugin instance so we can avoid double counting
            // This key will never be sent again with other analytics events, so there's no way to associate data with this key
            // The key will be regenerated when the ExpressiveText.ini is deleted/recreated and therefore can't identificate the user.

            TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

            JsonObject->SetStringField("appId", Id::Get().ToString() );
            JsonObject->SetStringField("pluginName", Plugin::PluginName());
            JsonObject->SetStringField("versionFriendly", Plugin::Version::Friendly());
            JsonObject->SetStringField("versionSHA", Plugin::Version::SHA());
            JsonObject->SetStringField("versionDate", Plugin::Version::Date().ToIso8601());
            JsonObject->SetStringField("unrealVersion", FEngineVersion::Current().ToString());

            FString JsonObjString;
            TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&JsonObjString);
            FJsonSerializer::Serialize(JsonObject, Writer);

            auto Request = FHttpModule::Get().CreateRequest();
            Request->SetURL(Plugin::Endpoints::ConfirmUsage());
            Request->SetVerb("POST");
            Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
            Request->SetContentAsString(JsonObjString);
            Request->OnProcessRequestComplete().BindStatic(&Guganana::HTTP::ConfirmAppUsageResponse, Callback);

            Request->ProcessRequest();
        }

        template< typename Plugin = PluginInformation >
        static void ConfirmAppUsageResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, TFunction<void(TSharedPtr<FJsonObject>)> Callback)
        {
            if (bSucceeded && HttpResponse.IsValid())
            {
                if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
                {
                    auto ResponseString = HttpResponse->GetContentAsString();

                    TSharedPtr<FJsonObject> JsonObject;
                    TSharedRef<TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseString);
                    bool DeserialiseResult = FJsonSerializer::Deserialize(JsonReader, JsonObject);
                    if (DeserialiseResult && JsonObject.IsValid())
                    {
                        FString ResponseMessage;
                        JsonObject->TryGetStringField(TEXT("responseMessage"), ResponseMessage);

                        if (!ResponseMessage.IsEmpty())
                        {
                            FNotificationInfo Info(FText::FromString(ResponseMessage));
                            Info.ExpireDuration = 30.0f;
                            Info.bFireAndForget = true;
                            Info.bUseLargeFont = true;
                            Info.bUseThrobber = false;
                            Info.bUseSuccessFailIcons = false;
                            FSlateNotificationManager::Get().AddNotification(Info);
                        }

                        if (Callback)
                        {
                            Callback(JsonObject);
                        }
                    }
                }
            }
        }
    };
}