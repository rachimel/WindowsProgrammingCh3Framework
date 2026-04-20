#pragma once

constexpr size_t g_maxThickness{ 10 };
template <typename Ty>
concept HasEnumEnd = std::is_enum_v<Ty> &&
	requires { {Ty::End};};

class Program
{
public:
	struct GridOption
	{
		RECT gridRegion;
		bool scalable{ false };
	};
	Program(HINSTANCE hInstance, const Vector2<LONG>& size = Vector2<LONG>{ 900,900 });
	~Program();

	void SetGrid(const Vector2<int>& gridSize,
		int gridThickness, DefaultColors gridColor,
		std::optional<GridOption> = std::nullopt);
	void UnSetGrid();
	void ToggleDebug() { _isDebug = !_isDebug; }
	void Init(int nCmdShow);

	void Update();
	void Render();

	int Run();
	static LRESULT CALLBACK WndProcEntry(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

private:
	template<typename... Args>
	void AddObject(Args&&... args)
	{
		Object<float> obj{std::forward<Args>(args)...};
		_objects.push_back(obj);
	}

	void ContactBorder(Object<float>& assignment1Obj);
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

	std::vector<Object<float>> _objects;
	std::vector<COLORREF> _colorPalettes;
	std::vector<HBRUSH> _brushes;
	
	std::array<std::array<HPEN, g_maxThickness>,
	std::to_underlying(DefaultColors::End)> _pens;
	Vector2<int> _mousePos;
	// Flags
	ShapeType _shapeType{ ShapeType::None };
	bool _isGrid;
	bool _isDebug;
	Object<float>* _debugSelectedObject;
	std::deque<Vector2<int>> _debugPosQueue;
private:
	void UpdateGrid();
private:
	// GridMode
	std::unordered_map<COLORREF, HPEN> _gridPens;
	DefaultColors _gridColor;
	Vector2<float> _gridCellPart;
	Vector2<int> _gridSize;
	std::optional<GridOption> _gridOption;
};
