#include "pch.h"
#include "Assignment1.h"

void DumpMemory()
{
	_CrtDumpMemoryLeaks();
}
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPWSTR szCmdLine, int nCmdSHow)
{
	Assignment1 p{ hInstance };
	p.Init(nCmdSHow);
	return p.Run();
	std::atexit(DumpMemory);
}