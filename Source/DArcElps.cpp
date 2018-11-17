#include "DArcElps.hpp"
#include "DMath.hpp"
#include <math.h>
#include <stddef.h>
#include "DPrimitive.hpp"

#include <stdio.h>

// for debugging purpose only
/*#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
extern HWND g_hStatus;*/
// -----

bool AddArcElpsPoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines)
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

    if(iInputLines == 2)
    {
        if((iCtrl < 1) && (nNorm < 2))
        {
            pPoints->AddPoint(x, y, 0);
            nNorm++;
        }
        bRes = (nNorm > 1);
    }
    return bRes;
}

CDPoint DAEFunc(CDPoint cPt1, CDPoint cPt2, CDPoint cPtSol)
{
    CDPoint cN1 = {cPt1.x - cPtSol.x, cPt1.y};
    CDPoint cN2 = {cPt2.x, cPt2.y + cPtSol.y};
    double dN1 = GetNorm(cN1);
    double dN2 = GetNorm(cN2);
    double dSol2 = cPtSol*cPtSol;
    CDPoint cRes;
    cRes.x = sqrt(dSol2) + dN1 - dN2;
    cRes.y = dSol2 + cPtSol.x*dN1 - cPtSol.y*dN2;
    return cRes;
}

CDPoint DAEFuncX(CDPoint cPt1, CDPoint cPt2, CDPoint cPtSol)
{
    CDPoint cN1 = {cPt1.x - cPtSol.x, cPt1.y};
    //CDPoint cN2 = {cPt2.x, cPt2.y + cPtSol.y};
    double dN1 = GetNorm(cN1);
    //double dN2 = GetNorm(cN2);
    double dSol2 = cPtSol*cPtSol;
    CDPoint cRes;
    cRes.x = cPtSol.x/sqrt(dSol2) - (cPt1.x - cPtSol.x)/dN1;
    cRes.y = 2.0*cPtSol.x + dN1 - cPtSol.x*(cPt1.x - cPtSol.x)/dN1;
    return cRes;
}

CDPoint DAEFuncY(CDPoint cPt1, CDPoint cPt2, CDPoint cPtSol)
{
    //CDPoint cN1 = {cPt1.x - cPtSol.x, cPt1.y};
    CDPoint cN2 = {cPt2.x, cPt2.y + cPtSol.y};
    //double dN1 = GetNorm(cN1);
    double dN2 = GetNorm(cN2);
    double dSol2 = cPtSol*cPtSol;
    CDPoint cRes;
    cRes.x = cPtSol.y/sqrt(dSol2) - (cPt2.y + cPtSol.y)/dN2;
    cRes.y = 2.0*cPtSol.y - dN2 - cPtSol.y*(cPt2.y + cPtSol.y)/dN2;
    return cRes;
}

void UpdateOffsets(int iMode, int nOffs2, int nOffs3, CDLine cTmpPt, PDPointList pPoints,
    PDPointList pCache, double *pdMovedDist)
{
    if((iMode == 2) && (cTmpPt.cDirection.x > 0.5))
    {
        if(pdMovedDist) *pdMovedDist = cTmpPt.cDirection.y;
        pCache->AddPoint(cTmpPt.cDirection.y, 0.0, 2);
        return;
    }

    if((iMode == 2) || (nOffs2 > 0) || (nOffs3 > 0))
    {
        CDPoint cPt1;
        int iMask = 0;

        if(iMode == 2)
        {
            cPt1 = cTmpPt.cOrigin;
            if(cTmpPt.cDirection.x < -0.5) iMask = 2;
        }
        else if(nOffs2 > 0)
        {
            cPt1 = pPoints->GetPoint(0, 2).cPoint;
        }
        else if(nOffs3 > 0)
        {
            cPt1 = pPoints->GetPoint(0, 3).cPoint;
            iMask = 2;
        }

        CDLine cPtX;
        double dExt = 0.0;
        double dExtOld = 0.0;
        dExt = GetArcElpsDistFromPt(cPt1, cPt1, iMask, pCache, &cPtX, NULL);

        if(iMode == 2)
        {
            if(nOffs2 > 0)
            {
                cPt1 = pPoints->GetPoint(0, 2).cPoint;
                dExtOld = GetArcElpsDistFromPt(cPt1, cPt1, 0, pCache, &cPtX, NULL);
            }
            else if(nOffs3 > 0)
            {
                cPt1 = pPoints->GetPoint(0, 3).cPoint;
                dExtOld = GetArcElpsDistFromPt(cPt1, cPt1, 2, pCache, &cPtX, NULL);
            }
        }
        if(pdMovedDist) *pdMovedDist = dExt - dExtOld;
        pCache->AddPoint(dExt, dExtOld, 2);
    }
}

