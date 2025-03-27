#pragma once
#include <type_traits>
#include <memory>

namespace NCL {

	/* Use these for conditional static_asserts inside templates
	 Example:
		template <typename T>
		void Process(const T& value) {
		if constexpr (std::is_integral_v<T>) {
			std::cout << "Integer: " << value << "\n";
		} else {
			static_assert(AlwaysFalse<T>, "Unsupported type!");
		}
	*/
	template <typename...>
	constexpr auto AlwaysFalse = std::false_type{};

	template <typename...>
	constexpr bool AlwaysTrue = std::true_type{};

	// Lambda overload trick for visitors
	// This version work for move-only lambdas and mixes of lambdas and other function objects (e.g. std::function)
	template <class ...Ts>
	struct Overload : Ts... {
		template <class ...Fs>
		Overload(Fs&& ...ts) : Ts{ std::forward<Fs>(ts) }... {}

		using Ts::operator()...;
	};

	template <class ...Ts>
	Overload(Ts&&...)->Overload<std::remove_reference_t<Ts>...>;

	// Used for counting number of __VA_ARGS__ in a macro
	// Avoid having to do things like std::tuple_size_v<decltype(std::make_tuple(__VA_ARGS__))>>
	template <typename... T>
	consteval std::size_t CountArgs(T&&...) {
		return sizeof...(T);
	}

}