#include "pch.h"
#include "Assignment2.h"
#include "resources_3_2.h"
#include <format>

Assignment2::Assignment2(HINSTANCE hInstance, const Vector2<LONG>& size)
	: _hWnd{}, _winSize{}, _resolution{ size }, _scale{ 1.0f,1.0f },
	_hDC{}, _dibDC{}, _dibBitmap{},
	_bitmap{}, _infoFont{}, _statusFont{},
	_msg{},
	_deltaTime{ 0.f }, _frameTime{ 0.f },
	_fps{ 0 }, _frameCount{ 0 },
	_frequency{}, _count{},
	_gridCellPart{}, _gridSize{}, _gridOption{ std::nullopt },
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
		RGB(0,127,255),
		RGB(0,0,255),
		RGB(255,0,255),
		RGB(127,0,255)
	}, _selectedArea{ -1 }, _isHolding{ false }, _gameRun{ false }
{
	if (_resolution.x <= 0 ||
		_resolution.y <= 0) {
		return;
	}
	WNDCLASSEXW wndClass{};
	wndClass.cbSize = sizeof(WNDCLASSEXW);
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = L"Assignment2";
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = Assignment2::WndProcEntry;
	wndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);

	RegisterClassExW(&wndClass);
	RECT clientRect{ 0,0,size.x, size.y };
	AdjustWindowRectEx(&clientRect, WS_OVERLAPPEDWINDOW,
		FALSE, NULL);
	_hWnd = CreateWindowExW(NULL, L"Assignment2", L"La App",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		NULL, FALSE,
		hInstance,
		this);

	QueryPerformanceFrequency(&_frequency);
	QueryPerformanceCounter(&_count);
}

Assignment2::~Assignment2()
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

void Assignment2::SetGrid(const Vector2<int>& gridSize, int gridThickness,
	DefaultColors gridColor, std::optional<GridOption> option)
{
	if (_isGrid) return;
	_gridSize = gridSize;
	_gridColor = gridColor;
	if (option.has_value()) {
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

void Assignment2::UnSetGrid()
{
	if (!_isGrid) return;
	if (_gridOption.has_value()) _gridOption.reset();
	for (const auto& [key, pen] : _gridPens)
		DeleteObject(pen);
}

void Assignment2::Init(int nCmdShow)
{
	_hDC = GetDC(_hWnd);

	_dibDC = CreateCompatibleDC(_hDC);

	ResizeScreen();

	int colorEd{ static_cast<int>(DefaultColors::Transparent)};
	for (int i = 0; i < colorEd; ++i)
		_brushes.push_back(CreateSolidBrush(_colorPalettes[i]));
	_brushes.push_back((HBRUSH)GetStockObject(NULL_BRUSH));
	for (int i = 0; i < colorEd; ++i)
	{
		for (int j = 0; j < g_maxThickness;++j)
		{
			_pens[i][j] = CreatePen(PS_SOLID, j + 1,
				_colorPalettes[i]);
		}
	}
	for (int j = 0; j < g_maxThickness; ++j)
	{
		_pens[colorEd][j] = (HPEN)GetStockObject(NULL_PEN);
	}
	_statusFont = CreateFontW(NULL, 12, NULL, NULL, FW_SEMIBOLD,
		FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH,
		L"Noto Sans KR");
	_infoFont = CreateFontW(NULL, 10, NULL, NULL, FW_SEMIBOLD,
		FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH,
		L"Noto Sans KR");

	for (auto& part : _parts)
	{
		part._centre.push_back(Object<float>{});
		part._centre.back().SetShapeType(ShapeType::Circle);
		part._centre.back().SetPenThickness(2);
		part._centre.back().SetFillColor(DefaultColors::Red);
		part._centre.back().SetSize(Vector2<float>{5.f, 5.f});
		part._centre.push_back(Object<float>{});
		part._centre.back().SetShapeType(ShapeType::Circle);
		part._centre.back().SetPenThickness(1);
		part._centre.back().SetPenColor(DefaultColors::Purple);
		part._centre.back().SetFillColor(DefaultColors::Transparent);
		part._centre.back().SetSize(Vector2<float>{100.f, 100.f});
		part.trailType = TrailType::Circle;
		_objCount += 2;
	}
	ShowWindow(_hWnd, nCmdShow);
}

void Assignment2::Update()
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
		auto partSize = _parts.size();
		for (int i = 0; i < partSize; ++i)
		{
			if (_parts[i].tickCount > 0)
			{
				_parts[i].tickCount--;
				if (_parts[i].tickCount == 0)
				{
					_objCount -= _parts[i]._particles.size();
					_parts[i]._particles.clear();
					_parts[i]._centre[0].SetShapeType(ShapeType::Circle);
					_parts[i]._centre[1].SetShapeType(ShapeType::Circle);
					_parts[i]._centre[1].SetSize(Vector2<float>{100.f, 100.f});
					_parts[i].trailType = TrailType::Circle;
					if (_isHolding)
					{
						Vector2<float> mid{ _winSize.right / 2.f, _winSize.bottom / 2.f };
						for (auto& centre : _parts[i]._centre)
						{
							centre.SetPos(Vector2<float>{
								mid.x / 2.f + mid.x * (i % (partSize / 2)),
									mid.y / 2.f + mid.y * (i / (partSize / 2))
							});
						}
						_isHolding = false;
					}
				}
			}
		}
	}

	for (auto& part : _parts)
	{
		for (auto& centreParts : part._centre)
		{
			centreParts.Update(_deltaTime);
		}
		for (auto& particle : part._particles)
		{
			particle.Update(_deltaTime);
			if (particle.GetPos() == part._centre.front().GetPos())
			{
				if (particle.GetOffsetPos().nearZero()) {
					part._centre[1].SetSizeLerp(
						part._centre[1].GetSize() +
						Vector2<float>{-5.f, -5.f}
					);
					particle.SetOffsetLerp(Vector2<float>{0, part._centre[1].GetSize().y});
				}
				if(particle.isOffsetLerpDone()){
					if(particle.GetTrailType() != part.trailType)
						particle.SetTrailType(part.trailType);
					particle.SetOffsetAmount(part._centre[1].GetSize().x);
				}
			}
			ContactBorder(particle);
		}
	}
}

