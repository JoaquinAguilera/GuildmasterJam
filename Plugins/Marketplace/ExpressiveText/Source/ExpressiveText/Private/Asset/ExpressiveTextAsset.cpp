// Copyright 2022 Guganana. All Rights Reserved.
#include "Asset/ExpressiveTextAsset.h"
#include "ExpressiveTextProcessor.h"


void UExpressiveTextAsset::OnPreSave()
{
	FExpressiveText Text;
	Text.SetWorldContext(this);
	Text.SetFields(Fields);

	UExpressiveTextProcessor::CompileText(Text).Next(
		[this, FieldsHandle = Fields.GetAliveHandle()](const FCompiledExpressiveText& InCompiledText)
		{
			if (FieldsHandle.IsValid())
			{
				Fields.ReferencedResources = InCompiledText.HarvestedResources;
			}
		}
	);

}