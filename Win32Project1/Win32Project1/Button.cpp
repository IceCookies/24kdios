#include <windows.h>
#include <GdiPlus.h>
#include "Button.h"

Button::Button() 
{
}

Button::Button(int x, int y, 
			   int width, int height, 
			   WCHAR* content,
			   HWND hParent,
			   HINSTANCE hInstance,
			   BtnHandler handler)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->content = content;
	this->parent = hParent;
	this->hInstance = hInstance;
	this->Handler = handler;
}

Button::~Button(void)
{

}

HWND Button::Create()
{
	HWND hwnd = CreateWindow(TEXT("Button"),
			            NULL,
						WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,
						x,
						y,
						width,
						height,
						parent,
						NULL,
						hInstance,
						(LPVOID)this);

	if (SetProp(hwnd, "NEWButton", (HANDLE)this) == 0) 
	{
		return NULL;
	}

	this->state = BUTTON_RELEASE;
	this->OrginProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)Proc);
	self = hwnd;
	return hwnd;

}

void Button::SetContent(WCHAR* content) 
{
	this->content = content;
	InvalidateRect(this->self, NULL, TRUE);	
}

int Button::GetState() 
{
	return this->state;
}

void Button::SetState(int state) 
{
	this->state = state;
}

LRESULT CALLBACK Button::Proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Button* w = (Button*)GetProp(hwnd, "NEWButton");
	return w->ButtonProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK Button::ButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	switch (message)
	{	
	case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint(hwnd, &ps);

			Gdiplus::Graphics graphics(hdc);
			Gdiplus::SolidBrush solidBrush1(Gdiplus::Color(255, 69, 133, 243));
            graphics.FillRectangle(&solidBrush1, 0, 0, 120, 30);


			Gdiplus::FontFamily  fontFamily(L"Arial");
			Gdiplus::Font        font(&fontFamily, 16, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
			Gdiplus::PointF      pointF(0.0f, 5.0f);
			Gdiplus::SolidBrush  solidBrush2(Gdiplus::Color(255, 255, 255, 255));
			graphics.DrawString(this->content, -1, &font, pointF, &solidBrush2);
			graphics.ReleaseHDC(hdc);

			EndPaint(hwnd, &ps);
			return 0;
		}	

	case WM_LBUTTONUP:
		{
			Handler();
			//SendMessage(hwnd, WM_PAINT, wParam, lParam);
        }

	default:
		return CallWindowProc(OrginProc, hwnd, message, wParam, lParam);
	}
}





