#include "DHyper.hpp"
#include "DMath.hpp"
#include <math.h>
#include "DPrimitive.hpp"

// for debugging purpose only
/*#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
extern HWND g_hStatus;*/
// -----

bool AddHyperPoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines)
{
    if(iCtrl == 2)
    {
        int nOffs = pPoints->GetCount(2);
        if(nOffs > 0) pPoints->SetPoint(0, 2, x, y, iCtrl);
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

bool BuildHyperCache(PDPoint pTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    PDLine pLines, double *pdDist)
{
    pCache->ClearAll();

    int nNorm = pPoints->GetCount(0);

    CDInputPoint cInPt1;
    CDPoint cOrig, cMainDir, cPt1, cPt2, cPt3;
    double d1, d2, d3;
    double dr1 = -1.0, dr2 = -1.0;

    if(!(pLines[0].bIsSet && pLines[1].bIsSet)) return false;

    int iX = LineXLine(false, pLines[0].cOrigin, pLines[0].cDirection,
        pLines[1].cOrigin, pLines[1].cDirection, &cOrig);
    if(iX < 1) return false;

    if(!(((nNorm < 1) && (iMode == 1)) || ((nNorm == 1) && (iMode != 1)))) return false;

    if(iMode == 1) cPt1 = *pTmpPt - cOrig;
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

    int nOffs = pPoints->GetCount(2);
    if((iMode == 2) || (nOffs > 0))
    {
        if(iMode == 2) cPt1 = *pTmpPt;
        else cPt1 = pPoints->GetPoint(0, 2).cPoint;

        CDLine cPtX;
        double dDist = GetHyperDistFromPt(cPt1, cPt1, pCache, &cPtX);
        double dDistOld = 0.0;

        if((nOffs > 0) && (iMode == 2))
        {
            cPt1 = pPoints->GetPoint(0, 2).cPoint;
            dDistOld = GetHyperDistFromPt(cPt1, cPt1, pCache, &cPtX);
        }

        *pdDist = dDist - dDistOld;
        if(fabs(dDist) > g_dPrec) pCache->AddPoint(dDist, dDistOld, 2);
    }
    return true;
}

double GetHyperYLength(double da, double db, double dr, double dy)
{
    if(fabs(dy) < g_dPrec) return 0.0;

    double dDir = 1.0;
    if(dy < 0)
    {
        dDir = -1.0;
        dy *= dDir;
    }

    double dlny = log(2.0*dy/db);
    int iSegs = (int)dlny/log(2.0) + 1;
    if(iSegs < 1) iSegs = 1;
    double dBase = dy/PowerN(iSegs - 1, 2.0)/db;

    CDPrimitive cQuad;
    cQuad.cPt3.x = da - dr;
    cQuad.cPt3.y = 0.0;

    CDPoint cDir1, cDir2;
    cDir2.x = 0.0;
    cDir2.y = 1.0;

    double dRes = 0.0;
    double dv;

    for(int i = 0; i < iSegs; i++)
    {
        cDir1 = cDir2;

        dv = sqrt(1.0 + Power2(dBase));
        cDir2.x = da*dBase/dv;
        cDir2.y = db;
        dlny = GetNorm(cDir2);

        cQuad.cPt1 = cQuad.cPt3;
        cQuad.cPt3.x = da*dv - dr*cDir2.y/dlny;
        cQuad.cPt3.y = db*dBase + dr*cDir2.x/dlny;

        LineXLine(false, cQuad.cPt1, cDir1, cQuad.cPt3, cDir2, &cQuad.cPt2);
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
    double dy1, double dy2, PDPrimObject pPrimList, PDRect pRect)
{
    double du1 = dy1/db;
    double du2 = dy2/db;

    return AddBoundCurve(da, db, dr, HyperFunc, HyperFuncDer, du1, du2, cOrig, cMainDir,
        pRect, pPrimList);
}

int AddHyperSegQuadsWithBounds(double da, double db, double dr, CDPoint cOrig, CDPoint cMainDir,
    double dy1, double dy2, PDPrimObject pPrimList, PDRect pRect)
{
    double du1 = dy1/db;
    double du2 = dy2/db;

    return AddBoundQuadCurve(da, db, dr, HyperFunc, HyperFuncDer, du1, du2, cOrig, cMainDir,
        pRect, pPrimList);
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

double GetHyperPtProj(double da, double db, CDPoint cPt, CDPoint cRefPt)
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
        int j = 0;
        double du1 = 0;
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

    CDPoint cPt1;
    double du = dRoots[0];
    cPt1.x = da*sqrt(1.0 + Power2(du));
    cPt1.y = db*du;
    double dMin = GetDist(cPt1, cRefPt);
    double dv;

    for(int i = 1; i < iRoots; i++)
    {
        dv = dRoots[i];
        cPt1.x = da*sqrt(1.0 + Power2(dv));
        cPt1.y = db*dv;
        d1 = GetDist(cPt1, cRefPt);
        if(d1 < dMin)
        {
            dMin = d1;
            du = dv;
        }
    }

    return du;
}

int BuildHyperPrimitives(PDPoint pTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly)
{
    if(pTmpPt) BuildHyperCache(pTmpPt, iMode, pPoints, pCache, pLines, pdDist);

    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return 0;

    CDPoint cOrig, cRad, cNorm, cPt1;

    cOrig = pCache->GetPoint(0, 0).cPoint;
    cRad = pCache->GetPoint(1, 0).cPoint;
    cNorm = pCache->GetPoint(2, 0).cPoint;

    double dr = dOffset;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr += pCache->GetPoint(0, 2).cPoint.x;

    double dCoefs[6];

    dCoefs[0] = 1.0;
    dCoefs[1] = 0.0;
    dCoefs[2] = 0.0;
    dCoefs[3] = -1.0/Power2(cRad.x);
    dCoefs[4] = 0.0;
    dCoefs[5] = 1.0/Power2(cRad.y);
    SubstituteBiQuad(dCoefs, cNorm, cOrig);

    CDPoint cLn1, cLn2;
    CDPoint cPtXs[8];
    int iXs = 0;

    cLn1.x = pRect->cPt1.x;
    cLn1.y = pRect->cPt1.y;
    cLn2.x = pRect->cPt1.x;
    cLn2.y = pRect->cPt2.y;
    iXs += BiQuadricXLine(dCoefs, cLn1, cLn2, &cPtXs[iXs]);

    cLn1.x = pRect->cPt1.x;
    cLn1.y = pRect->cPt2.y;
    cLn2.x = pRect->cPt2.x;
    cLn2.y = pRect->cPt2.y;
    iXs += BiQuadricXLine(dCoefs, cLn1, cLn2, &cPtXs[iXs]);

    cLn1.x = pRect->cPt2.x;
    cLn1.y = pRect->cPt2.y;
    cLn2.x = pRect->cPt2.x;
    cLn2.y = pRect->cPt1.y;
    iXs += BiQuadricXLine(dCoefs, cLn1, cLn2, &cPtXs[iXs]);

    cLn1.x = pRect->cPt2.x;
    cLn1.y = pRect->cPt1.y;
    cLn2.x = pRect->cPt1.x;
    cLn2.y = pRect->cPt1.y;
    iXs += BiQuadricXLine(dCoefs, cLn1, cLn2, &cPtXs[iXs]);

    if(iXs < 1) return 0;

    bool bFound = false;

    int i = 0;
    while(!bFound && (i < iXs))
    {
        cPt1 = Rotate(cPtXs[i++] - cOrig, cNorm, false);
        bFound = cPt1.x > 0;
    }

    if(!bFound) return 0;

    double cMinY = cPt1.y;
    double cMaxY = cPt1.y;

    while(i < iXs)
    {
        cPt1 = Rotate(cPtXs[i++] - cOrig, cNorm, false);
        if(cPt1.x > 0)
        {
            if(cPt1.y < cMinY) cMinY = cPt1.y;
            if(cPt1.y > cMaxY) cMaxY = cPt1.y;
        }
    }

    cMinY -= fabs(dr);
    cMaxY += fabs(dr);

    pDrawBnds->x = GetHyperYLength(cRad.x, cRad.y, dr, cMinY);
    pDrawBnds->y = GetHyperYLength(cRad.x, cRad.y, dr, cMaxY);

    if(pBounds[0].bIsSet)
    {
        if(cMinY < cRad.y*pBounds[0].dRef) cMinY = cRad.y*pBounds[0].dRef;
    }

    if(pBounds[1].bIsSet)
    {
        if(cRad.y*pBounds[1].dRef < cMaxY) cMaxY = cRad.y*pBounds[1].dRef;
    }

    if(cMaxY < cMinY) return 0;

    int iRes1 = 0;

    if(bQuadsOnly)
        iRes1 = AddHyperSegQuadsWithBounds(cRad.x, cRad.y, dr, cOrig, cNorm,
            cMinY, cMaxY, pPrimList, pRect);
    else
        iRes1 = AddHyperSegWithBounds(cRad.x, cRad.y, dr, cOrig, cNorm,
            cMinY, cMaxY, pPrimList, pRect);
    if(iRes1 < 0) iRes1 = 0;
    return iRes1;
}

double GetHyperDistFromPt(CDPoint cPt, CDPoint cRefPt, PDPointList pCache, PDLine pPtX)
{
    pPtX->bIsSet = false;

    int iCnt = pCache->GetCount(0);
    if(iCnt < 3) return 0.0;

    CDPoint cOrig = pCache->GetPoint(0, 0).cPoint;
    CDPoint cRad = pCache->GetPoint(1, 0).cPoint;
    CDPoint cNorm = pCache->GetPoint(2, 0).cPoint;

    CDPoint cPt1 = Rotate(cPt - cOrig, cNorm, false);
    CDPoint cRefPt1 = Rotate(cRefPt - cOrig, cNorm, false);

    double du = GetHyperPtProj(cRad.x, cRad.y, cPt1, cRefPt1);

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    double d1 = sqrt(1.0 + Power2(du));

    CDPoint cDir;
    cDir.x = -cRad.y;
    cDir.y = cRad.x*du/d1;
    double dNorm = GetNorm(cDir);
    cDir /= dNorm;

    CDPoint cPt2;
    cPt2.x = cRad.x*d1 + dr*cDir.x;
    cPt2.y = cRad.y*du + dr*cDir.y;
    double dDir = 1.0;
    if(cPt1.x > cPt2.x) dDir = -1.0;

    double dMin = GetDist(cPt1, cPt2);
    CDPoint cPtMin = cOrig + Rotate(cPt2, cNorm, true);
    CDPoint cNormMin = Rotate(cDir, cNorm, true);

    pPtX->bIsSet = true;
    pPtX->cOrigin = cPtMin;
    pPtX->cDirection = cNormMin;
    pPtX->dRef = du;
    return dDir*dMin;
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
    double du = GetHyperPtProj(cRad.x, cRad.y, cPt1, cPt1);

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

    double dy = cRad.y*dRef;
    *pdDist = GetHyperYLength(cRad.x, cRad.y, dr, dy);
    return true;
}

double GetHyperPointAtDist(double da, double db, double dr, double dDist)
{
    CDPoint cPt1 = GetCurveRefAtDist(da, db, dr, fabs(dDist), HyperFunc, HyperFuncDer);
    if(dDist < 0.0) cPt1.y *= -1.0;
    return db*GetHyperPtProj(da, db, cPt1, cPt1);
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

    double dy1 = GetHyperPointAtDist(cRad.x, cRad.y, dr, d1);
    double dy2 = GetHyperPointAtDist(cRad.x, cRad.y, dr, d2);

    AddHyperSegWithBounds(cRad.x, cRad.y, dr, cOrig, cNorm, dy1, dy2, pPrimList, pRect);
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
    double dy = GetHyperPtProj(cRad.x, cRad.y, cPt1, cPt1);

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

    *pdRef = GetHyperPointAtDist(cRad.x, cRad.y, dr, dDist);
    return true;
}

