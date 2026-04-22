#pragma once

template <RealNumber CoordType>
class Object
{
private:
	Object(ShapeType type, TrailType trailType) :
		_type{ type }, _trailType{ trailType }, _pos{},
		_velocityDirection{}, _velocityAmount{},
		_offsetDir{}, _offsetAmount{},
		_offsetVelocityDirection{}, _offsetVelocityAmount{},
		_offsetAngle{0.f}, _offsetAngleVelocity{0.f},
		_angleVelocity{ 0.f },
		_thickness{ 1 },
		_angle{}, _size{}, _sizeDelta{}, _sizeDeltaAmount{ 0.f },
		_penColor{ DefaultColors::Black },
		_lerpPos{ Vector2<float>{},Vector2<float>{}, 1.f },
		_lerpOffset{ Vector2<float>{},Vector2<float>{}, 1.f },
		_lerpSize{ Vector2<float>{}, Vector2<float>{}, 1.f }
	{
		std::random_device rd;
		std::mt19937_64 mt(rd());
		std::uniform_int_distribution uid{ std::to_underlying(
			DefaultColors::Black), std::to_underlying(
			DefaultColors::Transparent) - 1 };
		_fillColor = static_cast<DefaultColors>(uid(mt));
		_oldFillColor = _fillColor;
		_oldPenColor = _penColor;
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
		Vector2<CoordType> offsetVelocity;
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
		return _offsetDir * _offsetAmount;
	}

	ShapeType GetShapeType() const noexcept { return _type; }
	void SetShapeType(ShapeType type) { _type = type; }
	TrailType GetTrailType() const noexcept { return _trailType; }
	void SetTrailType(TrailType type) { _trailType = type; }

	template<RealNumber OtherCoordType>
	void SetPos(const Vector2<OtherCoordType>& pos)
	{
		SetPos(Vector2<CoordType>{
			static_cast<CoordType>(pos.x),
				static_cast<CoordType>(pos.y)
		});
	}

	template <RealNumber OtherCoordType>
	void SetPos(const Vector2<OtherCoordType>& pos)
		requires (isCommonNumberType<CoordType, OtherCoordType>)
	{
		_lerpPos.t = 1.f;
		_pos = pos;
	}


	template <RealNumber OtherCoordType>
	void SetPosLerp(const Vector2<OtherCoordType>& lerpPos)
	{
		SetPosLerp(Vector2<CoordType>{static_cast<CoordType>(lerpPos.x),
		static_cast<CoordType>(lerpPos.y)});
	}

	void SetPosLerp(const Vector2<CoordType>& lerpPos)
	{
		_lerpPos.st = _pos;
		_lerpPos.target = lerpPos;
		_lerpPos.t = 0.f;
	}

	void SetOffsetAmount(float amount)
	{
		_offsetAmount = amount;
	}
	template <RealNumber OtherCoordType>
	void SetOffsetLerp(const Vector2<OtherCoordType>& lerpOffset)
	{
		SetOffsetLerp(lerpOffset);
	}

	void SetOffsetLerp(const Vector2<CoordType>& lerpOffset)
	{
		if (_lerpOffset.t > 0.f && _lerpOffset.t < 1.f) return;
		_lerpOffset.st = _offsetDir * _offsetAmount;
		_lerpOffset.target = lerpOffset;
		_lerpOffset.t = 0.f;
	}

	bool isOffsetLerpDone() const noexcept
	{
		return std::abs(_lerpOffset.t - 1.f) < std::numeric_limits<float>::epsilon();
	}
	template<RealNumber OtherCoordType>
	void SetSize(const Vector2<OtherCoordType>& size)
	{
		SetSize(Vector2<CoordType>{
			static_cast<CoordType>(size.x),
			static_cast<CoordType>(size.y)
		});
	}
	void SetSize(const Vector2<CoordType>& size)
	{
		_lerpSize.t = 1.f;
		_size = size;
	}
	template<RealNumber OtherCoordType>
	void SetSizeLerp(const Vector2<OtherCoordType>& lerpSize)
	{
		SetSizeLerp(Vector2<CoordType> {
			static_cast<CoordType>(lerpSize.x),
				static_cast<CoordType>(lerpSize.y)
		});
	}
	void SetSizeLerp(const Vector2<CoordType>& lerpSize)
	{
		if (_lerpSize.t > 0.f && _lerpSize.t < 1.f) return;
		_lerpSize.st = _size;
		_lerpSize.target = lerpSize;
		_lerpSize.t = 0.f;
	}

