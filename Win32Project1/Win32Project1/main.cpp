#include <windows.h>
#include <GdiPlus.h>
#include <string.h>
#include <fstream>

#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <opencv\cvaux.h>

#include "OpenFileDialog.h"
#include "Utils.h"
#include "Button.h"

#pragma comment (lib, "Gdiplus.lib")

using namespace Gdiplus;
using namespace std;

#define ID_TIMER    1  
#define TEMPLATE_INC 3

WNDPROC OrginProc;
HINSTANCE hApp;
HWND      hMainWindow;

HWND hTestBtn;
HWND hPauseBtn;
HWND hShowBtn;

Button* pauseBtn;

CvCapture* pcapture = NULL;
IplImage* pFrameImg = NULL;
IplImage* frameImgBackup = NULL;
ClipResult clipResult = {0, 0, 0, 0, NULL};
LPClipResult pClipResult = NULL;
//IplImage* dst = NULL;
char* szClassName = "MainWindow";
int count_center_point = 0;
MyPoint cen_point[1000000];


int paused = 0;
CvPoint pt1 = cvPoint(0,0);    
CvPoint pt2 = cvPoint(0,0);    
bool is_selecting = false;  
cv::Rect selection;

MyPoint carPostion = {-1, -1, 0};

int test = 0;


int numFrames = 0;

POINT ptOne,ptTwo;   //画图的起点和终点
bool isPaint = false;

LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
VOID    CALLBACK TimerProc(HWND, UINT, UINT, DWORD);  
void reverseImgColor(IplImage* item);
void training();
void recognize();

void cvMouseCallback(int mouseEvent,int x,int y,int flags,void* param)    
{    
    switch(mouseEvent)    
    {    
    case CV_EVENT_LBUTTONDOWN:    
        pt1 = cvPoint(x,y);    
        pt2 = cvPoint(x,y);  
		selection = cv::Rect(x ,y ,0, 0);
        is_selecting = true;    
        break;    
    case CV_EVENT_MOUSEMOVE:   
		if(is_selecting)  
            pt2 = cvPoint(x,y);  
        break;    
    case CV_EVENT_LBUTTONUP:    
        pt2 = cvPoint(x,y);    
        is_selecting = false;   
		cvSetImageROI(pFrameImg, cvRect(pt1.x, pt1.y, std::abs(pt2.x - pt1.x), std::abs(pt2.y - pt1.y)));  
		IplImage* dst = cvCreateImage(cvSize(std::abs(pt2.x-pt1.x), std::abs(pt2.y-pt1.y)),  
                            IPL_DEPTH_8U,  
                            pFrameImg->nChannels);  
		cvCopy(pFrameImg, dst, 0);
		clipResult.x = (pt1.x + pt2.x) / 2;
		clipResult.y = (pt1.y + pt2.y) / 2;
		clipResult.width = std::abs(pt2.x - pt1.x);
		clipResult.height = std::abs(pt2.y - pt1.y);
		clipResult.img = dst;

		pClipResult = &clipResult;

		cvResetImageROI(pFrameImg);  
		
		cvNamedWindow("newimg", 1);
		cvMoveWindow("newimg", 0, 0);
		Utils::SetWindow("newimg", hMainWindow, 800, 50);
		cvShowImage("newimg", dst);
        break;    
    }    
    return;    
}  

