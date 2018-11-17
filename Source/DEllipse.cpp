#include "DEllipse.hpp"
#include "DPrimitive.hpp"
#include "DMath.hpp"
#include <math.h>

#include <stdio.h>

// for debugging purpose only
/*#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
extern HWND g_hStatus;*/
// -----

bool AddEllipsePoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines)
{
    if((iCtrl == 2) || (iCtrl == 3))
    {
        int nOffs2 = pPoints->GetCount(2);
        int nOffs3 = pPoints->GetCount(3);
        if(nOffs2 > 0) pPoints->SetPoint(0, 2, x, y, iCtrl);
        else if(nOffs3 > 0) pPoints->SetPoint(0, 3, x, y, iCtrl);
        else pPoints->AddPoint(x, y, iCtrl);
        return true;
    }

    bool bRes = false;
    int nNorm = pPoints->GetCount(0);
    int nCtrl = pPoints->GetCount(1);

    if(iInputLines == 2)
    {
        if(iCtrl < 1)
        {
            if(nNorm < 2)
            {
                pPoints->AddPoint(x, y, iCtrl);
                nNorm++;
            }
        }
        bRes = (nNorm > 1);
    }
    else
    {
        if(iCtrl == 1)
        {
            if(nCtrl < 2)
            {
                pPoints->AddPoint(x, y, iCtrl);
                nCtrl++;
            }
        }
        else if(nCtrl > 1)
        {
            if(nNorm < 2)
            {
                pPoints->AddPoint(x, y, iCtrl);
                nNorm++;
            }
        }
        bRes = (nCtrl > 1) && (nNorm > 0);
    }
    return bRes;
}

double GetElspBreakAngle(double dr, double da, double db, double dr1, double dr2)
{
    double dRes = -1.0;
    if((dr > dr1 - g_dPrec) && (dr < dr2 + g_dPrec))
    {
        if(dr < dr1 + g_dPrec) dRes = 0.0;
        else if(dr > dr2 - g_dPrec) dRes = M_PI/2.0;
        else
        {
            double da2 = Power2(da);
            double db2 = Power2(db);
            double d1 = cbrt(Power2(da*db*dr));
            double d2 = da2 - db2;
            double dsi = sqrt((d1 - db2)/d2);
            double dco = sqrt((da2 - d1)/d2);
            dRes = atan2(dsi, dco);
        }
    }
    return dRes;
}

double GetElpsFullLengthHalf(double da, double db, double dOffset, double dBreak)
{
    CDPrimitive cPrim;
    CDPoint cDir1, cDir2;

    double dLength = 0.0;
    double dco, dsi, dt;

    cPrim.cPt3.x = da + dOffset;
    cPrim.cPt3.y = 0.0;
    cDir2.x = 0.0;
    cDir2.y = 1.0;

    int i1 = 0;
    int i2 = 8;
    double dAng1 = 0.0;
    double dAng2 = M_PI/2.0;

    if((dBreak > g_dPrec) && (dBreak < dAng2 - g_dPrec))
    {
        dAng1 = dBreak;
        i1 = Round(8.0*dAng1/dAng2);
        if(i1 < 1) i1 = 1;
        i2 = 8 - i1;
        dAng2 -= dAng1;

        for(int i = 0; i < i1; i++)
        {
            cPrim.cPt1 = cPrim.cPt3;
            cDir1 = cDir2;
            dt = (double)dAng1*(i + 1)/i1;
            dco = cos(dt);
            dsi = sin(dt);
            cDir2.x = -da*dsi;
            cDir2.y = db*dco;
            dt = GetNorm(cDir2);
            cPrim.cPt3.x = da*dco + dOffset*cDir2.y/dt;
            cPrim.cPt3.y = db*dsi - dOffset*cDir2.x/dt;
            LineXLine(cPrim.cPt1, cDir1, cPrim.cPt3, cDir2, &cPrim.cPt2);
            dLength += GetQuadLength(&cPrim, 0.0, 1.0);
        }
    }

    for(int i = 0; i < i2; i++)
    {
        cPrim.cPt1 = cPrim.cPt3;
        cDir1 = cDir2;
        dt = dAng1 + (double)dAng2*(i + 1)/i2;
        dco = cos(dt);
        dsi = sin(dt);
        cDir2.x = -da*dsi;
        cDir2.y = db*dco;
        dt = GetNorm(cDir2);
        cPrim.cPt3.x = da*dco + dOffset*cDir2.y/dt;
        cPrim.cPt3.y = db*dsi - dOffset*cDir2.x/dt;
        LineXLine(cPrim.cPt1, cDir1, cPrim.cPt3, cDir2, &cPrim.cPt2);
        dLength += GetQuadLength(&cPrim, 0.0, 1.0);
    }

    return dLength;
}

