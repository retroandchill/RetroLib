/**
 * @file RetroLib.ixx
 * @brief Module declaration for RetroLib.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
module;

#include "RetroLib/RetroLibMacros.h"

#include <bit>
#include <cassert>
#include <ranges>
#include <typeinfo>
#include <initializer_list>

export module RetroLib;

import std;

#define RETROLIB_EXPORT export

// clang-format off
#include "RetroLib/TypeTraits.h"
#include "RetroLib/FunctionTraits.h"

#include "RetroLib/TypeTraits.h"

#include "RetroLib/Concepts/Operators.h"
#include "RetroLib/Concepts/Inheritance.h"
#include "RetroLib/Concepts/OpaqueStorage.h"
#include "RetroLib/Concepts/ParameterPacks.h"

#include "RetroLib/Optionals/OptionalOperations.h"
#include "RetroLib/Concepts/Pointers.h"

#include "RetroLib/Utils/ForwardLike.h"
#include "RetroLib/Utils/Operators.h"
#include "RetroLib/Utils/WrapArg.h"
#include "RetroLib/Utils/Polymorphic.h"
#include "RetroLib/Utils/ValidPtr.h"

#include "RetroLib/Optionals/Optional.h"

#include "RetroLib/Casting/Convert.h"
#include "RetroLib/Casting/ByteCast.h"
#include "RetroLib/Casting/ClassCast.h"
#include "RetroLib/Casting/InstanceOf.h"

#include "RetroLib/Functional/BindFunctor.h"
#include "RetroLib/Functional/BindBack.h"
#include "RetroLib/Functional/BindFront.h"
#include "RetroLib/Functional/BindMethod.h"
#include "RetroLib/Functional/ExtensionMethods.h"

#include "RetroLib/Optionals/Filter.h"

#include "RetroLib/Ranges/FeatureBridge.h"
#include "RetroLib/Ranges/Concepts/Containers.h"

#include "RetroLib/Ranges/Algorithm/To.h"
#include "RetroLib/Ranges/Views/AnyView.h"
#include "RetroLib/Ranges/Views/Filter.h"
#include "RetroLib/Ranges/Views/Transform.h"

// clang-format on
