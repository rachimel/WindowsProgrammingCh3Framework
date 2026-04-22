#include "pch.h"
#include "Assignment2.h"

void DumpMemory()
{
	_CrtDumpMemoryLeaks();
}
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPWSTR szCmdLine, int nCmdSHow)
{
	Assignment2 p{ hInstance };
	p.Init(nCmdSHow);
	// p.SetGrid(Vector2<int>{30, 30}, 3,DefaultColors::Grey);
	return p.Run();
	std::atexit(DumpMemory);
}