bool BuildEllipseCache(CDLine cTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    PDLine pLines, double *pdDist)
{
    pCache->ClearAll();

    int nNorm = pPoints->GetCount(0);
    int nCtrl = pPoints->GetCount(1);

    CDPoint cOrig, cMainDir, cPt1, cPt2, cPt3;
    double d1, d2, d3, da = -1.0, db = -1.0;
    double dr1 = -1.0, dr2 = -1.0;

    if(pLines[0].bIsSet && pLines[1].bIsSet)
    {
        CDPoint cDir1 = pLines[0].cDirection;
        CDPoint cDir2 = pLines[1].cDirection;

        int iX = LineXLine(pLines[0].cOrigin, cDir1, pLines[1].cOrigin, cDir2, &cOrig);
        if(iX < 1) return false;

        if(((nNorm < 1) && (iMode == 1)) || ((nNorm == 1) && (iMode != 1)))
        {
            if(iMode == 1) cPt1 = cTmpPt.cOrigin - cOrig;
            else if(nNorm > 0) cPt1 = pPoints->GetPoint(0, 0).cPoint - cOrig;
            else return false;

            d1 = fabs(Deter2(cDir1, cDir2));
            if(d1 < g_dPrec) return false;

            dr1 = sqrt(Power2(Deter2(cPt1, cDir1)) + Power2(Deter2(cPt1, cDir2)))/d1;
            dr2 = dr1;
        }
        else
        {
            if(nNorm > 0) cPt1 = pPoints->GetPoint(0, 0).cPoint - cOrig;
            else return false;

            if(iMode == 1) cPt2 = cTmpPt.cOrigin - cOrig;
            else if(nNorm > 1) cPt2 = pPoints->GetPoint(1, 0).cPoint - cOrig;
            else return false;

            CDPoint cMat1, cMat2;
            cMat1.x = Power2(Deter2(cPt1, cDir1));
            cMat1.y = Power2(Deter2(cPt2, cDir1));
            cMat2.x = Power2(Deter2(cPt1, cDir2));
            cMat2.y = Power2(Deter2(cPt2, cDir2));

            db = Power2(Deter2(cDir1, cDir2));
            da = Deter2(cMat1, cMat2);

            d1 = db*(cMat1.x - cMat1.y);
            if(fabs(d1) < g_dPrec) return false;

            d2 = da/d1;
            if(d2 < -g_dPrec) return false;
            if(d2 < g_dPrec) d2 = 0;

            dr1 = sqrt(d2);

            d1 = db*(cMat2.y - cMat2.x);
            if(fabs(d1) < g_dPrec) return false;

            d2 = da/d1;
            if(d2 < -g_dPrec) return false;
            if(d2 < g_dPrec) d2 = 0;

            dr2 = sqrt(d2);
        }

        double dco, dsi;

        if(fabs(dr1 - dr2) < g_dPrec)
        {
            dco = sqrt(2.0)/2.0;
            dsi = dco;
        }
        else
        {
            double dTanT = 2.0*dr1*dr2*(cDir1*cDir2)/(Power2(dr1) - Power2(dr2));
            double dt = atan(dTanT)/2.0;
            dco = cos(dt);
            dsi = sin(dt);
        }
        cMainDir = dco*dr1*cDir1 + dsi*dr2*cDir2;
        da = GetNorm(cMainDir);
        cMainDir /= da;

        d1 = dsi;
        dsi = dco;
        dco = -d1;

        cPt1 = dco*dr1*cDir1 + dsi*dr2*cDir2;
        db = GetNorm(cPt1);

        if(db > da)
        {
            dco = cMainDir.x;
            dsi = cMainDir.y;
            cMainDir.x = -dsi;
            cMainDir.y = dco;
            dsi = da;
            da = db;
            db = dsi;
        }
    }
    else if(nCtrl > 0)
    {
        if(nCtrl < 2)
        {
            cPt1 = pPoints->GetPoint(0, 1).cPoint;
            if(nNorm > 0) cPt2 = pPoints->GetPoint(0, 0).cPoint;
            else if(iMode == 1) cPt2 = cTmpPt.cOrigin;
            else return false;

            pCache->AddPoint(cPt1.x, cPt1.y, 0);
            pCache->AddPoint(cPt2.x, cPt2.y, 0);
            return true;
        }

        cPt1 = pPoints->GetPoint(0, 1).cPoint;
        cPt2 = pPoints->GetPoint(1, 1).cPoint;

        if(iMode == 1) cPt3 = cTmpPt.cOrigin;
        else if(nNorm > 0) cPt3 = pPoints->GetPoint(0, 0).cPoint;
        else return false;

        CDPoint cDir = cPt2 - cPt1;
        d1 = GetNorm(cDir);

        if(d1 < g_dPrec) return false;

        d2 = GetDist(cPt1, cPt3);
        d3 = GetDist(cPt2, cPt3);

        da = (d2 + d3)/2.0;
        db = sqrt(Power2(d2 + d3) - Power2(d1))/2.0;

        cMainDir = cDir/d1;
        cOrig = (cPt2 + cPt1)/2.0;
    }

    if((da > g_dPrec) && (db > g_dPrec))
    {
        pCache->AddPoint(cOrig.x, cOrig.y, 0);
        pCache->AddPoint(da, db, 0);
        pCache->AddPoint(cMainDir.x, cMainDir.y, 0);

        dr1 = Power2(db)/da;
        dr2 = Power2(da)/db;
        pCache->AddPoint(dr1, dr2, 3);

        double dr = -1.0;
        double dl1, dl2;

        if((iMode == 2) && (cTmpPt.cDirection.x > 0.5))
        {
            dr = GetElspBreakAngle(-cTmpPt.cDirection.y, da, db, dr1, dr2);
            pCache->AddPoint(dr, 0.0, 4);
            dl1 = 2.0*GetElpsFullLengthHalf(da, db, cTmpPt.cDirection.y, dr);
            dl2 = GetElpsPointAtDist(da, db, cTmpPt.cDirection.y, dr, dl1);
            pCache->AddPoint(dl1, dl2, 4);
            if(pdDist) *pdDist = cTmpPt.cDirection.y;
            pCache->AddPoint(cTmpPt.cDirection.y, 0.0, 2);
            return true;
        }

        int nOffs2 = pPoints->GetCount(2);
        int nOffs3 = pPoints->GetCount(3);
        int iSrchMask = 0;
        if((iMode == 2) || (nOffs2 > 0) || (nOffs3 > 0))
        {
            if(iMode == 2)
            {
                cPt1 = cTmpPt.cOrigin;
                if(cTmpPt.cDirection.x < -0.5) iSrchMask = 2;
            }
            else if(nOffs2 > 0) cPt1 = pPoints->GetPoint(0, 2).cPoint;
            else
            {
                cPt1 = pPoints->GetPoint(0, 3).cPoint;
                iSrchMask = 2;
            }

            CDLine cPtX;
            double dDist = GetElpsDistFromPt(cPt1, cPt1, iSrchMask, pCache, &cPtX, NULL);
            double dDistOld = 0.0;

            if(iMode == 2)
            {
                if(nOffs2 > 0)
                {
                    cPt1 = pPoints->GetPoint(0, 2).cPoint;
                    dDistOld = GetElpsDistFromPt(cPt1, cPt1, 0, pCache, &cPtX, NULL);
                }
                else if(nOffs3 > 0)
                {
                    cPt1 = pPoints->GetPoint(0, 3).cPoint;
                    dDistOld = GetElpsDistFromPt(cPt1, cPt1, 2, pCache, &cPtX, NULL);
                }
            }

            if(pdDist) *pdDist = dDist - dDistOld;
            if(fabs(dDist) > g_dPrec) pCache->AddPoint(dDist, dDistOld, 2);

            dr = GetElspBreakAngle(-dDist, da, db, dr1, dr2);
            pCache->AddPoint(dr, 0.0, 4);
            dl1 = 2.0*GetElpsFullLengthHalf(da, db, dDist, dr);
            dl2 = GetElpsPointAtDist(da, db, dDist, dr, dl1);
            pCache->AddPoint(dl1, dl2, 4);
        }
        else
        {
            pCache->AddPoint(-1.0, 0.0, 4);
            dl1 = 2.0*GetElpsFullLengthHalf(da, db, 0.0, -1.0);
            dl2 = GetElpsPointAtDist(da, db, 0.0, -1.0, dl1);
            pCache->AddPoint(dl1, dl2, 4);
        }
    }

    return true;
}

int BuildEllipseWithBounds(double da, double db, double dr, double dtStart, double dtEnd,
    CDPoint cOrig, CDPoint cMainDir, PDPrimObject pPrimList, PDRect pRect)
{
    CDPrimitive cPrim, cTmpPrim;
    cPrim.iType = 5;

    if(dtStart > dtEnd) dtEnd += 2.0*M_PI;

    double dAngle = dtEnd - dtStart;
    int iParts = 2 + (int)4.0*dAngle/M_PI;

    CDPoint cPts[5];
    double dt, dt1, dt2, dtDist;
    dtDist = dAngle/(double)iParts;
    dt2 = dtStart;

    CDPoint cDirStart, cDirEnd;
    double dco, dsi;

    dco = cos(dt2);
    dsi = sin(dt2);
    cDirEnd.x = -da*dsi;
    cDirEnd.y = db*dco;
    double dNorm = GetNorm(cDirEnd);

    cPts[4].x = da*dco + dr*cDirEnd.y/dNorm;
    cPts[4].y = db*dsi - dr*cDirEnd.x/dNorm;

    int iRes1 = 2;
    int iRes2 = 0;
    int k;

    for(int i = 0; i < iParts; i++)
    {
        dt1 = dt2;
        dt2 += dtDist;
        cPts[0] = cPts[4];
        cDirStart = cDirEnd;

        for(int j = 1; j < 5; j++)
        {
            dt = dt1 + (dt2 - dt1)*(double)j/4.0;
            dco = cos(dt);
            dsi = sin(dt);

            cDirEnd.x = -da*dsi;
            cDirEnd.y = db*dco;
            dNorm = GetNorm(cDirEnd);

            cPts[j].x = da*dco + dr*cDirEnd.y/dNorm;
            cPts[j].y = db*dsi - dr*cDirEnd.x/dNorm;
        }

        if(ApproxLineSeg(5, cPts, &cDirStart, &cDirEnd, &cTmpPrim) > -0.5)
        {
            cPrim.iType = 5;
            cPrim.cPt1 = cOrig + Rotate(cTmpPrim.cPt1, cMainDir, true);
            cPrim.cPt2 = cOrig + Rotate(cTmpPrim.cPt2, cMainDir, true);
            cPrim.cPt3 = cOrig + Rotate(cTmpPrim.cPt3, cMainDir, true);
            cPrim.cPt4 = cOrig + Rotate(cTmpPrim.cPt4, cMainDir, true);
        }
        else
        {
            cPrim.iType = 1;
            cPrim.cPt1 = cOrig + Rotate(cPts[0], cMainDir, true);
            cPrim.cPt2 = cOrig + Rotate(cPts[4], cMainDir, true);
        }

        k = CropPrimitive(cPrim, pRect, pPrimList);
        if(k < iRes1) iRes1 = 1;
        if(k > iRes2) iRes2 = 1;
    }
    if(iRes2 < 1) iRes1 = 0;
    return iRes1;
}

