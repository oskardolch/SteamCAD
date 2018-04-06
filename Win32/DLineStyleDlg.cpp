#include "DLineStyleDlg.hpp"
#include "SteamCAD.rh"

#include <stdio.h>
#include <math.h>

INT_PTR CALLBACK LineStyleDlgProc(HWND hwndDlg, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    PDLineStyleDlg plsd = NULL;
    if(uMsg == WM_INITDIALOG) plsd = (PDLineStyleDlg)lParam;
    else plsd = (PDLineStyleDlg)GetWindowLongPtr(hwndDlg, DWLP_USER);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        return(plsd->WMInitDialog(hwndDlg, (HWND)wParam, lParam));
    case WM_COMMAND:
        return(plsd->WMCommand(hwndDlg, HIWORD(wParam), LOWORD(wParam),
            (HWND)lParam));
    case WM_MOVE:
        return(plsd->WMMove(hwndDlg, (short int)LOWORD(lParam),
            (short int)HIWORD(lParam)));
    default:
        return(FALSE);
    }
}

// CDFileSetupDlg

CDLineStyleDlg::CDLineStyleDlg(HINSTANCE hInstance)
{
    m_hInstance = hInstance;
    m_iX = -100;
    m_iY = -100;
    m_bSettingUp = false;
}

CDLineStyleDlg::~CDLineStyleDlg()
{
}

bool CDLineStyleDlg::ShowDialog(HWND hWndParent, PDLineStyleRec pLSR)
{
    m_pLSR = pLSR;
    int iRes = DialogBoxParam(m_hInstance, L"LINESTYLEDLG", hWndParent,
        LineStyleDlgProc, (LPARAM)this);
    return (iRes == 1);
}

void CDLineStyleDlg::SaveSettings(CXMLWritter* pWrit)
{
    IXMLDOMElement* pE1 = pWrit->CreateSection(L"LineStyleDlg");
    pWrit->AddIntValue(pE1, L"Left", m_iX);
    pWrit->AddIntValue(pE1, L"Top", m_iY);
    pE1->Release();
    return;
}

void CDLineStyleDlg::RestoreSettings(CXMLReader* pRdr)
{
    int i;
    IXMLDOMElement* pE1 = pRdr->OpenSection(L"LineStyleDlg");
    if(pE1)
    {
        if(pRdr->GetIntValue(pE1, L"Left", &i)) m_iX = i;
        if(pRdr->GetIntValue(pE1, L"Top", &i)) m_iY = i;
        pE1->Release();
    }
    return;
}

INT_PTR CDLineStyleDlg::WMInitDialog(HWND hWnd, HWND hwndFocus, LPARAM lInitParam)
{
    SetWindowLongPtr(hWnd, DWLP_USER, lInitParam);

    m_bSettingUp = true;

    wchar_t buf[64];

    HWND wnd = GetDlgItem(hWnd, LSD_EDT_LINEWIDTH);
    SendMessage(wnd, EM_LIMITTEXT, 64, 0);
    if(m_pLSR->bWidthSet)
    {
        FormatFloatStr(m_pLSR->cLineStyle.dWidth/m_pLSR->cUnit.dBaseToUnit, buf);
        SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);
    }

    wnd = GetDlgItem(hWnd, LSD_LBL_LINEWIDTHUNIT);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)m_pLSR->cUnit.wsAbbrev);

    wnd = GetDlgItem(hWnd, LSD_EDT_LINEPERC);
    SendMessage(wnd, EM_LIMITTEXT, 64, 0);
    if(m_pLSR->bExcSet)
    {
        FormatFloatStr(m_pLSR->cLineStyle.dPercent, buf);
        SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);
    }

    wnd = GetDlgItem(hWnd, LSD_LBL_LINEPATUNIT);
    swprintf(buf, L"(%s)", m_pLSR->cUnit.wsAbbrev);
    SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);

    for(int i = 0; i < 6; i++)
    {
        wnd = GetDlgItem(hWnd, LSD_EDT_LINEPAT1 + i);
        SendMessage(wnd, EM_LIMITTEXT, 64, 0);
        if(m_pLSR->bPatSet && (i < m_pLSR->cLineStyle.iSegments))
        {
            FormatFloatStr(m_pLSR->cLineStyle.dPattern[i]/m_pLSR->cUnit.dBaseToUnit, buf);
            SendMessage(wnd, WM_SETTEXT, 0, (LPARAM)buf);
        }
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
    return TRUE;
}

INT_PTR CDLineStyleDlg::WMCommand(HWND hWnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
    switch(wID)
    {
    case IDOK:
        return(OKBtnClick(hWnd));
    case IDCANCEL:
        EndDialog(hWnd, 0);
        return(TRUE);
    case LSD_EDT_LINEWIDTH:
        return LineWidthChange(hWnd, wNotifyCode, hwndCtl);
    case LSD_EDT_LINEPERC:
        return LineExcChange(hWnd, wNotifyCode, hwndCtl);
    case LSD_EDT_LINEPAT1:
    case LSD_EDT_LINEPAT2:
    case LSD_EDT_LINEPAT3:
    case LSD_EDT_LINEPAT4:
    case LSD_EDT_LINEPAT5:
    case LSD_EDT_LINEPAT6:
        return LinePatChange(hWnd, wNotifyCode, wID - LSD_EDT_LINEPAT1, hwndCtl);
    default:
        return(FALSE);
    }
}