	Vector2<CoordType> GetSize() const noexcept
	{
		return _size;
	}
	Vector2<CoordType> GetVelocity() const noexcept
	{
		return _velocityDirection * _velocityAmount;
	}

	Vector2<CoordType> GetVelocityDir() const noexcept
	{
		return _velocityDirection;
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
	void SetAngle(float angle)
	{
		_angle = angle;
	}
	float GetAngle() const noexcept { return _angle; }
	void SetAngleVelocity(float velocity)
	{
		_angleVelocity = velocity;
	}
	void AddAngleVelocity(float velocity)
	{
		_angleVelocity += velocity;
	}
	void SetOffsetAngle(float angle) { _offsetAngle = _angle; }
	float GetOffsetAngle() { return _offsetAngle; }


	void SetOffsetAngleVelocity(float velocity)
	{
		_offsetAngleVelocity = velocity;
	}
	void AddOffsetAngleVelocity(float velocity)
	{
		_offsetAngleVelocity += velocity;
	}

	float GetOffsetAngleVelocity()
	{
		return _offsetAngleVelocity;
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
				_pos + (_offsetDir * _offsetAmount), _size.x, _size.y);
			break;
		case ShapeType::Square:
			RenderUtil::DrawRectCH(backDC,
				_pos + (_offsetDir * _offsetAmount), _size.x, _size.y);
			break;
		case ShapeType::Triangle:
		{
			{
				Vector2<CoordType> top{
				_pos.x, _pos.y - _size.y };
				Vector2<CoordType> left{ _pos.x -  _size.x,
					_pos.y + _size.y };
				Vector2<CoordType> right{ _pos.x +  _size.x,
					_pos.y + _size.y };
				RenderUtil::DrawPolygon(backDC, std::vector < Vector2<CoordType>>
				{ top  + (_offsetDir * _offsetAmount),
					left + (_offsetDir * _offsetAmount),
					right + (_offsetDir * _offsetAmount) });
			}
			break;
		}
		}
		SetWorldTransform(backDC, &oldTransform);
	}

	void UpdateLerp(float deltaTime)
	{
		if (_lerpPos.t < 1.f)
		{
			_pos = Util::Smooth(_lerpPos.st, _lerpPos.target,
				_lerpPos .t);
			_lerpPos.t += deltaTime;
			if (_lerpPos.t > 1.f)
			{
				_pos = _lerpPos.target;
				_lerpPos.t = 1.f;
			}
		}
		if (_lerpOffset.t < 1.f)
		{
			Vector2<CoordType> offsetSize = Util::Smooth(
				_lerpOffset.st, _lerpOffset.target, _lerpOffset.t
			);
			_offsetDir = offsetSize.normalize();
			_offsetAmount = offsetSize.magnitude();
			_lerpOffset.t += deltaTime;
			if (_lerpOffset.t > 1.f)
			{
				_offsetDir = _lerpOffset.target.normalize();
				_offsetAmount = _lerpOffset.target.magnitude();
				_lerpOffset.t = 1.f;
			}
		}
		if (_lerpSize.t < 1.f)
		{
			_size = Util::Smooth(
				_lerpSize.st, _lerpSize.target, _lerpSize.t
			);
			_lerpSize.t += deltaTime;
			if (_lerpSize.t > 1.f)
			{
				_size = _lerpSize.target;
				_lerpSize.t = 1.f;
			}
		}
	}
	void Update(float deltaTime)
	{
		if (_lerpSize.t < 1.f || _lerpPos.t < 1.f || _lerpOffset.t < 1.f)
		{
			UpdateLerp(deltaTime);
		}
		if (!(std::abs(_velocityAmount - static_cast<float>(0.f))
			<= std::numeric_limits<float>::epsilon()))
			_pos += (_velocityDirection)*_velocityAmount * deltaTime;
		UpdateOffset(deltaTime);
		UpdateAngle(deltaTime);
		if (!(std::abs(_sizeDeltaAmount) - 0.f <= std::numeric_limits<float>::epsilon()))
		{
			_size += _sizeDelta * _sizeDeltaAmount * deltaTime;
		}
	}

	void UpdateAngle(float deltaTime)
	{
		if (!(std::abs(_angleVelocity - static_cast<float>(0.f))
			<= std::numeric_limits<float>::epsilon()))
			_angle += _angleVelocity * deltaTime;
		if (_angle >= 2.f * std::numbers::pi_v<float>)
			_angle -= 2.f * std::numbers::pi_v<float>;
		else if (_angle < 0.f)
			_angle += 2.f * std::numbers::pi_v<float>;
		if (!(std::abs(_offsetAngleVelocity - static_cast<float>(0.f))
			<= std::numeric_limits<float>::epsilon()))
			_offsetAngle += _offsetAngleVelocity * deltaTime;
		if (_offsetAngle >= 2.f * std::numbers::pi_v<float>)
			_offsetAngle -= 2.f * std::numbers::pi_v<float>;
		else if (_offsetAngle < 0.f)
			_offsetAngle += 2.f * std::numbers::pi_v<float>;
	}
	void UpdateOffset(float deltaTime)
	{
		switch (_trailType)
		{
		case TrailType::Circle:
			_offsetDir = Vector2<float>{ std::cos(_offsetAngle),std::sin(_offsetAngle) };
			break;
		case TrailType::Triangle:
		{
			const float pi = std::numbers::pi_v<float>;
			Vector2<float> v[3] = { 
				Vector2<float>{0,-1}, 
				Vector2<float>{1,1},
				Vector2<float>{-1,1}
			};


			float lens[3];
			float total = 0;
			for (int i = 0; i < 3; i++) {
				Vector2<float> d = Vector2<float>{ v[(i + 1) % 3].x - v[i].x,
									 v[(i + 1) % 3].y - v[i].y };
				lens[i] = std::sqrt(d.x * d.x + d.y * d.y);
				total += lens[i];
			}

			float dist = (_offsetAngle / (2 * pi)) * total;
			dist = std::fmod(dist, total);
			if (dist < 0) dist += total;

			for (int i = 0; i < 3; i++) {
				if (dist <= lens[i]) {
					float t = dist / lens[i];
					_offsetDir = Vector2<float>{
						v[i].x + (v[(i + 1) % 3].x - v[i].x) * t,
						v[i].y + (v[(i + 1) % 3].y - v[i].y) * t
					};
					break;
				}
				dist -= lens[i];
			}
			break;
		}
		case TrailType::Square:
		{
			float sinOffset = std::sin(_offsetAngle);
			float cosOffset = std::cos(_offsetAngle);
			float scale = 1.0f / std::max(std::abs(sinOffset), std::abs(cosOffset));
			_offsetDir = Vector2<float>{
				cosOffset * scale,
				sinOffset * scale
			};
			break;
		}
		}
	}

	DebugInfo PrintDebug() const
	{
		return DebugInfo{ _type, _trailType,_pos,
			_offsetDir * _offsetAmount,
		Vector2<CoordType>{_velocityDirection* _velocityAmount},
		Vector2<CoordType>
		{_offsetVelocityDirection*
		_offsetVelocityAmount},
		_size, _angle };
	}

	void SetFillColor(DefaultColors color)
	{
		_oldFillColor = _fillColor;
		_fillColor = color;
	}

	void RevertFillColor()
	{
		std::swap(_oldFillColor, _fillColor);
	}

	void SetPenColor(DefaultColors color)
	{
		_oldPenColor = _penColor;
		_penColor = color;
	}

	void RevertPenColor()
	{
		std::swap(_oldPenColor, _penColor);
	}

	void SetPenThickness(int newThickness)
	{
		if (newThickness > 10) return;
		_thickness = newThickness;
	}

	DefaultColors GetFillColor() const noexcept { return _fillColor; }
	DefaultColors GetPenColor() const noexcept { return _penColor; }
	int GetPenThickness() const noexcept { return _thickness; }


	void FreeObject()
	{
		_trailType = TrailType::None;
		_velocityDirection = _offsetDir;
		_velocityAmount = 4000.f * (1.f + (_offsetAngleVelocity / 6.f));
	}
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
	DefaultColors _oldFillColor;
	DefaultColors _oldPenColor;
	int _thickness;

	ShapeType _type;
	TrailType _trailType;
	// 중심
	Vector2<CoordType> _pos;
	Vector2<CoordType> _velocityDirection;
	float _velocityAmount;

	// 궤도 
	Vector2<CoordType> _offsetDir;
	float _offsetAmount;
	Vector2<CoordType> _offsetVelocityDirection;
	float _offsetVelocityAmount;

	Vector2<CoordType> _sizeDelta;
	Vector2<CoordType> _size;
	float _sizeDeltaAmount;

	//각도
	float _angle;
	float _angleVelocity;

	//궤도 각도
	float _offsetAngle;
	float _offsetAngleVelocity;

	// Lerp 플래그
	struct LerpTarget
	{
		Vector2<float> st;
		Vector2<float> target;
		float t;
	};
	LerpTarget _lerpPos;
	LerpTarget _lerpOffset;
	LerpTarget _lerpSize;
	// 테두리만
	bool _onlyEdge;
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
		if (obj._fillColor == DefaultColors::Transparent)
			return CollideEdge(obj, pt);
		Vector2<CoordType> ptPrime{ pt - (obj._pos + (obj._offsetDir * obj._offsetAmount)) };
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
	static bool CollideEdge(const Object<CoordType>& obj,
		const Vector2<CoordType>& pt)
	{
		Vector2<CoordType> ptPrime{ pt - (obj._pos + (obj._offsetDir * obj._offsetAmount)) };
		Vector2<CoordType> ptConv{
				std::cos(obj._angle * -1.f) * ptPrime.x
				- std::sin(obj._angle * -1.f) * ptPrime.y,
				std::sin(obj._angle * -1.f) * ptPrime.x
				+ std::cos(obj._angle * -1.f) * ptPrime.y
		};
		switch (obj._type)
		{
		case ShapeType::Circle:
		{
			float eps = (ptConv.x * ptConv.x) / (obj._size.x * obj._size.x)
				+ (ptConv.y * ptConv.y) / (obj._size.y * obj._size.y) - 1.f;
			float gx = 2 * (ptConv.x) / (obj._size.x * obj._size.x);
			float gy = 2 * (ptConv.y) / (obj._size.y * obj._size.y);

			float gradMag = std::sqrt(gx * gx + gy * gy);
			float d = fabsf(eps) / gradMag;

			return d < obj._thickness * 0.5f; 
		}

		case ShapeType::Square:
		{
			return (ptConv.x == obj._size.x * -1 && ptConv.x == obj._size.x
				&& ptConv.y == obj._size.y * -1 && ptConv.y == obj._size.y);
		}
		case ShapeType::Triangle:
		{
			Vector2<CoordType> top{
				0.f,  (-2 / 3.f) * obj._size.y };
			Vector2<CoordType> left{ -0.5f * obj._size.x,
				 +(1 / 3.f) * obj._size.y };
			Vector2<CoordType> right{ 0.5f * obj._size.x,
				 +(1 / 3.f) * obj._size.y };
			CoordType d[3] = {
				Vector2<CoordType>{left - top}.cross
				(Vector2<CoordType>{ptConv - top}),
				Vector2<CoordType>{right - left}.cross
				(Vector2<CoordType>{ptConv - left}),
				Vector2<CoordType>{top - right}.cross
				(Vector2<CoordType>{ptConv - right})
			};
			return (d[0] == 0.f && d[1] == 0.f && d[2] == 0.f);
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
		{
			Vector2<CoordType> offset{ obj._offsetDir * obj._offsetAmount };
			return (obj._pos.x + offset.x - obj._size.x) < 0.f ||
				(obj._pos.x + offset.x + obj._size.x) > windowRect.right ||
				(obj._pos.y + offset.y - obj._size.y) < 0.f ||
				(obj._pos.y + offset.y + obj._size.y) > windowRect.bottom;
		}
		case ShapeType::Square:
		{
			Vector2<CoordType> offset{ obj._offsetDir * obj._offsetAmount };
			CoordType cx = obj._pos.x + offset.x;
			CoordType cy = obj._pos.y + offset.y;
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
			Vector2<CoordType> offset{ obj._offsetDir * obj._offsetAmount };
			CoordType cx = obj._pos.x + offset.x;
			CoordType cy = obj._pos.y + offset.y;
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