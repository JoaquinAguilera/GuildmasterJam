// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "ExpressiveTextEditor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "PackageHelperFunctions.h"
#include "Widgets/Notifications/SNotificationList.h"

#include <EditorSubsystem.h>
#include <Factories/FontFactory.h>
#include <Factories/FontFileImportFactory.h>
#include <AssetImportTask.h>
#include <Engine/FontFace.h>
#include <ExpressiveText/Public/ExpressiveTextSettings.h>
#include <ExpressiveText/Public/Asset/ExpressiveTextFont.h>
#include <ExpressiveText/Public/Parameters/ExpressiveTextParams.h>
#include <ExpressiveText/Public/Subsystems/ExpressiveTextSubsystem.h>
#include <Fonts/CompositeFont.h>
#include <Guganana/Plugin.h>
#include <Interfaces/IPluginManager.h>
#include <JsonObjectConverter.h>
#include <Misc/FileHelper.h>
#include <Misc/MessageDialog.h>
#include <UObject/UObjectIterator.h>

#if UE_VERSION_OLDER_THAN( 5, 1, 0 )
#include <AssetRegistryModule.h>
#else
#include <AssetRegistry/AssetRegistryModule.h>
#endif

#include "ExTextGoogleFontsImporterSubsystem.generated.h"


enum class FExTextNotificationLayer
{
	ImportFont,
	Misc
};

class ExTextNotificationManager : public FTickableGameObject
{
public:
	static TSharedPtr<SNotificationItem> DisplayNotification(FExTextNotificationLayer Layer, const FString& NotificationString, TArray<FNotificationButtonInfo> Buttons = TArray<FNotificationButtonInfo>())
	{
		auto& Instance = GetInstance();

		FText Text = FText::FromString("Expressive Text:\n" + NotificationString);

		NotificationEntry* FoundNotification = Instance.Notifications.Find(Layer);

		int64 ButtonsChecksum = 0;
		for (auto Button : Buttons)
		{
			ButtonsChecksum = HashCombine( GetTypeHash(Button.Text.ToString()), ButtonsChecksum);
		}

		// Remove notification from screen when adding a new one with buttons
		if (FoundNotification)
		{
			if (Buttons.Num() > 0 && Instance.LastButtonsChecksum != ButtonsChecksum)
			{
				FoundNotification->Entry->Fadeout();
				FoundNotification = nullptr;
				Instance.Notifications.Remove(Layer);
			}
		}

		if (!FoundNotification)
		{
			FNotificationInfo Info(Text);

			Info.bFireAndForget = false;
			Info.bUseLargeFont = false;
			Info.bUseThrobber = false;
			Info.bUseSuccessFailIcons = false;

			if (auto Style = FSlateStyleRegistry::FindSlateStyle("ExpressiveTextStyle"))
			{
				Info.Image = Style->GetBrush("ExTextLogo");
			}

			Info.ButtonDetails = Buttons;
			TSharedPtr<SNotificationItem> NewNotification = FSlateNotificationManager::Get().AddNotification(Info);
			NotificationEntry Entry;
			Entry.Entry = NewNotification;

			Instance.Notifications.Add(Layer, Entry);
		}

		FoundNotification = &Instance.Notifications.FindChecked(Layer);
		FoundNotification->Timer = 8.f;
		FoundNotification->Entry->SetText(Text);
		Instance.LastButtonsChecksum = ButtonsChecksum;

		auto CompletionState = Buttons.Num() > 0 ? SNotificationItem::ECompletionState::CS_Pending : SNotificationItem::ECompletionState::CS_Success;
		FoundNotification->Entry->SetCompletionState(CompletionState);

		return FoundNotification->Entry;
	}

private:

	static ExTextNotificationManager& GetInstance()
	{
		static ExTextNotificationManager Instance;
		return Instance;
	}

	virtual bool IsTickable() const
	{
		return Notifications.Num() > 0;
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
		RETURN_QUICK_DECLARE_CYCLE_STAT(ExTextNotificationManager, STATGROUP_Tickables);
	}

	// Manually handle expirations - lets us increment the expiration if new notifications come in
	virtual void Tick(float DeltaTime) override
	{
		for (auto It = Notifications.CreateIterator(); It; ++It)
		{
			auto& Value = It.Value();
			Value.Timer -= DeltaTime;
			if (Value.Timer <= 0.f)
			{
				Value.Entry->Fadeout();
				It.RemoveCurrent();
			}
		}
	}

private:

	struct NotificationEntry
	{
		TSharedPtr<SNotificationItem> Entry;
		float Timer;
	};

