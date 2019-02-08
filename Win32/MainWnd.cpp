#include "MainWnd.hpp"
#include "SteamCAD.rh"
#include <wchar.h>
#include <math.h>
#include <commctrl.h>

#include "XMLUtils.hpp"
#include "../Source/DMath.hpp"
#include "../Source/DExpCairo.hpp"
#include "../Source/DParser.hpp"
#include "../Source/DExpDXF.hpp"

//#define SQR(X) ((X)*(X))

double GetPtDist(LPPOINT pPt, int x, int y)
{
    return fabs(pPt->x - x) + fabs(pPt->y - y);
}


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMainWnd *mw = NULL;
    if(uMsg == WM_CREATE) mw = (CMainWnd*)((LPCREATESTRUCT)lParam)->lpCreateParams;
    else  mw = (CMainWnd*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_CREATE:
        return(mw->WMCreate(hwnd, (LPCREATESTRUCT)lParam));
    case WM_COMMAND:
        return(mw->WMCommand(hwnd, HIWORD(wParam), LOWORD(wParam),
            (HWND)lParam));
    case WM_PAINT:
        return(mw->WMPaint(hwnd, (HDC)wParam));
    case WM_SIZE:
        return(mw->WMSize(hwnd, wParam, LOWORD(lParam), HIWORD(lParam)));
    case WM_LBUTTONDBLCLK:
        return(mw->WMLButtonDblClk(hwnd, wParam, (short int)LOWORD(lParam), (short int)HIWORD(lParam)));
    case WM_MBUTTONDOWN:
        return(mw->WMMButtonDown(hwnd, wParam, (short int)LOWORD(lParam), (short int)HIWORD(lParam)));
    case WM_MBUTTONUP:
        return(mw->WMMButtonUp(hwnd, wParam, (short int)LOWORD(lParam), (short int)HIWORD(lParam)));
    case WM_LBUTTONDOWN:
        return(mw->WMLButtonDown(hwnd, wParam, (short int)LOWORD(lParam), (short int)HIWORD(lParam)));
    case WM_LBUTTONUP:
        return(mw->WMLButtonUp(hwnd, wParam, (short int)LOWORD(lParam), (short int)HIWORD(lParam)));
    case WM_RBUTTONDOWN:
        return(mw->WMRButtonDown(hwnd, wParam, (short int)LOWORD(lParam), (short int)HIWORD(lParam)));
    case WM_RBUTTONUP:
        return(mw->WMRButtonUp(hwnd, wParam, (short int)LOWORD(lParam), (short int)HIWORD(lParam)));
    case WM_MOUSEMOVE:
        return(mw->WMMouseMove(hwnd, wParam, (short int)LOWORD(lParam), (short int)HIWORD(lParam)));
    case WM_MOUSEWHEEL:
        return(mw->WMMouseWheel(hwnd, LOWORD(wParam), (short int)HIWORD(wParam),
            (short int)LOWORD(lParam), (short int)HIWORD(lParam)));
    case WM_CLOSE:
        return(mw->WMClose(hwnd));
    case WM_DESTROY:
        return(mw->WMDestroy(hwnd));
    /*case WM_CHAR:
        return(mw->WMChar(hwnd, (wchar_t)wParam, lParam));
    case WM_KEYDOWN:
        return(mw->WMKeyDown(hwnd, (int)wParam, lParam));*/
    default:
        return(CallWindowProc(DefWindowProc, hwnd, uMsg, wParam, lParam));
    }
}

LRESULT CALLBACK StatusWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC pwPrevProc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_COMMAND:
        return SendMessage(GetParent(hwnd), uMsg, wParam, lParam);
    default:
        return CallWindowProc(pwPrevProc, hwnd, uMsg, wParam, lParam);
    }
}

CMainWnd::CMainWnd(HINSTANCE hInstance)
{
    m_hInstance = hInstance;
    m_sAppPath = NULL;
    GetAppPath();

    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_DBLCLKS | CS_OWNDC;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, L"ICO_1_MAIN");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = L"MAINMENU";
    wc.lpszClassName = L"DRAWMAINFORM";
    wc.hIconSm = NULL;

    RegisterClassEx(&wc);

    m_hWnd = 0;
    //m_hToolBar = 0;
    m_hStatus = 0;

    m_iDrawMode = modSelect;
    m_iButton = 0;
    m_iToolMode = tolNone;

    m_cViewOrigin.x = 0;
    m_cViewOrigin.y = 0;
    m_cLastSnapPt.x = -100;
    m_cLastSnapPt.y = -100;

    m_hRedPen = CreatePen(PS_SOLID, 0, 0x000000FF);

    m_lSelColor = 0x0024E907; //0x00008888;
    m_lHighColor = 0x00EDD52C; //0x00888800;
    m_lActiveColor = 0x00FF0000;

    m_hSelPen = CreatePen(PS_SOLID, 0, m_lSelColor);
    m_hBrownPen = CreatePen(PS_SOLID, 0, 0x00004C80);

    m_pDrawObjects = new CDataList();
    m_pUndoObjects = new CDataList();
    m_iRedoCount = 0;

    m_pActiveObject = NULL;
    m_pHighObject = NULL;
    m_pSelForDimen = NULL;
    m_bPaperUnits = false;
    m_iDrawGridMode = 0;
    m_iHighDimen = -2;

    wcscpy(m_cFSR.cPaperSize.wsPaperSizeName, L"A4");
    m_cFSR.cPaperSize.dPaperWidth = 210.0;
    m_cFSR.cPaperSize.dPaperHeight = 297.0;
    m_cFSR.bPortrait = false;
    wcscpy(m_cFSR.cLenUnit.wsName, L"millimeter");
    wcscpy(m_cFSR.cLenUnit.wsAbbrev, L"mm");
    m_cFSR.cLenUnit.dBaseToUnit = 1.0;
    wcscpy(m_cFSR.cLenUnit.wsAbbrev2, L"");
    m_cFSR.dScaleNomin = 1.0;
    m_cFSR.dScaleDenom = 1.0;
    wcscpy(m_cFSR.cAngUnit.wsName, L"degree");
    wcscpy(m_cFSR.cAngUnit.wsAbbrev, L"deg");
    m_cFSR.cAngUnit.dBaseToUnit = 1.0;
    wcscpy(m_cFSR.cAngUnit.wsAbbrev2, L"\xB0");
    wcscpy(m_cFSR.cPaperUnit.wsName, L"millimeter");
    wcscpy(m_cFSR.cPaperUnit.wsAbbrev, L"mm");
    m_cFSR.cPaperUnit.dBaseToUnit = 1.0;
    wcscpy(m_cFSR.cPaperUnit.wsAbbrev2, L"");
    wcscpy(m_cFSR.cGraphUnit.wsName, L"millimeter");
    wcscpy(m_cFSR.cGraphUnit.wsAbbrev, L"mm");
    m_cFSR.cGraphUnit.dBaseToUnit = 1.0;
    wcscpy(m_cFSR.cGraphUnit.wsAbbrev2, L"");
    //m_cFSR.iAngUnit = 0;
    m_cFSR.dAngGrid = 15.0;
    m_cFSR.dXGrid = 10.0;
    m_cFSR.dYGrid = 10.0;
    m_cFSR.dDefLineWidth = 0.25;
    m_cFSR.iArrowType = 1;
    m_cFSR.dArrowLen = 4.0;
    m_cFSR.dArrowWidth = 2.0;
    m_cFSR.bFontAttrs = 0;
    m_cFSR.dFontSize = 5.0;
    m_cFSR.dBaseLine = 1.0;
    wcscpy(m_cFSR.wsFontFace, L"Arial");
    wcscpy(m_cFSR.wsLengthMask, L"[D:2]");
    wcscpy(m_cFSR.wsAngleMask, L"[r:2]\xB0");

    m_wsStatus2Msg[0] = 0;
    m_iRestrictSet = -1;
    m_wsFileName[0] = 0;
    m_iLastExportType = 0;

    m_pFileSetupDlg = new CDFileSetupDlg(m_hInstance, m_sAppPath);
    m_pLineStyleDlg = new CDLineStyleDlg(m_hInstance);
    m_pDimEditDlg = new CDDimEditDlg(m_hInstance);
    m_pScaleDlg = new CDScaleDlg(m_hInstance);
    m_pStatDlg = new CDStatDlg(m_hInstance);
    m_pSnapDlg = new CDSnapDlg(m_hInstance);

    m_cMeasPoint1.bIsSet = false;
    m_cMeasPoint2.bIsSet = false;
    m_cLastDynPt.bIsSet = false;
    m_bHasChanged = true;
}

CMainWnd::~CMainWnd()
{
    if(m_pActiveObject) delete m_pActiveObject;

    delete m_pSnapDlg;
    delete m_pStatDlg;
    delete m_pScaleDlg;
    delete m_pDimEditDlg;
    delete m_pLineStyleDlg;
    delete m_pFileSetupDlg;

    delete m_pUndoObjects;
    delete m_pDrawObjects;
    //delete m_pToolBar;

    DeleteObject(m_hBrownPen);
    DeleteObject(m_hSelPen);
    DeleteObject(m_hRedPen);

    free(m_sAppPath);
}

HWND CMainWnd::DisplayWindow()
{
    m_hWnd = CreateWindowEx(WS_EX_APPWINDOW, L"DRAWMAINFORM", L"Steam CAD",
        //WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL,
        m_hInstance, (LPVOID)this);

    LoadSettings(m_hWnd);

    if(fabs(m_cFSR.dScaleDenom) > g_dPrec)
        m_dDrawScale = m_cFSR.dScaleNomin/m_cFSR.dScaleDenom;
    else m_dDrawScale = 1.0;

    GetPageDims();
    CDFileAttrs cFAttrs;
    FilePropsToData(&cFAttrs);
    m_pDrawObjects->SetFileAttrs(&cFAttrs, true);
    SetTitle(m_hWnd, true);

    HMENU hMenu = GetMenu(m_hWnd);
    //UpdateSnapMenu(hMenu);

    UINT uCheck = MF_BYCOMMAND | MF_CHECKED;
    CheckMenuItem(hMenu, IDM_MODESELECT, uCheck);

    ViewFitCmd(m_hWnd, 0, 0);

    uCheck = MF_BYCOMMAND;
    if(m_bPaperUnits) uCheck |= MF_CHECKED;
    else uCheck |= MF_UNCHECKED;
    CheckMenuItem(hMenu, IDM_EDITPAPERUNITS, uCheck);

    uCheck = MF_BYCOMMAND;
    if(m_iDrawGridMode & 1) uCheck |= MF_CHECKED;
    else uCheck |= MF_UNCHECKED;
    CheckMenuItem(hMenu, IDM_VIEWGRIDPTS, uCheck);

    uCheck = MF_BYCOMMAND;
    if(m_iDrawGridMode & 2) uCheck |= MF_CHECKED;
    else uCheck |= MF_UNCHECKED;
    CheckMenuItem(hMenu, IDM_VIEWGRIDLNS, uCheck);
    return(m_hWnd);
}

//extern HWND g_hStatus;

LRESULT CMainWnd::WMCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpcs->lpCreateParams);

    /*m_hToolBar = CreateWindowEx(0, TOOLBARCLASSNAME, (LPCTSTR)NULL,
        WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)IDC_STATUS,
        m_hInstance, NULL);

    m_pToolBar = new CDToolbar(m_hToolBar, m_hInstance, GetMenu(hwnd));*/

    m_hStatus = CreateWindowEx(WS_EX_CONTROLPARENT, STATUSCLASSNAME, (LPCTSTR)NULL,
        WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, (HMENU)IDC_STATUS,
        m_hInstance, NULL);
    SendMessage(m_hStatus, SB_SETUNICODEFORMAT, (WPARAM)TRUE, 0);
//g_hStatus = m_hStatus;

    INT iWidth[3] = {150, 480, -1};
    SendMessage(m_hStatus, SB_SETPARTS, 3, (LPARAM)&iWidth);

    /*m_hProg = CreateWindowEx(0, PROGRESS_CLASS, NULL,
        WS_CHILD, 0, 0, 0, 0, m_hStatus,
        (HMENU)0, m_hInstance, NULL);*/

    GetDeviceToUnitScale(hwnd);

    RECT rc;
    /*GetClientRect(m_hToolBar, &rc);
    m_iToolBarHeight = rc.bottom + 2;*/
    m_iToolBarHeight = 0;
    m_cViewOrigin.y = m_iToolBarHeight;

    GetClientRect(m_hStatus, &rc);
    m_iStatusHeight = rc.bottom;

    HFONT hFnt = (HFONT)SendMessage(m_hStatus, WM_GETFONT, 0, 0);

    m_hEdt1 = CreateWindowEx(WS_EX_CONTROLPARENT, L"EDIT", NULL,
        WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP, 280, 2, 50,
        m_iStatusHeight - 3, m_hStatus, (HMENU)IDC_EDT1, m_hInstance, NULL);
    SendMessage(m_hEdt1, WM_SETFONT, (WPARAM)hFnt, 0);
    SendMessage(m_hEdt1, EM_LIMITTEXT, 64, 0);
    m_hEdt2 = CreateWindowEx(WS_EX_CONTROLPARENT, L"EDIT", NULL,
        WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP, 425, 2, 50, m_iStatusHeight - 3,
        m_hStatus, (HMENU)IDC_EDT2, m_hInstance, NULL);
    SendMessage(m_hEdt2, WM_SETFONT, (WPARAM)hFnt, 0);
    SendMessage(m_hEdt2, EM_LIMITTEXT, 64, 0);

    m_hLab1 = CreateWindowEx(0, L"STATIC", NULL,
        WS_CHILD | SS_LEFT, 340, 4, 80, m_iStatusHeight - 5,
        m_hStatus, (HMENU)IDC_LAB1, m_hInstance, NULL);
    wchar_t buf[64];
    LoadString(m_hInstance, IDS_NUMCOPIES, buf, 64);
    SendMessage(m_hLab1, WM_SETFONT, (WPARAM)hFnt, 0);
    SendMessage(m_hLab1, WM_SETTEXT, 0, (LPARAM)buf);

    WNDPROC wPrevProc = (WNDPROC)SetWindowLongPtr(m_hStatus, GWLP_WNDPROC, (LONG_PTR)StatusWndProc);
    SetWindowLongPtr(m_hStatus, GWLP_USERDATA, (LONG_PTR)wPrevProc);

    return(0);
}

