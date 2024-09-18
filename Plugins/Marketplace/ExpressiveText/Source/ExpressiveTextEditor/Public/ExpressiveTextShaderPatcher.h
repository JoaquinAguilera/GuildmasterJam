// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <HAL/IConsoleManager.h>
#include <Misc/EngineVersionComparison.h>
#include <Misc/MessageDialog.h>

class ExpressiveTextShaderPatcher
{

public:
    static void TryPatchShader()
    {
        bool HasPatchedAnything = false;
        FString Errors;
        TryPatchShadersImpl(Errors, HasPatchedAnything);

        if (Errors.Len() > 0)
        {
            const FText DlgTitle = FText::FromString(TEXT("Expressive Text: Failed to patch shaders"));
            const FText DlgMsg = FText::FromString(Errors);

#if UE_VERSION_OLDER_THAN( 5, 3, 0 )
			FMessageDialog::Open(EAppMsgType::Ok, DlgMsg,&DlgTitle);
#else
			FMessageDialog::Open(EAppMsgType::Ok, DlgMsg, DlgTitle);
#endif

            return;
        }

        if( HasPatchedAnything )
        {            
            const FText DlgTitle = FText::FromString(TEXT("Expressive Text: Shaders patched succesfully"));
            const FText DlgMsg = FText::FromString(TEXT("Slate shaders have been patched succesfully. Please restart the editor for the changes to take effect."));

#if UE_VERSION_OLDER_THAN( 5, 3, 0 )
			FMessageDialog::Open(EAppMsgType::Ok, DlgMsg,&DlgTitle);
#else
			FMessageDialog::Open(EAppMsgType::Ok, DlgMsg, DlgTitle);
#endif

	        FPlatformMisc::RequestExit(0);
        }
    }

private:

    struct FLoadedShader
    {
        FLoadedShader()
            : Name()
            , File()
            , BackupFile()
            , FileLoadSuccess( false )
            , BackupFileLoadSuccess( false )
            , IsFilePatched( false )
            , PatchVersion( -1 )
        {}

        FString Name;
        FString File;
        FString BackupFile;
        bool FileLoadSuccess;
        bool BackupFileLoadSuccess;
        bool IsFilePatched;
        int32 PatchVersion;
    };

    static void TryPatchShadersImpl( FString& Errors, bool& HasPatchedAnything );
    static TArray<FString> TouchedShaderFiles()
    {
        return {
            TEXT("SlateVertexShader.usf"),
            TEXT("SlateElementPixelShader.usf")
        };
    }
    
    static FAutoConsoleCommand CmdRemoveShaderPatch;
};