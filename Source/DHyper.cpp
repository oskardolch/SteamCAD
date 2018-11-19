#include "DHyper.hpp"
#include "DMath.hpp"
#include <math.h>
#include "DPrimitive.hpp"
#include <stdio.h>

// for debugging purpose only
/*#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
extern HWND g_hStatus;*/
// -----

bool AddHyperPoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines)
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
        if((iCtrl < 1) && (nNorm < 1))
        {
            pPoints->AddPoint(x, y, 0);
            nNorm++;
        }
        bRes = (nNorm > 0);
    }
    return bRes;
}

double GetHyperBreakAngle(double da, double db, double dr)
{
    double da2 = Power2(da);
    double db2 = Power2(db);
    double d1 = da2 + db2;
    if(d1 < g_dPrec) return -1.0;

    double d2 = cbrt(da2*db2*Power2(dr));
    double d3 = d2 - db2;

    if(d3 < -g_dPrec) return -1.0;
    if(d3 < g_dPrec) return 0.0;

    return sqrt(d3/d1);
}

bool BuildHyperCache(CDLine cTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    PDLine pLines, double *pdDist)
{
    pCache->ClearAll();

    int nNorm = pPoints->GetCount(0);

    CDInputPoint cInPt1;
    CDPoint cOrig, cMainDir, cPt1, cPt2, cPt3;
    double d1, d2, d3;
    double dr1 = -1.0, dr2 = -1.0;

    if(!(pLines[0].bIsSet && pLines[1].bIsSet)) return false;

    int iX = LineXLine(pLines[0].cOrigin, pLines[0].cDirection,
        pLines[1].cOrigin, pLines[1].cDirection, &cOrig);
    if(iX < 1) return false;

    if(!(((nNorm < 1) && (iMode == 1)) || ((nNorm == 1) && (iMode != 1)))) return false;

    if(iMode == 1) cPt1 = cTmpPt.cOrigin - cOrig;
    else if(nNorm > 0)
    {
        cInPt1 = pPoints->GetPoint(0, 0);
        cPt1 = cInPt1.cPoint - cOrig;
    }
    else return false;

    cPt2 = pLines[0].cDirection + pLines[1].cDirection;
    d2 = GetNorm(cPt2);
    cMainDir = cPt2/d2;

    cPt2 = Rotate(cPt1, cMainDir, false);
    cPt3 = Abs(Rotate(pLines[0].cDirection, cMainDir, false));

    d1 = Deter2(Abs(cPt2), cPt3);
    if(d1 < 0)
    {
        d2 = cMainDir.x;
        cMainDir.x = -cMainDir.y;
        cMainDir.y = d2;

        cPt2 = Rotate(cPt1, cMainDir, false);
        cPt3 = Abs(Rotate(pLines[0].cDirection, cMainDir, false));
    }

    if(cPt2.x < 0)
    {
        cMainDir.x *= -1.0;
        cMainDir.y *= -1.0;

        cPt2 = Rotate(cPt1, cMainDir, false);
        cPt3 = Abs(Rotate(pLines[0].cDirection, cMainDir, false));
    }

    d3 = sqrt(Power2(cPt2.x/cPt3.x) - Power2(cPt2.y/cPt3.y));

    dr1 = d3*cPt3.x;
    dr2 = d3*cPt3.y;

    if((dr1 < g_dPrec) || (dr2 < g_dPrec)) return false;

    pCache->AddPoint(cOrig.x, cOrig.y, 0);
    pCache->AddPoint(dr1, dr2, 0);
    pCache->AddPoint(cMainDir.x, cMainDir.y, 0);

    double dr = Power2(dr2)/dr1;
    pCache->AddPoint(dr1 + dr, 0.0, 3);

    if((iMode == 2) && (cTmpPt.cDirection.x > 0.5))
    {
        dr = GetHyperBreakAngle(dr1, dr2, -cTmpPt.cDirection.y);
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
        else if(nOffs2) cPt1 = pPoints->GetPoint(0, 2).cPoint;
        else
        {
            cPt1 = pPoints->GetPoint(0, 3).cPoint;
            iSrchMask = 2;
        }

        CDLine cPtX;
        double dDist = GetHyperDistFromPt(cPt1, cPt1, iSrchMask, pCache, &cPtX, NULL);
        double dDistOld = 0.0;

        dr = GetHyperBreakAngle(dr1, dr2, -dDist);
        if(dr > -0.5) pCache->AddPoint(dr, 0.0, 4);

        if(iMode == 2)
        {
            if(nOffs2 > 0)
            {
                cPt1 = pPoints->GetPoint(0, 2).cPoint;
                dDistOld = GetHyperDistFromPt(cPt1, cPt1, 0, pCache, &cPtX, NULL);
            }
            else if(nOffs3 > 0)
            {
                cPt1 = pPoints->GetPoint(0, 3).cPoint;
                dDistOld = GetHyperDistFromPt(cPt1, cPt1, 2, pCache, &cPtX, NULL);
            }
        }

        *pdDist = dDist - dDistOld;
        if(fabs(dDist) > g_dPrec) pCache->AddPoint(dDist, dDistOld, 2);
    }
    return true;
}

