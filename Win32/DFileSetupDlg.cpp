#include "DFileSetupDlg.hpp"
#include "SteamCAD.rh"
#include "../Source/DDataTypes.hpp"

#include <stdio.h>
#include <math.h>

INT_PTR CALLBACK FileSetupDlgProc(HWND hwndDlg, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    PDFileSetupDlg pfsd = NULL;
    if(uMsg == WM_INITDIALOG) pfsd = (PDFileSetupDlg)lParam;
    else pfsd = (PDFileSetupDlg)GetWindowLongPtr(hwndDlg, DWLP_USER);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        return(pfsd->WMInitDialog(hwndDlg, (HWND)wParam, lParam));
    case WM_COMMAND:
        return(pfsd->WMCommand(hwndDlg, HIWORD(wParam), LOWORD(wParam),
            (HWND)lParam));
    case WM_MOVE:
        return(pfsd->WMMove(hwndDlg, (short int)LOWORD(lParam),
            (short int)HIWORD(lParam)));
    default:
        return(FALSE);
    }
}

double RoundFloat(double dx)
{
    double dRes = dx;
    double dRest = dx - (int)dx;
    if(dRest > 0.9999) dRes = (int)dx + 1;
    else if(dRest < 0.0001) dRes = (int)dx;
    return dRes;
}

void FormatFloatStr(double dx, LPWSTR wbuf)
{
    double dRest = fabs(dx - (int)dx);
    if((dRest > 0.0001) && (dRest < 0.9999)) swprintf(wbuf, L"%.3f", dx);
    else swprintf(wbuf, L"%d", (int)dx);
}


// CDFileSetupDlg

CDFileSetupDlg::CDFileSetupDlg(HINSTANCE hInstance, LPCWSTR wsAppPath)
{
    m_hInstance = hInstance;
    m_iX = -100;
    m_iY = -100;
    m_pPaperSizes = NULL;
    m_iPaperSizes = 0;
    m_bSettingUp = false;
    m_pUnits = new CDUnitList();
    BuildLenUnitsList(wsAppPath);
    BuildPaperSizeList(wsAppPath);
    m_hFnt = 0;
}

CDFileSetupDlg::~CDFileSetupDlg()
{
    delete m_pUnits;
    if(m_hFnt) DeleteObject(m_hFnt);
    if(m_pPaperSizes) free(m_pPaperSizes);
}

bool CDFileSetupDlg::ShowDialog(HWND hWndParent, PDFileSetupRec pFSR)
{
    m_pFSR = pFSR;
    memcpy(&m_cFSR, m_pFSR, sizeof(CDFileSetupRec));
    int iRes = DialogBoxParam(m_hInstance, L"FILESETUPDLG", hWndParent,
        FileSetupDlgProc, (LPARAM)this);
    return (iRes == 1);
}

void CDFileSetupDlg::SaveSettings(CXMLWritter* pWrit)
{
    IXMLDOMElement* pE1 = pWrit->CreateSection(L"FileSetupDlg");
    pWrit->AddIntValue(pE1, L"Left", m_iX);
    pWrit->AddIntValue(pE1, L"Top", m_iY);
    pE1->Release();
    return;
}

void CDFileSetupDlg::RestoreSettings(CXMLReader* pRdr)
{
    int i;
    IXMLDOMElement* pE1 = pRdr->OpenSection(L"FileSetupDlg");
    if(pE1)
    {
        if(pRdr->GetIntValue(pE1, L"Left", &i)) m_iX = i;
        if(pRdr->GetIntValue(pE1, L"Top", &i)) m_iY = i;
        pE1->Release();
    }
    return;
}

int CALLBACK GetFontNames(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme,
    ULONG FontType, LPARAM lParam)
{
    HWND hwnd = (HWND)lParam;
    if(lpelfe->lfFaceName[0] == '@') return 1;
    if(wcslen(lpelfe->lfFaceName) > 63) return 1;
    int iPos = SendMessage(hwnd, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)lpelfe->lfFaceName);
    if(iPos == CB_ERR) SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)lpelfe->lfFaceName);
    return 1;
}

void CDFileSetupDlg::SetupFontSample(HWND hWnd)
{
    LOGFONT lFnt;
    lFnt.lfHeight = -13;
    lFnt.lfWidth = 0;
    lFnt.lfEscapement = 0;
    lFnt.lfOrientation = 0;
    lFnt.lfWeight = FW_NORMAL;
    if(m_cFSR.bFontAttrs & 8) lFnt.lfWeight = FW_BOLD;
    lFnt.lfItalic = m_cFSR.bFontAttrs & 1;
    lFnt.lfUnderline = m_cFSR.bFontAttrs & 2;
    lFnt.lfStrikeOut = m_cFSR.bFontAttrs & 4;
    lFnt.lfCharSet = DEFAULT_CHARSET;
    lFnt.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lFnt.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lFnt.lfQuality = DEFAULT_QUALITY;
    lFnt.lfPitchAndFamily = DEFAULT_PITCH;
    wcscpy(lFnt.lfFaceName, m_cFSR.wsFontFace);

    if(m_hFnt) DeleteObject(m_hFnt);
    m_hFnt = CreateFontIndirect(&lFnt);
    SendDlgItemMessage(hWnd, FSD_LBL_FONTSMPL, WM_SETFONT, (LPARAM)m_hFnt, MAKELPARAM(TRUE, 0));
}

