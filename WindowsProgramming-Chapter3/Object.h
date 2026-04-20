#pragma once

template <RealNumber CoordType>
class Object
{
private:
	Object(ShapeType type,TrailType trailType) :
		_type{ type }, _trailType{trailType}, _pos {},
		_velocityDirection{}, _velocityAmount{},
		_trailingVelocityDirection{}, _trailingVelocityAmount{},
		_angleVelocity{0.f},
		_thickness {1},
		_angle{}, _size{}, _sizeDelta{}, _sizeDeltaAmount{0.f},
		_penColor{DefaultColors::Black}
	{
		std::random_device rd;
		std::mt19937_64 mt(rd());
		std::uniform_int_distribution uid{ std::to_underlying(
			DefaultColors::Black), std::to_underlying(
			DefaultColors::End) - 1 };
		_fillColor = static_cast<DefaultColors>(uid(mt));
	}
public:
	struct DebugInfo
	{
		ShapeType type;
		TrailType trailType;
		//
		Vector2<CoordType> pos;
		Vector2<CoordType> offsetPos;
		Vector2<CoordType> velocity;
		Vector2<CoordType> trailingVelocity;
		Vector2<CoordType> size;
		float angle;
	};
	Object() : Object(ShapeType::None, TrailType::None)
	{
	}

	template <RealNumber AvailableCoordTypeT, RealNumber AvailableCoordTypeU>
	Object(ShapeType type, const Vector2<AvailableCoordTypeT>& pos,
		const Vector2<AvailableCoordTypeU>& size)
		: Object(type, TrailType::None)
	{
		SetPos(pos); SetSize(size);
	}


	~Object() {}

	Vector2<CoordType> GetPos() const noexcept {
		return _pos;
	}
	Vector2<CoordType> GetOffsetPos() const noexcept {
		return _offset;
	}
	template<RealNumber OtherCoordType>
	void SetPos(const Vector2<OtherCoordType>& pos)
		requires (!isCommonNumberType<CoordType,OtherCoordType>)
	{
		Vector2<CoordType> tmp{
			static_cast<CoordType>(pos.x),
			static_cast<CoordType>(pos.y)
		};
		_pos = tmp;
	}

	template <RealNumber OtherCoordType>
	void SetPos(const Vector2<OtherCoordType>& pos)
		requires (isCommonNumberType<CoordType,OtherCoordType>)
	{
		_pos = pos;
	}

	template <RealNumber OtherCoordType>
	void SetSize(const Vector2<OtherCoordType>& size)
		requires (isCommonNumberType<CoordType,OtherCoordType>)
	{
		_size = size;
	}

	Vector2<CoordType> GetSize() const noexcept
	{
		return _size;
	}
	Vector2<CoordType> GetVelocity() const noexcept
	{
		return _velocityDirection* _velocityAmount;
	}

	Vector2<CoordType> GetVelocityDir() const noexcept
	{
		return _velocityDirection;
	}

	template<RealNumber OtherCoordType>
	void SetSize(const Vector2<OtherCoordType>& size)
		requires (!isCommonNumberType<CoordType, OtherCoordType>)
	{
		Vector2<CoordType> tmp{
			static_cast<CoordType>(size.x),
			static_cast<CoordType>(size.y)
		};
		_size = tmp;
	}

	template <RealNumber OtherCoordType>
	void SetVelocity(const Vector2<OtherCoordType>& other)
	{
		SetVelocity(Vector2<CoordType>{static_cast<CoordType>(other.x),
			static_cast<CoordType>(other.y)});
	}
	void SetVelocity(const Vector2<CoordType>& other)
	{
		if (other.nearZero())
			_velocityAmount = 0.f;
		else
			_velocityAmount = other.magnitude();
		_velocityDirection = other.normalize();
	}

	template <RealNumber OtherCoordType>
	void SetVelocityDir(const Vector2<OtherCoordType>& dir)
	{
		SetVelocityDir(Vector2<CoordType>{static_cast<CoordType>(dir.x),
			static_cast<CoordType>(dir.y)});
	}
	void SetVelocityDir(const Vector2<CoordType>& dir)
	{
		_velocityDirection = dir.normalize();
	}

