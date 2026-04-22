#pragma once

constexpr size_t g_maxThickness{ 10 };
template <typename Ty>
concept HasEnumEnd = std::is_enum_v<Ty> &&
	requires { {Ty::End};};

enum class DefaultColors
{
	Black,
	Grey,
	White,
	Red,
	Orange,
	Yellow,
	Lime,
	Green,
	Skyblue,
	Blue,
	Pink,
	Purple,
	Transparent
};

namespace Color
{
	std::uniform_int_distribution<
		std::underlying_type_t<DefaultColors>> dist{
		std::to_underlying(DefaultColors::Black),
		std::to_underlying(DefaultColors::Transparent) - 1
	};
}
// Flag Enum Set
enum class ShapeType 
{
	None, 
	Square,
	Circle,
	Triangle,
};

enum class TrailType
{
	None,
	Square,
	Circle,
	Triangle
};

enum DebugFlag
{
	DEBUG_STATUS = 0b1,
	DEBUG_OBJECT = 0b10,
};