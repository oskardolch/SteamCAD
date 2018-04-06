#ifndef _DSTATDLG_HPP_
#define _DSTATDLG_HPP_

#include <windows.h>

#include "XMLUtils.hpp"

typedef struct CDStatRec
{
    int iLines;
    int iCircles;
    int iEllips;
    int iArcEllips;
    int iHypers;
    int iParabs;
    int iSplines;
    int iEvolvs;
    int iDimens;
} *PDStatRec;

typedef class CDStatDlg
{
private:
    HINSTANCE m_hInstance;
    int m_iX;
    int m_iY;
    PDStatRec m_pSR;
public:
    CDStatDlg(HINSTANCE hInstance);
    ~CDStatDlg();
    bool ShowDialog(HWND hWndParent, PDStatRec pSR);
    void SaveSettings(CXMLWritter* pWrit);
    void RestoreSettings(CXMLReader* pRdr);

    INT_PTR WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam);
    INT_PTR WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    INT_PTR WMMove(HWND hWnd, short int xPos, short int yPos);
} *PDStatDlg;

#endif
