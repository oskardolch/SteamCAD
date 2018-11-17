#include <string.h>
#include <gdk/gdkkeysyms.h>
#include "DLxBaseDefs.h"
#include "DViewToolsMenu.hpp"
#include "DApplication.hpp"

static void view_fitall_click(PDApplication pApp)
{
    pApp->ViewCommand(IDM_VIEWFITALL, false);
    return;
}

static void view_actsize_click(PDApplication pApp)
{
    pApp->ViewCommand(IDM_VIEWACTSIZE, false);
    return;
}

static void tools_knife_click(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSKNIFE, false);
    return;
}

static void tools_knife_accel(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSKNIFE, true);
    return;
}

static void tools_break_click(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSBREAK, false);
    return;
}

static void tools_break_accel(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSBREAK, true);
    return;
}

static void tools_round_click(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSROUND, false);
    return;
}

static void tools_round_accel(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSROUND, true);
    return;
}

static void tools_extend_click(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSEXTEND, false);
    return;
}

static void tools_extend_accel(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSEXTEND, true);
    return;
}

static void tools_mkconflicts_click(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSCONFLICTS, false);
    return;
}

static void tools_mkconflicts_accel(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSCONFLICTS, true);
    return;
}

static void tools_measdist_click(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSMEASURE, false);
    return;
}

static void tools_measdist_accel(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSMEASURE, true);
    return;
}

static void tools_rescale_click(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSCALE, false);
    return;
}

static void tools_stats_click(PDApplication pApp)
{
    pApp->ToolsCommand(IDM_TOOLSTAT, false);
    return;
}


void CreateViewMenu(void *pPtr, GtkMenuShell *pMenuBar, GtkAccelGroup *pAccel)
{
    PDApplication pApp = (PDApplication)pPtr;

    GtkMenuShell *menu = (GtkMenuShell*)gtk_menu_new();

    GtkWidget *menu_item;
    GtkStockItem gtkSI;

    if(gtk_stock_lookup(GTK_STOCK_ZOOM_FIT, &gtkSI))
    {
        menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_FIT, pAccel);
    }
    else
    {
        menu_item = gtk_menu_item_new_with_mnemonic(_("Best _Fit"));
    }
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(view_fitall_click), pApp);
    gtk_widget_show(menu_item);

    if(gtk_stock_lookup(GTK_STOCK_ZOOM_100, &gtkSI))
    {
        menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_100, pAccel);
    }
    else
    {
        menu_item = gtk_menu_item_new_with_mnemonic(_("_Normal Size"));
    }
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(view_actsize_click), pApp);
    gtk_widget_show(menu_item);

    char buf[128];
    strcpy(buf, _("_View"));
    GtkWidget *view_menu = gtk_menu_item_new_with_mnemonic(buf);
    gtk_widget_show(view_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menu), GTK_WIDGET(menu));
    gtk_menu_shell_append(pMenuBar, view_menu);
    return;
}

void CreateToolsMenu(void *pPtr, GtkMenuShell *pMenuBar, GtkAccelGroup *pAccel)
{
    PDApplication pApp = (PDApplication)pPtr;

    GtkMenuShell *menu = (GtkMenuShell*)gtk_menu_new();

    GtkWidget *menu_item;
    GClosure *pClos;
    GtkWidget *menu_label;

    menu_item = gtk_menu_item_new_with_mnemonic(_("_Knife"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(tools_knife_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_K, GDK_MOD1_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
    g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(tools_knife_click), pApp);
    gtk_widget_show(menu_item);

    menu_item = gtk_menu_item_new_with_mnemonic(_("_Break apart"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(tools_break_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_B, GDK_MOD1_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
    g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(tools_break_click), pApp);
    gtk_widget_show(menu_item);

    menu_item = gtk_menu_item_new_with_mnemonic(_("_Round"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(tools_round_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_B, GDK_CONTROL_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
    g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(tools_round_click), pApp);
    gtk_widget_show(menu_item);

    menu_item = gtk_menu_item_new_with_mnemonic(_("_Extend"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(tools_extend_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_E, GDK_CONTROL_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
    g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(tools_extend_click), pApp);
    gtk_widget_show(menu_item);

    menu_item = gtk_menu_item_new_with_mnemonic(_("Mark con_flicts"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(tools_mkconflicts_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_F, GDK_CONTROL_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
    g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(tools_mkconflicts_click), pApp);
    gtk_widget_show(menu_item);

    menu_item = gtk_menu_item_new_with_mnemonic(_("Measure _distance"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(tools_measdist_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_D, GDK_MOD1_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
    g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(tools_measdist_click), pApp);
    gtk_widget_show(menu_item);

    menu_item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    gtk_widget_show(menu_item);

    menu_item = gtk_menu_item_new_with_mnemonic(_("Re_scale and Units"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(tools_rescale_click), pApp);
    gtk_widget_show(menu_item);

    menu_item = gtk_menu_item_new_with_mnemonic(_("S_tatistics"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(tools_stats_click), pApp);
    gtk_widget_show(menu_item);

    char buf[128];
    strcpy(buf, _("_Tools"));
    GtkWidget *tools_menu = gtk_menu_item_new_with_mnemonic(buf);
    gtk_widget_show(tools_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(tools_menu), GTK_WIDGET(menu));
    gtk_menu_shell_append(pMenuBar, tools_menu);
    return;
}