	void SetVelocityAmount(float amount)
	{
		_velocityAmount = amount;
	}
	void SetAngleVelocity(float velocity)
	{
		_angleVelocity = velocity;
	}
	void AddAngleVelocity(float velocity)
	{
		_angleVelocity += velocity;
	}

	template <RealNumber OtherCoordType>
	void AddVelocity(const Vector2<OtherCoordType>& other)
	{
		AddVelocity(Vector2<CoordType>{static_cast<CoordType>(other.x),
			static_cast<CoordType>(other.y)});
	}
	void AddVelocity(const Vector2<CoordType>& other)
	{
		_velocityAmount += other.magnitude();
		_velocityDirection += other.normalize();
	}

	void AddVelocityAmount(float amount)
	{
		_velocityAmount += amount; 
	}

	template <RealNumber OtherCoordType>
	void SetSizeDelta(const Vector2<OtherCoordType>& dir)
	{
		SetSizeDelta(Vector2<CoordType>{static_cast<CoordType>(dir.x),
			static_cast<CoordType>(dir.y)});
	}
	void SetSizeDelta(const Vector2<CoordType>& dir)
	{
		_sizeDelta = dir.normalize();
	}

	void SetSizeDeltaAmount(float amount)
	{
		_sizeDeltaAmount = amount;
	}
	void AddSizeDeltaAmount(float amount)
	{
		_sizeDeltaAmount += amount;
	}

	template <RealNumber OtherCoordType>
	void AddSizeDelta(const Vector2<OtherCoordType>& other)
	{
		AddSizeDelta(Vector2<CoordType>{static_cast<CoordType>(other.x),
			static_cast<CoordType>(other.y)});
	}
	void AddSizeDelta(const Vector2<CoordType>& other)
	{
		_sizeDeltaAmount += other.magnitude();
		_sizeDelta = (_sizeDelta + other).normalize();
	}

	void Render(HDC backDC, const Vector2<float>& scale,
		std::optional<Vector2<float>> gridSize) 
	{
		if (scale.comp_eq_float(Vector2 <float>{0.f, 0.f}))
			return;
		if (gridSize.has_value()) 
			return RenderGrid(backDC, scale, gridSize.value());
		XFORM transform, oldTransform;
		GetWorldTransform(backDC, &oldTransform);
		FLOAT cos = std::cos(_angle);
		FLOAT sin = std::sin(_angle);
		transform.eM11 = cos;
		transform.eM12 = sin;
		transform.eM21 = -1.f * sin;
		transform.eM22 = cos;
		transform.eDx = (FLOAT)_pos.x - (cos * _pos.x - sin * _pos.y);
		transform.eDy = (FLOAT)_pos.y - (sin * _pos.x + cos * _pos.y);


		SetWorldTransform(backDC, &transform);
		switch (_type)
		{
		case ShapeType::Circle:
			RenderUtil::DrawEllipse(backDC,
				_pos + _offset, _size.x, _size.y);
			break;
		case ShapeType::Square:
			RenderUtil::DrawRectCH(backDC,
				_pos + _offset, _size.x, _size.y);
			break;
		case ShapeType::Triangle:
		{
			{
				Vector2<CoordType> top{
				_pos.x, _pos.y - (2 / 3.f) * _size.y };
				Vector2<CoordType> left{ _pos.x - 0.5f * _size.x,
					_pos.y + (1 / 3.f) * _size.y };
				Vector2<CoordType> right{ _pos.x + 0.5f * _size.x,
					_pos.y + (1 / 3.f) * _size.y };
				RenderUtil::DrawPolygon(backDC, std::vector < Vector2<CoordType>>
				{ top, left, right });
			}
			break;
		}
		}
		SetWorldTransform(backDC, &oldTransform);
	}
	Vector2<CoordType> Update(float deltaTime)
	{
		if (!(std::abs(_velocityAmount - static_cast<float>(0.f))
			<= std::numeric_limits<float>::epsilon()))
			_pos += (_velocityDirection) * _velocityAmount * deltaTime;
		if (!(std::abs(_trailingVelocityAmount - static_cast<float>(0.f))
			<= std::numeric_limits<float>::epsilon()))
			_offset += _trailingVelocityDirection * _trailingVelocityAmount
			* deltaTime;
		if (!(std::abs(_angleVelocity - static_cast<float>(0.f))
			<= std::numeric_limits<float>::epsilon()))
			_angle += _angleVelocity * deltaTime;
		if (_angle >= 2.f * std::numbers::pi_v<float>)
			_angle -= 2.f * std::numbers::pi_v<float>;
		else if (_angle < 0.f)
			_angle += 2.f * std::numbers::pi_v<float>;
		if (!(std::abs(_sizeDeltaAmount) - 0.f <= std::numeric_limits<float>::epsilon()))
		{
			_size += _sizeDelta * _sizeDeltaAmount * deltaTime;
		}
		return _pos;
	}