LRESULT CMainWnd::WMCommand(HWND hwnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    switch(wID)
    {
    case IDM_FILENEW:
        return(FileNewCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_FILEOPEN:
        return(FileOpenCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_FILESAVE:
        return(FileSaveCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_FILESAVEAS:
        return(FileSaveAsCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_FILESAVESEL:
        return(FileSaveSelCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_FILEINCLUDE:
        return(FileIncludeCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_FILEEXPORT:
        return(FileExportCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_FILEPROPS:
        return(FilePropsCmd(hwnd, wNotifyCode, hwndCtl));
    //case IDM_FILEPRINTSET:
    //    return(FilePrintSetCmd(hwnd, wNotifyCode, hwndCtl));
    //case IDM_FILEPRINT:
    //    return(FilePrintCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_FILEEXIT:
        return(FileExitCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_ENABLESNAP:
        m_pHighObject->SetSnapTo(true);
        return 0;
    case IDM_DISABLESNAP:
        m_pHighObject->SetSnapTo(false);
        return 0;
    case IDM_MODESELECT:
        return(ModeCmd(hwnd, wNotifyCode, hwndCtl, modSelect));
    case IDM_MODELINE:
        return(ModeCmd(hwnd, wNotifyCode, hwndCtl, modLine));
    case IDM_MODECIRCLE:
        return(ModeCmd(hwnd, wNotifyCode, hwndCtl, modCircle));
    case IDM_MODEELLIPSE:
        return(ModeCmd(hwnd, wNotifyCode, hwndCtl, modEllipse));
    case IDM_MODEARCELLIPSE:
        return(ModeCmd(hwnd, wNotifyCode, hwndCtl, modArcElps));
    case IDM_MODEHYPERBOLA:
        return(ModeCmd(hwnd, wNotifyCode, hwndCtl, modHyperbola));
    case IDM_MODEPARABOLA:
        return(ModeCmd(hwnd, wNotifyCode, hwndCtl, modParabola));
    case IDM_MODESPLINE:
        return(ModeCmd(hwnd, wNotifyCode, hwndCtl, modSpline));
    case IDM_MODEEVEOLVENT:
        return(ModeCmd(hwnd, wNotifyCode, hwndCtl, modEvolvent));
    case IDM_MODEDIMEN:
        return(ToolsCmd(hwnd, wNotifyCode, hwndCtl, tolDimen));
    case IDM_EDITCOPY:
        return(EditCopyCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITCUT:
        return(EditCutCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITPASTE:
        return(EditPasteCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITDELETE:
        return(EditDeleteCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITDELLASTPT:
        return(EditDelLastPtCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITCOPYPAR:
        return(EditCopyParCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITMOVE:
        return(EditMoveCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITROTATE:
        return(EditRotateCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITMIRROR:
        return(EditMirrorCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITLINESTYLE:
        return(EditLineStyleCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITTOGGLESNAP:
        return(EditToggleSnapCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITPAPERUNITS:
        return(EditPaperUnitsCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITUNDO:
        return(EditUndoCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITREDO:
        return(EditRedoCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_EDITCONFIRM:
        return(EditConfirmCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_VIEWFITALL:
        return(ViewFitCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_VIEWACTSIZE:
        return(ViewActSizeCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_VIEWGRIDPTS:
        return(ViewGridCmd(hwnd, wNotifyCode, hwndCtl, 1));
    case IDM_VIEWGRIDLNS:
        return(ViewGridCmd(hwnd, wNotifyCode, hwndCtl, 2));
    /*case IDM_SNAPELEMENT:
    case IDM_SNAPENDPOINT:
    case IDM_SNAPMIDPOINT:
    case IDM_SNAPINTERSECT:
        return(SnapCmd(hwnd, wNotifyCode, hwndCtl, wID - IDM_SNAPELEMENT));*/
    case IDM_TOOLSKNIFE:
        return(ToolsCmd(hwnd, wNotifyCode, hwndCtl, tolKnife));
    case IDM_TOOLSROUND:
        return(ToolsCmd(hwnd, wNotifyCode, hwndCtl, tolRound));
    case IDM_TOOLSEXTEND:
        return(ToolsCmd(hwnd, wNotifyCode, hwndCtl, tolExtend));
    case IDM_TOOLSCONFLICTS:
        return(ToolsCmd(hwnd, wNotifyCode, hwndCtl, tolConflict));
    case IDM_TOOLSMEASURE:
        return(ToolsCmd(hwnd, wNotifyCode, hwndCtl, tolMeas));
    case IDM_TOOLSBREAK:
        return(ToolsBreakCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_TOOLSCALE:
        return(ToolsScaleCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_TOOLSTAT:
        return(ToolsStatCmd(hwnd, wNotifyCode, hwndCtl));
    case IDM_HELPCONTENT:
        return(HelpContentCmd(hwnd, wNotifyCode, hwndCtl));
    case IDC_EDT1:
        return(Edit1Cmd(hwnd, wNotifyCode, hwndCtl));
    case IDC_EDT2:
        return(Edit2Cmd(hwnd, wNotifyCode, hwndCtl));
    default:
        return(0);
    }
}

LRESULT CMainWnd::WMClose(HWND hwnd)
{
    if(!PromptForSave(hwnd)) return 0;

    SaveSettings(hwnd);
    DestroyWindow(hwnd);
    return(0);
}

LRESULT CMainWnd::WMSize(HWND hwnd, WPARAM fwSizeType, WORD nWidth, WORD nHeight)
{
    SendMessage(m_hStatus, WM_SIZE, 0, 0);

    /*RECT R1, rc;
    m_pToolBar->Resize(m_hToolBar, &R1, 0);

    SendMessage(m_hStatus, SB_GETRECT, 0, (LPARAM)&rc);
    SetWindowPos(m_hProg, NULL, rc.left, rc.top, rc.right - rc.left,
        rc.bottom - rc.top, SWP_NOZORDER);

    InvalidateRect(hwnd, NULL, TRUE);*/

    CDRect cdr;
    cdr.cPt1.x = -m_cViewOrigin.x/m_dUnitScale;
    cdr.cPt1.y = (m_iToolBarHeight - m_cViewOrigin.y)/m_dUnitScale;
    cdr.cPt2.x = (nWidth - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (nHeight - m_iStatusHeight - m_cViewOrigin.y)/m_dUnitScale;
    m_pDrawObjects->BuildAllPrimitives(&cdr);

    m_iButton = 0;

    return(0);
}

LRESULT CMainWnd::WMDestroy(HWND hwnd)
{
    PostQuitMessage(0);
    return(0);
}

void CMainWnd::GetAppPath()
{
    wchar_t *buf = GetCommandLine();
    int slen = wcslen(buf);
    wchar_t *newbuf = (wchar_t*)malloc((slen + 1)*sizeof(wchar_t));
    wcscpy(newbuf, buf);
    wchar_t *bufstart = newbuf, *bufend = NULL;
    if(bufstart[0] == '"') bufend = wcschr(++bufstart, '"');
    else bufend = wcschr(bufstart, ' ');

    wchar_t *qbufstart = bufend;
    qbufstart++;

    if(bufend) *bufend = 0;
    bufend = wcsrchr(bufstart, '\\');

    slen = bufend - bufstart + 1;
    m_sAppPath = (wchar_t*)malloc((slen + 1)*sizeof(wchar_t));
    wcsncpy(m_sAppPath, bufstart, slen);
    m_sAppPath[slen] = 0;
}

void CMainWnd::LoadSettings(HWND hwnd)
{
    LPWSTR ininame = (LPWSTR)malloc((wcslen(m_sAppPath) + 32)*sizeof(wchar_t));
    wcscpy(ininame, m_sAppPath);
    wcscat(ininame, L"SteamCAD.xml");

    CXMLReader* pRdr = new CXMLReader(ininame);

    IXMLDOMElement* pElem = pRdr->OpenSection(L"MainForm");
    int i;

    if(pElem)
    {
        WINDOWPLACEMENT wndpl;
        wndpl.length = sizeof(WINDOWPLACEMENT);
        wndpl.flags = 0;
        wndpl.showCmd = SW_SHOWNORMAL;
        wndpl.rcNormalPosition.left = 20;
        wndpl.rcNormalPosition.top = 20;
        wndpl.rcNormalPosition.right = 400;
        wndpl.rcNormalPosition.bottom = 300;
        if(pRdr->GetIntValue(pElem, L"WindowState", &i)) wndpl.showCmd = i;
        if(pRdr->GetIntValue(pElem, L"Left", &i)) wndpl.rcNormalPosition.left = i;
        if(pRdr->GetIntValue(pElem, L"Top", &i)) wndpl.rcNormalPosition.top = i;
        if(pRdr->GetIntValue(pElem, L"Right", &i)) wndpl.rcNormalPosition.right = i;
        if(pRdr->GetIntValue(pElem, L"Bottom", &i)) wndpl.rcNormalPosition.bottom = i;

        pElem->Release();

        SetWindowPlacement(hwnd, &wndpl);
    }

    pElem = pRdr->OpenSection(L"DrawSettings");
    if(pElem)
    {
        if(pRdr->GetIntValue(pElem, L"PaperUnits", &i)) m_bPaperUnits = i;
        if(pRdr->GetIntValue(pElem, L"LastExportType", &i)) m_iLastExportType = i;
        if(pRdr->GetIntValue(pElem, L"DrawGridMode", &i)) m_iDrawGridMode = i;

        pElem->Release();
    }

    double d;
    BYTE b;
    IXMLDOMElement *pE1;

    pElem = pRdr->OpenSection(L"PageSettings");
    if(pElem)
    {
        pE1 = pRdr->OpenSubSection(pElem, L"PaperSize");
        if(pE1)
        {
            pRdr->GetStringValueBuf(pE1, L"PaperName", m_cFSR.cPaperSize.wsPaperSizeName, 64);
            if(pRdr->GetDoubleValue(pE1, L"PaperWidth", &d)) m_cFSR.cPaperSize.dPaperWidth = d;
            if(pRdr->GetDoubleValue(pE1, L"PaperHeight", &d)) m_cFSR.cPaperSize.dPaperHeight = d;
            pE1->Release();
        }
        if(pRdr->GetByteValue(pElem, L"Portrait", &b)) m_cFSR.bPortrait = b;
        pE1 = pRdr->OpenSubSection(pElem, L"LengthUnit");
        if(pE1)
        {
            pRdr->GetStringValueBuf(pE1, L"UnitName", m_cFSR.cLenUnit.wsName, 32);
            pRdr->GetStringValueBuf(pE1, L"UnitAbbrev", m_cFSR.cLenUnit.wsAbbrev, 8);
            if(pRdr->GetDoubleValue(pE1, L"UnitScale", &d)) m_cFSR.cLenUnit.dBaseToUnit = d;
            pRdr->GetStringValueBuf(pE1, L"UnitAbbrev2", m_cFSR.cLenUnit.wsAbbrev2, 8);
            pE1->Release();
        }
        if(pRdr->GetDoubleValue(pElem, L"ScaleNomin", &d)) m_cFSR.dScaleNomin = d;
        if(pRdr->GetDoubleValue(pElem, L"ScaleDenom", &d)) m_cFSR.dScaleDenom = d;
        pE1 = pRdr->OpenSubSection(pElem, L"AngularUnit");
        if(pE1)
        {
            pRdr->GetStringValueBuf(pE1, L"UnitName", m_cFSR.cAngUnit.wsName, 32);
            pRdr->GetStringValueBuf(pE1, L"UnitAbbrev", m_cFSR.cAngUnit.wsAbbrev, 8);
            if(pRdr->GetDoubleValue(pE1, L"UnitScale", &d)) m_cFSR.cAngUnit.dBaseToUnit = d;
            pRdr->GetStringValueBuf(pE1, L"UnitAbbrev2", m_cFSR.cAngUnit.wsAbbrev2, 8);
            pE1->Release();
        }
        pE1 = pRdr->OpenSubSection(pElem, L"PaperUnit");
        if(pE1)
        {
            pRdr->GetStringValueBuf(pE1, L"UnitName", m_cFSR.cPaperUnit.wsName, 32);
            pRdr->GetStringValueBuf(pE1, L"UnitAbbrev", m_cFSR.cPaperUnit.wsAbbrev, 8);
            if(pRdr->GetDoubleValue(pE1, L"UnitScale", &d)) m_cFSR.cPaperUnit.dBaseToUnit = d;
            pRdr->GetStringValueBuf(pE1, L"UnitAbbrev2", m_cFSR.cPaperUnit.wsAbbrev2, 8);
            pE1->Release();
        }
        pE1 = pRdr->OpenSubSection(pElem, L"GraphUnit");
        if(pE1)
        {
            pRdr->GetStringValueBuf(pE1, L"UnitName", m_cFSR.cGraphUnit.wsName, 32);
            pRdr->GetStringValueBuf(pE1, L"UnitAbbrev", m_cFSR.cGraphUnit.wsAbbrev, 8);
            if(pRdr->GetDoubleValue(pE1, L"UnitScale", &d)) m_cFSR.cGraphUnit.dBaseToUnit = d;
            pRdr->GetStringValueBuf(pE1, L"UnitAbbrev2", m_cFSR.cGraphUnit.wsAbbrev2, 8);
            pE1->Release();
        }
        if(pRdr->GetDoubleValue(pElem, L"AngularGrid", &d)) m_cFSR.dAngGrid = d;
        if(pRdr->GetDoubleValue(pElem, L"XGrid", &d)) m_cFSR.dXGrid = d;
        if(pRdr->GetDoubleValue(pElem, L"YGrid", &d)) m_cFSR.dYGrid = d;
        if(pRdr->GetDoubleValue(pElem, L"DefLineWidth", &d)) m_cFSR.dDefLineWidth = d;
        pE1 = pRdr->OpenSubSection(pElem, L"Dimensioning");
        if(pE1)
        {
            if(pRdr->GetIntValue(pE1, L"ArrowType", &i)) m_cFSR.iArrowType = i;
            if(pRdr->GetDoubleValue(pE1, L"ArrowLength", &d)) m_cFSR.dArrowLen = d;
            if(pRdr->GetDoubleValue(pE1, L"ArrowWidth", &d)) m_cFSR.dArrowWidth = d;
            if(pRdr->GetIntValue(pE1, L"FontAttrs", &i)) m_cFSR.bFontAttrs = i;
            if(pRdr->GetDoubleValue(pE1, L"FontSize", &d)) m_cFSR.dFontSize = d;
            if(pRdr->GetDoubleValue(pE1, L"BaseLine", &d)) m_cFSR.dBaseLine = d;
            pRdr->GetStringValueBuf(pE1, L"FontFace", m_cFSR.wsFontFace, 64);
            pRdr->GetStringValueBuf(pE1, L"LengthMask", m_cFSR.wsLengthMask, 64);
            pRdr->GetStringValueBuf(pE1, L"AngleMask", m_cFSR.wsAngleMask, 64);
            pE1->Release();
        }
        pElem->Release();
    }

    m_pFileSetupDlg->RestoreSettings(pRdr);
    m_pLineStyleDlg->RestoreSettings(pRdr);
    m_pDimEditDlg->RestoreSettings(pRdr);
    m_pScaleDlg->RestoreSettings(pRdr);
    m_pStatDlg->RestoreSettings(pRdr);
    m_pSnapDlg->RestoreSettings(pRdr);

    delete pRdr;
    return;
}

void CMainWnd::SaveSettings(HWND hwnd)
{
    //wchar_t wsBuf[64];

    LPWSTR ininame = (LPWSTR)malloc((wcslen(m_sAppPath) + 40)*sizeof(wchar_t));
    wcscpy(ininame, m_sAppPath);
    wcscat(ininame, L"SteamCAD.xml");

    CXMLWritter* pWrit = new CXMLWritter(ininame);
    pWrit->WriteComment(L"SteamCAD Workspace Settings");
    pWrit->CreateRoot(L"Settings");

    IXMLDOMElement* pElem = NULL;
    IXMLDOMElement* pE1 = NULL;

    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    wndpl.flags = 0;
    wndpl.showCmd = 0;
    GetWindowPlacement(hwnd, &wndpl);

    pElem = pWrit->CreateSection(L"MainForm");
    pWrit->AddIntValue(pElem, L"WindowState", wndpl.showCmd);
    pWrit->AddIntValue(pElem, L"Left", wndpl.rcNormalPosition.left);
    pWrit->AddIntValue(pElem, L"Top", wndpl.rcNormalPosition.top);
    pWrit->AddIntValue(pElem, L"Right", wndpl.rcNormalPosition.right);
    pWrit->AddIntValue(pElem, L"Bottom", wndpl.rcNormalPosition.bottom);
    pElem->Release();

    pElem = pWrit->CreateSection(L"DrawSettings");
    pWrit->AddIntValue(pElem, L"PaperUnits", (int)m_bPaperUnits);
    pWrit->AddIntValue(pElem, L"LastExportType", (int)m_iLastExportType);
    pWrit->AddIntValue(pElem, L"DrawGridMode", (int)m_iDrawGridMode);
    pElem->Release();

    pElem = pWrit->CreateSection(L"PageSettings");
    pE1 = pWrit->CreateSubSection(pElem, L"PaperSize");
    pWrit->AddStringValue(pE1, L"PaperName", m_cFSR.cPaperSize.wsPaperSizeName);
    pWrit->AddDoubleValue(pE1, L"PaperWidth", m_cFSR.cPaperSize.dPaperWidth);
    pWrit->AddDoubleValue(pE1, L"PaperHeight", m_cFSR.cPaperSize.dPaperHeight);
    pE1->Release();
    pWrit->AddByteValue(pElem, L"Portrait", m_cFSR.bPortrait);
    pE1 = pWrit->CreateSubSection(pElem, L"LengthUnit");
    pWrit->AddStringValue(pE1, L"UnitName", m_cFSR.cLenUnit.wsName);
    pWrit->AddStringValue(pE1, L"UnitAbbrev", m_cFSR.cLenUnit.wsAbbrev);
    pWrit->AddDoubleValue(pE1, L"UnitScale", m_cFSR.cLenUnit.dBaseToUnit);
    pWrit->AddStringValue(pE1, L"UnitAbbrev2", m_cFSR.cLenUnit.wsAbbrev2);
    pE1->Release();
    pWrit->AddDoubleValue(pElem, L"ScaleNomin", m_cFSR.dScaleNomin);
    pWrit->AddDoubleValue(pElem, L"ScaleDenom", m_cFSR.dScaleDenom);
    pE1 = pWrit->CreateSubSection(pElem, L"AngularUnit");
    pWrit->AddStringValue(pE1, L"UnitName", m_cFSR.cAngUnit.wsName);
    pWrit->AddStringValue(pE1, L"UnitAbbrev", m_cFSR.cAngUnit.wsAbbrev);
    pWrit->AddDoubleValue(pE1, L"UnitScale", m_cFSR.cAngUnit.dBaseToUnit);
    pWrit->AddStringValue(pE1, L"UnitAbbrev2", m_cFSR.cAngUnit.wsAbbrev2);
    pE1->Release();
    pE1 = pWrit->CreateSubSection(pElem, L"PaperUnit");
    pWrit->AddStringValue(pE1, L"UnitName", m_cFSR.cPaperUnit.wsName);
    pWrit->AddStringValue(pE1, L"UnitAbbrev", m_cFSR.cPaperUnit.wsAbbrev);
    pWrit->AddDoubleValue(pE1, L"UnitScale", m_cFSR.cPaperUnit.dBaseToUnit);
    pWrit->AddStringValue(pE1, L"UnitAbbrev2", m_cFSR.cPaperUnit.wsAbbrev2);
    pE1->Release();
    pE1 = pWrit->CreateSubSection(pElem, L"GraphUnit");
    pWrit->AddStringValue(pE1, L"UnitName", m_cFSR.cGraphUnit.wsName);
    pWrit->AddStringValue(pE1, L"UnitAbbrev", m_cFSR.cGraphUnit.wsAbbrev);
    pWrit->AddDoubleValue(pE1, L"UnitScale", m_cFSR.cGraphUnit.dBaseToUnit);
    pWrit->AddStringValue(pE1, L"UnitAbbrev2", m_cFSR.cGraphUnit.wsAbbrev2);
    pE1->Release();
    //pWrit->AddIntValue(pElem, L"AngularUnit", m_cFSR.iAngUnit);
    pWrit->AddDoubleValue(pElem, L"AngularGrid", m_cFSR.dAngGrid);
    pWrit->AddDoubleValue(pElem, L"XGrid", m_cFSR.dXGrid);
    pWrit->AddDoubleValue(pElem, L"YGrid", m_cFSR.dYGrid);
    pWrit->AddDoubleValue(pElem, L"DefLineWidth", m_cFSR.dDefLineWidth);
    pE1 = pWrit->CreateSubSection(pElem, L"Dimensioning");
    pWrit->AddIntValue(pE1, L"ArrowType", m_cFSR.iArrowType);
    pWrit->AddDoubleValue(pE1, L"ArrowLength", m_cFSR.dArrowLen);
    pWrit->AddDoubleValue(pE1, L"ArrowWidth", m_cFSR.dArrowWidth);
    pWrit->AddIntValue(pE1, L"FontAttrs", m_cFSR.bFontAttrs);
    pWrit->AddDoubleValue(pE1, L"FontSize", m_cFSR.dFontSize);
    pWrit->AddDoubleValue(pE1, L"BaseLine", m_cFSR.dBaseLine);
    pWrit->AddStringValue(pE1, L"FontFace", m_cFSR.wsFontFace);
    pWrit->AddStringValue(pE1, L"LengthMask", m_cFSR.wsLengthMask);
    pWrit->AddStringValue(pE1, L"AngleMask", m_cFSR.wsAngleMask);
    pE1->Release();
    pElem->Release();

    m_pFileSetupDlg->SaveSettings(pWrit);
    m_pLineStyleDlg->SaveSettings(pWrit);
    m_pDimEditDlg->SaveSettings(pWrit);
    m_pScaleDlg->SaveSettings(pWrit);
    m_pStatDlg->SaveSettings(pWrit);
    m_pSnapDlg->SaveSettings(pWrit);

    pWrit->Save();
    delete pWrit;

    free(ininame);
}

LRESULT CMainWnd::WMPaint(HWND hwnd, HDC hdc)
{
    RECT rc;
    if(!GetUpdateRect(hwnd, &rc, TRUE)) return(0);

    PAINTSTRUCT ps;
    HDC ldc = BeginPaint(hwnd, &ps);

    HBRUSH hOldBr = (HBRUSH)SelectObject(ldc, GetStockObject(NULL_BRUSH));
    HPEN hOldPen = (HPEN)SelectObject(ldc, m_hBrownPen);

    if(m_iDrawGridMode > 0)
    {
        double dx = m_dUnitScale*m_cFSR.dXGrid;
        double dy = m_dUnitScale*m_cFSR.dYGrid;
        int ix, iy;
        if((dx > 5) && (dy > 5))
        {
            RECT cr;
            GetClientRect(hwnd, &cr);
            HPEN hlPen;

            int iMin = -m_cViewOrigin.x/m_dUnitScale/m_cFSR.dXGrid;
            int iMax = (cr.right - m_cViewOrigin.x)/m_dUnitScale/m_cFSR.dXGrid;
            int jMin = -m_cViewOrigin.y/m_dUnitScale/m_cFSR.dYGrid;
            int jMax = (cr.bottom - m_cViewOrigin.y)/m_dUnitScale/m_cFSR.dYGrid;

            double dGray;
            int iGray;

            if(m_iDrawGridMode & 2)
            {
                if((dx < 200) || (dy < 200))
                {
                    if(dx < dy) dGray = 0.7 + 0.2*(200.0 - dx)/195.0;
                    else dGray = 0.7 + 0.2*(200.0 - dy)/195.0;
                }
                else dGray = 0.7;
                iGray = 255*dGray;

                hlPen = CreatePen(PS_SOLID, 0, RGB(iGray, iGray, iGray));
                SelectObject(ldc, hlPen);
                for(int i = iMin; i <= iMax; i++)
                {
                    dx = m_cViewOrigin.x + (double)i*m_dUnitScale*m_cFSR.dXGrid;
                    ix = Round(dx);
                    MoveToEx(ldc, ix, 0, NULL);
                    LineTo(ldc, ix, cr.bottom);
                }
                for(int j = jMin; j <= jMax; j++)
                {
                    dy = m_cViewOrigin.y + (double)j*m_dUnitScale*m_cFSR.dYGrid;
                    iy = Round(dy);
                    MoveToEx(ldc, 0, iy, NULL);
                    LineTo(ldc, cr.right, iy);
                }
                SelectObject(ldc, hOldPen);
                DeleteObject(hlPen);
            }

            if(m_iDrawGridMode & 1)
            {
                if(m_iDrawGridMode & 2)
                {
                    if((dx < 200) || (dy < 200))
                    {
                        if(dx < dy) dGray = 0.5 + 0.5*(200.0 - dx)/195.0;
                        else dGray = 0.5 + 0.5*(200.0 - dy)/195.0;
                    }
                    else dGray = 0.5;
                }
                else
                {
                    if((dx < 200) || (dy < 200))
                    {
                        if(dx < dy) dGray = 0.3 + 0.3*(200.0 - dx)/195.0;
                        else dGray = 0.3 + 0.3*(200.0 - dy)/195.0;
                    }
                    else dGray = 0.3;
                }
                iGray = 255*dGray;

                hlPen = CreatePen(PS_SOLID, 0, RGB(iGray, iGray, iGray));
                SelectObject(ldc, hlPen);
                for(int i = iMin; i <= iMax; i++)
                {
                    dx = m_cViewOrigin.x + (double)i*m_dUnitScale*m_cFSR.dXGrid;
                    ix = Round(dx);
                    for(int j = jMin; j <= jMax; j++)
                    {
                        dy = m_cViewOrigin.y + (double)j*m_dUnitScale*m_cFSR.dYGrid;
                        iy = Round(dy);
                        Ellipse(ldc, ix - 1, iy - 1, ix + 2, iy + 2);
                    }
                }
                SelectObject(ldc, hOldPen);
                DeleteObject(hlPen);
            }
        }
    }

    SelectObject(ldc, m_hBrownPen);
    Rectangle(ldc, m_cViewOrigin.x, m_cViewOrigin.y,
        m_cViewOrigin.x + m_dUnitScale*m_dwPage, m_cViewOrigin.y + m_dUnitScale*m_dhPage);

    CDRect cdr;
    cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
    cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

    m_pDrawObjects->BuildAllPrimitives(&cdr);

    PDObject pObj;
    int iObjs = m_pDrawObjects->GetCount();
    for(int i = 0; i < iObjs; i++)
    {
        pObj = m_pDrawObjects->GetItem(i);
        DrawObject(hwnd, ldc, pObj, 0, -2);
    }

    if(m_pHighObject) DrawObject(hwnd, ldc, m_pHighObject, 2, m_iHighDimen);

    int iDynMode = GetDynMode();
    CDLine cPtX;
    cPtX.cOrigin = m_cLastDrawPt;
    if(iDynMode == 1)
    {
        cPtX.bIsSet = m_cLastDynPt.bIsSet;
        cPtX.cDirection = m_cLastDynPt.cOrigin;
    }
    else if(iDynMode == 2)
    {
        cPtX.cDirection.x = 0.0;
        if(IS_LENGTH_VAL(m_iRestrictSet))
        {
            cPtX.cDirection.x = 1.0;
            cPtX.cDirection.y = m_dSavedDist;
        }
    }

    if((m_iDrawMode > modSelect) || (m_iToolMode > tolNone))
    {
        int iPrevROP = SetROP2(ldc, R2_NOTXORPEN);
        SelectObject(ldc, m_hRedPen);
        MoveToEx(ldc, m_cLastSnapPt.x - 10, m_cLastSnapPt.y, NULL);
        LineTo(ldc, m_cLastSnapPt.x + 10, m_cLastSnapPt.y);
        MoveToEx(ldc, m_cLastSnapPt.x, m_cLastSnapPt.y - 10, NULL);
        LineTo(ldc, m_cLastSnapPt.x, m_cLastSnapPt.y + 10);
        if(m_pActiveObject)
        {
            m_pActiveObject->BuildPrimitives(cPtX, iDynMode, &cdr, false, NULL);
            DrawObject(hwnd, ldc, m_pActiveObject, 1, -2);
        }
        SetROP2(ldc, iPrevROP);
    }

    SelectObject(ldc, hOldPen);
    SelectObject(ldc, hOldBr);
    EndPaint(hwnd, &ps);

    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;

    cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
    cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

    m_pDrawObjects->BuildAllPrimitives(&cdr);
    if(m_pActiveObject)
    {
        m_pActiveObject->BuildPrimitives(cPtX, iDynMode, &cdr, false, NULL);
    }

    //SendMessage(m_hStatus, WM_PAINT, 0, 0);
    return(0);
}

bool CMainWnd::PromptForSave(HWND hWnd)
{
    if(!m_pDrawObjects->GetChanged()) return true;

    wchar_t wsCaption[64];
    wchar_t wsPrompt[256];
    LoadString(m_hInstance, IDS_WARNING, wsCaption, 64);
    LoadString(m_hInstance, IDS_FILECHANGED, wsPrompt, 256);

    int iRes = MessageBox(hWnd, wsPrompt, wsCaption, MB_YESNOCANCEL | MB_ICONWARNING);
    if(iRes == IDCANCEL) return false;
    if(iRes == IDNO) return true;

    return SaveFile(hWnd, m_wsFileName, false);
}

bool CMainWnd::SaveFile(HWND hWnd, LPWSTR wsFile, bool bSelectOnly)
{
    bool bSave = true;
    bool bNewFile = false;
    if(!wsFile[0])
    {
        bNewFile = true;

        wchar_t wsFilter[128], wsCurDir[1];
        wsCurDir[0] = 0;
        LoadString(m_hInstance, IDS_STEAMDRAWFILTER, wsFilter, 128);

        int n = wcslen(wsFilter);
        for(int i = 0; i < n; i++)
        {
            if(wsFilter[i] == 1) wsFilter[i] = 0;
        }

        OPENFILENAME ofn = {sizeof(OPENFILENAME), hWnd, m_hInstance, wsFilter,
            NULL, 0, 0, wsFile, MAX_PATH, NULL, 0, wsCurDir, NULL,
            OFN_ENABLESIZING | OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
            0, 0, L"sdr", 0, NULL, NULL};

        bSave = GetSaveFileName(&ofn);
    }
    if(!bSave) return false;

    // save the file
    FILE *pf = _wfopen(wsFile, L"wb");
    m_pDrawObjects->SaveToFile(pf, true, bSelectOnly);
    fclose(pf);

    if(!bNewFile) SetTitle(hWnd, true);
    return true;
}

bool CMainWnd::LoadFile(HWND hWnd, LPWSTR wsFile, bool bClear)
{
    wchar_t wsFilter[128], wsCurDir[1];
    wsCurDir[0] = 0;
    LoadString(m_hInstance, IDS_STEAMDRAWFILTER, wsFilter, 128);

    int n = wcslen(wsFilter);
    for(int i = 0; i < n; i++)
    {
        if(wsFilter[i] == 1) wsFilter[i] = 0;
    }

    OPENFILENAME ofn = {sizeof(OPENFILENAME), hWnd, m_hInstance, wsFilter,
        NULL, 0, 0, wsFile, MAX_PATH, NULL, 0, wsCurDir, NULL,
        OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST,
        0, 0, L"sdr", 0, NULL, NULL};

    if(GetOpenFileName(&ofn))
    {
        // load the file
        FILE *pf = _wfopen(wsFile, L"rb");
        bool bRead = m_pDrawObjects->ReadFromFile(pf, true, bClear);
        fclose(pf);
        if(bRead)
        {
            if(bClear)
            {
                DataToFileProps();
                GetPageDims();
                m_pUndoObjects->ClearAll();
                m_iRedoCount = 0;
            }

            RECT rc;
            GetClientRect(hWnd, &rc);
            rc.top += m_iToolBarHeight;
            rc.bottom -= m_iStatusHeight;

            InvalidateRect(hWnd, &rc, TRUE);
            SetTitle(hWnd, true);
        }
        return bRead;
    }
    return false;
}

LRESULT CMainWnd::FileNewCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(!PromptForSave(hwnd)) return 0;

    if(m_pActiveObject) delete m_pActiveObject;
    m_pActiveObject = NULL;
    m_pHighObject = NULL;
    m_pDrawObjects->ClearAll();
    m_pUndoObjects->ClearAll();
    m_wsFileName[0] = 0;
    m_iRedoCount = 0;

    CDFileAttrs cFAttrs;
    FilePropsToData(&cFAttrs);
    m_pDrawObjects->SetFileAttrs(&cFAttrs, true);

    InvalidateRect(hwnd, NULL, true);
    SetTitle(hwnd, true);
    return 0;
}

LRESULT CMainWnd::FileOpenCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(!PromptForSave(hwnd)) return 0;

    LoadFile(hwnd, m_wsFileName, true);
    return 0;
}

LRESULT CMainWnd::FileSaveCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    SaveFile(hwnd, m_wsFileName, false);
    return 0;
}

LRESULT CMainWnd::FileSaveAsCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    wchar_t wsNewName[MAX_PATH];
    wsNewName[0] = 0;
    if(SaveFile(hwnd, wsNewName, false))
    {
        wcscpy(m_wsFileName, wsNewName);
        SetTitle(hwnd, true);
    }
    return 0;
}

LRESULT CMainWnd::FileSaveSelCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    wchar_t wsNewName[MAX_PATH];
    wsNewName[0] = 0;
    SaveFile(hwnd, wsNewName, true);
    return 0;
}

LRESULT CMainWnd::FileIncludeCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    wchar_t wsNewName[MAX_PATH];
    wsNewName[0] = 0;
    LoadFile(hwnd, wsNewName, false);
    return 0;
}

LRESULT CMainWnd::FileExportCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    bool bSave = true;
    wchar_t wsFile[MAX_PATH];

    wsFile[0] = 0;
    wchar_t wsFilter[256], wsCurDir[1];
    wsCurDir[0] = 0;
    LoadString(m_hInstance, IDS_STEAMEXPORTFILTER, wsFilter, 256);

    int n = wcslen(wsFilter);
    for(int i = 0; i < n; i++)
    {
        if(wsFilter[i] == 1) wsFilter[i] = 0;
    }

    if(m_wsFileName[0])
    {
        wchar_t *wsSlash = wcsrchr(m_wsFileName, '\\');
        if(wsSlash) wcscpy(wsFile, &wsSlash[1]);
        else wcscpy(wsFile, m_wsFileName);

        wchar_t *wsDot = wcsrchr(wsFile, '.');
        if(wsDot) *wsDot = 0;
    }

    wchar_t sDefExt[4];
    switch(m_iLastExportType)
    {
    case 0:
        wcscpy(sDefExt, L"pdf");
        break;
    case 1:
        wcscpy(sDefExt, L"ps");
        break;
    case 2:
        wcscpy(sDefExt, L"eps");
        break;
    case 3:
        wcscpy(sDefExt, L"png");
        break;
    case 4:
        wcscpy(sDefExt, L"svg");
        break;
    default:
        sDefExt[0] = 0;
    }

    OPENFILENAME ofn = {sizeof(OPENFILENAME), hwnd, m_hInstance, wsFilter,
        NULL, 0, m_iLastExportType + 1, wsFile, MAX_PATH, NULL, 0, wsCurDir, NULL,
        OFN_ENABLESIZING | OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
        0, 0, sDefExt, 0, NULL, NULL};

    if(!GetSaveFileName(&ofn)) return 0;

    m_iLastExportType = ofn.nFilterIndex - 1;

    // export to the file
    FILE *pf = _wfopen(wsFile, L"wb");
    if(m_iLastExportType < 5)
        ExportCairoFile(m_iLastExportType, pf, m_pDrawObjects, m_pFileSetupDlg->GetUnitList());
    else
        ExportDXFFile(pf, m_pDrawObjects, m_pFileSetupDlg->GetUnitList());
    fclose(pf);

    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;

    CDRect cdr;
    cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
    cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

    m_pDrawObjects->BuildAllPrimitives(&cdr);
    return 0;
}

void CMainWnd::FilePropsToData(PDFileAttrs pFAttrs)
{
    pFAttrs->dWidth = m_cFSR.cPaperSize.dPaperWidth;
    pFAttrs->dHeight = m_cFSR.cPaperSize.dPaperHeight;
    if(m_cFSR.bPortrait)
    {
        if(pFAttrs->dWidth > pFAttrs->dHeight)
        {
            pFAttrs->dWidth = m_cFSR.cPaperSize.dPaperHeight;
            pFAttrs->dHeight = m_cFSR.cPaperSize.dPaperWidth;
        }
    }
    else if(pFAttrs->dWidth < pFAttrs->dHeight)
    {
        pFAttrs->dWidth = m_cFSR.cPaperSize.dPaperHeight;
        pFAttrs->dHeight = m_cFSR.cPaperSize.dPaperWidth;
    }
    pFAttrs->dScaleNom = m_cFSR.dScaleNomin;
    pFAttrs->dScaleDenom = m_cFSR.dScaleDenom;
    pFAttrs->iArrowType = m_cFSR.iArrowType;
    pFAttrs->cArrowDim.x = m_cFSR.dArrowLen;
    pFAttrs->cArrowDim.y = m_cFSR.dArrowWidth;
    pFAttrs->dFontSize = m_cFSR.dFontSize;
    pFAttrs->dBaseLine = m_cFSR.dBaseLine;
    pFAttrs->bFontAttrs = m_cFSR.bFontAttrs;
    WideCharToMultiByte(CP_UTF8, 0, m_cFSR.wsFontFace, -1, pFAttrs->sFontFace, 64, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, m_cFSR.wsLengthMask, -1, pFAttrs->sLengthMask, 64, NULL, NULL);
    WideCharToMultiByte(CP_UTF8, 0, m_cFSR.wsAngleMask, -1, pFAttrs->sAngleMask, 64, NULL, NULL);
}

void CMainWnd::DataToFileProps()
{
    CDFileAttrs cFAttrs;
    m_pDrawObjects->GetFileAttrs(&cFAttrs);

    if(cFAttrs.dHeight > cFAttrs.dWidth)
    {
        m_cFSR.bPortrait = true;
        m_cFSR.cPaperSize.dPaperWidth = cFAttrs.dHeight;
        m_cFSR.cPaperSize.dPaperHeight = cFAttrs.dWidth;
    }
    else
    {
        m_cFSR.bPortrait = false;
        m_cFSR.cPaperSize.dPaperWidth = cFAttrs.dWidth;
        m_cFSR.cPaperSize.dPaperHeight = cFAttrs.dHeight;
    }

    PDPaperSize pSize = m_pFileSetupDlg->FindPaper(cFAttrs.dWidth, cFAttrs.dHeight);
    if(pSize) wcscpy(m_cFSR.cPaperSize.wsPaperSizeName, pSize->wsPaperSizeName);

    m_cFSR.dScaleNomin = cFAttrs.dScaleNom;
    m_cFSR.dScaleDenom = cFAttrs.dScaleDenom;

    if(fabs(m_cFSR.dScaleDenom) > g_dPrec)
        m_dDrawScale = m_cFSR.dScaleNomin/m_cFSR.dScaleDenom;
    else m_dDrawScale = 1.0;
}

LRESULT CMainWnd::FilePropsCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(m_pFileSetupDlg->ShowDialog(hwnd, &m_cFSR))
    {
        if(fabs(m_cFSR.dScaleDenom) > g_dPrec)
            m_dDrawScale = m_cFSR.dScaleNomin/m_cFSR.dScaleDenom;
        else m_dDrawScale = 1.0;

        GetPageDims();

        CDFileAttrs cFAttrs;
        FilePropsToData(&cFAttrs);
        // set new file attributes
        m_pDrawObjects->SetFileAttrs(&cFAttrs, false);

        RECT rc;
        GetClientRect(hwnd, &rc);
        rc.top += m_iToolBarHeight;
        rc.bottom -= m_iStatusHeight;

        CDRect cdr;
        cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
        cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
        cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
        cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

        m_pDrawObjects->BuildAllPrimitives(&cdr);
        InvalidateRect(hwnd, &rc, TRUE);
        SetTitle(hwnd, false);
    }
    return 0;
}

/*LRESULT CMainWnd::FilePrintSetCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    PAGESETUPDLG cps;
    cps.lStructSize = sizeof(PAGESETUPDLG);
    cps.hwndOwner = hwnd;
    cps.hDevMode = NULL;
    cps.hDevNames = NULL;
    cps.Flags = 0;
    cps.hInstance = m_hInstance;
    PageSetupDlg(&cps);
    return 0;
}

LRESULT CMainWnd::FilePrintCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    return 0;
}*/

LRESULT CMainWnd::FileExitCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    SendMessage(hwnd, WM_CLOSE, 0, 0);
    return 0;
}

UINT MapDrawModeToMenu(int iDrawMode)
{
    switch(iDrawMode)
    {
    case modLine:
        return IDM_MODELINE;
    case modCircle:
        return IDM_MODECIRCLE;
    case modEllipse:
        return IDM_MODEELLIPSE;
    case modArcElps:
        return IDM_MODEARCELLIPSE;
    case modHyperbola:
        return IDM_MODEHYPERBOLA;
    case modParabola:
        return IDM_MODEPARABOLA;
    case modSpline:
        return IDM_MODESPLINE;
    case modEvolvent:
        return IDM_MODEEVEOLVENT;
    default:
        return IDM_MODESELECT;
    }
}

LRESULT CMainWnd::ModeCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl, int iMode)
{
    if((wNotifyCode == 1) && (iMode > 0) && (IsWindowVisible(m_hEdt1) || IsWindowVisible(m_hEdt2)))
    {
        HWND hFocus = GetFocus();
        char c = 0;
        switch(iMode)
        {
        case 1:
            c = 'l';
            break;
        case 2:
            c = 'c';
            break;
        case 3:
            c = 'e';
            break;
        case 4:
            c = 'a';
            break;
        case 5:
            c = 'h';
            break;
        case 6:
            c = 'p';
            break;
        case 7:
            c = 's';
            break;
        case 8:
            c = 'v';
            break;
        }
        if(hFocus == m_hEdt1)
        {
            SendMessage(m_hEdt1, WM_CHAR, c, 0);
        }
        else if(hFocus == m_hEdt2)
        {
            SendMessage(m_hEdt2, WM_CHAR, c, 0);
        }
        return 0;
    }

    ShowWindow(m_hEdt1, SW_HIDE);
    ShowWindow(m_hEdt2, SW_HIDE);

    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;
    HDC hdc;

    if(m_pActiveObject)
    {
        hdc = GetDC(hwnd);
        IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);

        HBRUSH hPrevBr = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
        int iPrevROP = SetROP2(hdc, R2_NOTXORPEN);
        DrawObject(hwnd, hdc, m_pActiveObject, 1, -2);
        SetROP2(hdc, iPrevROP);
        SelectObject(hdc, hPrevBr);
        SelectClipRgn(hdc, NULL);
        ReleaseDC(hwnd, NULL);

        delete m_pActiveObject;
        m_pActiveObject = NULL;
    }
    else if(m_pSelForDimen)
    {
        m_pSelForDimen->DiscardDimen();
        m_pSelForDimen = NULL;
    }

    if(((m_iDrawMode + m_iToolMode > 0) && (iMode == 0)) ||
       ((m_iDrawMode + m_iToolMode == 0) && (iMode > 0)))
    {
        DrawCross(hwnd);
    }

    m_iToolMode = tolNone;

    HMENU hMenu = GetMenu(hwnd);

    UINT uCheck = MF_BYCOMMAND | MF_UNCHECKED;
    CheckMenuItem(hMenu, MapDrawModeToMenu(m_iDrawMode), uCheck);

    m_iDrawMode = iMode;

    StartNewObject(hwnd);
    if(!m_pActiveObject && (m_iDrawMode))
    {
        m_iDrawMode = modSelect;
        DrawCross(hwnd);
    }

    uCheck = MF_BYCOMMAND | MF_CHECKED;
    CheckMenuItem(hMenu, MapDrawModeToMenu(m_iDrawMode), uCheck);
    return 0;
}

LRESULT CMainWnd::EditCopyCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    return 0;
}

LRESULT CMainWnd::EditCutCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    return 0;
}

LRESULT CMainWnd::EditPasteCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    return 0;
}

HRGN CMainWnd::GetUpdateRegion(PDPtrList pPolygons)
{
    int iCnt = pPolygons->GetCount();
    int iTot = 0;
    INT *pCnts = (INT*)malloc(iCnt*sizeof(INT));

    PDPolygon pPoly;
    for(int i = 0; i < iCnt; i++)
    {
        pPoly = (PDPolygon)pPolygons->GetItem(i);
        pCnts[i] = pPoly->iPoints;
        iTot += pPoly->iPoints;
    }

    POINT *pPts = (POINT*)malloc(iTot*sizeof(POINT));
    int j = 0;
    for(int i = 0; i < iCnt; i++)
    {
        pPoly = (PDPolygon)pPolygons->GetItem(i);
        for(int k = 0; k < pPoly->iPoints; k++)
        {
            pPts[j].x = Round(m_cViewOrigin.x + m_dUnitScale*pPoly->pPoints[k].x);
            pPts[j++].y = Round(m_cViewOrigin.y + m_dUnitScale*pPoly->pPoints[k].y);
        }
    }

    HRGN hRes = CreatePolyPolygonRgn(pPts, pCnts, iCnt, WINDING);
    free(pPts);
    free(pCnts);
    return hRes;
}

LRESULT CMainWnd::EditDeleteCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
//MessageBox(hwnd, L"Dobry", L"Debug", MB_OK);
    if(m_iDrawMode + m_iToolMode > 0)
    {
        HWND hFocus = GetFocus();
        if(hFocus == m_hEdt1) SendMessage(m_hEdt1, WM_KEYDOWN, VK_DELETE, 0);
        if(hFocus == m_hEdt2) SendMessage(m_hEdt2, WM_KEYDOWN, VK_DELETE, 0);
        return 0;
    }

    m_pActiveObject = NULL;
    m_pHighObject = NULL;

    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;

    CDRect cdr;
    cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
    cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);

    if(m_pDrawObjects->DeleteSelected(m_pUndoObjects, &cdr, pRegions))
    {
        HRGN hRgn = GetUpdateRegion(pRegions);
        //InvalidateRect(hwnd, NULL, true);
        if(hRgn)
        {
            InvalidateRgn(hwnd, hRgn, TRUE);
            DeleteObject(hRgn);
        }
        SetTitle(hwnd, false);
    }

    ClearPolygonList(pRegions);
    delete pRegions;
    return 0;
}

LRESULT CMainWnd::EditDelLastPtCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    return 0;
}

void CMainWnd::DrawCross(HWND hWnd)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;

    HDC hdc = GetDC(hWnd);
    IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);

    HPEN hPrevPen = (HPEN)SelectObject(hdc, m_hRedPen);
    int iPrevROP = SetROP2(hdc, R2_NOTXORPEN);
    MoveToEx(hdc, m_cLastSnapPt.x - 10, m_cLastSnapPt.y, NULL);
    LineTo(hdc, m_cLastSnapPt.x + 10, m_cLastSnapPt.y);
    MoveToEx(hdc, m_cLastSnapPt.x, m_cLastSnapPt.y - 10, NULL);
    LineTo(hdc, m_cLastSnapPt.x, m_cLastSnapPt.y + 10);
    SetROP2(hdc, iPrevROP);
    SelectObject(hdc, hPrevPen);
    SelectClipRgn(hdc, NULL);
    ReleaseDC(hWnd, NULL);
}

LRESULT CMainWnd::EditCopyParCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    int iSel = m_pDrawObjects->GetSelectCount();
    if(iSel != 1) return 0;

    PDObject pObj = m_pDrawObjects->GetSelected(0);
    if(!pObj) return 0;

    PDObject pNewObj = NULL;
    int iType = pObj->GetType();
    pNewObj = pObj->Copy();
    if(!pNewObj) return 0;

    m_pActiveObject = pNewObj;
    m_iToolMode = tolCopyPar;

    DrawCross(hwnd);

    LoadString(m_hInstance, IDS_DISTANCE, m_wsStatus2Base, 64);
    wcscpy(m_wsStatus2Msg, m_wsStatus2Base);
    SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Msg);

    ShowWindow(m_hEdt1, SW_SHOW);
    SetFocus(m_hEdt1);
    return 0;
}

LRESULT CMainWnd::EditMoveCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    DrawCross(hwnd);
    LoadString(m_hInstance, IDS_DISTANCE, m_wsStatus2Base, 64);
    SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Base);
    ShowWindow(m_hEdt1, SW_SHOW);
    ShowWindow(m_hEdt2, SW_SHOW);
    ShowWindow(m_hLab1, SW_SHOW);

    SetFocus(m_hEdt1);

    wchar_t wBuf[64];
    SendMessage(m_hEdt1, WM_GETTEXT, 64, (LPARAM)wBuf);
    char sBuf[64];
    WideCharToMultiByte(CP_UTF8, 0, wBuf, -1, sBuf, 64, NULL, NULL);
    m_iRestrictSet = ParseInputString(sBuf, m_pFileSetupDlg->GetUnitList(), &m_dRestrictValue);
    if(IS_LENGTH_VAL(m_iRestrictSet))
        LoadString(m_hInstance, IDS_SELLINETOMOVE, m_wsStatus2Msg, 128);
    else LoadString(m_hInstance, IDS_SELPOINTFROMMOVE, m_wsStatus2Msg, 128);
    SendMessage(m_hStatus, SB_SETTEXT, 2, (LPARAM)m_wsStatus2Msg);
    m_iToolMode = tolMove;
    return 0;
}

LRESULT CMainWnd::EditRotateCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    DrawCross(hwnd);
    LoadString(m_hInstance, IDS_ANGLE, m_wsStatus2Base, 64);
    SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Base);
    ShowWindow(m_hEdt1, SW_SHOW);
    ShowWindow(m_hEdt2, SW_SHOW);
    ShowWindow(m_hLab1, SW_SHOW);
    SetFocus(m_hEdt1);
    LoadString(m_hInstance, IDS_SELPOINTTOROTATE, m_wsStatus2Msg, 128);
    SendMessage(m_hStatus, SB_SETTEXT, 2, (LPARAM)m_wsStatus2Msg);
    m_iToolMode = tolRotate;
    return 0;
}

LRESULT CMainWnd::EditMirrorCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    DrawCross(hwnd);
    LoadString(m_hInstance, IDS_SELLINETOMIRROR, m_wsStatus2Msg, 128);
    SendMessage(m_hStatus, SB_SETTEXT, 2, (LPARAM)m_wsStatus2Msg);
    m_iToolMode = tolMirror;
    return 0;
}

LRESULT CMainWnd::EditLineStyleCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    CDLineStyleRec cLSRec;
    CDDimension cDimen;
    cDimen.psLab = NULL;
    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);
    int iMask = m_pDrawObjects->GetSelectedLineStyle(&cLSRec.cLineStyle);
    if(iMask > -1)
    {
        memcpy(&cLSRec.cUnit, &m_cFSR.cGraphUnit, sizeof(CDFileUnit));
        cLSRec.bWidthSet = (iMask & 1);
        cLSRec.bExcSet = (iMask & 2);
        cLSRec.bPatSet = (iMask & 4);
        cLSRec.bWidthChanged = false;
        cLSRec.bExcChanged = false;
        cLSRec.bPatChanged = false;
        if(m_pLineStyleDlg->ShowDialog(hwnd, &cLSRec) == IDOK)
        {
            iMask = 0;
            if(cLSRec.bWidthSet && cLSRec.bWidthChanged) iMask |= 1;
            if(cLSRec.bExcSet && cLSRec.bExcChanged) iMask |= 2;
            if(cLSRec.bPatSet && cLSRec.bPatChanged) iMask |= 4;
            if(m_pDrawObjects->SetSelectedLineStyle(iMask, &cLSRec.cLineStyle, pRegions))
            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                rc.top += m_iToolBarHeight;
                rc.bottom -= m_iStatusHeight;

                CDRect cdr;
                cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
                cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
                cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
                cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

                m_pDrawObjects->BuildAllPrimitives(&cdr);
                HRGN hRgn = GetUpdateRegion(pRegions);
                //InvalidateRect(hwnd, NULL, true);
                if(hRgn)
                {
                    InvalidateRgn(hwnd, hRgn, TRUE);
                    DeleteObject(hRgn);
                }
                SetTitle(hwnd, false);
            }
        }
    }
    else if(m_pDrawObjects->GetSelectedDimen(&cDimen))
    {
        if(m_pDimEditDlg->ShowDialog(hwnd, &cDimen, m_pFileSetupDlg->GetUnitList(),
            &m_cFSR.cGraphUnit) == IDOK)
        {
            if(m_pDrawObjects->SetSelectedDimen(&cDimen, pRegions))
            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                rc.top += m_iToolBarHeight;
                rc.bottom -= m_iStatusHeight;

                CDRect cdr;
                cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
                cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
                cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
                cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

                m_pDrawObjects->BuildAllPrimitives(&cdr);
                HRGN hRgn = GetUpdateRegion(pRegions);
                //InvalidateRect(hwnd, NULL, true);
                if(hRgn)
                {
                    InvalidateRgn(hwnd, hRgn, true);
                    DeleteObject(hRgn);
                }
                SetTitle(hwnd, false);
            }
        }
        if(cDimen.psLab) free(cDimen.psLab);
    }
    ClearPolygonList(pRegions);
    delete pRegions;
    return 0;
}

