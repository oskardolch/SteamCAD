#include <string.h>
#include <gdk/gdkkeysyms.h>
#include "DLxBaseDefs.h"
#include "DEditMenu.hpp"
#include "DApplication.hpp"

static void edit_del_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITDELETE, true);
	return;
}

static void edit_return_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITCONFIRM, true);
	return;
}

static void edit_cppar_click(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITCOPYPAR, false);
	return;
}

static void edit_cppar_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITCOPYPAR, true);
	return;
}

static void edit_move_click(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITMOVE, false);
	return;
}

static void edit_move_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITMOVE, true);
	return;
}

static void edit_rotate_click(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITROTATE, false);
	return;
}

static void edit_rotate_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITROTATE, true);
	return;
}

static void edit_mirror_click(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITMIRROR, false);
	return;
}

static void edit_mirror_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITMIRROR, true);
	return;
}

static void edit_lnstyle_click(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITLINESTYLE, false);
	return;
}

static void edit_lnstyle_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITLINESTYLE, true);
	return;
}

static void edit_tglsnap_click(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITTOGGLESNAP, false);
	return;
}

static void edit_tglsnap_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITTOGGLESNAP, true);
	return;
}

static void edit_papunits_click(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITPAPERUNITS, false);
	return;
}

static void edit_papunits_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITPAPERUNITS, true);
	return;
}

static void edit_undo_click(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITUNDO, false);
	return;
}

static void edit_undo_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITUNDO, true);
	return;
}

static void edit_redo_click(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITREDO, false);
	return;
}

static void edit_redo_accel(PDApplication pApp)
{
    pApp->EditCommand(IDM_EDITREDO, true);
	return;
}

void CreateEditMenu(void *pPtr, GtkMenuShell *pMenuBar, GtkAccelGroup *pAccel)
{
    PDApplication pApp = (PDApplication)pPtr;

	GtkMenuShell *menu = (GtkMenuShell*)gtk_menu_new();
	
	GtkWidget *menu_item;
    GClosure *pClos;
    GtkWidget *menu_label;

    pClos = g_cclosure_new_swap(G_CALLBACK(edit_del_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_KEY_Delete, (GdkModifierType)0, GTK_ACCEL_MASK, pClos);
	
    pClos = g_cclosure_new_swap(G_CALLBACK(edit_return_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_KEY_Return, (GdkModifierType)0, GTK_ACCEL_MASK, pClos);
	
	menu_item = gtk_menu_item_new_with_mnemonic(_("_Copy paralel"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    //gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item), TRUE);

    pClos = g_cclosure_new_swap(G_CALLBACK(edit_cppar_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_C, GDK_CONTROL_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
	g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(edit_cppar_click), pApp);
	gtk_widget_show(menu_item);
	
	menu_item = gtk_menu_item_new_with_mnemonic(_("_Move"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(edit_move_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_M, GDK_MOD1_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
	g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(edit_move_click), pApp);
	gtk_widget_show(menu_item);
	
	menu_item = gtk_menu_item_new_with_mnemonic(_("_Rotate"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(edit_rotate_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_R, GDK_CONTROL_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
	g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(edit_rotate_click), pApp);
	gtk_widget_show(menu_item);
	
	menu_item = gtk_menu_item_new_with_mnemonic(_("M_irror"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(edit_mirror_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_I, GDK_MOD1_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
	g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(edit_mirror_click), pApp);
	gtk_widget_show(menu_item);
	
	menu_item = gtk_menu_item_new_with_mnemonic(_("Line _style"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(edit_lnstyle_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_S, GDK_MOD1_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
	g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(edit_lnstyle_click), pApp);
	gtk_widget_show(menu_item);
	
	menu_item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

	menu_item = gtk_menu_item_new_with_mnemonic(_("Toggle s_nap"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(edit_tglsnap_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_N, GDK_MOD1_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
	g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(edit_tglsnap_click), pApp);
	gtk_widget_show(menu_item);
	
	menu_item = gtk_check_menu_item_new_with_mnemonic(_("Paper units"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(edit_papunits_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, GDK_P, GDK_CONTROL_MASK, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
	g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(edit_papunits_click), pApp);
	gtk_widget_show(menu_item);
	
	menu_item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show(menu_item);

    GtkStockItem gtkSI;

	if(gtk_stock_lookup(GTK_STOCK_UNDO, &gtkSI))
	{
		menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_UNDO, NULL);
	}
	else
	{
		menu_item = gtk_menu_item_new_with_mnemonic(_("_Undo"));
	}
    gtkSI.keyval = GDK_Z;
    gtkSI.modifier = GDK_CONTROL_MASK;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(edit_undo_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, gtkSI.keyval, gtkSI.modifier, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
	g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(edit_undo_click), pApp);
	gtk_widget_show(menu_item);

	if(gtk_stock_lookup(GTK_STOCK_REDO, &gtkSI))
	{
		menu_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_REDO, NULL);
	}
	else
	{
		menu_item = gtk_menu_item_new_with_mnemonic(_("Re_do"));
	}
    gtkSI.keyval = GDK_Z;
    gtkSI.modifier = (GdkModifierType)(GDK_CONTROL_MASK | GDK_SHIFT_MASK);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);

    pClos = g_cclosure_new_swap(G_CALLBACK(edit_redo_accel), pApp, NULL);
    gtk_accel_group_connect(pAccel, gtkSI.keyval, gtkSI.modifier, GTK_ACCEL_MASK, pClos);
    menu_label = gtk_bin_get_child(GTK_BIN(menu_item));
    gtk_accel_label_set_accel_closure(GTK_ACCEL_LABEL(menu_label), pClos);
	g_signal_connect_swapped(G_OBJECT(menu_item), "activate", G_CALLBACK(edit_redo_click), pApp);
	gtk_widget_show(menu_item);

	char buf[128];
	strcpy(buf, _("_Edit"));
	GtkWidget *edit_menu = gtk_menu_item_new_with_mnemonic(buf);
	gtk_widget_show(edit_menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_menu), GTK_WIDGET(menu));
	gtk_menu_shell_append(pMenuBar, edit_menu);
}

