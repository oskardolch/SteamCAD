#ifndef _DFILESETUPDLG_HPP_
#define _DFILESETUPDLG_HPP_

#include <gtk/gtk.h>

#include "../Source/DParser.hpp"

typedef struct CDPaperSize
{
    gchar sPaperSizeName[64];
    double dPaperWidth;
    double dPaperHeight;
} *PDPaperSize;

typedef struct CDFileUnit
{
    gchar sName[32];
    gchar sAbbrev[8];
    double dBaseToUnit;
    gchar sAbbrev2[8];
} *PDFileUnit;

typedef struct CDFileSetupRec
{
    CDPaperSize cPaperSize;
    bool bPortrait;
    CDFileUnit cLenUnit; // physical lengths
    CDFileUnit cAngUnit; // angles
    CDFileUnit cPaperUnit; // lengths expressed at paper scale
    CDFileUnit cGraphUnit; // graphic elements size - line width, text size, arrow dims
    double dScaleNomin;
    double dScaleDenom;
    double dAngGrid; // in cAngUnit
    double dXGrid; // in cPaperUnit
    double dYGrid; // in cPaperUnit
    double dDefLineWidth; // in cGraphUnit
    int iArrowType;
    double dArrowLen; // in cGraphUnit
    double dArrowWidth; // in cGraphUnit
    gchar sLengthMask[64];
    gchar sAngleMask[64];
    gchar bFontAttrs;
    double dFontSize; // in cGraphUnit
    double dBaseLine; // in cGraphUnit
    gchar sFontFace[64];
} *PDFileSetupRec;

typedef class CDFileSetupDlg
{
private:
    PDFileSetupRec m_pFSR;
    CDFileSetupRec m_cFSR;
    PDPaperSize m_pPaperSizes;
    int m_iPaperSizes;
    int m_iDataSize;
    int m_iX;
    int m_iY;

    PDUnitList m_pUnits;

    bool m_bSettingUp;

    int m_iCurAngUnit;
    int m_iCurPaperUnit;
    int m_iCurGraphUnit;

    GtkWidget *m_pDlg;
    GtkWidget *m_pPortraitRB;
    GtkWidget *m_pLandscapeRB;
    GtkWidget *m_pPaperCB;
    GtkWidget *m_pWLenCB;
    GtkWidget *m_pScaleNomEdt;
    GtkWidget *m_pScaleDenomEdt;
    GtkWidget *m_pAngUnitCB;
    GtkWidget *m_pAngGridEdt;
    GtkWidget *m_pAngGridLab;
    GtkWidget *m_pPLenCB;
    GtkWidget *m_pXGridEdt;
    GtkWidget *m_pXGridLab;
    GtkWidget *m_pYGridEdt;
    GtkWidget *m_pYGridLab;
    GtkWidget *m_pGraphUnitCB;
    GtkWidget *m_pLineWidthEdt;
    GtkWidget *m_pLineWidthLab;
    GtkWidget *m_pArrowCB;
    GtkWidget *m_pALenEdt;
    GtkWidget *m_pALenLab;
    GtkWidget *m_pAWidthEdt;
    GtkWidget *m_pAWidthLab;
    GtkWidget *m_pLenMaskEdt;
    GtkWidget *m_pAngMaskEdt;
    GtkWidget *m_pFntFaceCB;
    GtkWidget *m_pBoldChB;
    GtkWidget *m_pItalicChB;
    GtkWidget *m_pUnderChB;
    GtkWidget *m_pFntSizeEdt;
    GtkWidget *m_pFntSizeLab;
    GtkWidget *m_pBaseEdt;
    GtkWidget *m_pBaseLab;
    GtkWidget *m_pFntSampleLab;

    void BuildPaperSizeList(gchar *sAppPath);
    void BuildLenUnitsList(gchar *sAppPath);
    int SetupUnitCB(GtkComboBoxText *pCB, int iType, PDFileUnit pFileUnit);
public:
    CDFileSetupDlg(gchar *sAppPath);
    ~CDFileSetupDlg();
    gboolean ShowDialog(GtkWidget *pWndParent, PDFileSetupRec pFSR);
    void SaveSettings(FILE *fp);
    void RestoreSettings(gint iLeft, gint iTop);

    gboolean Configure(GtkWidget *widget, GdkEvent *event);
    PDUnitList GetUnitList();
    PDPaperSize FindPaper(double dWidth, double dHeight);

    void FontSet(GtkFontButton *widget);
    void OkBtnClick(GtkButton *button);
    void AngUnitCBChanged(GtkComboBox *widget);
    void PaperUnitCBChanged(GtkComboBox *widget);
    void GraphUnitCBChanged(GtkComboBox *widget);
    void GridEditChanged(GtkEntry *widget, gint id);
    void FontAttrChBChange(GtkToggleButton *togglebutton, gint iMask);
} *PDFileSetupDlg;

double RoundFloat(double dx);
void FormatFloatStr(double dx, gchar *sbuf);

#endif