int CDFileSetupDlg::SetupUnitCB(HWND hCB, int iType, PDFileUnit pFileUnit)
{
    int iRes = -1;

    PDUnit pUnit;
    wchar_t wsName[64];
    int iLen = m_pUnits->GetCount(iType);
    for(int i = 0; i < iLen; i++)
    {
        pUnit = m_pUnits->GetUnit(iType, i);
        MultiByteToWideChar(CP_UTF8, 0, pUnit->sName, -1, wsName, 64);
        if(wcscmp(wsName, pFileUnit->wsName) == 0) iRes = i;
        SendMessage(hCB, CB_ADDSTRING, 0, (LPARAM)wsName);
        SendMessage(hCB, CB_SETITEMDATA, i, (LPARAM)pUnit);
    }

    CDUnit cUnit;
    if(iRes < 0)
    {
        WideCharToMultiByte(CP_UTF8, 0, pFileUnit->wsName, -1, cUnit.sName, 32, NULL, NULL);
        WideCharToMultiByte(CP_UTF8, 0, pFileUnit->wsAbbrev, -1, cUnit.sAbbrev, 8, NULL, NULL);
        cUnit.dBaseToUnit = pFileUnit->dBaseToUnit;
        WideCharToMultiByte(CP_UTF8, 0, pFileUnit->wsAbbrev2, -1, cUnit.sAbbrev2, 8, NULL, NULL);
        cUnit.iUnitType = iType;

        m_pUnits->AddUnit(cUnit);
        pUnit = m_pUnits->GetUnit(iType, iLen);

        SendMessage(hCB, CB_ADDSTRING, 0, (LPARAM)pFileUnit->wsName);
        SendMessage(hCB, CB_SETITEMDATA, iLen, (LPARAM)pUnit);
        SendMessage(hCB, CB_SETCURSEL, iLen, 0);

        iRes = iLen;
    }
    else if(iLen > 0) SendMessage(hCB, CB_SETCURSEL, iRes, 0);

    return iRes;
}

