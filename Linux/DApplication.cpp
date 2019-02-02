#include <string.h>
//#include <glib.h>
#include <glib/gstdio.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <math.h>
#include <malloc.h>

#include "DApplication.hpp"
#include "DMenu.hpp"
#include "DFileMenu.hpp"
#include "../Source/DMath.hpp"
#include "../Source/DExpCairo.hpp"
#include "../Source/DExpDXF.hpp"


double GetPtDist(GdkPoint *pPt, int x, int y)
{
    return fabs(pPt->x - x) + fabs(pPt->y - y);
}


gboolean app_delete(GtkWidget *widget, GdkEvent *event, PDApplication pApp)
{
    return pApp->Terminate();
}

gboolean app_configure(GtkWidget *widget, GdkEventConfigure *event, PDApplication pApp)
{
    pApp->Configure(widget, event);
    return FALSE;
}

gboolean app_paint(GtkWidget *widget, GdkEventExpose *event, PDApplication pApp)
{
    pApp->Paint(widget, event);
    return FALSE;
}

gboolean app_mouse_move(GtkWidget *widget, GdkEventMotion *event, PDApplication pApp)
{
    pApp->MouseMove(widget, event, FALSE);
    return FALSE;
}

gboolean app_mouse_wheel(GtkWidget *widget, GdkEventScroll *event, PDApplication pApp)
{
    pApp->MouseWheel(widget, event);
    return FALSE;
}

gboolean app_mouse_down(GtkWidget *widget, GdkEventButton *event, PDApplication pApp)
{
    switch(event->button)
    {
    case 1:
        if(event->type == GDK_2BUTTON_PRESS)
            pApp->MouseLDblClick(widget, event);
        else pApp->MouseLButtonDown(widget, event);
        break;
    case 2:
        pApp->MouseMButtonDown(widget, event);
        break;
    case 3:
        pApp->MouseRButtonDown(widget, event);
        break;
    }
    return FALSE;
}

gboolean app_mouse_up(GtkWidget *widget, GdkEventButton *event, PDApplication pApp)
{
    switch(event->button)
    {
    case 1:
        pApp->MouseLButtonUp(widget, event);
        break;
    case 2:
        pApp->MouseMButtonUp(widget, event);
        break;
    case 3:
        pApp->MouseRButtonUp(widget, event);
        break;
    }
    return FALSE;
}

void app_edt1_changed(GtkEntry *entry, PDApplication pApp)
{
    pApp->Edit1Changed(entry);
}

void app_enable_snap_mnu(GtkMenuItem *menuitem, PDApplication pApp)
{
    pApp->EnableSnap();
}

void app_disable_snap_mnu(GtkMenuItem *menuitem, PDApplication pApp)
{
    pApp->DisableSnap();
}


void CDApplication::CopyIniFiles(const char *psConfDir)
{
	const gchar *homedir = g_get_home_dir();

	gchar *sFile1, *sFile2;
    GFile *gf1, *gf2;

    gchar *cnfdir = g_strconcat(homedir, "/.SteamCAD", NULL);
    g_mkdir(cnfdir, S_IRWXU | S_IRWXG);

    sFile1 = g_strconcat(cnfdir, "/DPapers.ini", NULL);
    gf1 = g_file_new_for_path((const char *)sFile1);
    if(!g_file_query_exists(gf1, NULL))
    {
    	sFile2 = g_strconcat(psConfDir, "/DPapers.ini", NULL);
        gf2 = g_file_new_for_path((const char *)sFile2);
        if(g_file_query_exists(gf2, NULL))
        {
            g_file_copy(gf2, gf1, G_FILE_COPY_TARGET_DEFAULT_PERMS, NULL, NULL, NULL, NULL);
        }
        g_object_unref(gf2);
        g_free(sFile2);
    }
    g_object_unref(gf1);
    g_free(sFile1);

	sFile1 = g_strconcat(cnfdir, "/DUnits.ini", NULL);
    gf1 = g_file_new_for_path((const char *)sFile1);
    if(!g_file_query_exists(gf1, NULL))
    {
    	sFile2 = g_strconcat(psConfDir, "/DUnits.ini", NULL);
        gf2 = g_file_new_for_path((const char *)sFile2);
        if(g_file_query_exists(gf2, NULL))
        {
            g_file_copy(gf2, gf1, G_FILE_COPY_TARGET_DEFAULT_PERMS, NULL, NULL, NULL, NULL);
        }
        g_object_unref(gf2);
        g_free(sFile2);
    }
    g_object_unref(gf1);
    g_free(sFile1);

    g_free(cnfdir);
}

CDApplication::CDApplication(const char *psConfDir)
{
    if(psConfDir) CopyIniFiles(psConfDir);

    m_bSettingProps = false;
    m_pStatLab1 = NULL;

    m_pSnapEnableMnu = gtk_menu_new();
    m_pSnapDisableMnu = gtk_menu_new();
    GtkWidget *pMnuItem = gtk_menu_item_new_with_label(_("Enable snap"));
    g_signal_connect(G_OBJECT(pMnuItem), "activate", G_CALLBACK(app_enable_snap_mnu), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(m_pSnapEnableMnu), pMnuItem);
    gtk_widget_show(pMnuItem);

    pMnuItem = gtk_menu_item_new_with_label(_("Disable snap"));
    g_signal_connect(G_OBJECT(pMnuItem), "activate", G_CALLBACK(app_disable_snap_mnu), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(m_pSnapDisableMnu), pMnuItem);
    gtk_widget_show(pMnuItem);

    /* create a new window */
    m_pMainWnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(m_pMainWnd, 400, 300);
    gtk_window_set_title(GTK_WINDOW(m_pMainWnd), "SteamCAD");
    g_signal_connect(G_OBJECT(m_pMainWnd), "delete-event", G_CALLBACK(app_delete), this);

    /* A vbox to put a menu and a button in: */
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(m_pMainWnd), vbox);
    gtk_widget_show(vbox);

    m_pAccelGroup = gtk_accel_group_new();
    GtkAccelGroup *pAccelGroup = InitMenu(vbox, m_pAccelGroup, this);
	
    gtk_window_add_accel_group(GTK_WINDOW(m_pMainWnd), pAccelGroup);
    gtk_window_add_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);

    GtkWidget *draw = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), draw, TRUE, TRUE, 0);
    gtk_widget_show(draw);

    gtk_widget_set_events(draw, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_STRUCTURE_MASK);
    g_signal_connect(G_OBJECT(draw), "expose-event", G_CALLBACK(app_paint), this);
    g_signal_connect(G_OBJECT(draw), "configure-event", G_CALLBACK(app_configure), this);
    g_signal_connect(G_OBJECT(draw), "motion-notify-event", G_CALLBACK(app_mouse_move), this);
    g_signal_connect(G_OBJECT(draw), "scroll-event", G_CALLBACK(app_mouse_wheel), this);
    g_signal_connect(G_OBJECT(draw), "button-press-event", G_CALLBACK(app_mouse_down), this);
    g_signal_connect(G_OBJECT(draw), "button-release-event", G_CALLBACK(app_mouse_up), this);

    GtkWidget *pStatBar = GetStatusBar();

    GList *pChilds = gtk_container_get_children(GTK_CONTAINER(pStatBar));
    GtkWidget *pFrame = (GtkWidget*)g_list_nth(pChilds, 0)->data;

    gtk_box_set_child_packing(GTK_BOX(pStatBar), pFrame, FALSE, TRUE, 0, GTK_PACK_START);
    gtk_widget_set_size_request(pFrame, 150, 23);

    m_pStatSep1 = gtk_vseparator_new();
    gtk_box_pack_start(GTK_BOX(pStatBar), m_pStatSep1, FALSE, TRUE, 0);
    gtk_widget_show(m_pStatSep1);

    m_pStatLab1 = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(m_pStatLab1), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(m_pStatLab1), 5, 1);
    gtk_box_pack_start(GTK_BOX(pStatBar), m_pStatLab1, FALSE, TRUE, 0);
    gtk_widget_set_size_request(m_pStatLab1, 280, 23);
    gtk_widget_show(m_pStatLab1);

    m_pStatEdt1 = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pStatBar), m_pStatEdt1, FALSE, TRUE, 0);
    gtk_widget_set_size_request(m_pStatEdt1, 100, 23);
    //gtk_widget_show(m_pStatEdt1);
    g_signal_connect(G_OBJECT(m_pStatEdt1), "changed", G_CALLBACK(app_edt1_changed), this);

    m_pStatLab2 = gtk_label_new(_("# of copies: "));
    gtk_misc_set_alignment(GTK_MISC(m_pStatLab2), 1.0, 0.5);
    gtk_misc_set_padding(GTK_MISC(m_pStatLab2), 5, 1);
    gtk_box_pack_start(GTK_BOX(pStatBar), m_pStatLab2, FALSE, TRUE, 0);
    gtk_widget_set_size_request(m_pStatLab2, 150, 23);
    //gtk_widget_show(m_pStatLab2);

    m_pStatEdt2 = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(pStatBar), m_pStatEdt2, FALSE, TRUE, 0);
    gtk_widget_set_size_request(m_pStatEdt2, 100, 23);
    //gtk_widget_show(m_pStatEdt2);

    m_pStatSep2 = gtk_vseparator_new();
    gtk_box_pack_start(GTK_BOX(pStatBar), m_pStatSep2, FALSE, TRUE, 0);
    gtk_widget_show(m_pStatSep2);

    m_pStatLab3 = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(m_pStatLab3), 0, 0.5);
    gtk_misc_set_padding(GTK_MISC(m_pStatLab3), 5, 1);
    gtk_box_pack_start(GTK_BOX(pStatBar), m_pStatLab3, TRUE, TRUE, 0);
    gtk_widget_show(m_pStatLab3);

    m_pDrawObjects = new CDataList();
    m_pUndoObjects = new CDataList();
    m_bHasChanged = false;
    m_sFileName = NULL;
    m_pActiveObject = NULL;
    m_pHighObject = NULL;
    m_pSelForDimen = NULL;
    m_iRedoCount = 0;

    strcpy(m_cFSR.cPaperSize.sPaperSizeName, "A4");
    m_cFSR.cPaperSize.dPaperWidth = 210.0;
    m_cFSR.cPaperSize.dPaperHeight = 297.0;
    m_cFSR.bPortrait = false;
    strcpy(m_cFSR.cLenUnit.sName, "millimeter");
    strcpy(m_cFSR.cLenUnit.sAbbrev, "mm");
    m_cFSR.cLenUnit.dBaseToUnit = 1.0;
    strcpy(m_cFSR.cLenUnit.sAbbrev2, "");
    m_cFSR.dScaleNomin = 1.0;
    m_cFSR.dScaleDenom = 1.0;
    strcpy(m_cFSR.cAngUnit.sName, "degree");
    strcpy(m_cFSR.cAngUnit.sAbbrev, "deg");
    m_cFSR.cAngUnit.dBaseToUnit = 1.0;

    gsize iLen;
    gchar *sTmp = g_convert("\xB0", -1, "UTF-8", "ISO-8859-1", NULL, &iLen, NULL);
    strcpy(m_cFSR.cAngUnit.sAbbrev2, sTmp);
    g_free(sTmp);

    strcpy(m_cFSR.cPaperUnit.sName, "millimeter");
    strcpy(m_cFSR.cPaperUnit.sAbbrev, "mm");
    m_cFSR.cPaperUnit.dBaseToUnit = 1.0;
    strcpy(m_cFSR.cPaperUnit.sAbbrev2, "");
    strcpy(m_cFSR.cGraphUnit.sName, "millimeter");
    strcpy(m_cFSR.cGraphUnit.sAbbrev, "mm");
    m_cFSR.cGraphUnit.dBaseToUnit = 1.0;
    strcpy(m_cFSR.cGraphUnit.sAbbrev2, "");
    m_cFSR.dAngGrid = 15.0;
    m_cFSR.dXGrid = 10.0;
    m_cFSR.dYGrid = 10.0;
    m_cFSR.dDefLineWidth = 0.2;
    m_cFSR.iArrowType = 1;
    m_cFSR.dArrowLen = 3.0;
    m_cFSR.dArrowWidth = 0.6;
    m_cFSR.bFontAttrs = 0;
    m_cFSR.dFontSize = 2.0;
    m_cFSR.dBaseLine = 1.0;
    strcpy(m_cFSR.sFontFace, "Sans");
    strcpy(m_cFSR.sLengthMask, "[D:2]");

    sTmp = g_convert("[r:2]\xB0", -1, "UTF-8", "ISO-8859-1", NULL, &iLen, NULL);
    strcpy(m_cFSR.sAngleMask, sTmp);
    g_free(sTmp);

    m_sLastPath = NULL;
    m_dDrawScale = 1.0;
    m_cViewOrigin.x = 0;
    m_cViewOrigin.y = 0;
    m_cLastSnapPt.x = -100;
    m_cLastSnapPt.y = -100;
    m_lSelColor = 0x0024E907; //0x00008888;
    m_lHighColor = 0x00EDD52C; //0x00888800;
    m_lActiveColor = 0x00FF0000;
    m_iHighDimen = -2;
    m_iDrawMode = modSelect;
    m_iButton = 0;
    m_iToolMode = tolNone;
    m_cLastDynPt.bIsSet = false;
    m_pcs = NULL;
    m_pcp = NULL;
    m_bRenderDirect = FALSE;
    m_cMeasPoint1.bIsSet = false;
    m_cMeasPoint2.bIsSet = false;
    m_bPaperUnits = FALSE;
    m_iDrawGridMode = 0;

    m_sStatus2Msg[0] = 0;
    m_iRestrictSet = -1;
    m_iLastExportType = 0;

    m_pFileSetupDlg = new CDFileSetupDlg();
    m_pLineStyleDlg = new CDLineStyleDlg();
    m_pDimEditDlg = new CDDimEditDlg();
    m_pStatDlg = new CDStatDlg();
    m_pSnapDlg = new CDSnapDlg();
    m_pScaleDlg = new CDScaleDlg();
	
    RestoreSettings();

    m_bSettingProps = true;
    GtkWidget *menu = GetMenuBar();
    pChilds = gtk_container_get_children(GTK_CONTAINER(menu));

    GtkWidget *edit_menu = (GtkWidget*)g_list_nth(pChilds, 2)->data;
    GtkWidget *edit_top = (GtkWidget*)gtk_menu_item_get_submenu(GTK_MENU_ITEM(edit_menu));
    GList *pMenuChilds = gtk_container_get_children(GTK_CONTAINER(edit_top));
    GtkWidget *menu_item = (GtkWidget*)g_list_nth(pMenuChilds, 7)->data;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), m_bPaperUnits);

    edit_menu = (GtkWidget*)g_list_nth(pChilds, 3)->data;
    edit_top = (GtkWidget*)gtk_menu_item_get_submenu(GTK_MENU_ITEM(edit_menu));
    pMenuChilds = gtk_container_get_children(GTK_CONTAINER(edit_top));
    menu_item = (GtkWidget*)g_list_nth(pMenuChilds, 3)->data;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), m_iDrawGridMode & 1);

    menu_item = (GtkWidget*)g_list_nth(pMenuChilds, 4)->data;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), m_iDrawGridMode & 2);

    m_bSettingProps = false;

    if(fabs(m_cFSR.dScaleDenom) > g_dPrec)
        m_dDrawScale = m_cFSR.dScaleNomin/m_cFSR.dScaleDenom;
    else m_dDrawScale = 1.0;

    GetPageDims();
    CDFileAttrs cFAttrs;
    FilePropsToData(&cFAttrs);
    m_pDrawObjects->SetFileAttrs(&cFAttrs, true);

    SetTitle(m_pMainWnd, true);

    if(psConfDir)
    {
        gchar *sIcon = g_strconcat(psConfDir, "/steamcad.png", NULL);
        gtk_window_set_icon_from_file(GTK_WINDOW(m_pMainWnd), (const gchar*)sIcon, NULL);
        g_free(sIcon);
    }

    /* always display the window as the last step so it all splashes on
    * the screen at once. */
    gtk_widget_show(m_pMainWnd);

    GdkScreen *scr = gdk_window_get_screen(m_pMainWnd->window);
    gint iPixW = gdk_screen_get_width(scr);
    gint iMMW = gdk_screen_get_width_mm(scr);
    m_dDeviceToUnitScale = (double)iPixW/iMMW;
    m_iSelectTolerance = (int)1.0*m_dDeviceToUnitScale;
    m_iSnapTolerance = (int)2.0*m_dDeviceToUnitScale;

    ViewFitCmd(m_pMainWnd);
}

CDApplication::~CDApplication()
{
    if(m_pcp) cairo_pattern_destroy(m_pcp);
    if(m_pcs) cairo_surface_destroy(m_pcs);
    m_pcs = NULL;
    if(m_sFileName) g_free(m_sFileName);
    if(m_sLastPath) g_free(m_sLastPath);
    delete m_pUndoObjects;
    delete m_pDrawObjects;
    delete m_pScaleDlg;
    delete m_pSnapDlg;
    delete m_pStatDlg;
    delete m_pDimEditDlg;
    delete m_pLineStyleDlg;
    delete m_pFileSetupDlg;
}

GtkWidget* CDApplication::GetMainWindow()
{
    return(m_pMainWnd);
}

int od_find_attr(const gchar **attribute_names, const gchar *attr_name)
{
    int i = 0;
    bool bFound = false;
    const gchar *pAttr = attribute_names[i++];
    while(!bFound && pAttr)
    {
        bFound = (g_strcmp0(pAttr, attr_name) == 0);
        pAttr = attribute_names[i++];
    }
    return bFound ? i - 2 : -1;
}

