#include "DLineStyleDlg.hpp"
#include "DLxBaseDefs.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

gboolean lsd_configure(GtkWidget *widget, GdkEvent *event, PDLineStyleDlg pApp)
{
    return pApp->Configure(widget, event);
}

void lsd_okbtn_clicked(GtkButton *button, PDLineStyleDlg pApp)
{
    pApp->OKBtnClick(button);
}

void lsd_linewidthedt_changed(GtkEntry *entry, PDLineStyleDlg pApp)
{
    pApp->LineWidthChange(entry);
}

void lsd_excedt_changed(GtkEntry *entry, PDLineStyleDlg pApp)
{
    pApp->LineExcChange(entry);
}

void lsd_patedt_changed(GtkEntry *entry, PDLineStyleDlg pApp)
{
    pApp->LinePatChange(entry);
}



// CDFileSetupDlg

CDLineStyleDlg::CDLineStyleDlg()
{
    m_iX = -100;
    m_iY = -100;
    m_bSettingUp = FALSE;
}

CDLineStyleDlg::~CDLineStyleDlg()
{
}

gboolean CDLineStyleDlg::ShowDialog(GtkWidget *pWndParent, PDLineStyleRec pLSR)
{
    m_pLSR = pLSR;

    m_bSettingUp = TRUE;

    m_pDlg = gtk_dialog_new_with_buttons(_("Line Style"), GTK_WINDOW(pWndParent),
        GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    g_signal_connect(G_OBJECT(m_pDlg), "configure-event", G_CALLBACK(lsd_configure), this);

    GtkWidget *pAA = gtk_dialog_get_action_area(GTK_DIALOG(m_pDlg));
    GtkWidget *pBtn = gtk_button_new_from_stock("gtk-ok");
    gtk_box_pack_end(GTK_BOX(pAA), pBtn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(pBtn), "clicked", G_CALLBACK(lsd_okbtn_clicked), this);
    gtk_widget_set_can_default(pBtn, TRUE);
    gtk_widget_show(pBtn);

    GtkWidget *pCA = gtk_dialog_get_content_area(GTK_DIALOG(m_pDlg));

    GtkWidget *pTbl = gtk_table_new(4, 6, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(pTbl), 3);
    gtk_table_set_row_spacings(GTK_TABLE(pTbl), 3);
    gtk_container_set_border_width(GTK_CONTAINER(pTbl), 4);
    gtk_container_add(GTK_CONTAINER(pCA), pTbl);
    gtk_widget_show(pTbl);

    GtkWidget *pLab = gtk_label_new(_("Line width:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 3, 0, 1);
    gtk_widget_show(pLab);

    m_pLineWidthEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pLineWidthEdt, 40, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pLineWidthEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pLineWidthEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pLineWidthEdt, 3, 5, 0, 1);
    g_signal_connect(G_OBJECT(m_pLineWidthEdt), "changed", G_CALLBACK(lsd_linewidthedt_changed), this);
    gtk_widget_show(m_pLineWidthEdt);

    gchar buf[32];
    FormatFloatStr(m_pLSR->cLineStyle.dWidth/m_pLSR->cUnit.dBaseToUnit, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pLineWidthEdt), buf);

    pLab = gtk_label_new(m_pLSR->cUnit.sAbbrev);
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 5, 6, 0, 1);
    gtk_widget_show(pLab);

    pLab = gtk_label_new(_("Eccentricity:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 3, 1, 2);
    gtk_widget_show(pLab);

    m_pEccentEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pEccentEdt, 40, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pEccentEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pEccentEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pEccentEdt, 3, 5, 1, 2);
    g_signal_connect(G_OBJECT(m_pEccentEdt), "changed", G_CALLBACK(lsd_excedt_changed), this);
    gtk_widget_show(m_pEccentEdt);

    FormatFloatStr(m_pLSR->cLineStyle.dPercent, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pEccentEdt), buf);

    pLab = gtk_label_new(_("%"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 5, 6, 1, 2);
    gtk_widget_show(pLab);

    pLab = gtk_label_new(_("Line pattern:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 3, 2, 3);
    gtk_widget_show(pLab);

    sprintf(buf, "(%s)", m_pLSR->cUnit.sAbbrev);
    pLab = gtk_label_new(buf);
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 3, 6, 2, 3);
    gtk_widget_show(pLab);

    for(gint i = 0; i < 6; i++)
    {
        m_pPatternEdt[i] = gtk_entry_new();
        gtk_widget_set_size_request(m_pPatternEdt[i], 20, -1);
        gtk_entry_set_max_length(GTK_ENTRY(m_pPatternEdt[i]), 32);
        gtk_entry_set_activates_default(GTK_ENTRY(m_pPatternEdt[i]), TRUE);
        gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pPatternEdt[i], i, i + 1, 3, 4);
        g_signal_connect(G_OBJECT(m_pPatternEdt[i]), "changed", G_CALLBACK(lsd_patedt_changed), this);
        gtk_widget_show(m_pPatternEdt[i]);

        if(m_pLSR->bPatSet && (i < m_pLSR->cLineStyle.iSegments))
        {
            FormatFloatStr(m_pLSR->cLineStyle.dPattern[i]/m_pLSR->cUnit.dBaseToUnit, buf);
            gtk_entry_set_text(GTK_ENTRY(m_pPatternEdt[i]), buf);
        }
    }

    gtk_widget_grab_default(pBtn);
    gtk_window_set_default(GTK_WINDOW(m_pDlg), pBtn);

    gtk_window_set_resizable(GTK_WINDOW(m_pDlg), FALSE);
    if(m_iX > -100) gtk_window_move(GTK_WINDOW(m_pDlg), m_iX, m_iY);

    m_bSettingUp = FALSE;

    gboolean bRes = (gtk_dialog_run(GTK_DIALOG(m_pDlg)) == GTK_RESPONSE_OK);

    if(m_iX < -90) gdk_window_get_position(m_pDlg->window, &m_iX, &m_iY);

    gtk_widget_destroy(m_pDlg);

    return bRes;
}

void CDLineStyleDlg::SaveSettings(FILE *fp)
{
    if(m_iX < -90) return;

    gchar sbuf[256];
	sprintf(sbuf, "  <LineStyleDlg Left=\"%d\" Top=\"%d\"/>\n", m_iX, m_iY);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);
    return;
}

