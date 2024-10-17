/*
 * TNFS 1995
 * Windows build w/ OpenWatcom 1.9, DirectDraw
 */
#include <stdio.h>
#include <windows.h>
#include <ddraw.h>
#include <math.h>
#include "tnfs_math.h"
#include "tnfs_base.h"

LPDIRECTDRAW g_pDD = NULL;
LPDIRECTDRAWSURFACE g_pDDSFront = NULL;
LPDIRECTDRAWSURFACE g_pDDSBack = NULL;
HDC hdc = NULL;

char szAppName[] = "DDRAW Win32";
int scale = 10;
int playing;

void message_box(char *str, int val) {
	char text[128];
	sprintf(text, str, val);
	MessageBox(NULL, text, szAppName, MB_OK);
}

int InitDirectDraw(HWND hWnd) {
	DDSURFACEDESC ddsd;
	DDSCAPS ddscaps;
	HRESULT hRet;

	// Create the main DirectDraw object.
	hRet = DirectDrawCreate(NULL, &g_pDD, NULL);
	if (hRet != DD_OK)
		return -1;

	// Get exclusive mode.
	hRet = IDirectDraw_SetCooperativeLevel(g_pDD, hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	if (hRet != DD_OK)
		return -2;

	// Set the video mode
	hRet = IDirectDraw_SetDisplayMode(g_pDD, 800, 600, 32);
	if (hRet != DD_OK)
		return -3;

	// Create the primary surface with 1 back buffer
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = 1;
	hRet = IDirectDraw_CreateSurface(g_pDD, &ddsd, &g_pDDSFront, NULL);
	if (hRet != DD_OK)
		return -4;

	// Get a pointer to the back buffer.
	ZeroMemory(&ddscaps, sizeof(ddscaps));
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	hRet = IDirectDrawSurface_GetAttachedSurface(g_pDDSFront, &ddscaps, &g_pDDSBack);
	if (hRet != DD_OK)
		return -5;

	return 0;
}

void CloseDirectDraw() {
	if (g_pDDSBack)
		IDirectDraw_Release(g_pDDSBack);
	if (g_pDDSFront)
		IDirectDraw_Release(g_pDDSFront);
	if (g_pDD)
		IDirectDraw_Release(g_pDD);
}

void keys_keydown(int charcode) {
	switch (charcode) {
	case 38:
		g_control_throttle = 1;
		break;
	case 40:
		g_control_brake = 1;
		break;
	case 37:
		g_control_steer = -1;
		break;
	case 39:
		g_control_steer = 1;
		break;
	}
}

void keys_keyup(int charcode) {
	switch (charcode) {
	case 38:
		g_control_throttle = 0;
		break;
	case 40:
		g_control_brake = 0;
		break;
	case 37:
		g_control_steer = 0;
		break;
	case 39:
		g_control_steer = 0;
		break;
	case 32:
		player_car_ptr->handbrake = 0;
		break;
	}
}

void keys_keypress(int charcode) {
	switch (charcode) {
	case 27:
		playing = 0;
		break;
	case 32:
		player_car_ptr->handbrake = 1;
		break;
	case 114:
		tnfs_reset_car(player_car_ptr);
		break;
	case 97:
		tnfs_change_gear_up();
		break;
	case 122:
		tnfs_change_gear_down();
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_KEYDOWN:
		keys_keydown(wParam);
		break;
	case WM_KEYUP:
		keys_keyup(wParam);
		break;
	case WM_CHAR:
		keys_keypress(wParam);
		break;
	case WM_DESTROY:
		CloseDirectDraw();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void clearBackBuffer() {
	DDBLTFX ddbltfx;
	ZeroMemory(&ddbltfx, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = 0;
	IDirectDrawSurface_Blt(g_pDDSBack, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
}

void drawText(char *szMsg, int x, int y) {
	SIZE size;
	SetTextColor(hdc, RGB(255, 255, 0));
	GetTextExtentPoint(hdc, szMsg, lstrlen(szMsg), &size);
	TextOut(hdc, x, y, szMsg, lstrlen(szMsg));
}

void drawRect(float x, float y, double a, float l, float w) {
	float cay, cax, cby, cbx;
	double s, c;

	scale = 10;
	s = sin(a);
	c = cos(a);
	cay = w * s + l * c;
	cax = w * c - l * s;
	cby = w * s - l * c;
	cbx = w * c + l * s;

	MoveToEx(hdc, (x + cax) * scale, (y + cay) * scale, NULL);
	LineTo(hdc, (x + cbx) * scale, (y + cby) * scale);
	LineTo(hdc, (x - cax) * scale, (y - cay) * scale);
	LineTo(hdc, (x - cbx) * scale, (y - cby) * scale);
	LineTo(hdc, (x + cax) * scale, (y + cay) * scale);
}

void drawRoad(float ox, float oy, int node) {
	int i, j, n;

	for (n = 0; n < 12; n++) {

		i = node - 4 + n;
		if (i < 0) {
			i = i + g_road_node_count;
		} else if (i >= g_road_node_count) {
			i = i - g_road_node_count;
		}
		j = i + 1;

		MoveToEx(hdc, (int) ((-track_data[i].vf_margin_L.z - ox) * scale), (int) ((track_data[i].vf_margin_L.x - oy) * scale), NULL);
		LineTo(hdc, (int) ((-track_data[j].vf_margin_L.z - ox) * scale), (int) ((track_data[j].vf_margin_L.x - oy) * scale));
		LineTo(hdc, (int) ((-track_data[j].vf_margin_R.z - ox) * scale), (int) ((track_data[j].vf_margin_R.x - oy) * scale));
		LineTo(hdc, (int) ((-track_data[i].vf_margin_R.z - ox) * scale), (int) ((track_data[i].vf_margin_R.x - oy) * scale));
		LineTo(hdc, (int) ((-track_data[i].vf_margin_L.z - ox) * scale), (int) ((track_data[i].vf_margin_L.x - oy) * scale));
	}
}

void drawCar(tnfs_car_data * car, float x, float y) {
	float a, as, s, c, cay, cax, cby, cbx;

	a = (float) car->angle.y / 2670179; //to radians
	as = a + ((float) car->steer_angle / 2670179);

	//body
	drawRect(x, y, a, 1, 2);

	//wheels
	s = sin(a);
	c = cos(a);
	cay = 1.25 * s + 0.9 * c;
	cax = 1.25 * c - 0.9 * s;
	cby = 1.25 * s - 0.9 * c;
	cbx = 1.25 * c + 0.9 * s;
	drawRect(x + cax, y + cay, as, 0.1, 0.3);
	drawRect(x + cbx, y + cby, as, 0.1, 0.3);
	drawRect(x - cax, y - cay, a, 0.1, 0.3);
	drawRect(x - cbx, y - cby, a, 0.1, 0.3);
}

void render() {
	int i;
	char hud[128];
	float x0, y0, x1, y1;
	HPEN hPen;

	clearBackBuffer();

	// ddraw
	IDirectDrawSurface_GetDC(g_pDDSBack, &hdc);
	hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	SelectObject(hdc, hPen);
	SetBkColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, CreateSolidBrush(RGB(0, 255, 255)));

	//inverted axis from 3d world
	x0 = (float) player_car_ptr->position.z / 0x10000; //to meter scale
	y0 = (float) player_car_ptr->position.x / 0x10000;
	drawRoad(x0 - 30, y0 - 30, player_car_ptr->track_slice);
	drawCar(player_car_ptr, 30, 30);

	for (i = 1; i < g_total_cars_in_scene; i++) {
		x1 = x0 - ((float)g_car_ptr_array[i]->position.z / 0x10000);
		y1 = y0 - ((float)g_car_ptr_array[i]->position.x / 0x10000);
		drawCar(g_car_ptr_array[i], 30 - x1, 30 - y1);
	}

	// hud text
	sprintf(hud, "%d m/s - %d rpm - gear %d", player_car_ptr->speed_local_lon >> 16, player_car_ptr->rpm_engine, player_car_ptr->gear_selected + 1);
	drawText(&hud, 10, 10);

	IDirectDrawSurface_ReleaseDC(g_pDDSBack, &hdc);
}

int WINAPI WinMain( HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow ) {
	MSG Msg;
	WNDCLASS wndclass;
	HWND hwnd;
	int ret;
	char *trifile;

	//wndclass.cbSize = sizeof(wndclass); //ex
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	//wndclass.lpfnWndProc = (LPVOID) WindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = this_inst;
	//wndclass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wndclass.hIcon = (HICON)0;
	//wndclass.hCursor = LoadCursor(NULL, IDC_WAIT);
	wndclass.hCursor = LoadCursor((HINSTANCE)0, IDC_ARROW );
	//wndclass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH); //blackbrush
	wndclass.hbrBackground = GetStockObject( WHITE_BRUSH );
	wndclass.lpszMenuName = "DirectDraw Window";
	wndclass.lpszClassName = szAppName;
	//wndclass.hIconSm = LoadIcon(NULL, IDI_WINLOGO); //ex

	RegisterClass(&wndclass);

	hwnd = CreateWindow(szAppName,// window class name
			szAppName,// window caption
			WS_OVERLAPPEDWINDOW,//WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,  // window style
			CW_USEDEFAULT, CW_USEDEFAULT,// initial x & y position
			640, 480,
			NULL,// parent window handle
			NULL,// window menu handle
			this_inst,// program instance handle
			NULL);// creation parameters

	ShowWindow(hwnd, cmdshow);
	UpdateWindow(hwnd);

	ret = InitDirectDraw(hwnd);
	if (ret != 0) {
		message_box("ERROR: InitDirectDraw %d", ret);
		return 0;
	}

	trifile = 0;
	tnfs_init_sim("track.tri");

	playing = 1;
	while(playing) {
		if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
			if (Msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

		tnfs_update();
		render();
		IDirectDrawSurface_Flip(g_pDDSFront, NULL, DDFLIP_WAIT);
		Sleep(30);
	}

	CloseDirectDraw();

	return Msg.wParam;
}
