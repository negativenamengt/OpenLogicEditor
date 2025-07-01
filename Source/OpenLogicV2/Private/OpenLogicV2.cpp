// Copyright 2024 - NegativeNameSeller

#include "OpenLogicV2.h"

#include "Core/OpenLogicTypes.h"

#define LOCTEXT_NAMESPACE "FOpenLogicV2Module"

void FOpenLogicV2Module::StartupModule()
{}

void FOpenLogicV2Module::ShutdownModule()
{}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOpenLogicV2Module, OpenLogicV2)
DEFINE_LOG_CATEGORY(OpenLogicLog);