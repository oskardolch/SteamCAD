#include "DParabola.hpp"
#include "DMath.hpp"
#include <math.h>
#include <stdio.h>
#include "DPrimitive.hpp"

// for debugging purpose only
/*#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
extern HWND g_hStatus;
extern HANDLE g_hConsole;*/
// -----

bool AddParabPoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines)
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

    if(iInputLines == 1)
    {
        if((iCtrl < 1) && (nNorm < 1))
        {
            pPoints->AddPoint(x, y, iCtrl);
            nNorm++;
        }
        bRes = (nNorm > 0);
    }
    return bRes;
}

double GetParabBreakAngle(double dr, double da, double dr1)
{
    double dRes = -1.0;
    if(dr > dr1 - g_dPrec)
    {
        if(dr < dr1 + g_dPrec) dRes = 0.0;
        else
        {
            double d1 = 2.0*da;
            double d2 = cbrt(Power2(d1*dr));
            dRes = sqrt(d2 - 1.0)/d1;
        }
    }
    return dRes;
}

bool BuildParabCache(CDLine cTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    PDLine pLines, double *pdDist)
{
    pCache->ClearAll();

    int nNorm = pPoints->GetCount(0);

    CDInputPoint cInPt1;
    CDPoint cOrig, cMainDir, cPt1, cPt2, cPt3;
    double da;
    double dr1 = -1.0;

    if(!pLines[0].bIsSet) return false;

    if(!(((nNorm < 1) && (iMode == 1)) || ((nNorm == 1) && (iMode != 1)))) return false;

    if(iMode == 1) cPt1 = cTmpPt.cOrigin;
    else if(nNorm > 0)
    {
        cInPt1 = pPoints->GetPoint(0, 0);
        cPt1 = cInPt1.cPoint;
    }
    else return false;

    cMainDir = pLines[0].cDirection;
    dr1 = GetLineProj(cPt1, pLines[0].cOrigin, cMainDir, &cPt2)/2.0;
    cPt3 = Rotate(cPt1 - cPt2, cMainDir, false);
    if(cPt3.y < 0)
    {
        cMainDir = -1.0*pLines[0].cDirection;
        cPt3 = Rotate(cPt1 - cPt2, cMainDir, false);
    }

    cOrig = (cPt1 + cPt2)/2.0;
    da = 1.0/dr1/4.0;

    pCache->AddPoint(cOrig.x, cOrig.y, 0);
    pCache->AddPoint(da, 0.0, 0);
    pCache->AddPoint(cMainDir.x, cMainDir.y, 0);

    dr1 = 0.5/da;
    pCache->AddPoint(0.0, dr1, 3);
    double dr;

    if((iMode == 2) && (cTmpPt.cDirection.x > 0.5))
    {
        dr = GetParabBreakAngle(-cTmpPt.cDirection.y, da, dr1);
        if(dr > -0.5) pCache->AddPoint(dr, 0.0, 4);

        if(pdDist) *pdDist = cTmpPt.cDirection.y;
        pCache->AddPoint(cTmpPt.cDirection.y, 0.0, 2);
        return true;
    }

    int nOffs2 = pPoints->GetCount(2);
    int nOffs3 = pPoints->GetCount(3);
    if((iMode == 2) || (nOffs2 > 0) || (nOffs3 > 0))
    {
        int iSrchMask = 0;

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
        double dDist = GetParabDistFromPt(cPt1, cPt1, iSrchMask, pCache, &cPtX, NULL);
        double dDistOld = 0.0;

        dr = GetParabBreakAngle(-dDist, da, dr1);
        if(dr > -0.5) pCache->AddPoint(dr, 0.0, 4);

        if(iMode == 2)
        {
            if(nOffs2 > 0)
            {
                cPt1 = pPoints->GetPoint(0, 2).cPoint;
                dDistOld = GetParabDistFromPt(cPt1, cPt1, 0, pCache, &cPtX, NULL);
            }
            else if(nOffs3 > 0)
            {
                cPt1 = pPoints->GetPoint(0, 3).cPoint;
                dDistOld = GetParabDistFromPt(cPt1, cPt1, 2, pCache, &cPtX, NULL);
            }
        }

        *pdDist = dDist - dDistOld;
        if(fabs(dDist) > g_dPrec) pCache->AddPoint(dDist, dDistOld, 2);
    }
    return true;
}

CDPoint GetParabPointDir(double da, double dOffset, double dx, PDPoint pDir)
{
    CDPoint cRes, cDir;
    cRes.x = dx;
    cRes.y = da*Power2(dx);
    cDir.x = 2.0*da*dx;
    cDir.y = -1.0;
    double dNorm = GetNorm(cDir);
    cDir /= dNorm;
    pDir->x = -cDir.y;
    pDir->y = cDir.x;
    return cRes + dOffset*cDir;
}

