// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "CompiledExpressiveText.h"
#include "../Animation/ExpressiveTextAnimation.h"
#include "../Debug/ExpressiveTextDebugger.h"
#include "../ExpressiveTextSettings.h"
#include "../Extractions/TagsExtraction.h"
#include "../Extractions/TreeExtraction.h"
#include "../Handles/ExpressiveText.h"
#include "../Interjections/ExText_Interjection.h"
#include "../Interjections/ExText_ActionInterjection.h"
#include "../Interjections/ExText_PauseInterjection.h"
#include "../Parameters/ExpressiveTextParameterLookup.h"
#include "../Parameters/Extractors/ExpressiveTextInlineParameterExtractor.h"
#include "../Resources/ExpressiveTextResources.h"
#include "../Styles/ExpressiveTextStyle.h"
#include "../Styles/ExpressiveTextStyleBase.h"
#include "../Subsystems/ExpressiveTextSubsystem.h"

#include <Engine/Engine.h>
#include <Engine/Font.h>
#include <Engine/GameEngine.h>
#include <Engine/UserInterfaceSettings.h>
#include <Fonts/FontCache.h>
#include <Fonts/CompositeFont.h>
#include <Framework/Application/SlateApplication.h>
#include <Framework/Text/ShapedTextCache.h>
#include <Guganana/Async.h>
#include <Guganana/Logging.h>
#include <Kismet/KismetStringLibrary.h>
#include <Blueprint/WidgetLayoutLibrary.h>

namespace InterjectionTypes
{
    extern const FName Pause;
    extern const FName P;
    extern const FName Action;
    extern const FName A;
}

namespace ExpressiveTextCompilerFlag
{
	EXPRESSIVETEXT_API extern bool SpecialMode;
}

// Struct to help with latent steps of text "compilation", more specifically loading required assets
class FExpressiveTextCompiler : public TSharedFromThis<FExpressiveTextCompiler>
{

private:

	FExpressiveTextCompiler()
		: ExpressiveText()
		, TempCompiledText()
		, OnCompiledText()
		, TextStringRef( MakeShareable( new FString ) )
		, LinesRefs()
		, LinesExtractions()
		, IsCompiling(false)
		, DryRun(false)
	{
	}


	FExpressiveText ExpressiveText;
	FCompiledExpressiveText TempCompiledText;
	TPromise<FCompiledExpressiveText> OnCompiledText;
	TSharedRef<FString> TextStringRef;
	TArray<TSharedRef<FString>> LinesRefs;
	TArray<TSharedRef<FExpressiveTextExtraction>> LinesExtractions;
	bool IsCompiling;
	bool DryRun;
public:


	static TSharedRef<FExpressiveTextCompiler> MakeCompiler()
	{
		return MakeShareable(new FExpressiveTextCompiler());
	}


	TFuture<FCompiledExpressiveText> Compile(FExpressiveText InExpressiveText)
	{
		//DECLARE_CYCLE_STAT(TEXT("CompileText"), STAT_CompileText, STATGROUP_ExpressiveText);
		//SCOPE_CYCLE_COUNTER(STAT_CompileText);

		check(!IsCompiling);
		ExpressiveText = InExpressiveText;
		IsCompiling = true;

		const auto& Fields = ExpressiveText.GetFields();
		TextStringRef = MakeShareable(new FString(Fields.Text.ToString()));
		TArray<FString> Lines;
		TextStringRef->ParseIntoArrayLines(Lines, false);

		TArray<TFuture<void>> AllExtractionsGenerated;
		for (int32 i = 0; i < Lines.Num(); i++)
		{
			LinesRefs.Emplace(MakeShareable(new FString(Lines[i])));
			AllExtractionsGenerated.Add(GenerateExtraction(i));
		}

		TFuture<FCompiledExpressiveText> TextCompiled = Guganana::Async::WhenAllFutures(AllExtractionsGenerated).Next(
			[SharedCompiler = TSharedPtr<FExpressiveTextCompiler>(AsShared())](auto)
			{
				// TODO: SharedCompiler will be destroyed if we don't store the TFuture. 
				// this needs reworking to avoid that
				if (FExpressiveTextCompiler* Compiler = SharedCompiler.Get())
				{
					SharedCompiler->PopulateRuns();
					SharedCompiler->OnCompiledText.EmplaceValue(SharedCompiler->TempCompiledText);
					return SharedCompiler->TempCompiledText;
				}

				return FCompiledExpressiveText();
			}
		);

		return TextCompiled;
	}

