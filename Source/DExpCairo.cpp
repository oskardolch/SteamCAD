#include "DExpCairo.hpp"
#include <malloc.h>
#include <math.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>
#include <string.h>

const double dMmToIn = 25.4;
const double dPtToIn = 72.0;

cairo_status_t WriteCairoStream(void *closure, const unsigned char *data,
    unsigned int length)
{
    FILE *pfile = (FILE*)closure;
    fwrite(data, 1, length, pfile);
    return CAIRO_STATUS_SUCCESS;
}

void ExportDimArrow(cairo_t *pct, PDPrimitive pPrim)
{
    int iType = Round(pPrim->cPt1.x);
    double dr;

    switch(iType)
    {
    case 1:
        cairo_new_path(pct);
        cairo_move_to(pct, pPrim->cPt3.x, pPrim->cPt3.y);
        cairo_line_to(pct, pPrim->cPt2.x, pPrim->cPt2.y);
        cairo_line_to(pct, pPrim->cPt4.x, pPrim->cPt4.y);
        cairo_stroke(pct);
        break;
    case 2:
        cairo_new_path(pct);
        cairo_move_to(pct, pPrim->cPt3.x, pPrim->cPt3.y);
        cairo_line_to(pct, pPrim->cPt2.x, pPrim->cPt2.y);
        cairo_line_to(pct, pPrim->cPt4.x, pPrim->cPt4.y);
        cairo_close_path(pct);
        cairo_fill(pct);
        cairo_stroke(pct);
        break;
    case 3:
        dr = (pPrim->cPt3.x - pPrim->cPt2.x);
        cairo_new_path(pct);
        cairo_arc(pct, pPrim->cPt2.x, pPrim->cPt2.y, dr, 0.0, 2.0*M_PI);
        cairo_close_path(pct);
        cairo_fill(pct);
        cairo_stroke(pct);
        break;
    case 4:
    case 5:
        cairo_new_path(pct);
        cairo_move_to(pct, pPrim->cPt3.x, pPrim->cPt3.y);
        cairo_line_to(pct, pPrim->cPt4.x, pPrim->cPt4.y);
        cairo_stroke(pct);
        break;
    }
}

void ExportPrimitive(cairo_t *pct, PDPrimitive pPrim)
{
    double dr, da1, da2;
    CDPoint cPt1, cPt2;

    switch(pPrim->iType)
    {
    case 1:
        cairo_new_path(pct);
        cairo_move_to(pct, pPrim->cPt1.x, pPrim->cPt1.y);
        cairo_line_to(pct, pPrim->cPt2.x, pPrim->cPt2.y);
        cairo_stroke(pct);
        break;
    case 2:
        dr = (pPrim->cPt2.x - pPrim->cPt1.x);
        da2 = atan2(pPrim->cPt3.y - pPrim->cPt1.y, pPrim->cPt3.x - pPrim->cPt1.x);
        da1 = atan2(pPrim->cPt4.y - pPrim->cPt1.y, pPrim->cPt4.x - pPrim->cPt1.x);
        cairo_new_path(pct);
        cairo_arc(pct, pPrim->cPt1.x, pPrim->cPt1.y, dr, da1, da2);
        cairo_stroke(pct);
        break;
    case 3:
        dr = (pPrim->cPt2.x - pPrim->cPt1.x);
        cairo_new_path(pct);
        cairo_arc(pct, pPrim->cPt1.x, pPrim->cPt1.y, dr, 0.0, 2.0*M_PI);
        cairo_stroke(pct);
        break;
    case 4:
        cPt1 = (pPrim->cPt1 + 2.0*pPrim->cPt2)/3.0;
        cPt2 = (pPrim->cPt3 + 2.0*pPrim->cPt2)/3.0;
        cairo_new_path(pct);
        cairo_move_to(pct, pPrim->cPt1.x, pPrim->cPt1.y);
        cairo_curve_to(pct, cPt1.x, cPt1.y, cPt2.x, cPt2.y,
            pPrim->cPt3.x, pPrim->cPt3.y);
        cairo_stroke(pct);
        break;
    case 5:
        cairo_new_path(pct);
        cairo_move_to(pct, pPrim->cPt1.x, pPrim->cPt1.y);
        cairo_curve_to(pct, pPrim->cPt2.x, pPrim->cPt2.y,
            pPrim->cPt3.x, pPrim->cPt3.y,
            pPrim->cPt4.x, pPrim->cPt4.y);
        cairo_stroke(pct);
        break;
    case 9:
        ExportDimArrow(pct, pPrim);
        break;
    }
}

