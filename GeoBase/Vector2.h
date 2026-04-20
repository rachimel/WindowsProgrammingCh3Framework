#pragma once

template <RealNumber Ty>
struct Vector2
{
	Vector2() : x{}, y{}
	{
	}
	explicit Vector2(Ty x, Ty y)
		: x{ x }, y{ y }
	{
	}

	~Vector2() = default;

	Vector2(const Vector2&) = default;
	Vector2& operator= (const Vector2&) = default;

	Vector2(Vector2&&) noexcept = default;
	Vector2& operator= (Vector2&&) noexcept = default;

	Vector2 operator+(const Vector2& other) const noexcept
	{
		// RVO
		return Vector2{ x + other.x, y + other.y };
	}
	Vector2& operator+=(const Vector2& other) noexcept
	{
		x += other.x; y += other.y; return *this;
	}
	Vector2 operator-(const Vector2& other) const noexcept
	{
		return Vector2{ x - other.x, y - other.y };
	}
	Vector2& operator-=(const Vector2& other) noexcept
	{
		x -= other.x; y -= other.y; return *this;
	}
	template <typename U>
	Vector2 operator*(U scalar) const noexcept requires RealNumber<U>
	{
		return Vector2{ x * scalar,y * scalar };
	}

	bool operator== (const Vector2& other) const noexcept
		requires std::is_integral_v<Ty>
	{
		return x == other.x && y == other.y;
	}

	bool operator== (const Vector2& other) const noexcept
		requires std::is_floating_point_v<Ty>
	{
		return std::abs(x - other.x) < std::numeric_limits<Ty>::epsilon()
			&& std::abs(y - other.y) < std::numeric_limits<Ty>::epsilon();
	}

	bool operator!= (const Vector2& other) const noexcept
	{
		return !(*this == other);
	}

	bool comp_eq_float(const Vector2& other) const noexcept
		requires std::is_floating_point_v<Ty>
	{
		return operator==(other);
	}

	bool comp_neq_float(const Vector2& other) const noexcept
		requires std::is_floating_point_v<Ty>
	{
		return !(operator==(other));
	}

	Ty magnitude() const noexcept
	{
		return static_cast<Ty>(std::sqrt(x * x + y * y));
	}

	const bool nearZero() const requires std::is_floating_point_v<Ty>
	{
		return operator==(Vector2<Ty>{0.f, 0.f});
	}
	using RetFlt = std::conditional_t<
		sizeof(Ty) <= sizeof(int), float, double>;
	RetFlt magnitude_float() requires std::is_integral_v<Ty>
	{
		return static_cast<RetFlt>(std::sqrt(x * x + y * y));
	}

	Ty dot(const Vector2& other) const noexcept
	{
		return x * other.x + y * other.y;
	}

	Ty cross(const Vector2& other) const noexcept
	{
		return x * other.y - y * other.x;
	}

	void rotate(float angle)
	{
		if (angle - 0.f < std::numeric_limits<float>::epsilon())
			return;
		auto tmp = *this;
		x = std::cos(angle) * tmp.x - std::sin(angle) * tmp.y;
		y = std::sin(angle) * tmp.x + std::cos(angle) * tmp.y;
	}
	Vector2<Ty> rotateConv(float angle)
	{
		// ret 
		if (angle - 0.f < std::numeric_limits<float>::epsilon())
			return *this;
		return Vector2<Ty>{ std::cos(angle) * x - std::sin(angle) * y ,
		std::sin(angle)* x + std::cos(angle) * y };
	}

	Ty length_square() const noexcept
	{
		return x * x + y * y;
	}
	Vector2 normalize() const noexcept
		requires std::is_floating_point_v<Ty>
	{
		const auto mag = magnitude();
		return Vector2{ x / mag, y / mag };
	}
	Vector2<RetFlt> normalize() const noexcept
		requires std::is_integral_v<Ty>
	{
		const auto mag = magnitude_float();
		return Vector2<RetFlt>{ x / mag, y / mag };
	}

	Ty x;
	Ty y;
};
