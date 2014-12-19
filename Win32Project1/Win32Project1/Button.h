#pragma once


//typedef LRESULT (CALLBACK *BtnWndProc)(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

typedef void (*BtnHandler)();
#define BUTTON_PRESSED  1
#define BUTTON_RELEASE  0

class Button
{
public:
	Button::Button();
	Button::Button(int x, int y, int width, int height, WCHAR* content, HWND hParent, HINSTANCE, BtnHandler);
	~Button(void);

	HWND Create();
	void SetContent(WCHAR* content);
	int  GetState();
	void SetState(int state);
	HWND test(HWND parent, HINSTANCE hinstance);
	static LRESULT CALLBACK Proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT CALLBACK ButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	//friend LRESULT CALLBACK BtnProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	

private:
	int x;
	int y;
	int width;
	int height;
	WCHAR* content;
	int state;
	//int color;
	//int bgColor;

	HWND parent;
	HWND self;
	HINSTANCE hInstance;

	WNDPROC OrginProc;
	BtnHandler Handler;

};

