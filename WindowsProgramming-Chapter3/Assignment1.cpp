#include "pch.h"
#include "Assignment1.h"
#include <format>

Assignment1::Assignment1(HINSTANCE hInstance, const Vector2<LONG>& size)
	: _hWnd{}, _winSize{},_resolution{size},_scale{1.0f,1.0f},
	_hDC{}, _backDC{}, _dibDC{}, _dibBitmap{},
	_bitmap {}, _infoFont{}, _statusFont{},
	_msg{},
	_deltaTime{ 0.f }, _frameTime{ 0.f },
	_fps{ 0 }, _frameCount{ 0 },
	_frequency{}, _count{},
	_gridCellPart{}, _gridSize{}, _gridOption{std::nullopt},
	_mousePos{},
	_isGrid{}, _gridPens{},
	_isDebug{}, _debugSelectedObject{ nullptr }, _debugPosQueue{},
	_colorPalettes{
		RGB(0,0,0),
		RGB(128,128,128),
		RGB(255,255,255),
		RGB(255,0,0),
		RGB(255,127,0),
		RGB(255,255,0),
		RGB(127,255,0),
		RGB(0,255,0),
		RGB(0,255,127),
		RGB(0,255,255),
		RGB(127,0,255),
		RGB(255,0,255)
	}
{
	if(_resolution.x <= 0 ||
		_resolution.y <= 0) {
		return;
	}
	WNDCLASSEXW wndClass{};
	wndClass.cbSize = sizeof(WNDCLASSEXW);
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = L"Assignment1";
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = Assignment1::WndProcEntry;

	RegisterClassExW(&wndClass);
	RECT clientRect{ 0,0,size.x, size.y };
	AdjustWindowRectEx(&clientRect, WS_OVERLAPPEDWINDOW,
		FALSE, NULL);
	_hWnd = CreateWindowExW(NULL, L"Assignment1", L"La App",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		NULL, FALSE,
		hInstance,
		this);

	QueryPerformanceFrequency(&_frequency);
	QueryPerformanceCounter(&_count);
}

Assignment1::~Assignment1()
{
	DeleteObject(_infoFont);
	DeleteObject(_statusFont);
	if (_isGrid) UnSetGrid();
	for (auto& brush : _brushes)
		DeleteObject(brush);
	for (auto& thicknessPens : _pens)
	{
		for (auto& pen : thicknessPens)
		{
			DeleteObject(pen);
		}
	}
	DeleteObject(_dibBitmap);
	DeleteObject(_bitmap);
	DeleteDC(_dibDC);
	DeleteDC(_backDC);
	ReleaseDC(_hWnd, _hDC);
}

void Assignment1::SetGrid(const Vector2<int>& gridSize, int gridThickness,
	DefaultColors gridColor, std::optional<GridOption> option)
{
	if (_isGrid) return;
	_gridSize = gridSize;
	_gridColor = gridColor;
	if(option.has_value()){
		_gridOption = option;
		_gridCellPart = Vector2<float>{
			(option->gridRegion.right - option->gridRegion.left)
			/ static_cast<float>(_gridSize.x),
			(option->gridRegion.bottom - option->gridRegion.top)
			/ static_cast<float>(_gridSize.x)
		};
	}
	else
	{
		_gridCellPart = Vector2<float>{
		_winSize.right / static_cast<float>(_gridSize.x),
		_winSize.bottom / static_cast<float>(_gridSize.y) };
	}
	_isGrid = true;
	for (auto& color : _colorPalettes)
	{
		_gridPens[color] = CreatePen(PS_SOLID,
			gridThickness, color);
	}
}

void Assignment1::UnSetGrid()
{
	if (!_isGrid) return;
	if (_gridOption.has_value()) _gridOption.reset();
	for (const auto& [key,pen] : _gridPens)
		DeleteObject(pen);
}

