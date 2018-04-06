#ifndef _DSTATDLG_HPP_
#define _DSTATDLG_HPP_

#include <gtk/gtk.h>

typedef struct CDStatRec
{
    gint iLines;
    gint iCircles;
    gint iEllips;
    gint iArcEllips;
    gint iHypers;
    gint iParabs;
    gint iSplines;
    gint iEvolvs;
    gint iDimens;
} *PDStatRec;

typedef class CDStatDlg
{
private:
    gint m_iX;
    gint m_iY;
    gboolean m_bSettingUp;
public:
    CDStatDlg();
    ~CDStatDlg();
    gboolean ShowDialog(GtkWidget *pWndParent, PDStatRec pSR);
    void SaveSettings(FILE *fp);
    void RestoreSettings(gint iLeft, gint iTop);
    gboolean Configure(GtkWidget *widget, GdkEvent *event);
} *PDStatDlg;

#endif