	void GetExtractions(FExpressiveText InExpressiveText, TArray<FExpressiveLineExtractionsInfo>& OutLinesExtractionsInfo)
	{
		check(!IsCompiling);
		ExpressiveText = InExpressiveText;
		IsCompiling = true;
		DryRun = true;
		const auto& Fields = ExpressiveText.GetFields();

		if (FSlateApplication::IsInitialized())
		{
			TSharedRef<FString> TextAsStringRef = MakeShareable(new FString(Fields.Text.ToString()));
			TArray<FString> Lines;
			TextAsStringRef->ParseIntoArrayLines(Lines, false);

			for (int32 i = 0; i < Lines.Num(); i++)
			{
				FString& Line = Lines[i];
				LinesRefs.Emplace(MakeShareable(new FString(Line)));
				GenerateExtraction(i);

				FExpressiveLineExtractionsInfo NewInfo;
				NewInfo.UnprocessedLine = Line;
				NewInfo.Extraction = LinesExtractions[i];
				OutLinesExtractionsInfo.Add(NewInfo);
			}
		}

		OnCompiledText.SetValue(TempCompiledText);
	}

private:
	TSharedPtr<FExpressiveTextParameterLookup> CreateDefaultParameterLookup(UExpressiveTextStyleBase* CustomDefaultStyle, TOptional<int32> DefaultFontSize = TOptional<int32>())
	{
		TSharedPtr<FExpressiveTextParameterLookup> Result;

		// In ascending order of lookup priority:

		if (auto* Settings = GetDefault<UExpressiveTextSettings>())
		{
			if (auto* DefaultStyle = Settings->GetDefaultStyle())
			{
				Result = DefaultStyle->GetParameterLookup();
			}
		}

		if (CustomDefaultStyle)
		{
			TSharedPtr<FExpressiveTextParameterLookup> CustomDefault = CustomDefaultStyle->GetParameterLookup();
			CustomDefault->SetNext(Result);
			Result = CustomDefault;
		}

		if (DefaultFontSize.IsSet())
		{
			TSharedPtr<IExpressiveTextParameterExtractor> FontSizeParameter = FExpressiveTextInlineParameterExtractor::Create<UExTextValue_FontSize>(DefaultFontSize.GetValue());
			TSharedPtr<FExpressiveTextParameterLookup> DefaultFontSizeLookup = MakeShareable(new FExpressiveTextParameterLookup(FName("Fields Default Font Size"), FontSizeParameter));
			DefaultFontSizeLookup->SetNext(Result);
			Result = DefaultFontSizeLookup;
		}

		return Result;
	}

	void PopulateRuns()
	{
		const auto& Fields = ExpressiveText.GetFields();
		UObject* WorldContext = ExpressiveText.GetWorldContext();
		check(WorldContext);

		UWorld* World = WorldContext->GetWorld();

		const FExpressiveTextContext& Context = ExpressiveText.GetContext();
		auto TextLayout = ExpressiveText.GetTextLayout();
		TextLayout->SetJustification(Fields.Justification);

		if (ExpressiveTextCompilerFlag::SpecialMode)
		{
			return;
		}

		if (FSlateApplication::IsInitialized())
		{
			float Chronometer = 0.f;
			TArray<TSharedRef<FExpressiveTextRun>> AllRuns;
			TArray<FTextLayout::FNewLineData> LineDatas;

			for ( int LineIndex = 0; LineIndex < LinesRefs.Num(); LineIndex++ )
			{
				TSharedRef<FString> Line = LinesRefs[LineIndex];
				TSharedRef<FExpressiveTextExtraction> LineExtraction = LinesExtractions[LineIndex];

				TArray<TSharedRef<FExpressiveTextRun>> Runs;
				PopulateRunsFromExtraction(World, Line, LineExtraction, Runs, TextLayout, Chronometer);

				TArray<TSharedRef<IRun>> CastRuns;
				for (auto& Run : Runs)
				{
					CastRuns.Add(Run);
					AllRuns.Add(Run);
				}

				LineDatas.Add(FTextLayout::FNewLineData(Line, CastRuns));
			}

			EvaluateEndTimesForDirection(AllRuns, EExText_ClearDirection::Forwards, Chronometer);
			EvaluateEndTimesForDirection(AllRuns, EExText_ClearDirection::Backwards, Chronometer);

			TextLayout->AddLines(LineDatas);
			TextLayout->UpdateLayout();

			const FVector2D TextSize = TextLayout->GetDrawSize();
			TempCompiledText.DrawSize = TextSize;
			TempCompiledText.Alignment = Fields.Alignment;
			TempCompiledText.WrapSettings = Fields.WrapSettings;
			TempCompiledText.UseAutoSize = Fields.UseAutoSize;
		}
	}