CDPoint GetHyperPointDir(double da, double db, double dOffset, double dx, PDPoint pDir)
{
    CDPoint cRes, cDir;
    double dv = sqrt(1.0 + Power2(dx));
    cDir.x = -db;
    cDir.y = da*dx/dv;
    double dNorm = GetNorm(cDir);
    cDir /= dNorm;
    cRes.x = da*dv;
    cRes.y = db*dx;
    pDir->x = -cDir.y;
    pDir->y = cDir.x;
    return cRes + dOffset*cDir;
}

double GetHyperYLength(double da, double db, double dr, double dBreak, double dy)
{
    if(fabs(dy) < g_dPrec) return 0.0;

    double dDir = 1.0;
    if(dy < 0)
    {
        dDir = -1.0;
        dy *= dDir;
    }

    double dRes = 0.0;
    double dStart = 0;

    CDPrimitive cQuad;
    cQuad.cPt3.x = da - dr;
    cQuad.cPt3.y = 0.0;

    CDPoint cDir1, cDir2;
    cDir2.x = 0.0;
    cDir2.y = 1.0;

    if((dBreak > g_dPrec) && (dy > dBreak + g_dPrec))
    {
        int iSteps = (int)dBreak + 1;
        for(int i = 0; i < iSteps; i++)
        {
            cDir1 = cDir2;
            cQuad.cPt1 = cQuad.cPt3;

            dStart = (double)(i + 1.0)*dBreak/iSteps;
            cQuad.cPt3 = GetHyperPointDir(da, db, dr, dStart, &cDir2);
            LineXLine(cQuad.cPt1, cDir1, cQuad.cPt3, cDir2, &cQuad.cPt2);
            dRes += GetQuadLength(&cQuad, 0.0, 1.0);
        }
    }

    double dlny = log(2.0*(dy - dStart));
    int iSegs = (int)dlny/log(2.0) + 1;
    if(iSegs < 1) iSegs = 1;
    double dBase = (dy - dStart)/PowerN(iSegs - 1, 2.0);

    for(int i = 0; i < iSegs; i++)
    {
        cDir1 = cDir2;
        cQuad.cPt1 = cQuad.cPt3;

        cQuad.cPt3 = GetHyperPointDir(da, db, dr, dStart + dBase, &cDir2);
        LineXLine(cQuad.cPt1, cDir1, cQuad.cPt3, cDir2, &cQuad.cPt2);
        dRes += GetQuadLength(&cQuad, 0.0, 1.0);

        dBase *= 2.0;
    }

    return dDir*dRes;
}

CDPoint HyperFunc(double da, double db, double dt)
{
    double dv = sqrt(1.0 + Power2(dt));
    CDPoint cRes;
    cRes.x = da*dv;
    cRes.y = db*dt;
    return cRes;
}

CDPoint HyperFuncDer(double da, double db, double dt)
{
    double dv = sqrt(1.0 + Power2(dt));
    CDPoint cRes;
    cRes.x = -da*dt/dv;
    cRes.y = -db;
    return cRes;
}