LRESULT CMainWnd::EditToggleSnapCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    bool bSnapEnabled = m_pDrawObjects->GetSelSnapEnabled();
    if(m_pSnapDlg->ShowDialog(hwnd, &bSnapEnabled))
    {
        m_pDrawObjects->SetSelSnapEnabled(bSnapEnabled);
    }
    return 0;
}

LRESULT CMainWnd::EditPaperUnitsCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    HMENU hMnu = GetMenu(hwnd);
    UINT uiCheck = MF_BYCOMMAND;
    if(m_bPaperUnits)
    {
        m_bPaperUnits = false;
        uiCheck |= MF_UNCHECKED;
    }
    else
    {
        m_bPaperUnits = true;
        uiCheck |= MF_CHECKED;
    }
    CheckMenuItem(hMnu, IDM_EDITPAPERUNITS, uiCheck);
    return 0;
}

LRESULT CMainWnd::EditUndoCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    int iCnt = m_pUndoObjects->GetCount();
    if(iCnt < 1) return 0;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);

    PDObject pObj = m_pUndoObjects->GetItem(iCnt - 1);
    m_pUndoObjects->ClearSelection(pRegions);
    pObj->SetSelected(true, false, -1, pRegions);

    if(m_pUndoObjects->DeleteSelected(m_pDrawObjects, NULL, pRegions))
    {
        m_iRedoCount++;
        m_pDrawObjects->SetChanged();
        SetTitle(hwnd, false);

        HRGN hRgn = GetUpdateRegion(pRegions);
        //InvalidateRect(hwnd, NULL, true);
        if(hRgn)
        {
            InvalidateRgn(hwnd, hRgn, TRUE);
            DeleteObject(hRgn);
        }
    }

    ClearPolygonList(pRegions);
    delete pRegions;
    return 0;
}

