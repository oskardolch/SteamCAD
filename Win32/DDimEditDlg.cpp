#include "DDimEditDlg.hpp"
#include "SteamCAD.rh"

#include <stdio.h>

INT_PTR CALLBACK DimEditDlgProc(HWND hwndDlg, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    PDDimEditDlg pfsd = NULL;
    if(uMsg == WM_INITDIALOG) pfsd = (PDDimEditDlg)lParam;
    else pfsd = (PDDimEditDlg)GetWindowLongPtr(hwndDlg, DWLP_USER);

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


// CDDimEditDlg

CDDimEditDlg::CDDimEditDlg(HINSTANCE hInstance)
{
    m_hInstance = hInstance;
    m_iX = -100;
    m_iY = -100;
    m_bSettingUp = false;
    m_hFnt = 0;
    m_sCurText = NULL;
}

CDDimEditDlg::~CDDimEditDlg()
{
    if(m_sCurText) free(m_sCurText);
    if(m_hFnt) DeleteObject(m_hFnt);
}

bool CDDimEditDlg::ShowDialog(HWND hWndParent, PDDimension pDimen, PDUnitList pUnits,
    PDFileUnit pUnit)
{
    m_pDimen = pDimen;
    m_pUnits = pUnits;
    m_pUnit = pUnit;
    int iRes = DialogBoxParam(m_hInstance, L"DIMEDITDLG", hWndParent,
        DimEditDlgProc, (LPARAM)this);
    return (iRes == 1);
}

void CDDimEditDlg::SaveSettings(CXMLWritter* pWrit)
{
    IXMLDOMElement* pE1 = pWrit->CreateSection(L"DimEditDlg");
    pWrit->AddIntValue(pE1, L"Left", m_iX);
    pWrit->AddIntValue(pE1, L"Top", m_iY);
    pE1->Release();
    return;
}

void CDDimEditDlg::RestoreSettings(CXMLReader* pRdr)
{
    int i;
    IXMLDOMElement* pE1 = pRdr->OpenSection(L"DimEditDlg");
    if(pE1)
    {
        if(pRdr->GetIntValue(pE1, L"Left", &i)) m_iX = i;
        if(pRdr->GetIntValue(pE1, L"Top", &i)) m_iY = i;
        pE1->Release();
    }
    return;
}

void CDDimEditDlg::SetupFontSample(HWND hWnd)
{
    LOGFONT lFnt;
    lFnt.lfHeight = -13;
    lFnt.lfWidth = 0;
    lFnt.lfEscapement = 0;
    lFnt.lfOrientation = 0;
    lFnt.lfWeight = FW_NORMAL;
    if(m_bCurFntAttrs & 8) lFnt.lfWeight = FW_BOLD;
    lFnt.lfItalic = m_bCurFntAttrs & 1;
    lFnt.lfUnderline = m_bCurFntAttrs & 2;
    lFnt.lfStrikeOut = m_bCurFntAttrs & 4;
    lFnt.lfCharSet = DEFAULT_CHARSET;
    lFnt.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lFnt.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lFnt.lfQuality = DEFAULT_QUALITY;
    lFnt.lfPitchAndFamily = DEFAULT_PITCH;
    wcscpy(lFnt.lfFaceName, m_sCurFntName);

    if(m_hFnt) DeleteObject(m_hFnt);
    m_hFnt = CreateFontIndirect(&lFnt);
    SendDlgItemMessage(hWnd, DED_LBL_FONTSMPL, WM_SETFONT, (LPARAM)m_hFnt, MAKELPARAM(TRUE, 0));
}

INT_PTR CDDimEditDlg::WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam)
{
    SetWindowLongPtr(hWnd, DWLP_USER, lInitParam);

    m_bSettingUp = true;

    HWND wnd = GetDlgItem(hWnd, DED_CB_LEFTSHAPE);
    wchar_t sBuf[64];
    for(int i = IDS_ARROWNONE; i < IDS_ARROWBACKSL + 1; i++)
    {
        LoadString(m_hInstance, i, sBuf, 64);
        SendMessage(wnd, CB_ADDSTRING, 0, (LPARAM)sBuf);
    }
    SendMessage(wnd, CB_SETCURSEL, m_pDimen->iArrowType1, 0);

    wnd = GetDlgItem(hWnd, DED_CB_RIGHTSHAPE);
    for(int i = IDS_ARROWNONE; i < IDS_ARROWBACKSL + 1; i++)
    {
        LoadString(m_hInstance, i, sBuf, 64);
        SendMessage(wnd, CB_ADDSTRING, 0, (LPARAM)sBuf);
    }
    SendMessage(wnd, CB_SETCURSEL, m_pDimen->iArrowType2, 0);

    m_dCurLeftLen = m_pDimen->cArrowDim1.x/m_pUnit->dBaseToUnit;
    m_dCurLeftWidth = m_pDimen->cArrowDim1.y/m_pUnit->dBaseToUnit;
    m_dCurRightLen = m_pDimen->cArrowDim2.x/m_pUnit->dBaseToUnit;
    m_dCurRightWidth = m_pDimen->cArrowDim2.y/m_pUnit->dBaseToUnit;

    FormatFloatStr(m_dCurLeftLen, sBuf);
    wnd = GetDlgItem(hWnd, DED_EDT_LEFTLEN);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)sBuf);

    FormatFloatStr(m_dCurLeftWidth, sBuf);
    wnd = GetDlgItem(hWnd, DED_EDT_LEFTWIDTH);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)sBuf);

    FormatFloatStr(m_dCurRightLen, sBuf);
    wnd = GetDlgItem(hWnd, DED_EDT_RIGHTLEN);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)sBuf);

    FormatFloatStr(m_dCurRightWidth, sBuf);
    wnd = GetDlgItem(hWnd, DED_EDT_RIGHTWIDTH);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)sBuf);

    m_dCurFontSize = m_pDimen->dFontSize/m_pUnit->dBaseToUnit;
    m_bCurFntAttrs = m_pDimen->bFontAttrs;
    MultiByteToWideChar(CP_UTF8, 0, m_pDimen->psFontFace, -1, sBuf, 64);
    wcscpy(m_sCurFntName, sBuf);

    wnd = GetDlgItem(hWnd, DED_CB_FACE);
    HDC hdc = GetDC(hWnd);
    LOGFONT lFnt;
    lFnt.lfCharSet = DEFAULT_CHARSET;
    lFnt.lfFaceName[0] = 0;
    lFnt.lfPitchAndFamily = 0;
    EnumFontFamiliesEx(hdc, &lFnt, GetFontNames, (LPARAM)wnd, 0);

    int iPos = SendMessage(wnd, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)m_sCurFntName);
    SendMessage(wnd, CB_SETCURSEL, (WPARAM)iPos, 0);

    WPARAM fCheck;
    wnd = GetDlgItem(hWnd, DED_CHB_BOLD);
    if(m_bCurFntAttrs & 8) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    wnd = GetDlgItem(hWnd, DED_CHB_ITALIC);
    if(m_bCurFntAttrs & 1) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    wnd = GetDlgItem(hWnd, DED_CHB_UNDERLINE);
    if(m_bCurFntAttrs & 2) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    FormatFloatStr(m_dCurFontSize, sBuf);
    wnd = GetDlgItem(hWnd, DED_EDT_FONTSIZE);
    SendMessage(wnd, EM_SETLIMITTEXT, 32, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)sBuf);

    SendDlgItemMessage(hWnd, DED_LBL_LEFTLENUNIT, WM_SETTEXT, 0, (LPARAM)m_pUnit->wsAbbrev);
    SendDlgItemMessage(hWnd, DED_LBL_LEFTWIDTHUNIT, WM_SETTEXT, 0, (LPARAM)m_pUnit->wsAbbrev);
    SendDlgItemMessage(hWnd, DED_LBL_RIGHTLENUNIT, WM_SETTEXT, 0, (LPARAM)m_pUnit->wsAbbrev);
    SendDlgItemMessage(hWnd, DED_LBL_RIGHTWIDTHUNIT, WM_SETTEXT, 0, (LPARAM)m_pUnit->wsAbbrev);
    SendDlgItemMessage(hWnd, DED_LBL_FONTSIZEUNIT, WM_SETTEXT, 0, (LPARAM)m_pUnit->wsAbbrev);

    SetupFontSample(hWnd);

    if(m_pDimen->psLab)
    {
        wnd = GetDlgItem(hWnd, DED_EDT_TEXT);
        int iLen = MultiByteToWideChar(CP_UTF8, 0, m_pDimen->psLab, -1, NULL, 0);
        m_sCurText = (LPWSTR)malloc((iLen + 1)*sizeof(wchar_t));
        MultiByteToWideChar(CP_UTF8, 0, m_pDimen->psLab, -1, m_sCurText, iLen + 1);
        SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_sCurText);
    }

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

