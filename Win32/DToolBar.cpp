#include <tchar.h>
#include "DToolBar.hpp"
#include <commctrl.h>
#include "Draw.rh"

HBITMAP GetMenuBitmap(HIMAGELIST himl, HDC hdc, int iIndex)
{
    IMAGEINFO cii;
    ImageList_GetImageInfo(himl, iIndex, &cii);

    HBITMAP hBmpRes = CreateCompatibleBitmap(hdc, cii.rcImage.right - cii.rcImage.left,
        cii.rcImage.bottom - cii.rcImage.top);
    HDC hDstDC = CreateCompatibleDC(hdc);

    SelectObject(hDstDC, hBmpRes);
    HPEN hPen = CreatePen(PS_NULL, 0, 0);
    SelectObject(hDstDC, hPen);
    SelectObject(hDstDC, GetSysColorBrush(COLOR_MENU));
    Rectangle(hDstDC, 0, 0, cii.rcImage.right - cii.rcImage.left + 1,
        cii.rcImage.bottom - cii.rcImage.top + 1);
    ImageList_Draw(himl, iIndex, hDstDC, 0, 0, ILD_TRANSPARENT);
    DeleteObject(hPen);

    DeleteDC(hDstDC);

    return hBmpRes;
}

CDToolbar::CDToolbar(HWND hWnd, HINSTANCE hInstance, HMENU hMenu)
{
    HDC hdc = GetDC(hWnd);
    m_hWnd = hWnd;

    m_iMnuBmps = 0;

    MENUITEMINFO cmii;
    cmii.cbSize = sizeof(MENUITEMINFO);
    cmii.fMask = MIIM_BITMAP;

    TBBUTTON btns[24];
    SendMessage(hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    SendMessage(hWnd, TB_SETBITMAPSIZE , 0, MAKELONG(16, 16));
    //SendMessage(hWnd, TB_SETBUTTONSIZE , 0, MAKELONG(24, 24));
    SendMessage(hWnd, TB_LOADIMAGES, IDB_STD_SMALL_COLOR, (LPARAM)HINST_COMMCTRL);
    HIMAGELIST shiml = (HIMAGELIST)SendMessage(hWnd, TB_GETIMAGELIST, 0, 0);
    HICON ico;
    int ico_idx, btncount = 0;

    // New template button
    btns[btncount].iBitmap = STD_FILENEW;
    btns[btncount].idCommand = IDM_FILENEW;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_FILENEW);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_FILENEW, FALSE, &cmii);

    // Open template button
    btns[btncount].iBitmap = STD_FILEOPEN;
    btns[btncount].idCommand = IDM_FILEOPEN;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_FILEOPEN);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_FILEOPEN, FALSE, &cmii);

    // Save template button
    btns[btncount].iBitmap = STD_FILESAVE;
    btns[btncount].idCommand = IDM_FILESAVE;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_FILESAVE);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_FILESAVE, FALSE, &cmii);

    btns[btncount].iBitmap = 2;
    btns[btncount].idCommand = 0;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_SEP;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    // Properties template button
    btns[btncount].iBitmap = STD_PROPERTIES;
    btns[btncount].idCommand = IDM_FILEPROPS;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_PROPERTIES);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_FILEPROPS, FALSE, &cmii);

    // Print setup template button
    btns[btncount].iBitmap = STD_PRINTPRE;
    btns[btncount].idCommand = IDM_FILEPRINTSET;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_PRINTPRE);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_FILEPRINTSET, FALSE, &cmii);

    // Print template button
    btns[btncount].iBitmap = STD_PRINT;
    btns[btncount].idCommand = IDM_FILEPRINT;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_PRINT);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_FILEPRINT, FALSE, &cmii);

    btns[btncount].iBitmap = 6;
    btns[btncount].idCommand = 0;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_SEP;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    // Copy template button
    btns[btncount].iBitmap = STD_COPY;
    btns[btncount].idCommand = IDM_EDITCOPY;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_COPY);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_EDITCOPY, FALSE, &cmii);

    // Cut template button
    btns[btncount].iBitmap = STD_CUT;
    btns[btncount].idCommand = IDM_EDITCUT;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_CUT);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_EDITCUT, FALSE, &cmii);

    // Paste template button
    btns[btncount].iBitmap = STD_PASTE;
    btns[btncount].idCommand = IDM_EDITPASTE;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_PASTE);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_EDITPASTE, FALSE, &cmii);

    // Undo template button
    btns[btncount].iBitmap = STD_UNDO;
    btns[btncount].idCommand = IDM_EDITUNDO;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_UNDO);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_EDITUNDO, FALSE, &cmii);

    // Redo template button
    btns[btncount].iBitmap = STD_REDOW;
    btns[btncount].idCommand = IDM_EDITREDO;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, STD_REDOW);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_EDITREDO, FALSE, &cmii);

    m_iSepPos = btncount;
    btns[btncount].iBitmap = 120;
    btns[btncount].idCommand = 0;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_SEP;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    // Help button
    btns[btncount].iBitmap = STD_HELP;
    btns[btncount].idCommand = IDM_HELPCONTENT;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    // Exit button
    ico = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ICO_EXIT),
        IMAGE_ICON, 0, 0, 0);
    ico_idx = ImageList_ReplaceIcon(shiml, -1, ico);
    btns[btncount].iBitmap = ico_idx;
    btns[btncount].idCommand = IDM_FILEEXIT;
    btns[btncount].fsState = TBSTATE_ENABLED;
    btns[btncount].fsStyle = TBSTYLE_BUTTON; // or TBSTYLE_AUTOSIZE;
    btns[btncount].dwData = 0;
    btns[btncount++].iString = -1;

    m_phMnuBmps[m_iMnuBmps++] = GetMenuBitmap(shiml, hdc, ico_idx);
    cmii.hbmpItem = m_phMnuBmps[m_iMnuBmps - 1];
    SetMenuItemInfo(hMenu, IDM_FILEEXIT, FALSE, &cmii);

    SendMessage(hWnd, TB_ADDBUTTONS, btncount, (LPARAM)&btns[0]);
    m_hSepBtn = CreateWindowEx(0, _T("Button"), NULL,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 0, 0, 120, 16, hWnd, 0,
        hInstance, NULL);

    ReleaseDC(hWnd, hdc);
}