LRESULT CMainWnd::EditRedoCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(m_iRedoCount < 1) return 0;
    int iCnt = m_pDrawObjects->GetCount();
    if(iCnt < 1) return 0;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);

    PDObject pObj = m_pDrawObjects->GetItem(iCnt - 1);
    m_pDrawObjects->ClearSelection(pRegions);
    pObj->SetSelected(true, false, -1, pRegions);

    if(m_pDrawObjects->DeleteSelected(m_pUndoObjects, NULL, pRegions))
    {
        m_iRedoCount--;
        HRGN hRgn = GetUpdateRegion(pRegions);
        //InvalidateRect(hwnd, NULL, true);
        if(hRgn)
        {
            InvalidateRgn(hwnd, hRgn, TRUE);
            DeleteObject(hRgn);
        }
        SetTitle(hwnd, false);
    }

    ClearPolygonList(pRegions);
    delete pRegions;
    return 0;
}

LRESULT CMainWnd::EditConfirmCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    bool bConfirm = (m_iDrawMode == modLine) || (m_iDrawMode == modCircle) ||
        (m_iToolMode == tolRound) || (m_iToolMode == tolCopyPar);
    if(bConfirm)
    {
        wchar_t wBuf[64];
        SendMessage(m_hEdt1, WM_GETTEXT, 64, (LPARAM)wBuf);
        char sBuf[64];
        WideCharToMultiByte(CP_UTF8, 0, wBuf, -1, sBuf, 64, NULL, NULL);
        m_iRestrictSet = ParseInputString(sBuf, m_pFileSetupDlg->GetUnitList(), &m_dRestrictValue);
    }
    return 0;
}

void CMainWnd::GetPageDims()
{
    m_dwPage = m_cFSR.cPaperSize.dPaperWidth;
    m_dhPage = m_cFSR.cPaperSize.dPaperHeight;
    if(m_cFSR.bPortrait)
    {
        if(m_dwPage > m_dhPage)
        {
            m_dwPage = m_cFSR.cPaperSize.dPaperHeight;
            m_dhPage = m_cFSR.cPaperSize.dPaperWidth;
        }
    }
    else if(m_dwPage < m_dhPage)
    {
        m_dwPage = m_cFSR.cPaperSize.dPaperHeight;
        m_dhPage = m_cFSR.cPaperSize.dPaperWidth;
    }
}

LRESULT CMainWnd::ViewFitCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;

    double dwWin = rc.right - rc.left - 20;
    double dhWin = rc.bottom - rc.top - 20;

    double drw = dwWin/m_dwPage;
    double drh = dhWin/m_dhPage;

    if(drw < drh)
    {
        m_dUnitScale = drw;
        m_cViewOrigin.x = 10;
        m_cViewOrigin.y = m_iToolBarHeight + (dhWin + 20 - m_dUnitScale*m_dhPage)/2;
    }
    else
    {
        m_dUnitScale = drh;
        m_cViewOrigin.x = (dwWin + 20 - m_dUnitScale*m_dwPage)/2;
        m_cViewOrigin.y = 10 + m_iToolBarHeight;
    }

    InvalidateRect(hwnd, &rc, TRUE);
    return 0;
}

LRESULT CMainWnd::ViewActSizeCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;

    CDPoint cOrigOff;
    int idx = (rc.right - rc.left)/2.0;
    int idy = (rc.bottom - rc.top)/2.0;
    cOrigOff.x = (idx - m_cViewOrigin.x)/m_dUnitScale;
    cOrigOff.y = (idy - m_cViewOrigin.y)/m_dUnitScale;

    m_dUnitScale = m_dDeviceToUnitScale;

    m_cViewOrigin.x = (int)(idx - cOrigOff.x*m_dUnitScale);
    m_cViewOrigin.y = (int)(idy - cOrigOff.y*m_dUnitScale);

    InvalidateRect(hwnd, &rc, TRUE);
    return 0;
}

LRESULT CMainWnd::ViewGridCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl, int iType)
{
    HMENU hMnu = GetMenu(hwnd);
    UINT uiCheck = MF_BYCOMMAND;
    BOOL bChecked = (m_iDrawGridMode & iType);
    if(bChecked)
    {
        m_iDrawGridMode &= ~iType;
        uiCheck |= MF_UNCHECKED;
    }
    else
    {
        m_iDrawGridMode |= iType;
        uiCheck |= MF_CHECKED;
    }
    CheckMenuItem(hMnu, IDM_VIEWGRIDPTS + iType - 1, uiCheck);
    InvalidateRect(hwnd, NULL, TRUE);
    return 0;
}

/*LRESULT CMainWnd::SnapCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl, int iSnap)
{
    int iSnapBit = (1 << iSnap);
    if(m_iSnapType & iSnapBit) m_iSnapType &= ~iSnapBit;
    else m_iSnapType |= iSnapBit;
    UpdateSnapMenu(GetMenu(hwnd));
    return 0;
}*/

LRESULT CMainWnd::ToolsCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl, int iTool)
{
    if((wNotifyCode == 1) && (iTool == tolDimen) && (IsWindowVisible(m_hEdt1) || IsWindowVisible(m_hEdt2)))
    {
        HWND hFocus = GetFocus();
        char c = 'd';
        if(hFocus == m_hEdt1)
        {
            SendMessage(m_hEdt1, WM_CHAR, c, 0);
        }
        else if(hFocus == m_hEdt2)
        {
            SendMessage(m_hEdt2, WM_CHAR, c, 0);
        }
        return 0;
    }

    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;
    HDC hdc;

    if(m_pActiveObject)
    {
        hdc = GetDC(hwnd);
        IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);

        HBRUSH hPrevBr = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
        int iPrevROP = SetROP2(hdc, R2_NOTXORPEN);
        DrawObject(hwnd, hdc, m_pActiveObject, 1, -2);
        SetROP2(hdc, iPrevROP);
        SelectObject(hdc, hPrevBr);
        SelectClipRgn(hdc, NULL);
        ReleaseDC(hwnd, NULL);

        delete m_pActiveObject;
        m_pActiveObject = NULL;
    }
    else if(m_pSelForDimen)
    {
        m_pSelForDimen->DiscardDimen();
        m_pSelForDimen = NULL;
    }

    if(iTool == tolDimen)
    {
        if(m_pDrawObjects->GetSelectCount() != 1)
        {
            wchar_t sCap[64];
            wchar_t sMsg[128];
            LoadString(m_hInstance, IDS_WARNING, sCap, 64);
            LoadString(m_hInstance, IDS_ONEOBJFORDIMEN, sMsg, 128);
            MessageBox(hwnd, sMsg, sCap, MB_OK | MB_ICONWARNING);
            return 0;
        }
        m_pSelForDimen = m_pDrawObjects->GetSelected(0);
    }

    if(((m_iDrawMode + m_iToolMode > 0) && (iTool == 0)) ||
       ((m_iDrawMode + m_iToolMode == 0) && (iTool > 0)))
    {
        DrawCross(hwnd);
    }

    m_iToolMode = iTool;
    m_iDrawMode = modSelect;

    StartNewObject(hwnd);
    return 0;
}

