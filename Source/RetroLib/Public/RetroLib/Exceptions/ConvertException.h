// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#ifdef __UNREAL__
#include <Blueprint/BlueprintExceptionInfo.h>
#include <exception>

namespace Retro {
	RETROLIB_API FBlueprintExceptionInfo ConvertException(const std::exception& Exception, EBlueprintExceptionType::Type Type = EBlueprintExceptionType::AccessViolation);

	RETROLIB_API FBlueprintExceptionInfo ConvertException(const class FBlueprintException& Exception);
}
#endif