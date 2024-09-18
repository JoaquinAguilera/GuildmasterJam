// Copyright 2023 Guganana. All Rights Reserved.

#pragma once

#include <Unlog/UnlogImplementation.h>
#include <Unlog/Extras/Testing.h>
#include <Guganana/RemoteLogSink.h>

UNLOG_CATEGORY(LogExpressiveText);

struct RemoteLogger : public TUnlogRemoteSink<RemoteLogger>
{
	static bool ShouldLog()
	{
		static TAutoConsoleVariable<float> CVarSendExpressiveTextLogsToDevelopers(
			TEXT("ExpressiveText.SendLogsToDevelopers"), 
			0, 
			TEXT("Sends logs related to Expressive Text to the developers.")
			TEXT("This only includes logs defined by Expressive Text and nothing else.")
		);

		return CVarSendExpressiveTextLogsToDevelopers.GetValueOnGameThread() > 0;
	}
};

using Unlog =	TUnlog<>::WithDefaultCategory< LogExpressiveText >
						::WithTargets< Target::UELog, RemoteLogger >;