void Assignment1::Init(int nCmdShow)
{
	ShowWindow(_hWnd, nCmdShow);
	_hDC = GetDC(_hWnd);
	_backDC = CreateCompatibleDC(_hDC);
	RECT clientSize;
	GetClientRect(_hWnd, &clientSize);
	_bitmap = CreateCompatibleBitmap(_hDC, clientSize.right, clientSize.bottom);
	SelectObject(_backDC, _bitmap);

	_dibDC = CreateCompatibleDC(_backDC);
	BITMAPINFO bitmapInfo{};
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = _winSize.right;
	bitmapInfo.bmiHeader.biHeight = _winSize.bottom * -1;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	void* bits;
	_dibBitmap = CreateDIBSection(_backDC, &bitmapInfo, DIB_RGB_COLORS,
		&bits, NULL, 0);
	SetGraphicsMode(_dibDC, GM_ADVANCED);
	SelectObject(_dibDC, _dibBitmap);

	_pxs = reinterpret_cast<UINT32*>(bits);

	int colorEd{ static_cast<int>(DefaultColors::End) };
	for (int i = 0; i < colorEd; ++i)
		_brushes.push_back(CreateSolidBrush(_colorPalettes[i]));
	for (int i = 0; i < colorEd; ++i)
	{
		for (int j = 0; j < g_maxThickness;++j)
		{
			_pens[i][j] = CreatePen(PS_SOLID, j + 1,
				_colorPalettes[i]);
		}
	}


	_statusFont = CreateFontW(NULL, 12, NULL, NULL, FW_SEMIBOLD,
		FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH,
		L"Noto Sans KR");
	_infoFont = CreateFontW(NULL, 10, NULL, NULL, FW_SEMIBOLD,
		FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH,
		L"Noto Sans KR");
	PatBlt(_dibDC, 0, 0, _winSize.right, _winSize.bottom, WHITENESS);
}

void Assignment1::Update()
{
	LARGE_INTEGER currentCount;
	QueryPerformanceCounter(&currentCount);
	_deltaTime = (currentCount.QuadPart - _count.QuadPart) 
		/ static_cast<double>(_frequency.QuadPart);
	_count = currentCount;
	_frameTime += _deltaTime;
	_frameCount++;

	if (_frameTime >= 0.5f)
	{
		if (!_debugPosQueue.empty())
			_debugPosQueue.pop_front();
	}
	if (_frameTime >= 1.f)
	{
		_fps = static_cast<std::uint32_t>(_frameCount);
		_frameTime = 0.f;
		_frameCount = 0;
	}

	for (auto& object : _objects)
	{
		object.Update(_deltaTime);
		ContactBorder(object);
	}
}