LRESULT CMainWnd::HelpContentCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    MessageBox(hwnd, L"Help is not implemented yet", L"Information", MB_OK);
    return 0;
}

/*void CMainWnd::UpdateSnapMenu(HMENU hMenu)
{
    int iSnapBit = 1;
    int iSnapCount = IDM_SNAPINTERSECT - IDM_SNAPELEMENT + 1;
    UINT uCheck;
    for(int i = 0; i < iSnapCount; i++)
    {
        uCheck = MF_BYCOMMAND;
        if(m_iSnapType & iSnapBit) uCheck |= MF_CHECKED;
        else uCheck |= MF_UNCHECKED;
        CheckMenuItem(hMenu, IDM_SNAPELEMENT + i, uCheck);
        iSnapBit <<= 1;
    }
}*/

LRESULT CMainWnd::WMMButtonDown(HWND hwnd, WPARAM fwKeys, int xPos, int yPos)
{
    if(m_iButton > 0) return 0;

    m_iButton = 2;
    m_cZoomOrig.x = xPos - m_cViewOrigin.x;
    m_cZoomOrig.y = yPos - m_cViewOrigin.y;
    m_cLastMovePt.x = xPos;
    m_cLastMovePt.y = yPos;
    SetCapture(hwnd);
    return 0;
}

LRESULT CMainWnd::WMMButtonUp(HWND hwnd, WPARAM fwKeys, int xPos, int yPos)
{
    RECT rc;
    if(m_iButton == 2)
    {
        ReleaseCapture();

        //m_cViewOrigin.x = (xPos - m_cZoomOrig.x);
        //m_cViewOrigin.y = (yPos - m_cZoomOrig.y);

        GetClientRect(hwnd, &rc);
        rc.top += m_iToolBarHeight;
        rc.bottom -= m_iStatusHeight;
        InvalidateRect(hwnd, &rc, TRUE);
    }
    m_iButton = 0;
    return 0;
}

LRESULT CMainWnd::WMLButtonDown(HWND hwnd, WPARAM fwKeys, int xPos, int yPos)
{
    if(m_iButton > 0) return 0;

    m_cLastDownPt.x = xPos;
    m_cLastDownPt.y = yPos;
    m_cLastMovePt.x = xPos;
    m_cLastMovePt.y = yPos;
    m_iButton = 1;

    return 0;
}

LRESULT CMainWnd::WMLButtonUp(HWND hwnd, WPARAM fwKeys, int xPos, int yPos)
{
    if(m_iButton != 1) return 0;

    m_iButton = 0;

    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;

    CDRect cdr;
    cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
    cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

    double dTol = (double)m_iSnapTolerance/m_dUnitScale;
    CDPoint cDistPt;
    wchar_t wsBuf[128];
    double dNorm;
    wchar_t *wsUnit;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);
    HRGN hRgn;

    if(m_iDrawMode + m_iToolMode < 1) // selection
    {
        if(!(fwKeys & MK_CONTROL)) m_pDrawObjects->ClearSelection(pRegions);

        if(GetPtDist(&m_cLastDownPt, xPos, yPos) > 4) // select by rectangle
        {
            CDRect cdr1;
            cdr1.cPt1.x = (m_cLastDownPt.x - m_cViewOrigin.x)/m_dUnitScale;
            cdr1.cPt1.y = (m_cLastDownPt.y - m_cViewOrigin.y)/m_dUnitScale;
            cdr1.cPt2.x = (xPos - m_cViewOrigin.x)/m_dUnitScale;
            cdr1.cPt2.y = (yPos - m_cViewOrigin.y)/m_dUnitScale;
            m_pDrawObjects->SelectByRectangle(&cdr1, 2, pRegions);
        }
        else
        {
            if(m_pHighObject)
                m_pHighObject->SetSelected(true, fwKeys & MK_CONTROL, m_iHighDimen, pRegions);
        }
        hRgn = GetUpdateRegion(pRegions);
        //InvalidateRect(hwnd, &rc, TRUE);
        if(hRgn)
        {
            InvalidateRgn(hwnd, hRgn, TRUE);
            DeleteObject(hRgn);
        }
    }
    else if((m_iToolMode > 20) && (m_iToolMode != tolRound))
    {
        if(GetPtDist(&m_cLastDownPt, xPos, yPos) > 4)
        {
            delete pRegions;
            return 0;
        }

        switch(m_iToolMode)
        {
        case tolKnife:
            if(m_pDrawObjects->CutSelected(m_cLastDrawPt, dTol, &cdr, pRegions))
            {
                hRgn = GetUpdateRegion(pRegions);
                //InvalidateRect(hwnd, &rc, TRUE);
                if(hRgn)
                {
                    InvalidateRgn(hwnd, hRgn, TRUE);
                    DeleteObject(hRgn);
                }
                SetTitle(hwnd, false);
            }
            break;
        case tolExtend:
            if(m_pDrawObjects->ExtendSelected(m_cLastDrawPt, dTol, &cdr, pRegions))
            {
                hRgn = GetUpdateRegion(pRegions);
                //InvalidateRect(hwnd, &rc, TRUE);
                if(hRgn)
                {
                    InvalidateRgn(hwnd, hRgn, TRUE);
                    DeleteObject(hRgn);
                }
                SetTitle(hwnd, false);
            }
            break;
        case tolConflict:
            if(m_pDrawObjects->SetCrossSelected(m_cLastDrawPt, dTol, &cdr, pRegions))
            {
                hRgn = GetUpdateRegion(pRegions);
                //InvalidateRect(hwnd, &rc, TRUE);
                if(hRgn)
                {
                    InvalidateRgn(hwnd, hRgn, TRUE);
                    DeleteObject(hRgn);
                }
                SetTitle(hwnd, false);
            }
            break;
        case tolMeas:
            if(!m_cMeasPoint1.bIsSet)
            {
                m_cMeasPoint1.bIsSet = true;
                m_cMeasPoint1.cOrigin = m_cLastDrawPt;
            }
            else if(!m_cMeasPoint2.bIsSet)
            {
                m_cMeasPoint2.bIsSet = true;
                m_cMeasPoint2.cOrigin = m_cLastDrawPt;
                cDistPt = m_cMeasPoint2.cOrigin - m_cMeasPoint1.cOrigin;
                if(m_bPaperUnits)
                {
                    cDistPt /= m_cFSR.cPaperUnit.dBaseToUnit;
                    wsUnit = m_cFSR.cPaperUnit.wsAbbrev;
                }
                else
                {
                    cDistPt /= m_dDrawScale;
                    cDistPt /= m_cFSR.cLenUnit.dBaseToUnit;
                    wsUnit = m_cFSR.cLenUnit.wsAbbrev;
                }
                dNorm = GetNorm(cDistPt);
                swprintf(wsBuf, L"dx: %.3f, dy: %.3f, dist: %.4f (%s)", fabs(cDistPt.x),
                    fabs(cDistPt.y), dNorm, wsUnit);
                SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)wsBuf);
            }
            else
            {
                m_cMeasPoint1.bIsSet = true;
                m_cMeasPoint1.cOrigin = m_cLastDrawPt;
                m_cMeasPoint2.bIsSet = false;
                SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)L"");
            }
            break;
        }
    }
    else
    {
        if(GetPtDist(&m_cLastDownPt, xPos, yPos) > 4)
        {
            delete pRegions;
            return 0;
        }

        RECT rc;
        GetClientRect(hwnd, &rc);
        rc.top += m_iToolBarHeight;
        rc.bottom -= m_iStatusHeight;

        wchar_t buf[64];
        float f;
        int i;
        bool bdValSet = false;
        double dVal = 0;
        int iCop = 0;
        PDObject pSelLine = NULL;
        CDLine cLine;

        if((m_iToolMode > tolCopyPar) && (m_iToolMode < tolMirror))
        {
            SendMessage(m_hEdt1, WM_GETTEXT, 64, (LPARAM)buf);
            if(swscanf(buf, L"%f", &f) == 1)
            {
                dVal = f;
                bdValSet = true;
            }
            SendMessage(m_hEdt2, WM_GETTEXT, 64, (LPARAM)buf);
            if(swscanf(buf, L"%d", &i) == 1) iCop = i;
        }

        if(m_iToolMode == tolMove)
        {
            if(m_cMeasPoint1.bIsSet)
            {
                cLine.bIsSet = true;
                cLine.cOrigin = m_cMeasPoint1.cOrigin;
                cDistPt = m_cLastDrawPt - m_cMeasPoint1.cOrigin;
                dNorm = GetNorm(cDistPt);
                if(dNorm > g_dPrec)
                {
                    cLine.cDirection = cDistPt/dNorm;
                    if(m_pDrawObjects->MoveSelected(cLine, dNorm, iCop, &cdr, true, pRegions))
                    {
                        hRgn = GetUpdateRegion(pRegions);
                        //InvalidateRect(hwnd, &rc, TRUE);
                        if(hRgn)
                        {
                            InvalidateRgn(hwnd, hRgn, TRUE);
                            DeleteObject(hRgn);
                        }
                        SetTitle(hwnd, false);
                        StartNewObject(hwnd);
                    }
                }
                m_iToolMode = tolNone;
                m_cMeasPoint1.bIsSet = false;
            }
            else
            {
                if(bdValSet)
                {
                    pSelLine = m_pDrawObjects->SelectLineByPoint(m_cLastDrawPt, dTol);
                    if(pSelLine)
                    {
                        cLine = pSelLine->GetLine();
                        m_iToolMode = tolNone;
                        if(!m_bPaperUnits) dVal *= m_dDrawScale;
                        if(m_pDrawObjects->MoveSelected(cLine, dVal, iCop, &cdr, false, pRegions))
                        {
                            hRgn = GetUpdateRegion(pRegions);
                            //InvalidateRect(hwnd, &rc, TRUE);
                            if(hRgn)
                            {
                                InvalidateRgn(hwnd, hRgn, TRUE);
                                DeleteObject(hRgn);
                            }
                            SetTitle(hwnd, false);
                            StartNewObject(hwnd);
                        }
                    }
                }
                else
                {
                    m_cMeasPoint1.cOrigin = m_cLastDrawPt;
                    m_cMeasPoint1.bIsSet = true;
                    LoadString(m_hInstance, IDS_SELPOINTTOMOVE, m_wsStatus2Msg, 128);
                    SendMessage(m_hStatus, SB_SETTEXT, 2, (LPARAM)m_wsStatus2Msg);
                }
            }
        }
        else if(m_iToolMode == tolRotate)
        {
            if(bdValSet)
            {
                //if(m_cFSR.iAngUnit < 1) dVal *= M_PI/180.0;
                dVal *= M_PI/180.0/m_cFSR.cAngUnit.dBaseToUnit;

                m_iToolMode = tolNone;
                if(m_pDrawObjects->RotateSelected(m_cLastDrawPt, -dVal, iCop, &cdr, pRegions))
                {
                    hRgn = GetUpdateRegion(pRegions);
                    //InvalidateRect(hwnd, &rc, TRUE);
                    if(hRgn)
                    {
                        InvalidateRgn(hwnd, hRgn, TRUE);
                        DeleteObject(hRgn);
                    }
                    SetTitle(hwnd, false);
                    StartNewObject(hwnd);
                }
            }
            else if(m_cMeasPoint2.bIsSet)
            {
                CDPoint cDir = m_cMeasPoint2.cOrigin - m_cMeasPoint1.cOrigin;
                double dNorm = GetNorm(cDir);
                if(dNorm > g_dPrec)
                {
                    cDir /= dNorm;
                    CDPoint cPt1 = Rotate(m_cLastDrawPt - m_cMeasPoint1.cOrigin, cDir, false);
                    dVal = atan2(cPt1.y, cPt1.x);
                    m_cMeasPoint1.bIsSet = false;
                    m_cMeasPoint2.bIsSet = false;
                    m_iToolMode = tolNone;
                    if(m_pDrawObjects->RotateSelected(m_cMeasPoint1.cOrigin, dVal, iCop, &cdr, pRegions))
                    {
                        hRgn = GetUpdateRegion(pRegions);
                        //InvalidateRect(hwnd, &rc, TRUE);
                        if(hRgn)
                        {
                            InvalidateRgn(hwnd, hRgn, TRUE);
                            DeleteObject(hRgn);
                        }
                        SetTitle(hwnd, false);
                        StartNewObject(hwnd);
                    }
                }
            }
            else if(m_cMeasPoint1.bIsSet)
            {
                m_cMeasPoint2.bIsSet = true;
                m_cMeasPoint2.cOrigin = m_cLastDrawPt;
                LoadString(m_hInstance, IDS_SELPOINTTOROT, m_wsStatus2Msg, 128);
                SendMessage(m_hStatus, SB_SETTEXT, 2, (LPARAM)m_wsStatus2Msg);
            }
            else
            {
                m_cMeasPoint1.bIsSet = true;
                m_cMeasPoint1.cOrigin = m_cLastDrawPt;
                LoadString(m_hInstance, IDS_SELPOINTFROMROT, m_wsStatus2Msg, 128);
                SendMessage(m_hStatus, SB_SETTEXT, 2, (LPARAM)m_wsStatus2Msg);
            }
        }
        else if(m_iToolMode == tolMirror)
        {
            pSelLine = m_pDrawObjects->SelectLineByPoint(m_cLastDrawPt, dTol);
            if(pSelLine)
            {
                cLine = pSelLine->GetLine();
                m_iToolMode = tolNone;
                if(m_pDrawObjects->MirrorSelected(cLine, &cdr, pRegions))
                {
                    hRgn = GetUpdateRegion(pRegions);
                    //InvalidateRect(hwnd, &rc, TRUE);
                    if(hRgn)
                    {
                        InvalidateRgn(hwnd, hRgn, TRUE);
                        DeleteObject(hRgn);
                    }
                    SetTitle(hwnd, false);
                    StartNewObject(hwnd);
                }
            }
        }
        else if(m_iToolMode == tolDimen)
        {
            if(m_pDrawObjects->AddDimen(m_pSelForDimen, m_cLastDrawPt, dTol, &cdr, pRegions))
            {
                hRgn = GetUpdateRegion(pRegions);
                //InvalidateRect(hwnd, &rc, TRUE);
                if(hRgn)
                {
                    InvalidateRgn(hwnd, hRgn, TRUE);
                    DeleteObject(hRgn);
                }
                SetTitle(hwnd, false);
            }
        }
        else
        {
            int iCtrl = 0;
            if(m_iToolMode == tolCopyPar)
            {
                iCtrl = 2;
                if(fwKeys & MK_SHIFT) iCtrl = 3;
            }
            if(m_cLastDynPt.bIsSet)
            {
                CDInputPoint cInPt;
                m_pActiveObject->GetPoint(0, 0, &cInPt);
                cInPt.cPoint = m_cLastDynPt.cOrigin;
                m_pActiveObject->SetPoint(0, 0, cInPt);
            }
            if(m_pActiveObject->AddPoint(m_cLastDrawPt.x, m_cLastDrawPt.y, iCtrl, true))
            {
                m_pActiveObject->AddRegions(pRegions, -1);
                m_pDrawObjects->Add(m_pActiveObject);
                SetTitle(hwnd, false);
                m_pActiveObject = NULL;
                m_iToolMode = tolNone;
                m_wsStatus2Base[0] = 0;
                wcscpy(m_wsStatus2Msg, m_wsStatus2Base);
                SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Msg);

                hRgn = GetUpdateRegion(pRegions);
                //InvalidateRect(hwnd, &rc, TRUE);
                if(hRgn)
                {
                    InvalidateRgn(hwnd, hRgn, TRUE);
                    DeleteObject(hRgn);
                }
                StartNewObject(hwnd);
            }
        }
    }

    ClearPolygonList(pRegions);
    delete pRegions;

    m_cLastDynPt.bIsSet = false;
    return 0;
}

LRESULT CMainWnd::WMRButtonDown(HWND hwnd, WPARAM fwKeys, int xPos, int yPos)
{
    if(m_iButton > 0) return 0;

    m_cLastDownPt.x = xPos;
    m_cLastDownPt.y = yPos;
    m_cLastMovePt.x = xPos;
    m_cLastMovePt.y = yPos;
    m_iButton = 3;

    return 0;
}

LRESULT CMainWnd::WMRButtonUp(HWND hwnd, WPARAM fwKeys, int xPos, int yPos)
{
    if(m_iButton != 3) return 0;

    m_iButton = 0;

    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);
    HRGN hRgn;

    if(m_iDrawMode + m_iToolMode < 1) // selection
    {
        if(GetPtDist(&m_cLastDownPt, xPos, yPos) > 4) // select by rectangle
        {
            if(!(fwKeys & MK_CONTROL)) m_pDrawObjects->ClearSelection(pRegions);
            //MessageBox(0, L"Dobry", L"Debug", MB_OK);
            CDRect cdr1;
            cdr1.cPt1.x = (m_cLastDownPt.x - m_cViewOrigin.x)/m_dUnitScale;
            cdr1.cPt1.y = (m_cLastDownPt.y - m_cViewOrigin.y)/m_dUnitScale;
            cdr1.cPt2.x = (xPos - m_cViewOrigin.x)/m_dUnitScale;
            cdr1.cPt2.y = (yPos - m_cViewOrigin.y)/m_dUnitScale;
            m_pDrawObjects->SelectByRectangle(&cdr1, 1, pRegions);

            hRgn = GetUpdateRegion(pRegions);
            //InvalidateRect(hwnd, &rc, TRUE);
            if(hRgn)
            {
                InvalidateRgn(hwnd, hRgn, TRUE);
                DeleteObject(hRgn);
            }
        }
        else
        {
            if(m_pHighObject)
            {
                HMENU hMenu = LoadMenu(m_hInstance, L"POPUPMENU");
                if(m_pHighObject->GetSnapTo()) hMenu = GetSubMenu(hMenu, 1);
                else hMenu = GetSubMenu(hMenu, 0);
                POINT pt = {xPos, yPos};
                ClientToScreen(hwnd, &pt);
                TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
                    pt.x, pt.y, 0, hwnd, NULL);
            }
        }
    }
    else if(m_iToolMode > tolNone)
    {
    }
    else
    {
        if(GetPtDist(&m_cLastDownPt, xPos, yPos) > 4)
        {
            delete pRegions;
            return 0;
        }

        if(m_cLastDynPt.bIsSet)
        {
            CDInputPoint cInPt;
            m_pActiveObject->GetPoint(0, 0, &cInPt);
            cInPt.cPoint = m_cLastDynPt.cOrigin;
            m_pActiveObject->SetPoint(0, 0, cInPt);
        }
        if(m_pActiveObject->AddPoint(m_cLastDrawPt.x, m_cLastDrawPt.y, 1, true))
        {
            m_pActiveObject->AddRegions(pRegions, -1);
            m_pDrawObjects->Add(m_pActiveObject);
            SetTitle(hwnd, false);
            m_pActiveObject = NULL;
            m_iToolMode = tolNone;
            m_wsStatus2Base[0] = 0;
            wcscpy(m_wsStatus2Msg, m_wsStatus2Base);
            SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Msg);

            hRgn = GetUpdateRegion(pRegions);
            //InvalidateRect(hwnd, &rc, TRUE);
            if(hRgn)
            {
                InvalidateRgn(hwnd, hRgn, TRUE);
                DeleteObject(hRgn);
            }

            StartNewObject(hwnd);
        }
    }

    ClearPolygonList(pRegions);
    delete pRegions;

    m_cLastDynPt.bIsSet = false;
    return 0;
}