INT_PTR CDFileSetupDlg::WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam)
{
    SetWindowLongPtr(hWnd, DWLP_USER, lInitParam);

    m_bSettingUp = true;

    HWND wnd = GetDlgItem(hWnd, FSD_CB_PAPSIZE);
    int iIndex = -1;
    for(int i = 0; i < m_iPaperSizes; i++)
    {
        if(wcscmp(m_pPaperSizes[i].wsPaperSizeName, m_pFSR->cPaperSize.wsPaperSizeName) == 0)
            iIndex = i;
        SendMessage(wnd, CB_ADDSTRING, 0, (LPARAM)m_pPaperSizes[i].wsPaperSizeName);
        SendMessage(wnd, CB_SETITEMDATA, i, (LPARAM)&m_pPaperSizes[i]);
    }
    if(iIndex < 0)
    {
        SendMessage(wnd, CB_ADDSTRING, 0, (LPARAM)m_pFSR->cPaperSize.wsPaperSizeName);
        SendMessage(wnd, CB_SETITEMDATA, m_iPaperSizes, (LPARAM)&m_pFSR->cPaperSize);
        SendMessage(wnd, CB_SETCURSEL, m_iPaperSizes, 0);
    }
    else if(m_iPaperSizes > 0) SendMessage(wnd, CB_SETCURSEL, iIndex, 0);

    wnd = GetDlgItem(hWnd, FSD_CB_LENUNIT);
    SetupUnitCB(wnd, 1, &m_pFSR->cLenUnit);

    if(m_pFSR->bPortrait) wnd = GetDlgItem(hWnd, FSD_RB_PORTRAIT);
    else wnd = GetDlgItem(hWnd, FSD_RB_LANDSCAPE);
    SendMessage(wnd, BM_SETCHECK, BST_CHECKED, 0);

    wchar_t buf[32];
    FormatFloatStr(m_pFSR->dScaleNomin, buf);
    wnd = GetDlgItem(hWnd, FSD_EDT_SCALENOMIN);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    FormatFloatStr(m_pFSR->dScaleDenom, buf);
    wnd = GetDlgItem(hWnd, FSD_EDT_SCALEDENOM);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    wnd = GetDlgItem(hWnd, FSD_CB_ANGUNIT);
    m_iCurAngUnit = SetupUnitCB(wnd, 2, &m_pFSR->cAngUnit);

    wnd = GetDlgItem(hWnd, FSD_CB_PAPLENUNIT);
    m_iCurPaperUnit = SetupUnitCB(wnd, 1, &m_pFSR->cPaperUnit);

    wnd = GetDlgItem(hWnd, FSD_CB_GRAPHUNIT);
    m_iCurGraphUnit = SetupUnitCB(wnd, 1, &m_pFSR->cGraphUnit);

    wnd = GetDlgItem(hWnd, FSD_LBL_ANGGRIDUNIT);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pFSR->cAngUnit.wsAbbrev);

    FormatFloatStr(m_cFSR.dAngGrid*m_pFSR->cAngUnit.dBaseToUnit, buf);
    wnd = GetDlgItem(hWnd, FSD_EDT_ANGGRID);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    wnd = GetDlgItem(hWnd, FSD_LBL_XGRIDUNIT);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pFSR->cPaperUnit.wsAbbrev);
    wnd = GetDlgItem(hWnd, FSD_LBL_YGRIDUNIT);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pFSR->cPaperUnit.wsAbbrev);

    FormatFloatStr(m_cFSR.dXGrid/m_pFSR->cPaperUnit.dBaseToUnit, buf);
    wnd = GetDlgItem(hWnd, FSD_EDT_XGRID);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    FormatFloatStr(m_cFSR.dYGrid/m_pFSR->cPaperUnit.dBaseToUnit, buf);
    wnd = GetDlgItem(hWnd, FSD_EDT_YGRID);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    FormatFloatStr(m_cFSR.dDefLineWidth/m_pFSR->cGraphUnit.dBaseToUnit, buf);
    wnd = GetDlgItem(hWnd, FSD_EDT_DEFLINEWIDTH);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    wnd = GetDlgItem(hWnd, FSD_LBL_LINEWIDTHUNIT);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pFSR->cGraphUnit.wsAbbrev);

    wnd = GetDlgItem(hWnd, FSD_CB_ARROWSHAPE);
    wchar_t sBuf[64];
    for(int i = IDS_ARROWNONE; i < IDS_ARROWBACKSL + 1; i++)
    {
        LoadString(m_hInstance, i, sBuf, 64);
        SendMessage(wnd, CB_ADDSTRING, 0, (LPARAM)sBuf);
    }
    SendMessage(wnd, CB_SETCURSEL, m_pFSR->iArrowType, 0);

    FormatFloatStr(m_cFSR.dArrowLen/m_pFSR->cGraphUnit.dBaseToUnit, buf);
    wnd = GetDlgItem(hWnd, FSD_EDT_ARROWLEN);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    wnd = GetDlgItem(hWnd, FSD_LBL_ARROWLENUNIT);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pFSR->cGraphUnit.wsAbbrev);

    FormatFloatStr(m_cFSR.dArrowWidth/m_pFSR->cGraphUnit.dBaseToUnit, buf);
    wnd = GetDlgItem(hWnd, FSD_EDT_ARROWWIDTH);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    wnd = GetDlgItem(hWnd, FSD_LBL_ARROWWIDTHUNIT);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pFSR->cGraphUnit.wsAbbrev);

    wnd = GetDlgItem(hWnd, FSD_EDT_LENGTHMASK);
    SendMessage(wnd, EM_SETLIMITTEXT, 63, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pFSR->wsLengthMask);
    wnd = GetDlgItem(hWnd, FSD_EDT_ANGLEMASK);
    SendMessage(wnd, EM_SETLIMITTEXT, 63, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pFSR->wsAngleMask);

    wnd = GetDlgItem(hWnd, FSD_CB_FACE);
    HDC hdc = GetDC(hWnd);
    LOGFONT lFnt;
    lFnt.lfCharSet = DEFAULT_CHARSET;
    lFnt.lfFaceName[0] = 0;
    lFnt.lfPitchAndFamily = 0;
    EnumFontFamiliesEx(hdc, &lFnt, GetFontNames, (LPARAM)wnd, 0);

    int iPos = SendMessage(wnd, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)m_pFSR->wsFontFace);
    SendMessage(wnd, CB_SETCURSEL, (WPARAM)iPos, 0);

    WPARAM fCheck;
    wnd = GetDlgItem(hWnd, FSD_CHB_BOLD);
    if(m_pFSR->bFontAttrs & 8) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    wnd = GetDlgItem(hWnd, FSD_CHB_ITALIC);
    if(m_pFSR->bFontAttrs & 1) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    wnd = GetDlgItem(hWnd, FSD_CHB_UNDERLINE);
    if(m_pFSR->bFontAttrs & 2) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    FormatFloatStr(m_pFSR->dFontSize/m_pFSR->cGraphUnit.dBaseToUnit, buf);
    wnd = GetDlgItem(hWnd, FSD_EDT_FONTSIZE);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    wnd = GetDlgItem(hWnd, FSD_LBL_FONTSIZEUNIT);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pFSR->cGraphUnit.wsAbbrev);

    FormatFloatStr(m_pFSR->dBaseLine/m_pFSR->cGraphUnit.dBaseToUnit, buf);
    wnd = GetDlgItem(hWnd, FSD_EDT_FONTBASE);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    wnd = GetDlgItem(hWnd, FSD_LBL_FONTBASEUNIT);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pFSR->cGraphUnit.wsAbbrev);

    SetupFontSample(hWnd);

    if(m_iX > -100)
    {
        SetWindowPos(hWnd, NULL, m_iX, m_iY, 0, 0, SWP_NOSIZE);
    }
    else
    {
        RECT R;
        GetWindowRect(hWnd, &R);
        m_iX = R.left;
        m_iY = R.top;
    }

    m_bSettingUp = false;

    return(FALSE);
}

