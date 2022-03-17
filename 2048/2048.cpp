// @Jan Cichomski: Game 2048 
//
// parent - main game window
// second - window which mimics main window
// scoreWin1/scoreWin1 - scorebar windows
// w1 - array of squre windows for parent
// w2 - array of squre windows for second

#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"
#include "2048.h"
#include "Resource.h"
#include "resource.h"
#include <time.h>
#include <string.h>
#include <Windows.h>
#include <wingdi.h>
#include <errno.h>
//using namespace std;
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND second = NULL;
HWND parent = NULL;
HWND scoreWin1 = NULL;
HWND scoreWin2 = NULL;
HWND endWin1 = NULL;
HWND endWin2 = NULL;
struct area {
	HWND h = NULL;
	int val = -1;
	RECT rc = { 0,0,0,0 };
	int x = -1, y = -1;
};
area* w1[4][4];
area* w2[4][4];
int score = 0; // Players score
int gameGoal = 2048; // Required squre to win game
int biggestSquare = 2; // Keeps track of bigges square in game
int state = -1; // -1 - game goes on / 0 - player lost / 1- player won

// Functions declarations
ATOM MyRegisterClass(HINSTANCE hInstance);
ATOM RegisterChild(HINSTANCE hInstance);
ATOM RegisterSecond(HINSTANCE hInstance);
ATOM RegisterPointsWindow(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
BOOL InitSecond(HINSTANCE hInstance, HWND hWnd);
void AnimateMergeWin(HWND hWnd);
BOOL CALLBACK  Repaint(HWND hWnd, LPARAM lParam, HDC hdc, PAINTSTRUCT ps);
void UpdatePointsWin();
BOOL IsThereLegalMove();
bool CoverWinWithBitMap(HDC hdc, RECT dim, COLORREF penCol, unsigned int opacity, HWND hWnd);
void EndGameProcedure();
void MoveWindows(HWND hWnd, TCHAR c);
void UpdateMiniWindows();
void AnimateRandWin(HWND hWnd);
void FillRandomSquare(bool animate = true);
void NewGame();
void SaveGameState();
void LoadGameFailedProcedure();
void LoadGameState();
void TickMenuBox(int newGameGoal);
void PaintEndGameScreeen(HWND hWnd, HDC hdc, PAINTSTRUCT ps);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProcSecond(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcChild(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcScore(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	srand(time(NULL));
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MY2048, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	RegisterChild(hInstance);
	RegisterSecond(hInstance);
	RegisterPointsWindow(hInstance);
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY2048));
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
// Register main window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(250, 247, 238));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY2048);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON3));
	return RegisterClassExW(&wcex);
}
// Register square class window
ATOM RegisterChild(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcChild;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY2048));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(250, 247, 238));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY2048);
	wcex.lpszClassName = L"childClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassExW(&wcex);
}
// Register second main window class
ATOM RegisterSecond(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | WS_EX_TOOLWINDOW;
	wcex.lpfnWndProc = WndProcSecond;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(250, 247, 238));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY2048);
	wcex.lpszClassName = L"second";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON3));
	return RegisterClassExW(&wcex);
}
// Register scorebar window class
ATOM RegisterPointsWindow(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcScore;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(250, 247, 238));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY2048);
	wcex.lpszClassName = L"pointsWindow";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON3));
	return RegisterClassExW(&wcex);
}
// Initiate second main window
BOOL InitSecond(HINSTANCE hInstance, HWND hWnd)
{
	int y = GetSystemMetrics(SM_CYSCREEN);
	int x = GetSystemMetrics(SM_CXSCREEN);
	RECT rc;
	rc.bottom = 290 + 70;
	rc.top = 0;
	rc.left = 0;
	rc.right = 290;
	AdjustWindowRect(&rc, WS_CAPTION, TRUE);
	second = CreateWindowW(L"second", L"2048",
		WS_CAPTION,
		x / 2, y / 2, rc.right - rc.left, rc.bottom - rc.top, hWnd, nullptr, hInstance, nullptr);
	SetWindowTextA(second, "2048");
	SetWindowLong(second, GWL_EXSTYLE, GetWindowLong(second, GWL_EXSTYLE) | WS_EX_LAYERED);
	if (!second)
		return FALSE;
	ShowWindow(second, SW_HIDE);
	ShowWindow(second, SW_SHOW);
	UpdateWindow(second);
	return TRUE;
}
// initiate entire game
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	int y = GetSystemMetrics(SM_CYSCREEN);
	int x = GetSystemMetrics(SM_CXSCREEN);
	RECT rc;
	rc.bottom = 290 + 70;
	rc.top = 0;
	rc.left = 0;
	rc.right = 290;
	AdjustWindowRect(&rc, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, TRUE);
	HWND hWnd = CreateWindowW(szWindowClass, szTitle,
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		x / 2, x / 2, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	parent = hWnd;
	SetWindowTextA(hWnd, "2048");
	if (!hWnd)
		return FALSE;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	InitSecond(hInst, hWnd);
	LoadGameState();
	TickMenuBox(gameGoal);
	return TRUE;
}
// Animation of merging two squres with equal value
void AnimateMergeWin(HWND hWnd)
{
	static int s = 80;
	area* a = (area*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
	MoveWindow(a->h, a->rc.left + 30 - s / 2, a->rc.top + 30 - s / 2, s, s, TRUE);
	int x = a->x;
	int y = a->y;

	MoveWindow(w2[x][y]->h, w2[x][y]->rc.left + 30 - s / 2, w2[x][y]->rc.top + 30 - s / 2, s, s, TRUE);
	if (s <= 60) {
		KillTimer(hWnd, 2);
		s = 80;
	}
	s -= 2;
}
// Repains given window square with new color and text based on square value
BOOL CALLBACK  Repaint(HWND hWnd, LPARAM lParam, HDC hdc, PAINTSTRUCT ps) {
	area* a = (area*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
	RECT rc;
	GetClientRect(hWnd, &rc);
	COLORREF color;
	const int bufSize = 256;
	if (a->val == -1) color = RGB(204, 192, 174);
	else if (a->val == 2) color = RGB(238, 228, 198);
	else if (a->val == 4) color = RGB(239, 225, 218);
	else if (a->val == 8) color = RGB(243, 179, 124);
	else if (a->val == 16) color = RGB(246, 153, 100);
	else if (a->val == 32) color = RGB(246, 125, 98);
	else if (a->val == 64) color = RGB(247, 93, 60);
	else if (a->val == 128) color = RGB(237, 206, 116);
	else if (a->val == 256) color = RGB(239, 204, 98);
	else if (a->val == 512) color = RGB(243, 201, 85);
	else if (a->val == 1024) color = RGB(238, 200, 72);
	else if (a->val == 2048)color = RGB(239, 192, 47);
	else color = RGB(0, 0, 0);
	HPEN pen = CreatePen(PS_SOLID, 1, color);
	HBRUSH brush = CreateSolidBrush(color);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
	HPEN oldPen = (HPEN)SelectObject(hdc, pen);
	RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 25, 25);

	if (a->val == -1)
		return TRUE;

	int c = a->val;
	TCHAR s[10];
	const wchar_t len = 10;
	swprintf(s, len, L"%d", c);
	GetClientRect(hWnd, &rc);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));
	int fontSize = 30;
	if (a->val > 1000) fontSize = 24;
	HFONT font = CreateFont(30, 0, 0, 0, FW_BOLD, false, FALSE, 0, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T(" Verdana "));
	HFONT oldFont = (HFONT)SelectObject(hdc, font);
	DrawText(hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	SelectObject(hdc, oldFont);
	SelectObject(hdc, oldPen);
	SelectObject(hdc, oldBrush);
	DeleteObject(font);
	DeleteObject(pen);
	DeleteObject(brush);
	return TRUE;
}
// Repains scorebar windows
void UpdatePointsWin() {
	InvalidateRect(scoreWin2, NULL, NULL);
	InvalidateRect(scoreWin1, NULL, NULL);
}
// How to repaint scorebar window
void PaintScoreWin(HWND hWnd, HDC hdc, PAINTSTRUCT ps) {
	RECT rc;
	GetClientRect(hWnd, &rc);
	HPEN pen = CreatePen(PS_SOLID, 1, RGB(204, 192, 174));
	HBRUSH brush = CreateSolidBrush(RGB(204, 192, 174));
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
	HPEN oldPen = (HPEN)SelectObject(hdc, pen);
	RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 15, 15);
	HFONT font = CreateFont(30, 0, 0, 0, FW_BOLD, false, FALSE, 0, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T(" Verdana "));
	HFONT oldFont = (HFONT)SelectObject(hdc, font);
	TCHAR s[10];
	const wchar_t len = 10;
	swprintf(s, len, L"%d", score);
	GetClientRect(hWnd, &rc);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));
	DrawText(hdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	SelectObject(hdc, oldPen);
	SelectObject(hdc, oldBrush);
	SelectObject(hdc, oldFont);
	DeleteObject(pen);
	DeleteObject(brush);
	DeleteObject(font);
}
// Checks if player can perform any move, if not than game ends, otherwise game continues
BOOL IsThereLegalMove() {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			if (i < 3) {
				if (w1[i][j]->val == -1 || w1[i][j]->val == w1[i + 1][j]->val)
					return TRUE;
			}
			if (j < 3) {
				if (w1[i][j]->val == w1[i][j + 1]->val)
					return TRUE;
			}
		}
	return FALSE;
}
// I used:  https://stackoverflow.com/a/10975603/15767108 
bool CoverWinWithBitMap(HDC hdc, RECT dim, COLORREF penCol, unsigned int opacity, HWND hWnd) {
	HDC tempHdc = CreateCompatibleDC(hdc);
	BLENDFUNCTION blend = { AC_SRC_OVER, 0, opacity, 0 };
	HBITMAP hbitmap;
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = dim.right - dim.left;
	bmi.bmiHeader.biHeight = dim.bottom - dim.top;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = (dim.right - dim.left) * (dim.bottom - dim.top) * 4;
	hbitmap = CreateDIBSection(tempHdc, &bmi, DIB_RGB_COLORS, NULL, NULL, 0x0);
	HBITMAP old = (HBITMAP)SelectObject(tempHdc, hbitmap);

	if (state == 1)
		FillRect(tempHdc, &dim, CreateSolidBrush(RGB(0, 255, 0)));
	else if (state == 0)
		FillRect(tempHdc, &dim, CreateSolidBrush(RGB(255, 0, 0)));
	else //raczej nie może zajść
		FillRect(tempHdc, &dim, CreateSolidBrush(RGB(0, 0, 0)));
	TCHAR s[10];
	RECT rc;
	const wchar_t len = 10;
	GetClientRect(hWnd, &rc);
	HFONT font = CreateFont(100, 0, 0, 0, FW_BOLD, false, FALSE, 0, EASTEUROPE_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T(" Verdana "));
	HFONT oldFont = (HFONT)SelectObject(tempHdc, font);
	SetBkMode(tempHdc, TRANSPARENT);
	SetTextColor(tempHdc, RGB(255, 255, 255));
	if (hWnd == parent || hWnd == second) {
		if (state == 1) {
			swprintf(s, len, L"WIN!");
			DrawText(tempHdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
		if (state == 0) {
			swprintf(s, len, L"LOSE!");
			DrawText(tempHdc, s, (int)_tcslen(s), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}
	}

	bool ret = bool(AlphaBlend(hdc, dim.left, dim.top, dim.right, dim.bottom, tempHdc, dim.left, dim.top, dim.right, dim.bottom, blend));
	SelectObject(hdc, oldFont);
	SelectObject(tempHdc, old);

	DeleteObject(font);
	DeleteObject(old);
	DeleteObject(hbitmap);
	return ret;
}
// Displays endgame semitransparent bitmap on window (function called when game ended for any reason)
void EndGameProcedure() {
	InvalidateRect(scoreWin1, NULL, NULL);
	InvalidateRect(parent, NULL, NULL);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			InvalidateRect(w1[i][j]->h, NULL, NULL);

	InvalidateRect(second, NULL, NULL);
	InvalidateRect(scoreWin2, NULL, NULL);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			InvalidateRect(w2[i][j]->h, NULL, NULL);
	int a = 1;
}
// Game controles implementation (moving and merging all valid windows)
void MoveWindows(HWND hWnd, TCHAR c) {

	BOOL sthMoved = FALSE;
	BOOL merged[4][4] = { FALSE };
	switch (c)
	{
	case 'a':
	case 'A':
	{
		for (int i = 1; i < 4; i++)
			for (int j = 0; j < 4; j++)
				for (int k = 0; k < i; k++) {
					if (w1[k][j]->val != -1 && w1[k][j]->val == w1[i][j]->val && merged[k][j] == FALSE) {
						BOOL clearWay = TRUE;
						for (int w = k + 1; w < i; w++)
							if (w1[w][j]->val != -1) clearWay = FALSE;
						if (clearWay) {
							w1[k][j]->val = w1[k][j]->val * 2;
							w1[i][j]->val = -1;
							score += w1[k][j]->val;
							if (w1[k][j]->val > biggestSquare)
								biggestSquare = w1[k][j]->val;
							sthMoved = TRUE;
							merged[k][j] = TRUE;
							InvalidateRect(w1[i][j]->h, NULL, NULL);
							InvalidateRect(w1[k][j]->h, NULL, NULL);
							SetTimer(w1[k][j]->h, 2, 5, NULL);
						}
					}
					else if (w1[k][j]->val == -1 && w1[i][j]->val != -1) {
						w1[k][j]->val = w1[i][j]->val;
						w1[i][j]->val = -1;
						InvalidateRect(w1[i][j]->h, NULL, NULL);
						InvalidateRect(w1[k][j]->h, NULL, NULL);
						sthMoved = TRUE;
					}
				}
		break;
	}
	case 'd':
	case 'D':
	{
		for (int i = 2; i >= 0; i--)
			for (int j = 0; j < 4; j++)
				for (int k = 3; k > i; k--) {
					if (w1[k][j]->val != -1 && w1[k][j]->val == w1[i][j]->val && merged[k][j] == FALSE) {
						BOOL clearWay = TRUE;
						for (int w = i + 1; w < k; w++)
							if (w1[w][j]->val != -1) clearWay = FALSE;
						if (clearWay) {
							w1[k][j]->val = w1[k][j]->val * 2;
							w1[i][j]->val = -1;
							score += w1[k][j]->val;
							if (w1[k][j]->val > biggestSquare)
								biggestSquare = w1[k][j]->val;
							sthMoved = TRUE;
							merged[k][j] = TRUE;
							InvalidateRect(w1[i][j]->h, NULL, NULL);
							InvalidateRect(w1[k][j]->h, NULL, NULL);
							SetTimer(w1[k][j]->h, 2, 5, NULL);

						}
					}
					else if (w1[k][j]->val == -1 && w1[i][j]->val != -1) {
						w1[k][j]->val = w1[i][j]->val;
						w1[i][j]->val = -1;
						InvalidateRect(w1[i][j]->h, NULL, NULL);
						InvalidateRect(w1[k][j]->h, NULL, NULL);
						sthMoved = TRUE;
					}
				}
		break;
	}
	case 'w':
	case 'W':
	{
		for (int j = 0; j < 4; j++)
			for (int i = 1; i < 4; i++)
				for (int k = 0; k < i; k++) {
					if (w1[j][k]->val != -1 && w1[j][k]->val == w1[j][i]->val && merged[j][k] == FALSE) {
						BOOL clearWay = TRUE;
						for (int w = k + 1; w < i; w++)
							if (w1[j][w]->val != -1) clearWay = FALSE;
						if (clearWay) {
							w1[j][k]->val = w1[j][k]->val * 2;
							w1[j][i]->val = -1;
							score += w1[j][k]->val;
							if (w1[j][k]->val > biggestSquare)
								biggestSquare = w1[j][k]->val;
							sthMoved = TRUE;
							merged[j][k] = TRUE;
							InvalidateRect(w1[j][i]->h, NULL, NULL);
							InvalidateRect(w1[j][k]->h, NULL, NULL);
							SetTimer(w1[j][k]->h, 2, 5, NULL);
						}
					}
					else if (w1[j][k]->val == -1 && w1[j][i]->val != -1) {
						w1[j][k]->val = w1[j][i]->val;
						w1[j][i]->val = -1;
						InvalidateRect(w1[j][i]->h, NULL, NULL);
						InvalidateRect(w1[j][k]->h, NULL, NULL);
						sthMoved = TRUE;
					}
				}
		break;
	}
	case 's':
	case 'S':
	{
		for (int j = 0; j < 4; j++)
			for (int i = 2; i >= 0; i--)
				for (int k = 3; k > i; k--) {
					if (w1[j][k]->val != -1 && w1[j][k]->val == w1[j][i]->val && merged[j][k] == FALSE) {
						BOOL clearWay = TRUE;
						for (int w = i + 1; w < k; w++)
							if (w1[j][w]->val != -1) clearWay = FALSE;
						if (clearWay) {
							w1[j][k]->val = w1[j][k]->val * 2;
							w1[j][i]->val = -1;
							score += w1[j][k]->val;
							if (w1[j][k]->val > biggestSquare)
								biggestSquare = w1[j][k]->val;
							sthMoved = TRUE;
							merged[j][k] = TRUE;
							InvalidateRect(w1[j][i]->h, NULL, NULL);
							InvalidateRect(w1[j][k]->h, NULL, NULL);
							SetTimer(w1[j][k]->h, 2, 5, NULL);
						}
					}
					else if (w1[j][k]->val == -1 && w1[j][i]->val != -1) {
						w1[j][k]->val = w1[j][i]->val;
						w1[j][i]->val = -1;
						InvalidateRect(w1[j][i]->h, NULL, NULL);
						InvalidateRect(w1[j][k]->h, NULL, NULL);
						sthMoved = TRUE;
					}
				}
		break;
	}
	default:
		break;
	}

	if (sthMoved)
		FillRandomSquare();
	UpdatePointsWin();

	if (IsThereLegalMove() == FALSE) {
		state = 0;
		EndGameProcedure();
		return;
	}
	else if (gameGoal <= biggestSquare) {
		state = 1;
		EndGameProcedure();
		return;
	}
	else state = -1;
}
// Update values on second main window (since I perform all operation on main window and only copies output of these operation to second main windwo)
void UpdateMiniWindows() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			w2[i][j]->val = w1[i][j]->val;
			RECT rc;
			GetClientRect(w2[i][j]->h, &rc);
			InvalidateRect(w2[i][j]->h, &rc, TRUE);
		}
	}
	InvalidateRect(scoreWin2, NULL, NULL);
}
// Animation of new poping up, squares
void AnimateRandWin(HWND hWnd) {
	static int size = 20;

	area* a = (area*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
	MoveWindow(a->h, a->rc.left + 30 - size / 2, a->rc.top + 30 - size / 2, size, size, TRUE);
	int x = a->x;
	int y = a->y;
	MoveWindow(w2[x][y]->h, w2[x][y]->rc.left + 30 - size / 2, w2[x][y]->rc.top + 30 - size / 2, size, size, TRUE);

	if (size == 60) {
		KillTimer(hWnd, 1);
		size = 30;
	}
	size += 5;
}
// Fill one of free squares with new square with value 2
void FillRandomSquare(bool animate) {
	int y = rand() % 4;
	int x = rand() % 4;
	BOOL isFreeSquere = FALSE;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (w1[i][j]->val == -1) isFreeSquere = TRUE;
		}
	}
	if (isFreeSquere == FALSE)// probably not nessesary
		return;
	while (w1[x][y]->val != -1) {
		y = rand() % 4;
		x = rand() % 4;
	}
	w1[x][y]->val = 2;
	RECT rc;
	GetClientRect(w1[x][y]->h, &rc);
	InvalidateRect(w1[x][y]->h, &rc, TRUE);
	if (animate)
		SetTimer(w1[x][y]->h, 1, 30, NULL);
}
// Starts new game, sets default values for all game parameters, expect gameGoal
void NewGame() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			w1[i][j]->val = -1;
			RECT rc;
			GetClientRect(w1[i][j]->h, &rc);
			InvalidateRect(w1[i][j]->h, &rc, TRUE);
		}
	}
	FillRandomSquare();
	UpdateMiniWindows();
	UpdatePointsWin();
	biggestSquare = 2;
	score = 0;
	state = -1;
	InvalidateRect(parent, NULL, NULL);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			InvalidateRect(w1[i][j]->h, NULL, NULL);
		}
	InvalidateRect(second, NULL, NULL);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			InvalidateRect(w2[i][j]->h, NULL, NULL);
		}
}
// Save game state when player quits game
void SaveGameState() {
	const int bufSize = 256;
	WCHAR buf[bufSize];
	// My eyes hurt too...
	// Feel free to tell me how to do it better
	_stprintf_s(buf, bufSize, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
		w1[0][0]->val,
		w1[0][1]->val,
		w1[0][2]->val,
		w1[0][3]->val,
		w1[1][0]->val,
		w1[1][1]->val,
		w1[1][2]->val,
		w1[1][3]->val,
		w1[2][1]->val,
		w1[2][2]->val,
		w1[2][0]->val,
		w1[2][3]->val,
		w1[3][0]->val,
		w1[3][1]->val,
		w1[3][2]->val,
		w1[3][3]->val);
	WritePrivateProfileStringW(L"2048", L"SQUARES", buf, L".//.file.ini");
	memset((void*)buf, 0, bufSize);
	_stprintf_s(buf, bufSize, _T("%d"), score);
	WritePrivateProfileStringW(L"2048", L"SCORE", buf, L".//.file.ini");
	memset((void*)buf, 0, bufSize);
	_stprintf_s(buf, bufSize, _T("%d"), gameGoal);
	WritePrivateProfileStringW(L"2048", L"GOAL", buf, L".//.file.ini");
	memset((void*)buf, 0, bufSize);
	_stprintf_s(buf, bufSize, _T("%d"), biggestSquare);
	WritePrivateProfileStringW(L"2048", L"BIGGESTSQUARE", buf, L".//.file.ini");
	memset((void*)buf, 0, bufSize);
	_stprintf_s(buf, bufSize, _T("%d"), state);

	WritePrivateProfileStringW(L"2048", L"STATE", buf, L".//.file.ini");
}
// If game state can't bo loade form init file properlly, it solves all problems
void LoadGameFailedProcedure() {
	score = 0;
	gameGoal = 2048;
	state = -1;
	biggestSquare = 2;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			w1[i][j]->val = -1;
			InvalidateRect(w1[i][j]->h, NULL, FALSE);
		}
	FillRandomSquare();
}
// Loads game state form file
void LoadGameState() {
	const int bufSize = 256;
	WCHAR buf[bufSize];
	GetPrivateProfileStringW(L"2048", L"SCORE", L"ERROR", buf, bufSize, L".//.file.ini");
	if (errno == 0x2) {
		MessageBox(NULL, L"nie ma", NULL, NULL);
		return;
	}
	if (strcmp((const char*)buf, "ERROR") == 0) {
		LoadGameFailedProcedure();
		return;
	}
	score = _wtoi(buf);
	memset((void*)buf, 0, bufSize);

	GetPrivateProfileStringW(L"2048", L"GOAL", L"ERROR", buf, bufSize, L".//.file.ini");
	if (strcmp((const char*)buf, "ERROR") == 0) {
		LoadGameFailedProcedure();
		return;
	}
	gameGoal = _wtoi(buf);
	memset((void*)buf, 0, bufSize);

	GetPrivateProfileStringW(L"2048", L"BIGGESTSQUARE", L"ERROR", buf, bufSize, L".//.file.ini");
	if (strcmp((const char*)buf, "ERROR") == 0) {
		LoadGameFailedProcedure();
		return;
	}
	biggestSquare = _wtoi(buf);
	memset((void*)buf, 0, bufSize);

	GetPrivateProfileStringW(L"2048", L"STATE", L"ERROR", buf, bufSize, L".//.file.ini");
	if (strcmp((const char*)buf, "ERROR") == 0) {
		LoadGameFailedProcedure();
		return;
	}
	state = _wtoi(buf);
	memset((void*)buf, 0, bufSize);
	GetPrivateProfileStringW(L"2048", L"SQUARES", L"ERROR", buf, bufSize, L".//.file.ini");
	if (strcmp((const char*)buf, "ERROR") == 0) {
		LoadGameFailedProcedure();
		return;
	}
	const wchar_t delim = ',';
	wchar_t* c;

	wchar_t* xD = (wchar_t*)malloc(bufSize * sizeof(wchar_t));
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			if (i == 0 && j == 0)
				c = wcstok_s(buf, &delim, &xD);
			else
				c = wcstok_s(NULL, &delim, &xD);
			if (c == NULL) {
				LoadGameFailedProcedure();
				return;
			}
			int a = _wtoi(c);
			if (a == 0) a = -1;
			w1[i][j]->val = a;
		}
	if (state != -1)EndGameProcedure();
	UpdateMiniWindows();
	InvalidateRect(scoreWin1, NULL, NULL);
	InvalidateRect(scoreWin2, NULL, NULL);
}
// Showing which menu item is ticked
void TickMenuBox(int newGameGoal) {
	HMENU menu1 = GetMenu(parent);
	HMENU menu2 = GetMenu(second);

	if (gameGoal == 8) {

		CheckMenuItem(menu1, ID_GOALS_8, MF_UNCHECKED);
		CheckMenuItem(menu2, ID_GOALS_8, MF_UNCHECKED);
	}
	else if (gameGoal == 16) {

		CheckMenuItem(menu1, ID_GOALS_16, MF_UNCHECKED);
		CheckMenuItem(menu2, ID_GOALS_16, MF_UNCHECKED);
	}
	else if (gameGoal == 64) {

		CheckMenuItem(menu1, ID_GOALS_64, MF_UNCHECKED);
		CheckMenuItem(menu2, ID_GOALS_64, MF_UNCHECKED);
	}
	else if (gameGoal == 2048) {

		CheckMenuItem(menu1, ID_GOALS_2048, MF_UNCHECKED);
		CheckMenuItem(menu2, ID_GOALS_2048, MF_UNCHECKED);
	}
	if (newGameGoal == 8) {

		CheckMenuItem(menu1, ID_GOALS_8, MF_CHECKED);
		CheckMenuItem(menu2, ID_GOALS_8, MF_CHECKED);
	}
	else if (newGameGoal == 16) {

		CheckMenuItem(menu1, ID_GOALS_16, MF_CHECKED);
		CheckMenuItem(menu2, ID_GOALS_16, MF_CHECKED);
	}
	else if (newGameGoal == 64) {

		CheckMenuItem(menu1, ID_GOALS_64, MF_CHECKED);
		CheckMenuItem(menu2, ID_GOALS_64, MF_CHECKED);
	}
	else if (newGameGoal == 2048) {

		CheckMenuItem(menu1, ID_GOALS_2048, MF_CHECKED);
		CheckMenuItem(menu2, ID_GOALS_2048, MF_CHECKED);
	}
	gameGoal = newGameGoal;
}
// Pains game end screen( WIN OR LOSE + proper backgroud)
void PaintEndGameScreeen(HWND hWnd, HDC hdc, PAINTSTRUCT ps) {
	RECT a;
	GetClientRect(hWnd, &a);
	if (!CoverWinWithBitMap(hdc, a, RGB(255, 255, 0), 130, hWnd))
		MessageBox(NULL, L"fun albpa false", NULL, NULL);
}
// Proces messeges for main window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOVE:
	{
		int y = GetSystemMetrics(SM_CYSCREEN) / 2;
		int x = GetSystemMetrics(SM_CXSCREEN) / 2;
		RECT m;
		RECT s;
		RECT rc;
		rc.bottom = 290 + 70;
		rc.top = 0;
		rc.left = 0;
		rc.right = 290;
		AdjustWindowRect(&rc, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, TRUE);
		GetWindowRect(parent, &m);
		int a, b;
		a = 2 * x - m.right;
		b = 2 * y - m.bottom;
		MoveWindow(second, a, b, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		GetWindowRect(second, &s);
		GetWindowRect(parent, &m);
		if (((m.top >= s.top && m.top <= s.bottom - 9) || (m.bottom - 9 >= s.top && m.bottom <= s.bottom)) &&
			((m.left >= s.left && m.left <= s.right - 15) || (m.right - 15 >= s.left && m.right <= s.right)))
			SetLayeredWindowAttributes(second, NULL, 150, LWA_ALPHA);
		else
			SetLayeredWindowAttributes(second, NULL, 255, LWA_ALPHA);
		break;
	}
	case WM_SIZING:
	{
		RECT rc;
		rc.bottom = 290 + 70;
		rc.top = 0;
		rc.left = 0;
		rc.right = 290;
		AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION, FALSE);
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL);
		break;
	}
	case WM_CREATE:
	{
		RECT rc;
		RECT p;
		GetClientRect(parent, &p);
		rc.bottom = 60;
		rc.top = 0;
		rc.left = 0;
		rc.right = 270;
		AdjustWindowRect(&rc, WS_CHILD, FALSE);
		scoreWin1 = CreateWindowW(L"pointsWindow", L"asf", WS_CHILD | WS_VISIBLE,
			10, 10, rc.right - rc.left, rc.bottom - rc.top, hWnd, nullptr, hInst, nullptr);
		int gap = 10;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				w1[i][j] = new area();
				w1[i][j]->h = CreateWindowW(L"childClass", L"child", WS_CHILD | WS_VISIBLE, i * (gap + 60) + gap, 70 + j * (gap + 60) + gap, 60, 60, hWnd, nullptr, hInst, nullptr);
				SetWindowLongPtrA(w1[i][j]->h, GWLP_USERDATA, (LONG_PTR)w1[i][j]);
				w1[i][j]->val = -1;
				w1[i][j]->rc.left = i * (gap + 60) + gap;
				w1[i][j]->rc.top = 70 + j * (gap + 60) + gap;
				w1[i][j]->rc.right = i * (gap + 60) + gap + 60;
				w1[i][j]->rc.bottom = 70 + j * (gap + 60) + gap + 60;
				w1[i][j]->rc.bottom = 70 + j * (gap + 60) + gap + 60;
				w1[i][j]->x = i;
				w1[i][j]->y = j;
			};
		}
		FillRandomSquare(false);
		break;
	}
	case WM_CHAR:
	{
		if (state == -1) { // Code blocking actions after win/lose
			TCHAR c = (TCHAR)wParam;
			if (c == 'a' || c == 'A' || c == 'w' || c == 'W' || c == 's' || c == 'S' || c == 'd' || c == 'D')
				MoveWindows(hWnd, c);
			UpdateMiniWindows();
		}
		break;
	}
	case WM_COMMAND:
	{
		HMENU menu = GetMenu(hWnd);
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case ID_GOALS_8:
			TickMenuBox(8);
			break;
		case ID_GOALS_16:
			TickMenuBox(16);
			break;
		case ID_GOALS_64:
			TickMenuBox(64);
			break;
		case ID_GOALS_2048:
			TickMenuBox(2048);
			break;
		case ID_GAME_NEWGAME:
			NewGame();
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		RECT rc;
		GetClientRect(hWnd, &rc);
		FillRect(hdc, &rc, CreateSolidBrush(RGB(250, 247, 238)));
		if (state != -1)
			PaintEndGameScreeen(hWnd, hdc, ps);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_CLOSE:
		SaveGameState();
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
// Proces messeges for second main window
LRESULT CALLBACK WndProcSecond(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MOVE:
	{
		int y = GetSystemMetrics(SM_CYSCREEN) / 2;
		int x = GetSystemMetrics(SM_CXSCREEN) / 2;
		RECT m;
		RECT rc;
		rc.bottom = 290 + 70;
		rc.top = 0;
		rc.left = 0;
		rc.right = 290;
		AdjustWindowRect(&rc, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, TRUE);
		GetWindowRect(second, &m);
		int a, b;
		a = 2 * x - m.right;
		b = 2 * y - m.bottom;
		MoveWindow(parent, a, b, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		break;
	}
	case WM_CHAR:
	{
		if (state == -1) {
			TCHAR c = (TCHAR)wParam;
			if (c == 'a' || c == 'A' || c == 'w' || c == 'W' || c == 's' || c == 'S' || c == 'd' || c == 'D')
				MoveWindows(hWnd, c);
			UpdateMiniWindows();
		}
		break;
	}
	case WM_SIZING:
	{
		RECT rc;
		rc.bottom = 290 + 70;
		rc.top = 0;
		rc.left = 0;
		rc.right = 290;
		AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION, FALSE);
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL);
		break;
	}
	case WM_CREATE:
	{
		RECT rc;
		RECT p;
		GetClientRect(hWnd, &p);
		rc.bottom = 60;
		rc.top = 0;
		rc.left = 0;
		rc.right = 270;
		AdjustWindowRect(&rc, WS_CHILD, FALSE);
		scoreWin2 = CreateWindowW(L"pointsWindow", L"asf", WS_CHILD | WS_VISIBLE,
			10, 10, rc.right - rc.left, rc.bottom - rc.top, hWnd, nullptr, hInst, nullptr);
		int gap = 10;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				w2[i][j] = new area();
				int err = GetLastError();
				w2[i][j]->h = CreateWindowW(L"childClass", L"child", WS_CHILD | WS_VISIBLE, i * (gap + 60) + gap, 70 + j * (gap + 60) + gap, 60, 60, hWnd, nullptr, hInst, nullptr);
				SetWindowLongPtrA(w2[i][j]->h, GWLP_USERDATA, (LONG_PTR)w2[i][j]);
				w2[i][j]->val = -1;
				w2[i][j]->rc.left = i * (gap + 60) + gap;
				w2[i][j]->rc.top = 70 + j * (gap + 60) + gap;
				w2[i][j]->rc.right = i * (gap + 60) + gap + 60;
				w2[i][j]->rc.bottom = 70 + j * (gap + 60) + gap + 60;
				w2[i][j]->rc.bottom = 70 + j * (gap + 60) + gap + 60;
				w2[i][j]->x = i;
				w2[i][j]->y = j;
			}
		}
		UpdateMiniWindows();
	}
	break;
	case WM_COMMAND:
	{
		HMENU menu = GetMenu(hWnd);
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case ID_GOALS_8:
			TickMenuBox(8);
			break;
		case ID_GOALS_16:
			TickMenuBox(16);
			break;
		case ID_GOALS_64:
			TickMenuBox(64);
			break;
		case ID_GOALS_2048:
			TickMenuBox(2048);
			break;
		case ID_GAME_NEWGAME:
			NewGame();
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		RECT rc;
		GetClientRect(hWnd, &rc);
		FillRect(hdc, &rc, CreateSolidBrush(RGB(250, 247, 238)));
		if (state != -1)
			PaintEndGameScreeen(hWnd, hdc, ps);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
// Proces messeges for square windows
LRESULT CALLBACK WndProcChild(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_TIMER: {
		if (wParam == 1)
			AnimateRandWin(hWnd);
		if (wParam == 2)
			AnimateMergeWin(hWnd);
		break;
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		Repaint(hWnd, NULL, hdc, ps);
		if (state != -1)
			PaintEndGameScreeen(hWnd, hdc, ps);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
// Proces messeges for score windows
LRESULT CALLBACK WndProcScore(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_CREATE:
	{
		SetFocus(parent); // probably not nessesary
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		PaintScoreWin(hWnd, hdc, ps);
		if (state != -1)
			PaintEndGameScreeen(hWnd, hdc, ps);
		EndPaint(hWnd, &ps);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