void Assignment2::ResizeScreen()
{
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

	PatBlt(_dibDC, 0, 0, _winSize.right, _winSize.bottom, WHITENESS);
	_pxs = reinterpret_cast<UINT32*>(bits);
}

void Assignment2::Render()
{
	// Grid Layer
	// TODO : Add More Grids
	PatBlt(_dibDC, 0, 0, _winSize.right, _winSize.bottom, WHITENESS);
	std::optional<Vector2<float>> gridCellPt = std::nullopt;
	if (_isGrid)
	{
		HPEN old = (HPEN)SelectObject(_dibDC,
			_gridPens[GetPaletteColor(_gridColor)]);
		if (_gridOption.has_value())
		{
			DrawGrid(Vector2<LONG>{
				static_cast<LONG>(_gridOption->gridRegion.left* _scale.x),
					static_cast<LONG>(_gridOption->gridRegion.top* _scale.y)});
		}
		else
			DrawGrid();
		SelectObject(_dibDC, old);
		gridCellPt = _gridCellPart;
	}
	// Object Layer
	for (auto& part : _parts)
	{
		for (auto& centre : part._centre)
		{
			if (!centre.GetSize().nearZero())
				RenderObject(centre);
		}
		for (auto& particle : part._particles)
		{
			if (!particle.GetSize().nearZero())
				RenderObject(particle);
		}
	}
	// debug Layer

	if ((_isDebug & DEBUG_STATUS) && _winSize.bottom > 130
		&& _winSize.right > 10)
	{
		SelectObject(_dibDC, _statusFont);
		auto Fps = std::format(L"FPS : {}", _fps);
		auto DeltaTime = std::format(L"DeltaTime : {:.2f}ms", _deltaTime * 1000);
		auto Count = std::format(L"Objects : {}", _objCount);
		TextOutW(_dibDC, 10, _winSize.bottom - 130, Fps.c_str(), Fps.size());
		TextOutW(_dibDC, 10, _winSize.bottom - 100, DeltaTime.c_str(), DeltaTime.size());
		TextOutW(_dibDC, 10, _winSize.bottom - 70, Count.c_str(), Count.size());

		std::wstring placeMode{ L"Placing Mode : " };
		switch (_shapeType)
		{
		case ShapeType::None:
			placeMode = L"Trail Acceleration Mode";
			break;
		case ShapeType::Circle:
			placeMode += L"Circle";
			break;
		case ShapeType::Square:
			placeMode += L"Square";
			break;
		case ShapeType::Triangle:
			placeMode += L"Triangle";
			break;
		}
		TextOutW(_dibDC, 10, _winSize.bottom - 40, placeMode.c_str(), placeMode.size());
	}
	if ((_isDebug & DEBUG_OBJECT) && _debugSelectedObject != nullptr
		&& _winSize.bottom > 140 && _winSize.right > 220) {
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
		auto OffestVel = std::format(L"Offest Velocity : {{{:.3f}, {:.3f}}}", debugInfo.offsetVelocity.x, debugInfo.offsetVelocity.y);
		TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 90, OffestVel.c_str(), OffestVel.size());
		auto Size = std::format(L"Size : {{{:.3f}, {:.3f}}}", debugInfo.size.x, debugInfo.size.y);
		TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 64, Size.c_str(), Size.size());
		auto Angle = std::format(L"Angle : {:.3f}", debugInfo.angle);
		TextOutW(_dibDC, _winSize.right - 200, _winSize.bottom - 38, Angle.c_str(), Angle.size());
		_debugPosQueue.push_back(Vector2<int>{
			static_cast<int>(debugInfo.pos.x + debugInfo.offsetPos.x),
				static_cast<int>(debugInfo.pos.y + debugInfo.offsetPos.y)});
		if (_debugPosQueue.size() > 100)
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
	BitBlt(_hDC, 0, 0, _winSize.right, _winSize.bottom,
		_dibDC, 0, 0, SRCCOPY);
}