int AddHyperSegWithBounds(double da, double db, double dr, CDPoint cOrig, CDPoint cMainDir,
    double dy1, double dy2, double dBreak, PDPrimObject pPrimList, PDRect pRect)
{
    double du1 = dy1;
    double du2 = dy2;

    if(dBreak < -0.5)
        return AddBoundCurve(da, db, dr, HyperFunc, HyperFuncDer, du1, du2, cOrig, cMainDir,
            pRect, pPrimList);

    int iRes = -1;
    int k;
    if(RefInBounds(du1, du2, -dBreak) > 2)
    {
        k = AddBoundCurve(da, db, dr, HyperFunc, HyperFuncDer, du1, -dBreak, cOrig, cMainDir,
            pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
        du1 = -dBreak;
    }
    if(RefInBounds(du1, du2, dBreak) > 2)
    {
        k = AddBoundCurve(da, db, dr, HyperFunc, HyperFuncDer, du1, dBreak, cOrig, cMainDir,
            pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
        du1 = dBreak;
    }
    k = AddBoundCurve(da, db, dr, HyperFunc, HyperFuncDer, du1, du2, cOrig, cMainDir,
        pRect, pPrimList);
    if(iRes < 0) iRes = k;
    else if(iRes != k) iRes = 1;

    return iRes;
}

int AddHyperSegQuadsWithBounds(double da, double db, double dr, CDPoint cOrig, CDPoint cMainDir,
    double dy1, double dy2, double dBreak, PDPrimObject pPrimList, PDRect pRect)
{
    double du1 = dy1;
    double du2 = dy2;

    if(dBreak < -0.5)
        return AddBoundQuadCurve(da, db, dr, HyperFunc, HyperFuncDer, du1, du2, cOrig, cMainDir,
            pRect, pPrimList);

    int iRes = -1;
    int k;
    if(RefInBounds(du1, du2, -dBreak) > 2)
    {
        k = AddBoundQuadCurve(da, db, dr, HyperFunc, HyperFuncDer, du1, -dBreak, cOrig, cMainDir,
            pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
        du1 = -dBreak;
    }
    if(RefInBounds(du1, du2, dBreak) > 2)
    {
        k = AddBoundQuadCurve(da, db, dr, HyperFunc, HyperFuncDer, du1, dBreak, cOrig, cMainDir,
            pRect, pPrimList);
        if(iRes < 0) iRes = k;
        else if(iRes != k) iRes = 1;
        du1 = dBreak;
    }
    k = AddBoundQuadCurve(da, db, dr, HyperFunc, HyperFuncDer, du1, du2, cOrig, cMainDir,
        pRect, pPrimList);
    if(iRes < 0) iRes = k;
    else if(iRes != k) iRes = 1;

    return iRes;
}

double HypProjFn(double da, double db, double dx, double dy, double du)
{
    double dv = sqrt(1.0 + Power2(du));
    return du*dv*(Power2(da) + Power2(db)) - da*dx*du - db*dy*dv;
}

double HypProjFnDer(double da, double db, double dx, double dy, double du)
{
    double dv = sqrt(1.0 + Power2(du));
    double da1 = Power2(da) + Power2(db);
    return da1*(dv + Power2(du)/dv) - da*dx - db*dy*du/dv;
}

double GetHyperPtProjFromU(double da, double db, double dStart, CDPoint cPt)
{
    int j = 0;
    double du1 = dStart;
    double df = HypProjFn(da, db, cPt.x, cPt.y, du1);
    double df2 = HypProjFnDer(da, db, cPt.x, cPt.y, du1);
    if(fabs(df2) < g_dPrec) return 0.0;

    double du2 = du1 - df/df2;

    while((j < 8) && (fabs(df) > g_dRootPrec))
    {
        j++;
        du1 = du2;
        df = HypProjFn(da, db, cPt.x, cPt.y, du1);
        df2 = HypProjFnDer(da, db, cPt.x, cPt.y, du1);
        if(fabs(df2) > g_dPrec) du2 -= df/df2;
        else j = 8;
    }
    return du2;
}

double GetHyperPtProj(double da, double db, CDPoint cPt, CDPoint cRefPt, double dOffset, int iSrchMask, PDRefPoint pBounds)
{
    double dPoly[5];
    double d1 = Power2(da) + Power2(db);
    dPoly[4] = Power2(d1);
    dPoly[0] = Power2(db*cPt.y);
    dPoly[1] = -2.0*db*cPt.y*d1;
    dPoly[2] = dPoly[4] + dPoly[0] - Power2(da*cPt.x);
    dPoly[3] = dPoly[1];

    double dRoots[4];
    int iRoots = SolvePolynom(4, dPoly, dRoots);
    if(iRoots < 1)
    {
        return GetHyperPtProjFromU(da, db, 0.0, cPt);
    }

    for(int j = 0; j < iRoots; j++)
    {
        dRoots[j] = GetHyperPtProjFromU(da, db, dRoots[j], cPt);
    }

    bool bInBounds[4];
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
    double du = dRoots[0];
    double dc = sqrt(1.0 + Power2(du));
    cPt1.x = da*dc;
    cPt1.y = db*du;
    cPt2.x = -db;
    cPt2.y = da*du/dc;
    double dNorm = GetNorm(cPt2);
    double dXMin = GetDist(cRefPt, cPt1 + dOffset*cPt2/dNorm);
    double dXSec = dXMin;
    double duSec = du;
    bool bFirstSet = bInBounds[0];
    bool bIsBetter;
    bool bSecSet = false;

    for(int i = 1; i < iRoots; i++)
    {
        cPt1 = GetHyperPointDir(da, db, dOffset, dRoots[i], &cPt2);
        d1 = GetDist(cRefPt, cPt1);

        bIsBetter = ((d1 < dXMin - g_dPrec) && !(bFirstSet && !bInBounds[i])) || (!bFirstSet && bInBounds[i]);

        if(bIsBetter)
        {
            if(bSecSet)
            {
                duSec = du;
                dXSec = dXMin;
            }
            dXMin = d1;
            du = dRoots[i];
            bFirstSet |= bInBounds[i];
        }
        else if(bSecSet)
        {
            if(d1 < dXSec)
            {
                dXSec = d1;
                duSec = dRoots[i];
            }
        }
        else
        {
            dXSec = d1;
            duSec = dRoots[i];
            bSecSet = true;
        }
    }

    if(iSrchMask & 2) return duSec;
    return du;
}

bool GetHyperLineXFromU(double da, double db, double dOffset, double dStart, CDPoint cLnOrg, CDPoint cLnDir, double *pdX)
{
    CDPoint cPt1, cPt2, cPtX;
    double du = dStart;
    cPt1 = GetHyperPointDir(da, db, dOffset, du, &cPt2);
    int ix = LineXLine(cPt1, cPt2, cLnOrg, cLnDir, &cPtX);
    int i = 0;
    bool bFound = GetDist(cPt1, cPtX) < g_dPrec;
    while(!bFound && (ix > 0) && (i < 8))
    {
        du = GetHyperPtProj(da, db, cPtX, cPtX, dOffset, 0, NULL);
        cPt1 = GetHyperPointDir(da, db, dOffset, du, &cPt2);
        ix = LineXLine(cPt1, cPt2, cLnOrg, cLnDir, &cPtX);
        i++;
        bFound = GetDist(cPt1, cPtX) < g_dPrec;
    }
    if(bFound) *pdX = du;
    return bFound;
}

bool GetHyperQuadApp(double da, double db, double dOffset, double dt1, double dt2, PDPoint pQuad)
{
    CDPoint cDir1, cDir2;
    pQuad[0] = GetHyperPointDir(da, db, dOffset, dt1, &cDir1);
    pQuad[2] = GetHyperPointDir(da, db, dOffset, dt2, &cDir2);
    int iRes = LineXLine(pQuad[0], cDir1, pQuad[2], cDir2, &pQuad[1]);
    return iRes > 0;
}

bool GetHyperXCandidate(double da, double db, double dOffset, CDPoint cLnOrg, CDPoint cLnDir,
    double *pdMin, double *pdMax, double *pdX)
{
    double dt, dti;
    int iMax = 8;
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
            GetHyperQuadApp(da, db, dOffset, dt, dt + dti, cQuad);
            iRes = QuadXLine(cQuad, cLnOrg, cLnDir, cPtXs, dts);
            bFound = (iRes > 0);
            dt += dti;
            i++;
        }
        if(bFound)
        {
            *pdX = GetHyperPtProj(da, db, cPtXs[0], cPtXs[0], dOffset, 0, NULL);
        }
        return bFound;
    }

    while(!bFound && (i < iMax))
    {
        GetHyperQuadApp(da, db, dOffset, dt, dt + dti, cQuad);
        iRes = QuadXLine(cQuad, cLnOrg, cLnDir, cPtXs, dts);
        bFound = (iRes > 0);
        dt += dti;
        dti *= 2.0;
        i++;
    }

    CDPoint cDir1, cPt2, cDir2;

    if(!bFound)
    {
        cDir2.x = da;
        if(pdMin) cDir2.y = db;
        else cDir2.y = -db;
        dSpan = GetNorm(cDir2);
        cDir2 /= dSpan;
        cPt2.x = -dOffset*cDir2.y;
        cPt2.y = dOffset*cDir2.x;

        cQuad[0] = GetHyperPointDir(da, db, dOffset, dt, &cDir1);
        LineXLine(cQuad[0], cDir1, cPt2, cDir2, &cQuad[1]);
        iRes = LineXSeg(cLnOrg, cLnDir, cQuad[0], cQuad[1], cPtXs);

        bFound = iRes > 0;
    }

    if(!bFound)
    {
        iRes = LineXLine(cLnOrg, cLnDir, cQuad[1], cDir2, cPtXs);
        if(iRes > 0)
        {
            cPt2 = Rotate(cPtXs[0] - cQuad[1], cDir2, false);
            bFound = cPt2.x > -g_dPrec;
        }
    }

    if(bFound)
    {
        *pdX = GetHyperPtProj(da, db, cPtXs[0], cPtXs[0], dOffset, 0, NULL);
    }
    return bFound;
}

int GetHyperLineX(double da, double db, double dOffset, double dBreak, CDPoint cLnOrg, CDPoint cLnDir, double *pdXs)
{
    double du0;
    int iX = 0; // 0 - tangent, 1 - asympt, 2 - cross
    if(fabs(cLnDir.x) < g_dPrec) du0 = 0.0;
    else
    {
        double dDet = Power2(cLnDir.y*da) - Power2(cLnDir.x*db);
        if(dDet < -g_dPrec) iX = 2;
        else if(dDet < g_dPrec) iX = 1;
        else
        {
            du0 = cLnDir.x*db/sqrt(dDet);
            if(du0*cLnDir.x*cLnDir.y < -g_dPrec) du0 *= -1.0;
        }
    }

    double dt1, dt2;
    int iRes = 0;

    if(iX > 0)
    {
        if(dBreak < -0.5)
        {
            dt1 = 0.0;
            if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du0))
            {
                if(GetHyperLineXFromU(da, db, dOffset, du0, cLnOrg, cLnDir, &pdXs[0])) return 1;
            }
            if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt1, NULL, &du0))
            {
                if(GetHyperLineXFromU(da, db, dOffset, du0, cLnOrg, cLnDir, &pdXs[0])) return 1;
            }
            return 0;
        }

        dt1 = -dBreak;
        dt2 = dBreak;
        if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du0))
        {
            if(GetHyperLineXFromU(da, db, dOffset, du0, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt1, &dt2, &du0))
        {
            if(GetHyperLineXFromU(da, db, dOffset, du0, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt2, NULL, &du0))
        {
            if(GetHyperLineXFromU(da, db, dOffset, du0, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        return iRes;
    }

    double du1;

    if(dBreak < -0.5)
    {
        double dDet = sqrt(1.0 + Power2(du0));
        CDPoint cPt1, cPt2, cPt3;
        cPt1.x = da*dDet;
        cPt1.y = db*du0;
        du1 = cLnDir*(cPt1 - cLnOrg);
        cPt2 = cLnOrg + du1*cLnDir;
        cPt3.x = -db;
        cPt3.y = da*du0/dDet;
        dDet = GetNorm(cPt3);
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
            if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du1))
            {
                if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
            }
            if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt1, NULL, &du1))
            {
                if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
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
        if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du1))
        {
            if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt1, &dt2, &du1))
        {
            if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt2, NULL, &du1))
        {
            if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        return iRes;
    }

    if((fabs(du0 + dBreak) < g_dPrec) || (fabs(du0 - dBreak) < g_dPrec))
    {
        dt1 = -dBreak;
        dt2 = dBreak;
        if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du1))
        {
            if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt1, &dt2, &du1))
        {
            if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
        }
        if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt2, NULL, &du1))
        {
            if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
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

    if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, NULL, &dt1, &du1))
    {
        if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
    }
    if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt1, &dt2, &du1))
    {
        if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
    }
    if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt2, &dt3, &du1))
    {
        if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
    }
    if(GetHyperXCandidate(da, db, dOffset, cLnOrg, cLnDir, &dt3, NULL, &du1))
    {
        if(GetHyperLineXFromU(da, db, dOffset, du1, cLnOrg, cLnDir, &pdXs[iRes])) iRes++;
    }
    return iRes;
}

