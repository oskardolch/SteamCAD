#include "DFileSetupDlg.hpp"
#include "../Source/DDataTypes.hpp"
#include "DLxBaseDefs.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

double RoundFloat(double dx)
{
    double dRes = dx;
    double dRest = dx - (int)dx;
    if(dRest > 0.9999) dRes = (int)dx + 1;
    else if(dRest < 0.0001) dRes = (int)dx;
    return dRes;
}

void FormatFloatStr(double dx, gchar *sbuf)
{
    double dRest = fabs(dx - (int)dx);
    if((dRest > 0.0001) && (dRest < 0.9999)) sprintf(sbuf, "%.3f", dx);
    else sprintf(sbuf, "%d", (int)dx);
}

gboolean fsd_configure(GtkWidget *widget, GdkEvent *event, PDFileSetupDlg pApp)
{
    return pApp->Configure(widget, event);
}

void fsd_font_set(GtkFontButton *widget, PDFileSetupDlg pApp)
{
    pApp->FontSet(widget);
}

void fsd_okbtn_clicked(GtkButton *button, PDFileSetupDlg pApp)
{
    pApp->OkBtnClick(button);
}

void fsd_angunitcb_changed(GtkComboBox *widget, PDFileSetupDlg pApp)
{
    pApp->AngUnitCBChanged(widget);
}

void fsd_papunitcb_changed(GtkComboBox *widget, PDFileSetupDlg pApp)
{
    pApp->PaperUnitCBChanged(widget);
}

void fsd_graphunitcb_changed(GtkComboBox *widget, PDFileSetupDlg pApp)
{
    pApp->GraphUnitCBChanged(widget);
}

typedef struct CFsdGridEdtData
{
    PDFileSetupDlg pApp;
    gint id;
} *PFsdGridEdtData;

void fsd_gridedt_changed(GtkEntry *entry, PFsdGridEdtData pData)
{
    pData->pApp->GridEditChanged(entry, pData->id);
}

void fsd_fontattrchb_toggled(GtkToggleButton *togglebutton, PFsdGridEdtData pData)
{
    pData->pApp->FontAttrChBChange(togglebutton, pData->id);
}


// CDFileSetupDlg

CDFileSetupDlg::CDFileSetupDlg()
{
    m_iX = -100;
    m_iY = -100;
    m_pPaperSizes = NULL;
    m_iPaperSizes = 0;
    m_bSettingUp = false;
    m_pUnits = new CDUnitList();
    BuildLenUnitsList();
    BuildPaperSizeList();
}

CDFileSetupDlg::~CDFileSetupDlg()
{
    delete m_pUnits;
    if(m_pPaperSizes) g_free(m_pPaperSizes);
}