CDToolbar::~CDToolbar()
{
    for(int i = 0; i < m_iMnuBmps; i++)
    {
        DeleteObject(m_phMnuBmps[i]);
    }
}

void CDToolbar::Resize(HWND hWnd, RECT *pRect, int iRefBtnOffs)
{
    SendMessage(hWnd, WM_SIZE, 0, 0);

    int ivisible = m_iSepPos - 1 - iRefBtnOffs;

    RECT RBtn;
    GetClientRect(hWnd, pRect);
    int ex = 0;
    if(iRefBtnOffs > 7)
    {
        SendMessage(hWnd, TB_GETITEMRECT, ivisible + 1, (LONG_PTR)&RBtn);
        ex = RBtn.right - RBtn.left;
    }
    SendMessage(hWnd, TB_GETITEMRECT, ivisible, (LONG_PTR)&RBtn);
    int nw = pRect->right - RBtn.right - 2*(RBtn.right - RBtn.left) - ex;

    TBBUTTON tb;
    tb.iBitmap = nw;
    tb.idCommand = 0;
    tb.fsState = TBSTATE_ENABLED;
    tb.fsStyle = TBSTYLE_SEP;
    tb.dwData = 0;
    tb.iString = -1;

    SendMessage(hWnd, TB_DELETEBUTTON, m_iSepPos, 0);
    SendMessage(hWnd, TB_INSERTBUTTON, m_iSepPos, (LONG_PTR)&tb);
    SetWindowPos(m_hSepBtn, 0, RBtn.right, RBtn.top, nw, RBtn.bottom - RBtn.top,
        SWP_NOZORDER | SWP_NOOWNERZORDER);
    GetWindowRect(hWnd, pRect);
    OffsetRect(pRect, -pRect->left, -pRect->top);
    return;
}
