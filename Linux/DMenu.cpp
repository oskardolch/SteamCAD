#include <string.h>
#include <dlfcn.h>
#include <gio/gio.h>
#include <gdk/gdkkeysyms.h>
#include "DApplication.hpp"
#include "DMenu.hpp"
#include "DFileMenu.hpp"
#include "DModeMenu.hpp"
#include "DEditMenu.hpp"
#include "DViewToolsMenu.hpp"

GtkWidget* GetMenuGroup(GtkWidget *pMenuBar, char *sName)
{
    GtkWidget *pRes = NULL;
    GList *list = gtk_container_get_children((GtkContainer*)pMenuBar);
    bool bFound = false;
    guint i = 0;
    GtkMenuItem *mi = NULL;
    GList *pList;
    GtkLabel *pLab;
    const gchar *sLab;
    while(!bFound && (i < g_list_length(list)))
    {
        mi = (GtkMenuItem*)g_list_nth_data(list, i++);
        pList = gtk_container_get_children(GTK_CONTAINER(mi));
        pLab = (GtkLabel*)g_list_first(pList)->data;
        sLab = gtk_label_get_label(pLab);
        bFound = strcmp(sName, sLab) == 0;
    }

    if(bFound)
    {
        pRes = gtk_menu_item_get_submenu(mi);
    }
    else
    {
        pRes = gtk_menu_new();
        GtkWidget *group_menu = gtk_menu_item_new_with_mnemonic(sName);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(group_menu), pRes);
        gtk_widget_show(group_menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(pMenuBar), group_menu);
    }
    return(pRes);
}

/*static void escape_cmd(PDApplication pApp)
{
    pApp->DoEscape();
    return;
}*/

GtkAccelGroup* InitMenu(GtkWidget *wBox, GtkAccelGroup *pAccelGroup, void *pPtr)
{
    //PDApplication pApp = (PDApplication)pPtr;

    GtkAccelGroup *acg = gtk_accel_group_new();
    //GClosure *pClos = g_cclosure_new_swap(G_CALLBACK(escape_cmd), pApp, NULL);
    //gtk_accel_group_connect(acg, GDK_Escape, GdkModifierType(0), 
    //    GTK_ACCEL_MASK, pClos);

    /* Create a menu-bar to hold the menus and add it to our main window */
    GtkWidget *menu_bar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(wBox), menu_bar, FALSE, FALSE, 0);

    GtkWidget *stat_bar = gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(wBox), stat_bar, FALSE, FALSE, 0);

    CreateFileMenu(pPtr, GTK_MENU_SHELL(menu_bar), pAccelGroup);
    CreateModeMenu(pPtr, GTK_MENU_SHELL(menu_bar), pAccelGroup, acg);
    CreateEditMenu(pPtr, GTK_MENU_SHELL(menu_bar), pAccelGroup, acg);
    CreateViewMenu(pPtr, GTK_MENU_SHELL(menu_bar), pAccelGroup);
    CreateToolsMenu(pPtr, GTK_MENU_SHELL(menu_bar), pAccelGroup);

    gtk_widget_show(menu_bar);
    gtk_widget_show(stat_bar);

    return acg;
}