INT_PTR CDFileSetupDlg::WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    switch(wID)
    {
    case IDOK:
        return(OKBtnClick(hWnd));
    case IDCANCEL:
        EndDialog(hWnd, 0);
        return(TRUE);
    case FSD_CB_ANGUNIT:
        return(AngUnitCBChange(hWnd, wNotifyCode, hwndCtl));
    case FSD_CB_PAPLENUNIT:
        return(PaperUnitCBChange(hWnd, wNotifyCode, hwndCtl));
    case FSD_CB_GRAPHUNIT:
        return(GraphUnitCBChange(hWnd, wNotifyCode, hwndCtl));
    case FSD_EDT_ANGGRID:
    case FSD_EDT_XGRID:
    case FSD_EDT_YGRID:
    case FSD_EDT_DEFLINEWIDTH:
    case FSD_EDT_ARROWLEN:
    case FSD_EDT_ARROWWIDTH:
    case FSD_EDT_FONTSIZE:
    case FSD_EDT_FONTBASE:
        return(GridEditChange(hWnd, wNotifyCode, wID, hwndCtl));
    case FSD_CB_FACE:
        return(FaceCBChange(hWnd, wNotifyCode, hwndCtl));
    case FSD_CHB_BOLD:
        return(FontAttrChBChange(hWnd, wNotifyCode, hwndCtl, 8));
    case FSD_CHB_ITALIC:
        return(FontAttrChBChange(hWnd, wNotifyCode, hwndCtl, 1));
    case FSD_CHB_UNDERLINE:
        return(FontAttrChBChange(hWnd, wNotifyCode, hwndCtl, 2));
    default:
        return(FALSE);
    }
}

INT_PTR CDFileSetupDlg::WMMove(HWND hWnd, short int xPos, short int yPos)
{
    RECT R;
    GetWindowRect(hWnd, &R);
    m_iX = R.left;
    m_iY = R.top;
    return(FALSE);
}