	TMap<FExTextNotificationLayer, NotificationEntry> Notifications;
	uint32 LastButtonsChecksum;
};

class FExTextGoogleFontsHelpers
{
public:

	static FString GetDownloadedFontsDirName()
	{
		return TEXT("DownloadedFonts");
	}

	static FString GetDownloadedFontsPath()
	{
		return FPaths::Combine(*IPluginManager::Get().FindPlugin(TEXT("ExpressiveText"))->GetContentDir(), GetDownloadedFontsDirName() ) + "/";
	}

};

USTRUCT()
struct FGithubTreeEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FString path;
};

USTRUCT()
struct FGithubTree
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FGithubTreeEntry> tree;
};

USTRUCT()
struct FGithubDirectoryDetailsEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FString name;
	
	UPROPERTY()
	FName type;

	UPROPERTY()
	FString download_url;
};


USTRUCT()
struct FGithubDirectoryDetails
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FGithubDirectoryDetailsEntry> entries;
};


USTRUCT()
struct FExText_GeneratedFontFace
{
	GENERATED_BODY()

	TStrongObjectPtr<UFontFace> FontFace;
	FName FontFaceFileName;
};


UCLASS( BlueprintType )
class UExText_FontDownloader : public UObject
{
	GENERATED_BODY()

public:

	UExText_FontDownloader()
		: Super()
		, SavedFontFaces()
		, FontName()
		, RemainingFilesToDownload(0)
		, FontFolderRelative()
		, FontFolderAbsolute()
		, TypefacesNamesMap()
		, HasWarnedAboutVariableFont(false)
	{
	}

	const FString& GetFontName() const
	{
		return FontName;
	}
	
	void Start( const FString& InFontName)
	{
		FontName = InFontName;
		ExTextNotificationManager::DisplayNotification(FExTextNotificationLayer::ImportFont, FString::Printf( TEXT("Retrieving font '%s' from Google Fonts"), *FontName ) );

		FontFolderRelative = FPaths::Combine(TEXT("/ExpressiveText/"), FExTextGoogleFontsHelpers::GetDownloadedFontsDirName(), FontName) + TEXT("/");
		FontFolderAbsolute = FPaths::Combine(FExTextGoogleFontsHelpers::GetDownloadedFontsPath(), FontName) + TEXT("/");

		auto Request = FHttpModule::Get().CreateRequest();
		Request->SetURL( FString("https://api.github.com/repos/google/fonts/contents/ofl/") + FontName );
		Request->SetVerb("GET");
		Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
		Request->SetHeader(TEXT("Accept"), TEXT("application/vnd.github.VERSION.object"));

		Request->OnProcessRequestComplete().BindUObject(this, &UExText_FontDownloader::GoogleFontsDownloadDetailsResponse);
		Request->ProcessRequest();
	}

	void GoogleFontsDownloadDetailsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if( bWasSuccessful )
		{
			FString JsonString = Response->GetContentAsString();
			FGithubDirectoryDetails ParsedResult;
			FJsonObjectConverter::JsonObjectStringToUStruct<FGithubDirectoryDetails>(JsonString, &ParsedResult, 0, 0);

			for( const auto& Entry : ParsedResult.entries )
			{
				DownloadAndSaveFile(Entry);
			}
		}
	}
	
	void DownloadAndSaveFile(const FGithubDirectoryDetailsEntry& Entry)
	{
		if( Entry.type != FName("file"))
		{
			return;
		}

		auto DownloadRequest = FHttpModule::Get().CreateRequest();
		DownloadRequest->SetURL( Entry.download_url );
		DownloadRequest->SetVerb("GET");
		DownloadRequest->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));

		RemainingFilesToDownload++;

		DownloadRequest->OnProcessRequestComplete().BindUObject(this, &UExText_FontDownloader::OnDownloadEnd, Entry.name);
		DownloadRequest->ProcessRequest();
	}
	

	void OnDownloadEnd(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FString FileName )
	{
		if( SanitizeVariableFontName(FileName) )
		{
			// if( !HasWarnedAboutVariableFont )
			// {
			// 	const FText DlgTitle = FText::FromString( TEXT("Expressive Text: You are importing a variable font"));
			// 	const FText DlgMsg = FText::FromString( FString::Printf( TEXT("The font '%s' is a variable font which isn't fully supported by the engine yet - this means only a portion of the possible font variations will be imported.\n\nDownloading the font manually from Google Fonts could provide more variations than the automated process." ), *FontName ) );
			// 	FMessageDialog::Open(EAppMsgType::Ok, DlgMsg, &DlgTitle);
			// 	HasWarnedAboutVariableFont = true;
			// }
		}
		SaveFile( FileName, Response->GetContent(), Response->GetContentAsString());
	}

	void SaveFile(const FString& FileName, const TArray<uint8>& FileData, const FString& DataAsString)
	{
		const FString FileFullPath = FPaths::Combine(FontFolderAbsolute, FileName);
		const FString FileRelativePath = FPaths::Combine(FontFolderRelative, FileName);

		FFileHelper::SaveArrayToFile(FileData, *FileFullPath);
		
		if( FileName == TEXT("METADATA.pb") )
		{
			ParseMetadata(DataAsString);			
		}
		else if( FileName.EndsWith(".ttf") )
		{
			FString FaceUAssetName = FString(TEXT("FontFace_")) + FileName.Left(FileName.Len() - 4);
			FString FaceUAssetPath = FPaths::Combine( FontFolderRelative, FaceUAssetName);
		
			const uint8* DataPtr = FileData.GetData();
			const uint8* DataEndPtr = DataPtr + FileData.Num();

			UPackage* FacePackage = MakePackage(*FaceUAssetPath);
			FacePackage->FullyLoad();
			
			UFontFileImportFactory* FontFaceFactory = NewObject<UFontFileImportFactory>();
			UAssetImportTask* ImportTask = NewObject< UAssetImportTask>();
			ImportTask->bAutomated = true; // Skip prompt to create UFont object

			FontFaceFactory->SetAssetImportTask(ImportTask);

			EObjectFlags Flags = RF_Public | RF_Standalone | RF_Transactional;
			if (UFontFace* FontFace = Cast<UFontFace>(FontFaceFactory->FactoryCreateBinary(UFontFace::StaticClass(), FacePackage, FName(*FaceUAssetName), Flags, nullptr, TEXT(".ttf"), DataPtr, DataEndPtr, GWarn)))
			{
				FontFace->SourceFilename = FileFullPath;
				FacePackage->MarkPackageDirty();	

				FExText_GeneratedFontFace GeneratedFontFace;
				GeneratedFontFace.FontFace = TStrongObjectPtr<UFontFace>(FontFace);
				GeneratedFontFace.FontFaceFileName = FName(*FileName);

				SavedFontFaces.Add(FName(*FileName), FontFace);
				UEditorLoadingAndSavingUtils::SavePackages({ FacePackage }, false);				
			}
		}

		RemainingFilesToDownload--;

		if( RemainingFilesToDownload == 0 )
		{
			FinishedAllDownloads();
		}
	}

	void ParseMetadata( const FString& MetadataFile )
	{
		FString DataBeingParsed = MetadataFile;
		FString Unused;

		const auto CleanupParsedParam = [this]( FString& OutParam ){
			OutParam.ReplaceInline(TEXT("\""), TEXT(""));
			OutParam.TrimStartAndEndInline();
		};

		bool FoundPostScript;
		do
		{
			FString ParsedScriptName;
			FString ParsedFileName;
			FString PostScriptRightString;
			FString FileNameRightString;

			FoundPostScript = DataBeingParsed.Split(TEXT("post_script_name:"), &Unused, &PostScriptRightString);
			bool FoundFileName = DataBeingParsed.Split(TEXT("filename:"), &Unused, &FileNameRightString);

			if (FoundPostScript && FoundFileName)
			{		
				PostScriptRightString.Split("\n", &ParsedScriptName, &DataBeingParsed);
				FileNameRightString.Split("\n", &ParsedFileName, &Unused);

				if (ParsedScriptName.Len() > 0 && ParsedFileName.Len() > 0 )
				{
					CleanupParsedParam(ParsedScriptName);
					CleanupParsedParam(ParsedFileName);

					ParsedScriptName.ReplaceInline(*(FontName + FString(TEXT("-"))), TEXT(""));
					SanitizeVariableFontName(ParsedFileName);
					
					TypefacesNamesMap.Add(FName( *ParsedFileName ), FName(*ParsedScriptName) );
				}
			}
		} while (FoundPostScript);
	}
		
	void FinishedAllDownloads()
	{
		UFontFactory* FontFactory = NewObject<UFontFactory>();
		FontFactory->bEditAfterNew = false;

		FString UFontPackageName = TEXT("Font_") + FontName;
		FString FontPackagePath = FontFolderRelative + UFontPackageName;
		FString ExpressiveTextFontPackagePath = FontFolderRelative + FontName;

		UPackage* FontPackage = MakePackage(*FontPackagePath);
		UPackage* ExpressiveTextFontPackage = MakePackage(*ExpressiveTextFontPackagePath);


		EObjectFlags Flags = RF_Public | RF_Standalone | RF_Transactional;
		UFont* Font = Cast<UFont>(FontFactory->FactoryCreateNew(UFont::StaticClass(), FontPackage, *UFontPackageName, Flags, nullptr, GWarn));
		if (Font)
		{
			Font->FontCacheType = EFontCacheType::Runtime;

			for( const auto SavedFacePair : SavedFontFaces )
			{
				if( auto* ResultName = TypefacesNamesMap.Find(SavedFacePair.Key ) )
				{
					auto& FontFaceEntries = Font->CompositeFont.DefaultTypeface.Fonts;
					FTypefaceEntry& TypefaceEntry = *ResultName == FName("Regular") ? FontFaceEntries.Insert_GetRef(FTypefaceEntry(), 0) : FontFaceEntries.AddDefaulted_GetRef();
					TypefaceEntry.Name = *ResultName;
					TypefaceEntry.Font = FFontData(SavedFacePair.Value );
				}
			}

			auto* ExpressiveTextFont = NewObject<UExpressiveTextFont>(ExpressiveTextFontPackage, *FontName, Flags);
			ExpressiveTextFont->Font = Font;

			FAssetRegistryModule::AssetCreated(Font);
			FAssetRegistryModule::AssetCreated(ExpressiveTextFont);

			FontPackage->MarkPackageDirty();
			UEditorLoadingAndSavingUtils::SavePackages( { ExpressiveTextFontPackage, FontPackage }, false );
			
			ExTextNotificationManager::DisplayNotification( FExTextNotificationLayer::ImportFont, FString::Printf( TEXT("Font '%s' has been successfully imported to:\n%s"), *FontName, *FExTextGoogleFontsHelpers::GetDownloadedFontsPath() ) );
			Stop(ExpressiveTextFont);
		}
	}

	void Stop( UExpressiveTextFont* DownloadedFont )
	{
#if WITH_EDITORONLY_DATA
		if( DownloadedFont )
		{
			for ( TObjectIterator<UExTextValue_Font> It; It; ++It )
			{
				if ( It )
				{
					It->NewFontDownloaded( FName( *FontName ), DownloadedFont );
				}
			}
		}
#endif
		OnDownloadStopped.Broadcast();
		ConditionalBeginDestroy();	
	}

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDownloadStopped);
	UPROPERTY(BlueprintAssignable, Category ="Expressive Text Editor")
	FOnDownloadStopped OnDownloadStopped;