double GetParabXLen(double da, double dr, double dBreak, double dx)
{
    double dDir = 1.0;
    if(dx < 0)
    {
        dDir = -1.0;
        dx *= dDir;
    }

    CDPrimitive cQuad;

    if(fabs(dr) < g_dPrec)
    {
        cQuad.cPt1.x = 0;
        cQuad.cPt1.y = 0;
        cQuad.cPt2.x = dx/2.0;
        cQuad.cPt2.y = 0.0;
        cQuad.cPt3.x = dx;
        cQuad.cPt3.y = da*Power2(dx);
        return dDir*GetQuadLength(&cQuad, 0.0, 1.0);
    }

    cQuad.cPt3.x = 0.0;
    cQuad.cPt3.y = -dr;

    CDPoint cDir1, cDir2;
    cDir2.x = 1.0;
    cDir2.y = 0.0;

    double dRes = 0.0;
    double dStart = 0.0;

    if((dBreak > g_dPrec) && (dx > dBreak + g_dPrec))
    {
        int iSteps = (int)dBreak + 1;
        for(int i = 0; i < iSteps; i++)
        {
            cDir1 = cDir2;
            cQuad.cPt1 = cQuad.cPt3;

            dStart = (double)(i + 1.0)*dBreak/iSteps;
            cQuad.cPt3 = GetParabPointDir(da, dr, dStart, &cDir2);
            LineXLine(cQuad.cPt1, cDir1, cQuad.cPt3, cDir2, &cQuad.cPt2);
            dRes += GetQuadLength(&cQuad, 0.0, 1.0);
        }
    }

    double dlny = log(2.0*(dx - dStart));
    int iSegs = (int)dlny/log(2.0) + 1;
    if(iSegs < 1) iSegs = 1;
    double dBase = (dx - dStart)/PowerN(iSegs - 1, 2.0);

    for(int i = 0; i < iSegs; i++)
    {
        cDir1 = cDir2;
        cQuad.cPt1 = cQuad.cPt3;

        cQuad.cPt3 = GetParabPointDir(da, dr, dStart + dBase, &cDir2);
        LineXLine(cQuad.cPt1, cDir1, cQuad.cPt3, cDir2, &cQuad.cPt2);
        dRes += GetQuadLength(&cQuad, 0.0, 1.0);

        dBase *= 2.0;
    }

    return dDir*dRes;
}

CDPoint ParabFunc(double da, double db, double dt)
{
    CDPoint cRes;
    cRes.x = dt;
    cRes.y = da*Power2(dt);
    return cRes;
}

CDPoint ParabFuncDer(double da, double db, double dt)
{
    CDPoint cRes;
    cRes.x = 1.0;
    cRes.y = 2.0*da*dt;
    return cRes;
}

