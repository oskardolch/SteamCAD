#ifndef _DSCALEDLG_HPP_
#define _DSCALEDLG_HPP_

#include <windows.h>

#include "XMLUtils.hpp"

typedef struct CDScaleRec
{
    bool bScaleDraw;
    double dScaleNom;
    double dScaleDenom;
    bool bScaleWidth;
    bool bScalePattern;
    bool bScaleArrows;
    bool bScaleLabels;
    bool bChangeDimUnits;
    wchar_t wsLenMask[64];
    wchar_t wsAngMask[64];
} *PDScaleRec;

typedef class CDScaleDlg
{
private:
    HINSTANCE m_hInstance;
    int m_iX;
    int m_iY;
    bool m_bSettingUp;
    PDScaleRec m_pSR;
    bool m_bWidthChanged;
    bool m_bExcChanged;
    bool m_bPatChanged;

    INT_PTR OKBtnClick(HWND hWnd);
    INT_PTR ScaleDrawChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl);
    INT_PTR ChangeUnitsChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl);
public:
    CDScaleDlg(HINSTANCE hInstance);
    ~CDScaleDlg();
    bool ShowDialog(HWND hWndParent, PDScaleRec pSR);
    void SaveSettings(CXMLWritter* pWrit);
    void RestoreSettings(CXMLReader* pRdr);

    INT_PTR WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam);
    INT_PTR WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    INT_PTR WMMove(HWND hWnd, short int xPos, short int yPos);
} *PDScaleDlg;

#endif
