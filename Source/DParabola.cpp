#include "DParabola.hpp"
#include "DMath.hpp"
#include <math.h>
#include "DPrimitive.hpp"

// for debugging purpose only
/*#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
extern HWND g_hStatus;*/
// -----

bool AddParabPoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines)
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

bool BuildParabCache(PDPoint pTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
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

    if(iMode == 1) cPt1 = *pTmpPt;
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

    pCache->AddPoint(cOrig.x, cOrig.y, false);
    pCache->AddPoint(da, 0.0, false);
    pCache->AddPoint(cMainDir.x, cMainDir.y, false);

    int nOffs = pPoints->GetCount(2);
    if((iMode == 2) || (nOffs > 0))
    {
        if(iMode == 2) cPt1 = *pTmpPt;
        else cPt1 = pPoints->GetPoint(0, 2).cPoint;

        CDLine cPtX;
        double dDist = GetParabDistFromPt(cPt1, cPt1, pCache, &cPtX);
        double dDistOld = 0.0;

        if((nOffs > 0) && (iMode == 2))
        {
            cPt1 = pPoints->GetPoint(0, 2).cPoint;
            dDistOld = GetParabDistFromPt(cPt1, cPt1, pCache, &cPtX);
        }

        *pdDist = dDist - dDistOld;
        if(fabs(dDist) > g_dPrec) pCache->AddPoint(dDist, dDistOld, 2);
    }
    return true;
}

double GetParabXLen(double da, double dr, double dx)
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

    double dlny = log(2.0*dx);
    int iSegs = (int)dlny/log(2.0) + 1;
    if(iSegs < 1) iSegs = 1;
    double dBase = dx/PowerN(iSegs - 1, 2.0);

    cQuad.cPt3.x = 0.0;
    cQuad.cPt3.y = -dr;

    CDPoint cDir1, cDir2;
    cDir2.x = 1.0;
    cDir2.y = 0.0;

    double dRes = 0.0;

    for(int i = 0; i < iSegs; i++)
    {
        cDir1 = cDir2;

        cDir2.x = 1.0;
        cDir2.y = 2.0*da*dBase;
        dlny = GetNorm(cDir2);

        cQuad.cPt1 = cQuad.cPt3;
        cQuad.cPt3.x = dBase + dr*cDir2.y/dlny;
        cQuad.cPt3.y = da*Power2(dBase) - dr*cDir2.x/dlny;

        LineXLine(false, cQuad.cPt1, cDir1, cQuad.cPt3, cDir2, &cQuad.cPt2);
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
    double dx1, double dx2, PDPrimObject pPrimList, PDRect pRect)
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

    return AddBoundCurve(da, 1.0, dr, ParabFunc, ParabFuncDer, dx1, dx2, cOrig, cMainDir,
        pRect, pPrimList);
}

int BuildParabPrimitives(PDPoint pTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly)
{
    if(pTmpPt) BuildParabCache(pTmpPt, iMode, pPoints, pCache, pLines, pdDist);

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

    dCoefs[0] = 0.0;
    dCoefs[1] = 0.0;
    dCoefs[2] = 1.0;
    dCoefs[3] = -cRad.x;
    dCoefs[4] = 0.0;
    dCoefs[5] = 0.0;
    SubstituteBiQuad(dCoefs, cNorm, cOrig);

    CDPoint cLn1, cLn2;
    CDPoint cPtXs[8];
    int iXs = 0;

    cLn1.x = pRect->cPt1.x;
    cLn1.y = pRect->cPt1.y;
    cLn2.x = pRect->cPt1.x;
    cLn2.y = pRect->cPt2.y;
    iXs += BiQuadricXLine(dCoefs, cLn1, cLn2, &cPtXs[iXs]);

    cLn1 = cLn2;
    cLn2.x = pRect->cPt2.x;
    cLn2.y = pRect->cPt2.y;
    iXs += BiQuadricXLine(dCoefs, cLn1, cLn2, &cPtXs[iXs]);

    cLn1 = cLn2;
    cLn2.x = pRect->cPt2.x;
    cLn2.y = pRect->cPt1.y;
    iXs += BiQuadricXLine(dCoefs, cLn1, cLn2, &cPtXs[iXs]);

    cLn1 = cLn2;
    cLn2.x = pRect->cPt1.x;
    cLn2.y = pRect->cPt1.y;
    iXs += BiQuadricXLine(dCoefs, cLn1, cLn2, &cPtXs[iXs]);

    if(iXs < 1) return 0;

    double dLeftX, dRightX;
    cPt1 = Rotate(cPtXs[0] - cOrig, cNorm, false);
    dLeftX = cPt1.x;
    dRightX = cPt1.x;

    for(int i = 1; i < iXs; i++)
    {
        cPt1 = Rotate(cPtXs[i] - cOrig, cNorm, false);
        if(cPt1.x < dLeftX) dLeftX = cPt1.x;
        if(cPt1.x > dRightX) dRightX = cPt1.x;
    }

    dLeftX -= fabs(dr);
    dRightX += fabs(dr);

    pDrawBnds->x = GetParabXLen(cRad.x, dr, dLeftX);
    pDrawBnds->y = GetParabXLen(cRad.x, dr, dRightX);

    if(pBounds[0].bIsSet)
    {
        if(pBounds[0].dRef > dLeftX) dLeftX = pBounds[0].dRef;
    }

    if(pBounds[1].bIsSet)
    {
        if(pBounds[1].dRef < dRightX) dRightX = pBounds[1].dRef;
    }

    return AddParabSegWithBounds(cRad.x, dr, cOrig, cNorm, dLeftX, dRightX, pPrimList, pRect);
}