void Assignment1::Render()
{
	for (int i = 0; i < _winSize.right * _winSize.bottom; ++i)
	{
		UINT32 bg{ _pxs[i] };
		UINT32 bgR{ (bg >> 16) & 0xFF }, bgG{ (bg >> 8) & 0xFF }, bgB{ bg & 0xFF };

		UINT32 newBgR{ static_cast<UINT32>(bgR + (255 - bgR) * 0.05f) };
		UINT32 newBgG{ static_cast<UINT32>(bgG +(255 - bgG) * 0.05f) };
		UINT32 newBgB{ static_cast<UINT32>(bgB + (255 - bgB) * 0.05f) };
		if (bgR - newBgR == 0 &&
			bgG - newBgG == 0 &&
			bgB - newBgB == 0)
		{
			bgR = 255; bgG = 255; bgB = 255;
		}
		else
		{
			bgR = newBgR; bgG = newBgG; bgB = newBgB;
		}
		_pxs[i] = (bgR << 16) | (bgG << 8) | bgB;
	}
	// Grid Layer
	// TODO : Add More Grids
	std::optional<Vector2<float>> gridCellPt = std::nullopt;
	if (_isGrid)
	{
		HPEN old = (HPEN)SelectObject(_dibDC,
			_gridPens[GetPaletteColor(_gridColor)]);
		if (_gridOption.has_value())
		{
			DrawGrid(Vector2<LONG>{
				static_cast<LONG>(_gridOption->gridRegion.left *_scale.x),
				static_cast<LONG>(_gridOption->gridRegion.top *_scale.y)});
		}
		else
			DrawGrid();
		SelectObject(_dibDC, old);
		gridCellPt = _gridCellPart;
	}
	// Object Layer
	for (auto& object : _objects)
	{
		HPEN oldPen = (HPEN)SelectObject(_dibDC,
			_pens[std::to_underlying(object.GetPenColor())]
			[object.GetPenThickness() - 1]);
		HBRUSH oldBrush = (HBRUSH)SelectObject(_dibDC,
			_brushes[std::to_underlying(object.GetFillColor())]);
		object.Render(_dibDC, _scale, gridCellPt);
		SelectObject(_dibDC, oldPen);
		SelectObject(_dibDC, oldBrush);

		if (_isDebug)
		{
			HFONT oldFont = (HFONT)SelectObject(_dibDC, _statusFont);
			if (_winSize.bottom > 100 && _winSize.right > 10) 
			{
				auto Fps = std::format(L"FPS : {}", _fps);
				auto DeltaTime = std::format(L"DeltaTime : {:.2f}ms", _deltaTime * 1000);

				TextOutW(_dibDC, 10, _winSize.bottom - 100, Fps.c_str(), Fps.size());
				TextOutW(_dibDC, 10, _winSize.bottom - 70, DeltaTime.c_str(), DeltaTime.size());
			}
			if(CollideHelper::Collide(object,_mousePos))
			{
				object.SetPenColor(DefaultColors::Lime);
				object.SetPenThickness(3);
				_debugSelectedObject = &object;
				auto pos = object.GetPos(); 
				auto offsetPos = object.GetOffsetPos();
			}
			else if (object.GetPenColor() == DefaultColors::Lime)
			{
				object.SetPenColor(DefaultColors::Black);
				object.SetPenThickness(1);
			}
			SelectObject(_dibDC, oldFont);
		}
	}
	// debug Layer
	if (_isDebug && _debugSelectedObject != nullptr)
	{
		if (_winSize.bottom > 140 && _winSize.right > 220) {
			SelectObject(_dibDC, _infoFont);
			const auto& debugInfo = _debugSelectedObject->PrintDebug();
			std::wstring shape{ L"Shape type : " };
			std::wstring trail{ L"Trail type : " };
			switch (debugInfo.type)
			{
			case ShapeType::Circle:
				shape += L"Circle";
				break;
			case ShapeType::Square:
				shape += L"Square";
				break;
			case ShapeType::Triangle:
				shape += L"Triangle";
				break;
			}
			switch (debugInfo.trailType)
			{
			case TrailType::None:
				trail += L"None";
				break;
			case TrailType::Circle:
				trail += L"Circle";
				break;
			case TrailType::Square:
				trail += L"Square";
				break;
			case TrailType::Triangle:
				trail += L"Triangle";
				break;
			}
			TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 220, shape.c_str(), shape.size());
			TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 194, trail.c_str(), trail.size());
			auto Pos = std::format(L"Position {{{:.3f}, {:.3f}}}", debugInfo.pos.x, debugInfo.pos.y);
			TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 168, Pos.c_str(), Pos.size());
			auto Offset = std::format(L"Offset Position : {{{:.3f}, {:.3f}}}", debugInfo.offsetPos.x, debugInfo.offsetPos.y);
			TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 142, Offset.c_str(), Offset.size());
			auto Vel = std::format(L"Velocity {{{:.3f}, {:.3f}}}", debugInfo.velocity.x, debugInfo.velocity.y);
			TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 116, Vel.c_str(), Vel.size());
			auto OffsetVel = std::format(L"Offset Velocity : {{{:.3f}, {:.3f}}}", debugInfo.offsetVelocity.x, debugInfo.offsetVelocity.y);
			TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 90, OffsetVel.c_str(), OffsetVel.size());
			auto Size = std::format(L"Size : {{{:.3f}, {:.3f}}}", debugInfo.size.x, debugInfo.size.y);
			TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 64, Size.c_str(), Size.size());
			auto Angle = std::format(L"Angle : {:.3f}", debugInfo.angle);
			TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 38, Angle.c_str(), Angle.size());
			_debugPosQueue.push_back(Vector2<int>{
				static_cast<int>(debugInfo.pos.x + debugInfo.offsetPos.x),
					static_cast<int>(debugInfo.pos.y + debugInfo.offsetPos.y)});
			auto magSize = static_cast<size_t>(
				debugInfo.velocity.magnitude());
			magSize = (magSize > 200) ? 200 : magSize;
			if (_debugPosQueue.size() > magSize)
				_debugPosQueue.pop_front();
			int posQueueSize = _debugPosQueue.size();
			for (int i = 0; i < posQueueSize; ++i)
			{
				if (_debugPosQueue[i].x < 0 ||
					_debugPosQueue[i].y < 0 ||
					_debugPosQueue[i].y >= _winSize.bottom ||
					_debugPosQueue[i].x >= _winSize.right)
					continue;
				UINT32 bg{ _pxs[_debugPosQueue[i].y * _winSize.right + _debugPosQueue[i].x] };
				UINT32 bgR{ (bg >> 16) & 0xFF }, bgG{ (bg >> 8) & 0xFF }, bgB{ bg & 0xFF };
				float alpha = static_cast<float>(i) / posQueueSize;
				UINT32 r{ static_cast<UINT32>(bgR * (1.f - alpha)) };
				UINT32 g{ static_cast<UINT32>(bgG * (1.f - alpha) + 255 * (alpha)) };
				UINT32 b{ static_cast<UINT32>(bgB * (1.f - alpha)) };
				_pxs[_debugPosQueue[i].y * _winSize.right + _debugPosQueue[i].x]
					= (r << 16) | (g << 8) | b;
			}
		}
	}
	BitBlt(_hDC, 0, 0, _winSize.right, _winSize.bottom,
		_dibDC, 0, 0, SRCCOPY);
}

