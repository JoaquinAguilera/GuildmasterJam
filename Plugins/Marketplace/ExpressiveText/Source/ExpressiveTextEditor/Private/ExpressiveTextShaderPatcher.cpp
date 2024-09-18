// Copyright 2022 Guganana. All Rights Reserved.

#include "ExpressiveTextShaderPatcher.h"

#include <Misc/FileHelper.h>
#include <Misc/Paths.h>
#include <HAL/FileManager.h>
#include <HAL/IConsoleManager.h>
#include <Misc/EngineVersionComparison.h>

void ExpressiveTextShaderPatcher::TryPatchShadersImpl( FString& Errors, bool& HasPatchedAnything )
{
    TOptional<bool> PermissionGranted;
    constexpr int CurrentPatchVersion = 2;
    const FString PatchMark = TEXT("ExpressiveTextPatchVersion");
    const FString ShadersPath = *FPaths::EngineDir() + FString(TEXT("Shaders/Private/"));

    const auto AddError = [&](const FString& NewError)
    {
        Errors = Errors + "\r\n" + NewError;
    };

    const auto GetOriginalShaderFileName = []( const FString& FileName )
    {
        return TEXT("Original_") + FileName;
    };

    const auto LoadShaderFile = [&]( const FString& ShaderFileName )
    {
        ensure(TouchedShaderFiles().Contains( ShaderFileName ));
        FLoadedShader Result;
        Result.Name = ShaderFileName;
        Result.FileLoadSuccess = FFileHelper::LoadFileToString(Result.File, *(ShadersPath + ShaderFileName) );
        Result.BackupFileLoadSuccess = FFileHelper::LoadFileToString(Result.BackupFile, *(ShadersPath + GetOriginalShaderFileName(ShaderFileName)) );
        Result.IsFilePatched = Result.File.Contains( PatchMark );

        check( Result.BackupFile.Contains(PatchMark) == false );

        if( !Result.FileLoadSuccess )
        {
            AddError(TEXT(" ExpressiveTextShaderPatcher:: Failed to load " + ShaderFileName));
            return Result;
        }

        if( Result.IsFilePatched )
        {
            TArray<FString> Split;
            Result.File.ParseIntoArray(Split, TEXT("ExpressiveTextPatchVersion:"));
            const FString PatchVersionString = Split[1];
            Result.PatchVersion = FCString::Atoi( *PatchVersionString );
            check( Result.PatchVersion > 0 );
        }

        return Result;
    };

    const auto SelectFileToPatch = [&]( const FLoadedShader& Shader )
    {
        check( Shader.FileLoadSuccess );            
        if( Shader.IsFilePatched )
        {
            check( Shader.BackupFileLoadSuccess );
            return Shader.BackupFile;
        }
        
        //Create backup of original
        FFileHelper::SaveStringToFile( Shader.File, *(ShadersPath + GetOriginalShaderFileName(Shader.Name)), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_EvenIfReadOnly );
        return Shader.File;
    };

    const auto AskForPermission = [&PermissionGranted, &AddError]() -> bool
    {
        if (!PermissionGranted.IsSet())
        {
            const FText DlgTitle = FText::FromString(TEXT("Expressive Text: Permission to patch Slate Shaders"));
            const FText DlgMsg = 
            FText::FromString(TEXT("Expressive text needs to patch the slate shaders in order to access vertex data and allow text to be rendered as efficiently as possible. These changes will not affect rendering of other slate elements in any way and can be reverted by running the console command 'ExpressiveText.RevertSlateShadersPatch'.\r\n \r\nYou can learn more at: \r\nhttps://expressivetext.com/8e864348-00db-4995-917c-a0b395dbde2b\r\n \r\nDo you want to proceed?"));

#if UE_VERSION_OLDER_THAN( 5, 3, 0 )
            auto Result = FMessageDialog::Open(EAppMsgType::YesNo, DlgMsg, &DlgTitle);
#else
            auto Result = FMessageDialog::Open(EAppMsgType::YesNo, DlgMsg, DlgTitle);
#endif
            PermissionGranted = Result == EAppReturnType::Yes ? true : false;

            if( Result == EAppReturnType::No )
            {
                AddError(TEXT("ExpressiveTextShaderPatcher:: User denied permission to patch Slate shaders - some styles will not render properly"));
            }
        }

        return PermissionGranted.GetValue();
    };
    
    const auto Patch = [&]( const FLoadedShader& Shader, const auto PatchFunction )
    {
        if( Shader.IsFilePatched && Shader.PatchVersion >= CurrentPatchVersion )
        {
            return;
        }

        if (AskForPermission())
        {
            auto ShaderFile = SelectFileToPatch(Shader);
            ShaderFile = ShaderFile + FString::Printf(TEXT("\r\n//ExpressiveTextPatchVersion:%i"), CurrentPatchVersion);
            PatchFunction(ShaderFile);
            FFileHelper::SaveStringToFile( ShaderFile, *(ShadersPath + Shader.Name), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_EvenIfReadOnly );
            HasPatchedAnything = true;
        }
    };

    const auto PatchValue = [&]( FString& ShaderFile, const FString& OldValue, const FString& NewValue )
    {
        ShaderFile.ReplaceInline( *OldValue, *(NewValue + TEXT(" //EXPRESSIVE_TEXT_PATCHED")) );
    };      

    auto VertexShader = LoadShaderFile( TEXT("SlateVertexShader.usf") );
    auto PixelShader = LoadShaderFile(TEXT("SlateElementPixelShader.usf"));

    if( !VertexShader.IsFilePatched || VertexShader.PatchVersion < CurrentPatchVersion )
    {
        //Inject vertex id so we can know the character index by dividing by 4
        Patch( VertexShader, [&PatchValue]( FString& File ){
            PatchValue( File, "VertexToPixelInterpolants Main(",  "VertexToPixelInterpolants Main(\r\n\tuint VertexId : SV_VertexID," );
            PatchValue( File, "float2(0,0)  //uv channel 7", "float2(VertexId, 0)" );
        } );
    }

    if (!PixelShader.IsFilePatched || PixelShader.PatchVersion < CurrentPatchVersion)
    {
        //Injects vertex id baked in VertexShader uv into the PixelShader
#if UE_VERSION_OLDER_THAN( 5, 2, 0 )	
        const FString PatchString =
            "Parameters.TexCoords[1] = InVertex.MaterialTexCoords;\r\n"
            "\t#elif NUM_MATERIAL_TEXCOORDS >= 3\r\n"
            "\t\tParameters.TexCoords[0] = GetUV(InVertex, 1);\r\n"
            "\t\tParameters.TexCoords[1] = InVertex.MaterialTexCoords;\r\n"
            "\t\tUNROLL\r\n"
            "\t\tfor (int CoordinateIndex = 2; CoordinateIndex < NUM_MATERIAL_TEXCOORDS; CoordinateIndex++)\r\n"
            "\t\t{\r\n"
            "\t\t\tParameters.TexCoords[CoordinateIndex] = GetUV(InVertex, CoordinateIndex);\r\n"
            "\t\t}";
#else
        const FString PatchString =
            "Parameters.TexCoords[1] = InVertex.MaterialTexCoords;\r\n"
            "\t#elif NUM_MATERIAL_TEXCOORDS >= 3\r\n"
            "\t\tParameters.TexCoords[0] = SLATE_UV1(InVertex);\r\n"
            "\t\tParameters.TexCoords[1] = InVertex.MaterialTexCoords;\r\n"
            "\r\n"
            "\t\tfloat2 UVs[8]; GetSlateUVs(InVertex, UVs);\r\n"
            "\t\tUNROLL\r\n"
            "\t\tfor (int CoordinateIndex = 2; CoordinateIndex < NUM_MATERIAL_TEXCOORDS; CoordinateIndex++)\r\n"
            "\t\t{\r\n"
            "\t\t\tParameters.TexCoords[CoordinateIndex] = UVs[CoordinateIndex];\r\n"
            "\t\t}";
#endif
        
        Patch(PixelShader, [&PatchValue, &PatchString](FString& File) {
            PatchValue(
                File,
                "Parameters.TexCoords[1] = InVertex.MaterialTexCoords;",
                PatchString
            );
        });
    }  

}


