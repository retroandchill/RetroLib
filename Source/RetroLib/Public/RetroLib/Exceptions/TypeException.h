// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "BlueprintException.h"

#ifdef __UNREAL__
namespace Retro {
	/**
	 * 
	 */
	class RETROLIB_API FTypeException : public FBlueprintException {
	public:
		using FBlueprintException::FBlueprintException;
	};
}
#endif