#pragma once

typedef struct ClipResult_PTR
{
	int x;
	int y;
	int width;
	int height;
	IplImage* img;
}ClipResult, *LPClipResult;

typedef struct MyPoint_PTR
{
	union 
	{
		CvPoint point;
		struct 
		{
			int x, y;
		};
	};
	double value;
}MyPoint, *LPMyPoint;

class Utils
{
public:
	Utils(void);
	~Utils(void);

public:
	static void SetWindow(char* name, HWND hParent, int x, int y);
	static char* GetNativeFile();
};