INT_PTR CDFileSetupDlg::OKBtnClick(HWND hWnd)
{
    wchar_t buf[32], msg[128];
    float f;

    HWND wnd = GetDlgItem(hWnd, FSD_EDT_SCALEDENOM);
    SendMessage(wnd, WM_GETTEXT, 32, (LPARAM)buf);
    if(swscanf(buf, L"%f", &f) != 1)
    {
        LoadString(m_hInstance, IDS_EINVALIDNUMBER, msg, 128);
        LoadString(m_hInstance, IDS_ERRORBASE, buf, 32);
        MessageBox(hWnd, msg, buf, MB_OK | MB_ICONEXCLAMATION);
        SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
        SetFocus(wnd);
        return TRUE;
    }

    m_cFSR.dScaleDenom = f;

    wnd = GetDlgItem(hWnd, FSD_EDT_SCALENOMIN);
    SendMessage(wnd, WM_GETTEXT, 32, (LPARAM)buf);
    if(swscanf(buf, L"%f", &f) != 1)
    {
        LoadString(m_hInstance, IDS_EINVALIDNUMBER, msg, 128);
        LoadString(m_hInstance, IDS_ERRORBASE, buf, 32);
        MessageBox(hWnd, msg, buf, MB_OK | MB_ICONEXCLAMATION);
        SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
        SetFocus(wnd);
        return TRUE;
    }

    m_cFSR.dScaleNomin = f;

    wnd = GetDlgItem(hWnd, FSD_RB_PORTRAIT);
    m_cFSR.bPortrait = (SendMessage(wnd, BM_GETCHECK, 0, 0) == BST_CHECKED);

    wnd = GetDlgItem(hWnd, FSD_CB_LENUNIT);
    int idx = SendMessage(wnd, CB_GETCURSEL, 0, 0);
    PDUnit pLU = (PDUnit)SendMessage(wnd, CB_GETITEMDATA, idx, 0);
    SendMessage(wnd, CB_GETLBTEXT, idx, (LPARAM)buf);
    if(wcsicmp(buf, m_cFSR.cLenUnit.wsName) != 0)
    {
        wcscpy(m_cFSR.cLenUnit.wsName, buf);
        MultiByteToWideChar(CP_UTF8, 0, pLU->sAbbrev, -1, m_cFSR.cLenUnit.wsAbbrev, 8);
        m_pFSR->cLenUnit.dBaseToUnit = pLU->dBaseToUnit;
        MultiByteToWideChar(CP_UTF8, 0, pLU->sAbbrev2, -1, m_cFSR.cLenUnit.wsAbbrev2, 8);
    }

    wnd = GetDlgItem(hWnd, FSD_CB_ANGUNIT);
    SendMessage(wnd, CB_GETLBTEXT, m_iCurAngUnit, (LPARAM)buf);
    if(wcsicmp(buf, m_cFSR.cAngUnit.wsName) != 0)
    {
        wcscpy(m_cFSR.cAngUnit.wsName, buf);
        pLU = (PDUnit)SendMessage(wnd, CB_GETITEMDATA, m_iCurAngUnit, 0);
        MultiByteToWideChar(CP_UTF8, 0, pLU->sAbbrev, -1, m_cFSR.cAngUnit.wsAbbrev, 8);
        m_pFSR->cAngUnit.dBaseToUnit = pLU->dBaseToUnit;
        MultiByteToWideChar(CP_UTF8, 0, pLU->sAbbrev2, -1, m_cFSR.cAngUnit.wsAbbrev2, 8);
    }

    wnd = GetDlgItem(hWnd, FSD_CB_PAPLENUNIT);
    SendMessage(wnd, CB_GETLBTEXT, m_iCurPaperUnit, (LPARAM)buf);
    if(wcsicmp(buf, m_cFSR.cPaperUnit.wsName) != 0)
    {
        wcscpy(m_cFSR.cPaperUnit.wsName, buf);
        pLU = (PDUnit)SendMessage(wnd, CB_GETITEMDATA, m_iCurPaperUnit, 0);
        MultiByteToWideChar(CP_UTF8, 0, pLU->sAbbrev, -1, m_cFSR.cPaperUnit.wsAbbrev, 8);
        m_pFSR->cPaperUnit.dBaseToUnit = pLU->dBaseToUnit;
        MultiByteToWideChar(CP_UTF8, 0, pLU->sAbbrev2, -1, m_cFSR.cPaperUnit.wsAbbrev2, 8);
    }

    wnd = GetDlgItem(hWnd, FSD_CB_GRAPHUNIT);
    SendMessage(wnd, CB_GETLBTEXT, m_iCurGraphUnit, (LPARAM)buf);
    if(wcsicmp(buf, m_cFSR.cGraphUnit.wsName) != 0)
    {
        wcscpy(m_cFSR.cGraphUnit.wsName, buf);
        pLU = (PDUnit)SendMessage(wnd, CB_GETITEMDATA, m_iCurGraphUnit, 0);
        MultiByteToWideChar(CP_UTF8, 0, pLU->sAbbrev, -1, m_cFSR.cGraphUnit.wsAbbrev, 8);
        m_cFSR.cGraphUnit.dBaseToUnit = pLU->dBaseToUnit;
        MultiByteToWideChar(CP_UTF8, 0, pLU->sAbbrev2, -1, m_cFSR.cGraphUnit.wsAbbrev2, 8);
    }

    wnd = GetDlgItem(hWnd, FSD_CB_PAPSIZE);
    idx = SendMessage(wnd, CB_GETCURSEL, 0, 0);
    PDPaperSize pPS = (PDPaperSize)SendMessage(wnd, CB_GETITEMDATA, idx, 0);
    if(wcsicmp(m_cFSR.cPaperSize.wsPaperSizeName, pPS->wsPaperSizeName) != 0)
    {
        wcscpy(m_cFSR.cPaperSize.wsPaperSizeName, pPS->wsPaperSizeName);
        m_cFSR.cPaperSize.dPaperWidth = pPS->dPaperWidth;
        m_cFSR.cPaperSize.dPaperHeight = pPS->dPaperHeight;
    }

    char sBuf[64];
    wnd = GetDlgItem(hWnd, FSD_EDT_LENGTHMASK);
    SendMessage(wnd, WM_GETTEXT, 64, (LPARAM)msg);
    WideCharToMultiByte(CP_UTF8, 0, msg, -1, sBuf, 64, NULL, NULL);
    if(!ValidateMask(sBuf, m_pUnits))
    {
        LoadString(m_hInstance, IDS_EINVALIDMASK, msg, 128);
        LoadString(m_hInstance, IDS_ERRORBASE, buf, 32);
        MessageBox(hWnd, msg, buf, MB_OK | MB_ICONEXCLAMATION);
        SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
        SetFocus(wnd);
        return TRUE;
    }
    else wcscpy(m_cFSR.wsLengthMask, msg);

    wnd = GetDlgItem(hWnd, FSD_EDT_ANGLEMASK);
    SendMessage(wnd, WM_GETTEXT, 64, (LPARAM)msg);
    WideCharToMultiByte(CP_UTF8, 0, msg, -1, sBuf, 64, NULL, NULL);
    if(!ValidateMask(sBuf, m_pUnits))
    {
        LoadString(m_hInstance, IDS_EINVALIDMASK, msg, 128);
        LoadString(m_hInstance, IDS_ERRORBASE, buf, 32);
        MessageBox(hWnd, msg, buf, MB_OK | MB_ICONEXCLAMATION);
        SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
        SetFocus(wnd);
        return TRUE;
    }
    else wcscpy(m_cFSR.wsAngleMask, msg);

    wnd = GetDlgItem(hWnd, FSD_CB_ARROWSHAPE);
    m_cFSR.iArrowType = SendMessage(wnd, CB_GETCURSEL, 0, 0);

    memcpy(m_pFSR, &m_cFSR, sizeof(CDFileSetupRec));

    EndDialog(hWnd, 1);
    return(TRUE);
}

bool ReadLine(FILE *fp, LPWSTR wsBuf, int iBufSize)
{
    int iPos = 0;
    int cCur = fgetc(fp);
    bool bComment = false;

    if(cCur == '/')
    {
        wsBuf[iPos++] = cCur;
        cCur = fgetc(fp);
        if(cCur == '/')
        {
            wsBuf[iPos++] = cCur;
            wsBuf[iPos] = 0;
            bComment = true;
        }
    }

    bool bEOL = (cCur == 10) || (cCur == 13);
    bool bEOF = (cCur == EOF);

    if(bComment)
    {
        while(!(bEOL || bEOF))
        {
            cCur = fgetc(fp);
            bEOL = (cCur == 10) || (cCur == 13);
            bEOF = (cCur == EOF);
        }
        if(cCur == 13) cCur = fgetc(fp);
        return !bEOF;
    }

    while(!(bEOL || bEOF) && (iPos < iBufSize))
    {
        wsBuf[iPos++] = cCur;
        cCur = fgetc(fp);
        bEOL = (cCur == 10) || (cCur == 13);
        bEOF = (cCur == EOF);
    }
    wsBuf[iPos] = 0;

    while(!(bEOL || bEOF))
    {
        cCur = fgetc(fp);
        bEOL = (cCur == 10) || (cCur == 13);
        bEOF = (cCur == EOF);
    }

    if(cCur == 13) cCur = fgetc(fp);
    return (iPos > 0);
}