gboolean CDFileSetupDlg::ShowDialog(GtkWidget *pWndParent, PDFileSetupRec pFSR)
{
    m_bSettingUp = TRUE;

    m_pFSR = pFSR;
    memcpy(&m_cFSR, m_pFSR, sizeof(CDFileSetupRec));

    m_pDlg = gtk_dialog_new_with_buttons(_("File Setup"), GTK_WINDOW(pWndParent),
        GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    g_signal_connect(G_OBJECT(m_pDlg), "configure-event", G_CALLBACK(fsd_configure), this);

    GtkWidget *pAA = gtk_dialog_get_action_area(GTK_DIALOG(m_pDlg));
    GtkWidget *pBtn = gtk_button_new_from_stock("gtk-ok");
    gtk_box_pack_end(GTK_BOX(pAA), pBtn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(pBtn), "clicked", G_CALLBACK(fsd_okbtn_clicked), this);
    gtk_widget_set_can_default(pBtn, TRUE);
    gtk_widget_show(pBtn);

    GtkWidget *pCA = gtk_dialog_get_content_area(GTK_DIALOG(m_pDlg));

    GtkWidget *pBox1 = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pCA), pBox1, TRUE, TRUE, 0);
    gtk_widget_show(pBox1);

    GtkWidget *pBox2 = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pBox1), pBox2, TRUE, TRUE, 0);
    gtk_widget_show(pBox2);

    GtkWidget *pFrm = gtk_frame_new(_("General"));
    gtk_box_pack_start(GTK_BOX(pBox2), pFrm, TRUE, TRUE, 0);
    gtk_widget_show(pFrm);

    GtkWidget *pTbl = gtk_table_new(10, 5, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(pTbl), 3);
    gtk_table_set_row_spacings(GTK_TABLE(pTbl), 3);
    gtk_table_set_col_spacing(GTK_TABLE(pTbl), 1, 10);
    gtk_container_set_border_width(GTK_CONTAINER(pFrm), 2);
    gtk_container_set_border_width(GTK_CONTAINER(pTbl), 4);
    gtk_container_add(GTK_CONTAINER(pFrm), pTbl);
    gtk_widget_show(pTbl);

    GtkWidget *pLab = gtk_label_new(_("Paper size:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 2, 0, 1);
    gtk_widget_show(pLab);

    m_pPortraitRB = gtk_radio_button_new_with_label(NULL, _("Portrait"));
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pPortraitRB, 2, 5, 0, 1);
    gtk_widget_show(m_pPortraitRB);

    gint iIndex = -1;
    m_pPaperCB = gtk_combo_box_text_new();
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pPaperCB, 0, 2, 1, 2);
    for(int i = 0; i < m_iPaperSizes; i++)
    {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_pPaperCB), m_pPaperSizes[i].sPaperSizeName);
        if(strcmp(m_pFSR->cPaperSize.sPaperSizeName, m_pPaperSizes[i].sPaperSizeName) == 0) iIndex = i;
    }
    if(iIndex < 0)
    {
        if(m_iPaperSizes >= m_iDataSize)
        {
            m_iDataSize += 16;
            m_pPaperSizes = (PDPaperSize)g_realloc(m_pPaperSizes, m_iDataSize*sizeof(CDPaperSize));
        }

        strcpy(m_pPaperSizes[m_iPaperSizes].sPaperSizeName, m_pFSR->cPaperSize.sPaperSizeName);
        m_pPaperSizes[m_iPaperSizes].dPaperWidth = m_pFSR->cPaperSize.dPaperWidth;
        m_pPaperSizes[m_iPaperSizes].dPaperHeight = m_pFSR->cPaperSize.dPaperHeight;
        iIndex = m_iPaperSizes++;
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_pPaperCB), iIndex);
    gtk_widget_show(m_pPaperCB);

    m_pLandscapeRB = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(m_pPortraitRB), _("Landscape"));
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pLandscapeRB, 2, 5, 1, 2);
    gtk_widget_show(m_pLandscapeRB);

    if(m_pFSR->bPortrait) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pPortraitRB), TRUE);
    else gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pLandscapeRB), TRUE);

    pLab = gtk_label_new(_("World length unit:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 2, 2, 3);
    gtk_widget_show(pLab);

    pLab = gtk_label_new(_("Scale:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 2, 5, 2, 3);
    gtk_widget_show(pLab);

    m_pWLenCB = gtk_combo_box_text_new();
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pWLenCB, 0, 2, 3, 4);
    SetupUnitCB(GTK_COMBO_BOX_TEXT(m_pWLenCB), 1, &m_pFSR->cLenUnit);
    gtk_widget_show(m_pWLenCB);

    m_pScaleNomEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pScaleNomEdt, 40, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pScaleNomEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pScaleNomEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pScaleNomEdt, 2, 3, 3, 4);
    gtk_widget_show(m_pScaleNomEdt);

    gchar buf[32];
    FormatFloatStr(m_pFSR->dScaleNomin, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pScaleNomEdt), buf);
    pLab = gtk_label_new(_(" : "));
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 3, 4, 3, 4);
    gtk_widget_show(pLab);

    m_pScaleDenomEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pScaleDenomEdt, 60, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pScaleDenomEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pScaleDenomEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pScaleDenomEdt, 4, 5, 3, 4);
    gtk_widget_show(m_pScaleDenomEdt);

    FormatFloatStr(m_pFSR->dScaleDenom, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pScaleDenomEdt), buf);

    pLab = gtk_label_new(_("Angular unit:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 2, 4, 5);
    gtk_widget_show(pLab);

    pLab = gtk_label_new(_("Angular grid:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 2, 5, 4, 5);
    gtk_widget_show(pLab);

    m_pAngUnitCB = gtk_combo_box_text_new();
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pAngUnitCB, 0, 2, 5, 6);
    m_iCurAngUnit = SetupUnitCB(GTK_COMBO_BOX_TEXT(m_pAngUnitCB), 2, &m_pFSR->cAngUnit);
    g_signal_connect(G_OBJECT(m_pAngUnitCB), "changed", G_CALLBACK(fsd_angunitcb_changed), this);
    gtk_widget_show(m_pAngUnitCB);

    m_pAngGridEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pAngGridEdt, 60, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pAngGridEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pAngGridEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pAngGridEdt, 2, 4, 5, 6);
    CFsdGridEdtData cAngEdtData = {this, 0};
    g_signal_connect(G_OBJECT(m_pAngGridEdt), "changed", G_CALLBACK(fsd_gridedt_changed), &cAngEdtData);
    gtk_widget_show(m_pAngGridEdt);

    FormatFloatStr(m_cFSR.dAngGrid*m_pFSR->cAngUnit.dBaseToUnit, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pAngGridEdt), buf);

    m_pAngGridLab = gtk_label_new(m_pFSR->cAngUnit.sAbbrev);
    gtk_misc_set_alignment(GTK_MISC(m_pAngGridLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pAngGridLab, 4, 5, 5, 6);
    gtk_widget_show(m_pAngGridLab);

    pLab = gtk_label_new(_("Paper length unit:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 2, 6, 7);
    gtk_widget_show(pLab);

    m_pPLenCB = gtk_combo_box_text_new();
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pPLenCB, 0, 2, 7, 8);
    m_iCurPaperUnit = SetupUnitCB(GTK_COMBO_BOX_TEXT(m_pPLenCB), 1, &m_pFSR->cPaperUnit);
    g_signal_connect(G_OBJECT(m_pPLenCB), "changed", G_CALLBACK(fsd_papunitcb_changed), this);
    gtk_widget_show(m_pPLenCB);

    pLab = gtk_label_new(_("X grid:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 2, 8, 9);
    gtk_widget_show(pLab);

    pLab = gtk_label_new(_("Y grid:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 2, 5, 8, 9);
    gtk_widget_show(pLab);

    m_pXGridEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pXGridEdt, 50, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pXGridEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pXGridEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pXGridEdt, 0, 1, 9, 10);
    CFsdGridEdtData cXGridEdtData = {this, 1};
    g_signal_connect(G_OBJECT(m_pXGridEdt), "changed", G_CALLBACK(fsd_gridedt_changed), &cXGridEdtData);
    gtk_widget_show(m_pXGridEdt);

    FormatFloatStr(m_cFSR.dXGrid/m_pFSR->cPaperUnit.dBaseToUnit, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pXGridEdt), buf);

    m_pXGridLab = gtk_label_new(m_pFSR->cPaperUnit.sAbbrev);
    gtk_misc_set_alignment(GTK_MISC(m_pXGridLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pXGridLab, 1, 2, 9, 10);
    gtk_widget_show(m_pXGridLab);

    m_pYGridEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pYGridEdt, 50, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pYGridEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pYGridEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pYGridEdt, 2, 3, 9, 10);
    CFsdGridEdtData cYGridEdtData = {this, 2};
    g_signal_connect(G_OBJECT(m_pYGridEdt), "changed", G_CALLBACK(fsd_gridedt_changed), &cYGridEdtData);
    gtk_widget_show(m_pYGridEdt);

    FormatFloatStr(m_cFSR.dYGrid/m_pFSR->cPaperUnit.dBaseToUnit, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pYGridEdt), buf);

    m_pYGridLab = gtk_label_new(m_pFSR->cPaperUnit.sAbbrev);
    gtk_misc_set_alignment(GTK_MISC(m_pYGridLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pYGridLab, 3, 5, 9, 10);
    gtk_widget_show(m_pYGridLab);

    pFrm = gtk_frame_new(_("Graphic elements"));
    gtk_box_pack_start(GTK_BOX(pBox2), pFrm, TRUE, TRUE, 0);
    gtk_widget_show(pFrm);

    pTbl = gtk_table_new(3, 5, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(pTbl), 3);
    gtk_table_set_row_spacings(GTK_TABLE(pTbl), 3);
    gtk_table_set_col_spacing(GTK_TABLE(pTbl), 1, 10);
    gtk_container_set_border_width(GTK_CONTAINER(pFrm), 2);
    gtk_container_set_border_width(GTK_CONTAINER(pTbl), 4);
    gtk_container_add(GTK_CONTAINER(pFrm), pTbl);
    gtk_widget_show(pTbl);

    pLab = gtk_label_new(_("Size units for lines, arrows and fonts:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 5, 0, 1);
    gtk_widget_show(pLab);

    m_pGraphUnitCB = gtk_combo_box_text_new();
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pGraphUnitCB, 0, 2, 1, 2);
    m_iCurGraphUnit = SetupUnitCB(GTK_COMBO_BOX_TEXT(m_pGraphUnitCB), 1, &m_pFSR->cGraphUnit);
    g_signal_connect(G_OBJECT(m_pGraphUnitCB), "changed", G_CALLBACK(fsd_graphunitcb_changed), this);
    gtk_widget_show(m_pGraphUnitCB);

    pLab = gtk_label_new(_("Default line width:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 2, 2, 3);
    gtk_widget_show(pLab);

    m_pLineWidthEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pLineWidthEdt, 50, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pLineWidthEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pLineWidthEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pLineWidthEdt, 2, 3, 2, 3);
    CFsdGridEdtData cLineWEdtData = {this, 3};
    g_signal_connect(G_OBJECT(m_pLineWidthEdt), "changed", G_CALLBACK(fsd_gridedt_changed), &cLineWEdtData);
    gtk_widget_show(m_pLineWidthEdt);

    FormatFloatStr(m_cFSR.dDefLineWidth/m_pFSR->cGraphUnit.dBaseToUnit, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pLineWidthEdt), buf);

    m_pLineWidthLab = gtk_label_new(m_pFSR->cGraphUnit.sAbbrev);
    gtk_misc_set_alignment(GTK_MISC(m_pLineWidthLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pLineWidthLab, 3, 5, 2, 3);
    gtk_widget_show(m_pLineWidthLab);

    pBox2 = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pBox1), pBox2, TRUE, TRUE, 0);
    gtk_widget_show(pBox2);

    pFrm = gtk_frame_new(_("Dimensioning"));
    gtk_box_pack_start(GTK_BOX(pBox2), pFrm, TRUE, TRUE, 0);
    gtk_widget_show(pFrm);

    pTbl = gtk_table_new(6, 4, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(pTbl), 3);
    gtk_table_set_row_spacings(GTK_TABLE(pTbl), 3);
    gtk_table_set_col_spacing(GTK_TABLE(pTbl), 1, 10);
    gtk_container_set_border_width(GTK_CONTAINER(pFrm), 2);
    gtk_container_set_border_width(GTK_CONTAINER(pTbl), 4);
    gtk_container_add(GTK_CONTAINER(pFrm), pTbl);
    gtk_widget_show(pTbl);

    pLab = gtk_label_new(_("Default arrow shape:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 3, 0, 1);
    gtk_widget_show(pLab);

    m_pArrowCB = gtk_combo_box_text_new();
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pArrowCB, 0, 2, 1, 2);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_pArrowCB), _("None"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_pArrowCB), _("Standard"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_pArrowCB), _("Filled"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_pArrowCB), _("Point"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_pArrowCB), _("Slash"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_pArrowCB), _("Backslash"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(m_pArrowCB), m_pFSR->iArrowType);
    gtk_widget_show(m_pArrowCB);

    pLab = gtk_label_new(_("Default arrow length:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 2, 2, 3);
    gtk_widget_show(pLab);

    m_pALenEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pALenEdt, 50, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pALenEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pALenEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pALenEdt, 2, 3, 2, 3);
    CFsdGridEdtData cALenEdtData = {this, 4};
    g_signal_connect(G_OBJECT(m_pALenEdt), "changed", G_CALLBACK(fsd_gridedt_changed), &cALenEdtData);
    gtk_widget_show(m_pALenEdt);

    FormatFloatStr(m_cFSR.dArrowLen/m_pFSR->cGraphUnit.dBaseToUnit, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pALenEdt), buf);

    m_pALenLab = gtk_label_new(m_pFSR->cGraphUnit.sAbbrev);
    gtk_misc_set_alignment(GTK_MISC(m_pALenLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pALenLab, 3, 4, 2, 3);
    gtk_widget_show(m_pALenLab);

    pLab = gtk_label_new(_("Default arrow width:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 2, 3, 4);
    gtk_widget_show(pLab);

    m_pAWidthEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pAWidthEdt, 50, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pAWidthEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pAWidthEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pAWidthEdt, 2, 3, 3, 4);
    CFsdGridEdtData cAWidthEdtData = {this, 5};
    g_signal_connect(G_OBJECT(m_pAWidthEdt), "changed", G_CALLBACK(fsd_gridedt_changed), &cAWidthEdtData);
    gtk_widget_show(m_pAWidthEdt);

    FormatFloatStr(m_cFSR.dArrowWidth/m_pFSR->cGraphUnit.dBaseToUnit, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pAWidthEdt), buf);

    m_pAWidthLab = gtk_label_new(m_pFSR->cGraphUnit.sAbbrev);
    gtk_misc_set_alignment(GTK_MISC(m_pAWidthLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pAWidthLab, 3, 4, 3, 4);
    gtk_widget_show(m_pAWidthLab);

    pLab = gtk_label_new(_("Length mask:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 1, 4, 5);
    gtk_widget_show(pLab);

    m_pLenMaskEdt = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(m_pLenMaskEdt), 64);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pLenMaskEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pLenMaskEdt, 1, 4, 4, 5);
    gtk_widget_show(m_pLenMaskEdt);

    gtk_entry_set_text(GTK_ENTRY(m_pLenMaskEdt), m_pFSR->sLengthMask);

    pLab = gtk_label_new(_("Angular mask:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 1, 5, 6);
    gtk_widget_show(pLab);

    m_pAngMaskEdt = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(m_pAngMaskEdt), 64);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pAngMaskEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pAngMaskEdt, 1, 4, 5, 6);
    gtk_widget_show(m_pAngMaskEdt);

    gtk_entry_set_text(GTK_ENTRY(m_pAngMaskEdt), m_pFSR->sAngleMask);

    pFrm = gtk_frame_new(_("Default font"));
    gtk_box_pack_start(GTK_BOX(pBox2), pFrm, TRUE, TRUE, 0);
    gtk_widget_show(pFrm);

    pTbl = gtk_table_new(6, 4, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(pTbl), 3);
    gtk_table_set_row_spacings(GTK_TABLE(pTbl), 3);
    gtk_table_set_col_spacing(GTK_TABLE(pTbl), 1, 10);
    gtk_container_set_border_width(GTK_CONTAINER(pFrm), 2);
    gtk_container_set_border_width(GTK_CONTAINER(pTbl), 4);
    gtk_container_add(GTK_CONTAINER(pFrm), pTbl);
    gtk_widget_show(pTbl);

    pLab = gtk_label_new(_("Face:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 1, 0, 1);
    gtk_widget_show(pLab);

    /*m_pFntFaceCB = gtk_combo_box_text_new();
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pFntFaceCB, 1, 6, 0, 1);

    int iFonts = 0;
    PangoFontFamily **ppFonts = NULL, *pFont;
    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    pango_font_map_list_families(fontmap, &ppFonts, &iFonts);
    for(int i = 0; i < iFonts; i++)
    {
        pFont = ppFonts[i];
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_pFntFaceCB), pango_font_family_get_name(pFont));
    }
    g_free(ppFonts);
    gtk_widget_show(m_pFntFaceCB);*/

    gchar sBuf2[128];
    strcpy(sBuf2, m_pFSR->sFontFace);
    if(m_pFSR->bFontAttrs & 8) strcat(sBuf2, " Bold");
    if(m_pFSR->bFontAttrs & 1) strcat(sBuf2, " Italic");
    strcat(sBuf2, " 12");
    m_pFntFaceCB = gtk_font_button_new_with_font(sBuf2);
    gtk_font_button_set_show_size(GTK_FONT_BUTTON(m_pFntFaceCB), FALSE);
    //gtk_font_button_set_use_size(GTK_FONT_BUTTON(m_pFntFaceCB), FALSE);
    gtk_font_button_set_show_style(GTK_FONT_BUTTON(m_pFntFaceCB), FALSE);
    g_signal_connect(G_OBJECT(m_pFntFaceCB), "font-set", G_CALLBACK(fsd_font_set), this);

    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pFntFaceCB, 1, 6, 0, 1);
    gtk_widget_show(m_pFntFaceCB);

    m_pBoldChB = gtk_check_button_new_with_label(_("Bold"));
    if(m_pFSR->bFontAttrs & 8) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pBoldChB), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pBoldChB, 0, 2, 1, 2);
    CFsdGridEdtData cBoldChBData = {this, 8};
    g_signal_connect(G_OBJECT(m_pBoldChB), "toggled", G_CALLBACK(fsd_fontattrchb_toggled), &cBoldChBData);
    gtk_widget_show(m_pBoldChB);

    m_pItalicChB = gtk_check_button_new_with_label(_("Italic"));
    if(m_pFSR->bFontAttrs & 1) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pItalicChB), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pItalicChB, 2, 4, 1, 2);
    CFsdGridEdtData cItalicChBData = {this, 1};
    g_signal_connect(G_OBJECT(m_pItalicChB), "toggled", G_CALLBACK(fsd_fontattrchb_toggled), &cItalicChBData);
    gtk_widget_show(m_pItalicChB);

    m_pUnderChB = gtk_check_button_new_with_label(_("Underline"));
    if(m_pFSR->bFontAttrs & 2) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pUnderChB), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pUnderChB, 4, 6, 1, 2);
    CFsdGridEdtData cUnderChBData = {this, 2};
    g_signal_connect(G_OBJECT(m_pUnderChB), "toggled", G_CALLBACK(fsd_fontattrchb_toggled), &cUnderChBData);
    gtk_widget_show(m_pUnderChB);

    pLab = gtk_label_new(_("Size:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 0, 1, 2, 3);
    gtk_widget_show(pLab);

    m_pFntSizeEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pFntSizeEdt, 40, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pFntSizeEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pFntSizeEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pFntSizeEdt, 1, 2, 2, 3);
    CFsdGridEdtData cFntSizeEdtData = {this, 6};
    g_signal_connect(G_OBJECT(m_pFntSizeEdt), "changed", G_CALLBACK(fsd_gridedt_changed), &cFntSizeEdtData);
    gtk_widget_show(m_pFntSizeEdt);

    FormatFloatStr(m_pFSR->dFontSize/m_pFSR->cGraphUnit.dBaseToUnit, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pFntSizeEdt), buf);

    m_pFntSizeLab = gtk_label_new(m_pFSR->cGraphUnit.sAbbrev);
    gtk_misc_set_alignment(GTK_MISC(m_pFntSizeLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pFntSizeLab, 2, 3, 2, 3);
    gtk_widget_show(m_pFntSizeLab);

    pLab = gtk_label_new(_("Base:"));
    gtk_misc_set_alignment(GTK_MISC(pLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), pLab, 3, 4, 2, 3);
    gtk_widget_show(pLab);

    m_pBaseEdt = gtk_entry_new();
    gtk_widget_set_size_request(m_pBaseEdt, 40, -1);
    gtk_entry_set_max_length(GTK_ENTRY(m_pBaseEdt), 32);
    gtk_entry_set_activates_default(GTK_ENTRY(m_pBaseEdt), TRUE);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pBaseEdt, 4, 5, 2, 3);
    CFsdGridEdtData cBaseEdtData = {this, 7};
    g_signal_connect(G_OBJECT(m_pBaseEdt), "changed", G_CALLBACK(fsd_gridedt_changed), &cBaseEdtData);
    gtk_widget_show(m_pBaseEdt);

    FormatFloatStr(m_pFSR->dBaseLine/m_pFSR->cGraphUnit.dBaseToUnit, buf);
    gtk_entry_set_text(GTK_ENTRY(m_pBaseEdt), buf);

    m_pBaseLab = gtk_label_new(m_pFSR->cGraphUnit.sAbbrev);
    gtk_misc_set_alignment(GTK_MISC(m_pBaseLab), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pBaseLab, 5, 6, 2, 3);
    gtk_widget_show(m_pBaseLab);

    m_pFntSampleLab = gtk_label_new(_("Sample 0123456"));
    gtk_table_attach_defaults(GTK_TABLE(pTbl), m_pFntSampleLab, 0, 6, 3, 4);
    gtk_widget_show(m_pFntSampleLab);

    PangoFontDescription *ppfd = pango_font_description_from_string(
        gtk_font_button_get_font_name(GTK_FONT_BUTTON(m_pFntFaceCB)));
    pango_font_description_set_size(ppfd, 12288);
    gtk_widget_modify_font(m_pFntSampleLab, ppfd);
    pango_font_description_free(ppfd);

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

void CDFileSetupDlg::SaveSettings(FILE *fp)
{
    if(m_iX < -90) return;

    gchar sbuf[256];
	sprintf(sbuf, "  <FileSetupDlg Left=\"%d\" Top=\"%d\"/>\n", m_iX, m_iY);
    fwrite(sbuf, sizeof(gchar), strlen(sbuf), fp);
    return;
}

void CDFileSetupDlg::RestoreSettings(gint iLeft, gint iTop)
{
    m_iX = iLeft;
    m_iY = iTop;
    return;
}

int CDFileSetupDlg::SetupUnitCB(GtkComboBoxText *pCB, int iType, PDFileUnit pFileUnit)
{
    int iRes = -1;

    PDUnit pUnit;
    int iLen = m_pUnits->GetCount(iType);
    for(int i = 0; i < iLen; i++)
    {
        pUnit = m_pUnits->GetUnit(iType, i);
        if(strcmp(pUnit->sName, pFileUnit->sName) == 0) iRes = i;

        gtk_combo_box_text_append_text(pCB, pUnit->sName);
    }

    if(iRes < 0)
    {
        CDUnit cUnit;
        strcpy(cUnit.sName, pFileUnit->sName);
        strcpy(cUnit.sAbbrev, pFileUnit->sAbbrev);
        cUnit.dBaseToUnit = pFileUnit->dBaseToUnit;
        strcpy(cUnit.sAbbrev2, pFileUnit->sAbbrev2);
        cUnit.iUnitType = iType;

        m_pUnits->AddUnit(cUnit);
        pUnit = m_pUnits->GetUnit(iType, iLen);
        gtk_combo_box_text_append_text(pCB, pUnit->sName);

        iRes = iLen++;
    }

    if(iLen > 0) gtk_combo_box_set_active(GTK_COMBO_BOX(pCB), iRes);

    return iRes;
}

gboolean CDFileSetupDlg::Configure(GtkWidget *widget, GdkEvent *event)
{
    if(m_bSettingUp) return FALSE;

    m_iX = event->configure.x;
    m_iY = event->configure.y;
    return FALSE;
}

bool ReadLine(FILE *fp, gchar *sBuf, int iBufSize)
{
    int iPos = 0;
    sBuf[iPos] = 0;
    int cCur = fgetc(fp);
    bool bComment = false;

    if(cCur == '/')
    {
        sBuf[iPos++] = cCur;
        cCur = fgetc(fp);
        if(cCur == '/')
        {
            sBuf[iPos++] = cCur;
            sBuf[iPos] = 0;
            bComment = true;
        }
    }

    bool bEOL = (cCur == 10) || (cCur == 13);
    bool bEOF = (cCur == EOF);

    if(bComment)
    {
        while(!(bEOL || bEOF))
        {
            cCur = fgetc(fp);
            bEOL = (cCur == 10) || (cCur == 13);
            bEOF = (cCur == EOF);
        }
        if(cCur == 13) cCur = fgetc(fp);
        return !bEOF;
    }

    while(!(bEOL || bEOF) && (iPos < iBufSize))
    {
        sBuf[iPos++] = cCur;
        cCur = fgetc(fp);
        bEOL = (cCur == 10) || (cCur == 13);
        bEOF = (cCur == EOF);
    }
    sBuf[iPos] = 0;

    while(!(bEOL || bEOF))
    {
        cCur = fgetc(fp);
        bEOL = (cCur == 10) || (cCur == 13);
        bEOF = (cCur == EOF);
    }

    if(cCur == 13) cCur = fgetc(fp);
    return (iPos > 0);
}

void CDFileSetupDlg::BuildPaperSizeList()
{
	const gchar *homedir = g_get_home_dir();
	gchar *sFile = g_strconcat(homedir, "/.SteamCAD/DPapers.ini", NULL);

    FILE *fp = fopen(sFile, "r");
    g_free(sFile);

    if(!fp) return;

    m_iDataSize = 16;
    m_pPaperSizes = (PDPaperSize)g_malloc(m_iDataSize*sizeof(CDPaperSize));

    gchar sLine[64];
    gchar sBuf[32];
    gchar *sStart, *sEnd;
    float f;
    int iLen;
    while(ReadLine(fp, sLine, 64))
    {
        sStart = sLine;
        sEnd = strchr(sStart, ';');
        if(sEnd)
        {
            if(m_iPaperSizes >= m_iDataSize)
            {
                m_iDataSize += 16;
                m_pPaperSizes = (PDPaperSize)g_realloc(m_pPaperSizes, m_iDataSize*sizeof(CDPaperSize));
            }

            iLen = sEnd - sStart;
            if(iLen > 63) iLen = 63;
            strncpy(m_pPaperSizes[m_iPaperSizes].sPaperSizeName, sStart, iLen);
            m_pPaperSizes[m_iPaperSizes].sPaperSizeName[iLen] = 0;

            sStart = sEnd + 1;
            sEnd = strchr(sStart, ';');

            PDUnit pUnit = NULL;

            if(sEnd)
            {
                iLen = sEnd - sStart;
                if(iLen > 31) iLen = 31;
                strncpy(sBuf, sStart, iLen);
                sBuf[iLen] = 0;

                pUnit = m_pUnits->FindUnit(sBuf);

                sStart = sEnd + 1;
                sEnd = strchr(sStart, ';');
            }

            if(sEnd)
            {
                iLen = sEnd - sStart;
                if(iLen > 31) iLen = 31;
                strncpy(sBuf, sStart, iLen);
                sBuf[iLen] = 0;
                if(sscanf(sBuf, "%f", &f) == 1)
                {
                    if(pUnit) m_pPaperSizes[m_iPaperSizes].dPaperWidth = f*pUnit->dBaseToUnit;
                    else m_pPaperSizes[m_iPaperSizes].dPaperWidth = f;
                }
                else m_pPaperSizes[m_iPaperSizes].dPaperWidth = 0.0;

                sStart = sEnd + 1;
            }
            else
            {
                m_pPaperSizes[m_iPaperSizes].dPaperWidth = 0.0;
                sStart = NULL;
            }

            if(sStart)
            {
                iLen = strlen(sStart);
                if(iLen > 31) iLen = 31;
                strncpy(sBuf, sStart, iLen);
                sBuf[iLen] = 0;
                if(sscanf(sBuf, "%f", &f) == 1)
                {
                    if(pUnit) m_pPaperSizes[m_iPaperSizes].dPaperHeight = f*pUnit->dBaseToUnit;
                    else m_pPaperSizes[m_iPaperSizes].dPaperHeight = f;
                }
                else m_pPaperSizes[m_iPaperSizes].dPaperHeight = 0.0;
            }
            else m_pPaperSizes[m_iPaperSizes].dPaperHeight = 0.0;

            if((m_pPaperSizes[m_iPaperSizes].dPaperWidth > 0.01) &&
               (m_pPaperSizes[m_iPaperSizes].dPaperHeight > 0.01)) m_iPaperSizes++;
        }
    }

    fclose(fp);
}

void CDFileSetupDlg::BuildLenUnitsList()
{
	const gchar *homedir = g_get_home_dir();
	gchar *sFile = g_strconcat(homedir, "/.SteamCAD/DUnits.ini", NULL);

    FILE *fp = fopen(sFile, "r");
    g_free(sFile);

    if(!fp) return;

    gchar sLine[64];
    gchar sBuf[32];
    gchar *sStart, *sEnd;
    float f;
    int iLen;
    CDUnit cUnit;
    while(ReadLine(fp, sLine, 64))
    {
        if(!((sLine[0] == '\\') && (sLine[1] == '\\')))
        {
            cUnit.sName[0] = 0;
            cUnit.sAbbrev[0] = 0;
            cUnit.dBaseToUnit = 0.0;
            cUnit.sAbbrev2[0] = 0;
            cUnit.iUnitType = 0;

            sStart = sLine;
            sEnd = strchr(sStart, ';');
            if(sEnd)
            {
                iLen = sEnd - sStart;
                if(iLen > 31) iLen = 31;
                strncpy(cUnit.sName, sStart, iLen);
                cUnit.sName[iLen] = 0;
                sStart = sEnd + 1;
                sEnd = strchr(sStart, ';');
            }

            if(sEnd)
            {
                iLen = sEnd - sStart;
                if(iLen > 7) iLen = 7;
                strncpy(cUnit.sAbbrev, sStart, iLen);
                cUnit.sAbbrev[iLen] = 0;

                sStart = sEnd + 1;
                sEnd = strchr(sStart, ';');
            }
            else sStart = NULL;

            if(sEnd)
            {
                iLen = sEnd - sStart;
                if(iLen > 31) iLen = 31;
                strncpy(sBuf, sStart, iLen);
                sBuf[iLen] = 0;
                if(sscanf(sBuf, "%f", &f) == 1) cUnit.dBaseToUnit = f;
                else cUnit.dBaseToUnit = 0.0;

                sStart = sEnd + 1;
                sEnd = strchr(sStart, ';');
            }
            else sStart = NULL;

            if(sEnd)
            {
                if(sStart[0] != '-')
                {
                    iLen = sEnd - sStart;
                    if(iLen > 7) iLen = 7;
                    strncpy(cUnit.sAbbrev2, sStart, iLen);
                    cUnit.sAbbrev2[iLen] = 0;
                }

                sStart = sEnd + 1;
            }
            else sStart = NULL;

            if(sStart)
            {
                iLen = strlen(sStart);
                if(iLen > 0)
                {
                    if(sStart[0] == 'l') cUnit.iUnitType = 1;
                    else if(sStart[0] == 'a') cUnit.iUnitType = 2;
                }
            }

            if((cUnit.sName[0] > 0) && (cUnit.sAbbrev[0] > 0) &&
               (cUnit.dBaseToUnit > 0.00001))
            {
               m_pUnits->AddUnit(cUnit);
            }
        }
    }

    fclose(fp);
}

void CDFileSetupDlg::AngUnitCBChanged(GtkComboBox *widget)
{
    gint iIndex = gtk_combo_box_get_active(widget);
    if(m_iCurAngUnit != iIndex)
    {
        m_bSettingUp = true;

        PDUnit pUnit = m_pUnits->GetUnit(2, iIndex);
        gtk_label_set_label(GTK_LABEL(m_pAngGridLab), pUnit->sAbbrev);
        
        gchar buf[32];
        FormatFloatStr(m_cFSR.dAngGrid*pUnit->dBaseToUnit, buf);
        gtk_entry_set_text(GTK_ENTRY(m_pAngGridEdt), buf);

        m_iCurAngUnit = iIndex;

        m_bSettingUp = false;
    }
}

void CDFileSetupDlg::PaperUnitCBChanged(GtkComboBox *widget)
{
    gint iIndex = gtk_combo_box_get_active(widget);
    if(m_iCurPaperUnit != iIndex)
    {
        m_bSettingUp = true;

        PDUnit pUnit = m_pUnits->GetUnit(1, iIndex);
        gtk_label_set_label(GTK_LABEL(m_pXGridLab), pUnit->sAbbrev);
        gtk_label_set_label(GTK_LABEL(m_pYGridLab), pUnit->sAbbrev);

        gchar buf[32];
        FormatFloatStr(m_cFSR.dXGrid/pUnit->dBaseToUnit, buf);
        gtk_entry_set_text(GTK_ENTRY(m_pXGridEdt), buf);
        FormatFloatStr(m_cFSR.dYGrid/pUnit->dBaseToUnit, buf);
        gtk_entry_set_text(GTK_ENTRY(m_pYGridEdt), buf);

        m_iCurPaperUnit = iIndex;

        m_bSettingUp = false;
    }
}

void CDFileSetupDlg::GraphUnitCBChanged(GtkComboBox *widget)
{
    gint iIndex = gtk_combo_box_get_active(widget);
    if(m_iCurGraphUnit != iIndex)
    {
        m_bSettingUp = true;

        PDUnit pUnit = m_pUnits->GetUnit(1, iIndex);
        gtk_label_set_label(GTK_LABEL(m_pLineWidthLab), pUnit->sAbbrev);
        gtk_label_set_label(GTK_LABEL(m_pALenLab), pUnit->sAbbrev);
        gtk_label_set_label(GTK_LABEL(m_pAWidthLab), pUnit->sAbbrev);
        gtk_label_set_label(GTK_LABEL(m_pFntSizeLab), pUnit->sAbbrev);
        gtk_label_set_label(GTK_LABEL(m_pBaseLab), pUnit->sAbbrev);

        gchar buf[32];
        FormatFloatStr(m_cFSR.dDefLineWidth/pUnit->dBaseToUnit, buf);
        gtk_entry_set_text(GTK_ENTRY(m_pLineWidthEdt), buf);

        FormatFloatStr(m_cFSR.dArrowLen/pUnit->dBaseToUnit, buf);
        gtk_entry_set_text(GTK_ENTRY(m_pALenEdt), buf);

        FormatFloatStr(m_cFSR.dArrowWidth/pUnit->dBaseToUnit, buf);
        gtk_entry_set_text(GTK_ENTRY(m_pAWidthEdt), buf);

        FormatFloatStr(m_cFSR.dFontSize/pUnit->dBaseToUnit, buf);
        gtk_entry_set_text(GTK_ENTRY(m_pFntSizeEdt), buf);

        FormatFloatStr(m_cFSR.dBaseLine/pUnit->dBaseToUnit, buf);
        gtk_entry_set_text(GTK_ENTRY(m_pBaseEdt), buf);

        m_iCurGraphUnit = iIndex;

        m_bSettingUp = false;
    }
}

void CDFileSetupDlg::GridEditChanged(GtkEntry *widget, gint id)
{
    if(m_bSettingUp) return;

    const gchar *pbuf = gtk_entry_get_text(widget);

    float f;
    if(sscanf(pbuf, "%f", &f) == 1)
    {
        PDUnit pAngUnit = m_pUnits->GetUnit(2, m_iCurAngUnit);
        PDUnit pPaperUnit = m_pUnits->GetUnit(1, m_iCurPaperUnit);
        PDUnit pGraphUnit = m_pUnits->GetUnit(1, m_iCurGraphUnit);

        switch(id)
        {
        case 1:
            m_cFSR.dXGrid = f*pPaperUnit->dBaseToUnit;
            break;
        case 2:
            m_cFSR.dYGrid = f*pPaperUnit->dBaseToUnit;
            break;
        case 3:
            m_cFSR.dDefLineWidth = f*pGraphUnit->dBaseToUnit;
            break;
        case 4:
            m_cFSR.dArrowLen = f*pGraphUnit->dBaseToUnit;
            break;
        case 5:
            m_cFSR.dArrowWidth = f*pGraphUnit->dBaseToUnit;
            break;
        case 6:
            m_cFSR.dFontSize = f*pGraphUnit->dBaseToUnit;
            break;
        case 7:
            m_cFSR.dBaseLine = f*pGraphUnit->dBaseToUnit;
            break;
        default:
            m_cFSR.dAngGrid = f/pAngUnit->dBaseToUnit;
        }
    }
}

void CDFileSetupDlg::FontAttrChBChange(GtkToggleButton *togglebutton, gint iMask)
{
    if(m_bSettingUp) return;

    if(gtk_toggle_button_get_active(togglebutton)) m_cFSR.bFontAttrs |= iMask;
    else m_cFSR.bFontAttrs &= ~iMask;

    gchar sBuf2[128];
    strcpy(sBuf2, m_cFSR.sFontFace);
    if(m_cFSR.bFontAttrs & 8) strcat(sBuf2, " Bold");
    if(m_cFSR.bFontAttrs & 1) strcat(sBuf2, " Italic");
    strcat(sBuf2, " 12");

    m_bSettingUp = TRUE;
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(m_pFntFaceCB), sBuf2);

    PangoFontDescription *ppfd = pango_font_description_from_string(gtk_font_button_get_font_name(GTK_FONT_BUTTON(m_pFntFaceCB)));
    gtk_widget_modify_font(m_pFntSampleLab, ppfd);
    m_bSettingUp = FALSE;

    pango_font_description_free(ppfd);
}

PDUnitList CDFileSetupDlg::GetUnitList()
{
    return m_pUnits;
}

PDPaperSize CDFileSetupDlg::FindPaper(double dWidth, double dHeight)
{
    bool bFound = false;
    int i = 0;
    while(!bFound && (i < m_iPaperSizes))
    {
        bFound = ((fabs(m_pPaperSizes[i].dPaperWidth - dWidth) < 0.1) &&
            (fabs(m_pPaperSizes[i].dPaperHeight - dHeight) < 0.1)) ||
            ((fabs(m_pPaperSizes[i].dPaperWidth - dHeight) < 0.1) &&
            (fabs(m_pPaperSizes[i].dPaperHeight - dWidth) < 0.1));
        i++;
    }
    if(bFound) return &m_pPaperSizes[i - 1];
    return NULL;
}

void CDFileSetupDlg::FontSet(GtkFontButton *widget)
{
    if(m_bSettingUp) return;

    PangoFontDescription *ppfd = pango_font_description_from_string(gtk_font_button_get_font_name(widget));
    PangoStyle pst = pango_font_description_get_style(ppfd);
    PangoWeight pwe = pango_font_description_get_weight(ppfd);
    strcpy(m_cFSR.sFontFace, pango_font_description_get_family(ppfd));

    m_bSettingUp = TRUE;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pBoldChB), pwe > PANGO_WEIGHT_MEDIUM);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_pItalicChB), pst > PANGO_STYLE_NORMAL);

    pango_font_description_set_size(ppfd, 12288);
    gtk_widget_modify_font(m_pFntSampleLab, ppfd);

    m_bSettingUp = FALSE;

    pango_font_description_free(ppfd);
}