FAutoConsoleCommand ExpressiveTextShaderPatcher::CmdRemoveShaderPatch = FAutoConsoleCommand(
    TEXT("ExpressiveText.RevertSlateShadersPatch"),
    TEXT("Reverts the changes Expressive Text has made to the slate shaders"),
    FConsoleCommandDelegate::CreateLambda(
        []()
        {
            for (const auto& FileName : TouchedShaderFiles())
            {
                const FString ShadersPath = *FPaths::EngineDir() + FString(TEXT("Shaders/Private/"));

                const FString OriginalShaderFileName = TEXT("Original_") + FileName;

                const FString OriginalShaderFilePath = ShadersPath + OriginalShaderFileName;
                const FString ShaderFilePath = ShadersPath + FileName;

                if (FPaths::FileExists(OriginalShaderFilePath))
                {
                    IFileManager::Get().Move(*ShaderFilePath, *OriginalShaderFilePath, true, true);

                    const FText DlgTitle = FText::FromString(TEXT("Expressive Text: Successfuly reverted patched slate shaders"));
                    const FText DlgMsg =
                        FText::FromString(TEXT("The slate shaders patch has been reverted - visual changes will only happen after restarting the editor"));

        #if UE_VERSION_OLDER_THAN( 5, 3, 0 )
                    FMessageDialog::Open(EAppMsgType::Ok, DlgMsg, &DlgTitle);
        #else
                    FMessageDialog::Open(EAppMsgType::Ok, DlgMsg, DlgTitle);
        #endif
                }
            }

        }
    )
);