bool BuildArcElpsCache(CDLine cTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    PDLine pLines, double *pdMovedDist)
{
/*wchar_t buf[128];
swprintf(buf, L"%d %d", iNorm, iCtrl);
SendMessage(g_hStatus, SB_SETTEXT, 2, (LPARAM)buf);*/
    pCache->ClearAll();

    int nNorm = pPoints->GetCount(0);
    int nOffs2 = pPoints->GetCount(2);
    int nOffs3 = pPoints->GetCount(3);

    CDInputPoint cInPt1, cInPt2;
    CDPoint cOrig, cMainDir, cPt1, cPt2;
    CDPoint cSol, cInter;
    double d1, d2, da, db;
    double dr1 = -1.0, dr2 = -1.0;

    if(!(pLines[0].bIsSet && pLines[1].bIsSet)) return false;

    CDPoint cDir1 = pLines[0].cDirection;
    CDPoint cDir2 = pLines[1].cDirection;

    int iX = LineXLine(pLines[0].cOrigin, cDir1, pLines[1].cOrigin, cDir2, &cOrig);
    if(iX < 1) return false;

    bool bTwoPoints = false;

    if(((nNorm < 1) && (iMode == 1)) || ((nNorm == 1) && (iMode != 1)))
    {
        if(iMode == 1) cInPt1.cPoint = cTmpPt.cOrigin - cOrig;
        else if(nNorm > 0) cInPt1.cPoint = pPoints->GetPoint(0, 0).cPoint - cOrig;
        else return false;

        d1 = fabs(Deter2(cDir1, cDir2));
        if(d1 < g_dPrec) return false;

        cPt1 = cInPt1.cPoint;
        dr1 = sqrt(Power2(Deter2(cPt1, cDir1)) + Power2(Deter2(cPt1, cDir2)))/d1;
        dr2 = dr1;
    }
    else if(nNorm > 0)
    {
        cInPt1.cPoint = pPoints->GetPoint(0, 0).cPoint - cOrig;
        if(iMode == 1) cInPt2.cPoint = cTmpPt.cOrigin - cOrig;
        else if(nNorm > 1) cInPt2.cPoint = pPoints->GetPoint(1, 0).cPoint - cOrig;
        else return false;

        bTwoPoints = true;
        cPt1 = cInPt1.cPoint;
        cPt2 = cInPt2.cPoint;

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
    else return false;

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

    if(db < g_dPrec) return false;

    if(fabs(da - db) < g_dPrec)
    {
        pCache->AddPoint(cOrig.x, cOrig.y, 0);
        pCache->AddPoint(da, da, 0);
        if((iMode == 2) || (nOffs2 > 0) || (nOffs3 > 0))
        {
            double dExt;
            double dExtOld = 0.0;
            if(iMode == 2)
            {
                if(cTmpPt.cDirection.x > 0.5) dExt = cTmpPt.cDirection.y;
                else
                {
                    dExt = GetDist(cTmpPt.cOrigin, cOrig);
                    if(cTmpPt.cDirection.x < -0.5)
                    {
                        dExt += da;
                        dExt *= -1.0;
                    }
                    else dExt -= da;
                }
            }
            if(nOffs2 > 0)
            {
                cPt1 = pPoints->GetPoint(0, 2).cPoint;
                if(iMode == 2) dExtOld = GetDist(cPt1, cOrig) - da;
                else dExt = GetDist(cPt1, cOrig) - da;
            }
            else if(nOffs3 > 0)
            {
                cPt1 = pPoints->GetPoint(0, 3).cPoint;
                if(iMode == 2) dExtOld = -(GetDist(cPt1, cOrig) + da);
                else dExt = -(GetDist(cPt1, cOrig) + da);
            }
            if(pdMovedDist) *pdMovedDist = dExt - dExtOld;
            pCache->AddPoint(dExt, dExtOld, 2);
        }
        return true;
    }

    if(!bTwoPoints)
    {
        cPt1 = Abs(Rotate(cInPt1.cPoint, cMainDir, false));

        double deps = da/db;
        double du = 1 + Power2(deps);
        double dfact = du + (deps - 1)*sqrt(du);

        double dPoly[3];
        dPoly[0] = Power2(cPt1.x) + Power2(cPt1.y);
        dPoly[1] = cPt1.y*(dfact - 2);
        dPoly[2] = 1 - dfact;

        double dRoots[2];
        int iRoots = SolvePolynom(2, dPoly, dRoots);

        db = -1.0;
        if(iRoots > 0) db = dRoots[0];
        if((db < g_dPrec) && (iRoots > 1)) db = dRoots[1];

        CDLine cLn1, cLn2;
        cLn1.bIsSet = true;
        cLn2.bIsSet = true;

        if(db > g_dPrec)
        {
            da = deps*db;
            du = Power2(da) + Power2(db);
            dr1 = (du - (da - db)*sqrt(du))/da/2.0;
            dr2 = (du + (da - db)*sqrt(du))/db/2.0;

            cDir1.x = da - dr1;
            cDir1.y = dr2 - db;
            d1 = GetNorm(cDir1);
            cDir2 = cDir1/d1;
            cInter.x = cDir1.x + dr1*cDir2.x;
            cInter.y = dr1*cDir2.y;

            cLn1.cOrigin = cInter;
            cLn1.cDirection = cDir2;
            cLn2.cOrigin.x = 0;
            cLn2.cOrigin.y = db;
            cLn2.cDirection.x = 0;
            cLn2.cDirection.y = 1;
            if(PointInArc(cPt1, cLn1, cLn2))
            {
                pCache->AddPoint(cOrig.x, cOrig.y, 0);
                pCache->AddPoint(dr1, dr2, 0);
                pCache->AddPoint(cMainDir.x, cMainDir.y, 0);
                pCache->AddPoint(cDir1.x, cDir1.y, 0);
                pCache->AddPoint(cInter.x, cInter.y, 0);
                UpdateOffsets(iMode, nOffs2, nOffs3, cTmpPt, pPoints, pCache, pdMovedDist);
                return true;
            }
        }

        deps = db/da;
        du = 1 + Power2(deps);
        dfact = du - (1 - deps)*sqrt(du);

        dPoly[0] = Power2(cPt1.x) + Power2(cPt1.y);
        dPoly[1] = cPt1.x*(dfact - 2);
        dPoly[2] = 1 - dfact;

        iRoots = SolvePolynom(2, dPoly, dRoots);

        da = -1.0;
        if(iRoots > 0) da = dRoots[0];
        if((da < g_dPrec) && (iRoots > 1)) da = dRoots[1];

        if(da < g_dPrec) return false;

        db = deps*da;
        du = Power2(da) + Power2(db);
        dr1 = (du - (da - db)*sqrt(du))/da/2.0;
        dr2 = (du + (da - db)*sqrt(du))/db/2.0;

        cDir1.x = da - dr1;
        cDir1.y = dr2 - db;
        d1 = GetNorm(cDir1);
        cDir2 = cDir1/d1;
        cInter.x = cDir1.x + dr1*cDir2.x;
        cInter.y = dr1*cDir2.y;

        cLn1.cOrigin.x = da;
        cLn1.cOrigin.y = 0;
        cLn1.cDirection.x = 1;
        cLn1.cDirection.y = 0;
        cLn2.cOrigin = cInter;
        cLn2.cDirection = cDir2;
        if(PointInArc(cPt1, cLn1, cLn2))
        {
            pCache->AddPoint(cOrig.x, cOrig.y, 0);
            pCache->AddPoint(dr1, dr2, 0);
            pCache->AddPoint(cMainDir.x, cMainDir.y, 0);
            pCache->AddPoint(cDir1.x, cDir1.y, 0);
            pCache->AddPoint(cInter.x, cInter.y, 0);
            UpdateOffsets(iMode, nOffs2, nOffs3, cTmpPt, pPoints, pCache, pdMovedDist);
        }
        return true;
    }

    cPt1 = Rotate(cInPt1.cPoint, cMainDir, false);
    cPt2 = Rotate(cInPt2.cPoint, cMainDir, false);

    if(fabs(cPt2.y) < fabs(cPt1.y)) SwapPoints(&cPt1, &cPt2);

    d1 = GetNorm(cPt1);
    d2 = GetNorm(cPt2);

    if(d2 > d1)
    {
        dco = cMainDir.x;
        cMainDir.x = -cMainDir.y;
        cMainDir.y = dco;
        cPt1 = Rotate(cInPt1.cPoint, cMainDir, false);
        cPt2 = Rotate(cInPt2.cPoint, cMainDir, false);
        if(fabs(cPt2.y) < fabs(cPt1.y)) SwapPoints(&cPt1, &cPt2);
    }

    CDPoint cAxes;
    cAxes.x = fabs(cPt1.x);
    cAxes.y = fabs(cPt2.y);

    if((cAxes.x < g_dPrec) || (cAxes.y < g_dPrec)) return false;

    cPt1.x = cAxes.x;
    cPt2.y = cAxes.y;

    dr1 = Power2(db)/da;
    dr2 = Power2(da)/db;

    cSol.x = cAxes.x - dr1;
    cSol.y = dr2 - cAxes.y;

    CDPoint cFinalSol, cLowBound = {g_dPrec, g_dPrec};
    if(!Solve2ParamSystem(DAEFunc, DAEFuncX, DAEFuncY, cPt1, cPt2, cSol,
        &cFinalSol, &cLowBound)) return false;

    cSol = cFinalSol;

    CDPoint cMat1, cMat2;
    cMat1.x = cPt1.x - cSol.x;
    cMat1.y = cPt1.y;
    dr1 = GetNorm(cMat1);

    cMat2.x = cPt2.x;
    cMat2.y = cPt2.y + cSol.y;
    dr2 = GetNorm(cMat2);

    double dNormX = GetNorm(cSol);
    cDir2 = cSol/dNormX;
    cInter.x = cSol.x + dr1*cDir2.x;
    cInter.y = dr1*cDir2.y;

    pCache->AddPoint(cOrig.x, cOrig.y, 0);
    pCache->AddPoint(dr1, dr2, 0);
    pCache->AddPoint(cMainDir.x, cMainDir.y, 0);
    pCache->AddPoint(cSol.x, cSol.y, 0);
    pCache->AddPoint(cInter.x, cInter.y, 0);

    UpdateOffsets(iMode, nOffs2, nOffs3, cTmpPt, pPoints, pCache, pdMovedDist);
    return true;
}

CDPrimitive GetArcPrimitive(double dRad, CDPoint cCenter, double da1, double da2)
{
    CDPrimitive cRes;
    cRes.iType = 2;
    cRes.cPt1 = cCenter;

    double dr = fabs(dRad);
    cRes.cPt2.x = cCenter.x + dr;
    cRes.cPt2.y = cCenter.y + dr;

    cRes.cPt3.x = cCenter.x + dRad*cos(da2);
    cRes.cPt3.y = cCenter.y + dRad*sin(da2);
    cRes.cPt4.x = cCenter.x + dRad*cos(da1);
    cRes.cPt4.y = cCenter.y + dRad*sin(da1);
    return cRes;
}

CDPrimitive TransArcPrimitive(CDPoint cOrig, CDPoint cMainDir, CDPrimitive cPrim)
{
    double dRad = cPrim.cPt2.x - cPrim.cPt1.x;
    CDPrimitive cRes;
    cRes.iType = 2;
    cRes.cPt1 = cOrig + Rotate(cPrim.cPt1, cMainDir, true);
    cRes.cPt2.x = cRes.cPt1.x + dRad;
    cRes.cPt2.y = cRes.cPt1.y + dRad;
    cRes.cPt3 = cOrig + Rotate(cPrim.cPt3, cMainDir, true);
    cRes.cPt4 = cOrig + Rotate(cPrim.cPt4, cMainDir, true);
    return cRes;
}

int AddArcPrimitiveWithBounds(CDPoint cRad, CDPoint cOrig, CDPoint cMainDir, CDPoint cSol,
    PDRect pRect, PDPrimObject pPrimList, double d1, double d2, bool bFullCycle)
{
    double dAng = atan2(cSol.y, cSol.x);
    double db1 = M_PI - dAng;

    double dBnds[4];
    int iSegs = MergeBounds(d1, d2, db1, -db1, bFullCycle, dBnds);

    CDPoint cCenter = {-cSol.x, 0.0};
    int iRes = -1;
    int k;
    CDPrimitive cPrim;

    for(int i = 0; i < iSegs; i++)
    {
        cPrim = TransArcPrimitive(cOrig, cMainDir,
            GetArcPrimitive(cRad.x, cCenter, dBnds[2*i], dBnds[2*i + 1]));
        k = CropPrimitive(cPrim, pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    iSegs = MergeBounds(d1, d2, -db1, -dAng, bFullCycle, dBnds);
    cCenter.x = 0.0;
    cCenter.y = cSol.y;

    for(int i = 0; i < iSegs; i++)
    {
        cPrim = TransArcPrimitive(cOrig, cMainDir,
            GetArcPrimitive(cRad.y, cCenter, dBnds[2*i], dBnds[2*i + 1]));
        k = CropPrimitive(cPrim, pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    iSegs = MergeBounds(d1, d2, -dAng, dAng, bFullCycle, dBnds);
    cCenter.x = cSol.x;
    cCenter.y = 0.0;

    for(int i = 0; i < iSegs; i++)
    {
        cPrim = TransArcPrimitive(cOrig, cMainDir,
            GetArcPrimitive(cRad.x, cCenter, dBnds[2*i], dBnds[2*i + 1]));
        k = CropPrimitive(cPrim, pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    iSegs = MergeBounds(d1, d2, dAng, db1, bFullCycle, dBnds);
    cCenter.x = 0.0;
    cCenter.y = -cSol.y;

    for(int i = 0; i < iSegs; i++)
    {
        cPrim = TransArcPrimitive(cOrig, cMainDir,
            GetArcPrimitive(cRad.y, cCenter, dBnds[2*i], dBnds[2*i + 1]));
        k = CropPrimitive(cPrim, pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
    }

    if(iRes < 0) iRes = 0;
    return iRes;
}

int BuildArcElpsPrimitives(CDLine cTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdMovedDist, PDPoint pDrawBnds)
{
    /*int iInputLines = 0;
    if(pLines[0].bIsSet) iInputLines++;
    if(pLines[1].bIsSet) iInputLines++;*/

    if(iMode > 0) BuildArcElpsCache(cTmpPt, iMode, pPoints, pCache, pLines, pdMovedDist);

    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return 0;

    CDPrimitive cPrim, cPrimPt;
    CDPoint cOrig, cRad;

    cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;

    cRad.x += dOffset;
    cRad.y += dOffset;

    int nOffs = pCache->GetCount(2);
    if(nOffs > 0)
    {
        double dOff = pCache->GetPoint(0, 2).cPoint.x;
        cRad.x += dOff;
        cRad.y += dOff;
    }

    CDPoint cMainDir = {1.0, 0.0};
    double rx = fabs(cRad.x);

    if(iCnt < 3)
    {
        pDrawBnds->y = rx*M_PI;
        pDrawBnds->x = -pDrawBnds->y;

        cPrimPt.iType = 7;
        cPrimPt.cPt1.x = 1;
        cPrimPt.cPt1.y = 0;
        cPrimPt.cPt2 = cOrig;
        cPrimPt.cPt3 = 0;
        cPrimPt.cPt4 = 0;
        CropPrimitive(cPrimPt, pRect, pPrimList);

        if(pBounds[0].bIsSet && pBounds[1].bIsSet)
        {
            double dBnds[4];
            int iSegs = MergeBounds(-M_PI, M_PI, pBounds[0].dRef, pBounds[1].dRef, true, dBnds);

            CDPoint cCenter = {0.0, 0.0};
            int iRes = -1;
            int k;

            for(int i = 0; i < iSegs; i++)
            {
                cPrim = TransArcPrimitive(cOrig, cMainDir,
                    GetArcPrimitive(cRad.x, cCenter, dBnds[2*i], dBnds[2*i + 1]));
                k = CropPrimitive(cPrim, pRect, pPrimList);
                if(iRes < 0) iRes = k;
                else if(iRes != k) iRes = 1;
            }
            if(iRes < 0) iRes = 0;
            return iRes;
        }

        cPrim.iType = 3;
        cPrim.cPt1 = cOrig;
        cPrim.cPt2.x = cOrig.x + rx;
        cPrim.cPt2.y = cOrig.y + rx;
        return CropPrimitive(cPrim, pRect, pPrimList);
    }

    if(iCnt < 5) return 0;

    cMainDir = pCache->GetPoint(2, 0).cPoint;
    CDPoint cSol = pCache->GetPoint(3, 0).cPoint;
    //CDPoint cInter = pCache->GetPoint(4, 0).cPoint;

    double dAng = atan2(cSol.y, cSol.x);
    double ry = fabs(cRad.y);
    pDrawBnds->y = 2.0*dAng*(rx - ry) + M_PI*ry;
    pDrawBnds->x = -pDrawBnds->y;

    CDPoint cCenter;

    cCenter.x = cSol.x;
    cCenter.y = 0.0;

    cPrimPt.iType = 7;
    cPrimPt.cPt1.x = 1;
    cPrimPt.cPt1.y = 1;
    cPrimPt.cPt2 = cOrig + Rotate(cCenter, cMainDir, true);
    cCenter.x = -cSol.x;
    cPrimPt.cPt3 = cOrig + Rotate(cCenter, cMainDir, true);
    cPrimPt.cPt4 = 0;
    CropPrimitive(cPrimPt, pRect, pPrimList);

    cCenter.x = 0;
    cCenter.y = cSol.y;

    cPrimPt.iType = 7;
    cPrimPt.cPt1.x = 1;
    cPrimPt.cPt1.y = 1;
    cPrimPt.cPt2 = cOrig + Rotate(cCenter, cMainDir, true);
    cCenter.y = -cSol.y;
    cPrimPt.cPt3 = cOrig + Rotate(cCenter, cMainDir, true);
    cPrimPt.cPt4 = 0;
    CropPrimitive(cPrimPt, pRect, pPrimList);

    if(pBounds[0].bIsSet && pBounds[1].bIsSet)
        return AddArcPrimitiveWithBounds(cRad, cOrig, cMainDir, cSol, pRect, pPrimList,
            pBounds[0].dRef, pBounds[1].dRef, false);

    return AddArcPrimitiveWithBounds(cRad, cOrig, cMainDir, cSol, pRect, pPrimList,
        dAng - M_PI, dAng + M_PI, true);
}

int GetArcElpsProjCenter(CDPoint cSol, CDPoint cPt, char *piCenters)
{
    double dNorm = GetNorm(cSol);
    if(dNorm < g_dPrec) return 0;

    CDPoint cDir = cSol/dNorm;
    CDPoint cPos[4], cOrig;
    cOrig.x = cSol.x;
    cOrig.y = 0.0;
    cPos[0] = Rotate(cPt - cOrig, cDir, false);
    cDir.y *= -1.0;
    cPos[1] = Rotate(cPt - cOrig, cDir, false);
    cDir.x *= -1.0;
    cOrig.x *= -1.0;
    cPos[2] = Rotate(cPt - cOrig, cDir, false);
    cDir.y *= -1.0;
    cPos[3] = Rotate(cPt - cOrig, cDir, false);

    // 1
    if((cPos[0].y < -g_dPrec) && (cPos[1].y > g_dPrec))
    {
        piCenters[0] = 0;
        piCenters[1] = 2;
        return 2;
    }

    // 2
    if((cPos[2].y < -g_dPrec) && (cPos[3].y > g_dPrec))
    {
        piCenters[0] = 2;
        piCenters[1] = 0;
        return 2;
    }

    // 3
    if((cPos[1].y > g_dPrec) && (cPos[2].y < -g_dPrec))
    {
        piCenters[0] = 1;
        piCenters[1] = 3;
        return 2;
    }

    // 4
    if((cPos[0].y < -g_dPrec) && (cPos[3].y > g_dPrec))
    {
        piCenters[0] = 3;
        piCenters[1] = 1;
        return 2;
    }

    // 5
    if((cPos[0].y > g_dPrec) && (cPos[1].y > g_dPrec))
    {
        piCenters[0] = 1;
        piCenters[1] = 2;
        return 2;
    }

    // 6
    if((cPos[0].y < -g_dPrec) && (cPos[1].y < -g_dPrec))
    {
        piCenters[0] = 3;
        piCenters[1] = 2;
        return 2;
    }

    // 7
    if((cPos[2].y < -g_dPrec) && (cPos[3].y < -g_dPrec))
    {
        piCenters[0] = 1;
        piCenters[1] = 0;
        return 2;
    }

    // 8
    if((cPos[2].y > g_dPrec) && (cPos[3].y > g_dPrec))
    {
        piCenters[0] = 3;
        piCenters[1] = 0;
        return 2;
    }

    // 9
    if(cPt.y > g_dPrec)
    {
        piCenters[0] = 1;
        piCenters[1] = 3;
        piCenters[2] = 0;
        piCenters[3] = 2;
        return 4;
    }

    // 10
    piCenters[0] = 3;
    piCenters[1] = 1;
    piCenters[2] = 0;
    piCenters[3] = 2;
    return 4;
}

double GetCenterAndRad(int iCenter, CDPoint cRad, CDPoint cSol, PDPoint pCenter)
{
    if(iCenter == 0)
    {
        pCenter->x = cSol.x;
        pCenter->y = 0.0;
        return cRad.x;
    }

    if(iCenter == 1)
    {
        pCenter->x = 0.0;
        pCenter->y = -cSol.y;
        return cRad.y;
    }

    if(iCenter == 2)
    {
        pCenter->x = -cSol.x;
        pCenter->y = 0.0;
        return cRad.x;
    }

    pCenter->x = 0.0;
    pCenter->y = cSol.y;
    return cRad.y;
}

double GetArcElpsNorm(int iCenter, CDPoint cDir, PDPoint pNorm)
{
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec)
    {
        switch(iCenter)
        {
        case 0:
            pNorm->x = -1.0;
            pNorm->y = 0.0;
            break;
        case 1:
            pNorm->x = 0.0;
            pNorm->y = 1.0;
            break;
        case 2:
            pNorm->x = 1.0;
            pNorm->y = 0.0;
            break;
        case 3:
            pNorm->x = 0.0;
            pNorm->y = -1.0;
            break;
        }
    }
    else *pNorm = cDir/dNorm;
    return dNorm;
}

double GetArcElpsDistFromPt(CDPoint cPt, CDPoint cRefPt, int iSrchMask, PDPointList pCache, PDLine pPtX, PDRefPoint pBounds)
{
    pPtX->bIsSet = false;
    pPtX->dRef = 0.0;

    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return 0.0;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    CDPoint cDir, cClosePt, cCloseNorm;
    CDPoint cMainDir = {1.0, 0.0};
    CDPoint cPt1, cPt2, cPt3;

    int nOffs = pCache->GetCount(2);
    if(nOffs > 0)
    {
        double dOff = pCache->GetPoint(0, 2).cPoint.x;
        cRad.x += dOff;
        cRad.y += dOff;
    }

    double dRes, dMin, dNorm, d1;
    bool bIsNearCenter = true;
    bool bCenters = (iSrchMask & 1);
    bool bReverse = false;

    if(iCnt < 3)
    {
        bReverse = cRad.x < -g_dPrec;
        if(bReverse) cDir = cOrig - cPt;
        else cDir = cPt - cOrig;
        dNorm = GetNorm(cDir);
        if(dNorm > g_dPrec) cCloseNorm = cDir/dNorm;
        else
        {
            cCloseNorm.x = 1.0;
            cCloseNorm.y = 0.0;
        }
        cClosePt = cRad.x*cCloseNorm;
        dMin = GetDist(cClosePt, cRefPt - cOrig);
        dRes = dNorm - fabs(cRad.x);
        bIsNearCenter = false;

        if(bCenters)
        {
            d1 = GetDist(cOrig, cRefPt);
            if(d1 < dMin)
            {
                bIsNearCenter = true;
                dMin = d1;
                dRes = dNorm;
                cCloseNorm = 0;
                cClosePt = 0;
            }
        }
        if(bReverse) dRes *= -1.0;
    }
    else
    {
        if(iCnt < 5) return 0.0;

        cMainDir = pCache->GetPoint(2, 0).cPoint;
        CDPoint cSol = pCache->GetPoint(3, 0).cPoint;
        //CDPoint cInter = pCache->GetPoint(4, 0).cPoint;

        cPt1 = Rotate(cPt - cOrig, cMainDir, false);
        cPt2 = Rotate(cRefPt - cOrig, cMainDir, false);

        CDPoint cCenter, cNorm;
        double dRad, dNorm;

        char iCenters[4];
        int iCents = GetArcElpsProjCenter(cSol, cPt1, iCenters);
        if(iCents < 2) return 0.0;

        bool bInBounds[4];
        double dt;

        if(iSrchMask & 2)
        {
            dRad = GetCenterAndRad(iCenters[1], cRad, cSol, &cCenter);
            if(iCents > 2) cDir = cPt1 - cCenter;
            else cDir = cCenter - cPt1;
            dNorm = GetArcElpsNorm(iCenters[1], cDir, &cNorm);
            cPt3 = cCenter + dRad*cNorm;
            cCloseNorm = cNorm;
            cClosePt = cPt3;
            if(iCents > 2) dRes = dNorm - dRad;
            else dRes = -(dNorm + dRad);

            pPtX->dRef = atan2(cCloseNorm.y, cCloseNorm.x);

            cPt1 = cClosePt;
            cPt2 = cCloseNorm;
            cClosePt = cOrig + Rotate(cPt1, cMainDir, true);
            cCloseNorm = Rotate(cPt2, cMainDir, true);

            pPtX->bIsSet = true;
            pPtX->cOrigin = cClosePt;
            pPtX->cDirection = cCloseNorm;

            return dRes;
        }

        int iCentStart = 0;
        bool bFirstSet = false;

        if(bCenters)
        {
            bIsNearCenter = true;

            cCloseNorm = 0;
            GetCenterAndRad(0, cRad, cSol, &cCenter);
            cClosePt = cCenter;
            dMin = GetDist(cPt2, cClosePt);
            dRes = GetDist(cPt1, cClosePt);

            for(int i = 1; i < 4; i++)
            {
                GetCenterAndRad(i, cRad, cSol, &cCenter);
                d1 = GetDist(cPt2, cCenter);
                if(d1 < dMin)
                {
                    cClosePt = cCenter;
                    dMin = d1;
                    dRes = GetDist(cPt1, cClosePt);
                }
            }
        }
        else
        {
            bIsNearCenter = false;
            iCentStart = 1;

            dRad = GetCenterAndRad(iCenters[0], cRad, cSol, &cCenter);
            cDir = cPt1 - cCenter;
            dNorm = GetArcElpsNorm(iCenters[0], cDir, &cNorm);
            cPt3 = cCenter + dRad*cNorm;
            dMin = GetDist(cPt2, cPt3);
            cCloseNorm = cNorm;
            cClosePt = cPt3;
            dRes = dNorm - dRad;

            if(pBounds && pBounds[0].bIsSet && pBounds[1].bIsSet)
            {
                dt = atan2(cNorm.y, cNorm.x);
                bInBounds[0] = (RefInBounds(pBounds[0].dRef, pBounds[1].dRef, dt) > 0);
            }
            else bInBounds[0] = true;
            bFirstSet = bInBounds[0];
        }

        bool bIsBetter;

        for(int i = iCentStart; i < iCents; i++)
        {
            dRad = GetCenterAndRad(iCenters[i], cRad, cSol, &cCenter);
            bReverse = ((iCents < 3) && (i > 0)) || ((iCents > 2) && (i > 1));
            if(bReverse) cDir = cCenter - cPt1;
            else cDir = cPt1 - cCenter;
            dNorm = GetArcElpsNorm(iCenters[i], cDir, &cNorm);
            cPt3 = cCenter + dRad*cNorm;
            d1 = GetDist(cPt2, cPt3);

            if(pBounds && pBounds[0].bIsSet && pBounds[1].bIsSet)
            {
                dt = atan2(cNorm.y, cNorm.x);
                bInBounds[i] = (RefInBounds(pBounds[0].dRef, pBounds[1].dRef, dt) > 0);
            }
            else bInBounds[i] = true;

            bIsBetter = ((d1 < dMin - g_dPrec) && !(bFirstSet && !bInBounds[i])) || (!bFirstSet && bInBounds[i]);

            if(bIsBetter)
            {
                cCloseNorm = cNorm;
                cClosePt = cPt3;
                dMin = d1;
                bFirstSet = bInBounds[i];

                if(bReverse) dRes = -(dNorm + dRad);
                else dRes = dNorm - dRad;
                bIsNearCenter = false;
            }
        }
    }

    if(!bIsNearCenter)
        pPtX->dRef = atan2(cCloseNorm.y, cCloseNorm.x);

    cPt1 = cClosePt;
    cPt2 = cCloseNorm;
    cClosePt = cOrig + Rotate(cPt1, cMainDir, true);
    cCloseNorm = Rotate(cPt2, cMainDir, true);

    pPtX->bIsSet = true;
    pPtX->cOrigin = cClosePt;
    pPtX->cDirection = cCloseNorm;

    return dRes;
}

bool HasArcElpsEnoughPoints(PDPointList pPoints, int iInputLines)
{
    int nNorm = pPoints->GetCount(0);
    bool bRes = false;

    if(iInputLines == 2) bRes = (nNorm > 0);

    return bRes;
}

bool GetArcElpsRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
    PDPointList pCache)
{
    if(iMode != 2) return false;

    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return false;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    double dRad = dRestrictValue;

    int iOffs = pCache->GetCount(2);

    if(iOffs > 0)
    {
        CDPoint cOffs = pCache->GetPoint(0, 2).cPoint;
        cRad.x += cOffs.y;
        cRad.y += cOffs.y;
    }

    CDPoint cDir;
    double dNorm;

    if(iCnt < 3)
    {
        cDir = cPt - cOrig;
        dNorm = GetNorm(cDir);
        if(dNorm > g_dPrec)
        {
            cDir /= dNorm;
            *pSnapPt = cOrig + (cRad.x + dRad)*cDir;
        }
        else
        {
            pSnapPt->x = cOrig.x + cRad.x + dRad;
            pSnapPt->y = 0.0;
        }
        return true;
    }

    if(iCnt < 5) return false;

    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    CDPoint cSol = pCache->GetPoint(3, 0).cPoint;
    //CDPoint cInter = pCache->GetPoint(4, 0).cPoint;

    CDPoint cPt1 = Rotate(cPt - cOrig, cMainDir, false);
    char iCenters[4];
    int iCnts = GetArcElpsProjCenter(cSol, cPt1, iCenters);
    if(iCnts < 2) return false;

    CDPoint cCenter;
    double dr1 = GetCenterAndRad(iCenters[0], cRad, cSol, &cCenter);
    cDir = cPt1 - cCenter;
    dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return false;

    cDir /= dNorm;
    CDPoint cPt2 = cCenter + (dr1 + dRad)*cDir;
    *pSnapPt = cOrig + Rotate(cPt2, cMainDir, true);
    return true;
}

double GetArcElpsRadiusAtPt(CDPoint cPt, PDPointList pCache, PDLine pPtR, bool bNewPt,
    PDPointList pPoints, PDLine pLines)
{
    pPtR->bIsSet = false;
    pPtR->cOrigin = 0;
    pPtR->cDirection = 0;

    PDPointList pLocCache = pCache;
    CDLine cLn;
    cLn.bIsSet = false;
    cLn.cOrigin = cPt;
    cLn.cDirection.x = 0.0;
    if(bNewPt)
    {
        pLocCache = new CDPointList();
        BuildArcElpsCache(cLn, 1, pPoints, pLocCache, pLines, NULL);
    }

    int iCnt = pLocCache->GetCount(0);

    if(iCnt < 2) return -1.0;

    CDPoint cOrig = pLocCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pLocCache->GetPoint(1, 0).cPoint;

    int iOffs = pLocCache->GetCount(2);

    if(iOffs > 0)
    {
        CDPoint cOffs = pLocCache->GetPoint(0, 2).cPoint;
        cRad.x += cOffs.x;
        cRad.y += cOffs.x;
    }

    CDPoint cDir;
    CDPoint cMainDir = {1.0, 0.0};
    double dNorm, dRes = -1.0;

    if(iCnt < 3)
    {
        pPtR->bIsSet = true;
        pPtR->cOrigin = cOrig;
        cDir = cPt - cOrig;
        dNorm = GetNorm(cDir);
        if(dNorm > g_dPrec) pPtR->cDirection = cDir/dNorm;
        dRes = fabs(cRad.x);
    }
    else
    {
        if(iCnt < 5) return 0.0;

        cMainDir = pLocCache->GetPoint(2, 0).cPoint;
        CDPoint cSol = pLocCache->GetPoint(3, 0).cPoint;
        //CDPoint cInter = pLocCache->GetPoint(4, 0).cPoint;

        CDPoint cPt1 = Rotate(cPt - cOrig, cMainDir, false);
        char iCenters[4];
        int iCnts = GetArcElpsProjCenter(cSol, cPt1, iCenters);
        if(iCnts < 2) return dRes;

        CDPoint cCenter;
        dRes = fabs(GetCenterAndRad(iCenters[0], cRad, cSol, &cCenter));
        cDir = cPt1 - cCenter;
        dNorm = GetNorm(cDir);

        pPtR->bIsSet = true;
        pPtR->cOrigin = cOrig + Rotate(cCenter, cMainDir, true);
        if(dNorm > g_dPrec) pPtR->cDirection = Rotate(cDir/dNorm, cMainDir, true);
    }

    if(bNewPt) delete pLocCache;

    return dRes;
}

bool GetArcElpsPointRefDist(double dRef, PDPointList pCache, double *pdDist)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    int nOffs = pCache->GetCount(2);
    if(nOffs > 0)
    {
        double dOff = pCache->GetPoint(0, 2).cPoint.x;
        cRad.x += dOff;
        cRad.y += dOff;
    }

    double rx = fabs(cRad.x);

    if(iCnt < 3)
    {
        *pdDist = dRef*rx;
        return true;
    }

    if(iCnt < 5) return false;

    //CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    CDPoint cSol = pCache->GetPoint(3, 0).cPoint;
    //CDPoint cInter = pCache->GetPoint(4, 0).cPoint;

    double ry = fabs(cRad.y);

    double dAng = atan2(cSol.y, cSol.x);
    double dDir = 1.0;
    if(dRef < 0.0)
    {
        dDir = -1.0;
        dRef *= dDir;
    }

    if(dRef < dAng)
    {
        *pdDist = dDir*dRef*rx;
        return true;
    }

    double dRes = dAng*rx;

    if(dRef < M_PI - dAng)
    {
        *pdDist = dDir*(dRes + (dRef - dAng)*ry);
        return true;
    }

    dRes += (M_PI - 2*dAng)*ry;
    *pdDist = dDir*(dRes + (dRef + dAng - M_PI)*rx);
    return true;
}

double GetArcElpsRefAtDist(double dDist, CDPoint cRad, double dAng)
{
    if(fabs(dDist) < g_dPrec) return 0.0;

    double rx = fabs(cRad.x);
    double ry = fabs(cRad.y);

    double dTot = 2.0*dAng*rx + (M_PI - 2.0*dAng)*ry;
    double dOffs = 0.0;
    if(dDist > dTot)
    {
        dDist -= 2.0*dTot;
        dOffs = 2.0*M_PI;
    }
    else if(dDist < -dTot)
    {
        dDist += 2.0*dTot;
        dOffs = -2.0*M_PI;
    }

    double dDir = 1.0;
    if(dDist < 0)
    {
        dDir = -1.0;
        dDist *= dDir;
    }

    double d1 = rx*dAng;
    if(d1 > dDist - g_dPrec)
    {
        if(d1 > dDist + g_dPrec) return dDir*dDist/rx + dOffs;
        return dDir*dAng + dOffs;
    }

    dDist -= d1;

    d1 = (M_PI - 2.0*dAng)*ry;

    if(d1 > dDist - g_dPrec)
    {
        if(d1 > dDist + g_dPrec) return dDir*(dAng + dDist/ry) + dOffs;
        return dDir*(M_PI - dAng) + dOffs;
    }

    dDist -= d1;
    //d1 = rx*dAng;
    return dDir*(M_PI - dAng + dDist/rx) + dOffs;
}

void AddArcElpsSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    int nOffs = pCache->GetCount(2);
    if(nOffs > 0)
    {
        double dOff = pCache->GetPoint(0, 2).cPoint.x;
        cRad.x += dOff;
        cRad.y += dOff;
    }

    CDPrimitive cPrim;
    double dAng1, dAng2;

    if(iCnt < 3)
    {
        double dr = fabs(cRad.x);
        if(dr < g_dPrec) return;

        if(d1 > d2)
        {
            dAng1 = d1/dr;
            dAng2 = d2/dr;
        }
        else
        {
            dAng1 = d2/dr;
            dAng2 = d1/dr;
        }

        cPrim.iType = 2;
        cPrim.cPt1 = cOrig;
        cPrim.cPt2.x = cOrig.x + dr;
        cPrim.cPt2.y = cOrig.y + dr;
        cPrim.cPt3.x = cOrig.x + cRad.x*cos(dAng1);
        cPrim.cPt3.y = cOrig.y + cRad.x*sin(dAng1);
        cPrim.cPt4.x = cOrig.x + cRad.x*cos(dAng2);
        cPrim.cPt4.y = cOrig.y + cRad.x*sin(dAng2);
        CropPrimitive(cPrim, pRect, pPrimList);
        return;
    }

    if(iCnt < 5) return;

    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    CDPoint cSol = pCache->GetPoint(3, 0).cPoint;

    double dAng = atan2(cSol.y, cSol.x);
    double da1 = GetArcElpsRefAtDist(d1, cRad, dAng);
    double da2 = GetArcElpsRefAtDist(d2, cRad, dAng);
    AddArcPrimitiveWithBounds(cRad, cOrig, cMainDir, cSol, pRect, pPrimList, da1, da2, false);
}

bool GetArcElpsRefPoint(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return false;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cPt1;

    int nOffs = pCache->GetCount(2);
    if(nOffs > 0)
    {
        double dOff = pCache->GetPoint(0, 2).cPoint.x;
        cRad.x += dOff;
        cRad.y += dOff;
    }

    if(iCnt < 3)
    {
        cPt1.x = cos(dRef);
        cPt1.y = sin(dRef);
        *pPt = cOrig + cRad.x*cPt1;
        return true;
    }

    if(iCnt < 5) return false;

    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    CDPoint cSol = pCache->GetPoint(3, 0).cPoint;
    //CDPoint cInter = pCache->GetPoint(4, 0).cPoint;

    double dAng1 = atan2(cSol.y, cSol.x);
    double dAng2 = fabs(dRef);
    if(dAng2 < dAng1)
    {
        cPt1.x = cSol.x + cRad.x*cos(dRef);
        cPt1.y = cRad.x*sin(dRef);
    }
    else if(dAng2 < M_PI - dAng1)
    {
        cPt1.x = cRad.y*cos(dRef);
        if(dRef > 0)
            cPt1.y = -cSol.y + cRad.y*sin(dRef);
        else
            cPt1.y = cSol.y + cRad.y*sin(dRef);
    }
    else
    {
        cPt1.x = -cSol.x + cRad.x*cos(dRef);
        cPt1.y = cRad.x*sin(dRef);
    }

    *pPt = cOrig + Rotate(cPt1, cMainDir, true);
    return true;
}

bool GetArcElpsRefDir(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cPt1;

    cPt1.x = -sin(dRef);
    cPt1.y = cos(dRef);

    if(iCnt < 3)
    {
        if(cRad.x < -g_dPrec) *pPt = -1.0*cPt1;
        else *pPt = cPt1;
        return true;
    }

    if(iCnt < 5) return false;

    CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    //CDPoint cSol = pCache->GetPoint(3, 0).cPoint;
    //CDPoint cInter = pCache->GetPoint(4, 0).cPoint;

    *pPt = Rotate(cPt1, cMainDir, true);
    return true;
}

bool GetArcElpsReference(double dDist, PDPointList pCache, double *pdRef)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    int nOffs = pCache->GetCount(2);
    if(nOffs > 0)
    {
        double dOff = pCache->GetPoint(0, 2).cPoint.x;
        cRad.x += dOff;
        cRad.y += dOff;
    }

    if(iCnt < 3)
    {
        double dr = fabs(cRad.x);
        if(dr < g_dPrec) return false;
        *pdRef = dDist/dr;
        return true;
    }

    if(iCnt < 5) return false;

    //CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    CDPoint cSol = pCache->GetPoint(3, 0).cPoint;
    //CDPoint cInter = pCache->GetPoint(4, 0).cPoint;

    double dAng = atan2(cSol.y, cSol.x);
    *pdRef = GetArcElpsRefAtDist(dDist, cRad, dAng);
    return true;
}

int GetArcElpsNumParts(PDPointList pCache, PDRefPoint pBounds)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 5) return 0;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    int nOffs = pCache->GetCount(2);
    if(nOffs > 0)
    {
        double dOff = pCache->GetPoint(0, 2).cPoint.x;
        cRad.x += dOff;
        cRad.y += dOff;
    }

    if((cRad.x > g_dPrec) && (cRad.y > g_dPrec)) return 0;
    if((cRad.x < -g_dPrec) && (cRad.y < -g_dPrec)) return 0;

    //CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    CDPoint cSol = pCache->GetPoint(3, 0).cPoint;
    //CDPoint cInter = pCache->GetPoint(4, 0).cPoint;
    double dAng = atan2(cSol.y, cSol.x);
    int iRes = 0;
    int iRef;

    if(cRad.y < g_dPrec)
    {
        if(!pBounds[0].bIsSet) return 1;
        if(!pBounds[1].bIsSet)
        {
            iRef = RefInBounds(-dAng, dAng, pBounds[0].dRef);
            if(iRef > 2) return 2;
            if(iRef > 0) return 1;
            iRef = RefInBounds(M_PI - dAng, dAng - M_PI, pBounds[0].dRef);
            if(iRef > 2) return 2;
            if(iRef > 0) return 1;
            return 0;
        }

        if((RefInBounds(pBounds[0].dRef, pBounds[1].dRef, dAng) > 2) || (RefInBounds(pBounds[0].dRef, pBounds[1].dRef, M_PI - dAng) > 2)) iRes++;
        if((RefInBounds(pBounds[0].dRef, pBounds[1].dRef, dAng - M_PI) > 2) || (RefInBounds(pBounds[0].dRef, pBounds[1].dRef, -dAng) > 2)) iRes++;

        return iRes;
    }

    if(cRad.x < -g_dPrec)
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

    if(cRad.x < g_dPrec)
    {
        if(!pBounds[0].bIsSet) return 1;
        if(!pBounds[1].bIsSet)
        {
            iRef = RefInBounds(dAng, M_PI - dAng, pBounds[0].dRef);
            if(iRef > 2) return 2;
            if(iRef > 0) return 1;
            iRef = RefInBounds(dAng - M_PI, -dAng, pBounds[0].dRef);
            if(iRef > 2) return 2;
            if(iRef > 0) return 1;
            return 0;
        }

        if((RefInBounds(pBounds[0].dRef, pBounds[1].dRef, dAng) > 2) || (RefInBounds(pBounds[0].dRef, pBounds[1].dRef, -dAng) > 2)) iRes++;
        if((RefInBounds(pBounds[0].dRef, pBounds[1].dRef, dAng - M_PI) > 2) || (RefInBounds(pBounds[0].dRef, pBounds[1].dRef, M_PI - dAng) > 2)) iRes++;

        return iRes;
    }

    return 0;
}

bool ArcElpsRemovePart(bool bDown, PDPointList pCache, PDRefPoint pBounds)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 5) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;

    int nOffs = pCache->GetCount(2);
    if(nOffs > 0)
    {
        double dOff = pCache->GetPoint(0, 2).cPoint.x;
        cRad.x += dOff;
        cRad.y += dOff;
    }

    //CDPoint cMainDir = pCache->GetPoint(2, 0).cPoint;
    CDPoint cSol = pCache->GetPoint(3, 0).cPoint;
    //CDPoint cInter = pCache->GetPoint(4, 0).cPoint;
    double dAng = atan2(cSol.y, cSol.x);

    if(cRad.y < g_dPrec)
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
                pBounds[0].dRef = M_PI - dAng;
                pBounds[1].bIsSet = true;
                pBounds[1].dRef = dAng - M_PI;
            }
            return true;
        }

        double dTestVal = pBounds[0].dRef;
        if(pBounds[1].bIsSet) dTestVal = pBounds[1].dRef;

        int iEnd = -1;
        int iRef = RefInBounds(-dAng, dAng, dTestVal);
        if(iRef > 0) iEnd = 2;

        if(iEnd < 0)
        {
            iRef = RefInBounds(dAng, M_PI - dAng, dTestVal);
            if(iRef > 0) iEnd = 2;
        }
        if(iEnd < 0)
        {
            iRef = RefInBounds(M_PI - dAng, dAng - M_PI, dTestVal);
            if(iRef > 0) iEnd = 0;
        }
        if(iEnd < 0)
        {
            iRef = RefInBounds(dAng - M_PI, -dAng, dTestVal);
            if(iRef > 0) iEnd = 0;
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
        case 2:
            *pdVal = dAng - M_PI;
            break;
        }

        if((RefInBounds(dAng, M_PI - dAng, pBounds[0].dRef) > 0) && (RefInBounds(dAng, M_PI - dAng, pBounds[1].dRef) > 0)) return false;
        if((RefInBounds(dAng - M_PI, -dAng, pBounds[0].dRef) > 0) && (RefInBounds(dAng - M_PI, -dAng, pBounds[1].dRef) > 0)) return false;

        return true;
    }

    if(cRad.x < -g_dPrec)
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

    if(cRad.x < g_dPrec)
    {
        if(!pBounds[0].bIsSet)
        {
            if(bDown)
            {
                pBounds[0].bIsSet = true;
                pBounds[0].dRef = dAng - M_PI;
                pBounds[1].bIsSet = true;
                pBounds[1].dRef = -dAng;
            }
            else
            {
                pBounds[0].bIsSet = true;
                pBounds[0].dRef = dAng;
                pBounds[1].bIsSet = true;
                pBounds[1].dRef = M_PI - dAng;
            }
            return true;
        }

        double dTestVal = pBounds[0].dRef;
        if(pBounds[1].bIsSet) dTestVal = pBounds[1].dRef;

        int iEnd = -1;
        int iRef = RefInBounds(-dAng, dAng, dTestVal);
        if(iRef > 0) iEnd = 3;

        if(iEnd < 0)
        {
            iRef = RefInBounds(dAng, M_PI - dAng, dTestVal);
            if(iRef > 0) iEnd = 3;
        }
        if(iEnd < 0)
        {
            iRef = RefInBounds(M_PI - dAng, dAng - M_PI, dTestVal);
            if(iRef > 0) iEnd = 1;
        }
        if(iEnd < 0)
        {
            iRef = RefInBounds(dAng - M_PI, -dAng, dTestVal);
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
        case 1:
            *pdVal = M_PI - dAng;
            break;
        case 3:
            *pdVal = -dAng;
            break;
        }

        if((RefInBounds(-dAng, dAng, pBounds[0].dRef) > 0) && (RefInBounds(-dAng, dAng, pBounds[1].dRef) > 0)) return false;
        if((RefInBounds(M_PI - dAng, dAng - M_PI, pBounds[0].dRef) > 0) && (RefInBounds(M_PI - dAng, dAng - M_PI, pBounds[1].dRef) > 0)) return false;

        return true;
    }

    return false;
}