int GetHyperSegX(double da, double db, double dOffset, double dBreak, CDPoint cPt1, CDPoint cPt2, double *pdXs)
{
    CDPoint cDir = cPt2 - cPt1;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return 0;

    cDir /= dNorm;
    double cPtXs[4];

    int iX = GetHyperLineX(da, db, dOffset, dBreak, cPt1, cDir, cPtXs);
    int iRes = 0;
    CDPoint cPt3, cPt4;
    for(int i = 0; i < iX; i++)
    {
        cPt3 = GetHyperPointDir(da, db, dOffset, cPtXs[i], &cPt4);
        cPt4 = Rotate(cPt3 - cPt1, cDir, false);
        if((cPt4.x > -g_dPrec) && (cPt4.x < dNorm + g_dPrec)) pdXs[iRes++] = cPtXs[i];
    }
    return iRes;
}

int BuildHyperPrimitives(CDLine cTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly)
{
    if(iMode > 0) BuildHyperCache(cTmpPt, iMode, pPoints, pCache, pLines, pdDist);

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
    iXs += GetHyperSegX(cRad.x, cRad.y, dr, dBreak, cPt1, cPt2, &dXs[iXs]);

    cLn1.x = pRect->cPt1.x;
    cLn1.y = pRect->cPt2.y;
    cLn2.x = pRect->cPt2.x;
    cLn2.y = pRect->cPt2.y;
    cPt1 = Rotate(cLn1 - cOrig, cMainDir, false);
    cPt2 = Rotate(cLn2 - cOrig, cMainDir, false);
    iXs += GetHyperSegX(cRad.x, cRad.y, dr, dBreak, cPt1, cPt2, &dXs[iXs]);

    cLn1.x = pRect->cPt2.x;
    cLn1.y = pRect->cPt2.y;
    cLn2.x = pRect->cPt2.x;
    cLn2.y = pRect->cPt1.y;
    cPt1 = Rotate(cLn1 - cOrig, cMainDir, false);
    cPt2 = Rotate(cLn2 - cOrig, cMainDir, false);
    iXs += GetHyperSegX(cRad.x, cRad.y, dr, dBreak, cPt1, cPt2, &dXs[iXs]);

    cLn1.x = pRect->cPt2.x;
    cLn1.y = pRect->cPt1.y;
    cLn2.x = pRect->cPt1.x;
    cLn2.y = pRect->cPt1.y;
    cPt1 = Rotate(cLn1 - cOrig, cMainDir, false);
    cPt2 = Rotate(cLn2 - cOrig, cMainDir, false);
    iXs += GetHyperSegX(cRad.x, cRad.y, dr, dBreak, cPt1, cPt2, &dXs[iXs]);

    if(iXs < 1) return 0;

    double cMinY = dXs[0];
    double cMaxY = dXs[0];
    for(int i = 1; i < iXs; i++)
    {
        if(dXs[i] < cMinY) cMinY = dXs[i];
        if(dXs[i] > cMaxY) cMaxY = dXs[i];
    }
    pDrawBnds->x = GetHyperYLength(cRad.x, cRad.y, dr, dBreak, cMinY);
    pDrawBnds->y = GetHyperYLength(cRad.x, cRad.y, dr, dBreak, cMaxY);

    if(pBounds[0].bIsSet)
    {
        if(cMinY < pBounds[0].dRef) cMinY = pBounds[0].dRef;
    }

    if(pBounds[1].bIsSet)
    {
        if(pBounds[1].dRef < cMaxY) cMaxY = pBounds[1].dRef;
    }

    if(cMaxY < cMinY) return 0;

    int iRes1 = 0;

    if(bQuadsOnly)
        iRes1 = AddHyperSegQuadsWithBounds(cRad.x, cRad.y, dr, cOrig, cMainDir,
            cMinY, cMaxY, dBreak, pPrimList, pRect);
    else
        iRes1 = AddHyperSegWithBounds(cRad.x, cRad.y, dr, cOrig, cMainDir,
            cMinY, cMaxY, dBreak, pPrimList, pRect);
    if(iRes1 < 0) iRes1 = 0;
    return iRes1;
}