int AddParabSegWithBounds(double da, double dr, CDPoint cOrig, CDPoint cMainDir,
    double dx1, double dx2, double dBreak, PDPrimObject pPrimList, PDRect pRect)
{
    CDPrimitive cPrim;
    CDPoint cPt1, cPt2, cPt3;
    if(fabs(dr) < g_dPrec)
    {
        cPt1.x = dx1;
        cPt1.y = da*Power2(dx1);
        cPt2.x = (dx1 + dx2)/2.0;
        cPt2.y = da*dx1*dx2;
        cPt3.x = dx2;
        cPt3.y = da*Power2(dx2);

        /*cPrim.iType = 5;
        cPrim.cPt1 = cOrig + Rotate(cPt1, cMainDir, true);
        cPrim.cPt2 = cOrig + Rotate(cPt1/3.0 + 2.0*cPt2/3.0, cMainDir, true);
        cPrim.cPt3 = cOrig + Rotate(cPt3/3.0 + 2.0*cPt2/3.0, cMainDir, true);
        cPrim.cPt4 = cOrig + Rotate(cPt3, cMainDir, true);*/
        cPrim.iType = 4;
        cPrim.cPt1 = cOrig + Rotate(cPt1, cMainDir, true);
        cPrim.cPt2 = cOrig + Rotate(cPt2, cMainDir, true);
        cPrim.cPt3 = cOrig + Rotate(cPt3, cMainDir, true);
        return CropPrimitive(cPrim, pRect, pPrimList);
    }

    if(dBreak < -0.5)
        return AddBoundCurve(da, 1.0, dr, ParabFunc, ParabFuncDer, dx1, dx2, cOrig, cMainDir,
            pRect, pPrimList);

    int iRes = -1;
    int k;
    if(RefInBounds(dx1, dx2, -dBreak) > 2)
    {
        k = AddBoundCurve(da, 1.0, dr, ParabFunc, ParabFuncDer, dx1, -dBreak, cOrig, cMainDir,
            pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
        dx1 = -dBreak;
    }
    if(RefInBounds(dx1, dx2, dBreak) > 2)
    {
        k = AddBoundCurve(da, 1.0, dr, ParabFunc, ParabFuncDer, dx1, dBreak, cOrig, cMainDir,
            pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
        dx1 = dBreak;
    }
    k = AddBoundCurve(da, 1.0, dr, ParabFunc, ParabFuncDer, dx1, dx2, cOrig, cMainDir,
        pRect, pPrimList);
    if(iRes < 0) iRes = k;
    else if(iRes != k) iRes = 1;

    return iRes;
}

int AddParabSegQuadsWithBounds(double da, double dr, CDPoint cOrig, CDPoint cMainDir,
    double dx1, double dx2, double dBreak, PDPrimObject pPrimList, PDRect pRect)
{
    CDPrimitive cPrim;
    CDPoint cPt1, cPt2, cPt3;
    if(fabs(dr) < g_dPrec)
    {
        cPt1.x = dx1;
        cPt1.y = da*Power2(dx1);
        cPt2.x = (dx1 + dx2)/2.0;
        cPt2.y = da*dx1*dx2;
        cPt3.x = dx2;
        cPt3.y = da*Power2(dx2);

        /*cPrim.iType = 5;
        cPrim.cPt1 = cOrig + Rotate(cPt1, cMainDir, true);
        cPrim.cPt2 = cOrig + Rotate(cPt1/3.0 + 2.0*cPt2/3.0, cMainDir, true);
        cPrim.cPt3 = cOrig + Rotate(cPt3/3.0 + 2.0*cPt2/3.0, cMainDir, true);
        cPrim.cPt4 = cOrig + Rotate(cPt3, cMainDir, true);*/
        cPrim.iType = 4;
        cPrim.cPt1 = cOrig + Rotate(cPt1, cMainDir, true);
        cPrim.cPt2 = cOrig + Rotate(cPt2, cMainDir, true);
        cPrim.cPt3 = cOrig + Rotate(cPt3, cMainDir, true);
        return CropPrimitive(cPrim, pRect, pPrimList);
    }

    if(dBreak < -0.5)
        return AddBoundQuadCurve(da, 1.0, dr, ParabFunc, ParabFuncDer, dx1, dx2, cOrig, cMainDir,
            pRect, pPrimList);

    int iRes = -1;
    int k;
    if(RefInBounds(dx1, dx2, -dBreak) > 2)
    {
        k = AddBoundQuadCurve(da, 1.0, dr, ParabFunc, ParabFuncDer, dx1, -dBreak, cOrig, cMainDir,
            pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
        dx1 = -dBreak;
    }
    if(RefInBounds(dx1, dx2, dBreak) > 2)
    {
        k = AddBoundQuadCurve(da, 1.0, dr, ParabFunc, ParabFuncDer, dx1, dBreak, cOrig, cMainDir,
            pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
        dx1 = dBreak;
    }
    k = AddBoundQuadCurve(da, 1.0, dr, ParabFunc, ParabFuncDer, dx1, dx2, cOrig, cMainDir,
        pRect, pPrimList);
    if(iRes < 0) iRes = k;
    else if(iRes != k) iRes = 1;

    return iRes;
}

double ParProjFn(double da, double dx, double dy, double du)
{
    return 2.0*Power2(da)*Power3(du) + (1.0 - 2.0*da*dy)*du - dx;
}

double ParProjFnDer(double da, double dx, double dy, double du)
{
    return 6.0*Power2(da*du) + (1.0 - 2.0*da*dy);
}

double GetParabPtProjFromU(double da, double dStart, CDPoint cPt)
{
    int j = 0;
    double du1 = dStart;
    double df = ParProjFn(da, cPt.x, cPt.y, du1);
    double df2 = ParProjFnDer(da, cPt.x, cPt.y, du1);
    if(fabs(df2) < g_dPrec) return 0.0;

    double du2 = du1 - df/df2;

    while((j < 8) && (fabs(df) > g_dRootPrec))
    {
        j++;
        du1 = du2;
        df = ParProjFn(da, cPt.x, cPt.y, du1);
        df2 = ParProjFnDer(da, cPt.x, cPt.y, du1);
        if(fabs(df2) > g_dPrec) du2 -= df/df2;
        else j = 8;
    }
    return du2;
}

double GetParabPtProj(double da, CDPoint cPt, CDPoint cRefPt, double dOffset, int iSrchMask, PDRefPoint pBounds)
{
    double dPoly[4];
    dPoly[0] = -cPt.x;
    dPoly[1] = 1.0 - 2.0*da*cPt.y;
    dPoly[2] = 0.0;
    dPoly[3] = 2.0*Power2(da);

    double dRoots[3];
    int iRoots = SolvePolynom(3, dPoly, dRoots);

    if(iRoots < 1)
    {
        return GetParabPtProjFromU(da, 0.0, cPt);
    }

    for(int j = 0; j < iRoots; j++)
    {
        dRoots[j] = GetParabPtProjFromU(da, dRoots[j], cPt);
    }

    bool bInBounds[3];
    if(pBounds)
    {
        for(int j = 0; j < iRoots; j++)
        {
            bInBounds[j] = (RefInOpenBounds(pBounds, dRoots[j]) > 0);
        }
    }
    else
    {
        for(int j = 0; j < iRoots; j++) bInBounds[j] = true;
    }

    CDPoint cPt1, cPt2;
    cPt1.x = dRoots[0];
    cPt1.y = da*Power2(cPt1.x);
    double dXMin = cPt1.x;
    double dXSec = cPt1.x;
    bool bFirstSet = bInBounds[0];
    bool bIsBetter;
    bool bSecSet = false;

    cPt2.x = 2.0*da*cPt1.x;
    cPt2.y = -1.0;
    double dNorm = GetNorm(cPt2);
    cPt2 /= dNorm;

    double d1, d2;
    d1 = GetDist(cRefPt, cPt1 + dOffset*cPt2);
    double dSec = d1;

    for(int i = 1; i < iRoots; i++)
    {
        cPt1.x = dRoots[i];
        cPt1.y = da*Power2(cPt1.x);
        cPt2.x = 2.0*da*cPt1.x;
        cPt2.y = -1.0;
        dNorm = GetNorm(cPt2);
        cPt2 /= dNorm;
        d2 = GetDist(cRefPt, cPt1 + dOffset*cPt2);
        bIsBetter = ((d2 < d1 - g_dPrec) && !(bFirstSet && !bInBounds[i])) || (!bFirstSet && bInBounds[i]);

        if(bIsBetter)
        {
            if(bSecSet)
            {
                dXSec = dXMin;
                dSec = d1;
            }
            d1 = d2;
            dXMin = cPt1.x;
        }
        else if(bSecSet)
        {
            if(d2 < dSec)
            {
                dXSec = cPt1.x;
                dSec = d2;
            }
        }
        else
        {
            dXSec = cPt1.x;
            dSec = d2;
            bSecSet = true;
        }
    }
    if(iSrchMask & 2) return dXSec;
    return dXMin;
}

bool GetParabLineXFromU(double da, double dOffset, double dStart, CDPoint cLnOrg, CDPoint cLnDir, double *pdX)
{
    CDPoint cPt1, cPt2, cPtX;
    double du = dStart;
    cPt1 = GetParabPointDir(da, dOffset, du, &cPt2);
    int ix = LineXLine(cPt1, cPt2, cLnOrg, cLnDir, &cPtX);
    int i = 0;
    bool bFound = GetDist(cPt1, cPtX) < g_dPrec;
    while(!bFound && (ix > 0) && (i < 8))
    {
        du = GetParabPtProj(da, cPtX, cPtX, dOffset, 0, NULL);
        cPt1 = GetParabPointDir(da, dOffset, du, &cPt2);
        ix = LineXLine(cPt1, cPt2, cLnOrg, cLnDir, &cPtX);
        i++;
        bFound = GetDist(cPt1, cPtX) < g_dPrec;
    }
    if(bFound) *pdX = du;
    return bFound;
}

bool GetParabQuadApp(double da, double dOffset, double dt1, double dt2, PDPoint pQuad)
{
    CDPoint cDir1, cDir2;
    pQuad[0] = GetParabPointDir(da, dOffset, dt1, &cDir1);
    pQuad[2] = GetParabPointDir(da, dOffset, dt2, &cDir2);
    return (LineXLine(pQuad[0], cDir1, pQuad[2], cDir2, &pQuad[1]) > 0);
}

bool GetParabXCandidate(double da, double dOffset, CDPoint cLnOrg, CDPoint cLnDir,
    double *pdMin, double *pdMax, double *pdX)
{
    double dt, dti;
    int iMax = 20;
    double dSpan = -1.0;
    if(pdMin)
    {
        dt = *pdMin;
        dti = 1.0;
        if(pdMax)
        {
            dSpan = (*pdMax - *pdMin);
            iMax = (int)dSpan + 1;
            if(iMax < 1) iMax = 1;
            dti = dSpan/iMax;
        }
    }
    else if(pdMax)
    {
        dt = *pdMax;
        dti = -1.0;
    }
    else return 0;

    int i = 0;
    bool bFound = false;
    CDPoint cQuad[3];
    int iRes;
    CDPoint cPtXs[2];
    double dts[2];

    if(dSpan > -0.5)
    {
        while(!bFound && (i < iMax))
        {
            GetParabQuadApp(da, dOffset, dt, dt + dti, cQuad);
            iRes = QuadXLine(cQuad, cLnOrg, cLnDir, cPtXs, dts);
            bFound = (iRes > 0);
            dt += dti;
            i++;
        }
        if(bFound)
        {
            *pdX = GetParabPtProj(da, cPtXs[0], cPtXs[0], dOffset, 0, NULL);
        }
        return bFound;
    }

    while(!bFound && (i < iMax))
    {
        GetParabQuadApp(da, dOffset, dt, dt + dti, cQuad);
        iRes = QuadXLine(cQuad, cLnOrg, cLnDir, cPtXs, dts);
        bFound = (iRes > 0);
        dt += dti;
        dti *= 2.0;
        i++;
    }

    if(bFound)
    {
        *pdX = GetParabPtProj(da, cPtXs[0], cPtXs[0], dOffset, 0, NULL);
    }
    return bFound;
}

int GetParabLineX(double da, double dOffset, double dBreak, CDPoint cLnOrg, CDPoint cLnDir, double *pdXs)
{
    double du0;
    int iX = 0; // 0 - tangent, 1 - cross
    if(fabs(cLnDir.y) < g_dPrec) du0 = 0.0;
    else
    {
        if(fabs(cLnDir.x) < g_dPrec) iX = 1;
        else du0 = cLnDir.y/cLnDir.x/2.0/da;
    }

    double dt1, dt2;
    int iRes = 0;

    if(iX > 0)
    {
        if(dBreak < -0.5)
        {
            dt1 = 0.0;
            if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du0))
            {
                if(GetParabLineXFromU(da, dOffset, du0, cLnOrg, cLnDir, &pdXs[0])) return 1;
            }
            if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt1, NULL, &du0))
            {
                if(GetParabLineXFromU(da, dOffset, du0, cLnOrg, cLnDir, &pdXs[0])) return 1;
            }
            return 0;
        }

        dt1 = -dBreak;
        dt2 = dBreak;
        if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du0))
        {
            if(GetParabLineXFromU(da, dOffset, du0, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt1, &dt2, &du0))
        {
            if(GetParabLineXFromU(da, dOffset, du0, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt2, NULL, &du0))
        {
            if(GetParabLineXFromU(da, dOffset, du0, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        return iRes;
    }

    double du1;

    if(dBreak < -0.5)
    {
        CDPoint cPt1, cPt2, cPt3;
        cPt1.x = du0;
        cPt1.y = da*Power2(du0);
        du1 = cLnDir*(cPt1 - cLnOrg);
        cPt2 = cLnOrg + du1*cLnDir;
        cPt3.x = 2.0*da*du0;
        cPt3.y = -1.0;
        double dDet = GetNorm(cPt3);
        cPt3 /= dDet;
        if(fabs(cPt3.x) > g_dPrec) dDet = (cPt2.x - cPt1.x)/cPt3.x;
        else if(fabs(cPt3.y) > g_dPrec) dDet = (cPt2.y - cPt1.y)/cPt3.y;
        else dDet = -1.0;
        if(dDet > g_dPrec) return 0;
        if(dDet > -g_dPrec)
        {
            pdXs[0] = du0;
            return 1;
        }

        if(fabs(du0) < g_dPrec)
        {
            dt1 = 0.0;
            if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du1))
            {
                if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
            }
            if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt1, NULL, &du1))
            {
                if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
            }
            return iRes;
        }
        if(du0 < -g_dPrec)
        {
            dt1 = du0;
            dt2 = 0.0;
        }
        else
        {
            dt1 = 0.0;
            dt2 = du0;
        }
        if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du1))
        {
            if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt1, &dt2, &du1))
        {
            if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt2, NULL, &du1))
        {
            if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        return iRes;
    }

    if((fabs(du0 + dBreak) < g_dPrec) || (fabs(du0 - dBreak) < g_dPrec))
    {
        dt1 = -dBreak;
        dt2 = dBreak;
        if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du1))
        {
            if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt1, &dt2, &du1))
        {
            if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt2, NULL, &du1))
        {
            if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        return iRes;
    }

    double dt3;
    if(du0 < -dBreak - g_dPrec)
    {
        dt1 = du0;
        dt2 = -dBreak;
        dt3 = dBreak;
    }
    else if(du0 < dBreak - g_dPrec)
    {
        dt1 = -dBreak;
        dt2 = du0;
        dt3 = dBreak;
    }
    else
    {
        dt1 = -dBreak;
        dt2 = dBreak;
        dt3 = du0;
    }

    if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du1))
    {
        if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
    }
    if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt1, &dt2, &du1))
    {
        if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
    }
    if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt2, &dt3, &du1))
    {
        if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
    }
    if(GetParabXCandidate(da, dOffset, cLnOrg, cLnDir, &dt3, NULL, &du1))
    {
        if(GetParabLineXFromU(da, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
    }
    return iRes;
}

int GetParabSegX(double da, double dOffset, double dBreak, CDPoint cPt1, CDPoint cPt2, double *pdXs)
{
    CDPoint cDir = cPt2 - cPt1;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return 0;

    cDir /= dNorm;
    double cPtXs[4];

    int iX = GetParabLineX(da, dOffset, dBreak, cPt1, cDir, cPtXs);
    int iRes = 0;
    CDPoint cPt3, cPt4;
    for(int i = 0; i < iX; i++)
    {
        cPt3 = GetParabPointDir(da, dOffset, cPtXs[i], &cPt4);
        cPt4 = Rotate(cPt3 - cPt1, cDir, false);
        if((cPt4.x > -g_dPrec) && (cPt4.x < dNorm + g_dPrec)) pdXs[iRes++] = cPtXs[i];
    }
    return iRes;
}

int BuildParabPrimitives(CDLine cTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly)
{
    if(iMode > 0) BuildParabCache(cTmpPt, iMode, pPoints, pCache, pLines, pdDist);

    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return 0;

    CDPoint cOrig, cRad, cMainDir;

    cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;
    cMainDir = pCache->GetPoint(2, 0).cPoint;

    double dr = dOffset;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr += pCache->GetPoint(0, 2).cPoint.x;

    double dBreak = -1.0;
    if(pCache->GetCount(4) > 0) dBreak = pCache->GetPoint(0, 4).cPoint.x;

    double dXs[16];
    int iXs = 0;
    CDPoint cLn1, cLn2;
    CDPoint cPt1, cPt2;

    cLn1.x = pRect->cPt1.x;
    cLn1.y = pRect->cPt1.y;
    cLn2.x = pRect->cPt1.x;
    cLn2.y = pRect->cPt2.y;
    cPt1 = Rotate(cLn1 - cOrig, cMainDir, false);
    cPt2 = Rotate(cLn2 - cOrig, cMainDir, false);
    iXs += GetParabSegX(cRad.x, dr, dBreak, cPt1, cPt2, &dXs[iXs]);

    cLn1.x = pRect->cPt1.x;
    cLn1.y = pRect->cPt2.y;
    cLn2.x = pRect->cPt2.x;
    cLn2.y = pRect->cPt2.y;
    cPt1 = Rotate(cLn1 - cOrig, cMainDir, false);
    cPt2 = Rotate(cLn2 - cOrig, cMainDir, false);
    iXs += GetParabSegX(cRad.x, dr, dBreak, cPt1, cPt2, &dXs[iXs]);

    cLn1.x = pRect->cPt2.x;
    cLn1.y = pRect->cPt2.y;
    cLn2.x = pRect->cPt2.x;
    cLn2.y = pRect->cPt1.y;
    cPt1 = Rotate(cLn1 - cOrig, cMainDir, false);
    cPt2 = Rotate(cLn2 - cOrig, cMainDir, false);
    iXs += GetParabSegX(cRad.x, dr, dBreak, cPt1, cPt2, &dXs[iXs]);

    cLn1.x = pRect->cPt2.x;
    cLn1.y = pRect->cPt1.y;
    cLn2.x = pRect->cPt1.x;
    cLn2.y = pRect->cPt1.y;
    cPt1 = Rotate(cLn1 - cOrig, cMainDir, false);
    cPt2 = Rotate(cLn2 - cOrig, cMainDir, false);
    iXs += GetParabSegX(cRad.x, dr, dBreak, cPt1, cPt2, &dXs[iXs]);
//SendMessage(g_hStatus, SB_SETTEXT, 2, (LPARAM)buf);

    if(iXs < 1) return 0;

    double cMinX = dXs[0];
    double cMaxX = dXs[0];
    for(int i = 1; i < iXs; i++)
    {
        if(dXs[i] < cMinX) cMinX = dXs[i];
        if(dXs[i] > cMaxX) cMaxX = dXs[i];
    }

    pDrawBnds->x = GetParabXLen(cRad.x, dr, dBreak, cMinX);
    pDrawBnds->y = GetParabXLen(cRad.x, dr, dBreak, cMaxX);

    if(pBounds[0].bIsSet)
    {
        if(pBounds[0].dRef > cMinX) cMinX = pBounds[0].dRef;
    }

    if(pBounds[1].bIsSet)
    {
        if(pBounds[1].dRef < cMaxX) cMaxX = pBounds[1].dRef;
    }

    if(cMaxX < cMinX) return 0;

    int iRes1;
    if(bQuadsOnly)
        iRes1 = AddParabSegQuadsWithBounds(cRad.x, dr, cOrig, cMainDir, cMinX, cMaxX, dBreak, pPrimList, pRect);
    else
        iRes1 = AddParabSegWithBounds(cRad.x, dr, cOrig, cMainDir, cMinX, cMaxX, dBreak, pPrimList, pRect);
    if(iRes1 < 0) iRes1 = 0;
    return iRes1;
}

double GetParabDistFromPt(CDPoint cPt, CDPoint cRefPt, int iSrchMask, PDPointList pCache, PDLine pPtX, PDRefPoint pBounds)
{
    pPtX->bIsSet = false;

    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return 0.0;

    CDPoint cOrig, cRad, cNorm, cPt1, cPt2, cPt3, cPt4;

    cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;
    cNorm = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    cPt1 = Rotate(cPt - cOrig, cNorm, false);
    cPt2 = Rotate(cRefPt - cOrig, cNorm, false);

    double dMinX = 0.0;
    if((iSrchMask & 1) && (pCache->GetCount(3) > 0))
    {
        cPt3 = pCache->GetPoint(0, 3).cPoint;
        dMinX = GetDist(cPt2, cPt3);

        pPtX->bIsSet = true;
        pPtX->cOrigin = cOrig + Rotate(cPt3, cNorm, true);
        pPtX->cDirection = 0;
    }

    double dx = GetParabPtProj(cRad.x, cPt1, cPt2, dr, iSrchMask, pBounds);

    CDPoint cDir;
    cDir.x = 2.0*cRad.x*dx;
    cDir.y = -1.0;
    double d1 = GetNorm(cDir);
    if(d1 < g_dPrec) return dMinX;
    cDir /= d1;

    cPt3.x = dx + dr*cDir.x;
    cPt3.y = cRad.x*Power2(dx) + dr*cDir.y;
    cPt4 = Rotate(cPt1 - cPt3, cDir, false);

    if(!pPtX->bIsSet || (fabs(cPt4.x) < dMinX))
    {
        dMinX = cPt4.x;
        pPtX->bIsSet = true;
        pPtX->cOrigin = cOrig + Rotate(cPt3, cNorm, true);
        pPtX->cDirection = Rotate(cDir, cNorm, true);
        pPtX->dRef = dx;
    }
    return dMinX;
}

bool HasParabEnoughPoints(PDPointList pPoints, int iInputLines)
{
    int nNorm = pPoints->GetCount(0);
    bool bRes = false;

    if(iInputLines == 1) bRes = (nNorm > 0);

    return bRes;
}

double GetParabRadiusAtPt(CDPoint cPt, PDPointList pCache, PDLine pPtR, bool bNewPt)
{
    pPtR->bIsSet = false;
    pPtR->cOrigin = 0;
    pPtR->cDirection = 0;

    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return -1.0;

    CDPoint cOrig, cRad, cNorm, cPt1, cPt2, cDir;

    cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;
    cNorm = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    cPt1 = Rotate(cPt - cOrig, cNorm, false);

    cPt2.x = -4.0*cRad.x*cPt1.x*cPt1.y;
    cPt2.y = 3.0*cPt1.y + 1/cRad.x/2.0;

    cDir = cPt1 - cPt2;
    double dNorm = GetNorm(cDir);

    pPtR->bIsSet = true;
    pPtR->cOrigin = cOrig + Rotate(cPt2, cNorm, true);
    if(dNorm > g_dPrec) pPtR->cDirection = Rotate(cDir/dNorm, cNorm, true);

    return dNorm + dr;
}

bool GetParabPointRefDist(double dRef, PDPointList pCache, double *pdDist)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    //CDPoint cNorm = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    double dBreak = -1.0;
    if(pCache->GetCount(4) > 0) dBreak = pCache->GetPoint(0, 4).cPoint.x;

    *pdDist = GetParabXLen(cRad.x, dr, dBreak, dRef);
    return true;
}

