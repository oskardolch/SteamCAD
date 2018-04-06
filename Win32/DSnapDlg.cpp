#include "DSnapDlg.hpp"
#include "SteamCAD.rh"

INT_PTR CALLBACK SnapDlgProc(HWND hwndDlg, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    PDSnapDlg psd = NULL;
    if(uMsg == WM_INITDIALOG) psd = (PDSnapDlg)lParam;
    else psd = (PDSnapDlg)GetWindowLongPtr(hwndDlg, DWLP_USER);

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

// CDSnapDlg

CDSnapDlg::CDSnapDlg(HINSTANCE hInstance)
{
    m_hInstance = hInstance;
    m_iX = -100;
    m_iY = -100;
}

CDSnapDlg::~CDSnapDlg()
{
}

bool CDSnapDlg::ShowDialog(HWND hWndParent, bool *pbEnableSnap)
{
    m_pbEnableSnap = pbEnableSnap;
    int iRes = DialogBoxParam(m_hInstance, L"SNAPDLG", hWndParent,
        SnapDlgProc, (LPARAM)this);
    return (iRes == 1);
}

void CDSnapDlg::SaveSettings(CXMLWritter* pWrit)
{
    IXMLDOMElement* pE1 = pWrit->CreateSection(L"SnapDlg");
    pWrit->AddIntValue(pE1, L"Left", m_iX);
    pWrit->AddIntValue(pE1, L"Top", m_iY);
    pE1->Release();
    return;
}

void CDSnapDlg::RestoreSettings(CXMLReader* pRdr)
{
    int i;
    IXMLDOMElement* pE1 = pRdr->OpenSection(L"SnapDlg");
    if(pE1)
    {
        if(pRdr->GetIntValue(pE1, L"Left", &i)) m_iX = i;
        if(pRdr->GetIntValue(pE1, L"Top", &i)) m_iY = i;
        pE1->Release();
    }
    return;
}

INT_PTR CDSnapDlg::WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam)
{
    SetWindowLongPtr(hWnd, DWLP_USER, lInitParam);

    if(*m_pbEnableSnap) SendDlgItemMessage(hWnd, TSD_RB_DISABLESNAP, BM_SETCHECK, BST_CHECKED, 0);
    else SendDlgItemMessage(hWnd, TSD_RB_ENABLESNAP, BM_SETCHECK, BST_CHECKED, 0);

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

INT_PTR CDSnapDlg::WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    switch(wID)
    {
    case IDOK:
        return(OKBtnClick(hWnd));
    case IDCANCEL:
        EndDialog(hWnd, 0);
        return(TRUE);
    default:
        return(FALSE);
    }
}

INT_PTR CDSnapDlg::WMMove(HWND hWnd, short int xPos, short int yPos)
{
    RECT R;
    GetWindowRect(hWnd, &R);
    m_iX = R.left;
    m_iY = R.top;
    return(FALSE);
}

INT_PTR CDSnapDlg::OKBtnClick(HWND hWnd)
{
    WPARAM fCheck = (WPARAM)SendDlgItemMessage(hWnd, TSD_RB_ENABLESNAP, BM_GETCHECK, 0, 0);
    *m_pbEnableSnap = (fCheck == BST_CHECKED);
    EndDialog(hWnd, 1);
    return TRUE;
}