void od_start_elem(GMarkupParseContext *context, const gchar *element_name,
    const gchar **attribute_names, const gchar **attribute_values,
    gpointer user_data, GError **error)
{
    PDApplication pApp = (PDApplication)user_data;
    gint left, top, width, height;
    GdkGravity grav;
    CDFileSetupRec cFSR;
    float f;
    gint i;

    if(g_strcmp0(element_name, "MainForm") == 0)
    {
        sscanf(attribute_values[0], "%d", &left);
        sscanf(attribute_values[1], "%d", &top);
        sscanf(attribute_values[2], "%d", &width);
        sscanf(attribute_values[3], "%d", &height);
        sscanf(attribute_values[4], "%d", (int*)&grav);
        pApp->SetPosition(left, top, width, height, grav);
    }
    else if(g_strcmp0(element_name, "DrawSettings") == 0)
    {
        i = od_find_attr(attribute_names, "PaperUnits");
        if(i < 0) left = 0;
        else sscanf(attribute_values[i], "%d", &left);

        i = od_find_attr(attribute_names, "LastExportType");
        if(i < 0) top = 0;
        else sscanf(attribute_values[i], "%d", &top);

        i = od_find_attr(attribute_names, "DrawGridMode");
        if(i < 0) width = 0;
        else sscanf(attribute_values[i], "%d", &width);

        i = od_find_attr(attribute_names, "LastPath");
        if(i < 0) pApp->SetDrawSettings(left, top, width, NULL);
        else pApp->SetDrawSettings(left, top, width, attribute_values[i]);
    }
    else if(g_strcmp0(element_name, "PageSettings") == 0)
    {
        sscanf(attribute_values[0], "%d", &i);
        cFSR.bPortrait = i;
        sscanf(attribute_values[1], "%f", &f);
        cFSR.dScaleNomin = f;
        sscanf(attribute_values[2], "%f", &f);
        cFSR.dScaleDenom = f;
        sscanf(attribute_values[3], "%f", &f);
        cFSR.dAngGrid = f;
        sscanf(attribute_values[4], "%f", &f);
        cFSR.dXGrid = f;
        sscanf(attribute_values[5], "%f", &f);
        cFSR.dYGrid = f;
        sscanf(attribute_values[6], "%f", &f);
        cFSR.dDefLineWidth = f;
        pApp->SetPageSettings(&cFSR);
    }
    else if(g_strcmp0(element_name, "PaperSize") == 0)
    {
        strcpy(cFSR.cPaperSize.sPaperSizeName, attribute_values[0]);
        sscanf(attribute_values[1], "%f", &f);
        cFSR.cPaperSize.dPaperWidth = f;
        sscanf(attribute_values[2], "%f", &f);
        cFSR.cPaperSize.dPaperHeight = f;
        pApp->SetPaperSize(&cFSR);
    }
    else if(g_strcmp0(element_name, "LengthUnit") == 0)
    {
        strcpy(cFSR.cLenUnit.sName, attribute_values[0]);
        strcpy(cFSR.cLenUnit.sAbbrev, attribute_values[1]);
        sscanf(attribute_values[2], "%f", &f);
        cFSR.cLenUnit.dBaseToUnit = f;
        strcpy(cFSR.cLenUnit.sAbbrev2, attribute_values[3]);
        pApp->SetLengthUnit(&cFSR);
    }
    else if(g_strcmp0(element_name, "AngularUnit") == 0)
    {
        strcpy(cFSR.cAngUnit.sName, attribute_values[0]);
        strcpy(cFSR.cAngUnit.sAbbrev, attribute_values[1]);
        sscanf(attribute_values[2], "%f", &f);
        cFSR.cAngUnit.dBaseToUnit = f;
        strcpy(cFSR.cAngUnit.sAbbrev2, attribute_values[3]);
        pApp->SetAngularUnit(&cFSR);
    }
    else if(g_strcmp0(element_name, "PaperUnit") == 0)
    {
        strcpy(cFSR.cPaperUnit.sName, attribute_values[0]);
        strcpy(cFSR.cPaperUnit.sAbbrev, attribute_values[1]);
        sscanf(attribute_values[2], "%f", &f);
        cFSR.cPaperUnit.dBaseToUnit = f;
        strcpy(cFSR.cPaperUnit.sAbbrev2, attribute_values[3]);
        pApp->SetPaperUnit(&cFSR);
    }
    else if(g_strcmp0(element_name, "GraphUnit") == 0)
    {
        strcpy(cFSR.cGraphUnit.sName, attribute_values[0]);
        strcpy(cFSR.cGraphUnit.sAbbrev, attribute_values[1]);
        sscanf(attribute_values[2], "%f", &f);
        cFSR.cGraphUnit.dBaseToUnit = f;
        strcpy(cFSR.cGraphUnit.sAbbrev2, attribute_values[3]);
        pApp->SetGraphUnit(&cFSR);
    }
    else if(g_strcmp0(element_name, "Dimensioning") == 0)
    {
        sscanf(attribute_values[0], "%d", &i);
        cFSR.iArrowType = i;
        sscanf(attribute_values[1], "%f", &f);
        cFSR.dArrowLen = f;
        sscanf(attribute_values[2], "%f", &f);
        cFSR.dArrowWidth = f;
        sscanf(attribute_values[3], "%d", &i);
        cFSR.bFontAttrs = i;
        sscanf(attribute_values[4], "%f", &f);
        cFSR.dFontSize = f;
        sscanf(attribute_values[5], "%f", &f);
        cFSR.dBaseLine = f;
        strcpy(cFSR.sFontFace, attribute_values[6]);
        strcpy(cFSR.sLengthMask, attribute_values[7]);
        strcpy(cFSR.sAngleMask, attribute_values[8]);
        pApp->SetDimensioning(&cFSR);
    }
    else if(g_strcmp0(element_name, "FileSetupDlg") == 0)
    {
        sscanf(attribute_values[0], "%d", &left);
        sscanf(attribute_values[1], "%d", &top);
        pApp->SetFileSetupDlg(left, top);
    }
    else if(g_strcmp0(element_name, "LineStyleDlg") == 0)
    {
        sscanf(attribute_values[0], "%d", &left);
        sscanf(attribute_values[1], "%d", &top);
        pApp->SetLineStyleDlg(left, top);
    }
    else if(g_strcmp0(element_name, "DimEditDlg") == 0)
    {
        sscanf(attribute_values[0], "%d", &left);
        sscanf(attribute_values[1], "%d", &top);
        pApp->SetDimEditDlg(left, top);
    }
    else if(g_strcmp0(element_name, "StatDlg") == 0)
    {
        sscanf(attribute_values[0], "%d", &left);
        sscanf(attribute_values[1], "%d", &top);
        pApp->SetStatDlg(left, top);
    }
    else if(g_strcmp0(element_name, "SnapDlg") == 0)
    {
        sscanf(attribute_values[0], "%d", &left);
        sscanf(attribute_values[1], "%d", &top);
        pApp->SetSnapDlg(left, top);
    }
    else if(g_strcmp0(element_name, "ScaleDlg") == 0)
    {
        sscanf(attribute_values[0], "%d", &left);
        sscanf(attribute_values[1], "%d", &top);
        pApp->SetScaleDlg(left, top);
    }
    return;
}

void od_end_elem(GMarkupParseContext *context, const gchar *element_name,
    gpointer user_data, GError **error)
{
    return;
}

/* Called for character data */
/* text is not nul-terminated */
void od_text_elem(GMarkupParseContext *context, const gchar *text, gsize text_len,  
    gpointer user_data, GError **error)
{
    return;
}

void CDApplication::RestoreSettings()
{
    const gchar *homedir = g_get_home_dir();
    gchar *cnfname = g_strconcat(homedir, "/.SteamCAD/config.xml", NULL);

    gchar *filecont = NULL;
    gsize slen = 0;

    if(g_file_get_contents(cnfname, &filecont, &slen, NULL))
    {
        GMarkupParser parser = {od_start_elem, od_end_elem, od_text_elem, NULL, NULL};
        GMarkupParseContext* pmpc = g_markup_parse_context_new(&parser, 
	        (GMarkupParseFlags)0, this, NULL);
        g_markup_parse_context_parse(pmpc, filecont, slen, NULL);
        g_markup_parse_context_free(pmpc);
        g_free(filecont);
    }
    else
    {
        gtk_window_move(GTK_WINDOW(m_pMainWnd), 10, 10);
        gtk_window_resize(GTK_WINDOW(m_pMainWnd), 400, 300);
        gtk_window_set_gravity(GTK_WINDOW(m_pMainWnd), GDK_GRAVITY_STATIC);
    }

    g_free(cnfname);

    return;
}

void CDApplication::SaveSettings()
{
    const gchar *homedir = g_get_home_dir();
    gchar *cnfdir = g_strconcat(homedir, "/.SteamCAD", NULL);
    g_mkdir(cnfdir, S_IRWXU | S_IRWXG);
    gchar *cnfname = g_strconcat(cnfdir, "/config.xml", NULL);
    FILE *fp = g_fopen(cnfname, "w");
    g_free(cnfname);
    g_free(cnfdir);

    gint x, y, dx, dy;
    gtk_window_get_position(GTK_WINDOW(m_pMainWnd), &x, &y);
    gtk_window_get_size(GTK_WINDOW(m_pMainWnd), &dx, &dy);
    gint igr = gtk_window_get_gravity(GTK_WINDOW(m_pMainWnd));

    gchar sbuf[256];
    g_strlcpy(sbuf, "<?xml version=\"1.0\"?>\n<!--SteamCAD Workspace Settings-->\n<Settings>\n", 256);
        fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);
    g_sprintf(sbuf, "  <MainForm Left=\"%d\" Top=\"%d\" Width=\"%d\" Height=\"%d\" Gravity=\"%d\"/>\n", 
        x, y, dx, dy, igr);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    g_sprintf(sbuf, "  <DrawSettings PaperUnits=\"%d\" LastExportType=\"%d\" DrawGridMode=\"%d\"\n", m_bPaperUnits, m_iLastExportType, m_iDrawGridMode);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    if(m_sLastPath)	g_sprintf(sbuf, "    LastPath=\"%s\"/>\n", m_sLastPath);
    else strcpy(sbuf, "    LastPath=\"\"/>\n");
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    g_sprintf(sbuf, "  <PageSettings Portrait=\"%d\" ScaleNomin=\"%.3f\" ScaleDenom=\"%.3f\"\n",
        m_cFSR.bPortrait, m_cFSR.dScaleNomin, m_cFSR.dScaleDenom);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    g_sprintf(sbuf, "    AngularGrid=\"%.3f\" XGrid=\"%.3f\" YGrid=\"%.3f\" DefLineWidth=\"%.3f\">\n",
        m_cFSR.dAngGrid, m_cFSR.dXGrid, m_cFSR.dYGrid, m_cFSR.dDefLineWidth);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    g_sprintf(sbuf, "    <PaperSize PaperName=\"%s\" PaperWidth=\"%.3f\" PaperHeight=\"%.3f\"/>\n",
        m_cFSR.cPaperSize.sPaperSizeName, m_cFSR.cPaperSize.dPaperWidth, m_cFSR.cPaperSize.dPaperHeight);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    g_sprintf(sbuf, "    <LengthUnit UnitName=\"%s\" UnitAbbrev=\"%s\" UnitScale=\"%.3f\" UnitAbbrev2=\"%s\"/>\n",
        m_cFSR.cLenUnit.sName, m_cFSR.cLenUnit.sAbbrev, m_cFSR.cLenUnit.dBaseToUnit, m_cFSR.cLenUnit.sAbbrev2);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    g_sprintf(sbuf, "    <AngularUnit UnitName=\"%s\" UnitAbbrev=\"%s\" UnitScale=\"%.3f\" UnitAbbrev2=\"%s\"/>\n",
        m_cFSR.cAngUnit.sName, m_cFSR.cAngUnit.sAbbrev, m_cFSR.cAngUnit.dBaseToUnit, m_cFSR.cAngUnit.sAbbrev2);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    g_sprintf(sbuf, "    <PaperUnit UnitName=\"%s\" UnitAbbrev=\"%s\" UnitScale=\"%.3f\" UnitAbbrev2=\"%s\"/>\n",
        m_cFSR.cPaperUnit.sName, m_cFSR.cPaperUnit.sAbbrev, m_cFSR.cPaperUnit.dBaseToUnit, m_cFSR.cPaperUnit.sAbbrev2);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    g_sprintf(sbuf, "    <GraphUnit UnitName=\"%s\" UnitAbbrev=\"%s\" UnitScale=\"%.3f\" UnitAbbrev2=\"%s\"/>\n",
        m_cFSR.cGraphUnit.sName, m_cFSR.cGraphUnit.sAbbrev, m_cFSR.cGraphUnit.dBaseToUnit, m_cFSR.cGraphUnit.sAbbrev2);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    g_sprintf(sbuf, "    <Dimensioning ArrowType=\"%d\" ArrowLength=\"%.3f\" ArrowWidth=\"%.3f\" FontAttrs=\"%d\" FontSize=\"%.3f\"\n",
        m_cFSR.iArrowType, m_cFSR.dArrowLen, m_cFSR.dArrowWidth, m_cFSR.bFontAttrs, m_cFSR.dFontSize);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    g_sprintf(sbuf, "      BaseLine=\"%.3f\" FontFace=\"%s\" LengthMask=\"%s\" AngleMask=\"%s\"/>\n  </PageSettings>\n",
        m_cFSR.dBaseLine, m_cFSR.sFontFace, m_cFSR.sLengthMask, m_cFSR.sAngleMask);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    m_pFileSetupDlg->SaveSettings(fp);
    m_pLineStyleDlg->SaveSettings(fp);
    m_pDimEditDlg->SaveSettings(fp);
    m_pStatDlg->SaveSettings(fp);
    m_pSnapDlg->SaveSettings(fp);
    m_pScaleDlg->SaveSettings(fp);

    g_strlcpy(sbuf, "</Settings>", 256);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);

    fclose(fp);
    return;
}

void CDApplication::SetPosition(gint iLeft, gint iTop, gint iWidth, 
    gint iHeight, GdkGravity iGrav)
{
    gtk_window_move(GTK_WINDOW(m_pMainWnd), iLeft, iTop);
    gtk_window_resize(GTK_WINDOW(m_pMainWnd), iWidth, iHeight);
    gtk_window_set_gravity(GTK_WINDOW(m_pMainWnd), iGrav);
    return;
}

void CDApplication::SetFileSetupDlg(gint iLeft, gint iTop)
{
    m_pFileSetupDlg->RestoreSettings(iLeft, iTop);
}

void CDApplication::SetLineStyleDlg(gint iLeft, gint iTop)
{
    m_pLineStyleDlg->RestoreSettings(iLeft, iTop);
}

void CDApplication::SetDimEditDlg(gint iLeft, gint iTop)
{
    m_pDimEditDlg->RestoreSettings(iLeft, iTop);
}

void CDApplication::SetStatDlg(gint iLeft, gint iTop)
{
    m_pStatDlg->RestoreSettings(iLeft, iTop);
}

void CDApplication::SetSnapDlg(gint iLeft, gint iTop)
{
    m_pSnapDlg->RestoreSettings(iLeft, iTop);
}

void CDApplication::SetScaleDlg(gint iLeft, gint iTop)
{
    m_pScaleDlg->RestoreSettings(iLeft, iTop);
}

void CDApplication::SetDrawSettings(gboolean bPaperUnits, gint iLastExportType, gint iDrawGridMode, const gchar *sPath)
{
    m_bPaperUnits = bPaperUnits;
    m_iLastExportType = iLastExportType;
    m_iDrawGridMode = iDrawGridMode;
    if(!sPath) return;

    int iLen = strlen(sPath);
    if(iLen > 0)
    {
        m_sLastPath = (gchar*)g_malloc((iLen + 1)*sizeof(gchar));
        strcpy(m_sLastPath, sPath);
    }
}

gboolean CDApplication::Terminate()
{
    if(!PromptForSave(m_pMainWnd)) return TRUE;

    SaveSettings();
    gtk_main_quit();
    return FALSE;
}

void CDApplication::SetPageSettings(PDFileSetupRec pFSR)
{
    m_cFSR.bPortrait = pFSR->bPortrait;
    m_cFSR.dScaleNomin = pFSR->dScaleNomin;
    m_cFSR.dScaleDenom = pFSR->dScaleDenom;
    m_cFSR.dAngGrid = pFSR->dAngGrid;
    m_cFSR.dXGrid = pFSR->dXGrid;
    m_cFSR.dYGrid = pFSR->dYGrid;
    m_cFSR.dDefLineWidth = pFSR->dDefLineWidth;
}

void CDApplication::SetPaperSize(PDFileSetupRec pFSR)
{
    strcpy(m_cFSR.cPaperSize.sPaperSizeName, pFSR->cPaperSize.sPaperSizeName);
    m_cFSR.cPaperSize.dPaperWidth = pFSR->cPaperSize.dPaperWidth;
    m_cFSR.cPaperSize.dPaperHeight = pFSR->cPaperSize.dPaperHeight;
}

void CDApplication::SetLengthUnit(PDFileSetupRec pFSR)
{
    strcpy(m_cFSR.cLenUnit.sName, pFSR->cLenUnit.sName);
    strcpy(m_cFSR.cLenUnit.sAbbrev, pFSR->cLenUnit.sAbbrev);
    m_cFSR.cLenUnit.dBaseToUnit = pFSR->cLenUnit.dBaseToUnit;
    strcpy(m_cFSR.cLenUnit.sAbbrev2, pFSR->cLenUnit.sAbbrev2);
}