VOID CALLBACK TimerProc(HWND hwnd, UINT message, UINT iTimerID, DWORD dwTime)  
{
	
	pFrameImg = cvQueryFrame(pcapture);
	numFrames++;
	if (pFrameImg == NULL) 
	{
		KillTimer(hMainWindow, ID_TIMER);
		return;
	}
	frameImgBackup = pFrameImg;
	if (pClipResult != NULL) 
	{

		

		int width = pClipResult->width;    //截图的大小
		int height = pClipResult->height;
		int x = pClipResult->x - width / 2;   //截图的左上角坐标
		int y = pClipResult->y - height / 2;
		IplImage* templateImg = pClipResult->img;  

		int xTemplate;
		int yTemplate;
		if (carPostion.x != -1) 
		{
			xTemplate = carPostion.x - TEMPLATE_INC;
			yTemplate = carPostion.y - TEMPLATE_INC;
		}
		else 
		{
			xTemplate = x - TEMPLATE_INC;
			yTemplate = y - TEMPLATE_INC;	
		}
		cvSetImageROI(pFrameImg, cvRect(xTemplate, yTemplate, width + 2 * TEMPLATE_INC, height + 2 * TEMPLATE_INC));  
		IplImage* target = cvCreateImage(cvSize(width + 2 * TEMPLATE_INC, height + 2 * TEMPLATE_INC),  
                                         IPL_DEPTH_8U,  
                                         pFrameImg->nChannels);  

		
		cvCopy(pFrameImg, target, 0);
		cvResetImageROI(pFrameImg);  
		int resultWidth = target->width - templateImg->width + 1;
		int resultHeight = target->height - templateImg->height + 1;
		IplImage* resultImg = cvCreateImage(cvSize(resultWidth, resultHeight), 32, 1);
		cvMatchTemplate(target, templateImg, resultImg, CV_TM_CCOEFF);
		double minValue, maxValue;
		CvPoint minPoint;
		CvPoint maxPoint;

		cvMinMaxLoc(resultImg, &minValue, &maxValue, &minPoint, &maxPoint);

		carPostion.x = xTemplate + maxPoint.x;
		carPostion.y = yTemplate + maxPoint.y;
		
		cen_point[count_center_point].x = carPostion.x + width / 2;
		cen_point[count_center_point].y = carPostion.y + height / 2;
			//cen_point[count_center_point].value = maxValue;
		count_center_point++;
		
			

			//cvShowImage("target", target);
			//cvShowImage("result", resultImg);

			//KillTimer(hMainWindow, ID_TIMER);
	}

	//frameImgBackup = cvCreateImage(cvSize(pFrameImg->width, pFrameImg->height), IPL_DEPTH_8U, pFrameImg->nChannels);
	//cvCopy(pFrameImg, frameImgBackup, 0);
	cvShowImage("video", pFrameImg);
}  

void testBtnHandler() 
{
	pcapture = cvCreateFileCapture(Utils::GetNativeFile());
	
	cvNamedWindow("video", 1);
	cvMoveWindow("video", 0, 0);
	Utils::SetWindow("video", hMainWindow, 0, 50);

	SetTimer(hMainWindow, ID_TIMER, 30, TimerProc);  
}

void PauseBtnHandler()
{
	if (pauseBtn->GetState() == BUTTON_RELEASE) 
	{
		paused = 1;
		KillTimer(hMainWindow, ID_TIMER);
		pauseBtn->SetContent(L"Resume");
		pauseBtn->SetState(BUTTON_PRESSED);
		cvSetMouseCallback("video", cvMouseCallback);
	}
	else 
	{
		paused = 0;
		SetTimer(hMainWindow, ID_TIMER, 30, TimerProc);  
		test = 1;
		pauseBtn->SetContent(L"Pause");
		pauseBtn->SetState(BUTTON_RELEASE);
		cvSetMouseCallback("video", NULL);
	}
	
	
}

void showBtnHandler() 
{
	IplImage* pTrackImg = cvCreateImage(cvGetSize(frameImgBackup), 8, 3);
	cvZero(pTrackImg);
	CvScalar color = {0, 255, 0};
	cvNamedWindow("Track", 1);
	for (int i = 0; i < count_center_point - 1; i++) 
	{
		cvLine(pTrackImg, cen_point[i].point, cen_point[i + 1].point, color);
		//cvWaitKey(100);
		cvShowImage("Track", pTrackImg);
	}

	std::fstream fs;
	fs.open("out.txt", std::fstream::in | std::fstream::out | std::fstream::app);
	while (count_center_point > 0) 
	{
		int x = cen_point[count_center_point - 1].x;
		int y = cen_point[count_center_point - 1].y;
		fs << "(" << x << ", " <<  y << ")" << std::endl;
		count_center_point--;
		
	}
	fs.close();
	
	
}