int Assignment2::Run()
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
		if(_gameRun)
		{
			Update();
			Render();
		}
	}
}

LRESULT Assignment2::WndProcEntry(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
	else if (Assignment2* pThis = reinterpret_cast<Assignment2*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)))
	{
		return pThis->WndProc(hWnd, Msg, wParam, lParam);
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

LRESULT Assignment2::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_SIZE:
	{
		GetClientRect(hWnd, &_winSize);
		_scale.x = _winSize.right / static_cast<float>(_resolution.x);
		_scale.y = _winSize.bottom / static_cast<float>(_resolution.y);
		if (_winSize.right == 0 || _winSize.bottom == 0) break;
		if (_isGrid) {
			UpdateGrid();
		}
		DeleteObject(_dibBitmap);
		ResizeScreen();
		auto partSize = _parts.size();
		Vector2<float> mid{ _winSize.right / 2.f, _winSize.bottom / 2.f };
		for (int i = 0; i < partSize; ++i)
		{
			for (auto& centre : _parts[i]._centre)
			{
				centre.SetPos(Vector2<float>{
					mid.x / 2.f + mid.x * (i % (partSize / 2)),
					mid.y / 2.f + mid.y * (i / (partSize / 2))
				});
			}

		}
		if(_gameRun){
			Update();
		}
		Render();
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_GAME_START:
		{
			_gameRun = true;
			break;
		}
		case ID_GAME_STOP:
		{
			_gameRun = false;
			break;
		}
		case ID_GAME_QUIT:
		{
			DestroyWindow(hWnd);
			break;
		}
		case ID_SELECTION_1: case ID_SELECTION_2: case ID_SELECTION_3: case ID_SELECTION_4:
		{
			SelectRegion(LOWORD(wParam) - ID_SELECTION_1);
			break;
		}
		case ID_SPEED_FAST:
		{
			if (_selectedArea == -1) break;
			for (auto& particle : _parts[_selectedArea]._particles)
			{
				particle.SetOffsetAngleVelocity(15.f);
			}
			break;
		}
		case ID_SPEED_MEDIUM:
		{
			if (_selectedArea == -1) break;
			for (auto& particle : _parts[_selectedArea]._particles)
			{
				particle.SetOffsetAngleVelocity(6.f);
			}
			break;
		}
		case ID_SPEED_SLOW:
		{
			if (_selectedArea == -1) break;
			for (auto& particle : _parts[_selectedArea]._particles)
			{
				particle.SetOffsetAngleVelocity(1.f);
			}
			break;
		}
		case ID_COLOR_CYAN:
		{
			if (_selectedArea == -1) break;
			_parts[_selectedArea]._centre[1].SetPenColor(
				DefaultColors::Skyblue
			);
			break;
		}
		case ID_COLOR_MAGENTA:
		{
			if (_selectedArea == -1) break;
			_parts[_selectedArea]._centre[1].SetPenColor(
				DefaultColors::Pink
			);
			break;
		}
		case ID_COLOR_YELLOW:
		{
			if (_selectedArea == -1) break;
			_parts[_selectedArea]._centre[1].SetPenColor(
				DefaultColors::Yellow
			);
			break;
		}
		case ID_SHAPE_CIRCLE:
		{
			if (_selectedArea == -1) break;
			_parts[_selectedArea]._centre[1].SetShapeType(
				ShapeType::Circle
			);
			_parts[_selectedArea].trailType = TrailType::Circle;
			break;
		}
		case ID_SHAPE_RECTANGLE:
		{
			if (_selectedArea == -1) break;
			_parts[_selectedArea]._centre[1].SetShapeType(
				ShapeType::Square
			);
			_parts[_selectedArea].trailType = TrailType::Square;
			break;
		}
		case ID_SHAPE_TRIANGLE:
		{
			if (_selectedArea == -1) break;
			_parts[_selectedArea]._centre[1].SetShapeType(
				ShapeType::Triangle
			);
			_parts[_selectedArea].trailType = TrailType::Triangle;
			break;
		}
		break;
		}
	}
	case WM_KEYDOWN:
	{
		if ((LOWORD(lParam) & 0xFFFF) > 1)
			break;
		switch (wParam)
		{
		case VK_F1:
			_isDebug ^= DEBUG_STATUS;
			break;
		case VK_F2:
			_isDebug ^= DEBUG_OBJECT;
			break;
		case 'A':
			_shapeType = (_shapeType == ShapeType::Circle) ?
				ShapeType::None : ShapeType::Circle;
			break;
		case 'S':
			_shapeType = (_shapeType == ShapeType::Square) ?
				ShapeType::None : ShapeType::Square;
			break;
		case 'D':
			_shapeType = (_shapeType == ShapeType::Triangle) ?
				ShapeType::None : ShapeType::Triangle;
			break;
		case 'C':
		{
			if (_selectedArea == -1) break;
			for (auto& particle : _parts[_selectedArea]._particles)
			{
				particle.SetOffsetAngleVelocity
				(particle.GetOffsetAngleVelocity() * -1.f
				);
			}
			break;
		}
		case 'M':
		{
			if (_selectedArea == -1) break;
			for (auto& particle : _parts[_selectedArea]._particles)
			{
				auto nextType = ((std::to_underlying(particle.GetShapeType()) + 1)
					% std::to_underlying(ShapeType::Triangle) + 1);
				particle.SetShapeType(
					static_cast<ShapeType>(nextType)
				);
			}
			break;
		}
		case 'R':
		{
			if (_selectedArea == -1) break;
			_parts[_selectedArea].trailType =
				(_parts[_selectedArea].trailType == TrailType::Square)
				? TrailType::Circle : TrailType::Square;
			_parts[_selectedArea]._centre[1].SetShapeType(
				(_parts[_selectedArea].trailType == TrailType::Square) ?
				ShapeType::Square : ShapeType::Circle
			);
			break;
		}
		case 'T':
		{
			if (_selectedArea == -1) break;
			_parts[_selectedArea].trailType =
				(_parts[_selectedArea].trailType == TrailType::Triangle)
				? TrailType::Circle : TrailType::Triangle;
			_parts[_selectedArea]._centre[1].SetShapeType(
				(_parts[_selectedArea].trailType == TrailType::Triangle) ?
				ShapeType::Triangle : ShapeType::Circle
			);
			break;
		}
		case '1': case '2': case '3': case '4':
			if(_parts[wParam - '1'].tickCount == 0)
				SelectRegion(wParam - '1');
			break;
		case '5':
		{
			if (_shapeType == ShapeType::None) break;
			std::uniform_real_distribution<float> size (1.f, 90.f);
			std::uniform_int_distribution shape(1, 3);
			for(int i = 0; i < _parts.size(); ++i)
			{
				if (_parts[i]._particles.size() <= 700)
				{
					_objCount++;
					_parts[i]._particles.push_back(
						Object<float>{
						static_cast<ShapeType>(shape(mt)),
							_mousePos,
							Vector2<float>{size(mt), size(mt)}
					}
					);
					_parts[i]._particles.back().SetPosLerp(
						_parts[i]._centre.front().GetPos()
					);
					_parts[i]._particles.back().SetTrailType(
						_parts[i].trailType);
					_parts[i]._particles.back().SetOffsetAngleVelocity(6.28f);
				}
				else if (_parts[i].tickCount == 0)
				{
					for (auto& particle : _parts[i]._particles)
					{
						particle.FreeObject();
					}
					if(_parts[i]._centre.size() > 2){
						_parts[i]._centre.pop_back();
					}
					_parts[i].tickCount = 5;
					_parts[i]._centre[0].SetShapeType(ShapeType::None);
					_parts[i]._centre[1].SetShapeType(ShapeType::None);
					_selectedArea = -1;
				}
			}
			break;
		}
		}
		break;
	}
	case WM_LBUTTONUP:
	{
		if (_selectedArea == -1) break;
		if (_shapeType == ShapeType::None) {
			for (auto& particle : _parts[_selectedArea]._particles)
			{
				auto angleVel = particle.GetOffsetAngleVelocity();
				if (std::abs(angleVel ) < 3.f)
				{
					if (angleVel  < 0.f)
						particle.AddOffsetAngleVelocity(-0.1f);
					else
						particle.AddOffsetAngleVelocity(0.1f);
				}
				else
				{
					if (angleVel  < 0.f)
						particle.SetOffsetAngleVelocity(-0.1f);
					else
						particle.SetOffsetAngleVelocity(0.1f);
				}
			}
			break;
		}
		if(_parts[_selectedArea]._particles.size() <= 20)
		{
			_objCount++;
			_parts[_selectedArea]._particles.push_back(
				Object<float>{
				_shapeType,
					_mousePos,
					Vector2<float>{15.f, 15.f}
			}
			);
			_parts[_selectedArea]._particles.back().SetPosLerp(
				_parts[_selectedArea]._centre.front().GetPos()
			);
			_parts[_selectedArea]._particles.back().SetTrailType(
				_parts[_selectedArea].trailType);
			_parts[_selectedArea]._particles.back().SetOffsetAngleVelocity(0.6f);
		}
		else if(_parts[_selectedArea].tickCount == 0)
		{
			for (auto& particle : _parts[_selectedArea]._particles)
			{
				particle.FreeObject();
			}
			_parts[_selectedArea]._centre.pop_back();
			_parts[_selectedArea].tickCount = 5;
			_parts[_selectedArea]._centre[0].SetShapeType(ShapeType::None);
			_parts[_selectedArea]._centre[1].SetShapeType(ShapeType::None);
			_selectedArea = -1;
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{
		if (_selectedArea == -1) break;
		_isHolding = !_isHolding;
		if (!_isHolding)
		{
			auto partSize = _parts.size();
			Vector2<float> mid{ _winSize.right / 2.f, _winSize.bottom / 2.f };
			for (auto& centre : _parts[_selectedArea]._centre)
			{
				centre.SetPos(Vector2<float>{
					mid.x / 2.f + mid.x * (_selectedArea % (partSize / 2)),
						mid.y / 2.f + mid.y * (_selectedArea / (partSize / 2))
				});
			}
			for (auto& particle : _parts[_selectedArea]._particles)
			{
				particle.SetPosLerp(_parts[_selectedArea]._centre.front().GetPos());
			}
		}
		break;
	}
	case WM_RBUTTONDBLCLK:
	{
		int col = _mousePos.x > _winSize.right / 2 ? 1 : 0;
		int row = _mousePos.y > _winSize.bottom / 2 ? 1 : 0;
		int idx = row * 2 + col;

		for (auto& centre : _parts[idx]._centre)
			InvertColor(centre);
		for (auto& particle : _parts[idx]._particles)
			InvertColor(particle);
		break;
	}
	case WM_MOUSEMOVE:
	{
		_mousePos = Vector2<int>{ LOWORD(lParam),HIWORD(lParam) };
		if (_isHolding && _selectedArea != -1)
		{
			for (auto& centre : _parts[_selectedArea]._centre)
			{
				centre.SetPos(_mousePos);
			}
			for (auto& particle : _parts[_selectedArea]._particles)
			{
				particle.SetPosLerp(_parts[_selectedArea]._centre.front().GetPos());
			}
		}
		break;
	}
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}


void Assignment2::RenderObject(Object<float>& object)
{
	HPEN oldPen = (HPEN)SelectObject(_dibDC,
		_pens[std::to_underlying(object.GetPenColor())]
		[object.GetPenThickness()]);
	HBRUSH oldBrush = (HBRUSH)SelectObject(
		_dibDC, _brushes[std::to_underlying(object.GetFillColor())]);
	object.Render(_dibDC, _scale,std::nullopt);
	if (_isDebug & DEBUG_OBJECT)
	{
		if (CollideHelper::Collide(object, _mousePos))
		{
			if(object.GetPenColor() != DefaultColors::Lime)
			{
				object.SetPenColor(DefaultColors::Lime);
				object.SetPenThickness(3);
			}
			_debugSelectedObject = &object;
			auto pos = object.GetPos();
			auto offsetPos = object.GetOffsetPos();
		}
		else if (object.GetPenColor() == DefaultColors::Lime)
		{
			object.RevertPenColor();
			object.SetPenThickness(1);
		}
	}
	SelectObject(_dibDC, oldPen);
	SelectObject(_dibDC, oldBrush);
}

void Assignment2::SelectRegion(int regionNum)
{
	if (_selectedArea != -1) {
		_parts[_selectedArea]._centre.pop_back();
	}
	if (_selectedArea + '1' != regionNum + '1')
	{
		_parts[regionNum]._centre.push_back(
			Object<float>{
			ShapeType::Square,
				_parts[regionNum]._centre.front().GetPos(),
				_parts[regionNum]._centre.back().GetSize() +
				Vector2<float>{30.f, 30.f}
		}
		);
		_parts[regionNum]._centre.back().SetFillColor(DefaultColors::Transparent);
		_parts[regionNum]._centre.back().SetPenThickness(1);
		_parts[regionNum]._centre.back().SetPenColor(DefaultColors::Blue);
		if (_isHolding)
		{
			auto partSize = _parts.size();
			Vector2<float> mid{ _winSize.right / 2.f, _winSize.bottom / 2.f };
			for (auto& centre : _parts[_selectedArea]._centre)
			{
				centre.SetPos(Vector2<float>{
					mid.x / 2.f + mid.x * (_selectedArea % (partSize / 2)),
						mid.y / 2.f + mid.y * (_selectedArea / (partSize / 2))
				});
			}
			_isHolding = false;
		}
		_selectedArea = regionNum;
	}
	else
	{
		if (_isHolding)
		{
			auto partSize = _parts.size();
			Vector2<float> mid{ _winSize.right / 2.f, _winSize.bottom / 2.f };
			for (auto& centre : _parts[_selectedArea]._centre)
			{
				centre.SetPos(Vector2<float>{
					mid.x / 2.f + mid.x * (_selectedArea % (partSize / 2)),
						mid.y / 2.f + mid.y * (_selectedArea / (partSize / 2))
				});
			}
			_isHolding = false;
		}
		_selectedArea = -1;
	}
}

void Assignment2::DrawGrid(const Vector2<LONG>& regionSt)
{
	for (int y = 0; y < _gridSize.y; ++y)
	{
		for (int x = 0; x < _gridSize.x; ++x)
		{
			if (_isDebug && _mousePos.x > x * _gridCellPart.x
				+ regionSt.x &&
				_mousePos.x <= (x + 1) * _gridCellPart.x + regionSt.x &&
				_mousePos.y > y * _gridCellPart.y + regionSt.y &&
				_mousePos.y <= (y + 1) * _gridCellPart.y + regionSt.y)
			{
				HPEN old = (HPEN)SelectObject(_dibDC,
					_gridPens[GetPaletteColor(DefaultColors::Lime)]);
				RenderUtil::DrawRect(_dibDC,
					Vector2{ x * _gridCellPart.x
					+ regionSt.x , y * _gridCellPart.y + regionSt.y },
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

void Assignment2::ContactBorder(Object<float>& assignment2Obj)
{
	if (CollideHelper::CollideScreenBorder(assignment2Obj, _winSize))
	{
		auto pos = assignment2Obj.GetPos();
		auto size = assignment2Obj.GetSize();
		auto velDir = assignment2Obj.GetVelocityDir();
		auto velSize = assignment2Obj.GetVelocity().magnitude();
		velDir.rotate(std::numbers::pi_v<float> / 2.f);
		pos.x += velDir.x * size.x;
		pos.y += velDir.y * size.y;
		assignment2Obj.SetPos(pos);
		assignment2Obj.SetVelocityDir(velDir);
		assignment2Obj.SetVelocityAmount(velSize / 2.f);
	}
}

void Assignment2::UpdateGrid()
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