void CDFileSetupDlg::BuildPaperSizeList(LPCWSTR wsAppPath)
{
    LPWSTR sFile = (LPWSTR)malloc((wcslen(wsAppPath) + 16)*sizeof(wchar_t));
    wcscpy(sFile, wsAppPath);
    wcscat(sFile, L"DPapers.ini");

    FILE *fp = _wfopen(sFile, L"r");
    free(sFile);

    if(!fp) return;

    int iDataSize = 16;
    m_pPaperSizes = (PDPaperSize)malloc(iDataSize*sizeof(CDPaperSize));

    wchar_t sLine[64];
    wchar_t sBuf[32];
    LPWSTR sStart, sEnd;
    float f;
    int iLen;
    while(ReadLine(fp, sLine, 64))
    {
        sStart = sLine;
        sEnd = wcschr(sStart, ';');
        if(sEnd)
        {
            if(m_iPaperSizes >= iDataSize)
            {
                iDataSize += 16;
                m_pPaperSizes = (PDPaperSize)realloc(m_pPaperSizes, iDataSize*sizeof(CDPaperSize));
            }

            iLen = sEnd - sStart;
            if(iLen > 63) iLen = 63;
            wcsncpy(m_pPaperSizes[m_iPaperSizes].wsPaperSizeName, sStart, iLen);
            m_pPaperSizes[m_iPaperSizes].wsPaperSizeName[iLen] = 0;

            sStart = sEnd + 1;
            sEnd = wcschr(sStart, ';');

            PDUnit pUnit = NULL;

            if(sEnd)
            {
                iLen = sEnd - sStart;
                if(iLen > 31) iLen = 31;
                wcsncpy(sBuf, sStart, iLen);
                sBuf[iLen] = 0;

                char buf[32];
                WideCharToMultiByte(CP_UTF8, 0, sBuf, -1, buf, 32, NULL, NULL);
                pUnit = m_pUnits->FindUnit(buf);

                sStart = sEnd + 1;
                sEnd = wcschr(sStart, ';');
            }

            if(sEnd)
            {
                iLen = sEnd - sStart;
                if(iLen > 31) iLen = 31;
                wcsncpy(sBuf, sStart, iLen);
                sBuf[iLen] = 0;
                if(swscanf(sBuf, L"%f", &f) == 1)
                {
                    if(pUnit) m_pPaperSizes[m_iPaperSizes].dPaperWidth = f*pUnit->dBaseToUnit;
                    else m_pPaperSizes[m_iPaperSizes].dPaperWidth = f;
                }
                else m_pPaperSizes[m_iPaperSizes].dPaperWidth = 0.0;

                sStart = sEnd + 1;
            }
            else
            {
                m_pPaperSizes[m_iPaperSizes].dPaperWidth = 0.0;
                sStart = NULL;
            }

            if(sStart)
            {
                iLen = wcslen(sStart);
                if(iLen > 31) iLen = 31;
                wcsncpy(sBuf, sStart, iLen);
                sBuf[iLen] = 0;
                if(swscanf(sBuf, L"%f", &f) == 1)
                {
                    if(pUnit) m_pPaperSizes[m_iPaperSizes].dPaperHeight = f*pUnit->dBaseToUnit;
                    else m_pPaperSizes[m_iPaperSizes].dPaperHeight = f;
                }
                else m_pPaperSizes[m_iPaperSizes].dPaperHeight = 0.0;
            }
            else m_pPaperSizes[m_iPaperSizes].dPaperHeight = 0.0;

            if((m_pPaperSizes[m_iPaperSizes].dPaperWidth > 0.01) &&
               (m_pPaperSizes[m_iPaperSizes].dPaperHeight > 0.01)) m_iPaperSizes++;
        }
    }

    fclose(fp);
}

