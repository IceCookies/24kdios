#include <Windows.h>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <opencv\cvaux.h>

#include "OpenFileDialog.h"
#include "Utils.h"


Utils::Utils(void)
{
}


Utils::~Utils(void)
{
}

void Utils::SetWindow(char* name, HWND hParent, int x, int y) 
{
	HWND hVideo = (HWND)cvGetWindowHandle(name);
	HWND hVideoParent = GetParent(hVideo);
		
	if(hVideoParent != NULL)
	{
		SetWindowLong(hVideoParent,GWL_STYLE,((GetWindowLong(hVideoParent,GWL_STYLE) & ~WS_POPUP) & ~WS_OVERLAPPEDWINDOW ) | WS_CHILD );
		//SetWindowLong(hVideoParent,GWL_EXSTYLE,GetWindowLong(hVideoParent,GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE  & ~WS_EX_WINDOWEDGE  );
		POINT point;
		point.x = x;
		point.y = y;
		bool result = ScreenToClient(hParent, &point);
		if(result)
		{
			SetWindowPos(hVideoParent,HWND_TOP, point.x, point.y, 0, 0, SWP_SHOWWINDOW );

		}
		SetParent(hVideoParent, hParent);

		//SetWindowPos(hVideoParent,HWND_TOP, 0, 0, 100, 100, SWP_SHOWWINDOW);
	}
}

char* Utils::GetNativeFile() 
{
	char* fileName;
	OpenFileDialog* openFileDialog = new OpenFileDialog();		
	openFileDialog->FilterIndex = 1;

	openFileDialog->Title = "Open Video File";

	if (openFileDialog->ShowDialog())
	{
		fileName = openFileDialog->FileName;
		if (fileName == 0)
		{
			MessageBox(NULL, "Can not Open file", "Warning", MB_OK);
			return 0;
		}				
		else 
		{
			return fileName;
		}
	}
	else
	{
		MessageBox(NULL, "Can not Open file", "Error", MB_OK);
		return 0;
	}
}