double GetHyperDistFromPt(CDPoint cPt, CDPoint cRefPt, int iSrchMask, PDPointList pCache, PDLine pPtX, PDRefPoint pBounds)
{
    pPtX->bIsSet = false;

    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return 0.0;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cNorm = pCache->GetPoint(2, 0).cPoint;

    CDPoint cPt1 = Rotate(cPt - cOrig, cNorm, false);
    CDPoint cRefPt1 = Rotate(cRefPt - cOrig, cNorm, false);
    CDPoint cPt2;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    double dMinX = 0.0;
    if((iSrchMask & 1) && (pCache->GetCount(3) > 0))
    {
        cPt2 = pCache->GetPoint(0, 3).cPoint;
        dMinX = GetDist(cRefPt1, cPt2);

        pPtX->bIsSet = true;
        pPtX->cOrigin = cOrig + Rotate(cPt2, cNorm, true);
        pPtX->cDirection = 0;
    }

    double du = GetHyperPtProj(cRad.x, cRad.y, cPt1, cRefPt1, dr, iSrchMask, pBounds);

    double d1 = sqrt(1.0 + Power2(du));

    CDPoint cDir;
    cDir.x = -cRad.y;
    cDir.y = cRad.x*du/d1;
    double dNorm = GetNorm(cDir);
    cDir /= dNorm;

    cPt2.x = cRad.x*d1 + dr*cDir.x;
    cPt2.y = cRad.y*du + dr*cDir.y;
    double dDir = 1.0;

    double dMin = GetDist(cRefPt1, cPt2);
    if(!pPtX->bIsSet || (dMin < dMinX))
    {
        if(cPt1.x > cPt2.x) dDir = -1.0;

        CDPoint cPtMin = cOrig + Rotate(cPt2, cNorm, true);
        CDPoint cNormMin = Rotate(cDir, cNorm, true);

        pPtX->bIsSet = true;
        pPtX->cOrigin = cPtMin;
        pPtX->cDirection = cNormMin;
        pPtX->dRef = du;
        dMinX = dMin;
    }
    return dDir*dMinX;
}