void CDLineStyleDlg::RestoreSettings(gint iLeft, gint iTop)
{
    m_iX = iLeft;
    m_iY = iTop;
    return;
}

gboolean CDLineStyleDlg::Configure(GtkWidget *widget, GdkEvent *event)
{
    if(m_bSettingUp) return FALSE;

    m_iX = event->configure.x;
    m_iY = event->configure.y;
    return FALSE;
}

void CDLineStyleDlg::OKBtnClick(GtkButton *button)
{
    float f;
    GtkWidget *msg_dlg;

    if(m_pLSR->bWidthChanged)
    {
        if(sscanf(gtk_entry_get_text(GTK_ENTRY(m_pLineWidthEdt)), "%f", &f) == 1)
        {
            if(fabs(f) < 0.0001) f = 0.0;
            m_pLSR->cLineStyle.dWidth = f*m_pLSR->cUnit.dBaseToUnit;
            m_pLSR->bWidthSet = true;
        }
        else m_pLSR->bWidthSet = false;
    }

    if(m_pLSR->bExcChanged)
    {
        if(sscanf(gtk_entry_get_text(GTK_ENTRY(m_pEccentEdt)), "%f", &f) == 1)
        {
            if(fabs(f) < 100.0001)
            {
                m_pLSR->cLineStyle.dPercent = f;
                m_pLSR->bExcSet = true;
            }
            else
            {
                msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), GTK_DIALOG_MODAL,
                    GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, _("Eccentricity magnitude must not exceed 100"));
                gtk_dialog_run(GTK_DIALOG(msg_dlg));
                gtk_widget_destroy(msg_dlg);
                gtk_widget_grab_focus(m_pEccentEdt);
                return;
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
            if(sscanf(gtk_entry_get_text(GTK_ENTRY(m_pPatternEdt[i])), "%f", &f) == 1)
            {
                if(f < -0.0001)
                {
                    msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), GTK_DIALOG_MODAL,
                        GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, _("Line segment length must not be negative"));
                    gtk_dialog_run(GTK_DIALOG(msg_dlg));
                    gtk_widget_destroy(msg_dlg);
                    gtk_widget_grab_focus(m_pPatternEdt[i]);
                    return;
                }
                if(f > 0.0001)
                {
                    if(m_pLSR->cLineStyle.iSegments < i)
                    {
                        msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), GTK_DIALOG_MODAL,
                            GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, _("Invalid pattern sequence (zero or no-value must not precede a non-zero value)"));
                        gtk_dialog_run(GTK_DIALOG(msg_dlg));
                        gtk_widget_destroy(msg_dlg);
                        gtk_widget_grab_focus(m_pPatternEdt[i]);
                        return;
                    }

                    m_pLSR->cLineStyle.iSegments++;
                    m_pLSR->cLineStyle.dPattern[i] = f*m_pLSR->cUnit.dBaseToUnit;
                    m_pLSR->bExcSet = true;
                }
            }
        }
        if((m_pLSR->cLineStyle.iSegments % 2) > 0)
        {
            msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, _("The number of non-zero segments must be even"));
            gtk_dialog_run(GTK_DIALOG(msg_dlg));
            gtk_widget_destroy(msg_dlg);
            gtk_widget_grab_focus(m_pPatternEdt[0]);
            return;
        }
    }

    gtk_dialog_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_OK);
    return;
}

void CDLineStyleDlg::LineWidthChange(GtkEntry *entry)
{
    if(m_bSettingUp) return;
    m_pLSR->bWidthChanged = TRUE;
}

void CDLineStyleDlg::LineExcChange(GtkEntry *entry)
{
    if(m_bSettingUp) return;
    m_pLSR->bExcChanged = TRUE;
}

void CDLineStyleDlg::LinePatChange(GtkEntry *entry)
{
    if(m_bSettingUp) return;
    m_pLSR->bPatChanged = TRUE;
}

