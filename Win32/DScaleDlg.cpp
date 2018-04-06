#include "DScaleDlg.hpp"
#include "SteamCAD.rh"
#include "DFileSetupDlg.hpp"
#include "../Source/DMath.hpp"

#include <stdio.h>
#include <math.h>

INT_PTR CALLBACK ScaleDlgProc(HWND hwndDlg, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    PDScaleDlg psd = NULL;
    if(uMsg == WM_INITDIALOG) psd = (PDScaleDlg)lParam;
    else psd = (PDScaleDlg)GetWindowLongPtr(hwndDlg, DWLP_USER);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        return(psd->WMInitDialog(hwndDlg, (HWND)wParam, lParam));
    case WM_COMMAND:
        return(psd->WMCommand(hwndDlg, HIWORD(wParam), LOWORD(wParam),
            (HWND)lParam));
    case WM_MOVE:
        return(psd->WMMove(hwndDlg, (short int)LOWORD(lParam),
            (short int)HIWORD(lParam)));
    default:
        return(FALSE);
    }
}

// CDScaleDlg

CDScaleDlg::CDScaleDlg(HINSTANCE hInstance)
{
    m_hInstance = hInstance;
    m_iX = -100;
    m_iY = -100;
    m_bSettingUp = false;
}

CDScaleDlg::~CDScaleDlg()
{
}

bool CDScaleDlg::ShowDialog(HWND hWndParent, PDScaleRec pSR)
{
    m_pSR = pSR;
    int iRes = DialogBoxParam(m_hInstance, L"SCALEDLG", hWndParent,
        ScaleDlgProc, (LPARAM)this);
    return (iRes == 1);
}

void CDScaleDlg::SaveSettings(CXMLWritter* pWrit)
{
    IXMLDOMElement* pE1 = pWrit->CreateSection(L"ScaleDlg");
    pWrit->AddIntValue(pE1, L"Left", m_iX);
    pWrit->AddIntValue(pE1, L"Top", m_iY);
    pE1->Release();
    return;
}

void CDScaleDlg::RestoreSettings(CXMLReader* pRdr)
{
    int i;
    IXMLDOMElement* pE1 = pRdr->OpenSection(L"ScaleDlg");
    if(pE1)
    {
        if(pRdr->GetIntValue(pE1, L"Left", &i)) m_iX = i;
        if(pRdr->GetIntValue(pE1, L"Top", &i)) m_iY = i;
        pE1->Release();
    }
    return;
}

INT_PTR CDScaleDlg::WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam)
{
    SetWindowLongPtr(hWnd, DWLP_USER, lInitParam);

    m_bSettingUp = true;

    WPARAM fCheck;

    HWND wnd = GetDlgItem(hWnd, SD_CHB_SCALEDRAW);
    if(m_pSR->bScaleDraw) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    wnd = GetDlgItem(hWnd, SD_CHB_SCLINEWIDTH);
    if(m_pSR->bScaleWidth) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    wnd = GetDlgItem(hWnd, SD_CHB_SCLINEPATS);
    if(m_pSR->bScalePattern) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    wnd = GetDlgItem(hWnd, SD_CHB_SCDIMARROWS);
    if(m_pSR->bScaleArrows) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    wnd = GetDlgItem(hWnd, SD_CHB_SCLABS);
    if(m_pSR->bScaleLabels) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    wnd = GetDlgItem(hWnd, SD_CHB_CHANGEUNITS);
    if(m_pSR->bChangeDimUnits) fCheck = BST_CHECKED;
    else fCheck = BST_UNCHECKED;
    SendMessage(wnd, BM_SETCHECK, fCheck, 0);

    wnd = GetDlgItem(hWnd, SD_EDT_LENMASK);
    SendMessage(wnd, EM_SETLIMITTEXT, 64, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pSR->wsLenMask);

    wnd = GetDlgItem(hWnd, SD_EDT_ANGMASK);
    SendMessage(wnd, EM_SETLIMITTEXT, 64, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pSR->wsAngMask);

    wchar_t buf1[64], buf2[64], buf[128];
    FormatFloatStr(m_pSR->dScaleNom, buf1);
    FormatFloatStr(m_pSR->dScaleDenom, buf2);

    swprintf(buf, L"%s : %s", buf1, buf2);
    wnd = GetDlgItem(hWnd, SD_LBL_OLDSCALEVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    wnd = GetDlgItem(hWnd, SD_EDT_NEWSCALENOM);
    SendMessage(wnd, EM_LIMITTEXT, 64, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf1);

    wnd = GetDlgItem(hWnd, SD_EDT_NEWSCALEDENOM);
    SendMessage(wnd, EM_LIMITTEXT, 64, 0);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf2);

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

    ScaleDrawChange(hWnd, 0, GetDlgItem(hWnd, SD_CHB_SCALEDRAW));
    ChangeUnitsChange(hWnd, 0, GetDlgItem(hWnd, SD_CHB_CHANGEUNITS));
    return TRUE;
}

INT_PTR CDScaleDlg::WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    switch(wID)
    {
    case IDOK:
        return(OKBtnClick(hWnd));
    case IDCANCEL:
        EndDialog(hWnd, 0);
        return(TRUE);
    case SD_CHB_SCALEDRAW:
        return ScaleDrawChange(hWnd, wNotifyCode, hwndCtl);
    case SD_CHB_CHANGEUNITS:
        return ChangeUnitsChange(hWnd, wNotifyCode, hwndCtl);
    default:
        return(FALSE);
    }
}

