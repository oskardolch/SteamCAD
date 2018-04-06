#ifndef _DMAINWND_HPP_
#define _DMAINWND_HPP_

#include <windows.h>
//#include "DToolBar.hpp"
#include "../Source/DDrawTypes.hpp"
#include "DFileSetupDlg.hpp"
#include "DLineStyleDlg.hpp"
#include "DDimEditDlg.hpp"
#include "DScaleDlg.hpp"
#include "DStatDlg.hpp"
#include "DSnapDlg.hpp"

// Snap Type constants
#define stNone 0x00
#define stElement 0x01
#define stEndPoint 0x02
#define stMidPoint 0x04
#define stIntersect 0x08

class CMainWnd
{
private:
    HINSTANCE m_hInstance;
    LPWSTR m_sAppPath;
    HWND m_hWnd;
    //HWND m_hToolBar;
    HWND m_hStatus;
    //HWND m_hProg;
    HWND m_hEdt1, m_hEdt2;
    HWND m_hLab1;

    //CDToolbar *m_pToolBar;
    int m_iToolBarHeight;
    int m_iStatusHeight;

    //int m_iSnapType;
    int m_iDrawMode;
    int m_iButton; // 0 nothing, 1 LButon, 2 MButton, 3 RButton
    int m_iToolMode;

    POINT m_cViewOrigin;
    POINT m_cZoomOrig;
    POINT m_cLastMovePt;
    POINT m_cLastSnapPt;
    POINT m_cLastDownPt;
    CDPoint m_cLastDrawPt;

    HPEN m_hRedPen;
    HPEN m_hSelPen;

    long m_lSelColor;
    long m_lHighColor;
    long m_lActiveColor;

    PDataList m_pDrawObjects;
    PDataList m_pUndoObjects;
    int m_iRedoCount;

    PDObject m_pActiveObject;
    PDObject m_pHighObject;
    int m_iHighDimen;
    int m_iLastExportType;

    wchar_t m_wsStatus2Base[64];
    wchar_t m_wsStatus2Msg[128];
    double m_dRestrictValue;
    int m_iRestrictSet;
    wchar_t m_wsFileName[MAX_PATH];

    CDFileSetupRec m_cFSR;
    PDFileSetupDlg m_pFileSetupDlg;
    double m_dwPage;
    double m_dhPage;
    double m_dUnitScale;
    double m_dDrawScale;
    double m_dDeviceToUnitScale;

    int m_iSelectTolerance;
    int m_iSnapTolerance;

    bool m_bPaperUnits;

    PDLineStyleDlg m_pLineStyleDlg;
    PDDimEditDlg m_pDimEditDlg;
    PDScaleDlg m_pScaleDlg;
    PDStatDlg m_pStatDlg;
    PDSnapDlg m_pSnapDlg;

    CDLine cMeasPoint1, cMeasPoint2;
    bool m_bHasChanged;
    double m_dSavedAngle;
    CDLine m_cLastDynPt;

    void GetAppPath();
    void LoadSettings(HWND hwnd);
    void SaveSettings(HWND hwnd);
    LRESULT FileNewCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT FileOpenCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT FileSaveCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT FileSaveAsCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT FileSaveSelCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT FileIncludeCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT FileExportCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT FilePropsCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    //LRESULT FilePrintSetCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    //LRESULT FilePrintCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT FileExitCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT ModeCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl, int iMode);
    LRESULT EditCopyCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditCutCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditPasteCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditDeleteCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditDelLastPtCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditCopyParCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditMoveCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditRotateCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditMirrorCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditLineStyleCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditToggleSnapCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditPaperUnitsCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditUndoCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditRedoCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT EditConfirmCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT ViewFitCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT ViewActSizeCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    //LRESULT SnapCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl, int iSnap);
    LRESULT ToolsCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl, int iTool);
    LRESULT ToolsScaleCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT ToolsStatCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT HelpContentCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT Edit1Cmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);
    LRESULT Edit2Cmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl);

    //void UpdateSnapMenu(HMENU hMenu);
    void GetPageDims();
    void DrawDimArrow(HDC hdc, PDPrimitive pPrim);
    void DrawPrimitive(HDC hdc, PDPrimitive pPrim);
    void DrawObject(HWND hWnd, HDC hdc, PDObject pObj, int iMode, int iDimen);
    void StartNewObject(HWND hWnd);
    void GetDeviceToUnitScale(HWND hWnd);
    bool PromptForSave(HWND hWnd);
    bool SaveFile(HWND hWnd, LPWSTR wsFile, bool bSelectOnly);
    bool LoadFile(HWND hWnd, LPWSTR wsFile, bool bClear);
    int GetDynMode();
    void FilePropsToData(PDFileAttrs pFAttrs);
    void DataToFileProps();
    void DrawDimText(HWND hWnd, HDC hdc, PDPrimitive pPrim, PDObject pObj, DWORD dwColor,
        double dLineWidth);
    void DrawCross(HWND hWnd);
    void SetTitle(HWND hWnd, bool bForce);
    HRGN GetUpdateRegion(PDPtrList pPolygons);
public:
    CMainWnd(HINSTANCE hInstance);
    ~CMainWnd();

    HWND DisplayWindow();

    LRESULT WMCreate(HWND hwnd, LPCREATESTRUCT lpcs);
    LRESULT WMCommand(HWND hwnd, WORD wNotifyCode, WORD wID, HWND hwndCtl);
    LRESULT WMClose(HWND hwnd);
    LRESULT WMPaint(HWND hwnd, HDC hdc);
    LRESULT WMSize(HWND hwnd, WPARAM fwSizeType, WORD nWidth, WORD nHeight);
    LRESULT WMDestroy(HWND hwnd);
    LRESULT WMMButtonDown(HWND hwnd, WPARAM fwKeys, int xPos, int yPos);
    LRESULT WMMButtonUp(HWND hwnd, WPARAM fwKeys, int xPos, int yPos);
    LRESULT WMLButtonDown(HWND hwnd, WPARAM fwKeys, int xPos, int yPos);
    LRESULT WMLButtonUp(HWND hwnd, WPARAM fwKeys, int xPos, int yPos);
    LRESULT WMRButtonDown(HWND hwnd, WPARAM fwKeys, int xPos, int yPos);
    LRESULT WMRButtonUp(HWND hwnd, WPARAM fwKeys, int xPos, int yPos);
    LRESULT WMMouseMove(HWND hwnd, WPARAM fwKeys, int xPos, int yPos);
    LRESULT WMMouseWheel(HWND hwnd, WORD fwKeys, int zDelta, int xPos, int yPos);
    LRESULT WMLButtonDblClk(HWND hwnd, WPARAM fwKeys, int xPos, int yPos);
    //LRESULT WMChar(HWND hwnd, wchar_t chCharCode, LPARAM lKeyData);
    //LRESULT WMKeyDown(HWND hwnd, int nVirtKey, LPARAM lKeyData);
};

#endif