double GetParabPointAtDist(double da, double dr, double dBreak, double dDist)
{
    CDRefPoint cBnds[2];
    CDPoint cPt1 = GetCurveRefAtDist(da, 1.0, dr, dBreak, fabs(dDist), ParabFunc, ParabFuncDer, cBnds);
    if(dDist < 0.0)
    {
        cPt1.x *= -1.0;
        double dt = -cBnds[1].dRef;
        cBnds[1].dRef = -cBnds[0].dRef;
        cBnds[0].dRef = dt;
    }
    return GetParabPtProj(da, cPt1, cPt1, dr, 0, cBnds);
}

void AddParabSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return;

    CDPoint cOrig, cRad, cNorm;

    cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;
    cNorm = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    double dBreak = -1.0;
    if(pCache->GetCount(4) > 0) dBreak = pCache->GetPoint(0, 4).cPoint.x;

    double dx1 = GetParabPointAtDist(cRad.x, dr, dBreak, d1);
    double dx2 = GetParabPointAtDist(cRad.x, dr, dBreak, d2);
    AddParabSegWithBounds(cRad.x, dr, cOrig, cNorm, dx1, dx2, dBreak, pPrimList, pRect);
}

bool GetParabRefPoint(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return false;

    CDPoint cOrig, cRad, cNorm;

    cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;
    cNorm = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    CDPoint cDir;
    cDir.x = 2.0*cRad.x*dRef;
    cDir.y = -1.0;
    double d1 = GetNorm(cDir);
    if(d1 < g_dPrec) return false;
    cDir /= d1;

    CDPoint cPt1;
    cPt1.x = dRef + dr*cDir.x;
    cPt1.y = cRad.x*Power2(dRef) + dr*cDir.y;
    *pPt = cOrig + Rotate(cPt1, cNorm, true);
    return true;
}