void CDApplication::SetAngularUnit(PDFileSetupRec pFSR)
{
    strcpy(m_cFSR.cAngUnit.sName, pFSR->cAngUnit.sName);
    strcpy(m_cFSR.cAngUnit.sAbbrev, pFSR->cAngUnit.sAbbrev);
    m_cFSR.cAngUnit.dBaseToUnit = pFSR->cAngUnit.dBaseToUnit;
    strcpy(m_cFSR.cAngUnit.sAbbrev2, pFSR->cAngUnit.sAbbrev2);
}

void CDApplication::SetPaperUnit(PDFileSetupRec pFSR)
{
    strcpy(m_cFSR.cPaperUnit.sName, pFSR->cPaperUnit.sName);
    strcpy(m_cFSR.cPaperUnit.sAbbrev, pFSR->cPaperUnit.sAbbrev);
    m_cFSR.cPaperUnit.dBaseToUnit = pFSR->cPaperUnit.dBaseToUnit;
    strcpy(m_cFSR.cPaperUnit.sAbbrev2, pFSR->cPaperUnit.sAbbrev2);
}

void CDApplication::SetGraphUnit(PDFileSetupRec pFSR)
{
    strcpy(m_cFSR.cGraphUnit.sName, pFSR->cGraphUnit.sName);
    strcpy(m_cFSR.cGraphUnit.sAbbrev, pFSR->cGraphUnit.sAbbrev);
    m_cFSR.cGraphUnit.dBaseToUnit = pFSR->cGraphUnit.dBaseToUnit;
    strcpy(m_cFSR.cGraphUnit.sAbbrev2, pFSR->cGraphUnit.sAbbrev2);
}

void CDApplication::SetDimensioning(PDFileSetupRec pFSR)
{
    m_cFSR.iArrowType = pFSR->iArrowType;
    m_cFSR.dArrowLen = pFSR->dArrowLen;
    m_cFSR.dArrowWidth = pFSR->dArrowWidth;
    m_cFSR.bFontAttrs = pFSR->bFontAttrs;
    m_cFSR.dFontSize = pFSR->dFontSize;
    m_cFSR.dBaseLine = pFSR->dBaseLine;
    strcpy(m_cFSR.sFontFace, pFSR->sFontFace);
    strcpy(m_cFSR.sLengthMask, pFSR->sLengthMask);
    strcpy(m_cFSR.sAngleMask, pFSR->sAngleMask);
}

GtkWidget* CDApplication::GetMenuBar()
{
    GtkBox *wBox = (GtkBox*)gtk_bin_get_child(GTK_BIN(m_pMainWnd));
    GList *pChilds = gtk_container_get_children(GTK_CONTAINER(wBox));
    GtkWidget *pane = (GtkWidget*)g_list_nth(pChilds, 0)->data;
    return(pane);
}

GtkWidget* CDApplication::GetDrawing()
{
    GtkBox *wBox = (GtkBox*)gtk_bin_get_child(GTK_BIN(m_pMainWnd));
    GList *pChilds = gtk_container_get_children(GTK_CONTAINER(wBox));
    GtkWidget *pane = (GtkWidget*)g_list_nth(pChilds, 1)->data;
    return(pane);
}

GtkWidget* CDApplication::GetStatusBar()
{
    GtkBox *wBox = (GtkBox*)gtk_bin_get_child(GTK_BIN(m_pMainWnd));
    GList *pChilds = gtk_container_get_children(GTK_CONTAINER(wBox));
    GtkWidget *pane = (GtkWidget*)g_list_nth(pChilds, 2)->data;
    return(pane);
}

void CDApplication::SetStatusBarMsg(int iPanel, const gchar *pMsg)
{
    GtkStatusbar *pStatBar = (GtkStatusbar*)GetStatusBar();
    switch(iPanel)
    {
    case 0:
        gtk_statusbar_pop(pStatBar, 0);
        gtk_statusbar_push(pStatBar, 0, pMsg);
        break;
    case 1:
        gtk_label_set_text(GTK_LABEL(m_pStatLab1), pMsg);
        break;
    case 2:
        gtk_label_set_text(GTK_LABEL(m_pStatLab2), pMsg);
        break;
    case 3:
        gtk_label_set_text(GTK_LABEL(m_pStatLab3), pMsg);
        break;
    }
    return;
}

void CDApplication::Configure(GtkWidget *widget, GdkEventConfigure *event)
{
    if(m_pcp) cairo_pattern_destroy(m_pcp);
    m_pcp = NULL;
    if(m_pcs) cairo_surface_destroy(m_pcs);
    m_pcs = NULL;
}

void CDApplication::SetLColor(cairo_t *cr, long lColor)
{
    cairo_set_source_rgb(cr, (lColor & 0xFF)/255.0,
        ((lColor >> 8) & 0xFF)/255.0, ((lColor >> 16) & 0xFF)/255.0);
}

void CDApplication::SetLColorA(cairo_t *cr, long lColor, int iAlpha)
{
    cairo_set_source_rgba(cr, (lColor & 0xFF)/255.0,
        ((lColor >> 8) & 0xFF)/255.0, ((lColor >> 16) & 0xFF)/255.0, iAlpha/100.0);
}

void CDApplication::DrawDimArrow(cairo_t *cr, PDPrimitive pPrim)
{
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    CDPoint cStartPt, cEndPt;
    int iType = Round(pPrim->cPt1.x);

    switch(iType)
    {
    case 1:
        cairo_new_path(cr);
        cStartPt.x = pPrim->cPt3.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt3.y + m_cViewOrigin.y;
        cairo_move_to(cr, cStartPt.x, cStartPt.y);
        cEndPt.x = pPrim->cPt2.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt2.y + m_cViewOrigin.y;
        cairo_line_to(cr, cEndPt.x, cEndPt.y);
        cEndPt.x = pPrim->cPt4.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt4.y + m_cViewOrigin.y;
        cairo_line_to(cr, cEndPt.x, cEndPt.y);
        cairo_stroke(cr);
        break;
    case 2:
        cairo_new_path(cr);
        cStartPt.x = pPrim->cPt3.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt3.y + m_cViewOrigin.y;
        cairo_move_to(cr, cStartPt.x, cStartPt.y);
        cEndPt.x = pPrim->cPt2.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt2.y + m_cViewOrigin.y;
        cairo_line_to(cr, cEndPt.x, cEndPt.y);
        cEndPt.x = pPrim->cPt4.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt4.y + m_cViewOrigin.y;
        cairo_line_to(cr, cEndPt.x, cEndPt.y);
        cairo_line_to(cr, cStartPt.x, cStartPt.y);
        cairo_fill(cr);
        break;
    case 3:
        cairo_new_path(cr);
        cairo_arc(cr, pPrim->cPt2.x + m_cViewOrigin.x, pPrim->cPt2.y + m_cViewOrigin.y,
            fabs(pPrim->cPt3.x - pPrim->cPt2.x), 0.0, 2.0*M_PI);
        cairo_fill(cr);
        break;
    case 4:
    case 5:
        cairo_new_path(cr);
        cStartPt.x = pPrim->cPt3.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt3.y + m_cViewOrigin.y;
        cairo_move_to(cr, cStartPt.x, cStartPt.y);
        cEndPt.x = pPrim->cPt4.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt4.y + m_cViewOrigin.y;
        cairo_line_to(cr, cEndPt.x, cEndPt.y);
        cairo_stroke(cr);
        break;
    }
}

void CDApplication::DrawDimText(cairo_t *cr, PDPrimitive pPrim, PDObject pObj, long dwColor,
    double dLineWidth)
{
    int iPos = Round(pPrim->cPt2.y);

    PDUnitList pUnits = m_pFileSetupDlg->GetUnitList();

    char sBuf[64];
    char *psBuf = sBuf;
    int iLen = pObj->PreParseDimText(iPos, psBuf, 64, m_dDrawScale, pUnits);
    if(iLen > 0)
    {
        psBuf = (char*)g_malloc(iLen*sizeof(char));
        pObj->PreParseDimText(iPos, psBuf, iLen, m_dDrawScale, pUnits);
    }

    int iLen2 = strlen(psBuf);

    cairo_translate(cr, m_cViewOrigin.x + pPrim->cPt1.x, m_cViewOrigin.y + pPrim->cPt1.y);
    double dPi2 = M_PI/2.0;
    cairo_rotate(cr, pPrim->cPt2.x - dPi2);

    SetLColor(cr, dwColor);

    CDFileAttrs cFileAttrs;
    if(iPos < 0) m_pDrawObjects->GetFileAttrs(&cFileAttrs);
    else pObj->GetDimFontAttrs(iPos, &cFileAttrs);

    double da = 0.4*m_dUnitScale*cFileAttrs.dFontSize;

    bool bDiam = (psBuf[0] == '*');
    int iStart = 0;
    if(bDiam) iStart = 1;
    if(iStart >= iLen2)
    {
        psBuf[iStart] = '?';
        psBuf[iStart + 1] = 0;
        iLen2++;
    }

    char *sBufStart = &psBuf[iStart];
    char *sFrac = strchr(sBufStart, '_');
    char sBufNom[4];
    char sBufDenom[4];
    char *sBufEnd = NULL;

    if(sFrac)
    {
        sBufEnd = sFrac + 1;
        int i = 0;
        while((*sBufEnd >= '0') && (*sBufEnd <= '9') && (i < 3))
        {
            sBufNom[i++] = *(sBufEnd++);
        }
        sBufNom[i] = 0;

        while(*sBufEnd && (*sBufEnd != '/')) sBufEnd++;
        if(*sBufEnd) sBufEnd++;
        i = 0;
        while((*sBufEnd >= '0') && (*sBufEnd <= '9') && (i < 3))
        {
            sBufDenom[i++] = *(sBufEnd++);
        }
        sBufDenom[i] = 0;
        *sFrac = 0;
    }

    cairo_font_slant_t iSlant = CAIRO_FONT_SLANT_NORMAL;
    if(cFileAttrs.bFontAttrs & 1) iSlant = CAIRO_FONT_SLANT_ITALIC;

    cairo_font_weight_t iWeight = CAIRO_FONT_WEIGHT_NORMAL;
    if(cFileAttrs.bFontAttrs & 8) iWeight = CAIRO_FONT_WEIGHT_BOLD;

    cairo_select_font_face(cr, cFileAttrs.sFontFace, iSlant, iWeight);
    cairo_set_font_size(cr, 1.6*m_dUnitScale*cFileAttrs.dFontSize);

    cairo_text_extents_t ctexStart, ctexNom, ctexDenom, ctexEnd;
    cairo_text_extents(cr, sBufStart, &ctexStart);

    double dTextWidth = ctexStart.width + ctexStart.x_bearing;

    if(sFrac)
    {
        cairo_text_extents(cr, sBufEnd, &ctexEnd);
        cairo_set_font_size(cr, 0.96*m_dUnitScale*cFileAttrs.dFontSize);
        cairo_text_extents(cr, sBufNom, &ctexNom);
        cairo_text_extents(cr, sBufDenom, &ctexDenom);

        dTextWidth += (ctexNom.width + ctexNom.x_bearing);
        dTextWidth += (ctexDenom.width + ctexDenom.x_bearing);
        dTextWidth += (ctexEnd.width + ctexEnd.x_bearing);
        dTextWidth += 0.26*da;
    }

    cairo_set_font_size(cr, 1.6*m_dUnitScale*cFileAttrs.dFontSize);

    double dx = -dTextWidth/2.0;
    if(bDiam)
    {
        dTextWidth += 2.5*da;
        dx = da - dTextWidth/2.0;
        cairo_new_path(cr);
        cairo_arc(cr, dx, -1.3*da, da, 0.0, 2*M_PI);
        cairo_move_to(cr, dx - da, -0.3*da);
        cairo_line_to(cr, dx + da, -2.3*da);
        cairo_stroke(cr);
        dx += 1.5*da;
    }
    cairo_move_to(cr, dx, 0.0);
    cairo_show_text(cr, sBufStart);

    PDDimension pDim = pObj->GetDimen(iPos);
    pDim->cExt.cPt1.x = -dTextWidth/2.0/m_dUnitScale;
    pDim->cExt.cPt1.y = 0.0;
    pDim->cExt.cPt2.x = dTextWidth/2.0/m_dUnitScale;
    pDim->cExt.cPt2.y = 1.15*cFileAttrs.dFontSize;

    if(sFrac)
    {
        dx += (ctexStart.width + ctexStart.x_bearing + 0.08*da);
        cairo_set_font_size(cr, 0.96*m_dUnitScale*cFileAttrs.dFontSize);
        cairo_move_to(cr, dx, -1.7*da);
        cairo_show_text(cr, sBufNom);

        dx += (ctexNom.width + ctexNom.x_bearing + 0.12*da);
        cairo_move_to(cr, dx, 0.4*da);
        cairo_show_text(cr, sBufDenom);

        cairo_move_to(cr, dx - 0.7*da, -0.8*da);
        cairo_line_to(cr, dx + 0.7*da, -2.2*da);
        cairo_stroke(cr);

        dx += (ctexDenom.width + ctexDenom.x_bearing + 0.06*da);
        cairo_set_font_size(cr, 1.6*m_dUnitScale*cFileAttrs.dFontSize);
        cairo_move_to(cr, dx, 0.0);
        cairo_show_text(cr, sBufEnd);
    }

    cairo_identity_matrix(cr);

    if(iLen > 0) g_free(psBuf);

    /*CDPoint cStartPt, cEndPt;
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

    if(iLen > 0) free(psBuf);*/
}

void CDApplication::DrawPrimitive(cairo_t *cr, PDPrimitive pPrim)
{
    double dr, da1, da2;

    CDPoint cStartPt, cEndPt;

    switch(pPrim->iType)
    {
    case 1:
        cairo_new_path(cr);
        cStartPt.x = pPrim->cPt1.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt1.y + m_cViewOrigin.y;
        cairo_move_to(cr, cStartPt.x, cStartPt.y);
        cEndPt.x = pPrim->cPt2.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt2.y + m_cViewOrigin.y;
        cairo_line_to(cr, cEndPt.x, cEndPt.y);
        cairo_stroke(cr);
        break;
    case 2:
        cStartPt.x = pPrim->cPt3.x + m_cViewOrigin.x;
        cStartPt.y = pPrim->cPt3.y + m_cViewOrigin.y;
        cEndPt.x = pPrim->cPt4.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt4.y + m_cViewOrigin.y;

        dr = GetDist(cStartPt, cEndPt);
        if(dr > 2)
        {
            dr = (pPrim->cPt2.x - pPrim->cPt1.x);
            da2 = atan2(pPrim->cPt3.y - pPrim->cPt1.y, pPrim->cPt3.x - pPrim->cPt1.x);
            da1 = atan2(pPrim->cPt4.y - pPrim->cPt1.y, pPrim->cPt4.x - pPrim->cPt1.x);
            cairo_new_path(cr);
            cairo_arc(cr, m_cViewOrigin.x + pPrim->cPt1.x, m_cViewOrigin.y + pPrim->cPt1.y, dr, da1, da2);
            cairo_stroke(cr);
        }
        else
        {
            cairo_new_path(cr);
            cairo_move_to(cr, cStartPt.x, cStartPt.y);
            cairo_line_to(cr, cEndPt.x, cEndPt.y);
            cairo_stroke(cr);
        }
        break;
    case 3:
        dr = (pPrim->cPt2.x - pPrim->cPt1.x);
        cairo_new_path(cr);
        cairo_arc(cr, m_cViewOrigin.x + pPrim->cPt1.x, m_cViewOrigin.y + pPrim->cPt1.y, dr, 0.0, 2.0*M_PI);
        cairo_stroke(cr);
        break;
    case 4:
        cStartPt = (pPrim->cPt1 + 2.0*pPrim->cPt2)/3.0;
        cEndPt = (pPrim->cPt3 + 2.0*pPrim->cPt2)/3.0;
        cairo_new_path(cr);
        cairo_move_to(cr, m_cViewOrigin.x + pPrim->cPt1.x, m_cViewOrigin.y + pPrim->cPt1.y);
        cairo_curve_to(cr, m_cViewOrigin.x + cStartPt.x, m_cViewOrigin.y + cStartPt.y,
            m_cViewOrigin.x + cEndPt.x, m_cViewOrigin.y + cEndPt.y,
            m_cViewOrigin.x + pPrim->cPt3.x, m_cViewOrigin.y + pPrim->cPt3.y);
        cairo_stroke(cr);
        break;
    case 5:
        cairo_new_path(cr);
        cairo_move_to(cr, m_cViewOrigin.x + pPrim->cPt1.x, m_cViewOrigin.y + pPrim->cPt1.y);
        cairo_curve_to(cr, m_cViewOrigin.x + pPrim->cPt2.x, m_cViewOrigin.y + pPrim->cPt2.y,
            m_cViewOrigin.x + pPrim->cPt3.x, m_cViewOrigin.y + pPrim->cPt3.y,
            m_cViewOrigin.x + pPrim->cPt4.x, m_cViewOrigin.y + pPrim->cPt4.y);
        cairo_stroke(cr);
        break;
    case 7:
        cairo_new_path(cr);
        cStartPt.x = pPrim->cPt1.x + m_cViewOrigin.x - 6.0;
        cStartPt.y = pPrim->cPt1.y + m_cViewOrigin.y;
        cairo_move_to(cr, cStartPt.x, cStartPt.y);
        cairo_line_to(cr, cStartPt.x + 12.0, cStartPt.y);
        cEndPt.x = pPrim->cPt1.x + m_cViewOrigin.x;
        cEndPt.y = pPrim->cPt1.y + m_cViewOrigin.y + 6.0;
        cairo_move_to(cr, cEndPt.x, cEndPt.y - 12.0);
        cairo_line_to(cr, cEndPt.x, cEndPt.y);
        cairo_stroke(cr);
        break;
    case 9:
        DrawDimArrow(cr, pPrim);
        break;
    }
}