INT_PTR CDLineStyleDlg::WMMove(HWND hWnd, short int xPos, short int yPos)
{
    RECT R;
    GetWindowRect(hWnd, &R);
    m_iX = R.left;
    m_iY = R.top;
    return(FALSE);
}

INT_PTR CDLineStyleDlg::OKBtnClick(HWND hWnd)
{
    float f;
    wchar_t buf[64];
    wchar_t sMsg[256];

    HWND wnd;

    if(m_pLSR->bWidthChanged)
    {
        wnd = GetDlgItem(hWnd, LSD_EDT_LINEWIDTH);
        SendMessage(wnd, WM_GETTEXT, 64, (LPARAM)buf);
        if(swscanf(buf, L"%f", &f) == 1)
        {
            /*if(f < -0.0001)
            {
                LoadString(m_hInstance, IDS_ERRORBASE, buf, 64);
                LoadString(m_hInstance, IDS_ENEGATIVELINEWIDTH, sMsg, 256);
                MessageBox(hWnd, sMsg, buf, MB_ICONERROR | MB_OK);
                SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
                SetFocus(wnd);
                return 0;
            }
            if(f < 0.0001) f = 0.0;*/
            if(fabs(f) < 0.0001) f = 0.0;
            m_pLSR->cLineStyle.dWidth = f*m_pLSR->cUnit.dBaseToUnit;
            m_pLSR->bWidthSet = true;
        }
        else m_pLSR->bWidthSet = false;
    }

    if(m_pLSR->bExcChanged)
    {
        wnd = GetDlgItem(hWnd, LSD_EDT_LINEPERC);
        SendMessage(wnd, WM_GETTEXT, 64, (LPARAM)buf);
        if(swscanf(buf, L"%f", &f) == 1)
        {
            if(fabs(f) < 100.0001)
            {
                m_pLSR->cLineStyle.dPercent = f;
                m_pLSR->bExcSet = true;
            }
            else
            {
                LoadString(m_hInstance, IDS_ERRORBASE, buf, 64);
                LoadString(m_hInstance, IDS_EECCENTTOOBIG, sMsg, 256);
                MessageBox(hWnd, sMsg, buf, MB_ICONERROR | MB_OK);
                SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
                SetFocus(wnd);
                return 0;
            }
        }
        else m_pLSR->bExcSet = false;
    }

    if(m_pLSR->bPatChanged)
    {
        m_pLSR->bExcSet = false;
        m_pLSR->cLineStyle.iSegments = 0;
        for(int i = 0; i < 6; i++)
        {
            wnd = GetDlgItem(hWnd, LSD_EDT_LINEPAT1 + i);
            SendMessage(wnd, WM_GETTEXT, 64, (LPARAM)buf);
            if(swscanf(buf, L"%f", &f) == 1)
            {
                if(f < -0.0001)
                {
                    LoadString(m_hInstance, IDS_ERRORBASE, buf, 64);
                    LoadString(m_hInstance, IDS_ENEGATIVESEGLEN, sMsg, 256);
                    MessageBox(hWnd, sMsg, buf, MB_ICONERROR | MB_OK);
                    SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
                    SetFocus(wnd);
                    return 0;
                }
                if(f > 0.0001)
                {
                    if(m_pLSR->cLineStyle.iSegments < i)
                    {
                        LoadString(m_hInstance, IDS_ERRORBASE, buf, 64);
                        LoadString(m_hInstance, IDS_ELINESEGINVALID, sMsg, 256);
                        MessageBox(hWnd, sMsg, buf, MB_ICONERROR | MB_OK);
                        SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
                        SetFocus(wnd);
                        return 0;
                    }

                    m_pLSR->cLineStyle.iSegments++;
                    m_pLSR->cLineStyle.dPattern[i] = f*m_pLSR->cUnit.dBaseToUnit;
                    m_pLSR->bExcSet = true;
                }
            }
        }
        if((m_pLSR->cLineStyle.iSegments % 2) > 0)
        {
            LoadString(m_hInstance, IDS_ERRORBASE, buf, 64);
            LoadString(m_hInstance, IDS_EODDSEGMENTS, sMsg, 256);
            MessageBox(hWnd, sMsg, buf, MB_ICONERROR | MB_OK);
            SendMessage(wnd, EM_SETSEL, 0, (LPARAM)-1);
            SetFocus(wnd);
            return 0;
        }
    }

    EndDialog(hWnd, 1);
    return(TRUE);
}

INT_PTR CDLineStyleDlg::LineWidthChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(m_bSettingUp) return 0;
    if(wNotifyCode == EN_CHANGE) m_pLSR->bWidthChanged = true;
    return TRUE;
}

INT_PTR CDLineStyleDlg::LineExcChange(HWND hWnd, WORD wNotifyCode, HWND hwndCtl)
{
    if(m_bSettingUp) return 0;
    if(wNotifyCode == EN_CHANGE) m_pLSR->bExcChanged = true;
    return TRUE;
}

INT_PTR CDLineStyleDlg::LinePatChange(HWND hWnd, WORD wNotifyCode, int iSeg, HWND hwndCtl)
{
    if(m_bSettingUp) return 0;
    if(wNotifyCode == EN_CHANGE) m_pLSR->bPatChanged = true;
    return TRUE;
}
