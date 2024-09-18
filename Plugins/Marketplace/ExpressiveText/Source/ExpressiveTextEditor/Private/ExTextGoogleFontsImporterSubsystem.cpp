// Copyright 2022 Guganana. All Rights Reserved.

#include "ExTextGoogleFontsImporterSubsystem.h"

FAutoConsoleCommandWithWorldAndArgs UExTextGoogleFontsImporterSubsystem::CmdDownloadFont = FAutoConsoleCommandWithWorldAndArgs(
    TEXT("ExpressiveTextEditor.DownloadGoogleFont"),
    TEXT("Download Google Font with the specified name"),
        FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
            [](const TArray<FString>& Args, UWorld* World)
            {
				if( auto* Subsystem = GEditor->GetEditorSubsystem<UExTextGoogleFontsImporterSubsystem>() )
				{
					Subsystem->StartDownload( Args[0] );
				}
			}
		)
	);