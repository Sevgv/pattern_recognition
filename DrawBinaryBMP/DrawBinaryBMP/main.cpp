
#include <tchar.h>
#include <iostream>
#include "bmpstruct.h"
#include <windows.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildProc(HWND, UINT, WPARAM, LPARAM);

TCHAR WinName[] = _T("MainFrame");
TCHAR ChildName[] = _T("ChildFrame");

HINSTANCE hInst;

class FileBMP
{
private:
	unsigned short read_u16(FILE *fp)
	{
		unsigned char b0, b1;

		b0 = getc(fp);
		b1 = getc(fp);

		return b1 << 8 | b0;
	}


	unsigned int read_u32(FILE *fp)
	{
		unsigned char b0, b1, b2, b3;

		b0 = getc(fp);
		b1 = getc(fp);
		b2 = getc(fp);
		b3 = getc(fp);

		return ((b3 << 8 | b2) << 8 | b1) << 8 | b0;
	}


	int read_s32(FILE *fp)
	{
		unsigned char b0, b1, b2, b3;

		b0 = getc(fp);
		b1 = getc(fp);
		b2 = getc(fp);
		b3 = getc(fp);

		int value = (int)(((b3 << 8 | b2) << 8 | b1) << 8 | b0);
		if (value < 0)
		{
			value = ~value;
			value++;
		}

		return value;
	}

public:

	BMPFILEHEADER header;
	BMPINFOHEADER bmiHeader;

	RGBQ *Palette = new RGBQ[256];

	int** arrIndexes;

	bool bynary_color(int b, int g, int r)
	{
		int limit = 128;
		bool blue;
		bool green;
		bool red;

		if (b >= limit)
			blue = true;
		else
			blue = false;
		if (g >= limit)
			green = true;
		else
			green = false;
		if (r >= limit)
			red = true;
		else
			red = false;

		if (blue || green || red)
			return true;
		else
			return false;

	}

	int contrast_color(int color)
	{
		if (color >= 0 && color < 64)
		{
			return 0;
		}
		if (color >= 64 && color <= 128)
		{
			double temp = 0;
			temp = 0.03125*pow((color - 64), 2);
			return static_cast<int>(round(temp));
		}
		if (color > 18 && color <= 192)
		{
			double temp = 0;
			temp = -0.03125*pow((color - 192), 2) + 255;
			return static_cast<int>(round(temp));
		}
		if (color > 192 && color <= 255)
		{
			return 255;
		}
		return -1;

	}

	int contrast_color2(int color)
	{
		if (color >= 0 && color <= 255)
		{
			double temp = 0;
			temp = (25*pow((color - 128), 1.0/3)) + 128;
			if (temp < 0)
				return -1;
			return static_cast<int>(round(temp));
		}
		return -1;
	}

	int contrast_color3(int color)
	{
		if (color >= 0 && color <= 255)
		{
			double temp = 0;
			temp = 16*sqrt(color);
			if (temp < 0)
				return -1;
			return static_cast<int>(round(temp));
		}
		return -1;
	}


	void init(FILE *pFile)
	{
		// считываем заголовок файла
		header.bfType = read_u16(pFile);
		header.bfSize = read_u32(pFile);
		header.bfReserved1 = read_u16(pFile);
		header.bfReserved2 = read_u16(pFile);
		header.bfOffBits = read_u32(pFile);

		// считываем заголовок изображения
		bmiHeader.biSize = read_u32(pFile);
		bmiHeader.biWidth = read_s32(pFile);
		bmiHeader.biHeight = read_s32(pFile);
		bmiHeader.biPlanes = read_u16(pFile);
		bmiHeader.biBitCount = read_u16(pFile);
		bmiHeader.biCompression = read_u32(pFile);
		bmiHeader.biSizeImage = read_u32(pFile);
		bmiHeader.biXPelsPerMeter = read_s32(pFile);
		bmiHeader.biYPelsPerMeter = read_s32(pFile);
		bmiHeader.biClrUsed = read_u32(pFile);
		bmiHeader.biClrImportant = read_u32(pFile);

		for (int i = 0; i < 256; i++)
		{				
			Palette[i].rgbBlue = getc(pFile);
			Palette[i].rgbGreen = getc(pFile);
			Palette[i].rgbRed = getc(pFile);
			Palette[i].rgbReserved = getc(pFile);
		}

		arrIndexes = new int*[bmiHeader.biHeight];
		for (int i = 0; i < bmiHeader.biHeight; i++)
			arrIndexes[i] = new int[bmiHeader.biWidth];

		for (int i = 0; i < bmiHeader.biHeight; i++) {
			for (int j = 0; j < bmiHeader.biWidth; j++) {
				arrIndexes[i][j] = getc(pFile);
			}
			// пропускаем последний байт в строке
			// getc(pFile);
			// getc(pFile);
		}

	}

	int get_Width() { return bmiHeader.biWidth; }
	int get_Height() { return bmiHeader.biHeight; }

};




// TCHAR mainMessage[] = L"Какой то-текст!"; // строка с сообщением

