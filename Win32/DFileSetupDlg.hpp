#ifndef _DFILESETUPDLG_HPP_
#define _DFILESETUPDLG_HPP_

#include <windows.h>

#include "XMLUtils.hpp"
#include "../Source/DParser.hpp"

typedef struct CDPaperSize
{
    wchar_t wsPaperSizeName[64];
    double dPaperWidth;
    double dPaperHeight;
} *PDPaperSize;

typedef struct CDFileUnit
{
    wchar_t wsName[32];
    wchar_t wsAbbrev[8];
    double dBaseToUnit;
    wchar_t wsAbbrev2[8];
} *PDFileUnit;

typedef struct CDFileSetupRec
{
    CDPaperSize cPaperSize;
    bool bPortrait;
    CDFileUnit cLenUnit; // physical lengths
    CDFileUnit cAngUnit; // angles
    CDFileUnit cPaperUnit; // lengths expressed at paper scale
    CDFileUnit cGraphUnit; // graphic elements size - line width, text size, arrow dims
    double dScaleNomin;
    double dScaleDenom;
    double dAngGrid; // in cAngUnit
    double dXGrid; // in cPaperUnit
    double dYGrid; // in cPaperUnit
    double dDefLineWidth; // in cGraphUnit
    int iArrowType;
    double dArrowLen; // in cGraphUnit
    double dArrowWidth; // in cGraphUnit
    wchar_t wsLengthMask[64];
    wchar_t wsAngleMask[64];
    char bFontAttrs;
    double dFontSize; // in cGraphUnit
    double dBaseLine; // in cGraphUnit
    wchar_t wsFontFace[64];
} *PDFileSetupRec;

typedef class CDFileSetupDlg
{
private:
    HINSTANCE m_hInstance;
    PDFileSetupRec m_pFSR;
    CDFileSetupRec m_cFSR;
    PDPaperSize m_pPaperSizes;
    int m_iPaperSizes;
    int m_iX;
    int m_iY;

    PDUnitList m_pUnits;

    bool m_bSettingUp;
    int m_iCurAngUnit;
    int m_iCurPaperUnit;
    int m_iCurGraphUnit;

    HFONT m_hFnt;

    INT_PTR OKBtnClick(HWND hWnd);
    void BuildPaperSizeList(LPCWSTR wsAppPath);
    void BuildLenUnitsList(LPCWSTR wsAppPath);
    INT_PTR AngUnitCBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl);
    INT_PTR PaperUnitCBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl);
    INT_PTR GraphUnitCBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl);
    INT_PTR GridEditChange(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    INT_PTR FaceCBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl);
    INT_PTR FontAttrChBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl, int iMask);
    void SetupFontSample(HWND hWnd);
    int SetupUnitCB(HWND hCB, int iType, PDFileUnit pFileUnit);
public:
    CDFileSetupDlg(HINSTANCE hInstance, LPCWSTR wsAppPath);
    ~CDFileSetupDlg();
    bool ShowDialog(HWND hWndParent, PDFileSetupRec pFSR);
    void SaveSettings(CXMLWritter* pWrit);
    void RestoreSettings(CXMLReader* pRdr);

    INT_PTR WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam);
    INT_PTR WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    INT_PTR WMMove(HWND hWnd, short int xPos, short int yPos);
    PDUnitList GetUnitList();
    PDPaperSize FindPaper(double dWidth, double dHeight);
} *PDFileSetupDlg;

double RoundFloat(double dx);
void FormatFloatStr(double dx, LPWSTR wbuf);
int CALLBACK GetFontNames(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme,
    ULONG FontType, LPARAM lParam);

#endif