void CDFileSetupDlg::OkBtnClick(GtkButton *button)
{
    float f;
    GtkWidget *msg_dlg;

    if(sscanf(gtk_entry_get_text(GTK_ENTRY(m_pScaleDenomEdt)), "%f", &f) != 1)
    {
        msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, _("Invalid number"));
        gtk_dialog_run(GTK_DIALOG(msg_dlg));
        gtk_widget_destroy(msg_dlg);
        gtk_widget_grab_focus(m_pScaleDenomEdt);
        return;
    }

    m_cFSR.dScaleDenom = f;

    if(sscanf(gtk_entry_get_text(GTK_ENTRY(m_pScaleNomEdt)), "%f", &f) != 1)
    {
        msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, _("Invalid number"));
        gtk_dialog_run(GTK_DIALOG(msg_dlg));
        gtk_widget_destroy(msg_dlg);
        gtk_widget_grab_focus(m_pScaleNomEdt);
        return;
    }

    m_cFSR.dScaleNomin = f;

    m_cFSR.bPortrait = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_pPortraitRB));

    gint iIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(m_pWLenCB));
    PDUnit pLU = m_pUnits->GetUnit(1, iIndex);

    if(strcasecmp(pLU->sName, m_cFSR.cLenUnit.sName) != 0)
    {
        strcpy(m_cFSR.cLenUnit.sName, pLU->sName);
        strcpy(m_cFSR.cLenUnit.sAbbrev, pLU->sAbbrev);
        m_cFSR.cLenUnit.dBaseToUnit = pLU->dBaseToUnit;
        strcpy(m_cFSR.cLenUnit.sAbbrev2, pLU->sAbbrev2);
    }

    iIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(m_pAngUnitCB));
    pLU = m_pUnits->GetUnit(2, iIndex);

    if(strcasecmp(pLU->sName, m_cFSR.cAngUnit.sName) != 0)
    {
        strcpy(m_cFSR.cAngUnit.sName, pLU->sName);
        strcpy(m_cFSR.cAngUnit.sAbbrev, pLU->sAbbrev);
        m_cFSR.cAngUnit.dBaseToUnit = pLU->dBaseToUnit;
        strcpy(m_cFSR.cAngUnit.sAbbrev2, pLU->sAbbrev2);
    }

    iIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(m_pPLenCB));
    pLU = m_pUnits->GetUnit(1, iIndex);

    if(strcasecmp(pLU->sName, m_cFSR.cPaperUnit.sName) != 0)
    {
        strcpy(m_cFSR.cPaperUnit.sName, pLU->sName);
        strcpy(m_cFSR.cPaperUnit.sAbbrev, pLU->sAbbrev);
        m_cFSR.cPaperUnit.dBaseToUnit = pLU->dBaseToUnit;
        strcpy(m_cFSR.cPaperUnit.sAbbrev2, pLU->sAbbrev2);
    }

    iIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(m_pGraphUnitCB));
    pLU = m_pUnits->GetUnit(1, iIndex);

    if(strcasecmp(pLU->sName, m_cFSR.cGraphUnit.sName) != 0)
    {
        strcpy(m_cFSR.cGraphUnit.sName, pLU->sName);
        strcpy(m_cFSR.cGraphUnit.sAbbrev, pLU->sAbbrev);
        m_cFSR.cGraphUnit.dBaseToUnit = pLU->dBaseToUnit;
        strcpy(m_cFSR.cGraphUnit.sAbbrev2, pLU->sAbbrev2);
    }

    iIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(m_pPaperCB));
    PDPaperSize pPS = &m_pPaperSizes[iIndex];
    if(strcasecmp(m_cFSR.cPaperSize.sPaperSizeName, pPS->sPaperSizeName) != 0)
    {
        strcpy(m_cFSR.cPaperSize.sPaperSizeName, pPS->sPaperSizeName);
        m_cFSR.cPaperSize.dPaperWidth = pPS->dPaperWidth;
        m_cFSR.cPaperSize.dPaperHeight = pPS->dPaperHeight;
    }

    const gchar *sBuf = gtk_entry_get_text(GTK_ENTRY(m_pLenMaskEdt));
    char sMsgBuf[64];
    int iValRes = ValidateMask(sBuf, m_pUnits);
    if(iValRes != 0)
    {
        switch(iValRes)
        {
        case 1:
            strcpy(sMsgBuf, _("Precision should not be negative"));
            break;
        case 2:
            strcpy(sMsgBuf, _("Precision should not be greater than 16"));
            break;
        case 3:
            strcpy(sMsgBuf, _("Precision value cannot be parsed"));
            break;
        default:
            strcpy(sMsgBuf, _("Invalid mask (unknown reason)"));
        }
        msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", sMsgBuf);
        gtk_dialog_run(GTK_DIALOG(msg_dlg));
        gtk_widget_destroy(msg_dlg);
        gtk_widget_grab_focus(m_pLenMaskEdt);
        return;
    }
    else strcpy(m_cFSR.sLengthMask, sBuf);

    sBuf = gtk_entry_get_text(GTK_ENTRY(m_pAngMaskEdt));
    iValRes = ValidateMask(sBuf, m_pUnits);
    if(iValRes != 0)
    {
        switch(iValRes)
        {
        case 1:
            strcpy(sMsgBuf, _("Precision should not be negative"));
            break;
        case 2:
            strcpy(sMsgBuf, _("Precision should not be greater than 16"));
            break;
        case 3:
            strcpy(sMsgBuf, _("Precision value cannot be parsed"));
            break;
        default:
            strcpy(sMsgBuf, _("Invalid mask (unknown reason)"));
        }
        msg_dlg = gtk_message_dialog_new(GTK_WINDOW(m_pDlg), GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", sMsgBuf);
        gtk_dialog_run(GTK_DIALOG(msg_dlg));
        gtk_widget_destroy(msg_dlg);
        gtk_widget_grab_focus(m_pAngMaskEdt);
        return;
    }
    else strcpy(m_cFSR.sAngleMask, sBuf);

    m_cFSR.iArrowType = gtk_combo_box_get_active(GTK_COMBO_BOX(m_pArrowCB));

    memcpy(m_pFSR, &m_cFSR, sizeof(CDFileSetupRec));
    gtk_dialog_response(GTK_DIALOG(m_pDlg), GTK_RESPONSE_OK);
}