INT_PTR CDDimEditDlg::WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    switch(wID)
    {
    case IDOK:
        return(OKBtnClick(hWnd));
    case IDCANCEL:
        EndDialog(hWnd, 0);
        return(TRUE);
    case DED_EDT_LEFTLEN:
    case DED_EDT_LEFTWIDTH:
    case DED_EDT_RIGHTLEN:
    case DED_EDT_RIGHTWIDTH:
    case DED_EDT_FONTSIZE:
        return(GridEditChange(hWnd, wNotifyCode, wID, hwndCtl));
    case DED_CB_FACE:
        return(FaceCBChange(hWnd, wNotifyCode, hwndCtl));
    case DED_CHB_BOLD:
        return(FontAttrChBChange(hWnd, wNotifyCode, hwndCtl, 8));
    case DED_CHB_ITALIC:
        return(FontAttrChBChange(hWnd, wNotifyCode, hwndCtl, 1));
    case DED_CHB_UNDERLINE:
        return(FontAttrChBChange(hWnd, wNotifyCode, hwndCtl, 2));
    case DED_EDT_TEXT:
        return(TextEditChange(hWnd, wNotifyCode, hwndCtl));
    default:
        return(FALSE);
    }
}

INT_PTR CDDimEditDlg::WMMove(HWND hWnd, short int xPos, short int yPos)
{
    RECT R;
    GetWindowRect(hWnd, &R);
    m_iX = R.left;
    m_iY = R.top;
    return(FALSE);
}

