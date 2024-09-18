// Copyright 2022 Guganana. All Rights Reserved.
#include "ExpressiveTextProcessor.h"

#include "Compiled/ExpressiveTextCompiler.h"
#include "Parameters/ExpressiveTextParams.h"
#include "Subsystems/ExpressiveTextSubsystem.h"

#if WITH_EDITOR
void UExpressiveTextProcessor::GetExtractions(UObject* WorldContext, FExpressiveText ExpressiveText, TArray<FExpressiveLineExtractionsInfo>& OutLinesExtractionsInfo)
{
	ExpressiveText.SetWorldContext(WorldContext);
	FExpressiveTextCompiler::MakeCompiler()->GetExtractions(ExpressiveText, OutLinesExtractionsInfo);		
}
#endif

TFuture<FCompiledExpressiveText> UExpressiveTextProcessor::CompileText(FExpressiveText ExpressiveText )
{
	TFuture<FCompiledExpressiveText> Future = FExpressiveTextCompiler::MakeCompiler()->Compile(ExpressiveText);
	return Future;
}


UExpressiveTextParameterValue* UExpressiveTextProcessor::GetParameter(const FCompiledExpressiveCharacter& Character, TSubclassOf<UExpressiveTextParameterValue> Type)
{
	if (*Type)
	{
		// Unreal BP's do not support object constness.
		return const_cast<UExpressiveTextParameterValue*>(&Character.GetValueObject(Type));
	}
	return nullptr;
}

void UExpressiveTextProcessor::StringIntoLinesArray(const FString& String, TArray<FString>& OutLines)
{
	String.ParseIntoArrayLines(OutLines, false);
}