bool PointsEqual(POINT cPt1, POINT cPt2)
{
    return (cPt1.x == cPt2.x) && (cPt1.y == cPt2.y);
}

void CMainWnd::DrawDimArrow(HDC hdc, PDPrimitive pPrim)
{
    POINT cStartPt, cEndPt, cPoly[3];
    int iType = Round(pPrim->cPt1.x);
    HBRUSH hBr, hPrevBr;
    hBr = (HBRUSH)GetStockObject(BLACK_BRUSH);

    switch(iType)
    {
    case 1:
        cStartPt.x = pPrim->cPt3.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt3.y + m_cViewOrigin.y;
        MoveToEx(hdc, cStartPt.x, cStartPt.y, NULL);
        cEndPt.x = pPrim->cPt2.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt2.y + m_cViewOrigin.y;
        LineTo(hdc, cEndPt.x, cEndPt.y);
        cEndPt.x = pPrim->cPt4.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt4.y + m_cViewOrigin.y;
        LineTo(hdc, cEndPt.x, cEndPt.y);
        break;
    case 2:
        hPrevBr = (HBRUSH)SelectObject(hdc, hBr);
        cPoly[0].x = pPrim->cPt3.x + m_cViewOrigin.x;
        cPoly[0].y = pPrim->cPt3.y + m_cViewOrigin.y;
        cPoly[1].x = pPrim->cPt2.x + m_cViewOrigin.x;
        cPoly[1].y = pPrim->cPt2.y + m_cViewOrigin.y;
        cPoly[2].x = pPrim->cPt4.x + m_cViewOrigin.x;
        cPoly[2].y = pPrim->cPt4.y + m_cViewOrigin.y;
        Polygon(hdc, cPoly, 3);
        SelectObject(hdc, hPrevBr);
        break;
    case 3:
        hPrevBr = (HBRUSH)SelectObject(hdc, hBr);
        cStartPt.x = Round(pPrim->cPt3.x - pPrim->cPt2.x);
        cStartPt.y = Round(pPrim->cPt3.y - pPrim->cPt2.y);
        Ellipse(hdc, pPrim->cPt2.x + m_cViewOrigin.x - cStartPt.x,
            pPrim->cPt2.y + m_cViewOrigin.y - cStartPt.y,
            pPrim->cPt2.x + m_cViewOrigin.x + cStartPt.x,
            pPrim->cPt2.y + m_cViewOrigin.y + cStartPt.y);
        SelectObject(hdc, hPrevBr);
        break;
    case 4:
    case 5:
        cStartPt.x = pPrim->cPt3.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt3.y + m_cViewOrigin.y;
        MoveToEx(hdc, cStartPt.x, cStartPt.y, NULL);
        cEndPt.x = pPrim->cPt4.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt4.y + m_cViewOrigin.y;
        LineTo(hdc, cEndPt.x, cEndPt.y);
        break;
    }
}

void CMainWnd::DrawDimText(HWND hWnd, HDC hdc, PDPrimitive pPrim, PDObject pObj, DWORD dwColor,
    double dLineWidth)
{
    POINT cStartPt, cEndPt;
    int iPos = Round(pPrim->cPt2.y);

    char sBuf[64];
    char *psBuf = sBuf;
    int iLen = pObj->PreParseDimText(iPos, psBuf, 64, m_dDrawScale, m_pFileSetupDlg->GetUnitList());
    if(iLen > 0)
    {
        psBuf = (char*)malloc(iLen*sizeof(char));
        pObj->PreParseDimText(iPos, psBuf, iLen, m_dDrawScale, m_pFileSetupDlg->GetUnitList());
    }

    int iLen2 = strlen(psBuf);

    CDFileAttrs cFileAttrs;
    if(iPos < 0) m_pDrawObjects->GetFileAttrs(&cFileAttrs);
    else pObj->GetDimFontAttrs(iPos, &cFileAttrs);

    double dPi2 = M_PI/2.0;
    double dco = cos(pPrim->cPt2.x - dPi2);
    double dsi = sin(pPrim->cPt2.x - dPi2);

    double dx1 = 100.0*(pPrim->cPt1.x + m_cViewOrigin.x)/m_dUnitScale;
    double dy1 = -100.0*(pPrim->cPt1.y + m_cViewOrigin.y)/m_dUnitScale;
    double da = cFileAttrs.dFontSize/2.0;
    double dx2, dy2;

    bool bDiam = (psBuf[0] == '*');
    int iStart = 0;
    if(bDiam) iStart = 1;
    if(iStart >= iLen2)
    {
        psBuf[iStart] = '?';
        psBuf[iStart + 1] = 0;
        iLen2++;
    }

    int iwLen = MultiByteToWideChar(CP_UTF8, 0, &psBuf[iStart], -1, NULL, 0) + 1;
    LPWSTR wsBufStart = (LPWSTR)malloc(iwLen*sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, &psBuf[iStart], -1, wsBufStart, iwLen);
    int iLenStart = wcslen(wsBufStart);
    int iLenNom = 0;
    int iLenDenom = 0;
    int iLenEnd = 0;

    LPWSTR wsFrac = wcschr(wsBufStart, '_');
    wchar_t wsBufNom[4];
    wchar_t wsBufDenom[4];
    LPWSTR wsBufEnd = NULL;

    if(wsFrac)
    {
        wsBufEnd = wsFrac + 1;
        int i = 0;
        while((*wsBufEnd >= '0') && (*wsBufEnd <= '9') && (i < 3))
        {
            wsBufNom[i++] = *(wsBufEnd++);
        }
        wsBufNom[i] = 0;

        while(*wsBufEnd && (*wsBufEnd != '/')) wsBufEnd++;
        if(*wsBufEnd) wsBufEnd++;
        i = 0;
        while((*wsBufEnd >= '0') && (*wsBufEnd <= '9') && (i < 3))
        {
            wsBufDenom[i++] = *(wsBufEnd++);
        }
        wsBufDenom[i] = 0;
        *wsFrac = 0;

        iLenStart = wcslen(wsBufStart);
        iLenNom = wcslen(wsBufNom);
        iLenDenom = wcslen(wsBufDenom);
        iLenEnd = wcslen(wsBufEnd);
    }

    LOGFONT lFnt;
    lFnt.lfHeight = -Round(400.0*cFileAttrs.dFontSize/3.0);
    lFnt.lfWidth = 0;
    lFnt.lfEscapement = 0;
    lFnt.lfOrientation = 0;
    lFnt.lfWeight = FW_NORMAL;
    if(cFileAttrs.bFontAttrs & 8) lFnt.lfWeight = FW_BOLD;
    lFnt.lfItalic = cFileAttrs.bFontAttrs & 1;
    lFnt.lfUnderline = cFileAttrs.bFontAttrs & 2;
    lFnt.lfStrikeOut = cFileAttrs.bFontAttrs & 4;
    lFnt.lfCharSet = DEFAULT_CHARSET;
    lFnt.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lFnt.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lFnt.lfQuality = DEFAULT_QUALITY;
    lFnt.lfPitchAndFamily = DEFAULT_PITCH;
    MultiByteToWideChar(CP_UTF8, 0, cFileAttrs.sFontFace, -1, lFnt.lfFaceName, LF_FACESIZE);
    HFONT hPrevFnt, hFnt, hFntSm;
    hFnt = CreateFontIndirect(&lFnt);
    if(wsFrac)
    {
        lFnt.lfHeight = -Round(80*cFileAttrs.dFontSize);
        hFntSm = CreateFontIndirect(&lFnt);
    }

    RECT R;
    GetClientRect(hWnd, &R);

    int iDC = SaveDC(hdc);

    SetGraphicsMode(hdc, GM_ADVANCED);

    double dWidth = GetDeviceCaps(hdc, HORZSIZE); // mm
    double dRes = GetDeviceCaps(hdc, HORZRES); // pt
    double dScale = m_dUnitScale*dWidth/dRes;

    SetMapMode(hdc, MM_HIMETRIC);

    POINT pt = {R.right, R.bottom};
    DPtoLP(hdc, &pt, 1);

    XFORM xMat = {dScale*dco, -dScale*dsi, dScale*dsi, dScale*dco, dx1*dScale, dy1*dScale};
    SetWorldTransform(hdc, &xMat);

    hPrevFnt = (HFONT)SelectObject(hdc, hFnt);

    if(iPos > -1) SetBkMode(hdc, TRANSPARENT);

    PDDimension pDim = pObj->GetDimen(iPos);

    SetTextColor(hdc, dwColor);

    SetTextAlign(hdc, TA_BASELINE | TA_LEFT);

    SIZE cSizeStart, cSizeNom, cSizeDenom, cSizeEnd;
    GetTextExtentPoint32(hdc, wsBufStart, iLenStart, &cSizeStart);
    int iTextWidth = cSizeStart.cx;
    if(bDiam) iTextWidth += Round(150.0*da);

    if(wsFrac)
    {
        GetTextExtentPoint32(hdc, wsBufEnd, iLenEnd, &cSizeEnd);
        SelectObject(hdc, hFntSm);
        GetTextExtentPoint32(hdc, wsBufNom, iLenNom, &cSizeNom);
        GetTextExtentPoint32(hdc, wsBufDenom, iLenDenom, &cSizeDenom);
        iTextWidth += (cSizeEnd.cx + cSizeNom.cx + cSizeDenom.cx + Round(3.0*da));
        SelectObject(hdc, hFnt);
    }

    pDim->cExt.cPt1.x = -iTextWidth/200.0;
    pDim->cExt.cPt1.y = 0.0;
    pDim->cExt.cPt2.x = iTextWidth/200.0;
    pDim->cExt.cPt2.y = 1.15*cFileAttrs.dFontSize;

    int iWidth = Round(100.0*dLineWidth);
    HPEN hPen = CreatePen(PS_SOLID, iWidth, dwColor);
    HPEN hPrevPen = (HPEN)SelectObject(hdc, hPen);

    int ix = -iTextWidth/2;
    if(bDiam) ix += Round(150.0*da);
    TextOut(hdc, ix, 0, wsBufStart, iLenStart);

    if(wsFrac)
    {
        ix += cSizeStart.cx + Round(da);
        SelectObject(hdc, hFntSm);
        TextOut(hdc, ix, 120.0*da, wsBufNom, iLenNom);
        ix += cSizeNom.cx + Round(2.0*da);
        TextOut(hdc, ix, -20.0*da, wsBufDenom, iLenDenom);

        MoveToEx(hdc, ix - Round(50.0*da), Round(55.0*da), NULL);
        LineTo(hdc, ix + Round(50.0*da), Round(155.0*da));

        SelectObject(hdc, hFnt);
        ix += cSizeDenom.cx;
        TextOut(hdc, ix, 0, wsBufEnd, iLenEnd);
    }

    if(bDiam)
    {
        dx2 = 60.0*da - iTextWidth/2; //ix - cSize.cx/2.0 - 180.0;
        dy2 = 90.0*da;
        da *= 60.0;
        Ellipse(hdc, Round(dx2 - da), Round(dy2 - da), Round(dx2 + da), Round(dy2 + da));
        MoveToEx(hdc, Round(dx2 - da), Round(dy2 - da), NULL);
        LineTo(hdc, Round(dx2 + da), Round(dy2 + da));
    }

    //SelectObject(hdc, GetStockObject(BLACK_PEN));
    //Rectangle(hdc, 100.0*pDim->cExt.cPt1.x, 100.0*pDim->cExt.cPt1.y,
    //    100.0*pDim->cExt.cPt2.x, 100.0*pDim->cExt.cPt2.y);
    //Rectangle(hdc, -100, -100, 100, 100);
    DeleteObject(SelectObject(hdc, hPrevPen));

    if(iPos > -1) SetBkMode(hdc, OPAQUE);

    DeleteObject(SelectObject(hdc, hPrevFnt));
    free(wsBufStart);

    RestoreDC(hdc, iDC);

    if(iLen > 0) free(psBuf);
}

void CMainWnd::DrawPrimitive(HDC hdc, PDPrimitive pPrim)
{
    double dr;
    POINT pPts[3];

    POINT cStartPt, cEndPt;
    CDPoint cPt1, cPt2;

    switch(pPrim->iType)
    {
    case 1:
        cStartPt.x = pPrim->cPt1.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt1.y + m_cViewOrigin.y;
        MoveToEx(hdc, cStartPt.x, cStartPt.y, NULL);
        cEndPt.x = pPrim->cPt2.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt2.y + m_cViewOrigin.y;
        LineTo(hdc, cEndPt.x, cEndPt.y);
        break;
    case 2:
        cStartPt.x = pPrim->cPt3.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt3.y + m_cViewOrigin.y;
        cEndPt.x = pPrim->cPt4.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt4.y + m_cViewOrigin.y;

        dr = GetPtDist(&cStartPt, cEndPt.x, cEndPt.y);
        if(dr > 2)
        {
            dr = (pPrim->cPt2.x - pPrim->cPt1.x);
            Arc(hdc, m_cViewOrigin.x + pPrim->cPt1.x - dr, m_cViewOrigin.y + pPrim->cPt1.y - dr,
                m_cViewOrigin.x + pPrim->cPt1.x + dr, m_cViewOrigin.y + pPrim->cPt1.y + dr,
                cStartPt.x, cStartPt.y, cEndPt.x, cEndPt.y);
        }
        else
        {
            MoveToEx(hdc, cStartPt.x, cStartPt.y, NULL);
            LineTo(hdc, cEndPt.x, cEndPt.y);
        }
        break;
    case 3:
        dr = (pPrim->cPt2.x - pPrim->cPt1.x);
        Arc(hdc, m_cViewOrigin.x + pPrim->cPt1.x - dr, m_cViewOrigin.y + pPrim->cPt1.y - dr,
            m_cViewOrigin.x + pPrim->cPt1.x + dr, m_cViewOrigin.y + pPrim->cPt1.y + dr,
            m_cViewOrigin.x + pPrim->cPt1.x + dr, m_cViewOrigin.y + pPrim->cPt1.y,
            m_cViewOrigin.x + pPrim->cPt1.x + dr, m_cViewOrigin.y + pPrim->cPt1.y);
        break;
    case 4:
        cStartPt.x = pPrim->cPt1.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt1.y + m_cViewOrigin.y;
        MoveToEx(hdc, cStartPt.x, cStartPt.y, NULL);

        cPt1 = (pPrim->cPt1 + 2.0*pPrim->cPt2)/3.0;
        cPt2 = (pPrim->cPt3 + 2.0*pPrim->cPt2)/3.0;

        pPts[0].x = (int)m_cViewOrigin.x + cPt1.x;
        pPts[0].y = (int)m_cViewOrigin.y + cPt1.y;
        pPts[1].x = (int)m_cViewOrigin.x + cPt2.x;
        pPts[1].y = (int)m_cViewOrigin.y + cPt2.y;
        pPts[2].x = (int)m_cViewOrigin.x + pPrim->cPt3.x;
        pPts[2].y = (int)m_cViewOrigin.y + pPrim->cPt3.y;
        PolyBezierTo(hdc, pPts, 3);
        break;
    case 5:
        cStartPt.x = pPrim->cPt1.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt1.y + m_cViewOrigin.y;
        MoveToEx(hdc, cStartPt.x, cStartPt.y, NULL);

        pPts[0].x = (int)m_cViewOrigin.x + pPrim->cPt2.x;
        pPts[0].y = (int)m_cViewOrigin.y + pPrim->cPt2.y;
        pPts[1].x = (int)m_cViewOrigin.x + pPrim->cPt3.x;
        pPts[1].y = (int)m_cViewOrigin.y + pPrim->cPt3.y;
        pPts[2].x = (int)m_cViewOrigin.x + pPrim->cPt4.x;
        pPts[2].y = (int)m_cViewOrigin.y + pPrim->cPt4.y;
        PolyBezierTo(hdc, pPts, 3);
        break;
    case 7:
        cStartPt.x = pPrim->cPt1.x + m_cViewOrigin.x - 6;
        cStartPt.y = pPrim->cPt1.y + m_cViewOrigin.y;
        MoveToEx(hdc, cStartPt.x, cStartPt.y, NULL);
        LineTo(hdc, cStartPt.x + 13, cStartPt.y);
        cEndPt.x = pPrim->cPt1.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt1.y + m_cViewOrigin.y + 7;
        MoveToEx(hdc, cEndPt.x, cEndPt.y - 13, NULL);
        LineTo(hdc, cEndPt.x, cEndPt.y);
        break;
    case 9:
        DrawDimArrow(hdc, pPrim);
        break;
    }
}