int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{

	Gdiplus::GdiplusStartupInput gdiInput;
	ULONG_PTR gdiplusStartupToken;
	Gdiplus::GdiplusStartup(&gdiplusStartupToken,&gdiInput,NULL);

	memset(cen_point, 0, sizeof(cen_point));


	WNDCLASSEX wndclass;
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc   = WinProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szClassName;
	wndclass.hIconSm       = NULL;

	RegisterClassEx(&wndclass);

	HWND hwnd = ::CreateWindowEx(
		0,
		szClassName,
		"My First Window",
		WS_OVERLAPPEDWINDOW  ,
		100,
		100,
		640,
		480,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, "Error in Create Window", "Error", MB_OK);
		return -1;
	}

	hApp = hInstance;
	hMainWindow = hwnd;

	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);


	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Gdiplus::GdiplusShutdown(gdiplusStartupToken);


	return msg.wParam;	
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	HWND showResultBtn; //显示结果按钮

	switch(message)
	{
	case WM_CREATE:
		{
			showResultBtn = CreateWindow(TEXT("button"),//必须为：button    
                        TEXT("显示结果"),             //按钮上显示的字符    
                        WS_CHILD | WS_VISIBLE,  
                        0,0,100,40,  
                        hWnd,(HMENU)0,  
                        ((LPCREATESTRUCT)lParam)->hInstance,NULL);   

			return 0;
		}
	case WM_COMMAND:
		{
			training();
			recognize();
			return 0;
		}
	case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint(hWnd, &ps);

			GetWindowRect(hWnd, &rect);

			POINT point;
			point.x = 300;
			point.y = 200;
			
			HPEN hpen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
			SelectObject(hdc,hpen);
			::MoveToEx(hdc,150,0,&point);
			::LineTo(hdc,150,500);

			::MoveToEx(hdc,350,0,&point);
			::LineTo(hdc,350,500);

			::MoveToEx(hdc,0,100,&point);
			::LineTo(hdc,600,100);

			::MoveToEx(hdc,0,400,&point);
			::LineTo(hdc,600,400);
			EndPaint(hWnd, &ps);
			return 0;
		}
	case WM_LBUTTONDOWN:
		{
			isPaint = true;
			
			ptOne.x = (int)(short)LOWORD(lParam);
			ptOne.y = (int)(short)HIWORD(lParam);

		    return 0;
		}
	case WM_MOUSEMOVE:
		{
			if (isPaint) 
			{
				ptTwo.x = (int)(short)LOWORD(lParam);
				ptTwo.y = (int)(short)HIWORD(lParam);
				
				HDC hdc = GetDC(hMainWindow);

				HPEN hpen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
				SelectObject(hdc,hpen);
				::MoveToEx(hdc,ptOne.x,ptOne.y,NULL);
				::LineTo(hdc,ptTwo.x,ptTwo.y);
			 
				ptOne = ptTwo;
			}
				
			return 0;	
			
		}
	case WM_LBUTTONUP:
		{
			isPaint = false;
			
		    return 0;
		}

	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*训练*/
void training()
{
	int i=0,j,p,k;
    int flag;
    uchar* ptr;
	int sample[5][5];

    int width,height;
    int bottom,top,left,right;
    IplImage *src,*img_gray,*img_binary;
	
	IplImage *bg, *model;
	bg = cvLoadImage("H:\\trainning\\1.png");
    model = cvLoadImage("H:\\trainning\\l2.png");
	src = cvCreateImage(cvSize(bg->width, bg->height),IPL_DEPTH_8U,bg->nChannels);
	cvAbsDiff(model, bg, src);  //获取轨迹
	reverseImgColor(src);  //反转图片颜色

	//src = cvLoadImage("H:\\trainning\\2.png");
	cvShowImage("轨迹图片", src);
	cvWaitKey(20);

    img_binary = cvCreateImage(cvGetSize(src),src->depth,1);
    img_gray = cvCreateImage(cvGetSize(src),src->depth,1);
    height = src->height;
    width = src->width;

	cvCvtColor(src,img_gray,CV_BGR2GRAY);
    cvThreshold(img_gray,img_binary,145,255,CV_THRESH_BINARY);

	ptr = (uchar*) img_binary->imageData;

    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            flag = 0;
            if(ptr[i*img_binary->widthStep+j]==0)
            {
                flag = 1;
                top = i;
                break;
            }
        }
        if(flag)
        {
            break;
        }
    }

    for(i=height-1;i>0;i--)
    {
        for(j=0;j<width;j++)
        {
            flag = 0;
            if(ptr[i*img_binary->widthStep+j]==0)
            {
                flag = 1;
                bottom = i;
                break;
            }
        }
        if(flag)
        {
            break;
        }
    }

    for(i=0;i<width;i++)
    {
        for(j=0;j<height;j++)
        {
            flag = 0;
            if(ptr[j*img_binary->widthStep+i]==0)
            {
                flag = 1;
                left = i;
                break;
            }
        }
        if(flag)
        {
            break;
        }
    }

    for(i=width-1;i>0;i--)
    {
        for(j=0;j<height;j++)
        {
            flag = 0;
            if(ptr[j*img_binary->widthStep+i]==0)
            {
                flag = 1;
                right = i;
                break;
            }
        }
        if(flag)
        {
            break;
        }
    }

    int w = (right-left)/5;
    int h = (bottom-top)/5;

    for(p=0;p<5;p++)
    {
        for(k=0;k<5;k++)
        {
            int count = 0;
            for(int t=top+k*h;t<top+(k+1)*h;t++)
            {
                for(int r=left+p*w;r<left+(p+1)*w;r++)
                {
                    if(ptr[t*img_binary->widthStep+r]==0)
                    {
                        count++;
                    }
                }
            }
            if(count > 0)
            {
                sample[p][k] = 1;
            }else
                sample[p][k] = 0;
        }
    }

	/*  提取特征向量作为分类器
	ofstream outFile("train.txt", ios::out|ios::app);
	   for(int i=0;i<5;i++)
            {
                for(int j=0;j<5;j++)
                {
                    outFile << sample[i][j] << ",";
                }
            }
	   outFile << endl;
	   */

	//提取特征向量
	ofstream outFile("train1.txt", ios::out);
	   for(int i=0;i<5;i++)
            {
                for(int j=0;j<5;j++)
                {
                    outFile << sample[i][j] << ",";
                }
            }
	   outFile << endl;
	   outFile.close();
}