bool GetParabRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
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
    double dx = GetParabPtProj(cRad.x, cPt1, cPt1, dDist, 0, NULL);

    CDPoint cDir;
    cDir.x = 1.0;
    cDir.y = 2.0*cRad.x*dx;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return false;

    CDPoint cPt2;
    cPt2.x = dx + dRad*cDir.y/dNorm;
    cPt2.y = cRad.x*Power2(dx) - dRad*cDir.x/dNorm;
    *pSnapPt = cOrig + Rotate(cPt2, cMainDir, true);
    return true;
}

bool GetParabRefDir(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cNorm = pCache->GetPoint(2, 0).cPoint;

    CDPoint cDir;
    cDir.x = 1.0;
    cDir.y = 2.0*cRad.x*dRef;
    double d1 = GetNorm(cDir);
    if(d1 < g_dPrec) return false;
    cDir /= d1;

    *pPt = Rotate(cDir, cNorm, true);
    return true;
}

bool GetParabReference(double dDist, PDPointList pCache, double *pdRef)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return false;

    //CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    //CDPoint cNorm = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    double dBreak = -1.0;
    if(pCache->GetCount(4) > 0) dBreak = pCache->GetPoint(0, 4).cPoint.x;

    *pdRef = GetParabPointAtDist(cRad.x, dr, dBreak, dDist);
    return true;
}