INT_PTR CDScaleDlg::WMMove(HWND hWnd, short int xPos, short int yPos)
{
    RECT R;
    GetWindowRect(hWnd, &R);
    m_iX = R.left;
    m_iY = R.top;
    return(FALSE);
}

INT_PTR CDScaleDlg::OKBtnClick(HWND hWnd)
{
    float f;
    wchar_t buf[64];
    wchar_t sMsg[256];

    HWND wnd = GetDlgItem(hWnd, SD_CHB_SCALEDRAW);
    WPARAM fCheck = (WPARAM)SendMessage(wnd, BM_GETCHECK, 0, 0);
    m_pSR->bScaleDraw = (fCheck == BST_CHECKED);

    if(m_pSR->bScaleDraw)
    {
        wnd = GetDlgItem(hWnd, SD_EDT_NEWSCALENOM);
        SendMessage(wnd, WM_GETTEXT, 64, (LPARAM)buf);
        if((swscanf(buf, L"%f", &f) != 1) || (f < g_dPrec))
        {
            LoadString(m_hInstance, IDS_ERRORBASE, buf, 64);
            LoadString(m_hInstance, IDS_EINVALIDNUMBER, sMsg, 256);
            MessageBox(hWnd, sMsg, buf, MB_ICONERROR | MB_OK);
            SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
            SetFocus(wnd);
            return 0;
        }

        m_pSR->dScaleNom = f;

        wnd = GetDlgItem(hWnd, SD_EDT_NEWSCALEDENOM);
        SendMessage(wnd, WM_GETTEXT, 64, (LPARAM)buf);
        if((swscanf(buf, L"%f", &f) != 1) || (f < g_dPrec))
        {
            LoadString(m_hInstance, IDS_ERRORBASE, buf, 64);
            LoadString(m_hInstance, IDS_EINVALIDNUMBER, sMsg, 256);
            MessageBox(hWnd, sMsg, buf, MB_ICONERROR | MB_OK);
            SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
            SetFocus(wnd);
            return 0;
        }

        m_pSR->dScaleDenom = f;
    }

    wnd = GetDlgItem(hWnd, SD_CHB_SCLINEWIDTH);
    fCheck = (WPARAM)SendMessage(wnd, BM_GETCHECK, 0, 0);
    m_pSR->bScaleWidth = (fCheck == BST_CHECKED);

    wnd = GetDlgItem(hWnd, SD_CHB_SCLINEPATS);
    fCheck = (WPARAM)SendMessage(wnd, BM_GETCHECK, 0, 0);
    m_pSR->bScalePattern = (fCheck == BST_CHECKED);

    wnd = GetDlgItem(hWnd, SD_CHB_SCDIMARROWS);
    fCheck = (WPARAM)SendMessage(wnd, BM_GETCHECK, 0, 0);
    m_pSR->bScaleArrows = (fCheck == BST_CHECKED);

    wnd = GetDlgItem(hWnd, SD_CHB_SCLABS);
    fCheck = (WPARAM)SendMessage(wnd, BM_GETCHECK, 0, 0);
    m_pSR->bScaleLabels = (fCheck == BST_CHECKED);

    wnd = GetDlgItem(hWnd, SD_CHB_CHANGEUNITS);
    fCheck = (WPARAM)SendMessage(wnd, BM_GETCHECK, 0, 0);
    m_pSR->bChangeDimUnits = (fCheck == BST_CHECKED);

    wnd = GetDlgItem(hWnd, SD_EDT_LENMASK);
    SendMessage(wnd, WM_GETTEXT, 64, (LPARAM)m_pSR->wsLenMask);

    wnd = GetDlgItem(hWnd, SD_EDT_ANGMASK);
    SendMessage(wnd, WM_GETTEXT, 64, (LPARAM)m_pSR->wsAngMask);

    EndDialog(hWnd, 1);
    return(TRUE);
}

INT_PTR CDScaleDlg::ScaleDrawChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(m_bSettingUp) return TRUE;

    WPARAM fCheck = (WPARAM)SendMessage(hwndCtl, BM_GETCHECK, 0, 0);
    BOOL bEnable = (fCheck == BST_CHECKED);
    EnableWindow(GetDlgItem(hWnd, SD_EDT_NEWSCALENOM), bEnable);
    EnableWindow(GetDlgItem(hWnd, SD_EDT_NEWSCALEDENOM), bEnable);
    EnableWindow(GetDlgItem(hWnd, SD_CHB_SCLINEWIDTH), bEnable);
    EnableWindow(GetDlgItem(hWnd, SD_CHB_SCLINEPATS), bEnable);
    EnableWindow(GetDlgItem(hWnd, SD_CHB_SCDIMARROWS), bEnable);
    EnableWindow(GetDlgItem(hWnd, SD_CHB_SCLABS), bEnable);
    return TRUE;
}

INT_PTR CDScaleDlg::ChangeUnitsChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(m_bSettingUp) return TRUE;

    WPARAM fCheck = (WPARAM)SendMessage(hwndCtl, BM_GETCHECK, 0, 0);
    BOOL bEnable = (fCheck == BST_CHECKED);
    EnableWindow(GetDlgItem(hWnd, SD_EDT_LENMASK), bEnable);
    EnableWindow(GetDlgItem(hWnd, SD_EDT_ANGMASK), bEnable);
    return TRUE;
}
