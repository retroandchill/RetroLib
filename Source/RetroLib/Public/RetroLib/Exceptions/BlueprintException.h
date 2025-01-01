// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#ifdef __UNREAL__
#include <string>
#include <exception>

#include "Blueprint/BlueprintExceptionInfo.h"
#include <string_view>

namespace Retro {
	class RETROLIB_API FBlueprintException : public std::exception {
	public:
		FBlueprintException() = default;

		explicit FBlueprintException(const char* Message, EBlueprintExceptionType::Type Type = EBlueprintExceptionType::AccessViolation);

		explicit FBlueprintException(std::string_view Message, EBlueprintExceptionType::Type Type = EBlueprintExceptionType::AccessViolation);

		explicit FBlueprintException(std::string&& Message, EBlueprintExceptionType::Type Type = EBlueprintExceptionType::AccessViolation);

		[[nodiscard]] const char* what() const override;

		EBlueprintExceptionType::Type GetType() const {
			return Type;
		}

	private:
		std::string Message;
		EBlueprintExceptionType::Type Type = EBlueprintExceptionType::AccessViolation;
	};
}
#endif