	FSlateFontInfo GetFontInfoFromLookup(const TSharedPtr<FExpressiveTextParameterLookup>& Lookup)
	{
		if (auto* Font = Lookup->GetValue<UExTextValue_Font>())
		{
			if (auto* FontObject = Font->GetObject())
			{
				FName TypefaceToUse = Lookup->GetValue<UExTextValue_Typeface>();

				// Use default typeface if none is specified
				if (TypefaceToUse.IsNone())
				{
					const auto& Fonts = FontObject->CompositeFont.DefaultTypeface.Fonts;
					if (Fonts.Num() > 0)
					{
						TypefaceToUse = Fonts[0].Name;
					}
				}

				FSlateFontInfo FontInfo(FontObject, Lookup->GetValue<UExTextValue_FontSize>(), TypefaceToUse);
				FontInfo.LetterSpacing = Lookup->GetValue<UExTextValue_LetterSpacing>();
				return FontInfo;
			}
		}

		return FSlateFontInfo();
	}


	void PopulateRunsFromExtraction(UWorld* World, TSharedRef<FString> TextAsStringRef, TSharedRef<FExpressiveTextExtraction> Extraction, TArray<TSharedRef<FExpressiveTextRun>>& Runs, TSharedRef<FExpressiveTextSlateLayout> Layout, float& RevealStartTimer)
	{
		auto SharedData = Layout->GetSharedData();

		const auto AddRun = [this, &World, &RevealStartTimer, &Runs, &Extraction, &TextAsStringRef, &SharedData, &Layout](const TSharedPtr<FExpressiveTextParameterLookup>& Lookup, const FTextRange& Range, const TArray<FExText_ParsedInterjection>& Interjections)
		{
			if (Lookup->GetValue<UExTextValue_ForceFullTextShapingMethod>())
			{
				Layout->SetTextShapingMethod(ETextShapingMethod::FullShaping);
			}

			const bool DrawEachGlyphSeperately =
				(Lookup->GetValue<UExTextValue_RevealAnimation>().Animation != nullptr && Lookup->GetValue<UExTextValue_RevealRate>() > 0.f) ||
				(Lookup->GetValue<UExTextValue_ClearAnimation>().Animation != nullptr && Lookup->GetValue<UExTextValue_ClearRate>() > 0.f) ||
				Lookup->GetValue<UExTextValue_ForceDrawEachGlyphSeparately>();

			// separate each character to a different run so it can apply per character animations
			if (DrawEachGlyphSeperately)
			{
				// Break run into smaller ones so each glyph can be processed individually for animation purposes
				for (int i = Range.BeginIndex; i < Range.EndIndex; i++)
				{
					TArray<FExText_ParsedInterjection> InterjectionForThisGlyph;
					for (const auto& Interjection : Interjections)
					{
						const bool IsLastGlyphInterjection = i == Range.EndIndex - 1 && Interjection.Index == Range.EndIndex;
						if (Interjection.Index == i || IsLastGlyphInterjection)
						{
							InterjectionForThisGlyph.Add(Interjection);
						}
					}

					const TSharedRef<FExpressiveTextRun> Run = FExpressiveTextSlateLayout::CreateRun(TextAsStringRef, GetFontInfoFromLookup(Lookup), FTextRange(i, i + 1), SharedData, RevealStartTimer);
					Run->SetParameterLookup(Lookup);
					Run->SetOwnerExtraction(Extraction);
					Run->SetInterjections(InterjectionForThisGlyph);
					Run->SetWorld(World);
					Runs.Add(Run);
					RevealStartTimer += Run->CalculateDurationToFullyReveal();
				}
			}
			else
			{
				const TSharedRef<FExpressiveTextRun> Run = FExpressiveTextSlateLayout::CreateRun(TextAsStringRef, GetFontInfoFromLookup(Lookup), Range, SharedData, RevealStartTimer);
				Run.Get().SetParameterLookup(Lookup);
				Run.Get().SetOwnerExtraction(Extraction);
				Run->SetInterjections(Interjections);
				Run->SetWorld(World);
				Runs.Add(Run);
				RevealStartTimer += Run->CalculateDurationToFullyReveal();
			}
		};


		FInterjectionVisitor InterjectionVisitor(Extraction);

		int32 CurrentInterjectionIndex = 0;
		Extraction->ExtractionTree->TrawlSections(
			[&AddRun, &Extraction, &CurrentInterjectionIndex](const FTreeExtraction& TreeExtraction, const FTextRange& Range)
			{
				TArray<FExText_ParsedInterjection> ParsedInterjections;

				while (Extraction->Interjections.IsValidIndex(CurrentInterjectionIndex))
				{
					const auto& Interjection = Extraction->Interjections[CurrentInterjectionIndex];
					if (Interjection.Index >= Range.BeginIndex && Interjection.Index <= Range.EndIndex)
					{
						CurrentInterjectionIndex++;
						ParsedInterjections.Add(Interjection);
					}
					else
					{
						break;
					}
				}

				AddRun(TreeExtraction.ParameterLookup, Range, ParsedInterjections);
			}
		);

		SharedData->Chronos.SetRevealDuration(RevealStartTimer);
	}


