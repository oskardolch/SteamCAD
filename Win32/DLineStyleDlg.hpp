#ifndef _DLINESTYLEDLG_HPP_
#define _DLINESTYLEDLG_HPP_

#include <windows.h>

#include "XMLUtils.hpp"
#include "../Source/DDataTypes.hpp"
#include "DFileSetupDlg.hpp"

typedef struct CDLineStyleRec
{
    CDLineStyle cLineStyle;
    CDFileUnit cUnit;
    bool bWidthSet;
    bool bExcSet;
    bool bPatSet;
    bool bWidthChanged;
    bool bExcChanged;
    bool bPatChanged;
} *PDLineStyleRec;

typedef class CDLineStyleDlg
{
private:
    HINSTANCE m_hInstance;
    int m_iX;
    int m_iY;
    bool m_bSettingUp;
    PDLineStyleRec m_pLSR;
    bool m_bWidthChanged;
    bool m_bExcChanged;
    bool m_bPatChanged;

    INT_PTR OKBtnClick(HWND hWnd);
    INT_PTR LineWidthChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl);
    INT_PTR LineExcChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl);
    INT_PTR LinePatChange(HWND hWnd, WORD wNotifyCode, int iSeg, HWND hwndCtl);
public:
    CDLineStyleDlg(HINSTANCE hInstance);
    ~CDLineStyleDlg();
    bool ShowDialog(HWND hWndParent, PDLineStyleRec pLSR);
    void SaveSettings(CXMLWritter* pWrit);
    void RestoreSettings(CXMLReader* pRdr);

    INT_PTR WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam);
    INT_PTR WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    INT_PTR WMMove(HWND hWnd, short int xPos, short int yPos);
} *PDLineStyleDlg;

#endif