void CDApplication::DrawObject(cairo_t *cr, PDObject pObj, int iMode, int iDimen)
{
    bool bSel = pObj->GetSelected();
    CDLineStyle cStyle = pObj->GetLineStyle();

    long dwColor = 0;
    if(iMode == 1) dwColor = m_lActiveColor;
    else if(iMode == 2) dwColor = m_lHighColor;
    else if(bSel) dwColor = m_lSelColor;

    double dw1 = fabs(cStyle.dWidth);
    double dWidth;
    if(dw1 < g_dPrec) dWidth = 1.0;
    else dWidth = dw1*m_dUnitScale;
    double dPtRad = dWidth;
    if(dPtRad < 2.0) dPtRad = 2.0;

    cairo_set_line_width(cr, dWidth);
    if(fabs(fabs(cStyle.dWidth)) < 0.3) cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    else cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    SetLColor(cr, dwColor);

    CDPrimitive cPrim;
    PDDimension pDim;
    pObj->GetFirstPrimitive(&cPrim, -m_dUnitScale, iDimen);

    if(iDimen < -1)
    {
        while(cPrim.iType > 0)
        {
            if(cPrim.iType == 6)
            {
                cairo_set_line_width(cr, 0.7);
                cairo_new_path(cr);
                cairo_arc(cr, cPrim.cPt1.x + m_cViewOrigin.x, cPrim.cPt1.y + m_cViewOrigin.y, dPtRad, 0.0, 2.0*M_PI);
                cairo_stroke(cr);
                cairo_fill(cr);
                cairo_set_line_width(cr, dWidth);
            }
            else if(cPrim.iType == 7)
            {
                cairo_set_line_width(cr, 0.7);
                if(iMode == 0) cairo_set_source_rgb(cr, 0.53, 0.53, 0.53);
                DrawPrimitive(cr, &cPrim);
                SetLColor(cr, dwColor);
                cairo_set_line_width(cr, dWidth);
            }
            else if(cPrim.iType == 8)
            {
                cairo_set_line_width(cr, 0.7);
                cairo_new_path(cr);
                cairo_rectangle(cr, cPrim.cPt1.x + m_cViewOrigin.x - dPtRad,
                    cPrim.cPt1.y + m_cViewOrigin.y - dPtRad, 2.0*dPtRad, 2.0*dPtRad);
                cairo_stroke(cr);
                cairo_fill(cr);
                cairo_set_line_width(cr, dWidth);
            }
            else if(cPrim.iType == 10)
            {
                DrawDimText(cr, &cPrim, pObj, dwColor, fabs(cStyle.dWidth));
            }
            else DrawPrimitive(cr, &cPrim);
            pObj->GetNextPrimitive(&cPrim, -m_dUnitScale, iDimen);
        }

        if(iMode == 0)
        {
            for(int i = 0; i < pObj->GetDimenCount(); i++)
            {
                pDim = pObj->GetDimen(i);
                if(pDim->bSelected)
                {
                    dwColor = m_lSelColor;
                    SetLColor(cr, dwColor);
                }
                else
                {
                    dwColor = 0;
                    SetLColor(cr, dwColor);
                }

                pObj->GetFirstPrimitive(&cPrim, -m_dUnitScale, i);
                while(cPrim.iType > 0)
                {
                    if(cPrim.iType == 10)
                    {
                        DrawDimText(cr, &cPrim, pObj, dwColor, fabs(cStyle.dWidth));
                    }
                    else DrawPrimitive(cr, &cPrim);
                    pObj->GetNextPrimitive(&cPrim, -m_dUnitScale, i);
                }
            }
        }
    }
    else
    {
        if((iMode < 1) && (iDimen > -1))
        {
            pDim = pObj->GetDimen(iDimen);
            if(pDim->bSelected) dwColor = m_lSelColor;
            else dwColor = 0;
            SetLColor(cr, dwColor);
        }

        while(cPrim.iType > 0)
        {
            if(cPrim.iType == 10)
            {
                DrawDimText(cr, &cPrim, pObj, dwColor, fabs(cStyle.dWidth));
            }
            else DrawPrimitive(cr, &cPrim);
            pObj->GetNextPrimitive(&cPrim, -m_dUnitScale, iDimen);
        }
    }
}

void CDApplication::Paint(GtkWidget *widget, GdkEventExpose *event)
{
    int iWidth = gdk_window_get_width(event->window);
    int iHeight = gdk_window_get_height(event->window);

    gboolean bNotWholeWindow = (event->area.x > 0) || (event->area.y > 0) ||
        (event->area.width < iWidth) || (event->area.height < iHeight);
    gboolean bOffLine = !(m_bRenderDirect || bNotWholeWindow);

    cairo_t *cr2 = gdk_cairo_create(event->window);
    //cairo_set_antialias(cr, CAIRO_ANTIALIAS_FAST);
    if(!m_pcs && bOffLine)
    {
        m_pcs = cairo_surface_create_similar_image(cairo_get_target(cr2),
            CAIRO_FORMAT_RGB24, iWidth, iHeight);
        m_pcp = cairo_pattern_create_for_surface(m_pcs);
    }

    //if(bNotWholeWindow && m_pcp)
    //{
    //    cairo_pattern_destroy(m_pcp);
    //    m_pcp = NULL;
    //    if(m_pcs) m_pcp = cairo_pattern_create_for_surface(m_pcs);
    //}

    cairo_t *cr = cr2;
    //if(bOffLine)
    if(!m_bRenderDirect)
        cr = cairo_create(m_pcs);

    cairo_identity_matrix(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_rectangle(cr, event->area.x, event->area.y, event->area.width, event->area.height);
    cairo_fill(cr);

    cairo_set_line_width(cr, 1.0);

    if(m_iDrawGridMode > 0)
    {
        double dx = m_dUnitScale*m_cFSR.dXGrid;
        double dy = m_dUnitScale*m_cFSR.dYGrid;
        if((dx > 5) && (dy > 5))
        {
            int iMin = -m_cViewOrigin.x/m_dUnitScale/m_cFSR.dXGrid;
            int iMax = (iWidth - m_cViewOrigin.x)/m_dUnitScale/m_cFSR.dXGrid;
            int jMin = -m_cViewOrigin.y/m_dUnitScale/m_cFSR.dYGrid;
            int jMax = (iHeight - m_cViewOrigin.y)/m_dUnitScale/m_cFSR.dYGrid;

            double dGray;

            if(m_iDrawGridMode & 2)
            {
                if((dx < 200) || (dy < 200))
                {
                    if(dx < dy) dGray = 0.7 + 0.2*(200.0 - dx)/195.0;
                    else dGray = 0.7 + 0.2*(200.0 - dy)/195.0;
                }
                else dGray = 0.7;

                cairo_set_source_rgb(cr, dGray, dGray, dGray);
                cairo_new_path(cr);
                for(int i = iMin; i <= iMax; i++)
                {
                    dx = m_cViewOrigin.x + (double)i*m_dUnitScale*m_cFSR.dXGrid;
                    cairo_move_to(cr, dx, 0.0);
                    cairo_line_to(cr, dx, iHeight);
                }
                for(int j = jMin; j <= jMax; j++)
                {
                    dy = m_cViewOrigin.y + (double)j*m_dUnitScale*m_cFSR.dYGrid;
                    cairo_move_to(cr, 0.0, dy);
                    cairo_line_to(cr, iWidth, dy);
                }
                cairo_stroke(cr);
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

                cairo_set_source_rgb(cr, dGray, dGray, dGray);
                for(int i = iMin; i <= iMax; i++)
                {
                    dx = m_cViewOrigin.x + (double)i*m_dUnitScale*m_cFSR.dXGrid;
                    for(int j = jMin; j <= jMax; j++)
                    {
                        dy = m_cViewOrigin.y + (double)j*m_dUnitScale*m_cFSR.dYGrid;
                        cairo_arc(cr, dx, dy, 1.0, 0.0, 2.0*M_PI);
                        cairo_fill(cr);
                    }
                }
            }
        }
    }

    cairo_set_source_rgb(cr, 0.5, 0.3, 0.0);
    cairo_new_path(cr);
    cairo_rectangle(cr, m_cViewOrigin.x, m_cViewOrigin.y, m_dUnitScale*m_dwPage, m_dUnitScale*m_dhPage);
    cairo_stroke(cr);

    CDRect cdr;
    cdr.cPt1.x = (event->area.x - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt1.y = (event->area.y - m_cViewOrigin.y)/m_dUnitScale;
    cdr.cPt2.x = (event->area.x + event->area.width - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (event->area.y + event->area.height - m_cViewOrigin.y)/m_dUnitScale;

    m_pDrawObjects->BuildAllPrimitives(&cdr);

    PDObject pObj;
    int iObjs = m_pDrawObjects->GetCount();
    for(int i = 0; i < iObjs; i++)
    {
        pObj = m_pDrawObjects->GetItem(i);
        DrawObject(cr, pObj, 0, -2);
    }

    if(m_pHighObject) DrawObject(cr, m_pHighObject, 2, m_iHighDimen);

    //if(bOffLine)
    if(!m_bRenderDirect)
    {
        cairo_destroy(cr);
        cairo_set_source_surface(cr2, m_pcs, 0, 0);
        cairo_paint(cr2);
    }

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
        if(m_pActiveObject)
        {
            m_pActiveObject->BuildPrimitives(cPtX, iDynMode, &cdr, false, NULL);
            DrawObject(cr2, m_pActiveObject, 1, -2);
        }

        cairo_identity_matrix(cr2);
        DrawCross(cr2);
    }

    cairo_destroy(cr2);

    if(!m_bRenderDirect)
    {
        if(bNotWholeWindow)
        {
            cdr.cPt1.x = -m_cViewOrigin.x/m_dUnitScale;
            cdr.cPt1.y = -m_cViewOrigin.y/m_dUnitScale;
            cdr.cPt2.x = (iWidth - m_cViewOrigin.x)/m_dUnitScale;
            cdr.cPt2.y = (iHeight - m_cViewOrigin.y)/m_dUnitScale;

            m_pDrawObjects->BuildAllPrimitives(&cdr);
            if(m_pActiveObject)
            {
                m_pActiveObject->BuildPrimitives(cPtX, iDynMode, &cdr, false, NULL);
            }
        }
    }

    return;
}

bool CDApplication::PromptForSave(GtkWidget *widget)
{
    if(!m_pDrawObjects->GetChanged()) return true;

    GtkWidget *msg_dlg = gtk_message_dialog_new(GTK_WINDOW(widget), GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE, _("The file has been changed. Do you want to save it?"));
    gtk_dialog_add_buttons(GTK_DIALOG(msg_dlg), GTK_STOCK_YES, GTK_RESPONSE_YES,
        GTK_STOCK_NO, GTK_RESPONSE_NO, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    gint iRes = gtk_dialog_run(GTK_DIALOG(msg_dlg));
    gtk_widget_destroy(msg_dlg);

    if(iRes == GTK_RESPONSE_CANCEL) return false;
    if(iRes == GTK_RESPONSE_NO) return true;

    return SaveFile(widget, &m_sFileName, false);
}

bool CDApplication::SaveFile(GtkWidget *widget, gchar **psFile, bool bSelectOnly)
{
    bool bSave = true;
    bool bNewFile = false;
    if(!(*psFile))
    {
        bNewFile = true;

        //GError *error = NULL;
        GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Save File"),
        GTK_WINDOW(widget), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
        g_object_set(dialog, "do_overwrite_confirmation", TRUE, NULL);

        GtkFileFilter *flt = gtk_file_filter_new();
        gtk_file_filter_set_name(flt, _("SteamCAD Files"));
        gtk_file_filter_add_pattern(flt, "*.sdr");
        gtk_file_chooser_add_filter((GtkFileChooser*)dialog, flt);
        flt = gtk_file_filter_new();
        gtk_file_filter_set_name(flt, _("All files"));
        gtk_file_filter_add_pattern(flt, "*");
        gtk_file_chooser_add_filter((GtkFileChooser*)dialog, flt);

        if(m_sLastPath)
            gtk_file_chooser_set_current_folder((GtkFileChooser*)dialog, m_sLastPath);

        if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
        {
            if(m_sLastPath) g_free(m_sLastPath);
            gchar *sLocFileName = gtk_file_chooser_get_filename((GtkFileChooser*)dialog);
            gchar *sDot = strrchr(sLocFileName, '.');
            if(!sDot)
            {
                gint iLen = strlen(sLocFileName);
                *psFile = (gchar*)g_malloc((iLen + 5)*sizeof(gchar));
                strcpy(*psFile, sLocFileName);
                strcat(*psFile, ".sdr");
                g_free(sLocFileName);
            }
            else *psFile = sLocFileName;
            m_sLastPath = gtk_file_chooser_get_current_folder((GtkFileChooser*)dialog);
        }
        else bSave = false;
        gtk_widget_destroy(dialog);
    }
    if(!bSave) return false;

    // save the file
    FILE *pf = fopen(*psFile, "wb");
    m_pDrawObjects->SaveToFile(pf, true, bSelectOnly);
    fclose(pf);

    if(!bNewFile) SetTitle(widget, true);
    return true;
}

void CDApplication::SetTitle(GtkWidget *widget, bool bForce)
{
    bool bNewHasChanged = m_pDrawObjects->GetChanged();
    if((m_bHasChanged == bNewHasChanged) && !bForce) return;

    m_bHasChanged = bNewHasChanged;

    int iLen = strlen("SteamCAD - ");
    gchar *sFileName = NULL;
    if(m_sFileName)
    {
        sFileName = strrchr(m_sFileName, '\\');
        if(sFileName) sFileName++;
        else sFileName = m_sFileName;
        iLen += strlen(sFileName);
    }
    else iLen += strlen("new file");
    if(m_bHasChanged) iLen++;

    gchar *sCap = (gchar*)g_malloc((iLen + 1)*sizeof(gchar));
    strcpy(sCap, "SteamCAD - ");
    if(sFileName) strcat(sCap, sFileName);
    else strcat(sCap, "new file");
    if(m_bHasChanged) strcat(sCap, "*");

    gtk_window_set_title(GTK_WINDOW(widget), sCap);
    g_free(sCap);
}

void CDApplication::FileNewCmd(bool bFromAccel)
{
    if(!PromptForSave(m_pMainWnd)) return;

    if(m_pActiveObject) delete m_pActiveObject;
    m_pActiveObject = NULL;
    m_pHighObject = NULL;
    m_pDrawObjects->ClearAll();
    m_pUndoObjects->ClearAll();
    if(m_sFileName) g_free(m_sFileName);
    m_sFileName = NULL;
    m_iRedoCount = 0;

    CDFileAttrs cFAttrs;
    FilePropsToData(&cFAttrs);
    m_pDrawObjects->SetFileAttrs(&cFAttrs, true);

    gdk_window_invalidate_rect(GetDrawing()->window, NULL, FALSE);
    SetTitle(m_pMainWnd, true);
    return;
}

void CDApplication::FilePropsToData(PDFileAttrs pFAttrs)
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
    strcpy(pFAttrs->sFontFace, m_cFSR.sFontFace);
    strcpy(pFAttrs->sLengthMask, m_cFSR.sLengthMask);
    strcpy(pFAttrs->sAngleMask, m_cFSR.sAngleMask);
}

void CDApplication::DataToFileProps()
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
    if(pSize) strcpy(m_cFSR.cPaperSize.sPaperSizeName, pSize->sPaperSizeName);

    m_cFSR.dScaleNomin = cFAttrs.dScaleNom;
    m_cFSR.dScaleDenom = cFAttrs.dScaleDenom;

    if(fabs(m_cFSR.dScaleDenom) > g_dPrec)
        m_dDrawScale = m_cFSR.dScaleNomin/m_cFSR.dScaleDenom;
    else m_dDrawScale = 1.0;
}

void CDApplication::GetPageDims()
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

void CDApplication::FileCommand(int iCmd, bool bFromAccel)
{
    switch(iCmd)
    {
    case IDM_FILENEW:
        FileNewCmd(bFromAccel);
        break;
    case IDM_FILEOPEN:
        FileOpenCmd(bFromAccel);
        break;
    case IDM_FILESAVE:
        FileSaveCmd(bFromAccel);
        break;
    case IDM_FILESAVEAS:
        FileSaveAsCmd(bFromAccel);
        break;
    case IDM_FILESAVESEL:
        FileSaveSelCmd(bFromAccel);
        break;
    case IDM_FILEINCLUDE:
        FileIncludeCmd(bFromAccel);
        break;
    case IDM_FILEEXPORT:
        FileExportCmd(bFromAccel);
        break;
    case IDM_FILEPROPS:
        FilePropsCmd(bFromAccel);
        break;
    case IDM_FILEEXIT:
        FileQuitCmd(bFromAccel);
        break;
    }
    return;
}

bool CDApplication::LoadFile(GtkWidget *widget, gchar **psFile, bool bClear)
{
    bool bRead = false;

    GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Open File"),
    GTK_WINDOW(widget), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
    GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    GtkFileFilter *flt = gtk_file_filter_new();
    gtk_file_filter_set_name(flt, _("SteamCAD Files"));
    gtk_file_filter_add_pattern(flt, "*.sdr");
    gtk_file_chooser_add_filter((GtkFileChooser*)dialog, flt);
    flt = gtk_file_filter_new();
    gtk_file_filter_set_name(flt, _("All files"));
    gtk_file_filter_add_pattern(flt, "*");
    gtk_file_chooser_add_filter((GtkFileChooser*)dialog, flt);
    if(m_sLastPath)
        gtk_file_chooser_set_current_folder((GtkFileChooser*)dialog, m_sLastPath);

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        if(m_sLastPath) g_free(m_sLastPath);
        if(*psFile) g_free(*psFile);
        *psFile = gtk_file_chooser_get_filename((GtkFileChooser*)dialog);
        m_sLastPath = gtk_file_chooser_get_current_folder((GtkFileChooser*)dialog);

        // load the file
        FILE *pf = fopen(*psFile, "rb");
        bRead = m_pDrawObjects->ReadFromFile(pf, true, bClear);
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

            GtkWidget *draw = GetDrawing();
            gdk_window_invalidate_rect(draw->window, NULL, FALSE);
            SetTitle(widget, true);
        }
    }

    gtk_widget_destroy(dialog);
    return bRead;
}

