// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

#include "Utils/TerrainMagicThreading.h"

FGraphEventRef FTerrainMagicThreading::RunOnGameThread(TFunction<void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::GameThread);
}

FGraphEventRef FTerrainMagicThreading::RunOnAnyThread(TFunction<void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr, ENamedThreads::AnyThread);
}

FGraphEventRef FTerrainMagicThreading::RunOnAnyBackgroundThread(TFunction<void()> InFunction)
{
	return FFunctionGraphTask::CreateAndDispatchWhenReady(InFunction, TStatId(), nullptr,
														  ENamedThreads::AnyBackgroundThreadNormalTask);
}