void CDFileSetupDlg::BuildLenUnitsList(LPCWSTR wsAppPath)
{
    LPWSTR sFile = (LPWSTR)malloc((wcslen(wsAppPath) + 16)*sizeof(wchar_t));
    wcscpy(sFile, wsAppPath);
    wcscat(sFile, L"DUnits.ini");

    FILE *fp = _wfopen(sFile, L"r");
    free(sFile);

    if(!fp) return;

    wchar_t wsLine[64];
    wchar_t wsBuf[32];
    LPWSTR wsStart, wsEnd;
    float f;
    int iLen, iwLen;
    CDUnit cUnit;
    while(ReadLine(fp, wsLine, 64))
    {
        if(!((wsLine[0] == '\\') && (wsLine[1] == '\\')))
        {
            cUnit.sName[0] = 0;
            cUnit.sAbbrev[0] = 0;
            cUnit.dBaseToUnit = 0.0;
            cUnit.sAbbrev2[0] = 0;
            cUnit.iUnitType = 0;

            wsStart = wsLine;
            wsEnd = wcschr(wsStart, ';');
            if(wsEnd)
            {
                iwLen = wsEnd - wsStart;
                if(iwLen > 31) iwLen = 31;
                iLen = WideCharToMultiByte(CP_UTF8, 0, wsStart, iwLen, cUnit.sName, 32, NULL, NULL);
                cUnit.sName[iLen] = 0;
                wsStart = wsEnd + 1;
                wsEnd = wcschr(wsStart, ';');
            }

            if(wsEnd)
            {
                iwLen = wsEnd - wsStart;
                if(iwLen > 7) iwLen = 7;
                iLen = WideCharToMultiByte(CP_UTF8, 0, wsStart, iwLen, cUnit.sAbbrev, 8, NULL, NULL);
                cUnit.sAbbrev[iLen] = 0;

                wsStart = wsEnd + 1;
                wsEnd = wcschr(wsStart, ';');
            }
            else wsStart = NULL;

            if(wsEnd)
            {
                iwLen = wsEnd - wsStart;
                if(iwLen > 31) iwLen = 31;
                wcsncpy(wsBuf, wsStart, iwLen);
                wsBuf[iwLen] = 0;
                if(swscanf(wsBuf, L"%f", &f) == 1) cUnit.dBaseToUnit = f;
                else cUnit.dBaseToUnit = 0.0;

                wsStart = wsEnd + 1;
                wsEnd = wcschr(wsStart, ';');
            }
            else wsStart = NULL;

            if(wsEnd)
            {
                if(wsStart[0] != '-')
                {
                    iwLen = wsEnd - wsStart;
                    if(iwLen > 7) iwLen = 7;
                    iLen = WideCharToMultiByte(CP_UTF8, 0, wsStart, iwLen, cUnit.sAbbrev2, 8, NULL, NULL);
                    cUnit.sAbbrev2[iLen] = 0;
                }

                wsStart = wsEnd + 1;
            }
            else wsStart = NULL;

            if(wsStart)
            {
                iwLen = wcslen(wsStart);
                if(iwLen > 0)
                {
                    if(wsStart[0] == 'l') cUnit.iUnitType = 1;
                    else if(wsStart[0] == 'a') cUnit.iUnitType = 2;
                }
            }

            if((cUnit.sName[0] > 0) && (cUnit.sAbbrev[0] > 0) &&
               (cUnit.dBaseToUnit > 0.00001))
            {
               m_pUnits->AddUnit(cUnit);
            }
        }
    }

    fclose(fp);
}

INT_PTR CDFileSetupDlg::AngUnitCBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(wNotifyCode == CBN_SELCHANGE)
    {
        int iIndex = SendMessage(hwndCtl, CB_GETCURSEL, 0, 0);
        if(m_iCurAngUnit != iIndex)
        {
            m_bSettingUp = true;

            wchar_t buf[32];
            PDUnit pUnit = (PDUnit)SendMessage(hwndCtl, CB_GETITEMDATA, iIndex, 0);
            MultiByteToWideChar(CP_UTF8, 0, pUnit->sAbbrev, -1, buf, 32);

            HWND wnd = GetDlgItem(hWnd, FSD_LBL_ANGGRIDUNIT);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            FormatFloatStr(m_cFSR.dAngGrid*pUnit->dBaseToUnit, buf);
            wnd = GetDlgItem(hWnd, FSD_EDT_ANGGRID);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            m_iCurAngUnit = iIndex;

            m_bSettingUp = false;
        }
    }
    return(TRUE);
}

INT_PTR CDFileSetupDlg::PaperUnitCBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(wNotifyCode == CBN_SELCHANGE)
    {
        int iIndex = SendMessage(hwndCtl, CB_GETCURSEL, 0, 0);
        if(m_iCurPaperUnit != iIndex)
        {
            m_bSettingUp = true;

            wchar_t buf[32];
            PDUnit pUnit = (PDUnit)SendMessage(hwndCtl, CB_GETITEMDATA, iIndex, 0);
            MultiByteToWideChar(CP_UTF8, 0, pUnit->sAbbrev, -1, buf, 32);

            HWND wnd = GetDlgItem(hWnd, FSD_LBL_XGRIDUNIT);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);
            wnd = GetDlgItem(hWnd, FSD_LBL_YGRIDUNIT);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            FormatFloatStr(m_cFSR.dXGrid/pUnit->dBaseToUnit, buf);
            wnd = GetDlgItem(hWnd, FSD_EDT_XGRID);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            FormatFloatStr(m_cFSR.dYGrid/pUnit->dBaseToUnit, buf);
            wnd = GetDlgItem(hWnd, FSD_EDT_YGRID);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            m_iCurPaperUnit = iIndex;

            m_bSettingUp = false;
        }
    }
    return(TRUE);
}