bool HasHyperEnoughPoints(PDPointList pPoints, int iInputLines)
{
    int nNorm = pPoints->GetCount(0);
    bool bRes = false;

    if(iInputLines == 2) bRes = (nNorm > 0);

    return bRes;
}

double GetHyperRadiusAtPt(CDPoint cPt, PDPointList pCache, PDLine pPtR, bool bNewPt)
{
    pPtR->bIsSet = false;
    pPtR->cOrigin = 0;
    pPtR->cDirection = 0;

    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return -1.0;

    CDPoint cOrig, cRad, cNorm;

    cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;
    cNorm = pCache->GetPoint(2, 0).cPoint;

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    CDPoint cDir, cPt1, cPt2;

    cPt1 = Rotate(cPt - cOrig, cNorm, false);
    double du = GetHyperPtProj(cRad.x, cRad.y, cPt1, cPt1, dr, 0, NULL);

    double dx2 = 1.0 + Power2(du);
    double dy2 = Power2(du);
    double dab = Power2(cRad.y/cRad.x);

    cPt2.x = cRad.x*sqrt(dx2)*(1.0 + dy2 + dab*dx2);
    cPt2.y = cRad.y*du*(1.0 - dx2 - dy2/dab);

    pPtR->bIsSet = true;
    pPtR->cOrigin = cOrig + Rotate(cPt2, cNorm, true);

    cDir = cPt2 - cPt1;
    double dNorm = GetNorm(cDir);
    if(dNorm > g_dPrec) pPtR->cDirection = Rotate(cDir/dNorm, cNorm, true);

    return dNorm + dr;
}