/*识别*/
void recognize()
{
    float labels[15] = {1.0, 2.0, 3.0, 1.0, 2.0, 3.0, 1.0, 2.0, 3.0, 1.0, 2.0, 3.0, 1.0, 2.0, 3.0};
    cv::Mat labelsMat(15, 1, CV_32FC1, labels);     //1_3

    float trainingData[15][25] = 
        {{0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0},
         {0,0,0,1,1,0,0,0,1,1,0,0,1,1,1,0,0,1,1,0,1,1,1,0,0},
         {1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
         {0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0},
         {0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
         {0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0,0,1},
         {0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0},
         {0,0,0,0,1,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0},
         {1,1,1,0,1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,0},
         {0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0},
         {0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1},
         {0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1},
         {0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,0,0,1,1,0,1,1,1,0,0},
         {0,0,1,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0},
         {0,0,0,0,1,1,1,1,1,0,0,1,1,1,0,0,0,0,1,1,0,0,0,0,1}};

    cv::Mat trainingDataMat(15, 25, CV_32FC1, trainingData);
	
    // step 2: 设置SVM参数
    CvSVMParams params;
    params.svm_type = CvSVM::C_SVC;
    params.kernel_type = CvSVM::LINEAR;
    params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 1000, 1e-6);

    // step 3:  训练
    CvSVM SVM;
    SVM.train(trainingDataMat, labelsMat, cv::Mat(), cv::Mat(), params);

    // step 4:  识别
	ifstream inputFile("train1.txt", ios::in);
	float svmtest1[25];  //保存图片的特征
	for(int i=0; i< 25; i++)
	{
		inputFile>>svmtest1[i];
		char c;
		inputFile>>c;
	}
	inputFile.close();

    cv::Mat sampleMat(25,1,CV_32FC1,svmtest1);
           int response = SVM.predict(sampleMat);  //1_3

            switch(response)
            {
			   case 1:MessageBox(NULL, "左转", "轨迹识别", 0); break;
               case 2:MessageBox(NULL, "直行", "轨迹识别", 0); break;
               case 3:MessageBox(NULL, "右转", "轨迹识别", 0); break;
            }
}

/**
  * 图片颜色反转
  * @param item是要处理的图片
  *
  * @return
  */
void reverseImgColor(IplImage* item)
{
	    
	CvScalar cs;             //声明像素变量  
    for(int i = 0; i < item->height; i++) 
    {
        for (int j = 0; j < item->width; j++)  
          {  
               cs = cvGet2D(item, i, j);   //获取像素  
               cs.val[0] = 255-cs.val[0];             //对指定像素的RGB值进行重新设定  
               cs.val[1] = 255-cs.val[1]; 
               cs.val[2] = 255-cs.val[2];  
               cvSet2D(item, i, j, cs);    //将改变的像素保存到图片中 
           }
    }
}