INT_PTR CDDimEditDlg::OKBtnClick(HWND hWnd)
{
    wchar_t buf[32], msg[128];
    float f;
    HWND wnd;

    if(m_sCurText)
    {
        int iLen = WideCharToMultiByte(CP_UTF8, 0, m_sCurText, -1, NULL, 0, NULL, NULL);
        LPSTR sNewMask = (char*)malloc((iLen + 1)*sizeof(char));
        WideCharToMultiByte(CP_UTF8, 0, m_sCurText, -1, sNewMask, iLen + 1, NULL, NULL);

        int iValRes = ValidateMask(sNewMask, m_pUnits);
        if(iValRes != 0)
        {
            free(sNewMask);
            switch(iValRes)
            {
            case 1:
                LoadString(m_hInstance, IDS_EPRECNEGATIVE, msg, 128);
                break;
            case 2:
                LoadString(m_hInstance, IDS_EPRECTOOLARGE, msg, 128);
                break;
            case 3:
                LoadString(m_hInstance, IDS_EPRECCANNOTPARSE, msg, 128);
                break;
            default:
                LoadString(m_hInstance, IDS_EINVALIDMASK, msg, 128);
            }

            LoadString(m_hInstance, IDS_ERRORBASE, buf, 32);
            MessageBox(hWnd, msg, buf, MB_OK | MB_ICONEXCLAMATION);
            wnd = GetDlgItem(hWnd, DED_EDT_TEXT);
            SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
            SetFocus(wnd);
            return TRUE;
        }
        else
        {
            if(m_pDimen->psLab) free(m_pDimen->psLab);
            m_pDimen->psLab = NULL;
            m_pDimen->psLab = sNewMask;
        }
    }
    else
    {
        if(m_pDimen->psLab) free(m_pDimen->psLab);
        m_pDimen->psLab = NULL;
    }

    wnd = GetDlgItem(hWnd, DED_CB_LEFTSHAPE);
    m_pDimen->iArrowType1 = SendMessage(wnd, CB_GETCURSEL, 0, 0);
    m_pDimen->cArrowDim1.x = m_dCurLeftLen*m_pUnit->dBaseToUnit;
    m_pDimen->cArrowDim1.y = m_dCurLeftWidth*m_pUnit->dBaseToUnit;

    wnd = GetDlgItem(hWnd, DED_CB_RIGHTSHAPE);
    m_pDimen->iArrowType2 = SendMessage(wnd, CB_GETCURSEL, 0, 0);
    m_pDimen->cArrowDim2.x = m_dCurRightLen*m_pUnit->dBaseToUnit;
    m_pDimen->cArrowDim2.y = m_dCurRightWidth*m_pUnit->dBaseToUnit;

    m_pDimen->dFontSize = m_dCurFontSize*m_pUnit->dBaseToUnit;
    m_pDimen->bFontAttrs = m_bCurFntAttrs;
    WideCharToMultiByte(CP_UTF8, 0, m_sCurFntName, -1, m_pDimen->psFontFace, 64, NULL, NULL);

    EndDialog(hWnd, 1);
    return(TRUE);
}