	DebugInfo PrintDebug() const
	{
		return DebugInfo{ _type, _trailType,_pos,_offset,
		Vector2<CoordType>{_velocityDirection* _velocityAmount},
		Vector2<CoordType>{_trailingVelocityDirection*
		_trailingVelocityAmount},
		_size, _angle };
	}

	void SetFillColor(DefaultColors color)
	{
		if (color == DefaultColors::End) return;
		_fillColor = color;
	}

	void SetPenColor(DefaultColors color)
	{
		if (color == DefaultColors::End) return;
		_penColor = color;
	}

	void SetPenThickness(int newThickness)
	{
		if (newThickness > 10) return;
		_thickness = newThickness;
	}

	DefaultColors GetFillColor() const noexcept { return _fillColor; }
	DefaultColors GetPenColor() const noexcept { return _penColor; }
	int GetPenThickness() const noexcept { return _thickness; }

	friend struct CollideHelper;
private:


private:
	void RenderGrid(HDC backDC, const Vector2<float>& scale,
		const Vector2<float>& gridSize)
	{
		switch (_type)
		{
		case ShapeType::Circle:
			break;
		case ShapeType::Square:
			break;
		case ShapeType::Triangle:
			break;
		}
	}
private:
	DefaultColors _fillColor;
	DefaultColors _penColor; 
	int _thickness;

	ShapeType _type;
	TrailType _trailType;
	// 중심
	Vector2<CoordType> _pos;
	// 궤도 
	Vector2<CoordType> _offset;
	Vector2<CoordType> _velocityDirection;
	float _velocityAmount;

	Vector2<CoordType> _trailingVelocityDirection;
	float _trailingVelocityAmount;

	Vector2<CoordType> _sizeDelta;
	Vector2<CoordType> _size;
	float _sizeDeltaAmount;

	//각도
	float _angle;
	float _angleVelocity;
};

struct CollideHelper
{
	// 점과 도형 사이의 거리 <= 0?
	template <RealNumber CoordType, RealNumber AvailableCoordType>
	static bool Collide(const Object<CoordType>& obj,
		const Vector2<AvailableCoordType>& pt)
	{
		return Collide(obj, Vector2<CoordType>(
			static_cast<CoordType>(pt.x),
			static_cast<CoordType>(pt.y)
		));
	}


