// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#ifdef __UNREAL__
#include "BlueprintException.h"

namespace Retro {
	/**
	 * Exception for where there is an invalid argument in a Blueprint function.
	 */
	class RETROLIB_API FInvalidArgumentException : public FBlueprintException {
	public:
		using FBlueprintException::FBlueprintException;
	};
} // namespace Retro
#endif