int GetParabNumParts(PDPointList pCache, PDRefPoint pBounds)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return 0;

    int iBreaks = pCache->GetCount(4);
    if(iBreaks < 1) return 0;

    CDPoint cBreak = pCache->GetPoint(0, 4).cPoint;

    double dAng = cBreak.x;

    if(dAng < g_dPrec)
    {
        if(pBounds[0].bIsSet && (pBounds[0].dRef > -g_dPrec)) return 0;
        if(pBounds[1].bIsSet && (pBounds[1].dRef < g_dPrec)) return 0;
        return 1;
    }

    int iRes = 0;
    if(RefInOpenBounds(pBounds, -dAng) > 2) iRes++;
    if(RefInOpenBounds(pBounds, dAng) > 2) iRes++;

    return iRes;
}

bool ParabRemovePart(bool bDown, PDPointList pCache, PDRefPoint pBounds)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return false;

    int iBreaks = pCache->GetCount(4);
    if(iBreaks < 1) return false;

    CDPoint cBreak = pCache->GetPoint(0, 4).cPoint;

    double dAng = cBreak.x;

    if(dAng < g_dPrec)
    {
        if(bDown)
        {
            pBounds[0].bIsSet = true;
            pBounds[0].dRef = 0.0;
        }
        else
        {
            pBounds[1].bIsSet = true;
            pBounds[1].dRef = 0.0;
        }
        return true;
    }

    if(bDown)
    {
        if(RefInOpenBounds(pBounds, dAng) > 2)
        {
            pBounds[0].bIsSet = true;
            pBounds[0].dRef = dAng;
        }
        else if(RefInOpenBounds(pBounds, -dAng) > 2)
        {
            pBounds[0].bIsSet = true;
            pBounds[0].dRef = -dAng;
        }
    }
    else
    {
        if(RefInOpenBounds(pBounds, dAng) > 2)
        {
            pBounds[1].bIsSet = true;
            pBounds[1].dRef = dAng;
        }
        else if(RefInOpenBounds(pBounds, -dAng) > 2)
        {
            pBounds[1].bIsSet = true;
            pBounds[1].dRef = -dAng;
        }
    }
    return true;
}