	void PopulateCompiledTextFromTextLayout(const TArray<FString>& TextLines, const FTextLayout& TextLayout, FCompiledExpressiveText& CompiledText, const FVector2D StartPos)
	{
		const float FontScale = 1.0f;
		constexpr float Scale = 1.f;
		const float InLifetime = 5.f;
		float Chronometer = 0.f;

		FInterjectionVisitor InterjectionVisitor(nullptr);
		TSharedPtr<FExpressiveTextExtraction> CurrentExtraction;

		for (const FTextLayout::FLineView& LineView : TextLayout.GetLineViews())
		{
			const FString& LineStringRef = TextLines[LineView.ModelIndex];

			// Render every block for this line
			for (const TSharedRef< ILayoutBlock >& Block : LineView.Blocks)
			{
				const auto BlockRange = Block->GetTextRange();
				const FVector2D BlockOffset = Block->GetLocationOffset();
				const FLayoutBlockTextContext BlockTextContext = Block->GetTextContext();

				const FExpressiveTextRun& Run = ((FExpressiveTextRun&)Block->GetRun().Get());
				const FTextBlockStyle& RunStyle = Run.GetStyle();
				TSharedPtr<FExpressiveTextExtraction> NewExtraction = Run.GetOwnerExtraction();

				if (NewExtraction != CurrentExtraction)
				{
					CurrentExtraction = NewExtraction;
					InterjectionVisitor = FInterjectionVisitor(CurrentExtraction);
				}

				// Make sure we have up-to-date shaped text to work with
				// We use the full line view range (rather than the run range) so that text that spans runs will still be shaped correctly
				FShapedGlyphSequenceRef ShapedText = ShapedTextCacheUtil::GetShapedTextSubSequence(
					BlockTextContext.ShapedTextCache,
					FCachedShapedTextKey(LineView.Range, Scale, BlockTextContext, RunStyle.Font),
					BlockRange,
					*LineStringRef,
					BlockTextContext.TextDirection
				);

				int32 CurrentSequenceHeight = ShapedText->GetMaxTextHeight();
				int32 CurrentSequenceWidth = ShapedText->GetMeasuredWidth();

				FVector2D CurPos = StartPos;
				const auto& Glyphs = ShapedText->GetGlyphsToRender();
				for (int32 i = 0; i < Glyphs.Num(); i++)
				{
					const auto& Glyph = Glyphs[i];
					const TCHAR& Character = LineStringRef[Glyph.SourceIndex];
					FString CharAsString;
					CharAsString.AppendChars(&Character, 1);

					// Generate new glyph
					FCompiledExpressiveCharacter& CharacterCompiledData = CompiledText.Characters.AddDefaulted_GetRef();
					CharacterCompiledData.Glyph = CharAsString;

					// Generate glyph position
					FVector2D GlyphPos = CurPos;
					GlyphPos.X += Glyph.XOffset;
					GlyphPos.Y += Glyph.YOffset;
					CurPos.X += Glyph.XAdvance;
					CurPos.Y += Glyph.YAdvance;
					CharacterCompiledData.Position = GlyphPos + BlockOffset;


					if (CharacterCompiledData.StartTimeStamp > CompiledText.LastStartTimeStamp)
					{
						CompiledText.LastStartTimeStamp = CharacterCompiledData.StartTimeStamp;
					}

					int32 NextAdvance = Glyphs[i].XAdvance;
					CharacterCompiledData.Bounds = FVector2D(NextAdvance, CurrentSequenceHeight);
					CharacterCompiledData.ParameterLookup = Run.GetLookup();

					FInterjectionOutput InterjectionOutput;
					InterjectionVisitor.VisitInterjectionsAt(Glyph.SourceIndex,
						[&InterjectionOutput, &CharacterCompiledData](const TSharedRef<FExText_Interjection> Interjection)
						{
							Interjection->ProcessModifiers(InterjectionOutput);
							CharacterCompiledData.Interjections.Add(Interjection);
						}
					);

					// Set appear time
					float RevealRate = Run.GetLookup()->GetValue< UExTextValue_RevealRate >();
					if (RevealRate > 0.f)
					{
						Chronometer += 1.f / RevealRate;
					}
					if (InterjectionOutput.PauseDuration > 0.f)
					{
						Chronometer += InterjectionOutput.PauseDuration;
					}

					CharacterCompiledData.StartTimeStamp = Chronometer;
				}
			}
		}
	}