INT_PTR CDDimEditDlg::GridEditChange(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    if(m_bSettingUp) return(TRUE);

    if(wNotifyCode == EN_CHANGE)
    {
        wchar_t buf[32];
        SendMessage(hwndCtl, WM_GETTEXT, 32, (LPARAM)buf);
        float f;
        if(swscanf(buf, L"%f", &f) == 1)
        {
            switch(wID)
            {
            case DED_EDT_LEFTLEN:
                m_dCurLeftLen = f;
                break;
            case DED_EDT_LEFTWIDTH:
                m_dCurLeftWidth = f;
                break;
            case DED_EDT_RIGHTLEN:
                m_dCurRightLen = f;
                break;
            case DED_EDT_RIGHTWIDTH:
                m_dCurRightWidth = f;
                break;
            case DED_EDT_FONTSIZE:
                m_dCurFontSize = f;
                break;
            }
        }
    }
    return(TRUE);
}

INT_PTR CDDimEditDlg::FaceCBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(m_bSettingUp) return(TRUE);

    if(wNotifyCode == CBN_SELCHANGE)
    {
        int iIndex = SendMessage(hwndCtl, CB_GETCURSEL, 0, 0);
        SendMessage(hwndCtl, CB_GETLBTEXT, iIndex, (LPARAM)m_sCurFntName);
        SetupFontSample(hWnd);
    }
    return TRUE;
}

INT_PTR CDDimEditDlg::FontAttrChBChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl, int iMask)
{
    if(m_bSettingUp) return(TRUE);

    if(wNotifyCode == BN_CLICKED)
    {
        WPARAM fCheck = SendMessage(hwndCtl, BM_GETCHECK, 0, 0);
        if(fCheck == BST_CHECKED) m_bCurFntAttrs |= iMask;
        else m_bCurFntAttrs &= ~iMask;
        SetupFontSample(hWnd);
    }
    return TRUE;
}

INT_PTR CDDimEditDlg::TextEditChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(m_bSettingUp) return(TRUE);

    if(wNotifyCode == EN_CHANGE)
    {
        if(m_sCurText) free(m_sCurText);
        m_sCurText = NULL;

        int iLen = SendMessage(hwndCtl, WM_GETTEXTLENGTH, 0, 0);
        if(iLen > 0)
        {
            m_sCurText = (LPWSTR)malloc((iLen + 1)*sizeof(wchar_t));
            SendMessage(hwndCtl, WM_GETTEXT, iLen + 1, (LPARAM)m_sCurText);
        }
    }
    return(TRUE);
}
