#pragma once
class Assignment2
{
public:
	struct GridOption
	{
		RECT gridRegion;
		bool scalable{ false };
	};
	Assignment2(HINSTANCE hInstance, const Vector2<LONG>& size = Vector2<LONG>{ 900,900 });
	~Assignment2();

	void SetGrid(const Vector2<int>& gridSize,
		int gridThickness, DefaultColors gridColor,
		std::optional<GridOption> = std::nullopt);
	void UnSetGrid();
	void ToggleDebug() { _isDebug = !_isDebug; }
	void Init(int nCmdShow);

	void Update();
	void ResizeScreen();
	void Render();

	int Run();
	static LRESULT CALLBACK WndProcEntry(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

private:
	template<typename... Args>
	void AddObject(std::vector<Object<float>>& obj, Args&&... args)
	{
		Object<float> obj{ std::forward<Args>(args)... };
		obj.push_back(obj);
	}

	void RenderObject(Object<float>& object);
	void SelectRegion(int regionNum);
private:
	void DrawGrid(const Vector2<LONG>& regionSt = Vector2<LONG>{});
	COLORREF GetPaletteColor(DefaultColors color)
	{
		return _colorPalettes[static_cast<int>(
			color
			)];
	}
private:
	HWND _hWnd;
	RECT _winSize;
	Vector2<LONG> _resolution;
	Vector2<float> _scale;
	HDC _hDC;
	HDC _backDC;
	HDC _dibDC;
	HBITMAP _bitmap;
	HBITMAP _dibBitmap;
	UINT32* _pxs;
	HFONT _infoFont;
	HFONT _statusFont;
	MSG _msg;
	//
	float _deltaTime, _frameTime;
	std::uint32_t _frameCount, _fps;
	LARGE_INTEGER _frequency, _count;


	enum class Centre
	{
		Object,
		Trail,
		Selected
	};
	struct Parts
	{
		Parts() : _centre{}, _particles{} {}
		std::vector<Object<float>> _centre;
		std::vector<Object<float>> _particles;
		TrailType trailType;
		int tickCount;
	};

	void InvertColor(Object<float>& obj) {
		if(obj.GetFillColor() == DefaultColors::Transparent) return;
		COLORREF cur = GetPaletteColor(obj.GetFillColor());
		COLORREF inv = cur ^ 0x00FFFFFF;
		int bestIdx = 0, bestDist = INT_MAX;
		for (int i = 0; i < (int)_colorPalettes.size(); i++) {
			int dr = GetRValue(inv) - GetRValue(_colorPalettes[i]);
			int dg = GetGValue(inv) - GetGValue(_colorPalettes[i]);
			int db = GetBValue(inv) - GetBValue(_colorPalettes[i]);
			int dist = dr * dr + dg * dg + db * db;
			if (dist < bestDist) { bestDist = dist; bestIdx = i; }
		}
		obj.SetFillColor(static_cast<DefaultColors>(bestIdx));
	}

	std::array<Parts, 4> _parts;
	std::vector<COLORREF> _colorPalettes;
	std::vector<HBRUSH> _brushes;
	int _selectedArea;
	bool _isHolding;
	bool _isXor;

	std::array<std::array<HPEN, g_maxThickness>,
		std::to_underlying(DefaultColors::Transparent) + 1> _pens;
	Vector2<int> _mousePos;
	// Flags
	ShapeType _shapeType{ ShapeType::None };
	bool _isGrid;
	bool _gameRun;

	// Debug
	std::uint8_t _isDebug;
	Object<float>* _debugSelectedObject;
	std::deque<Vector2<int>> _debugPosQueue;
	std::random_device rd;
	std::mt19937_64 mt{ rd() };
	std::uint64_t _objCount{};
private:
	void UpdateGrid();
	void ContactBorder(Object<float>& assignment2Obj);
private:
	// GridMode
	std::unordered_map<COLORREF, HPEN> _gridPens;
	DefaultColors _gridColor;
	Vector2<float> _gridCellPart;
	Vector2<int> _gridSize;
	std::optional<GridOption> _gridOption;
};