	void EvaluateEndTimesForDirection(TArray<TSharedRef<FExpressiveTextRun>>& Runs, EExText_ClearDirection ClearDirection, float& Choronometer)
	{
		int32 RunsNum = Runs.Num();

		const auto LoopCheck = ClearDirection == EExText_ClearDirection::Forwards
			? TFunction<bool(int32)>([RunsNum](int32 index) { return index < RunsNum; })
			: TFunction<bool(int32)>([](int32 index) { return index >= 0; });

		const auto Advance = ClearDirection == EExText_ClearDirection::Forwards
			? TFunction<void(int32&)>([](int32& index) { index++; })
			: TFunction<void(int32&)>([](int32& index) { index--; });


		int32 i = ClearDirection == EExText_ClearDirection::Forwards ? 0 : RunsNum - 1;
		for (; LoopCheck(i); Advance(i))
		{
			auto& Run = Runs[i];

			float ClearTimer = Run->GetLookup()->GetValue< UExTextValue_ClearTimer >();
			float ClearRate = Run->GetLookup()->GetValue< UExTextValue_ClearRate >();
			EExText_ClearDirection CharClearDirection = Run->GetLookup()->GetValue<UExTextValue_ClearDirection>();

			if (CharClearDirection == ClearDirection)
			{
				if (ClearTimer >= 0.f)
				{
					Run->SetClearStartTime(Choronometer + ClearTimer);
					Choronometer += Run->CalculateDurationToFullyClear();
				}
			}

		}
	}

	int32 FindFromIndex(const FString& String, const TCHAR& Delimiter, int32 StartIndex, int32 EndIndex = -1 )
	{
		check(StartIndex >= 0 && StartIndex <= String.Len());

		int32 ForEnd = EndIndex == -1 ? String.Len() : EndIndex + 1;

		for (int32 i = StartIndex; i < ForEnd; i++)
		{
			if (String[i] == Delimiter)
			{
				return i;
			}
		}

		return INDEX_NONE;
	}



