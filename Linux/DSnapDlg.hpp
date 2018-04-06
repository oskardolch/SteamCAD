#ifndef _DSNAPDLG_HPP_
#define _DSNAPDLG_HPP_

#include <gtk/gtk.h>

typedef class CDSnapDlg
{
private:
    gint m_iX;
    gint m_iY;
    gboolean m_bSettingUp;
public:
    CDSnapDlg();
    ~CDSnapDlg();
    gboolean ShowDialog(GtkWidget *pWndParent, gboolean *pbEnableSnap);
    void SaveSettings(FILE *fp);
    void RestoreSettings(gint iLeft, gint iTop);
    gboolean Configure(GtkWidget *widget, GdkEvent *event);
} *PDSnapDlg;

#endif