void CDApplication::FileOpenCmd(bool bFromAccel)
{
    if(!PromptForSave(m_pMainWnd)) return;

    LoadFile(m_pMainWnd, &m_sFileName, true);
    return;
}

void CDApplication::FileSaveCmd(bool bFromAccel)
{
    SaveFile(m_pMainWnd, &m_sFileName, false);
    return;
}

void CDApplication::FileSaveAsCmd(bool bFromAccel)
{
    gchar *sNewName = NULL;
    if(SaveFile(m_pMainWnd, &sNewName, false))
    {
        if(m_sFileName) g_free(m_sFileName);
        m_sFileName = sNewName;
        SetTitle(m_pMainWnd, true);
    }
    return;
}

void CDApplication::FileSaveSelCmd(bool bFromAccel)
{
    gchar *sNewName = NULL;
    if(SaveFile(m_pMainWnd, &sNewName, true))
    {
        g_free(sNewName);
    }
    return;
}

void CDApplication::FileIncludeCmd(bool bFromAccel)
{
    gchar *sNewName = NULL;
    if(LoadFile(m_pMainWnd, &sNewName, false)) g_free(sNewName);
    return;
}

void CDApplication::FileExportCmd(bool bFromAccel)
{
    bool bSave = true;

    //GError *error = NULL;
    GtkWidget *dialog = gtk_file_chooser_dialog_new(_("Save File"),
        GTK_WINDOW(m_pMainWnd), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    g_object_set(dialog, "do_overwrite_confirmation", TRUE, NULL);

    GtkFileFilter *flt = gtk_file_filter_new();
    gtk_file_filter_set_name(flt, _("Adobe Acrobat (*.pdf)"));
    gtk_file_filter_add_pattern(flt, "*.pdf");
    gtk_file_chooser_add_filter((GtkFileChooser*)dialog, flt);
    GtkFileFilter *pActFilter = flt;

    flt = gtk_file_filter_new();
    gtk_file_filter_set_name(flt, _("PostScript (*.ps)"));
    gtk_file_filter_add_pattern(flt, "*.ps");
    gtk_file_chooser_add_filter((GtkFileChooser*)dialog, flt);
    if(m_iLastExportType == 1) pActFilter = flt;

    flt = gtk_file_filter_new();
    gtk_file_filter_set_name(flt, _("Encapsulated PostScript (*.eps)"));
    gtk_file_filter_add_pattern(flt, "*.eps");
    gtk_file_chooser_add_filter((GtkFileChooser*)dialog, flt);
    if(m_iLastExportType == 2) pActFilter = flt;

    flt = gtk_file_filter_new();
    gtk_file_filter_set_name(flt, _("PNG Image (*.png)"));
    gtk_file_filter_add_pattern(flt, "*.png");
    gtk_file_chooser_add_filter((GtkFileChooser*)dialog, flt);
    if(m_iLastExportType == 3) pActFilter = flt;

    flt = gtk_file_filter_new();
    gtk_file_filter_set_name(flt, _("Scalable Vector (*.svg)"));
    gtk_file_filter_add_pattern(flt, "*.svg");
    gtk_file_chooser_add_filter((GtkFileChooser*)dialog, flt);
    if(m_iLastExportType == 4) pActFilter = flt;

    flt = gtk_file_filter_new();
    gtk_file_filter_set_name(flt, _("Drawing Exchange Format (*.dxf)"));
    gtk_file_filter_add_pattern(flt, "*.dxf");
    gtk_file_chooser_add_filter((GtkFileChooser*)dialog, flt);
    if(m_iLastExportType == 5) pActFilter = flt;

    gtk_file_chooser_set_filter((GtkFileChooser*)dialog, pActFilter);

    if(m_sLastPath)
        gtk_file_chooser_set_current_folder((GtkFileChooser*)dialog, m_sLastPath);
    if(m_sFileName)
    {
        gchar *sSlash = strrchr(m_sFileName, '/');
        int iLen;
        if(sSlash) iLen = strlen(sSlash);
        else iLen = strlen(m_sFileName);
        gchar *sNewName = (gchar*)g_malloc((iLen + 4)*sizeof(gchar));
        if(sSlash) strcpy(sNewName, &sSlash[1]);
        else strcpy(sNewName, m_sFileName);

        gchar *sDot = strrchr(sNewName, '.');
        if(sDot) *sDot = 0;
        gtk_file_chooser_set_current_name((GtkFileChooser*)dialog, sNewName);
        g_free(sNewName);
    }

    gchar *sFile = NULL;
    gchar sExt[4];

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        if(m_sLastPath) g_free(m_sLastPath);
        sFile = gtk_file_chooser_get_filename((GtkFileChooser*)dialog);
        m_sLastPath = gtk_file_chooser_get_current_folder((GtkFileChooser*)dialog);
        pActFilter = gtk_file_chooser_get_filter((GtkFileChooser*)dialog);
        const gchar *sFltName = gtk_file_filter_get_name(pActFilter);
        switch(sFltName[0])
        {
        case 'A':
            m_iLastExportType = 0;
            strcpy(sExt, "pdf");
            break;
        case 'P':
            if(sFltName[1] == 'o')
            {
                m_iLastExportType = 1;
                strcpy(sExt, "ps");
            }
            else
            {
                m_iLastExportType = 3;
                strcpy(sExt, "png");
            }
            break;
        case 'E':
            m_iLastExportType = 2;
            strcpy(sExt, "eps");
            break;
        case 'S':
            m_iLastExportType = 4;
            strcpy(sExt, "svg");
            break;
        case 'D':
            m_iLastExportType = 5;
            strcpy(sExt, "dxf");
            break;
        }
    }
    else bSave = false;
    gtk_widget_destroy(dialog);

    if(!bSave) return;

    gchar *sFileExt = NULL;
    gchar *sDot = strrchr(sFile, '.');

    if(sDot) sFileExt = sFile;
    else
    {
        int iLen = strlen(sFile) + 5;
        sFileExt = (gchar*)g_malloc(iLen*sizeof(gchar));
        strcpy(sFileExt, sFile);
        strcat(sFileExt, ".");
        strcat(sFileExt, sExt);
    }

    // export to the file
    if(m_iLastExportType < 5)
    {
        FILE *pf = fopen(sFileExt, "wb");
        ExportCairoFile(m_iLastExportType, pf, m_pDrawObjects, m_pFileSetupDlg->GetUnitList());
        fclose(pf);
    }
    else ExportDXFFile(sFileExt, m_pDrawObjects, m_pFileSetupDlg->GetUnitList());

    g_free(sFile);
    if(!sDot) g_free(sFileExt);

    GtkWidget *draw = GetDrawing();
    int iWidth = gdk_window_get_width(draw->window);
    int iHeight = gdk_window_get_height(draw->window);

    CDRect cdr;
    cdr.cPt1.x = -m_cViewOrigin.x/m_dUnitScale;
    cdr.cPt1.y = -m_cViewOrigin.y/m_dUnitScale;
    cdr.cPt2.x = (iWidth - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (iHeight - m_cViewOrigin.y)/m_dUnitScale;

    m_pDrawObjects->BuildAllPrimitives(&cdr);
    return;
}

void CDApplication::FilePropsCmd(bool bFromAccel)
{
    if(m_pFileSetupDlg->ShowDialog(m_pMainWnd, &m_cFSR))
    {
        if(fabs(m_cFSR.dScaleDenom) > g_dPrec)
            m_dDrawScale = m_cFSR.dScaleNomin/m_cFSR.dScaleDenom;
        else m_dDrawScale = 1.0;

        GetPageDims();

        CDFileAttrs cFAttrs;
        FilePropsToData(&cFAttrs);
        // set new file attributes
        m_pDrawObjects->SetFileAttrs(&cFAttrs, false);

        GtkWidget *draw = GetDrawing();
        gdk_window_invalidate_rect(draw->window, NULL, FALSE);
        SetTitle(m_pMainWnd, false);
    }
    return;
}

void CDApplication::FileQuitCmd(bool bFromAccel)
{
    Terminate();
    return;
}

void CDApplication::DrawCross(cairo_t *cr)
{
    cairo_save(cr);
    cairo_identity_matrix(cr);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, m_cLastSnapPt.x - 10, m_cLastSnapPt.y);
    cairo_line_to(cr, m_cLastSnapPt.x + 10, m_cLastSnapPt.y);
    cairo_move_to(cr, m_cLastSnapPt.x, m_cLastSnapPt.y - 10);
    cairo_line_to(cr, m_cLastSnapPt.x, m_cLastSnapPt.y + 10);
    cairo_stroke(cr);
    cairo_restore(cr);
}

void CDApplication::StartNewObject(gboolean bShowEdit)
{
    if(gtk_widget_get_visible(m_pStatEdt1))
    {
        gtk_widget_hide(m_pStatEdt1);
        gtk_widget_hide(m_pStatLab2);
        gtk_widget_hide(m_pStatEdt2);
        gtk_window_add_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);
    }
    m_sStatus3Msg[0] = 0;
    SetStatusBarMsg(3, m_sStatus3Msg);

    m_pActiveObject = NULL;
    m_iRestrictSet = -1;

    m_cMeasPoint1.bIsSet = false;
    m_cMeasPoint2.bIsSet = false;

    int iLines = m_pDrawObjects->GetNumOfSelectedLines();
    CDLine cLine1, cLine2;
    int iLinesFlag = 0;

    PDObject pLineObj;

    m_sStatus1Base[0] = 0;
    m_sStatus2Base[0] = 0;
    GtkWidget *msg_dlg = NULL;

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
        strcpy(m_sStatus1Base, _("Angle: "));
        m_pActiveObject = new CDObject(dtLine, m_cFSR.dDefLineWidth);
        if(bShowEdit)
        {
            if(!gtk_widget_get_visible(m_pStatEdt1))
            {
                gtk_widget_show(m_pStatEdt1);
                gtk_window_remove_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);
            }
            gtk_widget_grab_focus(m_pStatEdt1);
        }
        break;
    case modCircle:
        strcpy(m_sStatus1Base, _("Radius: "));
        m_pActiveObject = new CDObject(dtCircle, m_cFSR.dDefLineWidth);
        if(iLinesFlag & 1) m_pActiveObject->SetInputLine(0, cLine1);
        //if(iLinesFlag & 2) m_pActiveObject->SetInputLine(1, cLine2);
        if(bShowEdit)
        {
            if(!gtk_widget_get_visible(m_pStatEdt1))
            {
                gtk_widget_show(m_pStatEdt1);
                gtk_window_remove_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);
            }
            gtk_widget_grab_focus(m_pStatEdt1);
        }
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
            msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pMainWnd), GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                _("Exactly two lines must be selected to insert an arc ellipse"));
            gtk_dialog_run(GTK_DIALOG(msg_dlg));
            gtk_widget_destroy(msg_dlg);
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
            msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pMainWnd), GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                _("Exactly two lines must be selected to insert a hyperbola"));
            gtk_dialog_run(GTK_DIALOG(msg_dlg));
            gtk_widget_destroy(msg_dlg);
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
            msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pMainWnd), GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                _("Exactly one line must be selected to insert a parabola"));
            gtk_dialog_run(GTK_DIALOG(msg_dlg));
            gtk_widget_destroy(msg_dlg);
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
            msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pMainWnd), GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                _("Exactly one circle must be selected to insert an evolventa"));
            gtk_dialog_run(GTK_DIALOG(msg_dlg));
            gtk_widget_destroy(msg_dlg);
        }
        break;
    }

    if(m_iToolMode == tolRound)
    {
        int iCnt = m_pDrawObjects->GetSelectCount();
        if(iCnt == 2)
        {
            m_pActiveObject = new CDObject(dtCircle, m_cFSR.dDefLineWidth);
            strcpy(m_sStatus1Base, _("Radius: "));
            if(!gtk_widget_get_visible(m_pStatEdt1))
            {
                gtk_widget_show(m_pStatEdt1);
                gtk_window_remove_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);
            }
            gtk_widget_grab_focus(m_pStatEdt1);
        }
        else
        {
            msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pMainWnd), GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                _("Exactly two objects must be selected to round corner"));
            gtk_dialog_run(GTK_DIALOG(msg_dlg));
            gtk_widget_destroy(msg_dlg);
        }
    }

    strcpy(m_sStatus1Msg, m_sStatus1Base);
    strcpy(m_sStatus2Msg, m_sStatus2Base);
    SetStatusBarMsg(1, m_sStatus1Msg);
    SetStatusBarMsg(2, m_sStatus2Msg);
}

void CDApplication::SetMode(int iNewMode, bool bFromAccel)
{
    if(m_bSettingProps) return;

    if(gtk_widget_get_visible(m_pStatEdt1))
    {
        gtk_widget_hide(m_pStatEdt1);
        gtk_widget_hide(m_pStatEdt2);
        gtk_window_add_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);
    }

    if(m_pActiveObject)
    {
        delete m_pActiveObject;
        m_pActiveObject = NULL;
    }
    else if(m_pSelForDimen)
    {
        m_pSelForDimen->DiscardDimen();
        m_pSelForDimen = NULL;
    }

    GtkWidget *draw = GetDrawing();
    cairo_t *cr = gdk_cairo_create(draw->window);

    if(m_pcs)
    {
        cairo_set_source_surface(cr, m_pcs, 0, 0);
        cairo_mask(cr, m_pcp);
    }

    m_iToolMode = tolNone;
    int iCurMode = m_iDrawMode;
    m_iDrawMode = iNewMode;

    StartNewObject(FALSE);
    if(!m_pActiveObject && (m_iDrawMode > modSelect)) m_iDrawMode = modSelect;

    if(m_iDrawMode > modSelect) DrawCross(cr);
    cairo_destroy(cr);

    m_bSettingProps = true;

    GtkWidget *menu = GetMenuBar();
    GList *pChilds = gtk_container_get_children(GTK_CONTAINER(menu));
    GtkWidget *mode_menu = (GtkWidget*)g_list_nth(pChilds, 1)->data;
    GtkWidget *mode_top = (GtkWidget*)gtk_menu_item_get_submenu(GTK_MENU_ITEM(mode_menu));
    pChilds = gtk_container_get_children(GTK_CONTAINER(mode_top));
    GtkWidget *menu_item = (GtkWidget*)g_list_nth(pChilds, iCurMode)->data;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), FALSE);
    menu_item = (GtkWidget*)g_list_nth(pChilds, m_iDrawMode)->data;
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), TRUE);

    m_bSettingProps = false;

    switch(m_iDrawMode)
    {
    case modLine:
        if(!gtk_widget_get_visible(m_pStatEdt1))
        {
            gtk_widget_show(m_pStatEdt1);
            gtk_window_remove_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);
        }
        gtk_widget_grab_focus(m_pStatEdt1);
        break;
    case modCircle:
        if(!gtk_widget_get_visible(m_pStatEdt1))
        {
            gtk_widget_show(m_pStatEdt1);
            gtk_window_remove_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);
        }
        gtk_widget_grab_focus(m_pStatEdt1);
        break;
    }
}

void CDApplication::SetTool(int iNewTool)
{
    if(m_pActiveObject)
    {
        delete m_pActiveObject;
        m_pActiveObject = NULL;
    }
    else if(m_pSelForDimen)
    {
        m_pSelForDimen->DiscardDimen();
        m_pSelForDimen = NULL;
    }

    m_iDrawMode = modSelect;
    m_iToolMode = tolNone;

    if(iNewTool == tolDimen)
    {
        if(m_pDrawObjects->GetSelectCount() != 1)
        {
            GtkWidget *msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pMainWnd), GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                _("Exactly one object must be selected to insert a dimension"));
            gtk_dialog_run(GTK_DIALOG(msg_dlg));
            gtk_widget_destroy(msg_dlg);
            return;
        }
        else m_pSelForDimen = m_pDrawObjects->GetSelected(0);
    }

    GtkWidget *draw = GetDrawing();
    cairo_t *cr = gdk_cairo_create(draw->window);
    if(m_pcs)
    {
        cairo_set_source_surface(cr, m_pcs, 0, 0);
        cairo_mask(cr, m_pcp);
    }
    DrawCross(cr);
    cairo_destroy(cr);

    m_iToolMode = iNewTool;

    StartNewObject(TRUE);
}

void CDApplication::ModeCommand(int iCmd, bool bFromAccel)
{
    if(m_bSettingProps) return;

    switch(iCmd)
    {
    case IDM_MODESELECT:
        SetMode(modSelect, bFromAccel);
        break;
    case IDM_MODELINE:
        SetMode(modLine, bFromAccel);
        break;
    case IDM_MODECIRCLE:
        SetMode(modCircle, bFromAccel);
        break;
    case IDM_MODEELLIPSE:
        SetMode(modEllipse, bFromAccel);
        break;
    case IDM_MODEARCELLIPSE:
        SetMode(modArcElps, bFromAccel);
        break;
    case IDM_MODEHYPERBOLA:
        SetMode(modHyperbola, bFromAccel);
        break;
    case IDM_MODEPARABOLA:
        SetMode(modParabola, bFromAccel);
        break;
    case IDM_MODESPLINE:
        SetMode(modSpline, bFromAccel);
        break;
    case IDM_MODEEVEOLVENT:
        SetMode(modEvolvent, bFromAccel);
        break;
    case IDM_MODEDIMEN:
        SetTool(tolDimen);
        break;
    }

    return;
}

