// Copyright 2022 Guganana. All Rights Reserved.
#include "Subsystems/ExpressiveTextSubsystem.h"

#include "ExpressiveText/Public/ExpressiveTextSettings.h"
#include "ExpressiveText/Public/Styles/ExpressiveTextStyleBase.h"

#include <Engine/AssetManager.h>


TOptional<TFuture<UExpressiveTextFont*>>  UExpressiveTextSubsystem::FetchFont(const FName& Tag, TOptional<FString> OnMissingFontMessage) const
{
	auto FetchResult = FetchResource<ExpressiveTextResource::Font>(Tag);

#if WITH_EDITOR
	if (!FetchResult.IsSet())
	{
		ExpressiveTextMissingFont.Broadcast(Tag, OnMissingFontMessage);
	}
#endif

	return FetchResult;
}

const FColor* UExpressiveTextSubsystem::FetchColorForTag( const FName& Tag ) const
{
	return ColorMap.Find(Tag);
}

void UExpressiveTextSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );
	PopulateColorMap();
}

void UExpressiveTextSubsystem::PopulateColorMap()
{
	check( ColorMap.Num() == 0 );
	
	ColorMap.Emplace( FName(TEXT("aliceblue")), FColor( 240, 248, 255 ) );
	ColorMap.Emplace( FName(TEXT("antiquewhite")), FColor( 250, 235, 215 ) );
	ColorMap.Emplace( FName(TEXT("aqua")), FColor( 0, 255, 255 ) );
	ColorMap.Emplace( FName(TEXT("aquamarine")), FColor( 127, 255, 212 ) );
	ColorMap.Emplace( FName(TEXT("azure")), FColor( 240, 255, 255 ) );
	ColorMap.Emplace( FName(TEXT("beige")), FColor( 245, 245, 220 ) );
	ColorMap.Emplace( FName(TEXT("bisque")), FColor( 255, 228, 196 ) );
	ColorMap.Emplace( FName(TEXT("black")), FColor( 0, 0, 0 ) );
	ColorMap.Emplace( FName(TEXT("blanchedalmond")), FColor( 255, 235, 205 ) );
	ColorMap.Emplace( FName(TEXT("blue")), FColor( 0, 0, 255 ) );
	ColorMap.Emplace( FName(TEXT("blueviolet")), FColor( 138, 43, 226 ) );
	ColorMap.Emplace( FName(TEXT("brown")), FColor( 165, 42, 42 ) );
	ColorMap.Emplace( FName(TEXT("burlywood")), FColor( 222, 184, 135 ) );
	ColorMap.Emplace( FName(TEXT("cadetblue")), FColor( 95, 158, 160 ) );
	ColorMap.Emplace( FName(TEXT("chartreuse")), FColor( 127, 255, 0 ) );
	ColorMap.Emplace( FName(TEXT("chocolate")), FColor( 210, 105, 30 ) );
	ColorMap.Emplace( FName(TEXT("coral")), FColor( 255, 127, 80 ) );
	ColorMap.Emplace( FName(TEXT("cornflowerblue")), FColor( 100, 149, 237 ) );
	ColorMap.Emplace( FName(TEXT("cornsilk")), FColor( 255, 248, 220 ) );
	ColorMap.Emplace( FName(TEXT("crimson")), FColor( 220, 20, 60 ) );
	ColorMap.Emplace( FName(TEXT("cyan")), FColor( 0, 255, 255 ) );
	ColorMap.Emplace( FName(TEXT("darkblue")), FColor( 0, 0, 139 ) );
	ColorMap.Emplace( FName(TEXT("darkcyan")), FColor( 0, 139, 139 ) );
	ColorMap.Emplace( FName(TEXT("darkgoldenrod")), FColor( 184, 134, 11 ) );
	ColorMap.Emplace( FName(TEXT("darkgray")), FColor( 169, 169, 169 ) );
	ColorMap.Emplace( FName(TEXT("darkgreen")), FColor( 0, 100, 0 ) );
	ColorMap.Emplace( FName(TEXT("darkgrey")), FColor( 169, 169, 169 ) );
	ColorMap.Emplace( FName(TEXT("darkkhaki")), FColor( 189, 183, 107 ) );
	ColorMap.Emplace( FName(TEXT("darkmagenta")), FColor( 139, 0, 139 ) );
	ColorMap.Emplace( FName(TEXT("darkolivegreen")), FColor( 85, 107, 47 ) );
	ColorMap.Emplace( FName(TEXT("darkorange")), FColor( 255, 140, 0 ) );
	ColorMap.Emplace( FName(TEXT("darkorchid")), FColor( 153, 50, 204 ) );
	ColorMap.Emplace( FName(TEXT("darkred")), FColor( 139, 0, 0 ) );
	ColorMap.Emplace( FName(TEXT("darksalmon")), FColor( 233, 150, 122 ) );
	ColorMap.Emplace( FName(TEXT("darkseagreen")), FColor( 143, 188, 143 ) );
	ColorMap.Emplace( FName(TEXT("darkslateblue")), FColor( 72, 61, 139 ) );
	ColorMap.Emplace( FName(TEXT("darkslategray")), FColor( 47, 79, 79 ) );
	ColorMap.Emplace( FName(TEXT("darkslategrey")), FColor( 47, 79, 79 ) );
	ColorMap.Emplace( FName(TEXT("darkturquoise")), FColor( 0, 206, 209 ) );
	ColorMap.Emplace( FName(TEXT("darkviolet")), FColor( 148, 0, 211 ) );
	ColorMap.Emplace( FName(TEXT("deeppink")), FColor( 255, 20, 147 ) );
	ColorMap.Emplace( FName(TEXT("deepskyblue")), FColor( 0, 191, 255 ) );
	ColorMap.Emplace( FName(TEXT("dimgray")), FColor( 105, 105, 105 ) );
	ColorMap.Emplace( FName(TEXT("dimgrey")), FColor( 105, 105, 105 ) );
	ColorMap.Emplace( FName(TEXT("dodgerblue")), FColor( 30, 144, 255 ) );
	ColorMap.Emplace( FName(TEXT("firebrick")), FColor( 178, 34, 34 ) );
	ColorMap.Emplace( FName(TEXT("floralwhite")), FColor( 255, 250, 240 ) );
	ColorMap.Emplace( FName(TEXT("forestgreen")), FColor( 34, 139, 34 ) );
	ColorMap.Emplace( FName(TEXT("fuchsia")), FColor( 255, 0, 255 ) );
	ColorMap.Emplace( FName(TEXT("gainsboro")), FColor( 220, 220, 220 ) );
	ColorMap.Emplace( FName(TEXT("ghostwhite")), FColor( 248, 248, 255 ) );
	ColorMap.Emplace( FName(TEXT("goldenrod")), FColor( 218, 165, 32 ) );
	ColorMap.Emplace( FName(TEXT("gold")), FColor( 255, 215, 0 ) );
	ColorMap.Emplace( FName(TEXT("gray")), FColor( 128, 128, 128 ) );
	ColorMap.Emplace( FName(TEXT("green")), FColor( 0, 128, 0 ) );
	ColorMap.Emplace( FName(TEXT("greenyellow")), FColor( 173, 255, 47 ) );
	ColorMap.Emplace( FName(TEXT("grey")), FColor( 128, 128, 128 ) );
	ColorMap.Emplace( FName(TEXT("honeydew")), FColor( 240, 255, 240 ) );
	ColorMap.Emplace( FName(TEXT("hotpink")), FColor( 255, 105, 180 ) );
	ColorMap.Emplace( FName(TEXT("indianred")), FColor( 205, 92, 92 ) );
	ColorMap.Emplace( FName(TEXT("indigo")), FColor( 75, 0, 130 ) );
	ColorMap.Emplace( FName(TEXT("ivory")), FColor( 255, 255, 240 ) );
	ColorMap.Emplace( FName(TEXT("khaki")), FColor( 240, 230, 140 ) );
	ColorMap.Emplace( FName(TEXT("lavenderblush")), FColor( 255, 240, 245 ) );
	ColorMap.Emplace( FName(TEXT("lavender")), FColor( 230, 230, 250 ) );
	ColorMap.Emplace( FName(TEXT("lawngreen")), FColor( 124, 252, 0 ) );
	ColorMap.Emplace( FName(TEXT("lemonchiffon")), FColor( 255, 250, 205 ) );
	ColorMap.Emplace( FName(TEXT("lightblue")), FColor( 173, 216, 230 ) );
	ColorMap.Emplace( FName(TEXT("lightcoral")), FColor( 240, 128, 128 ) );
	ColorMap.Emplace( FName(TEXT("lightcyan")), FColor( 224, 255, 255 ) );
	ColorMap.Emplace( FName(TEXT("lightgoldenrodyellow")), FColor( 250, 250, 210 ) );
	ColorMap.Emplace( FName(TEXT("lightgray")), FColor( 211, 211, 211 ) );
	ColorMap.Emplace( FName(TEXT("lightgreen")), FColor( 144, 238, 144 ) );
	ColorMap.Emplace( FName(TEXT("lightgrey")), FColor( 211, 211, 211 ) );
	ColorMap.Emplace( FName(TEXT("lightpink")), FColor( 255, 182, 193 ) );
	ColorMap.Emplace( FName(TEXT("lightsalmon")), FColor( 255, 160, 122 ) );
	ColorMap.Emplace( FName(TEXT("lightseagreen")), FColor( 32, 178, 170 ) );
	ColorMap.Emplace( FName(TEXT("lightskyblue")), FColor( 135, 206, 250 ) );
	ColorMap.Emplace( FName(TEXT("lightslategray")), FColor( 119, 136, 153 ) );
	ColorMap.Emplace( FName(TEXT("lightslategrey")), FColor( 119, 136, 153 ) );
	ColorMap.Emplace( FName(TEXT("lightsteelblue")), FColor( 176, 196, 222 ) );
	ColorMap.Emplace( FName(TEXT("lightyellow")), FColor( 255, 255, 224 ) );
	ColorMap.Emplace( FName(TEXT("lime")), FColor( 0, 255, 0 ) );
	ColorMap.Emplace( FName(TEXT("limegreen")), FColor( 50, 205, 50 ) );
	ColorMap.Emplace( FName(TEXT("linen")), FColor( 250, 240, 230 ) );
	ColorMap.Emplace( FName(TEXT("magenta")), FColor( 255, 0, 255 ) );
	ColorMap.Emplace( FName(TEXT("maroon")), FColor( 128, 0, 0 ) );
	ColorMap.Emplace( FName(TEXT("mediumaquamarine")), FColor( 102, 205, 170 ) );
	ColorMap.Emplace( FName(TEXT("mediumblue")), FColor( 0, 0, 205 ) );
	ColorMap.Emplace( FName(TEXT("mediumorchid")), FColor( 186, 85, 211 ) );
	ColorMap.Emplace( FName(TEXT("mediumpurple")), FColor( 147, 112, 219 ) );
	ColorMap.Emplace( FName(TEXT("mediumseagreen")), FColor( 60, 179, 113 ) );
	ColorMap.Emplace( FName(TEXT("mediumslateblue")), FColor( 123, 104, 238 ) );
	ColorMap.Emplace( FName(TEXT("mediumspringgreen")), FColor( 0, 250, 154 ) );
	ColorMap.Emplace( FName(TEXT("mediumturquoise")), FColor( 72, 209, 204 ) );
	ColorMap.Emplace( FName(TEXT("mediumvioletred")), FColor( 199, 21, 133 ) );
	ColorMap.Emplace( FName(TEXT("midnightblue")), FColor( 25, 25, 112 ) );
	ColorMap.Emplace( FName(TEXT("mintcream")), FColor( 245, 255, 250 ) );
	ColorMap.Emplace( FName(TEXT("mistyrose")), FColor( 255, 228, 225 ) );
	ColorMap.Emplace( FName(TEXT("moccasin")), FColor( 255, 228, 181 ) );
	ColorMap.Emplace( FName(TEXT("navajowhite")), FColor( 255, 222, 173 ) );
	ColorMap.Emplace( FName(TEXT("navy")), FColor( 0, 0, 128 ) );
	ColorMap.Emplace( FName(TEXT("oldlace")), FColor( 253, 245, 230 ) );
	ColorMap.Emplace( FName(TEXT("olive")), FColor( 128, 128, 0 ) );
	ColorMap.Emplace( FName(TEXT("olivedrab")), FColor( 107, 142, 35 ) );
	ColorMap.Emplace( FName(TEXT("orange")), FColor( 255, 165, 0 ) );
	ColorMap.Emplace( FName(TEXT("orangered")), FColor( 255, 69, 0 ) );
	ColorMap.Emplace( FName(TEXT("orchid")), FColor( 218, 112, 214 ) );
	ColorMap.Emplace( FName(TEXT("palegoldenrod")), FColor( 238, 232, 170 ) );
	ColorMap.Emplace( FName(TEXT("palegreen")), FColor( 152, 251, 152 ) );
	ColorMap.Emplace( FName(TEXT("paleturquoise")), FColor( 175, 238, 238 ) );
	ColorMap.Emplace( FName(TEXT("palevioletred")), FColor( 219, 112, 147 ) );
	ColorMap.Emplace( FName(TEXT("papayawhip")), FColor( 255, 239, 213 ) );
	ColorMap.Emplace( FName(TEXT("peachpuff")), FColor( 255, 218, 185 ) );
	ColorMap.Emplace( FName(TEXT("peru")), FColor( 205, 133, 63 ) );
	ColorMap.Emplace( FName(TEXT("pink")), FColor( 255, 192, 203 ) );
	ColorMap.Emplace( FName(TEXT("plum")), FColor( 221, 160, 221 ) );
	ColorMap.Emplace( FName(TEXT("powderblue")), FColor( 176, 224, 230 ) );
	ColorMap.Emplace( FName(TEXT("purple")), FColor( 128, 0, 128 ) );
	ColorMap.Emplace( FName(TEXT("rebeccapurple")), FColor( 102, 51, 153 ) );
	ColorMap.Emplace( FName(TEXT("red")), FColor( 255, 0, 0 ) );
	ColorMap.Emplace( FName(TEXT("rosybrown")), FColor( 188, 143, 143 ) );
	ColorMap.Emplace( FName(TEXT("royalblue")), FColor( 65, 105, 225 ) );
	ColorMap.Emplace( FName(TEXT("saddlebrown")), FColor( 139, 69, 19 ) );
	ColorMap.Emplace( FName(TEXT("salmon")), FColor( 250, 128, 114 ) );
	ColorMap.Emplace( FName(TEXT("sandybrown")), FColor( 244, 164, 96 ) );
	ColorMap.Emplace( FName(TEXT("seagreen")), FColor( 46, 139, 87 ) );
	ColorMap.Emplace( FName(TEXT("seashell")), FColor( 255, 245, 238 ) );
	ColorMap.Emplace( FName(TEXT("sienna")), FColor( 160, 82, 45 ) );
	ColorMap.Emplace( FName(TEXT("silver")), FColor( 192, 192, 192 ) );
	ColorMap.Emplace( FName(TEXT("skyblue")), FColor( 135, 206, 235 ) );
	ColorMap.Emplace( FName(TEXT("slateblue")), FColor( 106, 90, 205 ) );
	ColorMap.Emplace( FName(TEXT("slategray")), FColor( 112, 128, 144 ) );
	ColorMap.Emplace( FName(TEXT("slategrey")), FColor( 112, 128, 144 ) );
	ColorMap.Emplace( FName(TEXT("snow")), FColor( 255, 250, 250 ) );
	ColorMap.Emplace( FName(TEXT("springgreen")), FColor( 0, 255, 127 ) );
	ColorMap.Emplace( FName(TEXT("steelblue")), FColor( 70, 130, 180 ) );
	ColorMap.Emplace( FName(TEXT("tan")), FColor( 210, 180, 140 ) );
	ColorMap.Emplace( FName(TEXT("teal")), FColor( 0, 128, 128 ) );
	ColorMap.Emplace( FName(TEXT("thistle")), FColor( 216, 191, 216 ) );
	ColorMap.Emplace( FName(TEXT("tomato")), FColor( 255, 99, 71 ) );
	ColorMap.Emplace( FName(TEXT("turquoise")), FColor( 64, 224, 208 ) );
	ColorMap.Emplace( FName(TEXT("violet")), FColor( 238, 130, 238 ) );
	ColorMap.Emplace( FName(TEXT("wheat")), FColor( 245, 222, 179 ) );
	ColorMap.Emplace( FName(TEXT("white")), FColor( 255, 255, 255 ) );
	ColorMap.Emplace( FName(TEXT("whitesmoke")), FColor( 245, 245, 245 ) );
	ColorMap.Emplace( FName(TEXT("yellow")), FColor( 255, 255, 0 ) );
	ColorMap.Emplace( FName(TEXT("yellowgreen")), FColor( 154, 205, 50 ) );
}