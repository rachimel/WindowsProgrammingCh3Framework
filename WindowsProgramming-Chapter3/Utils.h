#pragma once

namespace Util
{
	template <RealNumber CoordType>
	Vector2<CoordType> Smooth(const
		Vector2<CoordType>& a, const Vector2<CoordType>& b,
		float t)
	{
		if (t >= 1.f)
			return b;
		else if (t <= 0.f)
			return a;
		float st{ (t * t * (3 - 2 * t)) };
		return Vector2{
			a.x + (b.x - a.x) * st,
			a.y + (b.y - a.y) * st
		};
	}
}
namespace RenderUtil
{
	template <std::integral Ty>
	static inline void DrawEllipse(HDC backDC, const Vector2<Ty>& centerPos,
		Ty radiusX, Ty radiusY)
	{
		Ellipse(backDC,
			centerPos.x - radiusX,
			centerPos.y - radiusY,
			centerPos.x + radiusX,
			centerPos.y + radiusY);
	}
	template <std::integral Ty>
	static inline void DrawCircle(HDC backDC, const Vector2<Ty>& centerPos, Ty radius)
	{
		DrawEllipse(backDC, centerPos, radius, radius);
	}

	template <std::integral Ty>
	static inline void DrawRect(HDC backDC, const Vector2<Ty>&
		stPos, Ty width, Ty height)
	{
		Rectangle(backDC, stPos.x, stPos.y, stPos.x + width, stPos.y + height);
	}

	template <std::integral Ty>
	static inline void DrawRectC(HDC backDC, const Vector2<Ty>&
		centerPos, Ty width, Ty height)
	{
		Rectangle(backDC,
			centerPos.x - width / 2,
			centerPos.y - height / 2,
			centerPos.x + width / 2,
			centerPos.y + height / 2);
	}

	template <std::integral Ty>
	static inline void DrawRectCH(HDC backDC, const Vector2<Ty>&
		centerPos, Ty widthHalf, Ty heightHalf)
	{
		Rectangle(backDC,
			centerPos.x - widthHalf,
			centerPos.y - heightHalf,
			centerPos.x + widthHalf,
			centerPos.y + heightHalf);
	}

	template <std::integral Ty>
	static inline void DrawPolygon(HDC backDC, const
		std::vector<Vector2<Ty>>& pts)
	{
		if constexpr (sizeof(Vector2<int>) == sizeof(POINT))
		{
			Polygon(backDC, reinterpret_cast<POINT*>(pts.data()),
				static_cast<int>(pts.size()));
		}
		else
		{
			std::vector<POINT> tmp{};
			tmp.reserve(pts.size());
			for (const auto& pt : pts) tmp.push_back(POINT{ pt.x,pt.y });
			Polygon(backDC, tmp.data(), static_cast<int>(tmp.size()));
		}
	}

	// floating point Utils

	template <std::floating_point Ty>
	static inline void DrawEllipse(HDC backDC, const Vector2<Ty>&
		centerPos, Ty radiusX, Ty radiusY)
	{
		Ellipse(backDC,
			static_cast<int>(centerPos.x - radiusX),
			static_cast<int>(centerPos.y - radiusY),
			static_cast<int>(centerPos.x + radiusX),
			static_cast<int>(centerPos.y + radiusY));
	}

	template <std::floating_point Ty>
	static inline void DrawCircle(HDC backDC, const Vector2<Ty>& centerPos, Ty radius)
	{
		DrawEllipse(backDC, centerPos, radius, radius);
	}

	template <std::floating_point Ty>
	static inline void DrawRect(HDC backDC, const Vector2<Ty>&
		stPos, Ty width, Ty height)
	{
		Rectangle(backDC,
			static_cast<int>(stPos.x),
			static_cast<int>(stPos.y),
			static_cast<int>(stPos.x + width),
			static_cast<int>(stPos.y + height));
	}

	template <std::floating_point Ty>
	static inline void DrawRectC(HDC backDC, const Vector2<Ty>&
		centerPos, Ty width, Ty height)
	{
		Rectangle(backDC,
			static_cast<int>(centerPos.x - width / 2.f),
			static_cast<int>(centerPos.y - height / 2.f),
			static_cast<int>(centerPos.x + width / 2.f),
			static_cast<int>(centerPos.y + height / 2.f));
	}

	template <std::floating_point Ty>
	static inline void DrawRectCH(HDC backDC, const Vector2<Ty>&
		centerPos, Ty widthHalf, Ty heightHalf)
	{
		Rectangle(backDC,
			static_cast<int>(centerPos.x - widthHalf),
			static_cast<int>(centerPos.y - heightHalf),
			static_cast<int>(centerPos.x + widthHalf),
			static_cast<int>(centerPos.y + heightHalf));
	}

	template <std::floating_point Ty>
	static inline void DrawPolygon(HDC backDC, const
		std::vector<Vector2<Ty>>& pts)
	{
		std::vector<POINT> tmp{};
		tmp.reserve(pts.size());
		for (const auto& pt : pts) tmp.push_back(POINT{
			static_cast<int>(pt.x), static_cast<int>(pt.y)
			});
		Polygon(backDC, tmp.data(), static_cast<int>(tmp.size()));
	}
}