double ParProjFn(double da, double dx, double dy, double du)
{
    return 2.0*Power2(da)*Power3(du) + (1.0 - 2.0*da*dy)*du - dx;
}

double ParProjFnDer(double da, double dx, double dy, double du)
{
    return 6.0*Power2(da*du) + (1.0 - 2.0*da*dy);
}

double GetParabPtProj(double da, CDPoint cPt, CDPoint cRefPt)
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
        int j = 0;
        double du1 = 0;
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

    CDPoint cPt1;
    cPt1.x = dRoots[0];
    cPt1.y = da*Power2(cPt1.x);
    double dXMin = cPt1.x;

    double d1, d2;
    d1 = GetDist(cRefPt, cPt1);

    for(int i = 1; i < iRoots; i++)
    {
        cPt1.x = dRoots[i];
        cPt1.y = da*Power2(cPt1.x);
        d2 = GetDist(cRefPt, cPt1);
        if(d2 < d1)
        {
            d1 = d2;
            dXMin = cPt1.x;
        }
    }
    return dXMin;
}

double GetParabDistFromPt(CDPoint cPt, CDPoint cRefPt, PDPointList pCache, PDLine pPtX)
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
    double dx = GetParabPtProj(cRad.x, cPt1, cPt2);

    CDPoint cDir;
    cDir.x = 2.0*cRad.x*dx;
    cDir.y = -1.0;
    double d1 = GetNorm(cDir);
    if(d1 < g_dPrec) return 0.0;
    cDir /= d1;

    cPt3.x = dx + dr*cDir.x;
    cPt3.y = cRad.x*Power2(dx) + dr*cDir.y;
    cPt4 = Rotate(cPt1 - cPt3, cDir, false);

    pPtX->bIsSet = true;
    pPtX->cOrigin = cOrig + Rotate(cPt3, cNorm, true);
    pPtX->cDirection = Rotate(cDir, cNorm, true);
    pPtX->dRef = dx;
    return cPt4.x;
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

    *pdDist = GetParabXLen(cRad.x, dr, dRef);
    return true;
}

double GetParabPointAtDist(double da, double dr, double dDist)
{
    CDPoint cPt1 = GetCurveRefAtDist(da, 1.0, dr, fabs(dDist), ParabFunc, ParabFuncDer);
    if(dDist < 0.0) cPt1.x *= -1.0;
    return GetParabPtProj(da, cPt1, cPt1);
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

    double dx1 = GetParabPointAtDist(cRad.x, dr, d1);
    double dx2 = GetParabPointAtDist(cRad.x, dr, d2);
    AddParabSegWithBounds(cRad.x, dr, cOrig, cNorm, dx1, dx2, pPrimList, pRect);
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
    double dx = GetParabPtProj(cRad.x, cPt1, cPt1);

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

    *pdRef = GetParabPointAtDist(cRad.x, dr, dDist);
    return true;
}