void CMainWnd::DrawObject(HWND hWnd, HDC hdc, PDObject pObj, int iMode, int iDimen)
{
    bool bSel = pObj->GetSelected();
    CDLineStyle cStyle = pObj->GetLineStyle();

    DWORD dwColor = 0;
    if(iMode == 1) dwColor = m_lActiveColor;
    else if(iMode == 2) dwColor = m_lHighColor;
    else if(bSel) dwColor = m_lSelColor;

    int iWidth = Round(fabs(cStyle.dWidth)*m_dUnitScale);
    int iPtRad = iWidth;
    if(iPtRad < 2) iPtRad = 2;
    HPEN hPen = CreatePen(PS_SOLID, iWidth, dwColor);
    HPEN hPtPen = CreatePen(PS_SOLID, 0, dwColor);
    HPEN hCentPen = CreatePen(PS_SOLID, 0, 0x00888888);

    LOGBRUSH lb;
    lb.lbStyle = BS_SOLID;
    lb.lbColor = dwColor/2;
    HBRUSH hBr = CreateBrushIndirect(&lb);
    HBRUSH hPrevBr;

    HPEN hPrevPen;
    hPrevPen = (HPEN)SelectObject(hdc, hPen);

    CDPrimitive cPrim;
    PDDimension pDim;
    pObj->GetFirstPrimitive(&cPrim, m_dUnitScale, iDimen);

    if(iDimen < -1)
    {
        while(cPrim.iType > 0)
        {
            if(cPrim.iType == 6)
            {
                SelectObject(hdc, hPtPen);
                hPrevBr = (HBRUSH)SelectObject(hdc, hBr);
                Ellipse(hdc, cPrim.cPt1.x + m_cViewOrigin.x - iPtRad,
                    cPrim.cPt1.y + m_cViewOrigin.y - iPtRad,
                    cPrim.cPt1.x + m_cViewOrigin.x + iPtRad,
                    cPrim.cPt1.y + m_cViewOrigin.y + iPtRad);
                SelectObject(hdc, hPrevBr);
                SelectObject(hdc, hPen);
            }
            else if(cPrim.iType == 7)
            {
                if(iMode == 0) SelectObject(hdc, hCentPen);
                else SelectObject(hdc, hPtPen);
                DrawPrimitive(hdc, &cPrim);
                SelectObject(hdc, hPen);
            }
            else if(cPrim.iType == 8)
            {
                SelectObject(hdc, hPtPen);
                hPrevBr = (HBRUSH)SelectObject(hdc, hBr);
                Rectangle(hdc, cPrim.cPt1.x + m_cViewOrigin.x - iPtRad,
                    cPrim.cPt1.y + m_cViewOrigin.y - iPtRad,
                    cPrim.cPt1.x + m_cViewOrigin.x + iPtRad,
                    cPrim.cPt1.y + m_cViewOrigin.y + iPtRad);
                SelectObject(hdc, hPrevBr);
                SelectObject(hdc, hPen);
            }
            else if(cPrim.iType == 10)
            {
                DrawDimText(hWnd, hdc, &cPrim, pObj, dwColor, fabs(cStyle.dWidth));
            }
            else DrawPrimitive(hdc, &cPrim);
            pObj->GetNextPrimitive(&cPrim, m_dUnitScale, iDimen);
        }

        if(iMode == 0)
        {
            SelectObject(hdc, hPrevPen);
            DeleteObject(hPen);
            hPen = CreatePen(PS_SOLID, iWidth, 0);
            HPEN hSelPen = CreatePen(PS_SOLID, iWidth, m_lSelColor);
            hPrevPen = (HPEN)SelectObject(hdc, hPen);
            for(int i = 0; i < pObj->GetDimenCount(); i++)
            {
                pDim = pObj->GetDimen(i);
                if(pDim->bSelected)
                {
                    dwColor = m_lSelColor;
                    SelectObject(hdc, hSelPen);
                }
                else
                {
                    dwColor = 0;
                    SelectObject(hdc, hPen);
                }

                pObj->GetFirstPrimitive(&cPrim, m_dUnitScale, i);
                while(cPrim.iType > 0)
                {
                    if(cPrim.iType == 10)
                    {
                        DrawDimText(hWnd, hdc, &cPrim, pObj, dwColor, fabs(cStyle.dWidth));
                    }
                    else DrawPrimitive(hdc, &cPrim);
                    pObj->GetNextPrimitive(&cPrim, m_dUnitScale, i);
                }
            }
            SelectObject(hdc, hPen);
            DeleteObject(hSelPen);
        }
    }
    else
    {
        if((iMode < 1) && (iDimen > -1))
        {
            pDim = pObj->GetDimen(iDimen);
            if(pDim->bSelected) dwColor = m_lSelColor;
            else dwColor = 0;
            SelectObject(hdc, hPrevPen);
            DeleteObject(hPen);
            hPen = CreatePen(PS_SOLID, iWidth, dwColor);
            hPrevPen = (HPEN)SelectObject(hdc, hPen);
        }

        while(cPrim.iType > 0)
        {
            if(cPrim.iType == 10)
            {
                DrawDimText(hWnd, hdc, &cPrim, pObj, dwColor, fabs(cStyle.dWidth));
            }
            else DrawPrimitive(hdc, &cPrim);
            pObj->GetNextPrimitive(&cPrim, m_dUnitScale, iDimen);
        }
    }

    SelectObject(hdc, hPrevPen);
    DeleteObject(hBr);
    DeleteObject(hCentPen);
    DeleteObject(hPtPen);
    DeleteObject(hPen);
}

int CMainWnd::GetDynMode()
{
    int iRes = 0;
    if(m_iDrawMode > modSelect) iRes = 1;
    else if(m_iToolMode == tolCopyPar) iRes = 2;
    else if(m_iToolMode == tolRound) iRes = 3;
    else if(m_iToolMode == tolDimen) iRes = 4;
    return iRes;
}

LRESULT CMainWnd::WMMouseMove(HWND hwnd, WPARAM fwKeys, int xPos, int yPos)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;

    if(m_iButton == 2)
    {
        m_cViewOrigin.x = (xPos - m_cZoomOrig.x);
        m_cViewOrigin.y = (yPos - m_cZoomOrig.y);

        int ix = xPos - m_cLastMovePt.x;
        int iy = yPos - m_cLastMovePt.y;
        m_cLastSnapPt.x += ix;
        m_cLastSnapPt.y += iy;
        ScrollWindowEx(hwnd, ix, iy, &rc, &rc, NULL, NULL, SW_ERASE | SW_INVALIDATE);
        m_cLastMovePt.x = xPos;
        m_cLastMovePt.y = yPos;
        return 0;
    }

    double dx = (xPos - m_cViewOrigin.x)/m_dUnitScale;
    double dy = (yPos - m_cViewOrigin.y)/m_dUnitScale;

    HDC hdc;
    HPEN hPrevPen;
    HBRUSH hPrevBr;
    int iPrevROP;
    double dTol;

    CDRect cdr;
    int iDynMode = GetDynMode();

    if(m_iDrawMode + m_iToolMode < 1)
    {
        if((m_iButton == 1) || (m_iButton == 3))
        {
            hdc = GetDC(hwnd);
            IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
            hPrevPen = (HPEN)SelectObject(hdc, m_hSelPen);
            hPrevBr = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
            iPrevROP = SetROP2(hdc, R2_NOTXORPEN);
            Rectangle(hdc, m_cLastDownPt.x, m_cLastDownPt.y, m_cLastMovePt.x, m_cLastMovePt.y);

            m_cLastMovePt.x = xPos;
            m_cLastMovePt.y = yPos;
            Rectangle(hdc, m_cLastDownPt.x, m_cLastDownPt.y, m_cLastMovePt.x, m_cLastMovePt.y);

            SetROP2(hdc, iPrevROP);
            SelectObject(hdc, hPrevBr);
            SelectObject(hdc, hPrevPen);
            SelectClipRgn(hdc, NULL);
            ReleaseDC(hwnd, NULL);
        }
        else if(m_iButton < 1)
        {
            CDPoint cPt = {dx, dy};
            dTol = (double)m_iSelectTolerance/m_dUnitScale;

            int iDimen;
            PDObject pNewHigh = m_pDrawObjects->SelectByPoint(cPt, dTol, &iDimen);

            if((m_pHighObject != pNewHigh) || (iDimen != m_iHighDimen))
            {
                hdc = GetDC(hwnd);
                IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
                HBRUSH hPrevBr = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
                if(m_pHighObject) DrawObject(hwnd, hdc, m_pHighObject, 0, m_iHighDimen);
                if(pNewHigh) DrawObject(hwnd, hdc, pNewHigh, 2, iDimen);
                SelectObject(hdc, hPrevBr);
                SelectClipRgn(hdc, NULL);
                ReleaseDC(hwnd, NULL);
                m_pHighObject = pNewHigh;
                m_iHighDimen = iDimen;
            }
        }
    }

    wchar_t buf[64];
    swprintf(buf, L"%.3f, %.3f", dx/m_cFSR.cPaperUnit.dBaseToUnit, dy/m_cFSR.cPaperUnit.dBaseToUnit);
    SendMessage(m_hStatus, SB_SETTEXT, 0, (LPARAM)buf);

    if(m_iButton > 0) return 0;

    int iCnt = 0;
    PDObject pObj1, pObj2;

    if(m_iDrawMode + m_iToolMode > 0)
    {
        hdc = GetDC(hwnd);
        IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
        hPrevPen = (HPEN)SelectObject(hdc, m_hRedPen);
        hPrevBr = (HBRUSH)SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
        iPrevROP = SetROP2(hdc, R2_NOTXORPEN);
        MoveToEx(hdc, m_cLastSnapPt.x - 10, m_cLastSnapPt.y, NULL);
        LineTo(hdc, m_cLastSnapPt.x + 10, m_cLastSnapPt.y);
        MoveToEx(hdc, m_cLastSnapPt.x, m_cLastSnapPt.y - 10, NULL);
        LineTo(hdc, m_cLastSnapPt.x, m_cLastSnapPt.y + 10);

        if(m_pActiveObject) DrawObject(hwnd, hdc, m_pActiveObject, 1, -2);

        if(iDynMode == 4)
        {
            iCnt = m_pDrawObjects->GetSelectCount();
            if(iCnt == 1)
            {
                pObj1 = m_pDrawObjects->GetSelected(0);
                DrawObject(hwnd, hdc, pObj1, 1, -1);
            }
        }
    }

    m_cLastSnapPt.x = xPos;
    m_cLastSnapPt.y = yPos;

    if(m_iDrawMode + m_iToolMode > 0)
    {
        CDLine cSnapPt;
        bool bHasLastPoint = false;
        CDInputPoint cLstInPt;
        CDPoint cDir1, cDir2;
        double dAng1;
        bool bDoSnap = true;

        if((m_iDrawMode == modLine) && m_pActiveObject)
        {
            bHasLastPoint = m_pActiveObject->GetPoint(0, 0, &cLstInPt);
        }

        CDLine cPtX;

        if(fwKeys & MK_CONTROL)
        {
            bDoSnap = false;

            if(bHasLastPoint)
            {
                CDPoint cMainDir = {1.0, 0.0};
                CDLine cPtX;

                iCnt = m_pDrawObjects->GetSelectCount();
                if(iCnt == 1)
                {
                    pObj1 = m_pDrawObjects->GetSelected(0);
                    if(m_cLastDynPt.bIsSet)
                        pObj1->GetDistFromPt(m_cLastDynPt.cOrigin, m_cLastDynPt.cOrigin, true, &cPtX, NULL);
                    else pObj1->GetDistFromPt(cLstInPt.cPoint, cLstInPt.cPoint, true, &cPtX, NULL);
                    if(cPtX.bIsSet) cMainDir = cPtX.cDirection;
                }

                if((fwKeys & MK_SHIFT) && (iCnt == 1))
                {
                    cDir2.x = dx;
                    cDir2.y = dy;
                    cDir1 = pObj1->GetPointToDir(cLstInPt.cPoint, m_dSavedAngle, cDir2);
                    m_cLastDynPt.bIsSet = true;
                    m_cLastDynPt.cOrigin = cDir1;
                    m_cLastDrawPt = cDir2;
                    bDoSnap = true;
                }
                else
                {
                    m_cLastDynPt.bIsSet = false;
                    cDir1.x = dx - cLstInPt.cPoint.x;
                    cDir1.y = dy - cLstInPt.cPoint.y;
                    cDir2 = Rotate(cDir1, cMainDir, false);
                    dAng1 = atan2(cDir2.y, cDir2.x);
                    dAng1 *= 180.0/M_PI/m_cFSR.cAngUnit.dBaseToUnit;
                    double dAng2 = m_cFSR.dAngGrid*(Round((double)dAng1/m_cFSR.dAngGrid));
                    dAng2 *= M_PI*m_cFSR.cAngUnit.dBaseToUnit/180.0;

                    m_dSavedAngle = dAng2;

                    CDPoint cDir3 = {cos(dAng2), sin(dAng2)};
                    cSnapPt.cOrigin = Rotate(cDir2, cDir3, false);
                    cSnapPt.cOrigin.y = 0.0;
                    cDir1 = Rotate(cSnapPt.cOrigin, cDir3, true);
                    m_cLastDrawPt = cLstInPt.cPoint + Rotate(cDir1, cMainDir, true);
                }
            }
            else
            {
                dx = m_cFSR.dXGrid*(Round((double)dx/m_cFSR.dXGrid));
                dy = m_cFSR.dYGrid*(Round((double)dy/m_cFSR.dYGrid));
                m_cLastDrawPt.x = dx;
                m_cLastDrawPt.y = dy;
            }
            m_cLastSnapPt.x = m_cViewOrigin.x + (int)Round(m_cLastDrawPt.x*m_dUnitScale);
            m_cLastSnapPt.y = m_cViewOrigin.y + (int)Round(m_cLastDrawPt.y*m_dUnitScale);
        }
        else m_cLastDynPt.bIsSet = false;

        bool bRestrict = false;
        double dRestrictVal = m_dRestrictValue;

        if(bDoSnap)
        {
            m_cLastDrawPt.x = (m_cLastSnapPt.x - m_cViewOrigin.x)/m_dUnitScale;
            m_cLastDrawPt.y = (m_cLastSnapPt.y - m_cViewOrigin.y)/m_dUnitScale;
            dTol = (double)m_iSnapTolerance/m_dUnitScale;

            int iSnapType = 0;
            if(m_iToolMode == tolConflict) iSnapType = 1;
            if(m_pDrawObjects->GetSnapPoint(iSnapType, m_cLastDrawPt,
                dTol, &cSnapPt, m_pActiveObject) > 0)
            {
                if((fwKeys & MK_SHIFT) && (iCnt == 1))
                {
                    for(int i = 0; i < 4; i++)
                    {
                        cDir2 = cSnapPt.cOrigin;
                        cDir1 = pObj1->GetPointToDir(cLstInPt.cPoint, m_dSavedAngle, cDir2);
                        m_cLastDynPt.bIsSet = true;
                        m_cLastDynPt.cOrigin = cDir1;
                        m_cLastDrawPt = cDir2;
                        m_pDrawObjects->GetSnapPoint(iSnapType, m_cLastDrawPt, dTol, &cSnapPt,
                            m_pActiveObject);
                    }
                }

                m_cLastDrawPt = cSnapPt.cOrigin;
                m_cLastSnapPt.x = m_cViewOrigin.x + (int)Round(m_cLastDrawPt.x*m_dUnitScale);
                m_cLastSnapPt.y = m_cViewOrigin.y + (int)Round(m_cLastDrawPt.y*m_dUnitScale);
            }

            if(m_pActiveObject)
            {
                if((m_iDrawMode == modLine) && (iDynMode != 2))
                {
                    bRestrict = IS_ANGLE_VAL(m_iRestrictSet);
                    if(bRestrict)
                    {
                        if(m_iRestrictSet == 0) dRestrictVal /= m_cFSR.cAngUnit.dBaseToUnit;
                        if(m_iRestrictSet != 3) dRestrictVal *= M_PI/180.0;
                    }
                }
                else
                {
                    bRestrict = IS_LENGTH_VAL(m_iRestrictSet);
                    if(bRestrict)
                    {
                        if(m_iRestrictSet == 0)
                        {
                            if(m_bPaperUnits)
                                dRestrictVal *= m_cFSR.cPaperUnit.dBaseToUnit;
                            else dRestrictVal *= m_cFSR.cLenUnit.dBaseToUnit;
                        }
                        if(!m_bPaperUnits) dRestrictVal *= m_dDrawScale;
                    }
                }

                if(iDynMode == 3)
                {
                    iCnt = m_pDrawObjects->GetSelectCount();
                    if(iCnt == 2)
                    {
                        pObj1 = m_pDrawObjects->GetSelected(0);
                        pObj2 = m_pDrawObjects->GetSelected(1);
                        m_pActiveObject->BuildRound(pObj1, pObj2, m_cLastDrawPt, bRestrict,
                            dRestrictVal);
                    }
                }

                bRestrict = m_pActiveObject->GetRestrictPoint(m_cLastDrawPt,
                    iDynMode, bRestrict, dRestrictVal, &cSnapPt.cOrigin);
            }

            if(bRestrict)
            {
                m_cLastDrawPt = cSnapPt.cOrigin;
                m_cLastSnapPt.x = m_cViewOrigin.x + (int)Round(m_cLastDrawPt.x*m_dUnitScale);
                m_cLastSnapPt.y = m_cViewOrigin.y + (int)Round(m_cLastDrawPt.y*m_dUnitScale);
            }
        }

        MoveToEx(hdc, m_cLastSnapPt.x - 10, m_cLastSnapPt.y, NULL);
        LineTo(hdc, m_cLastSnapPt.x + 10, m_cLastSnapPt.y);
        MoveToEx(hdc, m_cLastSnapPt.x, m_cLastSnapPt.y - 10, NULL);
        LineTo(hdc, m_cLastSnapPt.x, m_cLastSnapPt.y + 10);

        cPtX.cOrigin = m_cLastDrawPt;
        if(iDynMode == 1)
        {
            cPtX.bIsSet = m_cLastDynPt.bIsSet;
            cPtX.cDirection = m_cLastDynPt.cOrigin;
        }
        else if(iDynMode == 2)
        {
            cPtX.cDirection.x = 0.0;
            if(fwKeys & MK_SHIFT) cPtX.cDirection.x = -1.0;
            if(bRestrict)
            {
                cPtX.cDirection.x = 1.0;
                cPtX.cDirection.y = dRestrictVal;
                m_dSavedDist = dRestrictVal;
            }
        }

        if(m_pActiveObject)
        {
            double dVal;
            if(!bRestrict)
            {
                if(m_pActiveObject->GetDynValue(m_cLastDrawPt, iDynMode, &dVal))
                {
                    m_dSavedDist = dVal;

                    if((m_iDrawMode == modLine) && (iDynMode != 2))
                    {
                        dVal *= m_cFSR.cAngUnit.dBaseToUnit*180.0/M_PI;
                        swprintf(m_wsStatus2Msg, L"%s %.2f %s", m_wsStatus2Base, dVal,
                            m_cFSR.cAngUnit.wsAbbrev);
                    }
                    else
                    {
                        if(m_bPaperUnits)
                        {
                            dVal /= m_cFSR.cPaperUnit.dBaseToUnit;
                            swprintf(m_wsStatus2Msg, L"%s %.2f %s", m_wsStatus2Base, dVal,
                                m_cFSR.cPaperUnit.wsAbbrev);
                        }
                        else
                        {
                            dVal /= m_dDrawScale;
                            dVal /= m_cFSR.cLenUnit.dBaseToUnit;
                            swprintf(m_wsStatus2Msg, L"%s %.2f %s", m_wsStatus2Base, dVal,
                                m_cFSR.cLenUnit.wsAbbrev);
                        }
                    }
                    SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Msg);
                }
            }
            else
            {
                dVal = m_dRestrictValue;
                if((m_iDrawMode == modLine) && (iDynMode != 2))
                {
                    swprintf(m_wsStatus2Msg, L"%s %.2f %s", m_wsStatus2Base, dVal,
                        m_cFSR.cAngUnit.wsAbbrev);
                }
                else
                {
                    if(m_bPaperUnits)
                    {
                        swprintf(m_wsStatus2Msg, L"%s %.2f %s", m_wsStatus2Base, dVal,
                            m_cFSR.cPaperUnit.wsAbbrev);
                    }
                    else
                    {
                        swprintf(m_wsStatus2Msg, L"%s %.2f %s", m_wsStatus2Base, dVal,
                            m_cFSR.cLenUnit.wsAbbrev);
                    }
                }
                SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Msg);
            }

            cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
            cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
            cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
            cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

            m_pActiveObject->BuildPrimitives(cPtX, iDynMode, &cdr, false, NULL);

            DrawObject(hwnd, hdc, m_pActiveObject, 1, -2);
        }
        else if(iDynMode == 4)
        {
            iCnt = m_pDrawObjects->GetSelectCount();
            if(iCnt == 1)
            {
//SendMessage(g_hStatus, SB_SETTEXT, 2, (LPARAM)L"Dobry 1");
                pObj1 = m_pDrawObjects->GetSelected(0);
                CDFileAttrs cFAttrs;
                FilePropsToData(&cFAttrs);

                cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
                cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
                cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
                cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

                pObj1->BuildPrimitives(cPtX, iDynMode, &cdr, false, &cFAttrs);
                DrawObject(hwnd, hdc, pObj1, 1, -1);
            }
        }

        SetROP2(hdc, iPrevROP);
        SelectObject(hdc, hPrevBr);
        SelectObject(hdc, hPrevPen);
        SelectClipRgn(hdc, NULL);
        ReleaseDC(hwnd, NULL);

        if(m_iToolMode == tolMeas)
        {
            if(m_cMeasPoint1.bIsSet && !m_cMeasPoint2.bIsSet)
            {
                CDPoint cDistPt = m_cLastDrawPt - m_cMeasPoint1.cOrigin;
                wchar_t *wsUnit;
                if(m_bPaperUnits)
                {
                    cDistPt /= m_cFSR.cPaperUnit.dBaseToUnit;
                    wsUnit = m_cFSR.cPaperUnit.wsAbbrev;
                }
                else
                {
                    cDistPt /= m_dDrawScale;
                    cDistPt /= m_cFSR.cLenUnit.dBaseToUnit;
                    wsUnit = m_cFSR.cLenUnit.wsAbbrev;
                }
                double dNorm = GetNorm(cDistPt);
                swprintf(m_wsStatus2Msg, L"dx: %.3f, dy: %.3f, dist: %.4f (%s)", fabs(cDistPt.x),
                    fabs(cDistPt.y), dNorm, wsUnit);
                SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Msg);
            }
        }
    }
    return 0;
}