void CDApplication::EditDeleteCmd(GtkWidget *widget, bool bFromAccel)
{
    m_pActiveObject = NULL;
    m_pHighObject = NULL;

    GtkWidget *draw = GetDrawing();
    int iWidth = gdk_window_get_width(draw->window);
    int iHeight = gdk_window_get_height(draw->window);

    CDRect cdr;
    cdr.cPt1.x = -m_cViewOrigin.x/m_dUnitScale;
    cdr.cPt1.y = -m_cViewOrigin.y/m_dUnitScale;
    cdr.cPt2.x = (iWidth - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (iHeight - m_cViewOrigin.y)/m_dUnitScale;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);

    if(m_pDrawObjects->DeleteSelected(m_pUndoObjects, &cdr, pRegions))
    {
        GdkRectangle cRect;
        if(GetUpdateRegion(pRegions, &cRect))
            gdk_window_invalidate_rect(draw->window, &cRect, FALSE);
        SetTitle(widget, false);
    }

    ClearPolygonList(pRegions);
    delete pRegions;
    return;
}

void CDApplication::EditCopyParCmd(GtkWidget *widget, bool bFromAccel)
{
    int iSel = m_pDrawObjects->GetSelectCount();
    if(iSel != 1) return;

    PDObject pObj = m_pDrawObjects->GetSelected(0);
    if(!pObj) return;

    PDObject pNewObj = pObj->Copy();
    if(!pNewObj) return;

    m_pActiveObject = pNewObj;
    m_iToolMode = tolCopyPar;

    strcpy(m_sStatus1Base, _("Distance: "));
    strcpy(m_sStatus1Msg, m_sStatus1Base);
    SetStatusBarMsg(1, m_sStatus1Msg);

    if(!gtk_widget_get_visible(m_pStatEdt1))
    {
        gtk_widget_show(m_pStatEdt1);
        gtk_window_remove_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);
    }
    gtk_widget_grab_focus(m_pStatEdt1);
    return;
}

void CDApplication::EditMoveCmd(GtkWidget *widget)
{
    strcpy(m_sStatus1Base, _("Distance: "));
    SetStatusBarMsg(1, m_sStatus1Base);
    strcpy(m_sStatus2Base, _("# of copies: "));
    SetStatusBarMsg(2, m_sStatus2Base);
    if(!gtk_widget_get_visible(m_pStatEdt1))
    {
        gtk_widget_show(m_pStatEdt1);
        gtk_window_remove_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);
    }
    gtk_widget_show(m_pStatLab2);
    gtk_widget_show(m_pStatEdt2);

    gtk_widget_grab_focus(m_pStatEdt1);

    const gchar *sBuf = gtk_entry_get_text(GTK_ENTRY(m_pStatEdt1));

    m_iRestrictSet = ParseInputString((char*)sBuf, m_pFileSetupDlg->GetUnitList(), &m_dRestrictValue);
    if(IS_LENGTH_VAL(m_iRestrictSet))
        strcpy(m_sStatus3Msg, _("Click a line to move along"));
    else strcpy(m_sStatus3Msg, _("Click a point to move from"));
    SetStatusBarMsg(3, m_sStatus3Msg);
    m_iToolMode = tolMove;
    return;
}

void CDApplication::EditRotateCmd(GtkWidget *widget)
{
    strcpy(m_sStatus1Base, _("Angle: "));
    SetStatusBarMsg(1, m_sStatus1Base);
    strcpy(m_sStatus2Base, _("# of copies: "));
    SetStatusBarMsg(2, m_sStatus2Base);
    if(!gtk_widget_get_visible(m_pStatEdt1))
    {
        gtk_widget_show(m_pStatEdt1);
        gtk_window_remove_accel_group(GTK_WINDOW(m_pMainWnd), m_pAccelGroup);
    }
    gtk_widget_show(m_pStatLab2);
    gtk_widget_show(m_pStatEdt2);

    gtk_widget_grab_focus(m_pStatEdt1);

    strcpy(m_sStatus3Msg, _("Click a point to rotate about"));
    SetStatusBarMsg(3, m_sStatus3Msg);
    m_iToolMode = tolRotate;
    return;
}

void CDApplication::EditMirrorCmd(GtkWidget *widget)
{
    strcpy(m_sStatus3Msg, _("Click a line to mirror about"));
    SetStatusBarMsg(3, m_sStatus3Msg);
    m_iToolMode = tolMirror;
    return;
}

void CDApplication::EditLineStyleCmd(GtkWidget *widget)
{
    GtkWidget *draw = GetDrawing();
    int iWidth = gdk_window_get_width(draw->window);
    int iHeight = gdk_window_get_height(draw->window);
    GdkRectangle cRect;
    CDRect cdr;

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
        cLSRec.bWidthChanged = FALSE;
        cLSRec.bExcChanged = FALSE;
        cLSRec.bPatChanged = FALSE;
        if(m_pLineStyleDlg->ShowDialog(widget, &cLSRec))
        {
            iMask = 0;
            if(cLSRec.bWidthSet && cLSRec.bWidthChanged) iMask |= 1;
            if(cLSRec.bExcSet && cLSRec.bExcChanged) iMask |= 2;
            if(cLSRec.bPatSet && cLSRec.bPatChanged) iMask |= 4;
            if(m_pDrawObjects->SetSelectedLineStyle(iMask, &cLSRec.cLineStyle, pRegions))
            {
                cdr.cPt1.x = -m_cViewOrigin.x/m_dUnitScale;
                cdr.cPt1.y = -m_cViewOrigin.y/m_dUnitScale;
                cdr.cPt2.x = (iWidth - m_cViewOrigin.x)/m_dUnitScale;
                cdr.cPt2.y = (iHeight - m_cViewOrigin.y)/m_dUnitScale;

                m_pDrawObjects->BuildAllPrimitives(&cdr);
                if(GetUpdateRegion(pRegions, &cRect))
                    gdk_window_invalidate_rect(draw->window, &cRect, FALSE);
                SetTitle(widget, false);
            }
        }
    }
    else if(m_pDrawObjects->GetSelectedDimen(&cDimen))
    {
        if(m_pDimEditDlg->ShowDialog(widget, &cDimen, m_pFileSetupDlg->GetUnitList(), &m_cFSR.cGraphUnit))
        {
            if(m_pDrawObjects->SetSelectedDimen(&cDimen, pRegions))
            {
                CDRect cdr;
                cdr.cPt1.x = -m_cViewOrigin.x/m_dUnitScale;
                cdr.cPt1.y = -m_cViewOrigin.y/m_dUnitScale;
                cdr.cPt2.x = (iWidth - m_cViewOrigin.x)/m_dUnitScale;
                cdr.cPt2.y = (iHeight - m_cViewOrigin.y)/m_dUnitScale;

                m_pDrawObjects->BuildAllPrimitives(&cdr);
                if(GetUpdateRegion(pRegions, &cRect))
                    gdk_window_invalidate_rect(draw->window, &cRect, FALSE);
                SetTitle(widget, false);
            }
        }
        if(cDimen.psLab) free(cDimen.psLab);
    }
    ClearPolygonList(pRegions);
    delete pRegions;
}

void CDApplication::EditToggleSnapCmd(GtkWidget *widget)
{
    gboolean bSnapEnabled = m_pDrawObjects->GetSelSnapEnabled();
    if(m_pSnapDlg->ShowDialog(m_pMainWnd, &bSnapEnabled))
    {
        m_pDrawObjects->SetSelSnapEnabled(bSnapEnabled);
    }
    return;
}

void CDApplication::EditPaperUnitsCmd(GtkWidget *widget, bool bFromAccel)
{
    if(m_bSettingProps) return;

    if(bFromAccel)
    {
        GtkWidget *menu = GetMenuBar();
        GList *pChilds = gtk_container_get_children(GTK_CONTAINER(menu));
        GtkWidget *edit_menu = (GtkWidget*)g_list_nth(pChilds, 2)->data;
        GtkWidget *edit_top = (GtkWidget*)gtk_menu_item_get_submenu(GTK_MENU_ITEM(edit_menu));
        pChilds = gtk_container_get_children(GTK_CONTAINER(edit_top));
        GtkWidget *menu_item = (GtkWidget*)g_list_nth(pChilds, 7)->data;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), !m_bPaperUnits);
    }
    else m_bPaperUnits = !m_bPaperUnits;
    return;
}

void CDApplication::EditUndoCmd(GtkWidget *widget)
{
    int iCnt = m_pUndoObjects->GetCount();
    if(iCnt < 1) return;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);

    PDObject pObj = m_pUndoObjects->GetItem(iCnt - 1);
    m_pUndoObjects->ClearSelection(pRegions);
    pObj->SetSelected(true, false, -1, pRegions);

    if(m_pUndoObjects->DeleteSelected(m_pDrawObjects, NULL, pRegions))
    {
        m_iRedoCount++;
        m_pDrawObjects->SetChanged();
        SetTitle(m_pMainWnd, false);

        GdkRectangle cRect;
        GtkWidget *draw = GetDrawing();
        if(GetUpdateRegion(pRegions, &cRect))
            gdk_window_invalidate_rect(draw->window, &cRect, FALSE);
    }

    ClearPolygonList(pRegions);
    delete pRegions;
    return;
}

void CDApplication::EditRedoCmd(GtkWidget *widget)
{
    if(m_iRedoCount < 1) return;
    int iCnt = m_pDrawObjects->GetCount();
    if(iCnt < 1) return;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);

    PDObject pObj = m_pDrawObjects->GetItem(iCnt - 1);
    m_pDrawObjects->ClearSelection(pRegions);
    pObj->SetSelected(true, false, -1, pRegions);

    if(m_pDrawObjects->DeleteSelected(m_pUndoObjects, NULL, pRegions))
    {
        m_iRedoCount--;

        GdkRectangle cRect;
        GtkWidget *draw = GetDrawing();
        if(GetUpdateRegion(pRegions, &cRect))
            gdk_window_invalidate_rect(draw->window, &cRect, FALSE);

        SetTitle(m_pMainWnd, false);
    }

    ClearPolygonList(pRegions);
    delete pRegions;
    return;
}

void CDApplication::EditConfirmCmd(GtkWidget *widget)
{
    bool bConfirm = (m_iDrawMode == modLine) || (m_iDrawMode == modCircle) ||
        (m_iToolMode == tolRound) || (m_iToolMode == tolCopyPar);
    if(bConfirm)
    {
        const gchar *sBuf = gtk_entry_get_text(GTK_ENTRY(m_pStatEdt1));
        m_iRestrictSet = ParseInputString((char*)sBuf, m_pFileSetupDlg->GetUnitList(), &m_dRestrictValue);
    }
    return;
}

void CDApplication::EditCommand(int iCmd, bool bFromAccel)
{
    switch(iCmd)
    {
    case IDM_EDITDELETE:
        EditDeleteCmd(m_pMainWnd, bFromAccel);
        break;
    case IDM_EDITCOPYPAR:
        EditCopyParCmd(m_pMainWnd, bFromAccel);
        break;
    case IDM_EDITMOVE:
        EditMoveCmd(m_pMainWnd);
        break;
    case IDM_EDITROTATE:
        EditRotateCmd(m_pMainWnd);
        break;
    case IDM_EDITMIRROR:
        EditMirrorCmd(m_pMainWnd);
        break;
    case IDM_EDITLINESTYLE:
        EditLineStyleCmd(m_pMainWnd);
        break;
    case IDM_EDITTOGGLESNAP:
        EditToggleSnapCmd(m_pMainWnd);
        break;
    case IDM_EDITPAPERUNITS:
        EditPaperUnitsCmd(m_pMainWnd, bFromAccel);
        break;
    case IDM_EDITUNDO:
        EditUndoCmd(m_pMainWnd);
        break;
    case IDM_EDITREDO:
        EditRedoCmd(m_pMainWnd);
        break;
    case IDM_EDITCONFIRM:
        EditConfirmCmd(m_pMainWnd);
        break;
    }
    return;
}

void CDApplication::ViewCommand(int iCmd, bool bFromAccel)
{
    switch(iCmd)
    {
    case IDM_VIEWFITALL:
        ViewFitCmd(m_pMainWnd);
        break;
    case IDM_VIEWACTSIZE:
        ViewNormalCmd(m_pMainWnd);
        break;
    case IDM_VIEWGRIDPTS:
        ViewGridCmd(m_pMainWnd, bFromAccel, 1);
        break;
    case IDM_VIEWGRIDLNS:
        ViewGridCmd(m_pMainWnd, bFromAccel, 2);
        break;
    }
    return;
}

void CDApplication::ToolsStatCmd()
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

    m_pStatDlg->ShowDialog(m_pMainWnd, &cSR);
    return;
}

void CDApplication::ToolsCommand(int iCmd, bool bFromAccel)
{
    switch(iCmd)
    {
    case IDM_TOOLSKNIFE:
        SetTool(tolKnife);
        break;
    case IDM_TOOLSROUND:
        SetTool(tolRound);
        break;
    case IDM_TOOLSEXTEND:
        SetTool(tolExtend);
        break;
    case IDM_TOOLSCONFLICTS:
        SetTool(tolConflict);
        break;
    case IDM_TOOLSMEASURE:
        SetTool(tolMeas);
        break;
    case IDM_TOOLSBREAK:
        ToolsBreakCmd();
        break;
    case IDM_TOOLSCALE:
        ToolsScaleCmd();
        break;
    case IDM_TOOLSTAT:
        ToolsStatCmd();
        break;
    }
    return;
}

void CDApplication::ViewFitCmd(GtkWidget *widget)
{
    GtkWidget *draw = GetDrawing();

    int iWidth = gdk_window_get_width(draw->window);
    int iHeight = gdk_window_get_height(draw->window);

    double dwWin = iWidth - 20;
    double dhWin = iHeight - 20;

    double drw = dwWin/m_dwPage;
    double drh = dhWin/m_dhPage;

    if(drw < drh)
    {
        m_dUnitScale = drw;
        m_cViewOrigin.x = 10;
        m_cViewOrigin.y = (dhWin + 20 - m_dUnitScale*m_dhPage)/2;
    }
    else
    {
        m_dUnitScale = drh;
        m_cViewOrigin.x = (dwWin + 20 - m_dUnitScale*m_dwPage)/2;
        m_cViewOrigin.y = 10;
    }

    gdk_window_invalidate_rect(draw->window, NULL, FALSE);
    return;
}

void CDApplication::ViewNormalCmd(GtkWidget *widget)
{
    GtkWidget *draw = GetDrawing();

    int iWidth = gdk_window_get_width(draw->window);
    int iHeight = gdk_window_get_height(draw->window);

    CDPoint cOrigOff;
    double ddx = iWidth/2.0;
    double ddy = iHeight/2.0;
    cOrigOff.x = (ddx - m_cViewOrigin.x)/m_dUnitScale;
    cOrigOff.y = (ddy - m_cViewOrigin.y)/m_dUnitScale;

    m_dUnitScale = m_dDeviceToUnitScale;

    m_cViewOrigin.x = (int)(ddx - cOrigOff.x*m_dUnitScale);
    m_cViewOrigin.y = (int)(ddy - cOrigOff.y*m_dUnitScale);

    gdk_window_invalidate_rect(draw->window, NULL, FALSE);
}

void CDApplication::ViewGridCmd(GtkWidget *widget, bool bFromAccel, int iFlag)
{
    if(m_bSettingProps) return;

    gboolean bUncheck = (m_iDrawGridMode & iFlag);
    if(bFromAccel)
    {
        GtkWidget *menu = GetMenuBar();
        GList *pChilds = gtk_container_get_children(GTK_CONTAINER(menu));
        GtkWidget *edit_menu = (GtkWidget*)g_list_nth(pChilds, 3)->data;
        GtkWidget *edit_top = (GtkWidget*)gtk_menu_item_get_submenu(GTK_MENU_ITEM(edit_menu));
        pChilds = gtk_container_get_children(GTK_CONTAINER(edit_top));
        GtkWidget *menu_item = (GtkWidget*)g_list_nth(pChilds, iFlag + 2)->data;
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), !bUncheck);
    }
    else
    {
        if(bUncheck) m_iDrawGridMode &= ~iFlag;
        else m_iDrawGridMode |= iFlag;
        gdk_window_invalidate_rect(GetDrawing()->window, NULL, FALSE);
    }
}

int CDApplication::GetDynMode()
{
    int iRes = 0;
    if(m_iDrawMode > modSelect) iRes = 1;
    else if(m_iToolMode == tolCopyPar) iRes = 2;
    else if(m_iToolMode == tolRound) iRes = 3;
    else if(m_iToolMode == tolDimen) iRes = 4;
    return iRes;
}

int Min2(int i1, int i2)
{
    return i1 < i2 ? i1 : i2;
}

int Max2(int i1, int i2)
{
    return i1 > i2 ? i1 : i2;
}

