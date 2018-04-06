#ifndef _DLINESTYLEDLG_HPP_
#define _DLINESTYLEDLG_HPP_

#include <gtk/gtk.h>

#include "../Source/DDataTypes.hpp"
#include "DFileSetupDlg.hpp"

typedef struct CDLineStyleRec
{
    CDLineStyle cLineStyle;
    CDFileUnit cUnit;
    gboolean bWidthSet;
    gboolean bExcSet;
    gboolean bPatSet;
    gboolean bWidthChanged;
    gboolean bExcChanged;
    gboolean bPatChanged;
} *PDLineStyleRec;

typedef class CDLineStyleDlg
{
private:
    gint m_iX;
    gint m_iY;
    gboolean m_bSettingUp;
    PDLineStyleRec m_pLSR;
    gboolean m_bWidthChanged;
    gboolean m_bExcChanged;
    gboolean m_bPatChanged;

    GtkWidget *m_pDlg;
    GtkWidget *m_pLineWidthEdt;
    GtkWidget *m_pEccentEdt;
    GtkWidget *m_pPatternEdt[6];
public:
    CDLineStyleDlg();
    ~CDLineStyleDlg();
    gboolean ShowDialog(GtkWidget *pWndParent, PDLineStyleRec pLSR);
    void SaveSettings(FILE *fp);
    void RestoreSettings(gint iLeft, gint iTop);

    gboolean Configure(GtkWidget *widget, GdkEvent *event);
    void LineWidthChange(GtkEntry *entry);
    void LineExcChange(GtkEntry *entry);
    void LinePatChange(GtkEntry *entry);
    void OKBtnClick(GtkButton *button);
} *PDLineStyleDlg;

#endif

