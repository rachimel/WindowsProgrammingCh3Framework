#pragma once
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
	Lavender,
	Purple,
	End
};

namespace Color
{
	std::uniform_int_distribution<
		std::underlying_type_t<DefaultColors>> dist{
		std::to_underlying(DefaultColors::Black),
		std::to_underlying(DefaultColors::End) - 1
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
	Circle,
	Square,
	Triangle
};

// Scene
enum class SceneType
{
	Assignment1,
	Assignemnt2,
	Assignemnt3,

};