	TFuture<void> GenerateExtraction(int32 LineIndex)
	{
		check(LineIndex >= 0 && LineIndex < LinesRefs.Num());

		FString& Line = LinesRefs[LineIndex].Get();

		TArray<FExTextTokenPosition> Tokens;
		ParseTokens(Line, Tokens);

		const auto& Context = ExpressiveText.GetContext();

		TSharedRef<FTreeExtraction> Root = MakeShareable(new FTreeExtraction);
		TSharedRef<FExpressiveTextExtraction> Extraction = MakeShareable(new FExpressiveTextExtraction);
		Extraction->ExtractionTree = Root;

		Root->OriginalRange.BeginIndex = 0;
		Root->OriginalRange.EndIndex = Line.Len();
		Root->Range.BeginIndex = 0;
		Root->ParameterLookup = CreateDefaultParameterLookup(ExpressiveText.GetDefaultStyle(), ExpressiveText.GetDefaultFontSize());

		TArray< TSharedRef<FTreeExtraction> > ExtractionsStack;

		int32 CurrentIndex = 0;
		int32 RemovedCount = 0;

		const auto GetParent = [&Root, &ExtractionsStack]()
		{
			if (ExtractionsStack.Num() > 0)
			{
				return ExtractionsStack.Top();
			}
			return Root;
		};

		const auto AddExtractionToParent = [&GetParent](TSharedRef<FTreeExtraction> Child)
		{
			GetParent()->AddChild(Child);
		};

		const auto CharacterRealPosition = [&](const FExTextTokenPosition& Character) {
			return Character.Position - RemovedCount;
		};

		const auto CharacterRealPositionFromIndex = [&](int32 CharacterIndex) {
			return CharacterRealPosition(Tokens[CharacterIndex]);
		};

		const auto CharacterOriginalPositionFromIndex = [&](int32 CharacterIndex) {
			return Tokens[CharacterIndex].Position;
		};

		const auto IsCurrent = [&](TCHAR Character) -> bool {
			if (CurrentIndex < Tokens.Num())
			{
				int32 Index = CharacterRealPositionFromIndex(CurrentIndex);
				if (Index >= 0)
				{
					return Line[Index] == Character;
				}
			}
			return false;
		};

		const auto Next = [&CurrentIndex]() { CurrentIndex++; };

		const auto Remove = [&](int32 Start, int32 Count) {
			Line.RemoveAt(Start, Count, false);
			RemovedCount += Count;
		};

		TArray<TFuture<void>> AllLookupFuturesComplete;

		static const int32 UnicodeTokenLength = 7;
		while (CurrentIndex < Tokens.Num())
		{
			if (IsCurrent('\\'))
			{
				Remove(CharacterRealPositionFromIndex(CurrentIndex), 1);
				Next();
			}
			else if (IsCurrent('/'))
			{
				int32 SlashIndex = CharacterRealPositionFromIndex(CurrentIndex);
				int32 ExpectedUnicodeCodeEnd = SlashIndex + UnicodeTokenLength;

				if (ExpectedUnicodeCodeEnd <= Line.Len())
				{
					FString PossibleUnicode = Line.Mid(SlashIndex, UnicodeTokenLength);

					if (PossibleUnicode.RemoveFromStart("/U+"))
					{
						uint32 Unicode = FParse::HexNumber(*PossibleUnicode);

						// remove the unicode tag except the '/' which we are going to replace with the unicode itself to avoid another allocation
						Remove(SlashIndex + 1, UnicodeTokenLength - 1);
						Line[SlashIndex] = (TCHAR)Unicode;
					}
				}
				Next();
			}
			else if (IsCurrent('['))
			{
				Next();

				if (IsCurrent(']'))
				{
					Next();

					if (IsCurrent('('))
					{
						// check '(' starts right after ']'
						if (CharacterRealPositionFromIndex(CurrentIndex) == CharacterRealPositionFromIndex(CurrentIndex - 1) + 1)
						{
							TSharedRef< FTreeExtraction > NewExtraction = MakeShareable(new FTreeExtraction);
							auto& NewExtractionRaw = NewExtraction.Get();

							int32 TagsStart = CharacterRealPositionFromIndex(CurrentIndex - 2);
							int32 TagsEnd = CharacterRealPositionFromIndex(CurrentIndex - 1);

							int32 TagsCharCount = TagsEnd - TagsStart;
							FString Tags = Line.Mid(TagsStart + 1, TagsCharCount - 1);
							Tags.Replace(TEXT(" "), TEXT("")).ParseIntoArray(NewExtractionRaw.TagsList, TEXT(","));

							NewExtractionRaw.Parent = ExtractionsStack.Num() > 0 ? ExtractionsStack.Top() : Root;

							if (!DryRun)
							{
								AllLookupFuturesComplete.Add(ProcessTags(NewExtraction));
							}

							Remove(TagsStart, TagsCharCount + 1);

							//must only calculate content start after removing trailing tags markup
							int32 ContentStartIndex = CharacterRealPositionFromIndex(CurrentIndex);
							NewExtractionRaw.Range.BeginIndex = ContentStartIndex;
							NewExtractionRaw.OriginalRange.BeginIndex = CharacterOriginalPositionFromIndex(CurrentIndex - 2);
							NewExtractionRaw.TagsRange.BeginIndex = CharacterOriginalPositionFromIndex(CurrentIndex - 2);
							NewExtractionRaw.TagsRange.EndIndex = CharacterOriginalPositionFromIndex(CurrentIndex - 1);
							NewExtractionRaw.ContentRange.BeginIndex = CharacterOriginalPositionFromIndex(CurrentIndex);
							Remove(ContentStartIndex, 1);

							ExtractionsStack.Push(NewExtraction);
							Next();
						}
					}
				}
			}
			else if (IsCurrent(')'))
			{
				if (ExtractionsStack.Num() > 0)
				{
					TSharedRef< FTreeExtraction > ExtractionBeingClosed = ExtractionsStack.Pop();
					ExtractionBeingClosed->OriginalRange.EndIndex = CharacterOriginalPositionFromIndex(CurrentIndex);
					ExtractionBeingClosed->ContentRange.EndIndex = CharacterOriginalPositionFromIndex(CurrentIndex);
					int32 ContentEndIndex = CharacterRealPositionFromIndex(CurrentIndex);
					ExtractionBeingClosed->Range.EndIndex = ContentEndIndex;
					Remove(ContentEndIndex, 1);

					// Only when content is present
					if (ContentEndIndex > ExtractionBeingClosed->Range.BeginIndex)
					{
						ExtractionBeingClosed->Content = Line.Mid(ExtractionBeingClosed->Range.BeginIndex, ExtractionBeingClosed->Range.Len() - 1);
						AddExtractionToParent(ExtractionBeingClosed);
					}
				}

				Next();
			}
			else if (IsCurrent('<'))
			{
				Next();
				if (IsCurrent('>'))
				{
					int32 InterjectionStart = CharacterRealPositionFromIndex(CurrentIndex - 1);
					int32 InterjectionEnd = CharacterRealPositionFromIndex(CurrentIndex);

					int32 InterjectionCharCount = InterjectionEnd - InterjectionStart;
					FString InterjectionString = Line.Mid(InterjectionStart + 1, InterjectionCharCount - 1);

					TArray<FString> SplitInterjection;
					InterjectionString.ParseIntoArray(SplitInterjection, TEXT(":"));

					if (SplitInterjection.Num() == 2)
					{
						// Convert to FName for faster comparison
						const FName InterjectionType = FName(*SplitInterjection[0]);
						FString& InterjectionParams = SplitInterjection[1];
						InterjectionParams.TrimStartAndEndInline();

						FExText_ParsedInterjection ParsedInterjection;
						ParsedInterjection.Index = InterjectionStart;

						if (InterjectionType == InterjectionTypes::Action || InterjectionType == InterjectionTypes::A)
						{
							ParsedInterjection.Interjection = MakeShareable(new FExText_ActionInterjection(InterjectionParams, Context));
						}
						else if (InterjectionType == InterjectionTypes::Pause || InterjectionType == InterjectionTypes::P)
						{
							ParsedInterjection.Interjection = MakeShareable(new FExText_PauseInterjection(InterjectionParams));
						}

						if (ParsedInterjection.Interjection.IsValid())
						{
							Extraction->Interjections.Add(ParsedInterjection);
						}
					}

					Remove(InterjectionStart, InterjectionCharCount + 1);
					Next();
				}
			}
			else
			{
				Next();
			}
		}

		Line.Shrink();
		Root->Range.EndIndex = Line.Len();

		LinesExtractions.Add(Extraction);


		if (DryRun)
		{
			return Guganana::Async::MakeFullfiledFuture();
		}

		return Guganana::Async::WhenAllFutures(AllLookupFuturesComplete);
	}