int BuildEllipseWithBoundsBreaks(double da, double db, double dr, double dtStart, double dtEnd,
    CDPoint cOrig, CDPoint cMainDir, double dAngle, bool bFullCycle, PDPrimObject pPrimList, PDRect pRect)
{
    int iRes = -1;
    int iSegs, k;
    double dBnds[4];

    iSegs = MergeBounds(dtStart, dtEnd, dAngle - M_PI, -dAngle, bFullCycle, dBnds);
    for(int i = 0; i < iSegs; i++)
    {
        k = BuildEllipseWithBounds(da, db, dr, dBnds[2*i], dBnds[2*i + 1], cOrig, cMainDir, pPrimList, pRect);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    iSegs = MergeBounds(dtStart, dtEnd, -dAngle, dAngle, bFullCycle, dBnds);
    for(int i = 0; i < iSegs; i++)
    {
        k = BuildEllipseWithBounds(da, db, dr, dBnds[2*i], dBnds[2*i + 1], cOrig, cMainDir, pPrimList, pRect);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    iSegs = MergeBounds(dtStart, dtEnd, dAngle, M_PI - dAngle, bFullCycle, dBnds);
    for(int i = 0; i < iSegs; i++)
    {
        k = BuildEllipseWithBounds(da, db, dr, dBnds[2*i], dBnds[2*i + 1], cOrig, cMainDir, pPrimList, pRect);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    iSegs = MergeBounds(dtStart, dtEnd, M_PI - dAngle, dAngle - M_PI, bFullCycle, dBnds);
    for(int i = 0; i < iSegs; i++)
    {
        k = BuildEllipseWithBounds(da, db, dr, dBnds[2*i], dBnds[2*i + 1], cOrig, cMainDir, pPrimList, pRect);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    return iRes;
}

int BuildEllipseQuadsWithBounds(double da, double db, double dr, double dtStart, double dtEnd,
    CDPoint cOrig, CDPoint cMainDir, PDPrimObject pPrimList, PDRect pRect)
{
    CDPrimitive cPrim, cTmpPrim;
    cPrim.iType = 4;

    if(dtStart > dtEnd) dtEnd += 2.0*M_PI;

    double dAngle = dtEnd - dtStart;
    int iParts = 2 + (int)4.0*dAngle/M_PI;

    double dt2, dtDist;
    dtDist = dAngle/(double)iParts;
    dt2 = dtStart;

    CDPoint cDirStart, cDirEnd;
    double dco, dsi;

    dco = cos(dt2);
    dsi = sin(dt2);
    cDirEnd.x = -da*dsi;
    cDirEnd.y = db*dco;
    double dNorm = GetNorm(cDirEnd);

    cTmpPrim.cPt3.x = da*dco + dr*cDirEnd.y/dNorm;
    cTmpPrim.cPt3.y = db*dsi - dr*cDirEnd.x/dNorm;

    int iRes1 = 2;
    int iRes2 = 0;
    int k;

    for(int i = 0; i < iParts; i++)
    {
        dt2 += dtDist;
        cTmpPrim.cPt1 = cTmpPrim.cPt3;
        cDirStart = cDirEnd;

        dco = cos(dt2);
        dsi = sin(dt2);

        cDirEnd.x = -da*dsi;
        cDirEnd.y = db*dco;
        dNorm = GetNorm(cDirEnd);
        cTmpPrim.cPt3.x = da*dco + dr*cDirEnd.y/dNorm;
        cTmpPrim.cPt3.y = db*dsi - dr*cDirEnd.x/dNorm;
        LineXLine(cTmpPrim.cPt1, cDirStart, cTmpPrim.cPt3, cDirEnd, &cTmpPrim.cPt2);

        cPrim.cPt1 = cOrig + Rotate(cTmpPrim.cPt1, cMainDir, true);
        cPrim.cPt2 = cOrig + Rotate(cTmpPrim.cPt2, cMainDir, true);
        cPrim.cPt3 = cOrig + Rotate(cTmpPrim.cPt3, cMainDir, true);

        k = CropPrimitive(cPrim, pRect, pPrimList);
        if(k < iRes1) iRes1 = 1;
        if(k > iRes2) iRes2 = 1;
    }
    if(iRes2 < 1) iRes1 = 0;
    return iRes1;
}

int BuildEllipseQuadsWithBoundsBreaks(double da, double db, double dr, double dtStart, double dtEnd,
    CDPoint cOrig, CDPoint cMainDir, double dAngle, bool bFullCycle, PDPrimObject pPrimList, PDRect pRect)
{
    int iRes = -1;
    int iSegs, k;
    double dBnds[4];

    iSegs = MergeBounds(dtStart, dtEnd, dAngle - M_PI, -dAngle, bFullCycle, dBnds);
    for(int i = 0; i < iSegs; i++)
    {
        k = BuildEllipseQuadsWithBounds(da, db, dr, dBnds[2*i], dBnds[2*i + 1], cOrig, cMainDir, pPrimList, pRect);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    iSegs = MergeBounds(dtStart, dtEnd, -dAngle, dAngle, bFullCycle, dBnds);
    for(int i = 0; i < iSegs; i++)
    {
        k = BuildEllipseQuadsWithBounds(da, db, dr, dBnds[2*i], dBnds[2*i + 1], cOrig, cMainDir, pPrimList, pRect);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    iSegs = MergeBounds(dtStart, dtEnd, dAngle, M_PI - dAngle, bFullCycle, dBnds);
    for(int i = 0; i < iSegs; i++)
    {
        k = BuildEllipseQuadsWithBounds(da, db, dr, dBnds[2*i], dBnds[2*i + 1], cOrig, cMainDir, pPrimList, pRect);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    iSegs = MergeBounds(dtStart, dtEnd, M_PI - dAngle, dAngle - M_PI, bFullCycle, dBnds);
    for(int i = 0; i < iSegs; i++)
    {
        k = BuildEllipseQuadsWithBounds(da, db, dr, dBnds[2*i], dBnds[2*i + 1], cOrig, cMainDir, pPrimList, pRect);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    return iRes;
}

int BuildEllipsePrimitives(CDLine cTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly)
{
    if(iMode > 0) BuildEllipseCache(cTmpPt, iMode, pPoints, pCache, pLines, pdDist);

    int iCnt = pCache->GetCount(0);

    if(iCnt < 2) return 0;

    CDPoint cOrig, cRad, cMainDir;

    CDPrimitive cPrim;
    int iRes = 0;

    if(iCnt < 3)
    {
        cPrim.iType = 1;
        cPrim.cPt1 = pCache->GetPoint(0, 0).cPoint;
        cPrim.cPt2 = pCache->GetPoint(1, 0).cPoint;
        cPrim.cPt3 = 0;
        cPrim.cPt4 = 0;
        pDrawBnds->y = 0;
        pDrawBnds->x = GetDist(cPrim.cPt1, cPrim.cPt2);
        return CropPrimitive(cPrim, pRect, pPrimList);
    }

    cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;
    cMainDir = pCache->GetPoint(2, 0).cPoint;

    if(pCache->GetCount(3) > 0)
    {
        CDPrimitive cPrimPt;
        CDPoint cPt2 = pCache->GetPoint(0, 3).cPoint;
        CDPoint cCenter;
        cCenter.x = cRad.x - cPt2.x;
        cCenter.y = 0.0;

        cPrimPt.iType = 7;
        cPrimPt.cPt1.x = 1;
        cPrimPt.cPt1.y = 1;
        cPrimPt.cPt2 = cOrig + Rotate(cCenter, cMainDir, true);
        cCenter.x *= -1.0;
        cPrimPt.cPt3 = cOrig + Rotate(cCenter, cMainDir, true);
        cPrimPt.cPt4 = 0;
        CropPrimitive(cPrimPt, pRect, pPrimList);

        cCenter.x = 0.0;
        cCenter.y = cPt2.y - cRad.y;

        cPrimPt.iType = 7;
        cPrimPt.cPt1.x = 1;
        cPrimPt.cPt1.y = 1;
        cPrimPt.cPt2 = cOrig + Rotate(cCenter, cMainDir, true);
        cCenter.y *= -1.0;
        cPrimPt.cPt3 = cOrig + Rotate(cCenter, cMainDir, true);
        cPrimPt.cPt4 = 0;
        CropPrimitive(cPrimPt, pRect, pPrimList);
    }

    double dr = dOffset;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr += pCache->GetPoint(0, 2).cPoint.x;

    int iBreaks = pCache->GetCount(4);
    CDPoint cBreak = {-1.0, 0.0};
    CDPoint cLengths = {0.0, 0.0};
    if(iBreaks > 0)
    {
        cBreak = pCache->GetPoint(0, 4).cPoint;
        cLengths = pCache->GetPoint(1, 4).cPoint;
    }

    pDrawBnds->y = cLengths.x;
    pDrawBnds->x = -pDrawBnds->y;

    if(pBounds[0].bIsSet && pBounds[1].bIsSet)
    {
        if(bQuadsOnly)
        {
            if(cBreak.x > -0.5)
                iRes = BuildEllipseQuadsWithBoundsBreaks(cRad.x, cRad.y, dr, pBounds[0].dRef, pBounds[1].dRef,
                    cOrig, cMainDir, cBreak.x, false, pPrimList, pRect);
            else iRes = BuildEllipseQuadsWithBounds(cRad.x, cRad.y, dr, pBounds[0].dRef,
                pBounds[1].dRef, cOrig, cMainDir, pPrimList, pRect);
        }
        else
        {
            if(cBreak.x > -0.5)
                iRes = BuildEllipseWithBoundsBreaks(cRad.x, cRad.y, dr, pBounds[0].dRef, pBounds[1].dRef,
                    cOrig, cMainDir, cBreak.x, false, pPrimList, pRect);
            else iRes = BuildEllipseWithBounds(cRad.x, cRad.y, dr, pBounds[0].dRef,
                pBounds[1].dRef, cOrig, cMainDir, pPrimList, pRect);
        }
    }
    else
    {
        if(bQuadsOnly)
        {
            if(cBreak.x > -0.5)
                iRes = BuildEllipseQuadsWithBoundsBreaks(cRad.x, cRad.y, dr, -M_PI, M_PI, cOrig, cMainDir,
                    cBreak.x, true, pPrimList, pRect);
            else iRes = BuildEllipseQuadsWithBounds(cRad.x, cRad.y, dr, -M_PI, M_PI, cOrig, cMainDir,
                pPrimList, pRect);
        }
        else
        {
            if(cBreak.x > -0.5)
                iRes = BuildEllipseWithBoundsBreaks(cRad.x, cRad.y, dr, -M_PI, M_PI, cOrig, cMainDir,
                    cBreak.x, true, pPrimList, pRect);
            else iRes = BuildEllipseWithBounds(cRad.x, cRad.y, dr, -M_PI, M_PI, cOrig, cMainDir,
                pPrimList, pRect);
        }
    }

    return iRes;
}

bool GetElpsPtProjFromStartPt(double da, double db, CDPoint cPt, PDPoint pProj)
{
    int j = 0;
    double dA = Power2(da) - Power2(db);
    double dB = db*cPt.y;
    double dC = da*cPt.x;

    double du1 = pProj->x;
    double dv1 = pProj->y;
    double du2, dv2;

    double df1 = dA*du1*dv1 + dB*du1 - dC*dv1;
    double df2 = Power2(du1) + Power2(dv1) - 1.0;
    double df11 = dA*dv1 + dB;
    double df12 = dA*du1 - dC;
    double df21 = 2.0*du1;
    double df22 = 2.0*dv1;

    double dDet = df11*df22 - df12*df21;
    bool bFound = (fabs(df1) < g_dPrec) && (fabs(df2) < g_dPrec);
    while(!bFound && (j < 8) && (fabs(dDet) > g_dPrec))
    {
        du2 = du1 - (df1*df22 - df2*df12)/dDet;
        dv2 = dv1 - (-df1*df21 + df2*df11)/dDet;
        du1 = du2;
        dv1 = dv2;

        df1 = dA*du1*dv1 + dB*du1 - dC*dv1;
        df2 = Power2(du1) + Power2(dv1) - 1.0;
        df11 = dA*dv1 + dB;
        df12 = dA*du1 - dC;
        df21 = 2.0*du1;
        df22 = 2.0*dv1;

        dDet = df11*df22 - df12*df21;
        bFound = (fabs(df1) < g_dPrec) && (fabs(df2) < g_dPrec);
        j++;
    }
    pProj->x = du1;
    pProj->y = dv1;

    return bFound;
}

bool PtInList(CDPoint cPt, int iSize, PDPoint pList)
{
    int i = 0;
    bool bFound = false;
    while(!bFound && (i < iSize))
    {
        bFound = GetDist(cPt, pList[i++]) < g_dPrec;
    }
    return bFound;
}

// return cos and sin of dt
CDPoint GetElpsPtProj(double da, double db, CDPoint cPt, CDPoint cRefPt, double dOffset, int iSrchMask, PDRefPoint pBounds)
{
    double d1 = Power2(da) - Power2(db);
    double dDist2;
    CDPoint cPt1, cPt2;
    double dNorm, dDist, dDistMin;
    CDPoint cProj, cProjMin;

    if(fabs(d1) < g_dPrec)
    {
        dDist2 = GetNorm(cPt);
        if(dDist2 > g_dPrec)
        {
            cProjMin = cPt/dDist2;
            cPt1 = da*cProjMin;
            dDistMin = GetDist(cRefPt, cPt1);
            cProj = -1.0*cProjMin;
            cPt1 = da*cProj;
            dDist = GetDist(cRefPt, cPt1);
            if(dDist < dDistMin) cProjMin = cProj;
            return cProjMin;
        }

        dDist2 = GetNorm(cRefPt);
        if(dDist2 > g_dPrec) return cRefPt/dDist2;
        cProjMin.x = 6.0;
        return cProjMin;
    }

    dNorm = sqrt(Power2(cRefPt.x/da) + Power2(cRefPt.y/db));
    if(dNorm < g_dPrec)
    {
        cProjMin.x = 6.0;
        return cProjMin;
    }

    int iSols = 0;
    CDPoint cSols[4];
    bool bInBounds[4];

    cProj.x = 1.0;
    cProj.y = 0.0;
    if(GetElpsPtProjFromStartPt(da, db, cPt, &cProj)) cSols[iSols++] = cProj;
    cProj.x = -1.0;
    cProj.y = 0.0;
    if(GetElpsPtProjFromStartPt(da, db, cPt, &cProj) && !PtInList(cProj, iSols, cSols)) cSols[iSols++] = cProj;
    cProj.x = 0.0;
    cProj.y = 1.0;
    if(GetElpsPtProjFromStartPt(da, db, cPt, &cProj) && !PtInList(cProj, iSols, cSols)) cSols[iSols++] = cProj;
    cProj.x = 0.0;
    cProj.y = -1.0;
    if(GetElpsPtProjFromStartPt(da, db, cPt, &cProj) && !PtInList(cProj, iSols, cSols)) cSols[iSols++] = cProj;

    double dt;
    if(pBounds && pBounds[0].bIsSet && pBounds[1].bIsSet)
    {
        for(int j = 0; j < iSols; j++)
        {
            dt = atan2(cSols[j].y, cSols[j].x);
            bInBounds[j] = (RefInBounds(pBounds[0].dRef, pBounds[1].dRef, dt) > 0);
        }
    }
    else
    {
        for(int j = 0; j < iSols; j++) bInBounds[j] = true;
    }

    cProjMin = cSols[0];

    cPt1.x = da*cProjMin.x;
    cPt1.y = db*cProjMin.y;
    cPt2.x = db*cProjMin.x;
    cPt2.y = da*cProjMin.y;
    dNorm = GetNorm(cPt2);
    dDistMin = GetDist(cRefPt - dOffset*cPt2/dNorm, cPt1);
    bool bFirstSet = bInBounds[0];
    bool bIsBetter;

    bool bSecondSet = false;
    CDPoint cProjSecond = cProjMin;
    double dDistSecond = dDistMin;

    int i = 1;
    while(i < iSols)
    {
        cProj = cSols[i];
        cPt1.x = da*cProj.x;
        cPt1.y = db*cProj.y;
        cPt2.x = db*cProj.x;
        cPt2.y = da*cProj.y;
        dNorm = GetNorm(cPt2);
        dDist = GetDist(cRefPt - dOffset*cPt2/dNorm, cPt1);

        bIsBetter = ((dDist < dDistMin - g_dPrec) && !(bFirstSet && !bInBounds[i])) || (!bFirstSet && bInBounds[i]);

        if(bIsBetter)
        {
            if(bSecondSet)
            {
                cProjSecond = cProjMin;
                dDistSecond = dDistMin;
            }
            cProjMin = cProj;
            dDistMin = dDist;
            bFirstSet = bInBounds[i];
        }
        else if(bSecondSet)
        {
            if(dDist < dDistSecond)
            {
                cProjSecond = cProj;
                dDistSecond = dDist;
            }
        }
        else
        {
            cProjSecond = cProj;
            dDistSecond = dDist;
            bSecondSet = true;
        }
        i++;
    }

    if(iSrchMask & 2) return cProjSecond;

    return cProjMin;
}

double GetElpsDistFromPt(CDPoint cPt, CDPoint cRefPt, int iSrchMask, PDPointList pCache, PDLine pPtX, PDRefPoint pBounds)
{
    pPtX->bIsSet = false;
    pPtX->dRef = 0.0;
    pPtX->cOrigin = 0;
    pPtX->cDirection = 0;

    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return -1.0;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    double dDist = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dDist = pCache->GetPoint(0, 2).cPoint.x;

    CDPoint cPt1 = Rotate(cPt - cOrig, cMainDir, false);
    CDPoint cRefPt1 = Rotate(cRefPt - cOrig, cMainDir, false);

    double dMin = -1.0;
    double dNorm;

    if((pCache->GetCount(3) > 0) && (iSrchMask & 1))
    {
        CDPoint cPt2 = pCache->GetPoint(0, 3).cPoint;
        CDPoint cPt3 = {cRad.x - cPt2.x, 0.0};
        pPtX->bIsSet = true;
        pPtX->cOrigin = cOrig + Rotate(cPt3, cMainDir, true);
        dMin = GetDist(cRefPt1, cPt3);

        cPt3.x *= -1.0;
        dNorm = GetDist(cRefPt1, cPt3);
        if(dNorm < dMin)
        {
            dMin = dNorm;
            pPtX->cOrigin = cOrig + Rotate(cPt3, cMainDir, true);
        }

        cPt3.x = 0.0;
        cPt3.y = cPt2.y - cRad.y;
        dNorm = GetDist(cRefPt1, cPt3);
        if(dNorm < dMin)
        {
            dMin = dNorm;
            pPtX->cOrigin = cOrig + Rotate(cPt3, cMainDir, true);
        }

        cPt3.y *= -1.0;
        dNorm = GetDist(cRefPt1, cPt3);
        if(dNorm < dMin)
        {
            dMin = dNorm;
            pPtX->cOrigin = cOrig + Rotate(cPt3, cMainDir, true);
        }
    }

    CDPoint cProj = GetElpsPtProj(cRad.x, cRad.y, cPt1, cRefPt1, dDist, iSrchMask, pBounds);
    if(cProj.x > 4.0) return dMin;

    CDPoint cProjDir, cProjPt, cProjOrg;

    cProjDir.x = cRad.y*cProj.x;
    cProjDir.y = cRad.x*cProj.y;
    dNorm = GetNorm(cProjDir);
    if(dNorm < g_dPrec) return dMin;

    cProjDir /= dNorm;
    cProjPt.x = cRad.x*cProj.x;
    cProjPt.y = cRad.y*cProj.y;
    double dNorm2 = GetDist(cProjPt + dDist*cProjDir, cRefPt1);

    double da2 = Power2(cRad.x);
    double db2 = Power2(cRad.y);
    cProjOrg.x = (da2 - db2)*Power3(cProj.x)/cRad.x;
    cProjOrg.y = (db2 - da2)*Power3(cProj.y)/cRad.y;

    double d1 = GetDist(cRefPt1, cProjOrg);
    double d2 = GetDist(cProjPt, cProjOrg);

    CDPoint cPt2 = Rotate(cRefPt1 - cProjOrg, cProjDir, false);
    double dDir = 1.0;
    if(cPt2.x < -g_dPrec) dDir = -1.0;

    dNorm = dDir*d1 - d2 - dDist;
    if(!pPtX->bIsSet || (dNorm2 < dMin - g_dPrec))
    {
        dMin = dNorm;
        pPtX->bIsSet = true;
        pPtX->cOrigin = cOrig + Rotate(cProjPt + dDist*cProjDir, cMainDir, true);
        pPtX->cDirection = Rotate(cProjDir, cMainDir, true);
        pPtX->dRef = atan2(cProj.y, cProj.x);
    }

    return dMin;
}

bool GetElpsRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
    PDPointList pCache)
{
    if(iMode != 2) return false;

    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return false;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    double dDist = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dDist = pCache->GetPoint(0, 2).cPoint.y;

    double dRad = dDist + dRestrictValue;

    CDPoint cPt1 = Rotate(cPt - cOrig, cMainDir, false);
    CDPoint cProj = GetElpsPtProj(cRad.x, cRad.y, cPt1, cPt1, dDist, 0, NULL);

    CDPoint cDir;
    cDir.x = cRad.y*cProj.x;
    cDir.y = cRad.x*cProj.y;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return false;

    CDPoint cPt2;
    cPt2.x = cRad.x*cProj.x + dRad*cDir.x/dNorm;
    cPt2.y = cRad.y*cProj.y + dRad*cDir.y/dNorm;
    *pSnapPt = cOrig + Rotate(cPt2, cMainDir, true);
    return true;
}

bool HasElpsEnoughPoints(PDPointList pPoints, int iInputLines)
{
    int nNorm = pPoints->GetCount(0);
    int nCtrl = pPoints->GetCount(1);
    bool bRes = false;

    if(iInputLines == 2) bRes = (nNorm > 0);
    else bRes = (nCtrl > 1) && (nNorm > 0);

    return bRes;
}

double GetElpsRadiusAtPt(CDPoint cPt, PDPointList pCache, PDLine pPtR, bool bNewPt,
    PDPointList pPoints, PDLine pLines)
{
    pPtR->bIsSet = false;
    pPtR->cOrigin = 0;
    pPtR->cDirection = 0;

    PDPointList pLocCache = pCache;
    if(bNewPt)
    {
        pLocCache = new CDPointList();
        CDLine cPtX;
        cPtX.bIsSet = false;
        cPtX.cOrigin = cPt;
        double dOff;
        BuildEllipseCache(cPtX, 1, pPoints, pLocCache, pLines, &dOff);
    }

    int iCnt = pLocCache->GetCount(0);

    if(iCnt < 3)
    {
        if(bNewPt) delete pLocCache;
        return -1.0;
    }

    CDPoint cOrig = pLocCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pLocCache->GetPoint(1, 0).cPoint;
    CDPoint cMainDir = pLocCache->GetPoint(2, 0).cPoint;
    double dDist = 0.0;
    int nOffs = pLocCache->GetCount(2);
    if(nOffs > 0) dDist = pLocCache->GetPoint(0, 2).cPoint.x;

    if(bNewPt) delete pLocCache;

    CDPoint cDir, cPt1, cPt2, cPt3;

    cPt1 = Rotate(cPt - cOrig, cMainDir, false);

    CDPoint cProj = GetElpsPtProj(cRad.x, cRad.y, cPt1, cPt1, dDist, 0, NULL);
    if(cProj.x > 4.0) return -1.0;

    cDir.x = cRad.y*cProj.x;
    cDir.y = cRad.x*cProj.y;

    cPt3.x = cRad.x*cProj.x;
    cPt3.y = cRad.y*cProj.y;

    double da2 = Power2(cRad.x);
    double db2 = Power2(cRad.y);
    cPt2.x = (da2 - db2)*Power3(cProj.x)/cRad.x;
    cPt2.y = (db2 - da2)*Power3(cProj.y)/cRad.y;

    double dNorm = GetNorm(cDir);
    double dRad = GetDist(cPt3, cPt2);

    pPtR->bIsSet = true;
    pPtR->cOrigin = cOrig + Rotate(cPt2, cMainDir, true);
    if(dNorm > g_dPrec)
        pPtR->cDirection = Rotate(cDir/dNorm, cMainDir, true);

    return fabs(dRad + dDist);
}

bool GetElpsPointRefDist(double dRef, PDPointList pCache, double *pdDist)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 2) return false;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    double dDist = 0.0;

    if(iCnt < 3)
    {
        dDist = GetDist(cRad, cOrig);
        *pdDist = dRef*dDist;
        return true;
    }

    //CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    //CDPoint cPt1 = Rotate(cPt - cOrig, cMainDir, false);
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dDist = pCache->GetPoint(0, 2).cPoint.x;

    //CDPoint cProj = GetElpsPtProj(cRad.x, cRad.y, cPt1, cPt1);
    //if(cProj.x > 4.0) return false;
    double dAng = dRef; //atan2(cProj.y, cProj.x);
    double dDir = 1.0;
    if(dAng < 0.0)
    {
        dDir = -1.0;
        dAng *= dDir;
    }

    int iBreaks = pCache->GetCount(4);
    CDPoint cBreak = {-0.5, 0.0};
    if(iBreaks > 0) cBreak = pCache->GetPoint(0, 4).cPoint;

    double dStep = M_PI/8.0;
    int iSteps = 1 + dAng/dStep;
    dStep = dAng/iSteps;

    CDPrimitive cQuad;
    double dt = 0.0;
    CDPoint cDirStart;
    CDPoint cDirEnd = {0, 1.0};
    cQuad.cPt3.x = cRad.x + dDist;
    cQuad.cPt3.y = 0.0;

    double dRes = 0.0;
    double dco, dsi, dNorm;

    if((cBreak.x < g_dPrec) || (dAng < cBreak.x))
    {
        for(int i = 0; i < iSteps; i++)
        {
            dt += dStep;

            dco = cos(dt);
            dsi = sin(dt);

            cDirStart = cDirEnd;
            cDirEnd.x = -cRad.x*dsi;
            cDirEnd.y = cRad.y*dco;
            dNorm = GetNorm(cDirEnd);

            cQuad.cPt1 = cQuad.cPt3;
            cQuad.cPt3.x = cRad.x*dco + dDist*cDirEnd.y/dNorm;
            cQuad.cPt3.y = cRad.y*dsi - dDist*cDirEnd.x/dNorm;

            LineXLine(cQuad.cPt1, cDirStart, cQuad.cPt3, cDirEnd, &cQuad.cPt2);

            dRes += GetQuadLength(&cQuad, 0.0, 1.0);
        }

        *pdDist = dDir*dRes;
        return true;
    }

    int iSteps1 = 1 + cBreak.x/dStep;
    double dStep1 = cBreak.x/iSteps1;

    for(int i = 0; i < iSteps1; i++)
    {
        dt += dStep1;

        dco = cos(dt);
        dsi = sin(dt);

        cDirStart = cDirEnd;
        cDirEnd.x = -cRad.x*dsi;
        cDirEnd.y = cRad.y*dco;
        dNorm = GetNorm(cDirEnd);

        cQuad.cPt1 = cQuad.cPt3;
        cQuad.cPt3.x = cRad.x*dco + dDist*cDirEnd.y/dNorm;
        cQuad.cPt3.y = cRad.y*dsi - dDist*cDirEnd.x/dNorm;

        LineXLine(cQuad.cPt1, cDirStart, cQuad.cPt3, cDirEnd, &cQuad.cPt2);

        dRes += GetQuadLength(&cQuad, 0.0, 1.0);
    }

    if(dAng > M_PI - cBreak.x + g_dPrec)
    {
        iSteps1 = 1 + (M_PI - 2.0*cBreak.x)/dStep;
        dStep1 = (M_PI - 2.0*cBreak.x)/iSteps1;
    }
    else
    {
        iSteps1 = 1 + (dAng - cBreak.x)/dStep;
        dStep1 = (dAng - cBreak.x)/iSteps1;
    }

    for(int i = 0; i < iSteps1; i++)
    {
        dt += dStep1;

        dco = cos(dt);
        dsi = sin(dt);

        cDirStart = cDirEnd;
        cDirEnd.x = -cRad.x*dsi;
        cDirEnd.y = cRad.y*dco;
        dNorm = GetNorm(cDirEnd);

        cQuad.cPt1 = cQuad.cPt3;
        cQuad.cPt3.x = cRad.x*dco + dDist*cDirEnd.y/dNorm;
        cQuad.cPt3.y = cRad.y*dsi - dDist*cDirEnd.x/dNorm;

        LineXLine(cQuad.cPt1, cDirStart, cQuad.cPt3, cDirEnd, &cQuad.cPt2);

        dRes += GetQuadLength(&cQuad, 0.0, 1.0);
    }

    if(dAng > M_PI - cBreak.x + g_dPrec)
    {
        iSteps1 = 1 + (dAng - M_PI + cBreak.x)/dStep;
        dStep1 = (dAng - M_PI + cBreak.x)/iSteps1;

        for(int i = 0; i < iSteps1; i++)
        {
            dt += dStep1;

            dco = cos(dt);
            dsi = sin(dt);

            cDirStart = cDirEnd;
            cDirEnd.x = -cRad.x*dsi;
            cDirEnd.y = cRad.y*dco;
            dNorm = GetNorm(cDirEnd);

            cQuad.cPt1 = cQuad.cPt3;
            cQuad.cPt3.x = cRad.x*dco + dDist*cDirEnd.y/dNorm;
            cQuad.cPt3.y = cRad.y*dsi - dDist*cDirEnd.x/dNorm;

            LineXLine(cQuad.cPt1, cDirStart, cQuad.cPt3, cDirEnd, &cQuad.cPt2);

            dRes += GetQuadLength(&cQuad, 0.0, 1.0);
        }
    }

    *pdDist = dDir*dRes;
    return true;
}

double GetElpsPointAtDist(double da, double db, double dr, double dBreak, double dDist)
{
    double dStep = M_PI/8.0;
    double dDir = 1.0;
    if(dDist < 0.0)
    {
        dDir = -1.0;
        dDist *= dDir;
    }
    bool bFound = false;

    CDPrimitive cQuad;
    double dt = 0.0;
    CDPoint cDirStart;
    CDPoint cDirEnd = {0.0, 1.0};
    cQuad.cPt3.x = da + dr;
    cQuad.cPt3.y = 0.0;
    double d1, dco, dsi, dNorm;

    CDRefPoint cBnds[2];
    cBnds[0].bIsSet = true;
    cBnds[1].bIsSet = true;
    cBnds[0].dRef = 0.0;
    cBnds[1].dRef = 0.0;

    if(dBreak > -0.5)
    {
        int iSteps = (int)dBreak/dStep + 1;
        dStep = dBreak/iSteps;

        int i = 0;
        while(!bFound && (i < iSteps))
        {
            dt += dStep;

            dco = cos(dt);
            dsi = sin(dt);

            cDirStart = cDirEnd;
            cDirEnd.x = -da*dsi;
            cDirEnd.y = db*dco;
            dNorm = GetNorm(cDirEnd);

            cQuad.cPt1 = cQuad.cPt3;
            cQuad.cPt3.x = da*dco + dr*cDirEnd.y/dNorm;
            cQuad.cPt3.y = db*dsi - dr*cDirEnd.x/dNorm;

            LineXLine(cQuad.cPt1, cDirStart, cQuad.cPt3, cDirEnd, &cQuad.cPt2);

            d1 = GetQuadLength(&cQuad, 0.0, 1.0);
            if(d1 < dDist) dDist -= d1;
            else
            {
                bFound = true;
                cBnds[0].dRef = dt - dStep;
                cBnds[1].dRef = dt;
            }
            i++;
        }
        if(!bFound)
        {
            dStep = M_PI/8.0;
            iSteps = (int)(M_PI - 2.0*dBreak)/dStep + 1;
            dStep = (M_PI - 2.0*dBreak)/iSteps;

            i = 0;
            while(!bFound && (i < iSteps))
            {
                dt += dStep;

                dco = cos(dt);
                dsi = sin(dt);

                cDirStart = cDirEnd;
                cDirEnd.x = -da*dsi;
                cDirEnd.y = db*dco;
                dNorm = GetNorm(cDirEnd);

                cQuad.cPt1 = cQuad.cPt3;
                cQuad.cPt3.x = da*dco + dr*cDirEnd.y/dNorm;
                cQuad.cPt3.y = db*dsi - dr*cDirEnd.x/dNorm;

                LineXLine(cQuad.cPt1, cDirStart, cQuad.cPt3, cDirEnd, &cQuad.cPt2);

                d1 = GetQuadLength(&cQuad, 0.0, 1.0);
                if(d1 < dDist) dDist -= d1;
                else
                {
                    bFound = true;
                    cBnds[0].dRef = dt - dStep;
                    cBnds[1].dRef = dt;
                }
                i++;
            }
        }
        if(!bFound)
        {
            dStep = M_PI/8.0;
            iSteps = (int)dBreak/dStep + 1;
            dStep = dBreak/iSteps;

            i = 0;
            while(!bFound && (i < iSteps))
            {
                dt += dStep;

                dco = cos(dt);
                dsi = sin(dt);

                cDirStart = cDirEnd;
                cDirEnd.x = -da*dsi;
                cDirEnd.y = db*dco;
                dNorm = GetNorm(cDirEnd);

                cQuad.cPt1 = cQuad.cPt3;
                cQuad.cPt3.x = da*dco + dr*cDirEnd.y/dNorm;
                cQuad.cPt3.y = db*dsi - dr*cDirEnd.x/dNorm;

                LineXLine(cQuad.cPt1, cDirStart, cQuad.cPt3, cDirEnd, &cQuad.cPt2);

                d1 = GetQuadLength(&cQuad, 0.0, 1.0);
                if(d1 < dDist) dDist -= d1;
                else
                {
                    bFound = true;
                    cBnds[0].dRef = dt - dStep;
                    cBnds[1].dRef = dt;
                }
                i++;
            }
        }
    }
    else
    {
        while(!bFound)
        {
            dt += dStep;

            dco = cos(dt);
            dsi = sin(dt);

            cDirStart = cDirEnd;
            cDirEnd.x = -da*dsi;
            cDirEnd.y = db*dco;
            dNorm = GetNorm(cDirEnd);

            cQuad.cPt1 = cQuad.cPt3;
            cQuad.cPt3.x = da*dco + dr*cDirEnd.y/dNorm;
            cQuad.cPt3.y = db*dsi - dr*cDirEnd.x/dNorm;

            LineXLine(cQuad.cPt1, cDirStart, cQuad.cPt3, cDirEnd, &cQuad.cPt2);

            d1 = GetQuadLength(&cQuad, 0.0, 1.0);
            if(d1 < dDist) dDist -= d1;
            else
            {
                bFound = true;
                cBnds[0].dRef = dt - dStep;
                cBnds[1].dRef = dt;
            }
        }
    }

    double dt1 = GetQuadPointAtDist(&cQuad, 0.0, dDist);
    double dt2 = cBnds[1].dRef - cBnds[0].dRef;
    double dt0 = cBnds[0].dRef + dt1*dt2;
    double dr0 = dt2/10.0;
    if(dt0 + dr0 < cBnds[1].dRef) cBnds[1].dRef = dt0 + dr0;
    if(cBnds[0].dRef < dt0 - dr0) cBnds[0].dRef = dt0 - dr0;

    CDPoint cPt1 = GetQuadPoint(&cQuad, dt1);
    cPt1.y *= dDir;
    if(dDir < 0.0)
    {
        dt = -cBnds[0].dRef;
        cBnds[0].dRef = -cBnds[1].dRef;
        cBnds[1].dRef = dt;
    }

    CDPoint cProj = GetElpsPtProj(da, db, cPt1, cPt1, dr, 0, cBnds);
    return atan2(cProj.y, cProj.x);
}

void AddElpsSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 2) return;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    if(iCnt < 3)
    {
        CDPrimitive cPrim;
        CDPoint cPt1;
        CDPoint cDir = cRad - cOrig;
        double dNorm = GetNorm(cDir);
        if(dNorm < g_dPrec) return;

        cDir /= dNorm;
        cPt1.x = d1;
        cPt1.y = 0.0;

        cPrim.iType = 1;
        cPrim.cPt1 = cOrig + Rotate(cPt1, cDir, true);
        cPt1.x = d2;
        cPrim.cPt2 = cOrig + Rotate(cPt1, cDir, true);
        cPrim.cPt3 = 0;
        cPrim.cPt4 = 0;
        CropPrimitive(cPrim, pRect, pPrimList);
        return;
    }

    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    int iBreaks = pCache->GetCount(4);
    CDPoint cBreak = {-0.5, 0.0};
    CDPoint cLengths = {0.0, 0.0};
    if(iBreaks > 0)
    {
        cBreak = pCache->GetPoint(0, 4).cPoint;
        cLengths = pCache->GetPoint(1, 4).cPoint;
        if(d1 < -2.0*cLengths.x) d1 += 2.0*cLengths.x;
        if(d2 > cLengths.x) d2 -= 2.0*cLengths.x;
    }

    double dt1 = GetElpsPointAtDist(cRad.x, cRad.y, dr, cBreak.x, d1);
    double dt2 = GetElpsPointAtDist(cRad.x, cRad.y, dr, cBreak.x, d2);
    if(cLengths.y > g_dPrec)
    {
        dt1 *= M_PI/cLengths.y;
        dt2 *= M_PI/cLengths.y;
    }

    if(cBreak.x > -0.5)
        BuildEllipseWithBoundsBreaks(cRad.x, cRad.y, dr, dt1, dt2,
            cOrig, cMainDir, cBreak.x, false, pPrimList, pRect);
    else
        BuildEllipseWithBounds(cRad.x, cRad.y, dr, dt1, dt2, cOrig, cMainDir, pPrimList, pRect);
}

bool GetElpsRefPoint(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return false;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    double dco = cos(dRef);
    double dsi = sin(dRef);

    CDPoint cNorm;
    cNorm.x = cRad.y*dco;
    cNorm.y = cRad.x*dsi;
    double dN1 = GetNorm(cNorm);
    if(dN1 < g_dPrec) return false;

    CDPoint cPt1;
    cPt1.x = cRad.x*dco + dr*cNorm.x/dN1;
    cPt1.y = cRad.y*dsi + dr*cNorm.y/dN1;
    *pPt = cOrig + Rotate(cPt1, cMainDir, true);
    return true;
}

bool GetElpsRefDir(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;

    double dco = cos(dRef);
    double dsi = sin(dRef);

    CDPoint cNorm;
    cNorm.x = -cRad.x*dsi;
    cNorm.y = cRad.y*dco;
    double dN1 = GetNorm(cNorm);
    if(dN1 < g_dPrec) return false;

    CDPoint cPt1;
    cPt1.x = cNorm.x/dN1;
    cPt1.y = cNorm.y/dN1;
    *pPt = Rotate(cPt1, cMainDir, true);
    return true;
}

bool GetElpsReference(double dDist, PDPointList pCache, double *pdRef)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    //CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    int iBreaks = pCache->GetCount(4);
    CDPoint cBreak = {-0.5, 0.0};
    CDPoint cLengths = {0.0, 0.0};
    if(iBreaks > 0)
    {
        cBreak = pCache->GetPoint(0, 4).cPoint;
        cLengths = pCache->GetPoint(1, 4).cPoint;
    }

    double dt = GetElpsPointAtDist(cRad.x, cRad.y, dr, cBreak.x, dDist);
    if(cLengths.y > g_dPrec) dt *= M_PI/cLengths.y;

    *pdRef = dt;
    return true;
}

int GetElpsNumParts(PDPointList pCache, PDRefPoint pBounds)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return 0;

    CDPoint cBreak = {-0.5, 0.0};
    int iBreaks = pCache->GetCount(4);
    if(iBreaks > 0) cBreak = pCache->GetPoint(0, 4).cPoint;
    if(cBreak.x < -0.5) return 0;

    int iRes = 0;
    int iRef;
    double dAng = cBreak.x;

    if(dAng < g_dPrec)
    {
        if(!pBounds[0].bIsSet) return 1;
        if(!pBounds[1].bIsSet)
        {
            if(fabs(pBounds[0].dRef) < g_dPrec) return 1;
            if((fabs(pBounds[0].dRef - M_PI) < g_dPrec) || (fabs(pBounds[0].dRef + M_PI) < g_dPrec)) return 1;
            return 2;
        }

        if(RefInBounds(pBounds[0].dRef, pBounds[1].dRef, 0.0) > 2) iRes++;
        if((RefInBounds(pBounds[0].dRef, pBounds[1].dRef, M_PI) > 2) || (RefInBounds(pBounds[0].dRef, pBounds[1].dRef, -M_PI) > 2)) iRes++;
        return iRes;
    }

    if(dAng < M_PI/2.0 - g_dPrec)
    {
        if(!pBounds[0].bIsSet) return 3;
        if(!pBounds[1].bIsSet)
        {
            iRef = RefInBounds(-dAng, dAng, pBounds[0].dRef);
            if(iRef > 2) return 4;
            if(iRef > 0) return 3;
            iRef = RefInBounds(dAng, M_PI - dAng, pBounds[0].dRef);
            if(iRef > 2) return 4;
            if(iRef > 0) return 3;
            iRef = RefInBounds(M_PI - dAng, dAng - M_PI, pBounds[0].dRef);
            if(iRef > 2) return 4;
            if(iRef > 0) return 3;
            iRef = RefInBounds(dAng - M_PI, -dAng, pBounds[0].dRef);
            if(iRef > 2) return 4;
            if(iRef > 0) return 3;
            return 0;
        }

        if(RefInBounds(pBounds[0].dRef, pBounds[1].dRef, dAng) > 2) iRes++;
        if(RefInBounds(pBounds[0].dRef, pBounds[1].dRef, -dAng) > 2) iRes++;
        if(RefInBounds(pBounds[0].dRef, pBounds[1].dRef, M_PI - dAng) > 2) iRes++;
        if(RefInBounds(pBounds[0].dRef, pBounds[1].dRef, dAng - M_PI) > 2) iRes++;

        return iRes;
    }

    if(!pBounds[0].bIsSet) return 1;
    if(!pBounds[1].bIsSet)
    {
        if(fabs(pBounds[0].dRef - M_PI/2.0) < g_dPrec) return 1;
        if(fabs(pBounds[0].dRef + M_PI/2.0) < g_dPrec) return 1;
        return 2;
    }

    if(RefInBounds(pBounds[0].dRef, pBounds[1].dRef, -M_PI/2.0) > 2) iRes++;
    if(RefInBounds(pBounds[0].dRef, pBounds[1].dRef, M_PI/2.0) > 2) iRes++;
    return iRes;
}

bool ElpsRemovePart(bool bDown, PDPointList pCache, PDRefPoint pBounds)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return 0;

    CDPoint cBreak = {-0.5, 0.0};
    int iBreaks = pCache->GetCount(4);
    if(iBreaks > 0) cBreak = pCache->GetPoint(0, 4).cPoint;
    if(cBreak.x < -0.5) return 0;

    double dAng = cBreak.x;
    bool b1;

    if(dAng < g_dPrec)
    {
        b1 = !pBounds[0].bIsSet || (!pBounds[1].bIsSet && ((fabs(pBounds[0].dRef) < g_dPrec) ||
            (fabs(pBounds[0].dRef - M_PI) < g_dPrec) || (fabs(pBounds[0].dRef + M_PI) < g_dPrec)));
        if(b1)
        {
            if(bDown)
            {
                pBounds[0].bIsSet = true;
                pBounds[0].dRef = -M_PI;
                pBounds[1].bIsSet = true;
                pBounds[1].dRef = 0.0;
            }
            else
            {
                pBounds[0].bIsSet = true;
                pBounds[0].dRef = 0.0;
                pBounds[1].bIsSet = true;
                pBounds[1].dRef = M_PI;
            }
            return true;
        }

        double dTestVal = pBounds[0].dRef;
        if(pBounds[1].bIsSet) dTestVal = pBounds[1].dRef;

        int iEnd = -1;
        int iRef = RefInBounds(-M_PI, 0.0, dTestVal);
        if(iRef > 0) iEnd = 0;

        if(iEnd < 0)
        {
            iRef = RefInBounds(0.0, M_PI, dTestVal);
            if(iRef > 0) iEnd = 1;
        }
        if(iEnd < 0) return false;

        if(!pBounds[1].bIsSet)
        {
            pBounds[1].bIsSet = true;
            if(bDown) pBounds[1].dRef = pBounds[0].dRef;
        }

        double *pdVal = &pBounds[1].dRef;
        if(bDown) pdVal = &pBounds[0].dRef;
        
        switch(iEnd)
        {
        case 0:
            if(bDown) *pdVal = -M_PI;
            else *pdVal = M_PI;
            break;
        case 1:
            *pdVal = 0.0;
            break;
        }

        return true;
    }

    if(dAng < M_PI/2.0 - g_dPrec)
    {
        if(!pBounds[0].bIsSet)
        {
            if(bDown)
            {
                pBounds[0].bIsSet = true;
                pBounds[0].dRef = -dAng;
                pBounds[1].bIsSet = true;
                pBounds[1].dRef = dAng;
            }
            else
            {
                pBounds[0].bIsSet = true;
                pBounds[0].dRef = dAng;
                pBounds[1].bIsSet = true;
                pBounds[1].dRef = -dAng;
            }
            return true;
        }

        double dTestVal = pBounds[0].dRef;
        if(pBounds[1].bIsSet) dTestVal = pBounds[1].dRef;

        int iEnd = -1;
        int iRef = RefInBounds(-dAng, dAng, dTestVal);
        if(iRef > 1) iEnd = 3;
        else if(iRef > 0) iEnd = 2;

        if(iEnd < 0)
        {
            iRef = RefInBounds(dAng, M_PI - dAng, dTestVal);
            if(iRef > 1) iEnd = 0;
            else if(iRef > 0) iEnd = 3;
        }
        if(iEnd < 0)
        {
            iRef = RefInBounds(M_PI - dAng, dAng - M_PI, dTestVal);
            if(iRef > 1) iEnd = 1;
            else if(iRef > 0) iEnd = 0;
        }
        if(iEnd < 0)
        {
            iRef = RefInBounds(dAng - M_PI, -dAng, dTestVal);
            if(iRef > 1) iEnd = 2;
            else if(iRef > 0) iEnd = 1;
        }
        if(iEnd < 0) return false;

        if(!pBounds[1].bIsSet)
        {
            pBounds[1].bIsSet = true;
            if(bDown) pBounds[1].dRef = pBounds[0].dRef;
        }

        double *pdVal = &pBounds[1].dRef;
        if(bDown) pdVal = &pBounds[0].dRef;
        
        switch(iEnd)
        {
        case 0:
            *pdVal = dAng;
            break;
        case 1:
            *pdVal = M_PI - dAng;
            break;
        case 2:
            *pdVal = dAng - M_PI;
            break;
        case 3:
            *pdVal = -dAng;
            break;
        }

        return true;
    }

    b1 = !pBounds[0].bIsSet || (!pBounds[1].bIsSet &&
        ((fabs(pBounds[0].dRef - M_PI/2.0) < g_dPrec) || (fabs(pBounds[0].dRef + M_PI/2.0) < g_dPrec)));

    if(b1)
    {
        if(bDown)
        {
            pBounds[0].bIsSet = true;
            pBounds[0].dRef = M_PI/2.0;
            pBounds[1].bIsSet = true;
            pBounds[1].dRef = -M_PI/2.0;
        }
        else
        {
            pBounds[0].bIsSet = true;
            pBounds[0].dRef = -M_PI/2.0;
            pBounds[1].bIsSet = true;
            pBounds[1].dRef = M_PI/2.0;
        }
        return true;
    }

    double dTestVal = pBounds[0].dRef;
    if(pBounds[1].bIsSet) dTestVal = pBounds[1].dRef;

    int iEnd = -1;
    int iRef = RefInBounds(-M_PI/2.0, M_PI/2.0, dTestVal);
    if(iRef > 2) iEnd = 0;
    else if(iRef > 0) iEnd = 1;

    if(iEnd < 0)
    {
        iRef = RefInBounds(M_PI/2.0, -M_PI/2.0, dTestVal);
        if(iRef > 2) iEnd = 1;
        else if(iRef > 0) iEnd = 0;
    }
    if(iEnd < 0) return false;

    if(!pBounds[1].bIsSet)
    {
        pBounds[1].bIsSet = true;
        if(bDown) pBounds[1].dRef = pBounds[0].dRef;
    }

    double *pdVal = &pBounds[1].dRef;
    if(bDown) pdVal = &pBounds[0].dRef;
    
    switch(iEnd)
    {
    case 0:
        *pdVal = -M_PI/2.0;
        break;
    case 1:
        *pdVal = M_PI/2.0;
        break;
    }
    return true;
}