LRESULT CMainWnd::WMMouseWheel(HWND hwnd, WORD fwKeys, int zDelta, int xPos, int yPos)
{
    if(m_iButton == 0)
    {
        double dRatio = exp((double)zDelta/600.0);

        POINT cPt = {xPos, yPos};
        MapWindowPoints(HWND_DESKTOP, hwnd, &cPt, 1);

        m_cViewOrigin.x = cPt.x + (m_cViewOrigin.x - cPt.x)*dRatio;
        m_cViewOrigin.y = cPt.y + (m_cViewOrigin.y - cPt.y)*dRatio;
        m_dUnitScale *= dRatio;

        RECT rc;
        GetClientRect(hwnd, &rc);
        rc.top += m_iToolBarHeight;
        rc.bottom -= m_iStatusHeight;
        InvalidateRect(hwnd, &rc, TRUE);
    }
    return 0;
}

LRESULT CMainWnd::WMLButtonDblClk(HWND hwnd, WPARAM fwKeys, int xPos, int yPos)
{
    //MessageBox(hwnd, L"WMLButtonDblClk", L"Debug", MB_OK);
    //if((m_iDrawMode > modSelect) || (m_iToolMode == tolCopyPar))
    if(m_iDrawMode > modSelect)
    {
        if(m_pActiveObject)
        {
            if(m_pActiveObject->HasEnoughPoints())
            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                rc.top += m_iToolBarHeight;
                rc.bottom -= m_iStatusHeight;

                PDPtrList pRegions = new CDPtrList();
                pRegions->SetDblVal(m_dUnitScale);

                m_pActiveObject->AddRegions(pRegions, -1);
                m_pDrawObjects->Add(m_pActiveObject);
                SetTitle(hwnd, false);
                m_pActiveObject = NULL;

                HRGN hRgn = GetUpdateRegion(pRegions);
                //InvalidateRect(hwnd, &rc, TRUE);
                if(hRgn)
                {
                    InvalidateRgn(hwnd, hRgn, TRUE);
                    DeleteObject(hRgn);
                }

                ClearPolygonList(pRegions);
                delete pRegions;
                StartNewObject(hwnd);
            }
        }
    }
    else if(m_iDrawMode + m_iToolMode < 1)
    {
        EditLineStyleCmd(hwnd, 0, 0);
    }
    return 0;
}

/*LRESULT CMainWnd::WMChar(HWND hwnd, wchar_t chCharCode, LPARAM lKeyData)
{
    float f;
    int iLen;
    if(chCharCode == VK_RETURN)
    {
        iLen = wcslen(m_wsStatus2Base);
        m_wsStatus2Msg[iLen] = 0;
        m_bRestrictSet = false;
    }
    else
    {
        int iLen = wcslen(m_wsStatus2Msg);
        if(iLen < 127)
        {
            m_wsStatus2Msg[iLen] = chCharCode;
            m_wsStatus2Msg[iLen + 1] = 0;
        }

        m_bRestrictSet = true;
        iLen = wcslen(m_wsStatus2Base);
        if(swscanf(&m_wsStatus2Msg[iLen], L"%f", &f) == 1) m_dRestrictValue = f;
        else m_bRestrictSet = false;
    }
    SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Msg);
    WMMouseMove(hwnd, 0, m_cLastMovePt.x, m_cLastMovePt.y);
    return 0;
}

LRESULT CMainWnd::WMKeyDown(HWND hwnd, int nVirtKey, LPARAM lKeyData)
{
    float f;
    int iLen;
    bool bRedraw = false;

    if((nVirtKey == VK_DECIMAL) || (nVirtKey == VK_OEM_PERIOD))
    {
        iLen = wcslen(m_wsStatus2Msg);
        if(iLen < 127)
        {
            m_wsStatus2Msg[iLen] = '.';
            m_wsStatus2Msg[iLen + 1] = 0;
        }
        m_bRestrictSet = true;
        iLen = wcslen(m_wsStatus2Base);
        if(swscanf(&m_wsStatus2Msg[iLen], L"%f", &f) == 1) m_dRestrictValue = f;
        else m_bRestrictSet = false;
        bRedraw = true;
    }
    else if(nVirtKey == VK_BACK)
    {
        iLen = wcslen(m_wsStatus2Msg);
        if(iLen > wcslen(m_wsStatus2Base)) m_wsStatus2Msg[iLen - 1] = 0;

        m_bRestrictSet = true;
        iLen = wcslen(m_wsStatus2Base);
        if(swscanf(&m_wsStatus2Msg[iLen], L"%f", &f) == 1) m_dRestrictValue = f;
        else m_bRestrictSet = false;
        bRedraw = true;
    }
    if(bRedraw)
    {
        SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Msg);
        WMMouseMove(hwnd, 0, m_cLastMovePt.x, m_cLastMovePt.y);
    }
    return 0;
}*/

void CMainWnd::StartNewObject(HWND hWnd)
{
    ShowWindow(m_hEdt1, SW_HIDE);
    ShowWindow(m_hEdt2, SW_HIDE);
    ShowWindow(m_hLab1, SW_HIDE);
    SendMessage(m_hStatus, SB_SETTEXT, 2, (LPARAM)L"");

    m_pActiveObject = NULL;
    m_iRestrictSet = -1;

    m_cMeasPoint1.bIsSet = false;
    m_cMeasPoint2.bIsSet = false;

    int iLines = m_pDrawObjects->GetNumOfSelectedLines();
    CDLine cLine1, cLine2;
    int iLinesFlag = 0;

    wchar_t sCap[32];
    wchar_t sMsg[128];
    PDObject pLineObj;

    m_wsStatus2Base[0] = 0;

    if(iLines > 0)
    {
        pLineObj = m_pDrawObjects->GetSelectedLine(0);
        cLine1 = pLineObj->GetLine();
        if(cLine1.bIsSet) iLinesFlag |= 1;
    }

    if(iLines > 1)
    {
        pLineObj = m_pDrawObjects->GetSelectedLine(1);
        cLine2 = pLineObj->GetLine();
        if(cLine2.bIsSet) iLinesFlag |= 2;
    }

    switch(m_iDrawMode)
    {
    case modLine:
        LoadString(m_hInstance, IDS_ANGLE, m_wsStatus2Base, 64);
        m_pActiveObject = new CDObject(dtLine, m_cFSR.dDefLineWidth);
        ShowWindow(m_hEdt1, SW_SHOW);
        SetFocus(m_hEdt1);
        break;
    case modCircle:
        LoadString(m_hInstance, IDS_RADIUS, m_wsStatus2Base, 64);
        m_pActiveObject = new CDObject(dtCircle, m_cFSR.dDefLineWidth);
        if(iLinesFlag & 1) m_pActiveObject->SetInputLine(0, cLine1);
        //if(iLinesFlag & 2) m_pActiveObject->SetInputLine(1, cLine2);
        ShowWindow(m_hEdt1, SW_SHOW);
        SetFocus(m_hEdt1);
        break;
    case modEllipse:
        m_pActiveObject = new CDObject(dtEllipse, m_cFSR.dDefLineWidth);
        if(iLines == 2)
        {
            if(iLinesFlag & 1) m_pActiveObject->SetInputLine(0, cLine1);
            if(iLinesFlag & 2) m_pActiveObject->SetInputLine(1, cLine2);
        }
        break;
    case modArcElps:
        if(iLines == 2)
        {
            m_pActiveObject = new CDObject(dtArcEllipse, m_cFSR.dDefLineWidth);
            if(iLinesFlag & 1) m_pActiveObject->SetInputLine(0, cLine1);
            if(iLinesFlag & 2) m_pActiveObject->SetInputLine(1, cLine2);
        }
        else
        {
            LoadString(m_hInstance, IDS_WARNING, sCap, 32);
            LoadString(m_hInstance, IDS_TWOLINESFORARCELLIPSE, sMsg, 128);
            MessageBox(hWnd, sMsg, sCap, MB_OK | MB_ICONWARNING);
        }
        break;
    case modHyperbola:
        if(iLines == 2)
        {
            m_pActiveObject = new CDObject(dtHyperbola, m_cFSR.dDefLineWidth);
            if(iLinesFlag & 1) m_pActiveObject->SetInputLine(0, cLine1);
            if(iLinesFlag & 2) m_pActiveObject->SetInputLine(1, cLine2);
        }
        else
        {
            LoadString(m_hInstance, IDS_WARNING, sCap, 32);
            LoadString(m_hInstance, IDS_TWOLINESFORHYPERBOLA, sMsg, 128);
            MessageBox(hWnd, sMsg, sCap, MB_OK | MB_ICONWARNING);
        }
        break;
    case modParabola:
        if(iLines == 1)
        {
            m_pActiveObject = new CDObject(dtParabola, m_cFSR.dDefLineWidth);
            if(iLinesFlag & 1) m_pActiveObject->SetInputLine(0, cLine1);
        }
        else
        {
            LoadString(m_hInstance, IDS_WARNING, sCap, 32);
            LoadString(m_hInstance, IDS_ONELINEFORPARABOLA, sMsg, 128);
            MessageBox(hWnd, sMsg, sCap, MB_OK | MB_ICONWARNING);
        }
        break;
    case modSpline:
        m_pActiveObject = new CDObject(dtSpline, m_cFSR.dDefLineWidth);
        break;
    case modEvolvent:
        iLines = m_pDrawObjects->GetNumOfSelectedCircles();
        if(iLines == 1)
        {
            pLineObj = m_pDrawObjects->GetSelectedCircle(0);
            cLine1 = pLineObj->GetCircle();
            if(cLine1.bIsSet) iLinesFlag |= 1;
            m_pActiveObject = new CDObject(dtEvolvent, m_cFSR.dDefLineWidth);
            if(iLinesFlag & 1) m_pActiveObject->SetInputLine(0, cLine1);
        }
        else
        {
            LoadString(m_hInstance, IDS_WARNING, sCap, 32);
            LoadString(m_hInstance, IDS_ONECIRCFOREVOLV, sMsg, 128);
            MessageBox(hWnd, sMsg, sCap, MB_OK | MB_ICONWARNING);
        }
        break;
    }

    if(m_iToolMode == tolRound)
    {
        int iCnt = m_pDrawObjects->GetSelectCount();
        if(iCnt == 2)
        {
            m_pActiveObject = new CDObject(dtCircle, m_cFSR.dDefLineWidth);
            LoadString(m_hInstance, IDS_RADIUS, m_wsStatus2Base, 64);
            ShowWindow(m_hEdt1, SW_SHOW);
            SetFocus(m_hEdt1);
        }
        else
        {
            LoadString(m_hInstance, IDS_WARNING, sCap, 32);
            LoadString(m_hInstance, IDS_TWOOBJSFORROUND, sMsg, 128);
            MessageBox(hWnd, sMsg, sCap, MB_OK | MB_ICONWARNING);
        }
    }

    wcscpy(m_wsStatus2Msg, m_wsStatus2Base);
    SendMessage(m_hStatus, SB_SETTEXT, 1, (LPARAM)m_wsStatus2Msg);
}

void CMainWnd::GetDeviceToUnitScale(HWND hWnd)
{
    HDC hdc = GetDC(0);
    int iLogPizelsX = GetDeviceCaps(hdc, LOGPIXELSX);
    //int iLogPixelsY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(0, NULL);

    m_iSelectTolerance = (int)1.0*iLogPizelsX/g_dMmToIn;
    m_iSnapTolerance = (int)2.0*iLogPizelsX/g_dMmToIn;

    m_dDeviceToUnitScale = (double)iLogPizelsX/g_dMmToIn;
}

LRESULT CMainWnd::Edit1Cmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(wNotifyCode == EN_CHANGE)
    {
        wchar_t wBuf[64];
        SendMessage(hwndCtl, WM_GETTEXT, 64, (LPARAM)wBuf);

        int iOldRest = m_iRestrictSet;

        char sBuf[64];
        WideCharToMultiByte(CP_UTF8, 0, wBuf, -1, sBuf, 64, NULL, NULL);
        m_iRestrictSet = ParseInputString(sBuf, m_pFileSetupDlg->GetUnitList(), &m_dRestrictValue);

        WMMouseMove(hwnd, 0, m_cLastMovePt.x, m_cLastMovePt.y);

        if((m_iToolMode == tolMove) && !m_cMeasPoint1.bIsSet && (iOldRest != m_iRestrictSet))
        {
            if(IS_LENGTH_VAL(m_iRestrictSet))
                LoadString(m_hInstance, IDS_SELLINETOMOVE, m_wsStatus2Msg, 128);
            else LoadString(m_hInstance, IDS_SELPOINTFROMMOVE, m_wsStatus2Msg, 128);
            SendMessage(m_hStatus, SB_SETTEXT, 2, (LPARAM)m_wsStatus2Msg);
        }
    }
    return 0;
}

LRESULT CMainWnd::Edit2Cmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    return 0;
}

void CMainWnd::SetTitle(HWND hWnd, bool bForce)
{
    bool bNewHasChanged = m_pDrawObjects->GetChanged();
    if((m_bHasChanged == bNewHasChanged) && !bForce) return;

    m_bHasChanged = bNewHasChanged;

    int iLen = wcslen(L"SteamCAD - ");
    LPWSTR wsFileName = NULL;
    if(m_wsFileName[0])
    {
        wsFileName = wcsrchr(m_wsFileName, '\\');
        if(wsFileName) wsFileName++;
        else wsFileName = m_wsFileName;
        iLen += wcslen(wsFileName);
    }
    else iLen += wcslen(L"new file");
    if(m_bHasChanged) iLen++;

    LPWSTR wsCap = (LPWSTR)malloc((iLen + 1)*sizeof(wchar_t));
    wcscpy(wsCap, L"SteamCAD - ");
    if(wsFileName) wcscat(wsCap, wsFileName);
    else wcscat(wsCap, L"new file");
    if(m_bHasChanged) wcscat(wsCap, L"*");

    SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)wsCap);
    free(wsCap);
}

LRESULT CMainWnd::ToolsBreakCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    rc.top += m_iToolBarHeight;
    rc.bottom -= m_iStatusHeight;

    CDRect cdr;
    cdr.cPt1.x = (rc.left - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt1.y = (rc.top - m_cViewOrigin.y)/m_dUnitScale;
    cdr.cPt2.x = (rc.right - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (rc.bottom - m_cViewOrigin.y)/m_dUnitScale;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);

    if(m_pDrawObjects->BreakSelObjects(&cdr, pRegions));
    {
        HRGN hRgn = GetUpdateRegion(pRegions);
        //InvalidateRect(hwnd, NULL, true);
        if(hRgn)
        {
            InvalidateRgn(hwnd, hRgn, TRUE);
            DeleteObject(hRgn);
        }
        SetTitle(hwnd, false);
    }

    ClearPolygonList(pRegions);
    delete pRegions;
    return 0;
}

LRESULT CMainWnd::ToolsScaleCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    CDFileAttrs cFA;
    m_pDrawObjects->GetFileAttrs(&cFA);

    CDScaleRec cSR;
    cSR.bScaleDraw = false;
    cSR.dScaleNom = cFA.dScaleNom;
    cSR.dScaleDenom = cFA.dScaleDenom;
    cSR.bScaleWidth = true;
    cSR.bScalePattern = true;
    cSR.bScaleArrows = true;
    cSR.bScaleLabels = true;
    cSR.bChangeDimUnits = false;

    char sMaskBuf[64];

    int iRes = m_pDrawObjects->GetUnitMask(1, sMaskBuf, m_pFileSetupDlg->GetUnitList());
    if(iRes < 0) wcscpy(cSR.wsLenMask, m_cFSR.wsLengthMask);
    else MultiByteToWideChar(CP_UTF8, 0, sMaskBuf, -1, cSR.wsLenMask, 64);

    iRes = m_pDrawObjects->GetUnitMask(2, sMaskBuf, m_pFileSetupDlg->GetUnitList());
    if(iRes < 0) wcscpy(cSR.wsAngMask, m_cFSR.wsAngleMask);
    else MultiByteToWideChar(CP_UTF8, 0, sMaskBuf, -1, cSR.wsAngMask, 64);

    if(m_pScaleDlg->ShowDialog(hwnd, &cSR) == IDOK)
    {
        bool bRedraw = false;
        if(cSR.bScaleDraw)
        {
            bRedraw = true;
            m_pDrawObjects->RescaleDrawing(cSR.dScaleNom, cSR.dScaleDenom,
                cSR.bScaleWidth, cSR.bScalePattern, cSR.bScaleArrows, cSR.bScaleLabels);
            m_pUndoObjects->ClearAll();
            DataToFileProps();
        }
        if(cSR.bChangeDimUnits)
        {
            bRedraw = true;
            WideCharToMultiByte(CP_UTF8, 0, cSR.wsLenMask, -1, sMaskBuf, 64, NULL, NULL);
            m_pDrawObjects->ChangeUnitMask(1, sMaskBuf, m_pFileSetupDlg->GetUnitList());
            WideCharToMultiByte(CP_UTF8, 0, cSR.wsAngMask, -1, sMaskBuf, 64, NULL, NULL);
            m_pDrawObjects->ChangeUnitMask(2, sMaskBuf, m_pFileSetupDlg->GetUnitList());
        }
        if(bRedraw) InvalidateRect(hwnd, NULL, TRUE);
    }
    return 0;
}

LRESULT CMainWnd::ToolsStatCmd(HWND hwnd, WORD wNotifyCode, HWND hwndCtl)
{
    int iStats[9];
    for(int i = 0; i < 9; i++) iStats[i] = 0;

    m_pDrawObjects->GetStatistics(iStats);

    CDStatRec cSR;
    cSR.iLines = iStats[1];
    cSR.iCircles = iStats[2];
    cSR.iEllips = iStats[3];
    cSR.iArcEllips = iStats[4];
    cSR.iHypers = iStats[5];
    cSR.iParabs = iStats[6];
    cSR.iSplines = iStats[7];
    cSR.iEvolvs = iStats[8];
    cSR.iDimens = iStats[0];

    m_pStatDlg->ShowDialog(hwnd, &cSR);
    return 0;
}
