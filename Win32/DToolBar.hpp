#ifndef _DTOOLBAR_HPP_
#define _DTOOLBAR_HPP_

#include <windows.h>

class CDToolbar
{
private:
    HWND m_hWnd;
    HWND m_hSepBtn;
    int m_iSepPos;
    HBITMAP m_phMnuBmps[32];
    int m_iMnuBmps;
public:
    CDToolbar(HWND hWnd, HINSTANCE hInstance, HMENU hMenu);
    ~CDToolbar();
    void Resize(HWND hWnd, RECT *pRect, int iRefBtnOffs);
};

#endif