private:

	UPackage* MakePackage(FString PackageName)
	{
#if UE_VERSION_OLDER_THAN( 4, 26, 0 )
		return CreatePackage(nullptr, *PackageName);
#else
		return CreatePackage(*PackageName);
#endif
	}

	bool SanitizeVariableFontName( FString& FontNameToSanitize )
	{
		if (FontNameToSanitize.Find("[") >= 0)
		{
			FString Temp;
			FString Left;
			FString Right;
			FString Unused;

			FontNameToSanitize.Split("[", &Left, &Temp);
			Temp.Split("]", &Unused, &Right);

			FontNameToSanitize = Left + Right;
			return true;
		}

		return false;
	}

	UPROPERTY()
	TMap<FName, UFontFace* > SavedFontFaces;

	FString FontName;
	int32 RemainingFilesToDownload;

	FString FontFolderRelative;
	FString FontFolderAbsolute;
	TMap<FName, FName> TypefacesNamesMap;
	bool HasWarnedAboutVariableFont;
};

UCLASS()
class EXPRESSIVETEXTEDITOR_API UExTextGoogleFontsImporterSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	void TryImportFont(FName FontName, TOptional<FString> OnMissingFontMessage, bool ShouldAsk = true)
	{
		FString FontString = FontName.ToString();
		FontString.ToLowerInline();

		// check if we are already downloading
		for (const auto* Downloader : FontsBeingDownloaded)
		{
			if (Downloader->GetFontName().Equals(FontString, ESearchCase::IgnoreCase))
			{
				return;
			}
		}

		FString ExpectedFontDirectory = FPaths::Combine(FExTextGoogleFontsHelpers::GetDownloadedFontsPath(), FontString);

		if (auto* Settings = GetDefault<UExpressiveTextSettings>())
		{
			FString CustomPath = Settings->CustomDownloadedFontsDirectory.Path;
			if (CustomPath.Len() > 0)
			{
				if (FPaths::DirectoryExists(CustomPath))
				{
					ExpectedFontDirectory = FPaths::Combine(CustomPath, FontString) + "/";
				}
			}
		}

		// Font already downloaded?
		if (FPaths::DirectoryExists(ExpectedFontDirectory))
		{
			return;
		}

		const int32 FoundDataIndex = Algo::BinarySearchBy(GoogleFontsList.tree, FontString, &UExTextGoogleFontsImporterSubsystem::BinarySearchKey);
		if (FoundDataIndex != INDEX_NONE)
		{
			if (ShouldAsk)
			{
				// Need to check notification weak ptr to avoid spam - missing fonts events are fired every frame
				if (!NotificationWeakPtr.Pin())
				{
					TArray<FNotificationButtonInfo> Buttons;

					Buttons.Add(FNotificationButtonInfo(
						FText::FromString(TEXT("Import")),
						FText::FromString(TEXT("Download and automatically import the font")),
						FSimpleDelegate::CreateUObject(this, &UExTextGoogleFontsImporterSubsystem::DoStartDownload, FontString))
					);

					Buttons.Add(FNotificationButtonInfo(
						FText::FromString(TEXT("Open fonts manager")),
						FText::FromString(TEXT("Opens Expressive Text's fonts manager to download missing fonts")),
						FSimpleDelegate::CreateRaw(&FExpressiveTextMenuCommands::Get(), &FExpressiveTextMenuCommands::OnOpenFontsManager))
					);

					FString Message;
					if (OnMissingFontMessage.IsSet())
					{
						Message += OnMissingFontMessage.GetValue() + "\n";
					}

					Message += FString::Printf(TEXT("Would you like to import font '%s' from google fonts?"), *FontString);

					NotificationWeakPtr = ExTextNotificationManager::DisplayNotification(FExTextNotificationLayer::ImportFont, Message, Buttons);

					//if (NotificationWeakPtr.Pin())
					//{
					//	NotificationWeakPtr.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
					//}
				}
			}
			else
			{
				StartDownload(FontString);
			}
		}
	}

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override
	{
		Super::Initialize(Collection);
		GoogleFontsDownloader();

		if( GEngine )
		{
			if( auto* ExTextSubsystem = GEngine->GetEngineSubsystem<UExpressiveTextSubsystem>() )
			{
				ExTextSubsystem->OnMissingFont().AddUObject(this, &UExTextGoogleFontsImporterSubsystem::TryImportFont, true);
			}
		}
	}
	

	void GoogleFontsDownloader()
	{
		if (IsRunningCommandlet())
		{
			return;
		}

		auto Request = FHttpModule::Get().CreateRequest();
		Request->SetURL(Guganana::PluginInformation::Endpoints::GithubOFLFonts());
		Request->SetVerb("GET");
		Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));

		Request->OnProcessRequestComplete().BindUObject(this, &UExTextGoogleFontsImporterSubsystem::GoogleFontsResponse);
		Request->ProcessRequest();
	}

	void GoogleFontsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if( bWasSuccessful )
		{
			FString JsonString = Response->GetContentAsString();
			FJsonObjectConverter::JsonObjectStringToUStruct<FGithubTree>(JsonString, &GoogleFontsList, 0, 0);
		}
	}

	void DoStartDownload(FString FontString)
	{
		StartDownload(FontString);
	}


	UFUNCTION(BlueprintCallable, Category=ExpressiveTextEditor)
	UExText_FontDownloader* StartDownload( FString FontString )
	{
		auto* FontsDownloader = NewObject<UExText_FontDownloader>();
		FontsDownloader->Start( FontString.ToLower() );
		FontsBeingDownloaded.Add(FontsDownloader);
		return FontsDownloader;
	}

	static FString BinarySearchKey(const FGithubTreeEntry& Entry)
	{
		return Entry.path;
	} 

	UFUNCTION( BlueprintCallable, Category = ExpressiveTextEditor)
	bool IsFontAvailableOnGoogleFonts( const FString& FontString)const
	{
		const int32 FoundDataIndex = Algo::BinarySearchBy(GoogleFontsList.tree, FontString, &UExTextGoogleFontsImporterSubsystem::BinarySearchKey);
		return FoundDataIndex != INDEX_NONE;
	}
	
	UFUNCTION( BlueprintCallable, Category = ExpressiveTextEditor)
	bool IsDownloadingAnyFont( const FString& FontString)
	{
		FontsBeingDownloaded.Remove(nullptr);
		return FontsBeingDownloaded.Num() > 0;
	}

	UPROPERTY()
	TArray<UExText_FontDownloader*> FontsBeingDownloaded;
	
	FGithubTree GoogleFontsList;
	TWeakPtr<SNotificationItem> NotificationWeakPtr;

    static FAutoConsoleCommandWithWorldAndArgs CmdDownloadFont;
};


