#include "BackBufferController.h"
#include <stdio.h>
#include <Windows.h>
#include <tchar.h>


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstace, LPSTR lpCmdLine, int nCmdShow) {
	Vgfw vgfw;// = (Vgfw*)malloc(sizeof(Vgfw));

	SetPriorityClass(hInstance, REALTIME_PRIORITY_CLASS);
	vgfw.initialize(L"hello", hInstance);
	vgfw.run();
}