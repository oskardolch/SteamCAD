#ifndef _DSNAPDLG_HPP_
#define _DSNAPDLG_HPP_

#include <windows.h>

#include "XMLUtils.hpp"

typedef class CDSnapDlg
{
private:
    HINSTANCE m_hInstance;
    int m_iX;
    int m_iY;
    bool *m_pbEnableSnap;

    INT_PTR OKBtnClick(HWND hWnd);
public:
    CDSnapDlg(HINSTANCE hInstance);
    ~CDSnapDlg();
    bool ShowDialog(HWND hWndParent, bool *pbEnableSnap);
    void SaveSettings(CXMLWritter* pWrit);
    void RestoreSettings(CXMLReader* pRdr);

    INT_PTR WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam);
    INT_PTR WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    INT_PTR WMMove(HWND hWnd, short int xPos, short int yPos);
} *PDSnapDlg;

#endif
