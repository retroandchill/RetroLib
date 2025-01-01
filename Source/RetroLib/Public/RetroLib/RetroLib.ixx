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

export module RetroLib;

import std;

#define RETROLIB_EXPORT export

// clang-format off
#include "RetroLib/TypeTraits.h"
#include "RetroLib/FunctionTraits.h"

#include "RetroLib/TypeTraits.h"

#include "RetroLib/Concepts/CompleteType.h"
#include "RetroLib/Concepts/Operators.h"
#include "RetroLib/Concepts/Iterators.h"
#include "RetroLib/Concepts/Inheritance.h"
#include "RetroLib/Concepts/OpaqueStorage.h"
#include "RetroLib/Concepts/ParameterPacks.h"

#include "RetroLib/Optionals/OptionalOperations.h"
#include "RetroLib/Concepts/Pointers.h"

#include "RetroLib/Utils/ForwardLike.h"
#include "RetroLib/Utils/Operators.h"
#include "RetroLib/Utils/Unreachable.h"
#include "RetroLib/Utils/ValidPtr.h"
#include "RetroLib/Utils/Variant.h"

#include "RetroLib/Concepts/Tuples.h"
#include "RetroLib/Optionals/OptionalIterator.h"
#include "RetroLib/Optionals/Optional.h"

#include "RetroLib/Utils/Polymorphic.h"
#include "RetroLib/Utils/Tuple.h"
#include "RetroLib/Utils/NonPropagatingCache.h"
#include "RetroLib/Utils/UniqueAny.h"

#include "RetroLib/Casting/StaticCast.h"
#include "RetroLib/Casting/ByteCast.h"
#include "RetroLib/Casting/DynamicCast.h"
#include "RetroLib/Casting/InstanceOf.h"

#include "RetroLib/Functional/BindFunctor.h"
#include "RetroLib/Functional/BindBack.h"
#include "RetroLib/Functional/BindFront.h"
#include "RetroLib/Functional/BindMethod.h"
#include "RetroLib/Functional/CreateBinding.h"
#include "RetroLib/Functional/ExtensionMethods.h"
#include "RetroLib/Functional/FunctionalClosure.h"

#include "RetroLib/Optionals/AndThen.h"
#include "RetroLib/Optionals/Filter.h"
#include "RetroLib/Optionals/IfPresent.h"
#include "RetroLib/Optionals/IfPresentOrElse.h"
#include "RetroLib/Optionals/IsSet.h"
#include "RetroLib/Optionals/OrElse.h"
#include "RetroLib/Optionals/OrElseGet.h"
#include "RetroLib/Optionals/OrElseThrow.h"
#include "RetroLib/Optionals/OrElseValue.h"
#include "RetroLib/Optionals/PtrOrNull.h"
#include "RetroLib/Optionals/To.h"
#include "RetroLib/Optionals/Transform.h"
#include "RetroLib/Optionals/Value.h"

#include "RetroLib/Ranges/RangeBasics.h"
#include "RetroLib/Ranges/FeatureBridge.h"
#include "RetroLib/Ranges/Concepts/Containers.h"
#include "RetroLib/Ranges/Concepts/Concatable.h"
#include "RetroLib/Ranges/Concepts/Iterable.h"

#include "RetroLib/Ranges/Compatibility/ForEachRange.h"

#include "RetroLib/Ranges/Algorithm/NameAliases.h"
#include "RetroLib/Ranges/Algorithm/FindFirst.h"
#include "RetroLib/Ranges/Algorithm/Reduce.h"
#include "RetroLib/Ranges/Algorithm/To.h"

#include "RetroLib/Ranges/Views/NameAliases.h"
#include "RetroLib/Ranges/Views/AnyView.h"
#include "RetroLib/Ranges/Views/CacheLast.h"
#include "RetroLib/Ranges/Views/Concat.h"
#include "RetroLib/Ranges/Views/Elements.h"
#include "RetroLib/Ranges/Views/Enumerate.h"
#include "RetroLib/Ranges/Views/Filter.h"
#include "RetroLib/Ranges/Views/Generator.h"
#include "RetroLib/Ranges/Views/JoinWith.h"
#include "RetroLib/Ranges/Views/Transform.h"

#ifdef __UNREAL__
#include "RetroLib/Concepts/Delegates.h"
#include "RetroLib/Concepts/Interfaces.h"
#include "RetroLib/Concepts/Structs.h"

#include "RetroLib/Exceptions/BlueprintException.h"
#include "RetroLib/Exceptions/TypeException.h"
#include "RetroLib/Exceptions/InvalidArgumentException.h"
#include "RetroLib/Exceptions/VariantException.h"
#include "RetroLib/Exceptions/ConvertException.h"

#include "RetroLib/Functional/Delegates.h"
#include "RetroLib/Optionals/Compatiblity/UnrealOptional.h"
#include "RetroLib/Ranges/Compatibility/UnrealContainers.h"
#include "RetroLib/Ranges/Compatibility/Array.h"
#include "RetroLib/Utils/SoftObjectRef.h"

#include "RetroLib/Assets/AsyncLoadHandle.h"

#include "RetroLib/Variants/VariantObject.h"
#include "RetroLib/Variants/SoftVariantObject.h"
#include "RetroLib/Variants/VariantObjectStruct.h"
#include "RetroLib/Variants/VariantObjectUtilities.h"

#include "RetroLib/Casting/UClassCasts.h"
#include "RetroLib/Blueprints/BlueprintRuntimeUtils.h"
#include "RetroLib/Blueprints/Properties.h"
#endif
// clang-format on