bool GetHyperPointRefDist(double dRef, PDPointList pCache, double *pdDist)
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

    double dy = dRef;
    *pdDist = GetHyperYLength(cRad.x, cRad.y, dr, dBreak, dy);
    return true;
}

double GetHyperPointAtDist(double da, double db, double dr, double dBreak, double dDist)
{
    CDRefPoint cBnds[2];
    CDPoint cPt1 = GetCurveRefAtDist(da, db, dr, dBreak, fabs(dDist), HyperFunc, HyperFuncDer, cBnds);
    if(dDist < 0.0)
    {
        cPt1.y *= -1.0;
        double dt = -cBnds[1].dRef;
        cBnds[1].dRef = -cBnds[0].dRef;
        cBnds[0].dRef = dt;
    }
    return GetHyperPtProj(da, db, cPt1, cPt1, dr, 0, cBnds);
}

void AddHyperSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect)
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

    double dy1 = GetHyperPointAtDist(cRad.x, cRad.y, dr, dBreak, d1);
    double dy2 = GetHyperPointAtDist(cRad.x, cRad.y, dr, dBreak, d2);

    AddHyperSegWithBounds(cRad.x, cRad.y, dr, cOrig, cNorm, dy1, dy2, dBreak, pPrimList, pRect);
}

