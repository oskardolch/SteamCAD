#ifndef _DDIMEDITDLG_HPP_
#define _DDIMEDITDLG_HPP_

#include <windows.h>

#include "../Source/DDataTypes.hpp"
#include "../Source/DParser.hpp"
#include "DFileSetupDlg.hpp"
#include "XMLUtils.hpp"

typedef class CDDimEditDlg
{
private:
    HINSTANCE m_hInstance;
    HFONT m_hFnt;
    int m_iX;
    int m_iY;
    bool m_bSettingUp;
    double m_dCurFontSize;
    char m_bCurFntAttrs;
    wchar_t m_sCurFntName[64];
    double m_dCurLeftLen;
    double m_dCurLeftWidth;
    double m_dCurRightLen;
    double m_dCurRightWidth;
    PDDimension m_pDimen;
    LPWSTR m_sCurText;
    PDUnitList m_pUnits;
    PDFileUnit m_pUnit;

    INT_PTR OKBtnClick(HWND hWnd);
    INT_PTR GridEditChange(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    INT_PTR FaceCBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl);
    INT_PTR FontAttrChBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl, int iMask);
    INT_PTR TextEditChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl);
    void SetupFontSample(HWND hWnd);
public:
    CDDimEditDlg(HINSTANCE hInstance);
    ~CDDimEditDlg();
    bool ShowDialog(HWND hWndParent, PDDimension pDimen, PDUnitList pUnits, PDFileUnit pUnit);
    void SaveSettings(CXMLWritter* pWrit);
    void RestoreSettings(CXMLReader* pRdr);

    INT_PTR WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam);
    INT_PTR WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    INT_PTR WMMove(HWND hWnd, short int xPos, short int yPos);
} *PDDimEditDlg;

#endif