void CDApplication::DrawSizeRectClip(cairo_t *cr, double x, double y)
{
    int iMinX2 = Min2(m_cLastMovePt.x, x);
    int iMinY2 = Min2(m_cLastMovePt.y, y);
    int iMaxX2 = Max2(m_cLastMovePt.x, x);
    int iMaxY2 = Max2(m_cLastMovePt.y, y);

    cairo_new_path(cr);
    if(m_cLastDownPt.x < iMinX2)
    {
        if(m_cLastDownPt.y < iMinY2)
        {
            cairo_move_to(cr, m_cLastDownPt.x - 2, iMinY2 - 2);
            cairo_line_to(cr, m_cLastDownPt.x - 2, iMaxY2 + 2);
            cairo_line_to(cr, iMaxX2 + 2, iMaxY2 + 2);
            cairo_line_to(cr, iMaxX2 + 2, m_cLastDownPt.y - 2);
            cairo_line_to(cr, iMinX2 - 2, m_cLastDownPt.y - 2);
            cairo_line_to(cr, iMinX2 - 2, iMinY2 - 2);
        }
        else if(m_cLastDownPt.y > iMaxY2)
        {
            cairo_move_to(cr, m_cLastDownPt.x - 2, iMaxY2 + 2);
            cairo_line_to(cr, iMinX2 - 2, iMaxY2 + 2);
            cairo_line_to(cr, iMinX2 - 2, m_cLastDownPt.y + 2);
            cairo_line_to(cr, iMaxX2 + 2, m_cLastDownPt.y + 2);
            cairo_line_to(cr, iMaxX2 + 2, iMinY2 - 2);
            cairo_line_to(cr, m_cLastDownPt.x - 2, iMinY2 - 2);
        }
        else
        {
            cairo_move_to(cr, m_cLastDownPt.x - 2, iMaxY2 + 2);
            cairo_line_to(cr, iMaxX2 + 2, iMaxY2 + 2);
            cairo_line_to(cr, iMaxX2 + 2, iMinY2 - 2);
            cairo_line_to(cr, m_cLastDownPt.x - 2, iMinY2 - 2);
        }
    }
    else if(m_cLastDownPt.x > iMaxX2)
    {
        if(m_cLastDownPt.y < iMinY2)
        {
            cairo_move_to(cr, m_cLastDownPt.x + 2, iMinY2 - 2);
            cairo_line_to(cr, iMaxX2 + 2, iMinY2 - 2);
            cairo_line_to(cr, iMaxX2 + 2, m_cLastDownPt.y - 2);
            cairo_line_to(cr, iMinX2 - 2, m_cLastDownPt.y - 2);
            cairo_line_to(cr, iMinX2 - 2, iMaxY2 + 2);
            cairo_line_to(cr, m_cLastDownPt.x + 2, iMaxY2 + 2);
        }
        else if(m_cLastDownPt.y > iMaxY2)
        {
            cairo_move_to(cr, m_cLastDownPt.x + 2, iMaxY2 + 2);
            cairo_line_to(cr, m_cLastDownPt.x + 2, iMinY2 - 2);
            cairo_line_to(cr, iMinX2 - 2, iMinY2 - 2);
            cairo_line_to(cr, iMinX2 - 2, m_cLastDownPt.y + 2);
            cairo_line_to(cr, iMaxX2 + 2, m_cLastDownPt.y + 2);
            cairo_line_to(cr, iMaxX2 + 2, iMaxY2 + 2);
        }
        else
        {
            cairo_move_to(cr, m_cLastDownPt.x + 2, iMaxY2 + 2);
            cairo_line_to(cr, m_cLastDownPt.x + 2, iMinY2 - 2);
            cairo_line_to(cr, iMinX2 - 2, iMinY2 - 2);
            cairo_line_to(cr, iMinX2 - 2, iMaxY2 + 2);
        }
    }
    else
    {
        if(m_cLastDownPt.y < iMinY2)
        {
            cairo_move_to(cr, iMaxX2 + 2, m_cLastDownPt.y - 2);
            cairo_line_to(cr, iMinX2 - 2, m_cLastDownPt.y - 2);
            cairo_line_to(cr, iMinX2 - 2, iMaxY2 + 2);
            cairo_line_to(cr, iMaxX2 + 2, iMaxY2 + 2);
        }
        else if(m_cLastDownPt.y > iMaxY2)
        {
            cairo_move_to(cr, iMinX2 - 2, iMinY2 - 2);
            cairo_line_to(cr, iMinX2 - 2, m_cLastDownPt.y + 2);
            cairo_line_to(cr, iMaxX2 + 2, m_cLastDownPt.y + 2);
            cairo_line_to(cr, iMaxX2 + 2, iMinY2 - 2);
        }
        else
        {
            cairo_move_to(cr, iMinX2 - 2, iMinY2 - 2);
            cairo_line_to(cr, iMinX2 - 2, iMaxY2 + 2);
            cairo_line_to(cr, iMaxX2 + 2, iMaxY2 + 2);
            cairo_line_to(cr, iMaxX2 + 2, iMinY2 - 2);
        }
    }
    cairo_clip(cr);
}

void CDApplication::MouseMove(GtkWidget *widget, GdkEventMotion *event, gboolean bForce)
{
    // if the previous request lasted more than 0.01 seconds, skip this one
    if(!bForce)
    {
        if(g_get_monotonic_time() - 1000*(gint64)event->time > 10000) return;
    }

    int iWidth = gdk_window_get_width(event->window);
    int iHeight = gdk_window_get_height(event->window);

    double xPos = event->x;
    double yPos = event->y;

    cairo_t *cr = NULL;

    if(m_iButton == 2)
    {
        m_cViewOrigin.x = (xPos - m_cZoomOrig.x);
        m_cViewOrigin.y = (yPos - m_cZoomOrig.y);

        int ix = xPos - m_cLastMovePt.x;
        int iy = yPos - m_cLastMovePt.y;
        m_cLastSnapPt.x += ix;
        m_cLastSnapPt.y += iy;
        gdk_window_scroll(event->window, ix, iy);

        m_cLastMovePt.x = xPos;
        m_cLastMovePt.y = yPos;
        return;
    }

    double dx = (xPos - m_cViewOrigin.x)/m_dUnitScale;
    double dy = (yPos - m_cViewOrigin.y)/m_dUnitScale;

    CDRect cdr;
    int iDynMode = GetDynMode();

    double dTol;

    if(m_iDrawMode + m_iToolMode < 1)
    {
        if((m_iButton == 1) || (m_iButton == 3))
        {
            cr = gdk_cairo_create(event->window);
            DrawSizeRectClip(cr, xPos, yPos);

            if(m_pcs)
            {
                cairo_set_source_surface(cr, m_pcs, 0, 0);
                cairo_paint(cr);
            }

            m_cLastMovePt.x = xPos;
            m_cLastMovePt.y = yPos;

            GdkRectangle cRect2 = {m_cLastDownPt.x, m_cLastDownPt.y, m_cLastMovePt.x - m_cLastDownPt.x, m_cLastMovePt.y - m_cLastDownPt.y};
            /*if(cRect2.width < 0)
            {
                cRect2.x += cRect2.width;
                cRect2.width *= -1;
            }
            if(cRect2.height < 0)
            {
                cRect2.y += cRect2.height;
                cRect2.height *= -1;
            }*/

            cairo_rectangle(cr, cRect2.x, cRect2.y, cRect2.width, cRect2.height);
            SetLColorA(cr, m_lSelColor, 35);
            cairo_fill_preserve(cr);
            SetLColor(cr, m_lSelColor);
            cairo_stroke(cr);
            cairo_destroy(cr);

            /*GdkGC *pdc = gdk_gc_new(event->window);
            gdk_gc_set_function(pdc, GDK_EQUIV);
            SetGdkColor(pdc, m_lSelColor);

            gdk_draw_rectangle(event->window, pdc, FALSE, cRect1.x, cRect1.y, cRect1.width, cRect1.height);
            gdk_draw_rectangle(event->window, pdc, FALSE, cRect2.x, cRect2.y, cRect2.width, cRect2.height);

            gdk_gc_destroy(pdc);*/
        }
        else if(m_iButton < 1)
        {
            CDPoint cPt = {dx, dy};
            dTol = (double)m_iSelectTolerance/m_dUnitScale;

            int iDimen;
            PDObject pNewHigh = m_pDrawObjects->SelectByPoint(cPt, dTol, &iDimen);

            if((m_pHighObject != pNewHigh) || (iDimen != m_iHighDimen))
            {
                cr = gdk_cairo_create(event->window);
                
                if(m_pHighObject) DrawObject(cr, m_pHighObject, 0, m_iHighDimen);
                if(pNewHigh) DrawObject(cr, pNewHigh, 2, iDimen);
                m_pHighObject = pNewHigh;
                m_iHighDimen = iDimen;
                cairo_destroy(cr);
            }
        }
    }

    gchar buf[64];
    sprintf(buf, "%.3f, %.3f", dx/m_cFSR.cPaperUnit.dBaseToUnit, dy/m_cFSR.cPaperUnit.dBaseToUnit);
    SetStatusBarMsg(0, buf);

    if(m_iButton > 0) return;

    int iCnt = 0;
    PDObject pObj1, pObj2;

    m_cLastSnapPt.x = xPos;
    m_cLastSnapPt.y = yPos;

    if(m_iDrawMode + m_iToolMode > 0)
    {
        cdr.cPt1.x = -m_cViewOrigin.x/m_dUnitScale;
        cdr.cPt1.y = -m_cViewOrigin.y/m_dUnitScale;
        cdr.cPt2.x = (iWidth - m_cViewOrigin.x)/m_dUnitScale;
        cdr.cPt2.y = (iHeight - m_cViewOrigin.y)/m_dUnitScale;

        cr = gdk_cairo_create(event->window);

        cairo_set_source_surface(cr, m_pcs, 0, 0);
        cairo_mask(cr, m_pcp);

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

        if(event->state & GDK_CONTROL_MASK)
        {
            bDoSnap = false;

            if(bHasLastPoint)
            {
                CDPoint cMainDir = {1.0, 0.0};

                iCnt = m_pDrawObjects->GetSelectCount();
                if(iCnt == 1)
                {
                    pObj1 = m_pDrawObjects->GetSelected(0);
                    if(m_cLastDynPt.bIsSet)
                        pObj1->GetDistFromPt(m_cLastDynPt.cOrigin, m_cLastDynPt.cOrigin, true, &cPtX, NULL);
                    else pObj1->GetDistFromPt(cLstInPt.cPoint, cLstInPt.cPoint, true, &cPtX, NULL);
                    if(cPtX.bIsSet) cMainDir = cPtX.cDirection;
                }

                if((event->state & GDK_SHIFT_MASK) && (iCnt == 1))
                {
                    cDir2.x = dx;
                    cDir2.y = dy;
                    cDir1 = pObj1->GetPointToDir(cLstInPt.cPoint, m_dSavedAngle, cDir2);
                    m_cLastDynPt.bIsSet = true;
                    m_cLastDynPt.cOrigin = cDir1;
                    m_cLastDynPt.cDirection.x = 0.0;
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
            if(m_iToolMode == tolCopyPar) iSnapType = 2;
            if(m_pDrawObjects->GetSnapPoint(iSnapType, m_cLastDrawPt,
                dTol, &cSnapPt, m_pActiveObject) > 0)
            {
                if((event->state & GDK_SHIFT_MASK) && (iCnt == 1))
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

        cPtX.cOrigin = m_cLastDrawPt;
        if(iDynMode == 1)
        {
            cPtX.bIsSet = m_cLastDynPt.bIsSet;
            cPtX.cDirection = m_cLastDynPt.cOrigin;
        }
        else if(iDynMode == 2)
        {
            cPtX.cDirection.x = 0.0;
            if(event->state & GDK_SHIFT_MASK) cPtX.cDirection.x = -1.0;
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
                        sprintf(m_sStatus1Msg, "%s %.2f %s", m_sStatus1Base, dVal,
                            m_cFSR.cAngUnit.sAbbrev);
                    }
                    else
                    {
                        if(m_bPaperUnits)
                        {
                            dVal /= m_cFSR.cPaperUnit.dBaseToUnit;
                            sprintf(m_sStatus1Msg, "%s %.2f %s", m_sStatus1Base, dVal,
                                m_cFSR.cPaperUnit.sAbbrev);
                        }
                        else
                        {
                            dVal /= m_dDrawScale;
                            dVal /= m_cFSR.cLenUnit.dBaseToUnit;
                            sprintf(m_sStatus1Msg, "%s %.2f %s", m_sStatus1Base, dVal,
                                m_cFSR.cLenUnit.sAbbrev);
                        }
                    }
                    SetStatusBarMsg(1, m_sStatus1Msg);
                }
            }
            else
            {
                dVal = m_dRestrictValue;
                if((m_iDrawMode == modLine) && (iDynMode != 2))
                {
                    sprintf(m_sStatus1Msg, "%s %.2f %s", m_sStatus1Base, dVal,
                        m_cFSR.cAngUnit.sAbbrev);
                }
                else
                {
                    if(m_bPaperUnits)
                    {
                        sprintf(m_sStatus1Msg, "%s %.2f %s", m_sStatus1Base, dVal,
                            m_cFSR.cPaperUnit.sAbbrev);
                    }
                    else
                    {
                        sprintf(m_sStatus1Msg, "%s %.2f %s", m_sStatus1Base, dVal,
                            m_cFSR.cLenUnit.sAbbrev);
                    }
                }
                SetStatusBarMsg(1, m_sStatus1Msg);
            }

            m_pActiveObject->BuildPrimitives(cPtX, iDynMode, &cdr, false, NULL);

            DrawObject(cr, m_pActiveObject, 1, -2);
        }
        else if(iDynMode == 4)
        {
            iCnt = m_pDrawObjects->GetSelectCount();
            if(iCnt == 1)
            {
                pObj1 = m_pDrawObjects->GetSelected(0);
                CDFileAttrs cFAttrs;
                FilePropsToData(&cFAttrs);

                pObj1->BuildPrimitives(cPtX, iDynMode, &cdr, false, &cFAttrs);
                DrawObject(cr, pObj1, 1, -1);
            }
        }

        DrawCross(cr);

        cairo_destroy(cr);
    }
    return;
}

void CDApplication::MouseWheel(GtkWidget *widget, GdkEventScroll *event)
{
    if(m_iButton == 0)
    {
        double dRatio = 1.0;
        if(event->direction == GDK_SCROLL_UP) dRatio = exp(0.5);
        else if(event->direction == GDK_SCROLL_DOWN)  dRatio = exp(-0.5);

        CDPoint cPt = {event->x, event->y};

        m_cViewOrigin.x = cPt.x + (m_cViewOrigin.x - cPt.x)*dRatio;
        m_cViewOrigin.y = cPt.y + (m_cViewOrigin.y - cPt.y)*dRatio;
        m_dUnitScale *= dRatio;

        gdk_window_invalidate_rect(event->window, NULL, FALSE);
    }
    return;
}

void CDApplication::MouseLButtonDown(GtkWidget *widget, GdkEventButton *event)
{
    if(m_iButton > 0) return;

    m_cLastDownPt.x = event->x;
    m_cLastDownPt.y = event->y;
    m_cLastMovePt.x = event->x;
    m_cLastMovePt.y = event->y;
    m_iButton = 1;
    return;
}

gboolean CDApplication::GetUpdateRegion(PDPtrList pPolygons, GdkRectangle *pRect)
{
    int iCnt = pPolygons->GetCount();
    if(iCnt < 1) return FALSE;

    PDPolygon pPoly;
    bool bFound = false;
    int i = 0;
    while(!bFound && (i < iCnt))
    {
        pPoly = (PDPolygon)pPolygons->GetItem(i++);
        bFound = (pPoly->iPoints > 0);
    }

    if(!bFound) return FALSE;

    pRect->x = Round(m_cViewOrigin.x + m_dUnitScale*pPoly->pPoints[0].x);
    pRect->y = Round(m_cViewOrigin.y + m_dUnitScale*pPoly->pPoints[0].y);
    pRect->width = pRect->x;
    pRect->height = pRect->y;

    gint ix, iy;

    for(int k = 1; k < pPoly->iPoints; k++)
    {
        ix = Round(m_cViewOrigin.x + m_dUnitScale*pPoly->pPoints[k].x);
        iy = Round(m_cViewOrigin.y + m_dUnitScale*pPoly->pPoints[k].y);
        if(ix < pRect->x) pRect->x = ix;
        if(ix > pRect->width) pRect->width = ix;
        if(iy < pRect->y) pRect->y = iy;
        if(iy > pRect->height) pRect->height = iy;
    }

    while(i < iCnt)
    {
        pPoly = (PDPolygon)pPolygons->GetItem(i++);

        for(int k = 0; k < pPoly->iPoints; k++)
        {
            ix = Round(m_cViewOrigin.x + m_dUnitScale*pPoly->pPoints[k].x);
            iy = Round(m_cViewOrigin.y + m_dUnitScale*pPoly->pPoints[k].y);
            if(ix < pRect->x) pRect->x = ix;
            if(ix > pRect->width) pRect->width = ix;
            if(iy < pRect->y) pRect->y = iy;
            if(iy > pRect->height) pRect->height = iy;
        }
    }

    pRect->width -= pRect->x;
    pRect->height -= pRect->y;

    /*pRect->x -= 5;
    pRect->y -= 5;
    pRect->width += 10;
    pRect->height += 10;*/

    return TRUE;
}

void CDApplication::MouseLButtonUp(GtkWidget *widget, GdkEventButton *event)
{
    if(m_iButton != 1) return;

    m_iButton = 0;

    int iWidth = gdk_window_get_width(event->window);
    int iHeight = gdk_window_get_height(event->window);

    CDRect cdr;
    cdr.cPt1.x = -m_cViewOrigin.x/m_dUnitScale;
    cdr.cPt1.y = -m_cViewOrigin.y/m_dUnitScale;
    cdr.cPt2.x = (iWidth - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (iHeight - m_cViewOrigin.y)/m_dUnitScale;

    double dTol = (double)m_iSnapTolerance/m_dUnitScale;
    CDPoint cDistPt;
    gchar sBuf[128];
    double dNorm;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);

    double xPos = event->x;
    double yPos = event->y;
    GdkRectangle cRect;

    gboolean bUpdate = FALSE;

    if(m_iDrawMode + m_iToolMode < 1) // selection
    {
        if(!(event->state & GDK_CONTROL_MASK)) m_pDrawObjects->ClearSelection(pRegions);

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
                m_pHighObject->SetSelected(true, event->state & GDK_CONTROL_MASK, m_iHighDimen, pRegions);
        }
        
        if(GetUpdateRegion(pRegions, &cRect))
            gdk_window_invalidate_rect(event->window, &cRect, FALSE);
    }
    else if((m_iToolMode > 20) && (m_iToolMode != tolRound))
    {
        if(GetPtDist(&m_cLastDownPt, xPos, yPos) > 4)
        {
            delete pRegions;
            return;
        }

        switch(m_iToolMode)
        {
        case tolKnife:
            if(m_pDrawObjects->CutSelected(m_cLastDrawPt, dTol, &cdr, pRegions))
            {
                bUpdate = TRUE;
                if(GetUpdateRegion(pRegions, &cRect))
                    gdk_window_invalidate_rect(event->window, &cRect, FALSE);
                SetTitle(m_pMainWnd, false);
            }
            break;
        case tolExtend:
            if(m_pDrawObjects->ExtendSelected(m_cLastDrawPt, dTol, &cdr, pRegions))
            {
                bUpdate = TRUE;
                if(GetUpdateRegion(pRegions, &cRect))
                    gdk_window_invalidate_rect(event->window, &cRect, FALSE);
                SetTitle(m_pMainWnd, false);
            }
            break;
        case tolConflict:
            if(m_pDrawObjects->SetCrossSelected(m_cLastDrawPt, dTol, &cdr, pRegions))
            {
                bUpdate = TRUE;
                if(GetUpdateRegion(pRegions, &cRect))
                    gdk_window_invalidate_rect(event->window, &cRect, FALSE);
                SetTitle(m_pMainWnd, false);
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
                dNorm = GetNorm(cDistPt);
                if(!m_bPaperUnits)
                {
                    cDistPt /= m_dDrawScale;
                    dNorm /= m_dDrawScale;
                }
                sprintf(sBuf, "dx: %.3f, dy: %.3f, dist: %.4f", fabs(cDistPt.x),
                    fabs(cDistPt.y), dNorm);
                SetStatusBarMsg(1, sBuf);
            }
            else
            {
                m_cMeasPoint1.bIsSet = true;
                m_cMeasPoint1.cOrigin = m_cLastDrawPt;
                m_cMeasPoint2.bIsSet = false;
                SetStatusBarMsg(1, "");
            }
            break;
        }
    }
    else
    {
        if(GetPtDist(&m_cLastDownPt, xPos, yPos) > 4)
        {
            delete pRegions;
            return;
        }

        float f;
        int i;
        bool bdValSet = false;
        double dVal = 0;
        int iCop = 0;
        PDObject pSelLine = NULL;
        CDLine cLine;

        if((m_iToolMode > tolCopyPar) && (m_iToolMode < tolMirror))
        {
            if(sscanf(gtk_entry_get_text(GTK_ENTRY(m_pStatEdt1)), "%f", &f) == 1)
            {
                dVal = f;
                bdValSet = true;
            }
            if(sscanf(gtk_entry_get_text(GTK_ENTRY(m_pStatEdt2)), "%d", &i) == 1) iCop = i;
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
                        bUpdate = TRUE;
                        if(GetUpdateRegion(pRegions, &cRect))
                            gdk_window_invalidate_rect(event->window, &cRect, FALSE);
                        SetTitle(m_pMainWnd, false);
                        StartNewObject(TRUE);
                    }
                }
                m_iToolMode = 0;
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
                            bUpdate = TRUE;
                            if(GetUpdateRegion(pRegions, &cRect))
                                gdk_window_invalidate_rect(event->window, &cRect, FALSE);
                            SetTitle(m_pMainWnd, false);
                            StartNewObject(TRUE);
                        }
                    }
                }
                else
                {
                    m_cMeasPoint1.cOrigin = m_cLastDrawPt;
                    m_cMeasPoint1.bIsSet = true;
                    strcpy(m_sStatus3Msg, _("Click a point to move to"));
                    SetStatusBarMsg(3, m_sStatus3Msg);
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
                    bUpdate = TRUE;
                    if(GetUpdateRegion(pRegions, &cRect))
                        gdk_window_invalidate_rect(event->window, &cRect, FALSE);
                    SetTitle(m_pMainWnd, false);
                    StartNewObject(TRUE);
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
                        bUpdate = TRUE;
                        if(GetUpdateRegion(pRegions, &cRect))
                            gdk_window_invalidate_rect(event->window, &cRect, FALSE);
                        SetTitle(m_pMainWnd, false);
                        StartNewObject(TRUE);
                    }
                }
            }
            else if(m_cMeasPoint1.bIsSet)
            {
                m_cMeasPoint2.bIsSet = true;
                m_cMeasPoint2.cOrigin = m_cLastDrawPt;
                strcpy(m_sStatus3Msg, _("Click a point to rotate to"));
                SetStatusBarMsg(3, m_sStatus3Msg);
            }
            else
            {
                m_cMeasPoint1.bIsSet = true;
                m_cMeasPoint1.cOrigin = m_cLastDrawPt;
                strcpy(m_sStatus3Msg, _("Click a point to rotate from"));
                SetStatusBarMsg(3, m_sStatus3Msg);
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
                    bUpdate = TRUE;
                    if(GetUpdateRegion(pRegions, &cRect))
                        gdk_window_invalidate_rect(event->window, &cRect, FALSE);
                    SetTitle(m_pMainWnd, false);
                    StartNewObject(TRUE);
                }
            }
        }
        else if(m_iToolMode == tolDimen)
        {
            if(m_pDrawObjects->AddDimen(m_pSelForDimen, m_cLastDrawPt, dTol, &cdr, pRegions))
            {
                bUpdate = TRUE;
                if(GetUpdateRegion(pRegions, &cRect))
                    gdk_window_invalidate_rect(event->window, &cRect, FALSE);
                SetTitle(m_pMainWnd, false);
            }
        }
        else
        {
            int iCtrl = 0;
            if(m_iToolMode == tolCopyPar)
            {
                iCtrl = 2;
                if(event->state & GDK_SHIFT_MASK) iCtrl = 3;
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
                SetTitle(m_pMainWnd, false);
                m_pActiveObject = NULL;
                m_iToolMode = tolNone;
                m_sStatus1Base[0] = 0;
                strcpy(m_sStatus1Msg, m_sStatus1Base);
                SetStatusBarMsg(1, m_sStatus1Msg);

                bUpdate = TRUE;
                if(GetUpdateRegion(pRegions, &cRect))
                    gdk_window_invalidate_rect(event->window, &cRect, FALSE);
                StartNewObject(TRUE);
            }
        }
    }

    if(bUpdate)
    {
        GtkWidget *draw = GetDrawing();
        cairo_t *cr = gdk_cairo_create(draw->window);

        if(m_pcs)
        {
            cairo_set_source_surface(cr, m_pcs, 0, 0);
            cairo_mask(cr, m_pcp);
        }

        if(m_iDrawMode > modSelect) DrawCross(cr);
        cairo_destroy(cr);
    }

    ClearPolygonList(pRegions);
    delete pRegions;

    m_cLastDynPt.bIsSet = false;
    return;
}