bool GetHyperRefPoint(double dRef, PDPointList pCache, PDPoint pPt)
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

    double dv = sqrt(1.0 + Power2(dRef));
    CDPoint cDir;
    cDir.x = -cRad.y;
    cDir.y = cRad.x*dRef/dv;
    double dN1 = GetNorm(cDir);
    if(dN1 < g_dPrec) return false;

    CDPoint cPt1;
    cPt1.x = cRad.x*dv + dr*cDir.x/dN1;
    cPt1.y = cRad.y*dRef + dr*cDir.y/dN1;
    *pPt = cOrig + Rotate(cPt1, cNorm, true);
    return true;
}

bool GetHyperRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
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
    double dy = GetHyperPtProj(cRad.x, cRad.y, cPt1, cPt1, dDist, 0, NULL);

    double dv = sqrt(1.0 + Power2(dy));

    CDPoint cDir;
    cDir.x = cRad.x*dy/dv;
    cDir.y = cRad.y;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return false;

    CDPoint cPt2;
    cPt2.x = cRad.x*dv - dRad*cDir.y/dNorm;
    cPt2.y = cRad.y*dy + dRad*cDir.x/dNorm;
    *pSnapPt = cOrig + Rotate(cPt2, cMainDir, true);
    return true;
}

bool GetHyperRefDir(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 3) return false;

    CDPoint cRad, cNorm;

    //cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;
    cNorm = pCache->GetPoint(2, 0).cPoint;

    double dv = sqrt(1.0 + Power2(dRef));
    CDPoint cDir;
    cDir.x = cRad.x*dRef/dv;
    cDir.y = cRad.y;
    double dN1 = GetNorm(cDir);
    if(dN1 < g_dPrec) return false;

    CDPoint cPt1 = cDir/dN1;
    *pPt = Rotate(cPt1, cNorm, true);
    return true;
}

bool GetHyperReference(double dDist, PDPointList pCache, double *pdRef)
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

    *pdRef = GetHyperPointAtDist(cRad.x, cRad.y, dr, dBreak, dDist);
    return true;
}

int GetHyperNumParts(PDPointList pCache, PDRefPoint pBounds)
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

bool HyperRemovePart(bool bDown, PDPointList pCache, PDRefPoint pBounds)
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
        if(RefInOpenBounds(pBounds, -dAng) > 2)
        {
            pBounds[1].bIsSet = true;
            pBounds[1].dRef = -dAng;
        }
        else if(RefInOpenBounds(pBounds, dAng) > 2)
        {
            pBounds[1].bIsSet = true;
            pBounds[1].dRef = dAng;
        }
        else return false;
    }
    else
    {
        if(RefInOpenBounds(pBounds, -dAng) > 2)
        {
            pBounds[0].bIsSet = true;
            pBounds[0].dRef = -dAng;
        }
        else if(RefInOpenBounds(pBounds, dAng) > 2)
        {
            pBounds[0].bIsSet = true;
            pBounds[0].dRef = dAng;
        }
        else return false;
    }
    return true;
}

