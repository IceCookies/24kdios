#pragma once


class OpenFileDialog
{
public:
	OpenFileDialog(void);
	~OpenFileDialog(void);


    char* DefaultExtension;
    char* FileName;
    char* Filter;
    int   FilterIndex;
    int   Flags;
    char* InitialDir;
    HWND  Owner;
    char* Title;

    bool ShowDialog();
};