FileBMP* fBMP;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	FILE *pFile;

	// errno_t e = fopen_s(&pFile, "file.bmp", "rb"); // 24 - разряда
	// errno_t e = fopen_s(&pFile, "file1.bmp", "rb"); // монохромный
	errno_t e = fopen_s(&pFile, "F44_2.bmp", "rb"); // 256 цветов
													// errno_t e = fopen_s(&pFile, "F00046_24.bmp", "rb"); // 256 цветов (маленький)
													// errno_t e = fopen_s(&pFile, "2.bmp", "rb"); // 256 цветов (маленький)


	if (e)
	{
		MessageBox(NULL, L"Ошибка, файл не найден!", L"Ошибка", MB_OK);
		return NULL;
	}



	// FILE *pFile;
	// errno_t e = fopen_s(&pFile, "F1_7.bmp", "rb");
	fBMP = new FileBMP;
	fBMP->init(pFile);


	HWND hMainWnd; // дескриптор окна
	MSG msg; // структура сообщения
	WNDCLASS w1, w2; // структура класса окна

					 // Регистрация класса окна
	memset(&w1, 0, sizeof(WNDCLASS));
	w1.style = CS_HREDRAW | CS_VREDRAW;
	w1.lpfnWndProc = WndProc; // имя оконной функции
	w1.hInstance = hInstance;
	w1.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	w1.lpszClassName = WinName;

	if (!RegisterClass(&w1)) return 0;

	// Регистрация класса окна
	memset(&w2, 0, sizeof(WNDCLASS));
	w2.style = CS_HREDRAW | CS_VREDRAW;
	w2.lpfnWndProc = ChildProc; // имя оконной функции
	w2.hInstance = hInstance;
	w2.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	w2.lpszClassName = ChildName;

	if (!RegisterClass(&w2)) return 0;

	// Создание окна
	hMainWnd = CreateWindow(WinName,
		_T("Обычное изображение"),
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
		0,
		0,
		fBMP->get_Width() * 1.05,
		fBMP->get_Height() * 1.1,
		HWND_DESKTOP,
		NULL,
		hInstance,
		NULL);

	if (!hMainWnd) {
		// в случае некорректного создания окошка (неверные параметры и тп):
		MessageBox(NULL, L"Не получилось создать окно!", L"Ошибка", MB_OK);
		return NULL;
	}
	ShowWindow(hMainWnd, nCmdShow); // отображение
	UpdateWindow(hMainWnd);          // перерисовка

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;	
}

LRESULT WINAPI WndProc(HWND hWnd, UINT Message, WPARAM wparam, LPARAM lparam)
{
	HDC hDC; // создаём дескриптор устройства
	PAINTSTRUCT ps; // структура, сод-щая информацию о клиентской области (размеры, цвет и тп)

	COLORREF* color = new COLORREF[256];
	for (int i = 0; i < 256; i++)
	{
		color[i] = RGB(i, i, i);
	}

	switch (Message)
	{
	case WM_PAINT: // если нужно нарисовать, то:
		hDC = BeginPaint(hWnd, &ps); // инициализируем контекст устройства
									 // BitBlt(hDC, 0, 0, Width, Height, memBit, 0, 0, SRCCOPY);//вывод изображения

		for (int i = 0; i < fBMP->get_Height(); i++) {
			for (int j = 0; j < fBMP->get_Width(); j++) {
				SetPixel(hDC, j, i, color[fBMP->arrIndexes[i][j]]);
			}
		}

		EndPaint(hWnd, &ps); // заканчиваем рисовать
		break;
	case WM_KEYDOWN:
		switch (wparam) {
		case VK_ESCAPE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case VK_SPACE:
			CreateWindow(ChildName,
				_T("Бинаризованное изображение"),
				WS_VISIBLE |
				WS_CAPTION |
				WS_SYSMENU |
				WS_MINIMIZEBOX |
				WS_MAXIMIZEBOX,
				600,
				300,
				fBMP->get_Width() * 1.05,
				fBMP->get_Height() * 1.1,
				hWnd,
				NULL,
				hInst,
				NULL);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, Message, wparam, lparam);
	}
	return 0;
}

LRESULT WINAPI ChildProc(HWND hWnd, UINT Message, WPARAM wparam, LPARAM lparam)
{
	HDC hDC; // создаём дескриптор устройства
	PAINTSTRUCT ps; // структура, сод-щая информацию о клиентской области (размеры, цвет и тп)

	COLORREF* colorBlack = new COLORREF[256];
	for (int i = 0; i < 256; i++)
	{
		if (fBMP->bynary_color(fBMP->Palette[i].rgbRed, fBMP->Palette[i].rgbGreen, fBMP->Palette[i].rgbBlue))
		{
			colorBlack[i] = RGB(255, 255, 255);
		}
		else
		{
			colorBlack[i] = RGB(0, 0, 0);
		}

		 // if(fBMP->contrast_color(fBMP->Palette[i].rgbReserved) == -1)
		 // {
		 // 	int a = 0;
		 // }
   //
		 // int color = fBMP->contrast_color(fBMP->Palette[i].rgbReserved);
   //
		 // colorBlack[i] = RGB(color, color, color);
		
	}

	switch (Message)
	{
	case WM_PAINT: // если нужно нарисовать, то:
		hDC = BeginPaint(hWnd, &ps); // инициализируем контекст устройства
		// BitBlt(hDC, 0, 0, Width, Height, memBit, 0, 0, SRCCOPY);//вывод изображения

		 for (int i = 0; i < fBMP->get_Height(); i++) {
		 	for (int j = 0; j < fBMP->get_Width(); j++) {
		 		SetPixel(hDC, j, i, colorBlack[fBMP->arrIndexes[i][j]]); 
		 	}
		 }

		EndPaint(hWnd, &ps); // заканчиваем рисовать
		break;
	// case WM_DESTROY:
	// 	PostQuitMessage(0);
	// 	break;
	case WM_KEYDOWN:
		switch (wparam) {
		case VK_ESCAPE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	default:
		return DefWindowProc(hWnd, Message, wparam, lparam);
	}
	return 0;
}





