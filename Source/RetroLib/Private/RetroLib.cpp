﻿// Copyright Epic Games, Inc. All Rights Reserved.

#ifdef __UNREAL__
#include "RetroLib.h"

#define LOCTEXT_NAMESPACE "FRetroLibUEModule"

void FRetroLibModule::StartupModule() {
}

void FRetroLibModule::ShutdownModule() {
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRetroLibModule, RetroLibUE)
#endif