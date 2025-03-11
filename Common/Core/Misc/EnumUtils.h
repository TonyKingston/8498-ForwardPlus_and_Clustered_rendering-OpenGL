#pragma once

#define ENUM_CLASS_FLAGS(Enum) \
	ENUM_CLASS_FLAGS_INTERNAL(Enum)

#define ENUM_CLASS_FLAGS_INTERNAL(Enum) \
	inline constexpr Enum& operator|=(Enum& Lhs, Enum Rhs) { \
		using Type = __underlying_type(Enum); \
		return Lhs = static_cast<Enum>(static_cast<Type>(Lhs) | static_cast<Type>(Rhs)); \
	} \
	inline constexpr Enum& operator&=(Enum& Lhs, Enum Rhs) { \
		using Type = __underlying_type(Enum); \
		return Lhs = static_cast<Enum>(static_cast<Type>(Lhs) & static_cast<Type>(Rhs)); \
	} \
	inline constexpr Enum& operator^=(Enum& Lhs, Enum Rhs) { \
		using Type = __underlying_type(Enum); \
		return Lhs = static_cast<Enum>(static_cast<Type>(Lhs) ^ static_cast<Type>(Rhs)); \
	} \
	[[nodiscard]] inline constexpr Enum operator|(Enum Lhs, Enum Rhs) { \
		using Type = __underlying_type(Enum); \
		return static_cast<Enum>(static_cast<Type>(Lhs) | static_cast<Type>(Rhs)); \
	} \
	[[nodiscard]] inline constexpr Enum operator&(Enum Lhs, Enum Rhs) { \
		using Type = __underlying_type(Enum); \
		return static_cast<Enum>(static_cast<Type>(Lhs) & static_cast<Type>(Rhs)); \
	} \
	[[nodiscard]] inline constexpr Enum operator^(Enum Lhs, Enum Rhs) { \
		using Type = __underlying_type(Enum); \
		return static_cast<Enum>(static_cast<Type>(Lhs) ^ static_cast<Type>(Rhs)); \
	} \
	[[nodiscard]] inline constexpr bool operator!(Enum Rhs)             { \
		using Type = __underlying_type(Enum); \
		return !static_cast<Type>(E); \
	} \
	[[nodiscard]] inline constexpr Enum operator~(Enum Rhs)             { \
		using Type = __underlying_type(Enum); \
		return static_cast<Enum>(~static_cast<Type>(Rhs)); \
	} \

template<typename Enum>
constexpr bool EnumHasFlag(Enum Flags, Enum Mask) {
	using Type = __underlying_type(Enum);
	return static_cast<Type>(Flags & Mask) == static_cast<Type>(Mask);
}

template<typename Enum>
constexpr bool EnumHasAnyFlags(Enum Flags, Enum Mask) {
	using Type = __underlying_type(Enum);
	return static_cast<Type>(Flags & Mask) != 0;
}
