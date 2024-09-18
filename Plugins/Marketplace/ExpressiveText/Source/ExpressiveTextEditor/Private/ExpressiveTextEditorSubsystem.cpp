// Copyright 2022 Guganana. All Rights Reserved.
#include "ExpressiveTextEditorSubsystem.h"

#include "ExpressiveTextEditorFunctions.h"
#include "ExpressiveTextShaderPatcher.h"

#include <ExpressiveText/Public/ExpressiveTextSettings.h>
#include <ExpressiveText/Public/Animation/ExpressiveTextAnimation.h>
#include <ExpressiveText/Public/Asset/ExpressiveTextFont.h>
#include <ExpressiveText/Public/Compiled/ExpressiveTextCompiler.h>
#include <ExpressiveText/Public/Styles/ExpressiveTextStyleBase.h>
#include <ExpressiveText/Public/Subsystems/ExpressiveTextSubsystem.h>
#include <Framework/Notifications/NotificationManager.h>
#include <Guganana/Engine.h>
#include <Guganana/HTTP.h>
#include <HttpModule.h>
#include <Interfaces/IHttpRequest.h>
#include <Interfaces/IHttpResponse.h>
#include <Misc/EngineVersion.h>
#include <Engine/AssetManagerSettings.h>
#include <Engine/AssetManager.h>
#include <Widgets/Notifications/SNotificationList.h>

#include <JsonObjectConverter.h>
#include <SSettingsEditorCheckoutNotice.h>

FAutoConsoleCommand UExpressiveTextEditorSubsystem::CmdUpdateViewportCameraPosititioning(
        TEXT("ExpressiveTextEditor.UpdateViewportCameraPositioning"),
        TEXT("Updates the viewport camera positioning and feeds it to the ExpressiveText editor subsystem so it can be accessed by non-editor modules"),
        FConsoleCommandDelegate::CreateLambda(
            []()
            {
				if( GEngine )
				{
					if( auto* Subsystem = GEngine->GetEngineSubsystem<UExpressiveTextSubsystem>() )
					{								
						FVector Location;
						FRotator Rotation;
						UExpressiveTextEditorFunctions::GetViewportCameraInfo(Location, Rotation);

						Subsystem->ViewportCameraLoc = Location;
						Subsystem->ViewportCameraRot = Rotation;
					}
				}
            })
       );

UExpressiveTextEditorSubsystem::UExpressiveTextEditorSubsystem()
	: Super()
	, KeepLoadedClasses()
{
}

void UExpressiveTextEditorSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );

	Guganana::HTTP::ConfirmAppUsage(
		[](TSharedPtr<FJsonObject> ResponseObject) 
		{
			FString SpecialMessage;
			ResponseObject->TryGetStringField(TEXT("specialMessage"), SpecialMessage);

			if (!SpecialMessage.IsEmpty())
			{
				ExpressiveTextCompilerFlag::SpecialMode = true;
			}
		}
	);

	if (GEngine)
	{
		if (auto* Settings = GetDefault<UExpressiveTextSettings>())
		{
			if (!Settings->StopShaderPatchPrompts)
			{
				if (GEngine->IsInitialized())
				{
					OnEngineInitialized();
				}
				else
				{
					FCoreDelegates::OnPostEngineInit.AddUObject(this, &UExpressiveTextEditorSubsystem::OnEngineInitialized);
				}
			}
		}
	}
}

void UExpressiveTextEditorSubsystem::OnEngineInitialized()
{
	ExpressiveTextShaderPatcher::TryPatchShader();
	GenerateAssetManagerEntries();
}