INT_PTR CDFileSetupDlg::GraphUnitCBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(wNotifyCode == CBN_SELCHANGE)
    {
        int iIndex = SendMessage(hwndCtl, CB_GETCURSEL, 0, 0);
        if(m_iCurGraphUnit != iIndex)
        {
            m_bSettingUp = true;

            wchar_t buf[32];
            PDUnit pUnit = (PDUnit)SendMessage(hwndCtl, CB_GETITEMDATA, iIndex, 0);
            MultiByteToWideChar(CP_UTF8, 0, pUnit->sAbbrev, -1, buf, 32);

            HWND wnd = GetDlgItem(hWnd, FSD_LBL_LINEWIDTHUNIT);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            wnd = GetDlgItem(hWnd, FSD_LBL_ARROWLENUNIT);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            wnd = GetDlgItem(hWnd, FSD_LBL_ARROWWIDTHUNIT);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            wnd = GetDlgItem(hWnd, FSD_LBL_FONTSIZEUNIT);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            wnd = GetDlgItem(hWnd, FSD_LBL_FONTBASEUNIT);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            FormatFloatStr(m_cFSR.dDefLineWidth/pUnit->dBaseToUnit, buf);
            wnd = GetDlgItem(hWnd, FSD_EDT_DEFLINEWIDTH);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            FormatFloatStr(m_cFSR.dArrowLen/pUnit->dBaseToUnit, buf);
            wnd = GetDlgItem(hWnd, FSD_EDT_ARROWLEN);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            FormatFloatStr(m_cFSR.dArrowWidth/pUnit->dBaseToUnit, buf);
            wnd = GetDlgItem(hWnd, FSD_EDT_ARROWWIDTH);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            FormatFloatStr(m_cFSR.dFontSize/pUnit->dBaseToUnit, buf);
            wnd = GetDlgItem(hWnd, FSD_EDT_FONTSIZE);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            FormatFloatStr(m_cFSR.dBaseLine/pUnit->dBaseToUnit, buf);
            wnd = GetDlgItem(hWnd, FSD_EDT_FONTBASE);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

            m_iCurGraphUnit = iIndex;

            m_bSettingUp = false;
        }
    }
    return(TRUE);
}

INT_PTR CDFileSetupDlg::GridEditChange(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    if(m_bSettingUp) return(TRUE);

    if(wNotifyCode == EN_CHANGE)
    {
        wchar_t buf[32];
        SendMessage(hwndCtl, WM_GETTEXT, 32, (LPARAM)buf);
        float f;
        if(swscanf(buf, L"%f", &f) == 1)
        {
            HWND wnd = GetDlgItem(hWnd, FSD_CB_ANGUNIT);
            PDUnit pAngUnit = (PDUnit)SendMessage(wnd, CB_GETITEMDATA, m_iCurAngUnit, 0);
            wnd = GetDlgItem(hWnd, FSD_CB_PAPLENUNIT);
            PDUnit pPaperUnit = (PDUnit)SendMessage(wnd, CB_GETITEMDATA, m_iCurPaperUnit, 0);
            wnd = GetDlgItem(hWnd, FSD_CB_GRAPHUNIT);
            PDUnit pGraphUnit = (PDUnit)SendMessage(wnd, CB_GETITEMDATA, m_iCurGraphUnit, 0);

            switch(wID)
            {
            case FSD_EDT_XGRID:
                m_cFSR.dXGrid = f*pPaperUnit->dBaseToUnit;
                break;
            case FSD_EDT_YGRID:
                m_cFSR.dYGrid = f*pPaperUnit->dBaseToUnit;
                break;
            case FSD_EDT_DEFLINEWIDTH:
                m_cFSR.dDefLineWidth = f*pGraphUnit->dBaseToUnit;
                break;
            case FSD_EDT_ARROWLEN:
                m_cFSR.dArrowLen = f*pGraphUnit->dBaseToUnit;
                break;
            case FSD_EDT_ARROWWIDTH:
                m_cFSR.dArrowWidth = f*pGraphUnit->dBaseToUnit;
                break;
            case FSD_EDT_FONTSIZE:
                m_cFSR.dFontSize = f*pGraphUnit->dBaseToUnit;
                break;
            case FSD_EDT_FONTBASE:
                m_cFSR.dBaseLine = f*pGraphUnit->dBaseToUnit;
                break;
            default:
                m_cFSR.dAngGrid = f/pAngUnit->dBaseToUnit;
            }
        }
    }
    return(TRUE);
}

INT_PTR CDFileSetupDlg::FaceCBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(m_bSettingUp) return(TRUE);

    if(wNotifyCode == CBN_SELCHANGE)
    {
        int iIndex = SendMessage(hwndCtl, CB_GETCURSEL, 0, 0);
        SendMessage(hwndCtl, CB_GETLBTEXT, iIndex, (LPARAM)m_cFSR.wsFontFace);
        SetupFontSample(hWnd);
    }
    return TRUE;
}

INT_PTR CDFileSetupDlg::FontAttrChBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl, int iMask)
{
    if(m_bSettingUp) return(TRUE);

    if(wNotifyCode == BN_CLICKED)
    {
        WPARAM fCheck = SendMessage(hwndCtl, BM_GETCHECK, 0, 0);
        if(fCheck == BST_CHECKED) m_cFSR.bFontAttrs |= iMask;
        else m_cFSR.bFontAttrs &= ~iMask;
        SetupFontSample(hWnd);
    }
    return TRUE;
}

PDUnitList CDFileSetupDlg::GetUnitList()
{
    return m_pUnits;
}

PDPaperSize CDFileSetupDlg::FindPaper(double dWidth, double dHeight)
{
    bool bFound = false;
    int i = 0;
    while(!bFound && (i < m_iPaperSizes))
    {
        bFound = ((fabs(m_pPaperSizes[i].dPaperWidth - dWidth) < 0.1) &&
            (fabs(m_pPaperSizes[i].dPaperHeight - dHeight) < 0.1)) ||
            ((fabs(m_pPaperSizes[i].dPaperWidth - dHeight) < 0.1) &&
            (fabs(m_pPaperSizes[i].dPaperHeight - dWidth) < 0.1));
        i++;
    }
    if(bFound) return &m_pPaperSizes[i - 1];
    return NULL;
}