	void ParseTokens(const FString& String, TArray<FExTextTokenPosition>& OutTokens)
	{
		static TArray<TCHAR> ReservedCharacters = { '[', ']', '(', ')', '<', '>', '/' };

		for (int i = 0; i < String.Len(); i++)
		{
			if (ReservedCharacters.Contains(String[i]))
			{
				if (i > 0)
				{
					if (String[i - 1] == '\\') // Check if character is being escaped
					{
						FExTextTokenPosition Char;
						Char.Position = i - 1;
						OutTokens.Emplace(Char);
						continue;
					}
				}

				FExTextTokenPosition Token;
				Token.Position = i;
				OutTokens.Emplace(Token);
			}
		}
	}




	TFuture<TSharedPtr<FExpressiveTextParameterLookup>> MakeLookupFromTag(const FString& Tag)
	{
		FName TagName(*Tag);

		auto* ExpressiveTextSubsystem = GEngine->GetEngineSubsystem<UExpressiveTextSubsystem>();
		check(ExpressiveTextSubsystem);

		const auto MakeFullfiledExtractorFuture = [&TagName](TSharedPtr<IExpressiveTextParameterExtractor> Extractor) {
			TSharedPtr<FExpressiveTextParameterLookup> Lookup = MakeShareable(new FExpressiveTextParameterLookup(TagName, Extractor));
			return Guganana::Async::MakeFullfiledFuture<TSharedPtr<FExpressiveTextParameterLookup>>(Lookup);
		};

		const auto MakeFullfiledEmptyFuture = []()
		{
			return Guganana::Async::MakeFullfiledFuture<TSharedPtr<FExpressiveTextParameterLookup>>(nullptr);
		};

		TOptional<TFuture<UExpressiveTextStyle*>> FoundStyle = ExpressiveTextSubsystem->FetchResource<ExpressiveTextResource::Style>(TagName);
		if (FoundStyle.IsSet())
		{
			return FoundStyle.GetValue().Next(
				[Handle = AsShared()](UExpressiveTextStyle* Style)
				{
					Handle->TempCompiledText.HarvestedResources.Add(Style);
					return Style->GetParameterLookup();
				}
			);
		}

		const TCHAR FirstToken = Tag[0];

		if (FirstToken == '#')
		{
			TSharedPtr<IExpressiveTextParameterExtractor> InlineExtractor = FExpressiveTextInlineParameterExtractor::Create<UExTextValue_FontColor>(FLinearColor(FColor::FromHex(Tag)));
			return MakeFullfiledExtractorFuture(InlineExtractor);
		}

		if (FirstToken == '&')
		{
			FString MutatedTag = Tag;
			MutatedTag.RemoveAt(0, 1);

			const FName FontName(*MutatedTag);
			TOptional<TFuture<UExpressiveTextFont*>> Font = ExpressiveTextSubsystem->FetchFont(FontName);
			if (Font.IsSet())
			{
				return Font.GetValue().Next(
					[FontName, Handle=AsShared()](UExpressiveTextFont* Font)
					{
						Handle->TempCompiledText.HarvestedResources.Add(Font);
						TSharedPtr<IExpressiveTextParameterExtractor> InlineExtractor = FExpressiveTextInlineParameterExtractor::Create<UExTextValue_Font>(Font);
						TSharedPtr<FExpressiveTextParameterLookup> Lookup = MakeShareable(new FExpressiveTextParameterLookup(FontName, InlineExtractor));
						return Lookup;
					}
				);				
			}
			return MakeFullfiledEmptyFuture();
		}

		if (FirstToken == '@')
		{
			FString MutatedTag = Tag;
			MutatedTag.RemoveAt(0, 1);

			const FName AnimationName(*MutatedTag);
			TOptional<TFuture<UExpressiveTextAnimation*>> Animation = ExpressiveTextSubsystem->FetchResource<ExpressiveTextResource::Animation>(AnimationName);
			if (Animation.IsSet())
			{
				return Animation.GetValue().Next(
					[AnimationName, Handle=AsShared()](UExpressiveTextAnimation* Animation)
					{
						Handle->TempCompiledText.HarvestedResources.Add(Animation);
						TSharedPtr<IExpressiveTextParameterExtractor> InlineExtractor = FExpressiveTextInlineParameterExtractor::Create<UExTextValue_RevealAnimation>(FExText_GlyphAnimation(Animation));
						TSharedPtr<FExpressiveTextParameterLookup> Lookup = MakeShareable(new FExpressiveTextParameterLookup(AnimationName, InlineExtractor));
						return Lookup;
					}
				);

			}

			return MakeFullfiledEmptyFuture();
		}

		if (FirstToken == '*')
		{
			FString MutatedTag = Tag;
			MutatedTag.RemoveAt(0, 1);

			TSharedPtr<IExpressiveTextParameterExtractor> InlineExtractor = FExpressiveTextInlineParameterExtractor::Create<UExTextValue_Typeface>(FName(*MutatedTag));
			return MakeFullfiledExtractorFuture(InlineExtractor);
		}

		if (Tag.EndsWith(TEXT("pt")))
		{
			FString MutatedTag = Tag;
			MutatedTag.RemoveFromEnd(TEXT("pt"));
			int32 ParsedFontSize = FCString::Atoi(*MutatedTag);
			if (ParsedFontSize > 0)
			{
				TSharedPtr<IExpressiveTextParameterExtractor> InlineExtractor = FExpressiveTextInlineParameterExtractor::Create<UExTextValue_FontSize>(ParsedFontSize);
				return MakeFullfiledExtractorFuture(InlineExtractor);
			}

			return MakeFullfiledEmptyFuture();
		}

		if (Tag.EndsWith(TEXT("rr")))
		{
			FString MutatedTag = Tag;
			MutatedTag.RemoveFromEnd(TEXT("rr"));
			int32 ParsedFontSize = FCString::Atoi(*MutatedTag);
			if (ParsedFontSize > 0)
			{
				TSharedPtr<IExpressiveTextParameterExtractor> InlineExtractor = FExpressiveTextInlineParameterExtractor::Create<UExTextValue_RevealRate>(ParsedFontSize);
				return MakeFullfiledExtractorFuture(InlineExtractor);
			}

			return MakeFullfiledEmptyFuture();
		}

		if (const FColor* TagAsColor = ExpressiveTextSubsystem->FetchColorForTag(FName(*Tag)))
		{
			TSharedPtr<IExpressiveTextParameterExtractor> InlineExtractor = FExpressiveTextInlineParameterExtractor::Create<UExTextValue_FontColor>(FLinearColor(*TagAsColor));
			return MakeFullfiledExtractorFuture(InlineExtractor);
		}

		Unlog::Warnf(TEXT("Unable to find match for tag: %s"), *Tag);

		return MakeFullfiledEmptyFuture();
	}