void ExportDimText(cairo_t *pct, PDPrimitive pPrim, PDObject pObj, double dScale,
    PDUnitList pUnits, double dRat)
{
    int iPos = Round(pPrim->cPt2.y);

    char sBuf[64];
    char *psBuf = sBuf;
    int iLen = pObj->PreParseDimText(iPos, psBuf, 64, dScale, pUnits);
    if(iLen > 0)
    {
        psBuf = (char*)malloc(iLen*sizeof(char));
        pObj->PreParseDimText(iPos, psBuf, iLen, dScale, pUnits);
    }

    int iLen2 = strlen(psBuf);
    if(iLen2 < 1) return;

    cairo_translate(pct, pPrim->cPt1.x, pPrim->cPt1.y);
    double dPi2 = M_PI/2.0;
    cairo_rotate(pct, pPrim->cPt2.x - dPi2);

    CDFileAttrs cFileAttrs;
    pObj->GetDimFontAttrs(iPos, &cFileAttrs);

    double da = 1.6*cFileAttrs.dFontSize*dRat;

    bool bDiam = (psBuf[0] == '*');
    int iStart = 0;
    if(bDiam) iStart = 1;
    if(iStart >= iLen2)
    {
        cairo_new_path(pct);
        cairo_arc(pct, -0.25*da, -0.325*da, 0.25*da, 0.0, 2*M_PI);
        cairo_move_to(pct, -0.25*da, -0.075*da);
        cairo_line_to(pct, dRat, -0.575*da);
        cairo_stroke(pct);
        cairo_identity_matrix(pct);
        return;
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

    cairo_select_font_face(pct, cFileAttrs.sFontFace, iSlant, iWeight);
    cairo_set_font_size(pct, da);

    cairo_text_extents_t ctexStart, ctexNom, ctexDenom, ctexEnd;
    cairo_text_extents(pct, sBufStart, &ctexStart);

    double dTextWidth = ctexStart.width + ctexStart.x_bearing;

    if(sFrac)
    {
        cairo_text_extents(pct, sBufEnd, &ctexEnd);
        cairo_set_font_size(pct, 0.6*da);
        cairo_text_extents(pct, sBufNom, &ctexNom);
        cairo_text_extents(pct, sBufDenom, &ctexDenom);

        dTextWidth += (ctexNom.width + ctexNom.x_bearing);
        dTextWidth += (ctexDenom.width + ctexDenom.x_bearing);
        dTextWidth += (ctexEnd.width + ctexEnd.x_bearing);
        dTextWidth += 0.065*da;
    }

    cairo_set_font_size(pct, da);

    double dx = -dTextWidth/2.0;
    if(bDiam)
    {
        dTextWidth += 0.625*da;
        dx = 0.25*da - dTextWidth/2.0;
        cairo_new_path(pct);
        cairo_arc(pct, dx, -0.325*da, 0.25*da, 0.0, 2*M_PI);
        cairo_move_to(pct, dx - 0.25*da, -0.075*da);
        cairo_line_to(pct, dx + 0.25*da, -0.575*da);
        cairo_stroke(pct);
        dx += 0.375*da;
    }
    cairo_move_to(pct, dx, 0.0);
    cairo_show_text(pct, sBufStart);

    if(sFrac)
    {
        dx += (ctexStart.width + ctexStart.x_bearing + 0.02*da);
        cairo_set_font_size(pct, 0.6*da);
        cairo_move_to(pct, dx, -0.425*da);
        cairo_show_text(pct, sBufNom);

        dx += (ctexNom.width + ctexNom.x_bearing + 0.03*da);
        cairo_move_to(pct, dx, 0.1*da);
        cairo_show_text(pct, sBufDenom);

        cairo_move_to(pct, dx - 0.175*da, -0.2*da);
        cairo_line_to(pct, dx + 0.175*da, -0.55*da);
        cairo_stroke(pct);

        dx += (ctexDenom.width + ctexDenom.x_bearing + 0.015*da);
        cairo_set_font_size(pct, da);
        cairo_move_to(pct, dx, 0.0);
        cairo_show_text(pct, sBufEnd);
    }

    cairo_identity_matrix(pct);

    if(iLen > 0) free(psBuf);
}

void ExportObject(PDObject pObj, cairo_t *pct, PDFileAttrs pFileAttrs, double dRat,
    cairo_line_cap_t *pcCap, PDUnitList pUnits)
{
    CDLineStyle cStyle = pObj->GetLineStyle();
    double dScale = pFileAttrs->dScaleNom/pFileAttrs->dScaleDenom;

    bool bVisible = (cStyle.dWidth > -0.00001);
    double dLineWdth = fabs(cStyle.dWidth);

    if((fabs(cStyle.dPercent) > 10.0) || (dLineWdth > 0.3))
    {
        if(*pcCap == CAIRO_LINE_CAP_ROUND)
        {
            *pcCap = CAIRO_LINE_CAP_BUTT;
            cairo_set_line_cap(pct, *pcCap);
        }
    }
    else
    {
        if(*pcCap == CAIRO_LINE_CAP_BUTT)
        {
            *pcCap = CAIRO_LINE_CAP_ROUND;
            cairo_set_line_cap(pct, *pcCap);
        }
    }

    cairo_set_source_rgb(pct, 0.0, 0.0, 0.0);
    cairo_set_line_width(pct, dLineWdth*dRat);

    CDPrimitive cPrim;
    pObj->GetFirstPrimitive(&cPrim, -dRat, -2);

    while(cPrim.iType > 0)
    {
        if(cPrim.iType == 10) ExportDimText(pct, &cPrim, pObj, dScale, pUnits, dRat);
        else if(bVisible) ExportPrimitive(pct, &cPrim);
        pObj->GetNextPrimitive(&cPrim, -dRat, -2);
    }

    for(int i = 0; i < pObj->GetDimenCount(); i++)
    {
        pObj->GetFirstPrimitive(&cPrim, -dRat, i);
        while(cPrim.iType > 0)
        {
            if(cPrim.iType == 10) ExportDimText(pct, &cPrim, pObj, dScale, pUnits, dRat);
            else ExportPrimitive(pct, &cPrim);
            pObj->GetNextPrimitive(&cPrim, -dRat, i);
        }
    }
}

void ExportCairoFile(int iType, FILE *pFile, PDataList pDrawData, PDUnitList pUnits)
{
    CDFileAttrs cFileAttrs;
    pDrawData->GetFileAttrs(&cFileAttrs);

    double dMmToPt = dPtToIn/dMmToIn;

    double dWidth = cFileAttrs.dWidth*dMmToPt;
    double dHeight = cFileAttrs.dHeight*dMmToPt;
    cairo_surface_t *pcs = NULL;

    switch(iType)
    {
    case 0:
        pcs = cairo_pdf_surface_create_for_stream(WriteCairoStream,
            pFile, dWidth, dHeight);
        break;
    case 1:
    case 2:
        pcs = cairo_ps_surface_create_for_stream(WriteCairoStream,
            pFile, dWidth, dHeight);
        break;
    case 3:
        dMmToPt = 600.0/dMmToIn;
        dWidth = cFileAttrs.dWidth*dMmToPt;
        dHeight = cFileAttrs.dHeight*dMmToPt;
        pcs =  cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)dWidth, (int)dHeight);
        break;
    case 4:
        pcs = cairo_svg_surface_create_for_stream(WriteCairoStream,
            pFile, dWidth, dHeight);
        break;
    }

    if(!pcs) return;

    if(iType == 2) cairo_ps_surface_set_eps(pcs, true);

    cairo_t *pct = cairo_create(pcs);

    CDRect cRect;
    cRect.cPt1 = 0;
    cRect.cPt2.x = cFileAttrs.dWidth;
    cRect.cPt2.y = cFileAttrs.dHeight;
    pDrawData->BuildAllPrimitives(&cRect);

    cairo_line_cap_t cCap = CAIRO_LINE_CAP_ROUND;
    cairo_set_line_cap(pct, cCap);
    cairo_set_line_join(pct, CAIRO_LINE_JOIN_ROUND);

    PDObject pObj;
    int n = pDrawData->GetCount();
    for(int i = 0; i < n; i++)
    {
        pObj = pDrawData->GetItem(i);
        ExportObject(pObj, pct, &cFileAttrs, dMmToPt, &cCap, pUnits);
    }

    cairo_identity_matrix(pct);

    cairo_save(pct);
    cairo_destroy(pct);
    cairo_surface_flush(pcs);
    if(iType == 3)
    {
        cairo_surface_write_to_png_stream(pcs, WriteCairoStream, pFile);
    }
    cairo_surface_finish(pcs);
    cairo_surface_destroy(pcs);
}

