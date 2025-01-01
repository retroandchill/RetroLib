#if WITH_TESTS
#include "TestAdapter.h"
#include "RetroLib/Optionals/Compatiblity/UnrealOptional.h"

TEST_CASE_NAMED(FOptionalUnsetTest, "RetroLib::Optionals::Unset", "[RetroLib][Optionals]") {
	SECTION("Can use the unset state of an Unreal type with a Retro::Optional") {
		static_assert(Retro::HasAlternateIntrusiveUnsetState<TArray<int32>>);
		static_assert(sizeof(TArray<int32>) == sizeof(Retro::Optional<TArray<int32>>));
		Retro::Optional OptionalValue = TArray<int32>();
		CHECK(OptionalValue.HasValue());
		OptionalValue.Reset();
		CHECK_FALSE(OptionalValue.HasValue());
	}
}
#endif