void UExpressiveTextEditorSubsystem::GenerateAssetManagerEntries()
{
	// Based on GameFeaturesEditorModule.cpp
	UAssetManagerSettings* AssetManagerSettings = GetMutableDefault<UAssetManagerSettings>();

	const bool ShouldResave = static_cast<bool>(
		AddAssetManagerEntryForResource<ExpressiveTextResource::Animation>({ "/ExpressiveText",  "/Game" }) |
		AddAssetManagerEntryForResource<ExpressiveTextResource::Style>({ "/ExpressiveText",  "/Game" }) |
		AddAssetManagerEntryForResource<ExpressiveTextResource::Font>({ "/ExpressiveText",  "/Game" })
	);
	
	if (ShouldResave)
	{
		bool WantsCheckout = false;

		const FString& ConfigFileName = AssetManagerSettings->GetDefaultConfigFilename();
		FText NotificationOpText;
		if (!SettingsHelpers::IsCheckedOut(ConfigFileName, true))
		{
			FText ErrorMessage;
			WantsCheckout = SettingsHelpers::CheckOutOrAddFile(ConfigFileName, true, !IsRunningCommandlet(), &ErrorMessage);
			if (WantsCheckout)
			{
				NotificationOpText = FText::FromString("Checked out {0}");
			}
			else
			{
				bool MadeWritable = SettingsHelpers::MakeWritable(ConfigFileName);

				if (MadeWritable)
				{
					NotificationOpText = FText::FromString("Made {0} writable (you may need to manually add to version control)");
				}
				else
				{
					NotificationOpText = FText::FromString("Failed to check out {0} or make it writable, so no rule was added");
				}
			}
		}
		else
		{
			NotificationOpText = FText::FromString("Updated {0}");
		}

		// Show a message that the file was checked out/updated and must be submitted
		FNotificationInfo Info(FText::Format(NotificationOpText, FText::FromString(FPaths::GetCleanFilename(ConfigFileName))));
		Info.ExpireDuration = 3.0f;

		FSlateNotificationManager::Get().AddNotification(Info);
		AssetManagerSettings->SaveConfig(CPF_Config, *ConfigFileName);

		UAssetManager::Get().ReinitializeFromConfig();
	}
}

bool UExpressiveTextEditorSubsystem::AddAssetManagerEntryImpl(const FPrimaryAssetType& Type, UClass* Class, const TArray<FString>& Paths)
{
	UAssetManagerSettings* AssetManagerSettings = GetMutableDefault<UAssetManagerSettings>();

	// Load PrimaryAssetType classes for all entries
	bool Unused = false;
	for (auto& TypesToScan : AssetManagerSettings->PrimaryAssetTypesToScan)
	{
		TypesToScan.FillRuntimeData(Unused, Unused);
	}

	const auto ExistsPredicate = [Type](const FPrimaryAssetTypeInfo& AssetTypeInfo)
	{
		return AssetTypeInfo.PrimaryAssetType == Type.GetName();
	};

	const auto RecreateCondition = [Type, Class](const FPrimaryAssetTypeInfo& AssetTypeInfo)
	{
		return AssetTypeInfo.PrimaryAssetType == Type.GetName() && AssetTypeInfo.AssetBaseClassLoaded != Class;
	};

	if ( AssetManagerSettings->PrimaryAssetTypesToScan.ContainsByPredicate(ExistsPredicate) && 
		!AssetManagerSettings->PrimaryAssetTypesToScan.ContainsByPredicate(RecreateCondition) )
	{
		return false;
	}

	// Delete any old entries
	AssetManagerSettings->PrimaryAssetTypesToScan.RemoveAll(ExistsPredicate);

	TArray<FDirectoryPath> DirectoryPaths;
	for (const auto& Path : Paths)
	{
		FDirectoryPath NewPath;
		NewPath.Path = Path;
		DirectoryPaths.Add(NewPath);
	}

#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
	FPrimaryAssetTypeInfo NewTypeInfo(
		Type.GetName(),
		Class,
		false,
		false
	);

	// Only way to set the directories prior to UE 5.0 is going throught the reflextion system.
	FProperty* DirectoriesProp = FPrimaryAssetTypeInfo::StaticStruct()->FindPropertyByName(TEXT("Directories"));
	check(DirectoriesProp);

	TArray<FDirectoryPath>* DirectoriesPtr = DirectoriesProp->ContainerPtrToValuePtr<TArray<FDirectoryPath>>(&NewTypeInfo);
	(*DirectoriesPtr) = DirectoryPaths;

#else
	FPrimaryAssetTypeInfo NewTypeInfo(
		Type.GetName(),
		Class,
		false,
		false,
		TArray<FDirectoryPath>(DirectoryPaths),
		TArray<FSoftObjectPath>()
	);
#endif

	NewTypeInfo.Rules.CookRule = EPrimaryAssetCookRule::AlwaysCook;

	AssetManagerSettings->PrimaryAssetTypesToScan.Add(NewTypeInfo);

	AssetManagerSettings->Modify(true);
	AssetManagerSettings->PostEditChange();

	return true;
}