	template <RealNumber CoordType>
	static bool Collide(const Object<CoordType>& obj,
		const Vector2<CoordType>& pt) 
	{
		Vector2<CoordType> ptPrime{pt - (obj._pos + obj._offset)};
		Vector2<CoordType> ptConv{
				std::cos(obj._angle * -1.f) * ptPrime.x
				- std::sin(obj._angle * -1.f) * ptPrime.y,
				std::sin(obj._angle * -1.f) * ptPrime.x
				+ std::cos(obj._angle * -1.f) * ptPrime.y
		};
		switch (obj._type)
		{
		case ShapeType::Circle:
			return (ptConv.x * ptConv.x)/(obj._size.x * obj._size.x)
				+ (ptConv.y * ptConv.y)/(obj._size.y * obj._size.y) <= 1.f;
		case ShapeType::Square:
		{
			return (ptConv.x >= obj._size.x * -1 && ptConv.x <= obj._size.x
				&& ptConv.y >= obj._size.y * -1 && ptConv.y <= obj._size.y);
		}
		case ShapeType::Triangle:
		{
			Vector2<CoordType> top{
				0.f,  (-2 / 3.f) * obj._size.y };
			Vector2<CoordType> left{ -0.5f * obj._size.x,
				 + (1 / 3.f) * obj._size.y };
			Vector2<CoordType> right{ 0.5f * obj._size.x,
				 + (1 / 3.f) * obj._size.y };
			CoordType d[3] = {
				Vector2<CoordType>{left - top}.cross
				(Vector2<CoordType>{ptConv - top}),
				Vector2<CoordType>{right - left}.cross
				(Vector2<CoordType>{ptConv - left}),
				Vector2<CoordType>{top - right}.cross
				(Vector2<CoordType>{ptConv - right})
			};
			return (d[0] >= 0.f && d[1] >= 0.f && d[2] >= 0.f)
				|| (d[0] <= 0.f && d[1] <= 0.f && d[2] <= 0.f);
		}
		}
	}

	template <RealNumber CoordType>
	static bool CollideScreenBorder(const Object<CoordType>& obj,
		const RECT& windowRect)
	{
		// top
		switch (obj._type)
		{
		case ShapeType::Circle:
			return (obj._pos.x + obj._offset.x - obj._size.x) < 0.f ||
				(obj._pos.x + obj._offset.x + obj._size.x) > windowRect.right ||
				(obj._pos.y + obj._offset.y - obj._size.y) < 0.f ||
				(obj._pos.y + obj._offset.y + obj._size.y) > windowRect.bottom;
		case ShapeType::Square:
		{
			CoordType cx = obj._pos.x + obj._offset.x;
			CoordType cy = obj._pos.y + obj._offset.y;
			Vector2<CoordType> Vertices[4] = {
				Vector2<CoordType>{- obj._size.x, - obj._size.y}.rotateConv(obj._angle)
				+Vector2<CoordType>{cx,cy},
				Vector2<CoordType>{obj._size.x,  - obj._size.y}.rotateConv(obj._angle)
				+ Vector2<CoordType>{cx,cy},
				Vector2<CoordType>{- obj._size.x,  obj._size.y}.rotateConv(obj._angle)
				+ Vector2<CoordType>{cx,cy},
				Vector2<CoordType>{+obj._size.x, obj._size.y}.rotateConv(obj._angle)
				+ Vector2<CoordType>{cx,cy},
			};
			for (const auto& v : Vertices)
			{
				if (v.x < 0 || v.x > windowRect.right ||
					v.y < 0 || v.y >windowRect.bottom)
					return true;
			}
			return false;
		}
		case ShapeType::Triangle:
		{
			CoordType cx = obj._pos.x + obj._offset.x;
			CoordType cy = obj._pos.y + obj._offset.y;
			Vector2<CoordType> Vertices[3] = {
				Vector2<CoordType>{obj._size.x, (-2 / 3.f) *obj._size.y}.rotateConv(obj._angle)
				+ Vector2<CoordType>{cx,cy},
				Vector2<CoordType>{obj._size.x *(-0.5f),  obj._size.y *(1/3.f)}.rotateConv(obj._angle)
				+ Vector2<CoordType>{cx,cy},
				Vector2<CoordType>{-obj._size.x *(0.5f),  obj._size.y *(1/3.f)}.rotateConv(obj._angle)
				+ Vector2<CoordType>{cx,cy},
			};
			for (const auto& v : Vertices)
			{
				if (v.x < 0 || v.x > windowRect.right ||
					v.y < 0 || v.y >windowRect.bottom)
					return true;
			}
			return false;
		}
		}

	}
};