void CDApplication::MouseMButtonDown(GtkWidget *widget, GdkEventButton *event)
{
    if(m_iButton > 0) return;

    m_iButton = 2;
    m_cZoomOrig.x = event->x - m_cViewOrigin.x;
    m_cZoomOrig.y = event->y - m_cViewOrigin.y;
    m_cLastMovePt.x = event->x;
    m_cLastMovePt.y = event->y;
    m_cLastDownPt.x = event->x;
    m_cLastDownPt.y = event->y;

    gdk_pointer_grab(event->window, FALSE, (GdkEventMask)(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK),
        NULL, NULL, GDK_CURRENT_TIME);

    m_bRenderDirect = TRUE;
    return;
}

void CDApplication::MouseMButtonUp(GtkWidget *widget, GdkEventButton *event)
{
    if(m_iButton == 2)
    {
        m_bRenderDirect = FALSE;
        gdk_pointer_ungrab(GDK_CURRENT_TIME);
        gdk_window_invalidate_rect(event->window, NULL, FALSE);
    }
    m_iButton = 0;
    return;
}

void CDApplication::MouseRButtonDown(GtkWidget *widget, GdkEventButton *event)
{
    if(m_iButton > 0) return;

    m_cLastDownPt.x = event->x;
    m_cLastDownPt.y = event->y;
    m_cLastMovePt.x = event->x;
    m_cLastMovePt.y = event->y;
    m_iButton = 3;

    return;
}

void CDApplication::MouseRButtonUp(GtkWidget *widget, GdkEventButton *event)
{
    if(m_iButton != 3) return;

    m_iButton = 0;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);

    double xPos = event->x;
    double yPos = event->y;
    GdkRectangle cRect;

    gboolean bUpdate = FALSE;

    if(m_iDrawMode + m_iToolMode < 1) // selection
    {
        if(GetPtDist(&m_cLastDownPt, xPos, yPos) > 4) // select by rectangle
        {
            if(!(event->state & GDK_CONTROL_MASK)) m_pDrawObjects->ClearSelection(pRegions);
            CDRect cdr1;
            cdr1.cPt1.x = (m_cLastDownPt.x - m_cViewOrigin.x)/m_dUnitScale;
            cdr1.cPt1.y = (m_cLastDownPt.y - m_cViewOrigin.y)/m_dUnitScale;
            cdr1.cPt2.x = (xPos - m_cViewOrigin.x)/m_dUnitScale;
            cdr1.cPt2.y = (yPos - m_cViewOrigin.y)/m_dUnitScale;
            m_pDrawObjects->SelectByRectangle(&cdr1, 1, pRegions);

            bUpdate = TRUE;
            if(GetUpdateRegion(pRegions, &cRect))
                gdk_window_invalidate_rect(event->window, &cRect, FALSE);
        }
        else
        {
            if(m_pHighObject)
            {
                GtkMenu *pMenu;
                if(m_pHighObject->GetSnapTo()) pMenu = GTK_MENU(m_pSnapDisableMnu);
                else pMenu = GTK_MENU(m_pSnapEnableMnu);
                gtk_menu_popup(pMenu, NULL, NULL, NULL, this, event->button, event->time);
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
            return;
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
            SetTitle(m_pMainWnd, false);
            m_pActiveObject = NULL;
            m_iToolMode = tolNone;
            m_sStatus1Base[0] = 0;
            strcpy(m_sStatus1Msg, m_sStatus1Base);
            SetStatusBarMsg(1, m_sStatus1Msg);

            bUpdate = TRUE;
            if(GetUpdateRegion(pRegions, &cRect))
                gdk_window_invalidate_rect(event->window, &cRect, FALSE);
            StartNewObject(TRUE);
        }
    }

    if(bUpdate)
    {
        GtkWidget *draw = GetDrawing();
        cairo_t *cr = gdk_cairo_create(draw->window);

        if(m_pcs)
        {
            cairo_set_source_surface(cr, m_pcs, 0, 0);
            cairo_mask(cr, m_pcp);
        }

        if(m_iDrawMode > modSelect) DrawCross(cr);
        cairo_destroy(cr);
    }

    ClearPolygonList(pRegions);
    delete pRegions;

    m_cLastDynPt.bIsSet = false;
    return;
}

void CDApplication::MouseLDblClick(GtkWidget *widget, GdkEventButton *event)
{
    if(m_iDrawMode > modSelect)
    {
        if(m_pActiveObject)
        {
            if(m_pActiveObject->HasEnoughPoints())
            {
                GdkRectangle cRect;
                PDPtrList pRegions = new CDPtrList();
                pRegions->SetDblVal(m_dUnitScale);

                m_pActiveObject->AddRegions(pRegions, -1);
                m_pDrawObjects->Add(m_pActiveObject);
                SetTitle(m_pMainWnd, false);
                m_pActiveObject = NULL;

                if(GetUpdateRegion(pRegions, &cRect))
                    gdk_window_invalidate_rect(event->window, &cRect, FALSE);

                ClearPolygonList(pRegions);
                delete pRegions;
                StartNewObject(TRUE);
            }
        }
    }
    else if(m_iDrawMode + m_iToolMode < 1)
    {
        EditLineStyleCmd(m_pMainWnd);
    }
    m_iButton = 0;
    return;
}

void CDApplication::Edit1Changed(GtkEntry *entry)
{
    const gchar *sBuf = gtk_entry_get_text(entry);

    // Ugly hack for Ubuntu
    if((strcmp(sBuf, "l") == 0) || (strcmp(sBuf, "c") == 0) || (strcmp(sBuf, "r") == 0))
    {
        gtk_entry_set_text(entry, "");
        return;
    }

    int iOldRest = m_iRestrictSet;

    m_iRestrictSet = ParseInputString((char*)sBuf, m_pFileSetupDlg->GetUnitList(), &m_dRestrictValue);

    GdkEventMotion event;
    GtkWidget *draw = GetDrawing();
    event.window = draw->window;
    event.x = m_cLastMovePt.x;
    event.y = m_cLastMovePt.y;
    event.state = 0;
    MouseMove(draw, &event, TRUE);

    if((m_iToolMode == tolMove) && !m_cMeasPoint1.bIsSet && (iOldRest != m_iRestrictSet))
    {
        if(IS_LENGTH_VAL(m_iRestrictSet))
            strcpy(m_sStatus3Msg, _("Click a line to move along"));
        else strcpy(m_sStatus3Msg, _("Click a point to move from"));
        SetStatusBarMsg(3, m_sStatus3Msg);
    }
    return;
}

void CDApplication::EnableSnap()
{
    if(m_pHighObject) m_pHighObject->SetSnapTo(true);
}

void CDApplication::DisableSnap()
{
    if(m_pHighObject) m_pHighObject->SetSnapTo(false);
}

void CDApplication::ToolsBreakCmd()
{
    GtkWidget *draw = GetDrawing();

    int iWidth = gdk_window_get_width(draw->window);
    int iHeight = gdk_window_get_height(draw->window);

    CDRect cdr;
    cdr.cPt1.x = -m_cViewOrigin.x/m_dUnitScale;
    cdr.cPt1.y = -m_cViewOrigin.y/m_dUnitScale;
    cdr.cPt2.x = (iWidth - m_cViewOrigin.x)/m_dUnitScale;
    cdr.cPt2.y = (iHeight - m_cViewOrigin.y)/m_dUnitScale;

    PDPtrList pRegions = new CDPtrList();
    pRegions->SetDblVal(m_dUnitScale);

    m_pDrawObjects->BreakSelObjects(&cdr, pRegions);
    {
        GdkRectangle cRect;
        if(GetUpdateRegion(pRegions, &cRect))
            gdk_window_invalidate_rect(draw->window, &cRect, FALSE);
        SetTitle(m_pMainWnd, false);
    }

    ClearPolygonList(pRegions);
    delete pRegions;
}

void CDApplication::ToolsScaleCmd()
{
    CDFileAttrs cFA;
    m_pDrawObjects->GetFileAttrs(&cFA);

    CDScaleRec cSR;
    cSR.bScaleDraw = FALSE;
    cSR.dScaleNom = cFA.dScaleNom;
    cSR.dScaleDenom = cFA.dScaleDenom;
    cSR.bScaleWidth = TRUE;
    cSR.bScalePattern = TRUE;
    cSR.bScaleArrows = TRUE;
    cSR.bScaleLabels = TRUE;
    cSR.bChangeDimUnits = FALSE;

    gchar sMaskBuf[64];

    int iRes = m_pDrawObjects->GetUnitMask(1, sMaskBuf, m_pFileSetupDlg->GetUnitList());
    if(iRes < 0) strcpy(cSR.sLenMask, m_cFSR.sLengthMask);
    else strcpy(cSR.sLenMask, sMaskBuf);

    iRes = m_pDrawObjects->GetUnitMask(2, sMaskBuf, m_pFileSetupDlg->GetUnitList());
    if(iRes < 0) strcpy(cSR.sAngMask, m_cFSR.sAngleMask);
    else strcpy(cSR.sAngMask, sMaskBuf);

    if(m_pScaleDlg->ShowDialog(m_pMainWnd, &cSR))
    {
        gboolean bRedraw = FALSE;
        if(cSR.bScaleDraw)
        {
            bRedraw = TRUE;
            m_pDrawObjects->RescaleDrawing(cSR.dScaleNom, cSR.dScaleDenom,
                cSR.bScaleWidth, cSR.bScalePattern, cSR.bScaleArrows, cSR.bScaleLabels);
            m_pUndoObjects->ClearAll();
            DataToFileProps();
        }
        if(cSR.bChangeDimUnits)
        {
            bRedraw = TRUE;
            m_pDrawObjects->ChangeUnitMask(1, cSR.sLenMask, m_pFileSetupDlg->GetUnitList());
            m_pDrawObjects->ChangeUnitMask(2, cSR.sAngMask, m_pFileSetupDlg->GetUnitList());
        }
        if(bRedraw)
        {
            GtkWidget *draw = GetDrawing();
            gdk_window_invalidate_rect(draw->window, NULL, FALSE);
        }
    }
    return;
}