int Assignment1::Run()
{
	while (true)
	{
		while (PeekMessageW(&_msg, NULL, NULL, NULL, PM_REMOVE))
		{
			if (_msg.message == WM_QUIT)
				return static_cast<int>(_msg.wParam);
			TranslateMessage(&_msg);
			DispatchMessageW(&_msg);
		}
		Update();
		Render();
	}
}

LRESULT Assignment1::WndProcEntry(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_CREATE)
	{
		SetWindowLongPtrW(hWnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(reinterpret_cast<LPCREATESTRUCTW>
				(lParam)->lpCreateParams));
	}
	else if (Msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	else if (Assignment1* pThis = reinterpret_cast<Assignment1*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)))
	{
		return pThis->WndProc(hWnd, Msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

LRESULT Assignment1::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_SIZE:
	{
		GetClientRect(hWnd, &_winSize);
		_scale.x = _winSize.right / static_cast<float>(_resolution.x);
		_scale.y = _winSize.bottom / static_cast<float>(_resolution.y);
		if (_winSize.right == 0 || _winSize.bottom == 0) break;
		DeleteObject(_bitmap);
		_bitmap = CreateCompatibleBitmap(_hDC, _winSize.right, _winSize.bottom);
		SelectObject(_backDC, _bitmap);
		if (_isGrid) {
			UpdateGrid();
		}

		DeleteObject(_dibBitmap);
		BITMAPINFO bitmapInfo{};
		bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfo.bmiHeader.biWidth = _winSize.right;
		bitmapInfo.bmiHeader.biHeight = _winSize.bottom * -1;
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biBitCount = 32;
		void* bits;
		_dibBitmap = CreateDIBSection(_backDC, &bitmapInfo, DIB_RGB_COLORS,
			&bits, NULL, 0);
		SelectObject(_dibDC, _dibBitmap);
		PatBlt(_dibDC, 0, 0, _winSize.right, _winSize.bottom, WHITENESS);
		_pxs = reinterpret_cast<UINT32*>(bits);
		Update();
		Render();
		break;
	}
	case WM_KEYDOWN:
	{
		if ((LOWORD(lParam) & 0xFFFF) > 1)
			break;
		switch (wParam)
		{
		case VK_F1:
			ToggleDebug();
			break;
		case 'R':
			_shapeType = (_shapeType == ShapeType::Square) ?
				ShapeType::None : ShapeType::Square;
			break;
		case 'E':
			_shapeType = (_shapeType == ShapeType::Circle) ?
				ShapeType::None : ShapeType::Circle;
			break;
		case 'T':
			_shapeType = (_shapeType == ShapeType::Triangle) ?
				ShapeType::None : ShapeType::Triangle;
			break;
		case 'Q':
			DestroyWindow(hWnd);
			break;
		case 'H':
			if (_objects.empty()) break;
			if (_objects.front().GetVelocity().nearZero())
			{
				_objects.front().SetVelocity(Vector2<float>(50.f, 0.f));
				_objects.front().SetAngleVelocity(1.f);
			}
			else
				_objects.front().SetVelocityDir(Vector2<float>{1.f, 0.f});
			break;
		case 'V':
			if (_objects.empty()) break;
			if (_objects.front().GetVelocity().nearZero())
			{
				_objects.front().SetVelocity(Vector2<float>(0.f, 50.f));
				_objects.front().SetAngleVelocity(1.f);
			}
			else
				_objects.front().SetVelocityDir(Vector2<float>{0.f, 1.f});
			break;
		case 'S':
		{
			if (_objects.empty()) break;
			std::random_device rd;
			std::uniform_real_distribution<float> angleDist{ 0.1f,0.9f };
			float firstAngle = angleDist(rd);
			if (_objects.front().GetVelocity().nearZero())
			{
				_objects.front().SetVelocity(Vector2<float>{firstAngle, std::sqrt(1.0f - firstAngle * firstAngle)} *50.f);
				_objects.front().SetAngleVelocity(1.f);
			}
			else
				_objects.front().SetVelocityDir(Vector2<float>{firstAngle, std::sqrt(1.0f - firstAngle * firstAngle)});
			break;
		}
		case 'P':
			if (_objects.empty()) break;
			_objects.front().SetVelocityAmount(0.f);
			_objects.front().SetAngleVelocity(0.f);
			break;
		case '1': case VK_OEM_1:
			if (_objects.empty()) break;
			_objects.front().AddAngleVelocity(0.1f);
			break;
		case '2':case VK_OEM_2:
			if (_objects.empty()) break;
			_objects.front().AddAngleVelocity(-0.1f);
			break;
		case '3':case VK_OEM_3:
			if (_objects.empty()) break;
			_objects.front().SetSizeDelta(Vector2<float>{1.0f, 0.f});
			break;
		case '4':case VK_OEM_4:
			if (_objects.empty()) break;
			_objects.front().SetSizeDelta(Vector2<float>{0.f, 1.f});
			break;
		case '5':case VK_OEM_5:
			if (_objects.empty()) break;
			_objects.front().SetSizeDelta(Vector2<float>{-1.0f, 0.f});
			break;
		case '6':case VK_OEM_6:
			if (_objects.empty()) break;
			_objects.front().SetSizeDelta(Vector2<float>{0.f, -1.f});
			break;
		case '7':case VK_OEM_7:
			if (_objects.empty()) break;
			_objects.front().AddSizeDeltaAmount(1.f);
			break;
		case '8':case VK_OEM_8:
			if (_objects.empty()) break;
			_objects.front().AddSizeDeltaAmount(-1.f);
			break;
		case '9':case VK_OEM_102:
			if (_objects.empty()) break;
			_objects.front().SetSizeDeltaAmount(0.f);
			break;
		case VK_OEM_PLUS:
			if (_objects.empty()) break;
			_objects.front().AddVelocityAmount(10.f);
			break;
		case VK_OEM_MINUS:
			if (_objects.empty()) break;
			if (_objects.front().GetVelocity().nearZero()) break;
			_objects.front().AddVelocityAmount(-10.f);
			break;
		}
		break;
	}
	case WM_LBUTTONUP:
	{
		if (_shapeType == ShapeType::None) break;
		auto&& obj = Object<float>{_shapeType, _mousePos,
			Vector2<float>{30.f, 30.f}};
		if (_shapeType == ShapeType::Triangle)
			obj.SetSize(obj.GetSize() * 2.f);
		if (_objects.empty())
			_objects.push_back(obj);
		else
			_objects.front() = obj;
		break;
	}
	case WM_MOUSEMOVE:
	{
		_mousePos = Vector2<int>{ LOWORD(lParam),HIWORD(lParam) };
		break;
	}
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

void Assignment1::ContactBorder(Object<float>& assignment1Obj)
{
	if (CollideHelper::CollideScreenBorder(assignment1Obj, _winSize))
	{
		auto pos = assignment1Obj.GetPos();
		auto size = assignment1Obj.GetSize();
		auto velDir = assignment1Obj.GetVelocityDir();
		if (std::abs(assignment1Obj.GetVelocity().y - 0.f) < std::numeric_limits<float>::epsilon()
			&& std::abs(assignment1Obj.GetVelocity().x - 0.f) < std::numeric_limits<float>::epsilon())
		{
			assignment1Obj.SetSizeDeltaAmount(0.f);
		}
		else if (std::abs(assignment1Obj.GetVelocity().y - 0.f) < std::numeric_limits<float>::epsilon())
		{
			pos.x += velDir.x * size.x * -1;
			pos.y += size.y;
			if (pos.y >= static_cast<float>(_winSize.bottom))
				assignment1Obj.SetPos(Vector2<float>{size.x, size.y});
			else
				assignment1Obj.SetPos(pos);
			velDir = velDir * -1.f;
		}
		else if (std::abs(assignment1Obj.GetVelocity().x - 0.f) < std::numeric_limits<float>::epsilon())
		{
			pos.x += size.x;
			pos.y += velDir.y * size.y * -1;
			assignment1Obj.SetPos(pos);
			if (pos.x >= static_cast<float>(_winSize.right))
				assignment1Obj.SetPos(Vector2<float>{size.x, size.y});
			else
				assignment1Obj.SetPos(pos);
			velDir = velDir * -1.f;
		}
		else
		{
			velDir.rotate(std::numbers::pi_v<float> / 2.f);
			pos.x += velDir.x * size.x;
			pos.y += velDir.y * size.y;
			assignment1Obj.SetPos(pos);
			
		}
		assignment1Obj.SetVelocityDir(velDir);
	}
}

void Assignment1::DrawGrid(const Vector2<LONG>& regionSt)
{
	for (int y = 0; y < _gridSize.y; ++y)
	{
		for (int x = 0; x < _gridSize.x; ++x)
		{
			if (_isDebug && _mousePos.x > x * _gridCellPart.x 
				+ regionSt.x &&
				_mousePos.x <= (x + 1) * _gridCellPart.x  + regionSt.x &&
				_mousePos.y > y * _gridCellPart.y +regionSt.y &&
				_mousePos.y <= (y + 1) * _gridCellPart.y + regionSt.y)
			{
				HPEN old = (HPEN)SelectObject(_dibDC,
					_gridPens[GetPaletteColor(DefaultColors::Lime)]);
				RenderUtil::DrawRect(_dibDC,
					Vector2{ x * _gridCellPart.x
					+ regionSt.x , y * _gridCellPart.y +regionSt.y},
					_gridCellPart.x, _gridCellPart.y);
				SelectObject(_dibDC, old);
			}
			else
			{
				RenderUtil::DrawRect(_dibDC,
					Vector2{ x * _gridCellPart.x + regionSt.x,
					y * _gridCellPart.y + regionSt.y },
					_gridCellPart.x, _gridCellPart.y);
			}
		}
	}
}

void Assignment1::UpdateGrid()
{
	if (_gridOption.has_value())
	{
		if (_gridOption->scalable)
		{
			_gridCellPart = Vector2<float>{
			(_gridOption->gridRegion.right - _gridOption->gridRegion.left)
			* _scale.x / static_cast<float>(_gridSize.x),
			(_gridOption->gridRegion.bottom - _gridOption->gridRegion.top)
			* _scale.y / static_cast<float>(_gridSize.y) };
		}
	}
	else
	{
		_gridCellPart = Vector2<float>{
		_winSize.right / static_cast<float>(_gridSize.x),
		_winSize.bottom / static_cast<float>(_gridSize.y) };
	}
}