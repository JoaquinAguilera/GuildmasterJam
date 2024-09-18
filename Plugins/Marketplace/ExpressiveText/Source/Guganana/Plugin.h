// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <PluginInformation.h>
#include <Interfaces/IPluginManager.h>

namespace Guganana
{
    using PluginInformation = ExportedPluginInformation;

    class Plugin
    {
    public:
        static FString BaseDir()
        {
            return IPluginManager::Get().FindPlugin( PluginInformation::PluginName() )->GetBaseDir();
        }
    };
}
