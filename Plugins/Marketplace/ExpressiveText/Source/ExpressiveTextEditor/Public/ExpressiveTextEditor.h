// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include "Containers/Array.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ExpressiveTextEditorFunctions.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Guganana/Id.h"

#include <Developer/AssetTools/Public/AssetToolsModule.h>
#include <Developer/AssetTools/Public/IAssetTools.h>
#include <PropertyEditorModule.h>
#include <ExpressiveText/Public/ExpressiveTextSettings.h>
#include <InputCoreTypes.h>
#include <Framework/Commands/Commands.h>
#include <Framework/MultiBox/MultiBoxBuilder.h>
#include <Kismet/KismetSystemLibrary.h>
#include <LevelEditor.h>
#include <Interfaces/IPluginManager.h>
#include <Styling/SlateStyle.h>
#include <Styling/SlateStyleRegistry.h>
#include <Guganana/Plugin.h>
#include <Developer/ToolMenus/Public/ToolMenus.h>

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FExpressiveTextEditorModule::PluginImagePath( RelativePath, ".png" ), __VA_ARGS__ )

#define LOCTEXT_NAMESPACE "ExpressiveTextEditorModule"

class FExpressiveTextMenuCommands : public TCommands<FExpressiveTextMenuCommands>
{
public:

	FExpressiveTextMenuCommands()
		: TCommands<FExpressiveTextMenuCommands>(TEXT("FExpressiveTextEditorModule"), FText::FromString("Expressive Text"), NAME_None, TEXT("ExpressiveTextStyle") )
	{
	}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(OpenFontsManager, "Fonts Manager", "Open the fonts manager", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(ReportABug, "Report a bug", "Opens the report a bug website", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(JoinDiscord, "Join Discord ", "Joins our Discord Server as a verified user", EUserInterfaceActionType::Button, FInputChord());
	}
	
	void OnOpenFontsManager() const
	{
		FSoftObjectPath FontsManagerPanel(TEXT("/ExpressiveText/Core/Editor/CustomEditors/FontsManagerPanel.FontsManagerPanel"));
		UExpressiveTextEditorFunctions::StartCustomEditor(FontsManagerPanel);
	}

	void OnReportABug() const
	{
		UKismetSystemLibrary::LaunchURL(Guganana::PluginInformation::DocsPages::ReportABug());
	}
	
	void OnJoinDiscord() const
	{
		FGuid AppId = Guganana::Id::Get();
		FString DiscordLink = Guganana::PluginInformation::Endpoints::JoinDiscord() + AppId.ToString();
		UKismetSystemLibrary::LaunchURL(DiscordLink);
	}
	
	void FillCommandsList( TSharedRef<FUICommandList> CommandsList ) const
	{
		CommandsList->MapAction(
					OpenFontsManager,
					FExecuteAction::CreateRaw(this, &FExpressiveTextMenuCommands::OnOpenFontsManager),
					FCanExecuteAction());
					
 		CommandsList->MapAction(
					ReportABug,
					FExecuteAction::CreateRaw(this, &FExpressiveTextMenuCommands::OnReportABug),
					FCanExecuteAction());
				
		CommandsList->MapAction(
					JoinDiscord,
					FExecuteAction::CreateRaw(this, &FExpressiveTextMenuCommands::OnJoinDiscord),
					FCanExecuteAction());
	}

	void BuildSubMenuExtension(FMenuBuilder& Builder) const
	{
		TSharedRef<FUICommandList> Commands = MakeShareable(new FUICommandList);
		FillCommandsList(Commands);

		Builder.PushCommandList(Commands);

		Builder.BeginSection("General", LOCTEXT("ExpressiveTextSubMenu_GeneralSection", "General"));
		Builder.AddMenuEntry(OpenFontsManager);
		Builder.EndSection();

		Builder.BeginSection("Support", LOCTEXT("ExpressiveTextSubMenu_SupportSection", "Support"));
		Builder.AddMenuEntry(JoinDiscord);
		Builder.AddMenuEntry(ReportABug);
		Builder.EndSection();

		Builder.PopCommandList();
	}


	void RegisterMenus() const
	{
		if (!UToolMenus::IsToolMenuUIEnabled())
		{
			return;
		}

		FToolMenuOwnerScoped MenuOwner("ExpressiveText");
		UToolMenus* ToolMenus = UToolMenus::Get();

		UToolMenu* ToolsOption = ToolMenus->FindMenu("LevelEditor.MainMenu.Tools");
		if (!ToolsOption)
		{
			if (UToolMenu* Parent = ToolMenus->ExtendMenu("LevelEditor.MainMenu"))
			{
				Parent->AddSubMenu(FToolMenuOwner("LevelEditor.MainMenu"),
					"Tools",
					"Tools",
					LOCTEXT("Tools", "Tools"),
					LOCTEXT("Tools_Tooltip", "More tools")
				);
			}
		}

		ToolsOption = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Tools");
		FToolMenuSection& ExpressiveTextSection = ToolsOption->FindOrAddSection("ExpressiveText");
		
		ExpressiveTextSection.AddSubMenu( "ExpressiveText",
			LOCTEXT("ExpressiveTextMenu_Label", "Expressive Text"),
			LOCTEXT("ExpressiveTextMenu_Tooltip", "More Expressive Text Options"),
			FNewMenuDelegate::CreateRaw(this, &FExpressiveTextMenuCommands::BuildSubMenuExtension)
		);
	}

	TSharedPtr<FUICommandInfo> OpenFontsManager;
	TSharedPtr<FUICommandInfo> ReportABug;
	TSharedPtr<FUICommandInfo> JoinDiscord;
};

class FExpressiveTextEditorModule
	: public IModuleInterface
{

public:

	virtual void StartupModule() override
	{
		if (!StyleSet.IsValid())
		{
			const FVector2D Icon16x16(16.0f, 16.0f);
			const FVector2D Icon64x64(64.0f, 64.0f);
			const FVector2D Icon128x128(128.0f, 128.0f);

			StyleSet = MakeShareable(new FSlateStyleSet("ExpressiveTextStyle"));
			StyleSet->SetContentRoot(PluginContentDir() / TEXT("Core/Editor/Resources"));
			StyleSet->SetCoreContentRoot(PluginContentDir() / TEXT("Core/Editor/Resources"));

			StyleSet->Set("ExTextLogo", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/ExTextLogo", Icon64x64));
			
			auto* ExpressiveTextFontThumbnail = new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextFontIcon", Icon64x64);
			ExpressiveTextFontThumbnail->DrawAs = ESlateBrushDrawType::NoDrawType;
			StyleSet->Set("ClassThumbnail.ExpressiveTextFont", ExpressiveTextFontThumbnail);

#if UE_VERSION_OLDER_THAN(5,0,0)

			StyleSet->Set("ClassIcon.ExpressiveTextFont", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextFontIcon", Icon16x16));

			StyleSet->Set("ClassIcon.ExpressiveTextAsset", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextIcon", Icon16x16));
			StyleSet->Set("ClassThumbnail.ExpressiveTextAsset", new	IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextIcon", Icon64x64));

			StyleSet->Set("ClassIcon.ExText_ActionBase", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextActionIcon", Icon16x16));
			StyleSet->Set("ClassThumbnail.ExText_ActionBase", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextActionIcon", Icon64x64));
			
			StyleSet->Set("ClassIcon.ExpressiveTextStyle", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextStyleIcon", Icon16x16));
			StyleSet->Set("ClassThumbnail.ExpressiveTextStyle", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextStyleIcon", Icon64x64));
			
			StyleSet->Set("ClassIcon.ExpressiveTextAnimation", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextAnimationIcon", Icon16x16));
			StyleSet->Set("ClassThumbnail.ExpressiveTextAnimation", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextAnimationIcon", Icon64x64));
			
			StyleSet->Set("ClassIcon.ExpressiveTextMaterial", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextMaterialIcon", Icon16x16));
			StyleSet->Set("ClassThumbnail.ExpressiveTextMaterial", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE4/ExTextMaterialIcon", Icon64x64));
#else
			StyleSet->Set("ClassIcon.ExpressiveTextFont", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextFontIcon", Icon16x16));

			StyleSet->Set("ClassIcon.ExpressiveTextAsset", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextIcon", Icon16x16));
			StyleSet->Set("ClassThumbnail.ExpressiveTextAsset", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextIcon", Icon64x64));

			StyleSet->Set("ClassIcon.ExText_ActionBase", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextActionIcon", Icon16x16));
			StyleSet->Set("ClassThumbnail.ExText_ActionBase", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextActionIcon", Icon64x64));
			
			StyleSet->Set("ClassIcon.ExpressiveTextStyle", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextStyleIcon", Icon16x16));
			StyleSet->Set("ClassThumbnail.ExpressiveTextStyle", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextStyleIcon", Icon64x64));

			StyleSet->Set("ClassIcon.ExpressiveTextAnimation", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextAnimationIcon", Icon16x16));
			StyleSet->Set("ClassThumbnail.ExpressiveTextAnimation", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextAnimationIcon", Icon64x64));
			
			StyleSet->Set("ClassIcon.ExpressiveTextMaterial", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextMaterialIcon", Icon16x16));
			StyleSet->Set("ClassThumbnail.ExpressiveTextMaterial", new IMAGE_PLUGIN_BRUSH("Core/Editor/Resources/UE5/ExTextMaterialIcon", Icon64x64));
#endif

			FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
		}


		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(("PropertyEditor"));
		
		FExpressiveTextMenuCommands::Register();
		Commands = MakeShareable(new FUICommandList);
		FExpressiveTextMenuCommands::Get().FillCommandsList(Commands.ToSharedRef());

		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(
				&FExpressiveTextMenuCommands::Get(), 
				&FExpressiveTextMenuCommands::RegisterMenus
			)
		);

		RegisterAssetTools();
		RegisterMenuExtensions();
		RegisterSettings();
	}
	virtual void ShutdownModule() override
	{
		if (StyleSet.IsValid())
		{
			FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
			ensure(StyleSet.IsUnique());
			StyleSet.Reset();
		}

		FExpressiveTextMenuCommands::Unregister();

		UnregisterAssetTools();
		UnregisterMenuExtensions();
		UnregisterSettings();
		UnregisterStructCustomizations();	
	}

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

	TSharedPtr<FSlateStyleSet> StyleSet;
	static uint32 ExpressiveTextCategory;

protected:

	void RegisterAssetTools();

	/**
	 * Registers a single asset type action.
	 *
	 * @param AssetTools The asset tools object to register with.
	 * @param Action The asset type action to register.
	 */
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}

	/** Register the text asset editor settings. */
	void RegisterSettings()
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Editor", "Plugins", "ExpressiveText",
				LOCTEXT("ExpressiveTextSettingsName", "Expressive Text"),
				LOCTEXT("ExpressiveTextSettingsDescription", "Configure the ExpressiveText plugin."),
				GetMutableDefault<UExpressiveTextSettings>()
			);
		}
	}

	/** Unregisters asset tool actions. */
	void UnregisterAssetTools()
	{
		FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

		if (AssetToolsModule != nullptr)
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (auto Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	}

	/** Unregister the text asset editor settings. */
	void UnregisterSettings()
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterSettings("Editor", "Plugins", "ExpressiveText");
		}
	}

	void UnregisterStructCustomizations()
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		
		for (auto It = RegisteredPropertyTypes.CreateConstIterator(); It; ++It)
		{
			if(It->IsValid())
			{
				PropertyModule.UnregisterCustomPropertyTypeLayout(*It);
			}
		}
	
	}

	static FString PluginImagePath(const FString& RelativePath, const ANSICHAR* Extension)
	{
		return (PluginContentDir() / RelativePath) + Extension;
	}

	static FString PluginContentDir()
	{
		return IPluginManager::Get().FindPlugin(TEXT("ExpressiveText"))->GetContentDir();
	}

protected:

	/** Registers main menu and tool bar menu extensions. */
	void RegisterMenuExtensions()
	{
		MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
		ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
	}

	/** Unregisters main menu and tool bar menu extensions. */
	void UnregisterMenuExtensions()
	{
		MenuExtensibilityManager.Reset();
		ToolBarExtensibilityManager.Reset();
	}

private:

	void RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate)
	{
		check(PropertyTypeName != NAME_None);

		RegisteredPropertyTypes.Add(PropertyTypeName);
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomPropertyTypeLayout(PropertyTypeName, PropertyTypeLayoutDelegate);
	}

	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
	TArray< FName > RegisteredPropertyTypes;

	TSharedPtr<FUICommandList> Commands;

	/** Holds the plug-ins style set. */
	TSharedPtr<ISlateStyle> Style;

	/** Holds the tool bar extensibility manager. */
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
};

#undef IMAGE_PLUGIN_BRUSH

#undef LOCTEXT_NAMESPACE