	TFuture<void> ProcessTags(TSharedRef< FTreeExtraction > Extraction)
	{
		TArray<TFuture<TSharedPtr<FExpressiveTextParameterLookup>>> ParameterLookupFutures;

		for (const auto& Tag : Extraction->TagsList)
		{
			if (Tag.Len() == 0) { continue; }
			ParameterLookupFutures.Add(MakeLookupFromTag(Tag));
		}

		TSharedRef<TPromise<void>> WhenProcessedPromise = MakeShareable(new TPromise<void>());

		Guganana::Async::WhenAllFutures(ParameterLookupFutures).Next(
			[SharedCompiler = AsShared(), Extraction, WhenProcessedPromise](TArray<TSharedPtr<FExpressiveTextParameterLookup>> LookupList)
			{
				SharedCompiler->GenerateParameterLookupChain(LookupList, Extraction);
				WhenProcessedPromise->SetValue();
			}
		);

		return WhenProcessedPromise->GetFuture();
	}
	
	void GenerateParameterLookupChain(TArray<TSharedPtr<FExpressiveTextParameterLookup>> LookupList, TSharedRef< FTreeExtraction > Extraction)
	{
		TSharedPtr<FExpressiveTextParameterLookup> Root = MakeShareable(new FExpressiveTextParameterLookup(TEXT("Root"), nullptr));
		TSharedPtr<FExpressiveTextParameterLookup> Current = Root;
		

		// Reverse order to establish priority
		for (int i = LookupList.Num() - 1; i >= 0; i--)
		{
			TSharedPtr<FExpressiveTextParameterLookup>& Lookup = LookupList[i];
			if (Lookup)
			{
				Current->SetNext(Lookup);
				Current = Lookup;
			}
		}		

		if (Extraction->Parent && Extraction->Parent->ParameterLookup)
		{
			Current->SetNext(Extraction->Parent->ParameterLookup);
		}
		else
		{
			Current->SetNext(CreateDefaultParameterLookup(nullptr));
		}

		Extraction->ParameterLookup = Root;
		Current = nullptr;
	}


};