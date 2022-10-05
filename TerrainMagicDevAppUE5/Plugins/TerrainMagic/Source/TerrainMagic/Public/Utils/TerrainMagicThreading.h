// Copyright (c) 2022 GDi4K. All Rights Reserved.

#pragma once

class FTerrainMagicThreading
{
public:
	static FGraphEventRef RunOnGameThread(TFunction<void()> InFunction);

	static FGraphEventRef RunOnAnyThread(TFunction<void()> InFunction);

	static FGraphEventRef RunOnAnyBackgroundThread(TFunction<void()> InFunction);
};