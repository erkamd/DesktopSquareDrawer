//
// keystroke.c - Pauses, then simulates a key press
// and release of the "A" key.
//
// Written by Ted Burke - last updated 17-4-2012
//
// To compile with MinGW:
//
//      gcc -o keystroke.exe keystroke.c
//
// To run the program:
//
//      keystroke.exe
//
// ...then switch to e.g. a Notepad window and wait
// 5 seconds for the A key to be magically pressed.
//

// Because the SendInput function is only supported in
// Windows 2000 and later, WINVER needs to be set as
// follows so that SendInput gets defined when windows.h
// is included below.
#define WINVER 0x0500
#include <windows.h>

#include <tuple>
#include <vector>
#include <windows.h>
#include <Commctrl.h>
#include <math.h>
#include <iostream>

void moveCursor(int x, int y)
{
	//INPUT ip;

	//// Set up a generic keyboard event.
	//ip.type = INPUT_MOUSE;
	//ip.mi.dx = x;
	//ip.mi.dy = y;
	//ip.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	//SendInput(1, &ip, sizeof(INPUT));

	SetCursorPos(x, y);
}

void leftClick()
{
	INPUT ip;

	// Set up a generic keyboard event.
	ip.type = INPUT_MOUSE;
	ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &ip, sizeof(INPUT));
}

void releaseButton()
{
	INPUT ip;

	ip.type = INPUT_MOUSE;
	ip.mi.dwFlags = MOUSEEVENTF_LEFTUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
}

std::pair<int, int> getRandomPos_free()
{
	//DECIDE FIRST POSITION
	int firstX = rand() % 1366;
	int firstY = rand() % 768;

	return std::make_pair(firstX, firstY);
}

std::pair<int, int> getRandomPos()
{
	HWND hProgman = FindWindow("Progman", NULL);
	HWND hWnd;
	HANDLE hExplorer;
	std::vector<POINT> lspan;
	DWORD dwProcessID = 0;
	unsigned short uspan;

	// Get a handle to the listview so we can interact with the desktop
	if (!hProgman)
	{
		return std::make_pair(0,0);
	}

	hWnd = FindWindowEx(hProgman, 0, "SHELLDLL_DefView", NULL);
	if (!hWnd)
	{
		return std::make_pair(0, 0);
	}

	hWnd = FindWindowEx(hWnd, 0, "SysListView32", NULL);
	if (!hWnd)
	{
		return std::make_pair(0, 0);
	}

	// Get the process handle for explorer
	GetWindowThreadProcessId(hWnd, &dwProcessID);
	if (!dwProcessID)
	{
		return std::make_pair(0, 0);
	}

	// Get a handle to explorer
	hExplorer = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, dwProcessID);
	if (!hExplorer)
	{
		return std::make_pair(0, 0);
	}

	// Find out how many <span class="searchlite">icon</span>s are on the desktop
	uspan = SendMessage(hWnd, LVM_GETITEMCOUNT, 0, 0);

	POINT *pCoords = (POINT*)VirtualAllocEx(hExplorer, NULL, sizeof(POINT), MEM_COMMIT, PAGE_READWRITE);
	if (!pCoords)
	{
		return std::make_pair(0, 0);
	}

	// Put a POINT object in explorer's virtual address space to hold the coords of an icon
	if (!WriteProcessMemory(hExplorer, pCoords, &pCoords, sizeof(POINT), NULL))
	{
		return std::make_pair(0, 0);
	}

	// Get the coordinates of all the <span class="searchlite">icon</span>s and store them for future use
	POINT pt;
	for (int x = 0; x < uspan; x++)
	{
		SendMessage(hWnd, LVM_GETITEMPOSITION, (WPARAM)x, (LPARAM)pCoords);

		if (!ReadProcessMemory(hExplorer, pCoords, &pt, sizeof(POINT), NULL)) {
			return std::make_pair(0, 0);
		}

		lspan.push_back(pt);
	}

	again:
	//DECIDE FIRST POSITION
	int firstX = rand() % 1366;
	int firstY = rand() % 720;


	for (size_t i = 0; i < lspan.size(); i++)
	{
		if (firstX >= lspan[i].x - 65 && firstX <= lspan[i].x + 65 &&
			firstY >= lspan[i].y - 70 && firstY <= lspan[i].y + 70)
		{
			goto again;
		}
	}

	std::cout << firstX << ", " << firstY << "\n";
	return std::make_pair(firstX, firstY);
}

void aKey()
{
	// This structure will be used to create the keyboard
	// input event.
	INPUT ip;

	

	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;


	// Press the "A" key
	ip.ki.wVk = 0x41; // virtual-key code for the "a" key
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the "A" key
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
}

std::pair<int, int> last;

void Update()
{
	bool first = true;

	//loop
	while (true)
	{
		POINT point;
		GetCursorPos(&point);

		if (!first && (last.first != point.x || last.second != point.y))
		{
			std::cout << "canceled by user. \n";
			break;
		}

		first = false;

		//DECIDE FIRST POSITION
		std::pair<int,int> firstPos = getRandomPos();
		moveCursor(firstPos.first, firstPos.second);
		leftClick();

		std::pair<int, int> lastPos = getRandomPos_free();
		std::pair<int, int> diff;

		diff.first = (lastPos.first - firstPos.first) / 10;
		diff.second = (lastPos.second - firstPos.second) / 10;

		for (size_t i = 0; i < 10; i++)
		{
			GetCursorPos(&point);

			if (point.x != firstPos.first || point.y != firstPos.second)
			{
				std::cout << "canceled by user. \n";
				break;
			}

			Sleep(50);

			firstPos.first += diff.first;
			firstPos.second += diff.second;

			moveCursor(firstPos.first, firstPos.second);
			
		}

		releaseButton();

		GetCursorPos(&point);
		last.first = point.x;
		last.second = point.y;

		Sleep(100);
	}
}

#include <time.h>

int main()
{
	srand(time(NULL));

	// Pause for 5 seconds.

	POINT point;

	while (true)
	{
		GetCursorPos(&point);
		last.first = point.x;
		last.second = point.y;

		Sleep(4000);

		GetCursorPos(&point);

		if (last.first == point.x && last.second == point.y)
		{
			std::cout << "Started again. \n";
			goto beginagain;
		}
	}

beginagain:
	Update();

	

	while (true)
	{
		GetCursorPos(&point);
		last.first = point.x;
		last.second = point.y;

		Sleep(4000);

		GetCursorPos(&point);

		if (last.first == point.x && last.second == point.y)
		{
			std::cout << "Started again. \n";
			goto beginagain;
		}
	}

	// Exit normally
	return 0;
}
