#include "DStatDlg.hpp"
#include "SteamCAD.rh"

#include <stdio.h>
#include <math.h>

INT_PTR CALLBACK StatDlgProc(HWND hwndDlg, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    PDStatDlg psd = NULL;
    if(uMsg == WM_INITDIALOG) psd = (PDStatDlg)lParam;
    else psd = (PDStatDlg)GetWindowLongPtr(hwndDlg, DWLP_USER);

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

// CDStatDlg

CDStatDlg::CDStatDlg(HINSTANCE hInstance)
{
    m_hInstance = hInstance;
    m_iX = -100;
    m_iY = -100;
}

CDStatDlg::~CDStatDlg()
{
}

bool CDStatDlg::ShowDialog(HWND hWndParent, PDStatRec pSR)
{
    m_pSR = pSR;
    int iRes = DialogBoxParam(m_hInstance, L"STATDLG", hWndParent,
        StatDlgProc, (LPARAM)this);
    return (iRes == 1);
}

void CDStatDlg::SaveSettings(CXMLWritter* pWrit)
{
    IXMLDOMElement* pE1 = pWrit->CreateSection(L"StatDlg");
    pWrit->AddIntValue(pE1, L"Left", m_iX);
    pWrit->AddIntValue(pE1, L"Top", m_iY);
    pE1->Release();
    return;
}

void CDStatDlg::RestoreSettings(CXMLReader* pRdr)
{
    int i;
    IXMLDOMElement* pE1 = pRdr->OpenSection(L"StatDlg");
    if(pE1)
    {
        if(pRdr->GetIntValue(pE1, L"Left", &i)) m_iX = i;
        if(pRdr->GetIntValue(pE1, L"Top", &i)) m_iY = i;
        pE1->Release();
    }
    return;
}

INT_PTR CDStatDlg::WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam)
{
    SetWindowLongPtr(hWnd, DWLP_USER, lInitParam);

    int iTotal = m_pSR->iLines + m_pSR->iCircles + m_pSR->iEllips + m_pSR->iArcEllips +
        m_pSR->iHypers + m_pSR->iParabs + m_pSR->iSplines + m_pSR->iEvolvs; // + m_pSR->iDimens;

    wchar_t buf[64];
    swprintf(buf, L"%d", iTotal);

    HWND wnd = GetDlgItem(hWnd, STD_LBL_TOTALVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    swprintf(buf, L"%d", m_pSR->iLines);
    wnd = GetDlgItem(hWnd, STD_LBL_LINESVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    swprintf(buf, L"%d", m_pSR->iCircles);
    wnd = GetDlgItem(hWnd, STD_LBL_CIRCLESVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    swprintf(buf, L"%d", m_pSR->iEllips);
    wnd = GetDlgItem(hWnd, STD_LBL_ELIPSVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    swprintf(buf, L"%d", m_pSR->iArcEllips);
    wnd = GetDlgItem(hWnd, STD_LBL_ARCELPSVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    swprintf(buf, L"%d", m_pSR->iHypers);
    wnd = GetDlgItem(hWnd, STD_LBL_HYPERSVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    swprintf(buf, L"%d", m_pSR->iParabs);
    wnd = GetDlgItem(hWnd, STD_LBL_PARABSVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    swprintf(buf, L"%d", m_pSR->iSplines);
    wnd = GetDlgItem(hWnd, STD_LBL_SPLINESVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    swprintf(buf, L"%d", m_pSR->iEvolvs);
    wnd = GetDlgItem(hWnd, STD_LBL_EVOLVSVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    swprintf(buf, L"%d", m_pSR->iDimens);
    wnd = GetDlgItem(hWnd, STD_LBL_DIMENSVAL);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

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

    return TRUE;
}

INT_PTR CDStatDlg::WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    switch(wID)
    {
    case IDOK:
        EndDialog(hWnd, 1);
        return(TRUE);
    default:
        return(FALSE);
    }
}

INT_PTR CDStatDlg::WMMove(HWND hWnd, short int xPos, short int yPos)
{
    RECT R;
    GetWindowRect(hWnd, &R);
    m_iX = R.left;
    m_iY = R.top;
    return(FALSE);
}
