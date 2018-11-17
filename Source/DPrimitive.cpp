#include "DPrimitive.hpp"
#include "DMath.hpp"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// for debugging purpose only
/*#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
extern HWND g_hStatus;*/
// -----

int CropLineLeft(CDPrimitive cPrim, CDPoint cPt1, CDPoint cPt2,
    PDPrimObject pPrimList)
{
    CDPoint cDir = cPt2 - cPt1;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return 0;

    CDPoint cN1 = cDir/dNorm;
    CDPoint cX1 = Rotate(cPrim.cPt1 - cPt1, cN1, false);
    CDPoint cX2 = Rotate(cPrim.cPt2 - cPt1, cN1, false);

    if((cX1.y > 0) && (cX2.y > 0)) return 0;
    if((cX1.y < 0) && (cX2.y < 0))
    {
        pPrimList->AddPrimitive(cPrim);
        return 2;
    }

    CDPoint cY1 = {0, 0};
    CDPoint cY2 = {dNorm, 0.0};

    CDPoint cRes;

    int iX = LineXLine(cX1, cX2 - cX1, cY1, cY2 - cY1, &cRes);

    if(iX < 1) return 0;

    cY1 = Rotate(cRes, cN1, true);

    if(cX1.y > 0) cPrim.cPt1 = cY1 + cPt1;
    else if(cX2.y > 0) cPrim.cPt2 = cY1 + cPt1;
    pPrimList->AddPrimitive(cPrim);

    return 1;
}

int GetQuadrant(CDPoint cPt1)
{
    if((cPt1.y <= 0) && (cPt1.x > 0)) return 1;
    if((cPt1.y < 0) && (cPt1.x <= 0)) return 2;
    if((cPt1.y >= 0) && (cPt1.x < 0)) return 3;
    return 4;
}

int CmpAngle(CDPoint cPt1, CDPoint cPt2)
{
    double dx = fabs(cPt2.x - cPt1.x);
    double dy = fabs(cPt2.y - cPt1.y);

    if((dx < g_dPrec) && (dy < g_dPrec)) return 0; // the angles are equal

    int iq1 = GetQuadrant(cPt1);
    int iq2 = GetQuadrant(cPt2);

    if(iq1 < iq2) return 1;
    if(iq2 < iq1) return -1;

    if(iq1 < 3) return cPt1.x < cPt2.x ? -1: 1;
    return cPt1.x > cPt2.x ? -1: 1;
}

int CropArcLeft(CDPrimitive cPrim, CDPoint cPt1, CDPoint cPt2,
    PDPrimObject pPrimList)
{
    CDPoint cDir = cPt2 - cPt1;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return 0;

    double dr = cPrim.cPt2.x - cPrim.cPt1.x;
    CDPoint cN1 = cDir/dNorm;
    CDPoint cX1 = Rotate(cPrim.cPt1 - cPt1, cN1, false);
    CDPoint cX2 = Rotate(cPrim.cPt3 - cPt1, cN1, false);
    CDPoint cX3 = Rotate(cPrim.cPt4 - cPt1, cN1, false);

    if(cX1.y - dr > 0) return 0;
    if(cX1.y + dr < 0)
    {
        pPrimList->AddPrimitive(cPrim);
        return 2;
    }

    CDPoint cDir2 = cPrim.cPt3 - cPrim.cPt1;
    double dNorm2 = GetNorm(cDir2);
    if(dNorm2 < g_dPrec) return 0;

    CDPoint cY1 = {0, 0};
    CDPoint cY2 = {dNorm, 0.0};

    CDPoint cRes[2];

    int iRes = 0;

    int iX = CircXLine(false, cX1, dr, cY1, cY2, cRes);

    if(iX < 2)
    {
        if(cX1.y < 0)
        {
            pPrimList->AddPrimitive(cPrim);
            iRes = 2;
        }
        return iRes;
    }

    CDPoint cxX1 = cPt1 + Rotate(cRes[1], cN1, true);
    CDPoint cxX2 = cPt1 + Rotate(cRes[0], cN1, true);

    CDPoint cN2 = cDir2/dNorm2;

    CDPoint cXR1 = Rotate(cxX1 - cPrim.cPt1, cN2, false);
    CDPoint cXR2 = Rotate(cxX2 - cPrim.cPt1, cN2, false);
    CDPoint cXR3 = Rotate(cPrim.cPt4 - cPrim.cPt1, cN2, false);

    int i1 = CmpAngle(cXR1, cXR3);
    if(i1 < 1)
    {
        if(cX2.y < 0)
        {
            iRes = 2;
            if(cX3.y > 0)
            {
                cPrim.cPt4 = cPt1 + Rotate(cRes[0], cN1, true);
                iRes = 1;
            }
            pPrimList->AddPrimitive(cPrim);
        }
        else if(cX3.y < 0)
        {
            iRes = 2;
            if(cX2.y > 0)
            {
                cPrim.cPt3 = cPt1 + Rotate(cRes[1], cN1, true);
                iRes = 1;
            }
            pPrimList->AddPrimitive(cPrim);
        }
        return iRes;
    }

    int i2 = CmpAngle(cXR2, cXR3);
    if(i2 < 1)
    {
        if(cX2.y < 0) cPrim.cPt4 = cPt1 + Rotate(cRes[0], cN1, true);
        else cPrim.cPt3 = cPt1 + Rotate(cRes[1], cN1, true);
        pPrimList->AddPrimitive(cPrim);
        return 1;
    }

    if(cX2.y > 0)
    {
        cPrim.cPt3 = cPt1 + Rotate(cRes[1], cN1, true);
        cPrim.cPt4 = cPt1 + Rotate(cRes[0], cN1, true);
        pPrimList->AddPrimitive(cPrim);
    }
    else
    {
        CDPrimitive cPrim1;
        cPrim1.iType = 2;
        cPrim1.cPt1 = cPrim.cPt1;
        cPrim1.cPt2 = cPrim.cPt2;
        cPrim1.cPt3 = cPrim.cPt3;
        cPrim1.cPt4 = cPt1 + Rotate(cRes[0], cN1, true);
        pPrimList->AddPrimitive(cPrim1);
        cPrim1.cPt3 = cPt1 + Rotate(cRes[1], cN1, true);
        cPrim1.cPt4 = cPrim.cPt4;
        pPrimList->AddPrimitive(cPrim1);
    }
    return 1;
}

int CropCircLeft(CDPrimitive cPrim, CDPoint cPt1, CDPoint cPt2,
    PDPrimObject pPrimList)
{
    CDPoint cDir = cPt2 - cPt1;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return 0;

    double dr = cPrim.cPt2.x - cPrim.cPt1.x;
    CDPoint cN1 = cDir/dNorm;
    CDPoint cX1 = Rotate(cPrim.cPt1 - cPt1, cN1, false);

    if(cX1.y - dr > 0) return 0;
    if(cX1.y + dr < 0)
    {
        pPrimList->AddPrimitive(cPrim);
        return 2;
    }

    CDPoint cY1 = {0, 0};
    CDPoint cY2 = {dNorm, 0.0};

    CDPoint cRes[2];

    int iX = CircXLine(false, cX1, dr, cY1, cY2, cRes);

    if(iX < 1) return 0;
    if(iX < 2)
    {
        if(cX1.y > 0) return 0;
        pPrimList->AddPrimitive(cPrim);
        return 2;
    }

    cPrim.iType = 2;
    cPrim.cPt3 = cPt1 + Rotate(cRes[1], cN1, true);
    cPrim.cPt4 = cPt1 + Rotate(cRes[0], cN1, true);
    pPrimList->AddPrimitive(cPrim);
    return 1;
}

CDPrimitive SubQuad(CDPrimitive cPrim, double dt1, double dt2)
{
    CDPrimitive cRes;
    cRes.iType = 4;
    double dt = dt2 - dt1;

    CDPoint cp11 = cPrim.cPt1;
    CDPoint cp12 = 2.0*(cPrim.cPt2 - cPrim.cPt1);
    CDPoint cp13 = cPrim.cPt3 - 2.0*cPrim.cPt2 + cPrim.cPt1;

    CDPoint cp21 = cp11 + dt1*cp12 + Power2(dt1)*cp13;
    CDPoint cp22 = dt*(cp12 + 2.0*dt1*cp13);
    CDPoint cp23 = Power2(dt)*cp13;

    cRes.cPt1 = cp21;
    cRes.cPt2 = cRes.cPt1 + cp22/2.0;
    cRes.cPt3 = cp23 + 2.0*cRes.cPt2 - cRes.cPt1;

    return cRes;
}

CDPrimitive SubBezier(CDPrimitive cPrim, double dt1, double dt2)
{
    CDPrimitive cRes;
    cRes.iType = 5;
    double dt = dt2 - dt1;

    CDPoint cp11 = cPrim.cPt1;
    CDPoint cp12 = 3.0*(cPrim.cPt2 - cPrim.cPt1);
    CDPoint cp13 = 3.0*(cPrim.cPt3 - 2.0*cPrim.cPt2 + cPrim.cPt1);
    CDPoint cp14 = cPrim.cPt4 - 3.0*cPrim.cPt3 + 3.0*cPrim.cPt2 - cPrim.cPt1;

    CDPoint cp21 = cp11 + dt1*cp12 + Power2(dt1)*cp13 + Power3(dt1)*cp14;
    CDPoint cp22 = dt*(cp12 + 2.0*dt1*cp13 + 3.0*Power2(dt1)*cp14);
    CDPoint cp23 = Power2(dt)*(cp13 + 3.0*dt1*cp14);
    CDPoint cp24 = Power3(dt)*cp14;

    cRes.cPt1 = cp21;
    cRes.cPt2 = cRes.cPt1 + cp22/3.0;
    cRes.cPt3 = cp23/3.0 + 2.0*cRes.cPt2 - cRes.cPt1;
    cRes.cPt4 = cp24 + 3.0*cRes.cPt3 - 3.0*cRes.cPt2 + cRes.cPt1;

    return cRes;
}

CDPoint GetBezier(CDPrimitive cPrim, double dt)
{
    double ds = 1.0 - dt;
    CDPoint cRes = Power3(ds)*cPrim.cPt1 + 3.0*Power2(ds)*dt*cPrim.cPt2 +
        3.0*ds*Power2(dt)*cPrim.cPt3 + Power3(dt)*cPrim.cPt4;
    return cRes;
}

CDPoint GetBezierDeriv(CDPrimitive cPrim, double dt)
{
    double ds = 1.0 - dt;
    CDPoint cRes = Power2(ds)*(cPrim.cPt2 - cPrim.cPt1) +
        2.0*ds*dt*(cPrim.cPt3 - cPrim.cPt2) +
        Power2(dt)*(cPrim.cPt4 - cPrim.cPt3);
    return 3.0*cRes;
}

CDPoint GetBezierDeriv2(CDPrimitive cPrim, double dt)
{
    double ds = 1.0 - dt;
    CDPoint cRes = ds*(cPrim.cPt3 - 2.0*cPrim.cPt2 + cPrim.cPt1) +
        dt*(cPrim.cPt4 - 2.0*cPrim.cPt3 + cPrim.cPt2);
    return 6.0*cRes;
}

CDPoint GetBezierDir(CDPrimitive cPrim, double dt)
{
    double ds = 1.0 - dt;
    CDPoint cRes = Power2(ds)*(cPrim.cPt2 - cPrim.cPt1) +
        2.0*ds*dt*(cPrim.cPt3 - cPrim.cPt2) +
        Power2(dt)*(cPrim.cPt4 - cPrim.cPt3);
    double dNorm = GetNorm(cRes);
    if(dNorm > g_dPrec) return cRes/dNorm;
    if(dt < g_dPrec)
    {
        cRes = cPrim.cPt2 - cPrim.cPt1;
        dNorm = GetNorm(cRes);
        if(dNorm > g_dPrec) return cRes/dNorm;
        cRes = cPrim.cPt3 - cPrim.cPt1;
        dNorm = GetNorm(cRes);
        if(dNorm > g_dPrec) return cRes/dNorm;
        cRes = cPrim.cPt4 - cPrim.cPt1;
        dNorm = GetNorm(cRes);
        if(dNorm > g_dPrec) return cRes/dNorm;
    }
    if(ds < g_dPrec)
    {
        cRes = cPrim.cPt4 - cPrim.cPt3;
        dNorm = GetNorm(cRes);
        if(dNorm > g_dPrec) return cRes/dNorm;
        cRes = cPrim.cPt4 - cPrim.cPt2;
        dNorm = GetNorm(cRes);
        if(dNorm > g_dPrec) return cRes/dNorm;
        cRes = cPrim.cPt4 - cPrim.cPt1;
        dNorm = GetNorm(cRes);
        if(dNorm > g_dPrec) return cRes/dNorm;
    }
    return cRes;
}

int CropQuadLeft(CDPrimitive cPrim, CDPoint cPt1, CDPoint cPt2,
    PDPrimObject pPrimList)
{
    CDPoint cDir = cPt2 - cPt1;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return 0;

    CDPoint cN1 = cDir/dNorm;

    CDPoint cCoefs[3];
    cCoefs[0] = cPrim.cPt1;
    cCoefs[1] = cPrim.cPt2;
    cCoefs[2] = cPrim.cPt3;

    CDPoint cptX[2];
    double dts[2];
    int iX = QuadXSeg(cCoefs, cPt1, cPt2, cptX, dts);

    CDPoint cp1 = Rotate(cPrim.cPt1 - cPt1, cN1, false);

    int iRes = 0;

    if(iX < 1)
    {
        if(cp1.y < g_dPrec)
        {
            pPrimList->AddPrimitive(cPrim);
            iRes = 2;
        }
        return iRes;
    }

    int iCurRoot = 0;
    //CDPoint cp2 = Rotate(cPrim.cPt4 - cPt1, cN1, false);
    double dt1 = 0.0;
    double dt2 = dts[iCurRoot++];

    CDPrimitive cPrim1;
    CDPoint cp3, cp4;

    if(dt2 < g_dPrec)
    {
        if(iCurRoot < iX) dt2 = dts[iCurRoot++];
        else dt2 = 1.0;
    }

    cp3 = GetQuadPoint(&cPrim, (dt1 + dt2)/2.0);
    cp4 = Rotate(cp3 - cPt1, cN1, false);

    if(cp4.y < 0)
    {
        cPrim1 = SubQuad(cPrim, dt1, dt2);
        pPrimList->AddPrimitive(cPrim1);
        iRes = 1;
    }

    if(dt2 > 1.0 - g_dPrec) return iRes;

    dt1 = dt2;
    if(iCurRoot < iX) dt2 = dts[iCurRoot++];
    else dt2 = 1.0;

    cp3 = GetQuadPoint(&cPrim, (dt1 + dt2)/2.0);
    cp4 = Rotate(cp3 - cPt1, cN1, false);

    if(cp4.y < 0)
    {
        cPrim1 = SubQuad(cPrim, dt1, dt2);
        pPrimList->AddPrimitive(cPrim1);
        iRes = 1;
    }

    if(dt2 > 1.0 - g_dPrec) return iRes;

    dt1 = dt2;
    if(iCurRoot < iX) dt2 = dts[iCurRoot++];
    else dt2 = 1.0;

    cp3 = GetQuadPoint(&cPrim, (dt1 + dt2)/2.0);
    cp4 = Rotate(cp3 - cPt1, cN1, false);

    if(cp4.y < 0)
    {
        cPrim1 = SubQuad(cPrim, dt1, dt2);
        pPrimList->AddPrimitive(cPrim1);
        iRes = 1;
    }

    return iRes;
}

int CropBezierLeft(CDPrimitive cPrim, CDPoint cPt1, CDPoint cPt2,
    PDPrimObject pPrimList)
{
    CDPoint cDir = cPt2 - cPt1;
    double dNorm = GetNorm(cDir);
    if(dNorm < g_dPrec) return 0;

    CDPoint cN1 = cDir/dNorm;

    CDPoint cptX[3];
    double dts[3];
    int iX = BezXLine(cPrim.cPt1, cPrim.cPt2, cPrim.cPt3, cPrim.cPt4,
        cPt1, cPt2, cptX, dts);

    CDPoint cp1 = Rotate(cPrim.cPt1 - cPt1, cN1, false);

    int iRes = 0;

    if(iX < 1)
    {
        if(cp1.y < g_dPrec)
        {
            pPrimList->AddPrimitive(cPrim);
            iRes = 2;
        }
        return iRes;
    }

    int iCurRoot = 0;
    //CDPoint cp2 = Rotate(cPrim.cPt4 - cPt1, cN1, false);
    double dt1 = 0.0;
    double dt2 = dts[iCurRoot++];

    CDPrimitive cPrim1;
    CDPoint cp3, cp4;

    if(dt2 < g_dPrec)
    {
        if(iCurRoot < iX) dt2 = dts[iCurRoot++];
        else dt2 = 1.0;
    }

    cp3 = GetBezier(cPrim, (dt1 + dt2)/2.0);
    cp4 = Rotate(cp3 - cPt1, cN1, false);

    if(cp4.y < 0)
    {
        cPrim1 = SubBezier(cPrim, dt1, dt2);
        pPrimList->AddPrimitive(cPrim1);
        iRes = 1;
    }

    if(dt2 > 1.0 - g_dPrec) return iRes;

    dt1 = dt2;
    if(iCurRoot < iX) dt2 = dts[iCurRoot++];
    else dt2 = 1.0;

    cp3 = GetBezier(cPrim, (dt1 + dt2)/2.0);
    cp4 = Rotate(cp3 - cPt1, cN1, false);

    if(cp4.y < 0)
    {
        cPrim1 = SubBezier(cPrim, dt1, dt2);
        pPrimList->AddPrimitive(cPrim1);
        iRes = 1;
    }

    if(dt2 > 1.0 - g_dPrec) return iRes;

    dt1 = dt2;
    if(iCurRoot < iX) dt2 = dts[iCurRoot++];
    else dt2 = 1.0;

    cp3 = GetBezier(cPrim, (dt1 + dt2)/2.0);
    cp4 = Rotate(cp3 - cPt1, cN1, false);

    if(cp4.y < 0)
    {
        cPrim1 = SubBezier(cPrim, dt1, dt2);
        pPrimList->AddPrimitive(cPrim1);
        iRes = 1;
    }

    return iRes;
}

int CropPrimitiveByHalfPlaneLeft(CDPrimitive cPrim, CDPoint cPt1, CDPoint cPt2,
    PDPrimObject pPrimList)
{
    switch(cPrim.iType)
    {
    case 1:
        return CropLineLeft(cPrim, cPt1, cPt2, pPrimList);
    case 2:
        return CropArcLeft(cPrim, cPt1, cPt2, pPrimList);
    case 3:
        return CropCircLeft(cPrim, cPt1, cPt2, pPrimList);
    case 4:
        return CropQuadLeft(cPrim, cPt1, cPt2, pPrimList);
    case 5:
        return CropBezierLeft(cPrim, cPt1, cPt2, pPrimList);
    default:
        return 0;
    }
}

int CropPoints(CDPrimitive cPrim, PDRect pRect, PDPrimObject pPrimList)
{
    int iRes = 0;
    CDPrimitive cPrim1;
    if(cPrim.cPt1.x > 0.5)
    {
        if(DPtInDRect(cPrim.cPt2, pRect))
        {
            cPrim1.iType = cPrim.iType;
            cPrim1.cPt1 = cPrim.cPt2;
            pPrimList->AddPrimitive(cPrim1);
            iRes++;
        }
    }
    if(cPrim.cPt1.y > 0.5)
    {
        if(DPtInDRect(cPrim.cPt3, pRect))
        {
            cPrim1.iType = cPrim.iType;
            cPrim1.cPt1 = cPrim.cPt3;
            pPrimList->AddPrimitive(cPrim1);
            iRes++;
        }
    }
    return iRes;
}

int CropPrimitive(CDPrimitive cPrim, PDRect pRect, PDPrimObject pPrimList)
{
    if(cPrim.iType > 5)
        return CropPoints(cPrim, pRect, pPrimList);

    CDPoint cp1, cp2;
    cp1.x = pRect->cPt1.x;
    cp1.y = pRect->cPt1.y;
    cp2.x = pRect->cPt1.x;
    cp2.y = pRect->cPt2.y;

    PDPrimObject pPL1 = new CDPrimObject();
    int iRes1 = CropPrimitiveByHalfPlaneLeft(cPrim, cp1, cp2, pPL1);

    PDPrimObject pPL2 = new CDPrimObject();
    CDPrimitive cPr1;

    cp1.x = pRect->cPt1.x;
    cp1.y = pRect->cPt2.y;
    cp2.x = pRect->cPt2.x;
    cp2.y = pRect->cPt2.y;

    int iRes2 = iRes1;
    int iRes3 = 0;
    int k;

    for(int i = 0; i < pPL1->GetCount(); i++)
    {
        cPr1 = pPL1->GetPrimitive(i);
        k = CropPrimitiveByHalfPlaneLeft(cPr1, cp1, cp2, pPL2);
        if(k < 2) iRes2 = 1;
        if(k > 0) iRes3 = 1;
    }
    iRes1 = iRes2;
    if(iRes3 < 1) iRes1 = 0;

    pPL1->Clear();

    cp1.x = pRect->cPt2.x;
    cp1.y = pRect->cPt2.y;
    cp2.x = pRect->cPt2.x;
    cp2.y = pRect->cPt1.y;

    iRes2 = iRes1;
    iRes3 = 0;

    for(int i = 0; i < pPL2->GetCount(); i++)
    {
        cPr1 = pPL2->GetPrimitive(i);
        k = CropPrimitiveByHalfPlaneLeft(cPr1, cp1, cp2, pPL1);
        if(k < 2) iRes2 = 1;
        if(k > 0) iRes3 = 1;
    }
    iRes1 = iRes2;
    if(iRes3 < 1) iRes1 = 0;

    cp1.x = pRect->cPt2.x;
    cp1.y = pRect->cPt1.y;
    cp2.x = pRect->cPt1.x;
    cp2.y = pRect->cPt1.y;

    iRes2 = iRes1;
    iRes3 = 0;

    for(int i = 0; i < pPL1->GetCount(); i++)
    {
        cPr1 = pPL1->GetPrimitive(i);
        k = CropPrimitiveByHalfPlaneLeft(cPr1, cp1, cp2, pPrimList);
        if(k < 2) iRes2 = 1;
        if(k > 0) iRes3 = 1;
    }
    iRes1 = iRes2;
    if(iRes3 < 1) iRes1 = 0;

    delete pPL2;
    delete pPL1;

    return iRes1;
}

bool IsSameDir(CDPoint cPt1, CDPoint cPt2)
{
	double dVect = fabs(Deter2(cPt1, cPt2));
	double dScal = cPt1*cPt2;
	if(fabs(dScal) < g_dPrec) return(false);
	dVect /= dScal;
	return((dVect < g_dPrec) && (dScal > g_dPrec));
}

double Approx3pt(PDPoint pPoints, PDPoint pStartDir, PDPoint pEndDir, PDPrimitive pPrim)
{
	CDPoint pCt[2];

	double d1 = GetDist(pPoints[0], pPoints[1]);
	double d2 = GetDist(pPoints[1], pPoints[2]);
	if(d1 + d2 < g_dPrec) return(-1.0);

	double t = d1/(d1 + d2);
	if(t < g_dPrec)
    {
        pPrim->iType = 1;
        pPrim->cPt1 = pPoints[1];
        pPrim->cPt2 = pPoints[2];
        pPrim->cPt3 = 0;
        pPrim->cPt4 = 0;
        return 0.0;
    }
	if(t > 1 - g_dPrec)
    {
        pPrim->iType = 1;
        pPrim->cPt1 = pPoints[0];
        pPrim->cPt2 = pPoints[1];
        pPrim->cPt3 = 0;
        pPrim->cPt4 = 0;
        return 0.0;
    }

	double dc[2];
	double dDet;
	int iType = 2;
	if(pStartDir && pEndDir)
	{
		dDet = Deter2(*pStartDir, *pEndDir);
		if(fabs(dDet) < g_dPrec) iType = 3;
		else
		{
			dc[0] = Deter2(pPoints[1] - pPoints[0], *pEndDir)/dDet;
			dc[1] = -Deter2(pPoints[1] - pPoints[0], *pStartDir)/dDet;
			if((dc[0] > g_dPrec) && (dc[1] > g_dPrec))
			{
				pCt[0] = pPoints[0] + dc[0]*(*pStartDir);
				pCt[1] = pPoints[1] - dc[1]*(*pEndDir);
				if(GetDist(pCt[0], pCt[1]) > g_dPrec) iType = 3;
			}
			else iType = 3;
		}
	}
	else
	{
		pCt[0] = (pPoints[1] - Power2(1 - t)*pPoints[0] -
			Power2(t)*pPoints[2])/2.0/t/(1 - t);
	}

    if(iType < 3)
    {
        /*CDPoint pCv[3];
        pCv[0] = pPoints[0];
        pCv[1] = pCt[0];
        pCv[2] = pPoints[2];

        pPrim->iType = 4;
        pPrim->cPt1 = pCv[0];
        pPrim->cPt2 = pCv[0]/3.0 + 2.0*pCv[1]/3.0;
        pPrim->cPt3 = 2.0*pCv[1]/3.0 + pCv[2]/3.0;
        pPrim->cPt4 = pCv[2];*/
        pPrim->iType = 4;
        pPrim->cPt1 = pPoints[0];
        pPrim->cPt2 = pCt[0];
        pPrim->cPt3 = pPoints[2];
        return 0.0;
    }

    dDet = Deter2(*pEndDir, *pStartDir);
    if(fabs(dDet) < g_dPrec) return(-1.0);

    double dt1 = t*Power2(1 - t);
    double dt2 = Power2(t)*(1 - t);

    CDPoint cPt = (pPoints[1] - Power3(1 - t)*pPoints[0] -
        Power3(t)*pPoints[2])/3.0 - dt1*pPoints[0] - dt2*pPoints[2];
    dc[0] = Deter2(*pEndDir, cPt)*dt2/dDet;
    dc[1] = Deter2(*pStartDir, cPt)*dt1/dDet;

    if((dc[0] > g_dPrec) && (dc[1] > g_dPrec))
    {
        pCt[0] = pPoints[0] + dc[0]*(*pStartDir);
        pCt[1] = pPoints[1] - dc[1]*(*pEndDir);
    }
    else return(-1.0);

    pPrim->iType = 5;
    pPrim->cPt1 = pPoints[0];
    pPrim->cPt2 = pCt[0];
    pPrim->cPt3 = pCt[1];
    pPrim->cPt4 = pPoints[2];
	return(0.0);
}

double Approx4pt(PDPoint pPoints, PDPrimitive pPrim)
{
	double d1 = GetDist(pPoints[0], pPoints[1]);
	double d2 = GetDist(pPoints[1], pPoints[2]);
	double d3 = GetDist(pPoints[2], pPoints[3]);
	double d = d1 + d2 + d3;
	if(d < g_dPrec) return(-1.0);

	double t1 = d1/d;
	double t2 = (d1 + d2)/d;

	if(t1 < g_dPrec)
		return(Approx3pt(&pPoints[1], NULL, NULL, pPrim));

	if(t2 > 1 - g_dPrec)
		return(Approx3pt(pPoints, NULL, NULL, pPrim));

	if(fabs(t2 - t1) < g_dPrec)
	{
		CDPoint plPts[3];
		plPts[0] = pPoints[0];
		plPts[1] = pPoints[1];
		plPts[2] = pPoints[3];
		return(Approx3pt(plPts, NULL, NULL, pPrim));
	}

	double b[2] = {pPoints[1].x - pPoints[0].x*Power3(1 - t1) -
		pPoints[3].x*Power3(t1),
		pPoints[2].x - pPoints[0].x*Power3(1 - t2) - pPoints[3].x*Power3(t2)};
	double m[4] = {3.0*t1*Power2(1 - t1), 3.0*Power2(t1)*(1 - t1),
		3.0*t2*Power2(1 - t2), 3.0*Power2(t2)*(1 - t2)};
	d = m[0]*m[3] - m[1]*m[2];

	CDPoint cPt[2];

	cPt[0].x = (m[3]*b[0] - m[1]*b[1])/d;
	cPt[1].x = (m[0]*b[1] - m[2]*b[0])/d;

	b[0] = pPoints[1].y - pPoints[0].y*Power3(1 - t1) - pPoints[3].y*Power3(t1);
	b[1] = pPoints[2].y - pPoints[0].y*Power3(1 - t2) - pPoints[3].y*Power3(t2);

	cPt[0].y = (m[3]*b[0] - m[1]*b[1])/d;
	cPt[1].y = (m[0]*b[1] - m[2]*b[0])/d;

    pPrim->iType = 5;
    pPrim->cPt1 = pPoints[0];
    pPrim->cPt2 = cPt[0];
    pPrim->cPt3 = cPt[1];
    pPrim->cPt4 = pPoints[3];
	return(0.0);
}

double ApproxNptFixT(int n, PDPoint pPoints, double *pt, PDPrimitive pPrim)
{
	double b[2] = {0, 0};
	double m[4] = {0, 0, 0, 0};
	for(int i = 0; i < n - 2; i++)
	{
		b[0] += (pPoints[i + 1].x - pPoints[0].x*Power3(1 - pt[i]) -
			pPoints[n - 1].x*Power3(pt[i]))*pt[i]*Power2(1 - pt[i]);
		b[1] += (pPoints[i + 1].x - pPoints[0].x*Power3(1 - pt[i]) -
			pPoints[n - 1].x*Power3(pt[i]))*Power2(pt[i])*(1 - pt[i]);

		m[0] += Power2(pt[i])*Power4(1 - pt[i]);
		m[1] += Power3(pt[i])*Power3(1 - pt[i]);
		m[3] += Power4(pt[i])*Power2(1 - pt[i]);
	}
	m[0] *= 3.0;
	m[1] *= 3.0;
	m[3] *= 3.0;
	m[2] = m[1];

	double d = m[0]*m[3] - m[1]*m[2];
	if(fabs(d) < g_dCrossPrec) return(-1.0);

	CDPoint cPt[2];
	cPt[0].x = (m[3]*b[0] - m[1]*b[1])/d;
	cPt[1].x = (m[0]*b[1] - m[2]*b[0])/d;

	b[0] = 0;
	b[1] = 0;
	for(int i = 0; i < n - 2; i++)
	{
		b[0] += (pPoints[i + 1].y - pPoints[0].y*Power3(1 - pt[i]) -
			pPoints[n - 1].y*Power3(pt[i]))*pt[i]*Power2(1 - pt[i]);
		b[1] += (pPoints[i + 1].y - pPoints[0].y*Power3(1 - pt[i]) -
			pPoints[n - 1].y*Power3(pt[i]))*Power2(pt[i])*(1 - pt[i]);
	}

	cPt[0].y = (m[3]*b[0] - m[1]*b[1])/d;
	cPt[1].y = (m[0]*b[1] - m[2]*b[0])/d;

    pPrim->iType = 5;
    pPrim->cPt1 = pPoints[0];
    pPrim->cPt2 = cPt[0];
    pPrim->cPt3 = cPt[1];
    pPrim->cPt4 = pPoints[n - 1];

	d = 0;
	for(int i = 0; i < n - 2; i++)
	{
		cPt[0] = GetBezier(*pPrim, pt[i]) - pPoints[i + 1];
		d += cPt[0]*cPt[0];
	}

	return(d);
}

double ApproxNptFixTDir(int n, PDPoint pPoints, CDPoint cStartDir,
	CDPoint cEndDir, double *pt, PDPrimitive pPrim)
{
	double b[2] = {0, 0};
	double m[4] = {0, 0, 0, 0};
	for(int i = 0; i < n - 2; i++)
	{
		b[0] += pt[i]*Power2(1 - pt[i])*(
			cStartDir.x*(pPoints[i + 1].x -
			pPoints[0].x*Power2(1 - pt[i])*(1.0 + 2.0*pt[i]) -
			pPoints[n - 1].x*Power2(pt[i])*(3.0 - 2.0*pt[i])) +
			cStartDir.y*(pPoints[i + 1].y -
			pPoints[0].y*Power2(1 - pt[i])*(1.0 + 2.0*pt[i]) -
			pPoints[n - 1].y*Power2(pt[i])*(3.0 - 2.0*pt[i]))
			);
		b[1] += Power2(pt[i])*(1 - pt[i])*(
			cEndDir.x*(pPoints[i + 1].x -
			pPoints[0].x*Power2(1 - pt[i])*(1.0 + 2.0*pt[i]) -
			pPoints[n - 1].x*Power2(pt[i])*(3.0 - 2.0*pt[i])) +
			cEndDir.y*(pPoints[i + 1].y -
			pPoints[0].y*Power2(1 - pt[i])*(1.0 + 2.0*pt[i]) -
			pPoints[n - 1].y*Power2(pt[i])*(3.0 - 2.0*pt[i])));

		m[0] += Power2(pt[i])*Power4(1 - pt[i]);
		m[1] += Power3(pt[i])*Power3(1 - pt[i]);
		m[3] += Power4(pt[i])*Power2(1 - pt[i]);
	}
	m[0] *= 3.0*cStartDir*cStartDir;
	m[1] *= 3.0*cStartDir*cEndDir;
	m[3] *= 3.0*cEndDir*cEndDir;
	m[2] = m[1];

	double d = m[0]*m[3] - m[1]*m[2];
	if(fabs(d) < g_dCrossPrec) return(-1.0);

	CDPoint cPt[2];
	double u = (m[3]*b[0] - m[1]*b[1])/d;
	double v = (m[0]*b[1] - m[2]*b[0])/d;
	cPt[0] = pPoints[0] + u*cStartDir;
	cPt[1] = pPoints[n - 1] + v*cEndDir;

    pPrim->iType = 5;
    pPrim->cPt1 = pPoints[0];
    pPrim->cPt2 = cPt[0];
    pPrim->cPt3 = cPt[1];
    pPrim->cPt4 = pPoints[n - 1];

	d = 0;
	for(int i = 0; i < n - 2; i++)
	{
		cPt[0] = GetBezier(*pPrim, pt[i]) - pPoints[i + 1];
		d += cPt[0]*cPt[0];
	}

	return(d);
}

double ApproxNpt(int n, PDPoint pPoints, PDPoint pStartDir,
	PDPoint pEndDir, PDPrimitive pPrim)
{
	double *pd = (double*)malloc((n - 1)*sizeof(double));
	double d = 0;
	for(int i = 0; i < n - 1; i++)
	{
		pd[i] = GetDist(pPoints[i], pPoints[i + 1]);
		d += pd[i];
	}
	if(d < g_dPrec) return(-1.0);

	double d1 = 0;
	//double *pt = (double*)malloc((n - 2)*sizeof(double));
	// use pd instead
	double *pt = pd;
	for(int i = 0; i < n - 2; i++)
	{
		d1 += pd[i];
		pt[i] = d1/d;
	}

	if(pStartDir && pEndDir)
		d = ApproxNptFixTDir(n, pPoints, *pStartDir, *pEndDir, pt, pPrim);
	else d = ApproxNptFixT(n, pPoints, pt, pPrim);

	int k = 0;
	const int nIter = 4;//2;
	double h1, h2;
	CDPoint fPt, d1Pt, d2Pt;

	while((d > g_dRootPrec) && (k < nIter))
	{
		for(int i = 0; i < n - 2; i++)
		{
			fPt = GetBezier(*pPrim, pt[i]);
			d1Pt = GetBezierDeriv(*pPrim, pt[i]);
			d2Pt = GetBezierDeriv2(*pPrim, pt[i]);

            h1 = (fPt - pPoints[i + 1])*d1Pt;
            h2 = d1Pt*d1Pt + (fPt - pPoints[i + 1])*d2Pt;

			if(fabs(h2) > g_dPrec) pt[i] -= h1/h2;
			if(pt[i] < 0.0) pt[i] = 0.0;
			if(pt[i] > 1.0) pt[i] = 1.0;
		}

		if(pStartDir && pEndDir)
			d = ApproxNptFixTDir(n, pPoints, *pStartDir, *pEndDir, pt, pPrim);
		else
			d = ApproxNptFixT(n, pPoints, pt, pPrim);
		k++;
	}

	free(pd);

	return(d);
}

double ApproxLineSeg(int iPoints, PDPoint pPoints, PDPoint pStartDir,
	PDPoint pEndDir, PDPrimitive pPrim)
{
	if(iPoints < 2) return(-1.0);
	if(iPoints == 2)
    {
        pPrim->iType = 1;
        pPrim->cPt1 = pPoints[0];
        pPrim->cPt2 = pPoints[1];
        pPrim->cPt3 = 0;
        pPrim->cPt4 = 0;
        return 0.0;
    }

	// first of all try whether the segment is not a straight line
	double dDet = GetDist(pPoints[0], pPoints[iPoints - 1]);
	if(dDet > g_dPrec)
	{
		double dSum = 0.0, t;
		CDPoint cPt;
		int i = 1;
		int n = iPoints - 1;
		double dnPrec = (double)n*g_dPrec*20.0;
		while((dSum < dnPrec) && (i < n))
		{
			t = (pPoints[i] - pPoints[0])*(pPoints[n] - pPoints[0])/dDet;
			cPt = (1.0 - t)*pPoints[0] + t*pPoints[n];
			dSum += GetDist(cPt, pPoints[i]);
			i++;
		}

		if(dSum < dnPrec)
		{
			if(pStartDir && pEndDir)
			{
				cPt = pPoints[n] - pPoints[0];
				if(!IsSameDir(*pStartDir, cPt) || !IsSameDir(*pEndDir, cPt))
					dSum = 1.0;
			}
		}

		if(dSum < dnPrec)
		{
            pPrim->iType = 1;
            pPrim->cPt1 = pPoints[0];
            pPrim->cPt2 = pPoints[n];
            pPrim->cPt3 = 0;
            pPrim->cPt4 = 0;
			return(dSum/n);
		}
	}

	if(iPoints == 3)
	{
		return(Approx3pt(pPoints, pStartDir, pEndDir, pPrim));
	}

	if((iPoints == 4) && (!pStartDir || !pEndDir))
	{
		return(Approx4pt(pPoints, pPrim));
	}

	return(ApproxNpt(iPoints, pPoints, pStartDir, pEndDir, pPrim));
}

bool PointInArc(CDPoint cPt, CDLine cStart, CDLine cEnd)
{
    CDPoint cOrig, cPt1, cPt2;
    if(cStart.bIsSet && cEnd.bIsSet)
    {
        int iX = LineXLine(cStart.cOrigin, cStart.cDirection,
            cEnd.cOrigin, cEnd.cDirection, &cOrig);

        if(iX < 1) // lines are parallel
        {
            cPt2 = Rotate(cEnd.cDirection, cStart.cDirection, false);
            if(cPt2.x > 0)
            {
                cPt1 = Rotate(cPt - cStart.cOrigin, cStart.cDirection, false);
                if(cPt1.y > 0) return cPt1.y < cPt2.y;
                return cPt1.y > cPt2.y;
            }
            else cOrig = (cStart.cOrigin + cEnd.cOrigin)/2.0;
        }

        double dt;
        if(fabs(cStart.cDirection.x) > g_dPrec)
            dt = (cOrig.x - cStart.cOrigin.x)/cStart.cDirection.x;
        else
            dt = (cOrig.y - cStart.cOrigin.y)/cStart.cDirection.y;

        if(dt > 0)
        {
            CDLine cTmpLine = cStart;
            cStart.cOrigin = cEnd.cOrigin;
            cStart.cDirection = -1.0*cEnd.cDirection;
            cEnd.cOrigin = cTmpLine.cOrigin;
            cEnd.cDirection = -1.0*cTmpLine.cDirection;
        }

        cPt1 = Rotate(cPt - cOrig, cStart.cDirection, false);
        cPt2 = Rotate(cEnd.cDirection, cStart.cDirection, false);

        double d1 = GetNorm(cPt1);

        if(d1 < g_dPrec) return true;

        int iRes = CmpAngle(cPt1/d1, cPt2);
        return (iRes <= 0);
    }
    else if(cStart.bIsSet)
    {
        cPt1 = Rotate(cPt - cStart.cOrigin, cStart.cDirection, false);
        return cPt1.y >= 0;
    }
    else if(cEnd.bIsSet)
    {
        cPt1 = Rotate(cPt - cEnd.cOrigin, cEnd.cDirection, false);
        return cPt1.y <= 0;
    }
    return true;
}

CDPoint GetQuadPoint(PDPrimitive pQuad, double dt)
{
	return Power2(1.0 - dt)*pQuad->cPt1 + 2.0*dt*(1.0 - dt)*pQuad->cPt2 + Power2(dt)*pQuad->cPt3;
}

CDPoint GetQuadDir(PDPrimitive pQuad, double dt)
{
    CDPoint cRes = {0, 0};
    CDPoint cDeriv = (1.0 - dt)*(pQuad->cPt2 - pQuad->cPt1) + dt*(pQuad->cPt3 - pQuad->cPt2);
    double dNorm = GetNorm(cDeriv);
    if(dNorm < g_dPrec)
    {
        cDeriv = pQuad->cPt2 - pQuad->cPt1;
        dNorm = GetNorm(cDeriv);
    }
    if(dNorm < g_dPrec)
    {
        cDeriv = pQuad->cPt3 - pQuad->cPt2;
        dNorm = GetNorm(cDeriv);
    }
    if(dNorm < g_dPrec) return cRes;

    cRes = cDeriv/dNorm;
    return cRes;
}

CDPoint GetQuadNormal(PDPrimitive pQuad, double dt)
{
    CDPoint cRes = {0, 0};
    CDPoint cDeriv = (1.0 - dt)*(pQuad->cPt2 - pQuad->cPt1) + dt*(pQuad->cPt3 - pQuad->cPt2);
    double dNorm = GetNorm(cDeriv);
    if(dNorm < g_dPrec)
    {
        cDeriv = pQuad->cPt2 - pQuad->cPt1;
        dNorm = GetNorm(cDeriv);
    }
    if(dNorm < g_dPrec)
    {
        cDeriv = pQuad->cPt3 - pQuad->cPt2;
        dNorm = GetNorm(cDeriv);
    }
    if(dNorm < g_dPrec) return cRes;

    cRes = GetNormal(cDeriv/dNorm);
    return cRes;
}

double LenInt(double x)
{
	double y = sqrt(1 + x*x);
	return log(x + y) + x*y;
}

double GetQuadLength(PDPrimitive pQuad, double t1, double t2)
{
	CDPoint cPt1 = 2.0*(pQuad->cPt2 - pQuad->cPt1);
	CDPoint cPt2 = 2.0*(pQuad->cPt3 - pQuad->cPt2);
	CDPoint cA = cPt2 - cPt1;
	CDPoint cB = cPt1;

	double dx1 = cA*cA;
	double dx2 = cB*cB;
	double dx12 = cA*cB;
	double dDet = dx1*dx2 - Power2(dx12); // always >= 0 from Schwarz inequality

	double dRes = 0.0;

	if(dDet > g_dPrec)
	{
		double dA = sqrt(dDet);
		double v1 = (dx1*t1 + dx12)/dA;
		double v2 = (dx1*t2 + dx12)/dA;
		dRes = (LenInt(v2) - LenInt(v1))*dDet/2.0/dx1/sqrt(dx1);
        if(dRes < g_dPrec) dRes = 0.0;
	}
	else
	{
		if(dx1 < g_dPrec)
		{
			dRes = sqrt(dx2)*(t2 - t1);
		}
		else
		{
			dx2 = sqrt(dx1);
			dRes = (t2 - t1)*(dx2*(t2 + t1)/2.0 + dx12/dx2);
            if(dRes < g_dPrec) dRes = 0.0;
		}
	}

	return(dRes);
}

double GetQuadLengthDeriv(PDPrimitive pQuad, double t2)
{
	CDPoint cPt1 = 2.0*(pQuad->cPt2 - pQuad->cPt1);
	CDPoint cPt2 = 2.0*(pQuad->cPt3 - pQuad->cPt2);
	CDPoint cPt3 = (1.0 - t2)*cPt1 + t2*cPt2;
	return GetNorm(cPt3);
}

double GetQuadPointAtDist(PDPrimitive pQuad, double t1, double dDist)
{
	CDPoint cPt1 = 2.0*(pQuad->cPt2 - pQuad->cPt1);
	CDPoint cPt2 = 2.0*(pQuad->cPt3 - pQuad->cPt2);
	CDPoint cA = cPt2 - cPt1;
	CDPoint cB = cPt1;

	double dx1 = cA*cA;
	double dx2 = cB*cB;
	double dx12 = cA*cB;
	double dDet = dx1*dx2 - Power2(dx12); // always >= 0 from Schwarz inequality

	double dRes = 0.0;
    double a0, a1, a2; //, a3, a4;

	double dCoefs[5];
	double dRoots[4];
	int iRoots;

	if(dDet > g_dPrec)
	{
		double dA = sqrt(dDet);
		double v1 = (dx1*t1 + dx12)/dA;
		//double v2 = (dx1*t2 + dx12)/dA;
		//dDist = (LenInt(v2) - LenInt(v1))*dDet/2.0/dx1/sqrt(dx1);
		//LenInt(v2) = 2.0*dx1*sqrt(dx1)*dDist/dDet + LenInt(v1);
		double dB = 2.0*dx1*sqrt(dx1)*dDist/dDet + LenInt(v1);

		dCoefs[0] = -Power2(dB);
		dCoefs[1] = 2.0*dB;
		dCoefs[2] = 0.0;
		dCoefs[3] = 0.0;
		dCoefs[4] = 1.0;

		iRoots = SolvePolynom(4, dCoefs, dRoots);

        a1 = GetQuadLength(pQuad, t1, 1.0);
		dRes = t1 + dDist/a1;
        for(int i = 0; i < iRoots; i++)
		{
            a0 = (dRoots[i]*dA - dx12)/dx1;
            if(a0 > t1 - g_dPrec)
            {
			    a2 = GetQuadLength(pQuad, t1, a0);
			    if(fabs(dDist - a2) < fabs(dDist - a1))
			    {
				    a1 = a2;
				    dRes = a0;
			    }
            }
		}

		// we believe we are pretty close to the solution at the moment
		// so we only perform three iterations
		for(int i = 0; i < 3; i++)
		{
			a1 = GetQuadLength(pQuad, t1, dRes) - dDist;
			a2 = GetQuadLengthDeriv(pQuad, dRes);
			if(fabs(a2) > g_dPrec)
			{
				dRes -= a1/a2;
			}
		}
	}
	else
	{
		if(dx1 < g_dPrec)
		{
			if(dx2 > g_dPrec) dRes = t1 + dDist/sqrt(dx2);
		}
		else
		{
			dx2 = sqrt(dx1);
			//dRes = (t2 - t1)*(dx2*(t2 + t1)/2.0 + dx12/dx2);

			a0 = dx2/2.0;
			a1 = dx12/dx2;
			a2 = -dDist - a0*t1*t1 - a1*t1;

			dCoefs[2] = dx2/2.0;
			dCoefs[1] = dx12/dx2;
			dCoefs[0] = -dDist - a0*t1*t1 - a1*t1;

			iRoots = SolvePolynom(2, dCoefs, dRoots);

			if(iRoots > 0)
			{
				dRes = dRoots[0];
				if(iRoots > 1)
				{
					a1 = GetQuadLength(pQuad, t1, dRes);
					a2 = GetQuadLength(pQuad, t1, dRoots[1]);
					if(fabs(dDist - a2) < fabs(dDist - a1))
						dRes = dRoots[1];
				}
			}
		}
	}

	return(dRes);
}

int RefInBounds(double da1, double da2, double dRef)
{
    bool bCycle = da2 < da1 - g_dPrec;
    bool bInside;

    if(bCycle) bInside = (da1 + g_dPrec < dRef) || (dRef < da2 - g_dPrec);
    else bInside = (da1 + g_dPrec < dRef) && (dRef < da2 - g_dPrec);

    if(bInside) return 3;

    if((da2 - g_dPrec < dRef) && (dRef < da2 + g_dPrec)) return 2;
    if((da1 - g_dPrec < dRef) && (dRef < da1 + g_dPrec)) return 1;

    return 0;
}

int RefInOpenBounds(PDRefPoint pBounds, double dRef)
{
    if(pBounds[0].bIsSet && pBounds[1].bIsSet) return RefInBounds(pBounds[0].dRef, pBounds[1].dRef, dRef);
    if(pBounds[0].bIsSet)
    {
        if(pBounds[0].dRef > dRef + g_dPrec) return 0;
        if(pBounds[0].dRef > dRef - g_dPrec) return 1;
        return 3;
    }
    if(pBounds[1].bIsSet)
    {
        if(pBounds[1].dRef < dRef - g_dPrec) return 0;
        if(pBounds[1].dRef < dRef + g_dPrec) return 2;
    }
    return 3;
}

int MergeBounds(double da1, double da2, double db1, double db2, bool bFullCycle, double *pdBnds)
{
    if(bFullCycle)
    {
        pdBnds[0] = db1;
        pdBnds[1] = db2;
        return 1;
    }

    int ia1 = RefInBounds(db1, db2, da1);
    int ia2 = RefInBounds(db1, db2, da2);
    int ib1 = RefInBounds(da1, da2, db1);
    int ib2 = RefInBounds(da1, da2, db2);

    int iSum = ia1 + ia2 + ib1 + ib2;
    if(iSum < 4) return 0;
    if(iSum > 10)
    {
        pdBnds[0] = da1;
        pdBnds[1] = db2;
        pdBnds[2] = db1;
        pdBnds[3] = da2;
        return 2;
    }

    if((ia1 < 3) && (ia2 < 3) && (ib1 < 3) && (ib2 < 3))
    {
        if(ia1 > 1) return 0;

        pdBnds[0] = db1;
        pdBnds[1] = db2;
        return 1;
    }

    if(ia1 > 2) pdBnds[0] = da1;
    if(ib1 > 2) pdBnds[0] = db1;
    if(ia2 > 2) pdBnds[1] = da2;
    if(ib2 > 2) pdBnds[1] = db2;

    if(iSum == 5) pdBnds[0] = db1;
    if(iSum == 7) pdBnds[1] = db2;

    return 1;
}

int AddBoundCurve(double da, double db, double dr, CurveFunc pFunc, CurveFunc pFuncDer,
    double dt1, double dt2, CDPoint cOrig, CDPoint cMainDir, PDRect pRect,
    PDPrimObject pPrimList)
{
    double dBase, dStart;

    bool bBothSides = false;
    double dDir = 1.0;

    if(dt1 > g_dPrec)
    {
        dBase = dt1;
        dStart = dt1;
    }
    else if(dt2 < -g_dPrec)
    {
        dBase = dt2;
        dStart = dt2;
        dDir = -1.0;
    }
    else if(dt1 > -g_dPrec)
    {
        if(dt2 < g_dPrec) return 0;
        dBase = 1.0;
        dStart = 0.0;
    }
    else if(dt2 < g_dPrec)
    {
        if(dt1 > -g_dPrec) return 0;
        dBase = -1.0;
        dStart = 0.0;
        dDir = -1.0;
    }
    else
    {
        bBothSides = true;
        dBase = -1.0;
        dStart = 0.0;
        dDir = -1.0;
    }

    CDPoint cHypPts[5];
    double du, dv, dNorm;
    CDPoint cDir1, cDir2;
    int iFinished = 0;
    CDPrimitive cPrim, cTmpPrim;

    int iRes = -1;
    int k;

    while(iFinished < 1)
    {
        if(dDir > 0)
        {
            if(dStart + dBase > dt2 - g_dPrec)
            {
                dBase = dt2 - dStart;
                if(dBase < g_dPrec) iFinished = 2;
                else iFinished = 1;
            }
        }
        else
        {
            if(dStart + dBase < dt1 + g_dPrec)
            {
                dBase = dt1 - dStart;
                if(dBase > g_dPrec) iFinished = 2;
                else iFinished = 1;
            }
        }

        if(iFinished < 2)
        {
            for(int j = 0; j < 5; j++)
            {
                dv = sqrt(j/4.0);
                du = dStart + dBase*dv;

                cHypPts[j] = pFunc(da, db, du);
                cDir1 = pFuncDer(da, db, du);
                dNorm = GetNorm(cDir1);

                cHypPts[j].x += dr*cDir1.y/dNorm;
                cHypPts[j].y -= dr*cDir1.x/dNorm;
            }

            cDir2 = cDir1;
            cDir1 = pFuncDer(da, db, dStart);

            if(ApproxLineSeg(5, cHypPts, &cDir1, &cDir2, &cTmpPrim) > -0.5)
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
                cPrim.cPt1 = cOrig + Rotate(cHypPts[0], cMainDir, true);
                cPrim.cPt2 = cOrig + Rotate(cHypPts[4], cMainDir, true);
            }

            k = CropPrimitive(cPrim, pRect, pPrimList);
            if(iRes < 0) iRes = k;
            else if(iRes != k) iRes = 1;

            dStart += dBase;
            dBase *= 2.0;
        }
    }

    if(!bBothSides) return iRes;

    dBase = 1.0;
    dStart = 0.0;
    iFinished = 0;

    while(iFinished < 1)
    {
        if(dStart + dBase > dt2 - g_dPrec)
        {
            dBase = dt2 - dStart;
            if(dBase < g_dPrec) iFinished = 2;
            else iFinished = 1;
        }

        if(iFinished < 2)
        {
            for(int j = 0; j < 5; j++)
            {
                dv = sqrt(j/4.0);
                du = dStart + dBase*dv;

                cHypPts[j] = pFunc(da, db, du);
                cDir1 = pFuncDer(da, db, du);
                dNorm = GetNorm(cDir1);

                cHypPts[j].x += dr*cDir1.y/dNorm;
                cHypPts[j].y -= dr*cDir1.x/dNorm;
            }

            cDir2 = cDir1;
            cDir1 = pFuncDer(da, db, dStart);

            if(ApproxLineSeg(5, cHypPts, &cDir1, &cDir2, &cTmpPrim) > -0.5)
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
                cPrim.cPt1 = cOrig + Rotate(cHypPts[0], cMainDir, true);
                cPrim.cPt2 = cOrig + Rotate(cHypPts[4], cMainDir, true);
            }

            k = CropPrimitive(cPrim, pRect, pPrimList);
            if(iRes < 0) iRes = k;
            else if(iRes != k) iRes = 1;

            dStart += dBase;
            dBase *= 2.0;
        }
    }
    return iRes;
}

int AddBoundQuadCurve(double da, double db, double dr, CurveFunc pFunc, CurveFunc pFuncDer,
    double dt1, double dt2, CDPoint cOrig, CDPoint cMainDir, PDRect pRect,
    PDPrimObject pPrimList)
{
    double dBase, dStart;

    bool bBothSides = false;
    double dDir = 1.0;

    if(dt1 > g_dPrec)
    {
        dBase = dt1;
        dStart = dt1;
    }
    else if(dt2 < -g_dPrec)
    {
        dBase = dt2;
        dStart = dt2;
        dDir = -1.0;
    }
    else if(dt1 > -g_dPrec)
    {
        if(dt2 < g_dPrec) return 0;
        dBase = 1.0;
        dStart = 0.0;
    }
    else if(dt2 < g_dPrec)
    {
        if(dt1 > -g_dPrec) return 0;
        dBase = -1.0;
        dStart = 0.0;
        dDir = -1.0;
    }
    else
    {
        bBothSides = true;
        dBase = -1.0;
        dStart = 0.0;
        dDir = -1.0;
    }

    double du, dNorm;
    CDPoint cDir1, cDir2;
    int iFinished = 0;
    CDPrimitive cPrim, cTmpPrim;

    int iRes = -1;
    int k;

    while(iFinished < 1)
    {
        if(dDir > 0)
        {
            if(dStart + dBase > dt2 - g_dPrec)
            {
                dBase = dt2 - dStart;
                if(dBase < g_dPrec) iFinished = 2;
                else iFinished = 1;
            }
        }
        else
        {
            if(dStart + dBase < dt1 + g_dPrec)
            {
                dBase = dt1 - dStart;
                if(dBase > g_dPrec) iFinished = 2;
                else iFinished = 1;
            }
        }

        if(iFinished < 2)
        {
            du = dStart;
            cDir1 = pFuncDer(da, db, du);
            dNorm = GetNorm(cDir1);
            cTmpPrim.cPt1 = pFunc(da, db, du);
            cTmpPrim.cPt1.x += dr*cDir1.y/dNorm;
            cTmpPrim.cPt1.y -= dr*cDir1.x/dNorm;

            du = dStart + dBase;
            cDir2 = pFuncDer(da, db, du);
            dNorm = GetNorm(cDir2);
            cTmpPrim.cPt3 = pFunc(da, db, du);
            cTmpPrim.cPt3.x += dr*cDir2.y/dNorm;
            cTmpPrim.cPt3.y -= dr*cDir2.x/dNorm;

            LineXLine(cTmpPrim.cPt1, cDir1, cTmpPrim.cPt3, cDir2, &cTmpPrim.cPt2);

            cPrim.iType = 4;
            cPrim.cPt1 = cOrig + Rotate(cTmpPrim.cPt1, cMainDir, true);
            cPrim.cPt2 = cOrig + Rotate(cTmpPrim.cPt2, cMainDir, true);
            cPrim.cPt3 = cOrig + Rotate(cTmpPrim.cPt3, cMainDir, true);

            k = CropPrimitive(cPrim, pRect, pPrimList);
            if(iRes < 0) iRes = k;
            else if(iRes != k) iRes = 1;

            dStart += dBase;
            dBase *= 2.0;
        }
    }

    if(!bBothSides) return iRes;

    dBase = 1.0;
    dStart = 0.0;
    iFinished = 0;

    while(iFinished < 1)
    {
        if(dStart + dBase > dt2 - g_dPrec)
        {
            dBase = dt2 - dStart;
            if(dBase < g_dPrec) iFinished = 2;
            else iFinished = 1;
        }

        if(iFinished < 2)
        {
            du = dStart;
            cDir1 = pFuncDer(da, db, du);
            dNorm = GetNorm(cDir1);
            cTmpPrim.cPt1 = pFunc(da, db, du);
            cTmpPrim.cPt1.x += dr*cDir1.y/dNorm;
            cTmpPrim.cPt1.y -= dr*cDir1.x/dNorm;

            du = dStart + dBase;
            cDir2 = pFuncDer(da, db, du);
            dNorm = GetNorm(cDir2);
            cTmpPrim.cPt3 = pFunc(da, db, du);
            cTmpPrim.cPt3.x += dr*cDir2.y/dNorm;
            cTmpPrim.cPt3.y -= dr*cDir2.x/dNorm;

            LineXLine(cTmpPrim.cPt1, cDir1, cTmpPrim.cPt3, cDir2, &cTmpPrim.cPt2);

            cPrim.iType = 4;
            cPrim.cPt1 = cOrig + Rotate(cTmpPrim.cPt1, cMainDir, true);
            cPrim.cPt2 = cOrig + Rotate(cTmpPrim.cPt2, cMainDir, true);
            cPrim.cPt3 = cOrig + Rotate(cTmpPrim.cPt3, cMainDir, true);

            k = CropPrimitive(cPrim, pRect, pPrimList);
            if(iRes < 0) iRes = k;
            else if(iRes != k) iRes = 1;

            dStart += dBase;
            dBase *= 2.0;
        }
    }
    return iRes;
}

CDPoint GetCurveRefAtDist(double da, double db, double dr, double dBreak, double dDist,
    CurveFunc pFunc, CurveFunc pFuncDer, PDRefPoint pBounds)
{
    pBounds[0].bIsSet = true;
    pBounds[1].bIsSet = true;

    double dBase = 0.5;

    CDPrimitive cQuad;
    cQuad.cPt3 = pFunc(da, db, 0.0);

    CDPoint cDir1, cDir2;
    cDir2 = pFuncDer(da, db, 0.0);
    double d1 = GetNorm(cDir2);

    cQuad.cPt3.x += dr*cDir2.y/d1;
    cQuad.cPt3.y -= dr*cDir2.x/d1;

    bool bFound = false;
    double dt = 0.0;

    if(dBreak > g_dPrec)
    {
        int i = 0;
        int iSteps = (int)dBreak + 1;

        while(!bFound && (i < iSteps))
        {
            cDir1 = cDir2;
            cQuad.cPt1 = cQuad.cPt3;

            dt = (double)(1.0 + i)*dBreak/iSteps;
            cDir2 = pFuncDer(da, db, dt);
            d1 = GetNorm(cDir2);

            cQuad.cPt3 = pFunc(da, db, dt);
            cQuad.cPt3.x += dr*cDir2.y/d1;
            cQuad.cPt3.y -= dr*cDir2.x/d1;

            LineXLine(cQuad.cPt1, cDir1, cQuad.cPt3, cDir2, &cQuad.cPt2);
            d1 = GetQuadLength(&cQuad, 0.0, 1.0);

            if(d1 < dDist) dDist -= d1;
            else
            {
                bFound = true;
                pBounds[0].dRef = (double)i*dBreak/iSteps;
                pBounds[1].dRef = dt;
            }
            i++;
        }
        dBase += dt;
    }

    while(!bFound)
    {
        cDir1 = cDir2;
        cQuad.cPt1 = cQuad.cPt3;

        cDir2 = pFuncDer(da, db, dBase);
        d1 = GetNorm(cDir2);

        cQuad.cPt3 = pFunc(da, db, dBase);
        cQuad.cPt3.x += dr*cDir2.y/d1;
        cQuad.cPt3.y -= dr*cDir2.x/d1;

        LineXLine(cQuad.cPt1, cDir1, cQuad.cPt3, cDir2, &cQuad.cPt2);
        d1 = GetQuadLength(&cQuad, 0.0, 1.0);

        if(d1 < dDist) dDist -= d1;
        else
        {
            bFound = true;
            pBounds[0].dRef = dt;
            pBounds[1].dRef = dBase;
        }

        dt = dBase;
        dBase *= 2.0;
    }

    double dt1 = GetQuadPointAtDist(&cQuad, 0.0, dDist);
    double dt2 = pBounds[1].dRef - pBounds[0].dRef;
    double dt0 = pBounds[0].dRef + dt1*dt2;
    double dr0 = dt2/10.0;
    if(dt0 + dr0 < pBounds[1].dRef) pBounds[1].dRef = dt0 + dr0;
    if(pBounds[0].dRef < dt0 - dr0) pBounds[0].dRef = dt0 - dr0;
    return GetQuadPoint(&cQuad, dt1);
}

CDPoint GetLineRegion(CDPrimitive cPrim, double dLineWidth, PDPoint pPoints)
{
    CDPoint cRes = {0, 0};
    CDPoint cDir = cPrim.cPt2 - cPrim.cPt1;
    double dNorm = GetNorm(cDir);
    if(dNorm > g_dPrec)
    {
        cDir /= dNorm;
        CDPoint cNorm = GetNormal(cDir);
        pPoints[0] = cPrim.cPt1 - 2.0*dLineWidth*(cDir + cNorm);
        pPoints[1] = cPrim.cPt2 + 2.0*dLineWidth*(cDir - cNorm);
        pPoints[2] = cPrim.cPt2 + 2.0*dLineWidth*(cDir + cNorm);
        pPoints[3] = cPrim.cPt1 - 2.0*dLineWidth*(cDir - cNorm);
        cRes.x = 4.1;
    }
    return cRes;
}

CDPoint GetArcRegion(CDPrimitive cPrim, double dLineWidth, PDPoint pPoints)
{
    CDPoint cRes = {0, 0};
    double dr = cPrim.cPt2.x - cPrim.cPt1.x;
    if(dr > g_dPrec)
    {
        double da1 = atan2(cPrim.cPt3.y - cPrim.cPt1.y, cPrim.cPt3.x - cPrim.cPt1.x);
        double da2 = atan2(cPrim.cPt4.y - cPrim.cPt1.y, cPrim.cPt4.x - cPrim.cPt1.x);
        if(da1 < da2) da1 += 2.0*M_PI;

        double da, dsi, dco;
        CDPoint cPt, cDir, cNorm;

        dco = cos(da2);
        dsi = sin(da2);
        cNorm.x = dco;
        cNorm.y = dsi;
        cPt = cPrim.cPt1 + dr*cNorm;
        cDir.x = -dsi;
        cDir.y = dco;

        pPoints[0] = cPt + 2.0*(0.1*dr + dLineWidth)*(cNorm - cDir);
        pPoints[15] = cPt - 2.0*dLineWidth*(cNorm + cDir);

        for(int i = 1; i < 7; i++)
        {
            da = da2 + i*(da1 - da2)/7.0;
            dco = cos(da);
            dsi = sin(da);
            cNorm.x = dco;
            cNorm.y = dsi;
            cPt = cPrim.cPt1 + dr*cNorm;
            pPoints[i] = cPt + 2.0*(0.1*dr + dLineWidth)*cNorm;
            pPoints[15 - i] = cPt - 2.0*dLineWidth*cNorm;
        }

        dco = cos(da1);
        dsi = sin(da1);
        cNorm.x = dco;
        cNorm.y = dsi;
        cPt = cPrim.cPt1 + dr*cNorm;
        cDir.x = -dsi;
        cDir.y = dco;

        pPoints[7] = cPt + 2.0*(0.1*dr + dLineWidth)*(cNorm + cDir);
        pPoints[8] = cPt - 2.0*dLineWidth*(cNorm - cDir);

        cRes.x = 16.1;
    }
    return cRes;
}

CDPoint GetCircRegion(CDPrimitive cPrim, double dLineWidth, PDPoint pPoints1, PDPoint pPoints2)
{
    CDPoint cRes = {0, 0};
    double dr = cPrim.cPt2.x - cPrim.cPt1.x;
    if(dr > g_dPrec)
    {
        double da, dco, dsi;
        CDPoint cNorm;

        for(int i = 0; i < 8; i++)
        {
            da = i*M_PI/4.0;
            dco = cos(da);
            dsi = sin(da);
            cNorm.x = dco;
            cNorm.y = dsi;
            pPoints1[i] = cPrim.cPt1 + 1.1*(dr + 2.0*dLineWidth)*cNorm;
            pPoints2[7 - i] = cPrim.cPt1 + (dr - 2.0*dLineWidth)*cNorm;
        }
        cRes.x = 8.1;
        cRes.y = 8.1;
    }
    return cRes;
}

CDPoint GetBezRegion(CDPrimitive cPrim, double dLineWidth, PDPoint pPoints)
{
    CDPoint cRes = {0, 0};

    CDPoint cDir1, cDir2, cNorm1, cNorm2;
    cDir1 = GetBezierDir(cPrim, 0.0);
    cDir2 = GetBezierDir(cPrim, 1.0);
    cNorm1 = GetNormal(cDir1);
    cNorm2 = GetNormal(cDir2);

    double dSum = (cPrim.cPt2.x - cPrim.cPt1.x)*(cPrim.cPt2.y + cPrim.cPt1.y) +
        (cPrim.cPt3.x - cPrim.cPt2.x)*(cPrim.cPt3.y + cPrim.cPt2.y) +
        (cPrim.cPt4.x - cPrim.cPt3.x)*(cPrim.cPt4.y + cPrim.cPt3.y) +
        (cPrim.cPt1.x - cPrim.cPt4.x)*(cPrim.cPt1.y + cPrim.cPt4.y);
    if(dSum < 0.0)
    {
        pPoints[0] = cPrim.cPt1 - 4.0*dLineWidth*(cDir1 + cNorm1);
        pPoints[1] = cPrim.cPt2 - 4.0*dLineWidth*cNorm1;
        pPoints[2] = cPrim.cPt3 - 4.0*dLineWidth*cNorm2;
        pPoints[3] = cPrim.cPt4 + 4.0*dLineWidth*(cDir2 - cNorm2);
        pPoints[4] = cPrim.cPt4 + 4.0*dLineWidth*(cDir2 + cNorm2);
        pPoints[5] = cPrim.cPt1 - 4.0*dLineWidth*(cDir1 - cNorm1);
    }
    else
    {
        pPoints[0] = cPrim.cPt1 - 4.0*dLineWidth*(cDir1 + cNorm1);
        pPoints[1] = cPrim.cPt4 + 4.0*dLineWidth*(cDir2 - cNorm2);
        pPoints[2] = cPrim.cPt4 + 4.0*dLineWidth*(cDir2 + cNorm2);
        pPoints[3] = cPrim.cPt3 + 4.0*dLineWidth*cNorm2;
        pPoints[4] = cPrim.cPt2 + 4.0*dLineWidth*cNorm1;
        pPoints[5] = cPrim.cPt1 - 4.0*dLineWidth*(cDir1 - cNorm1);
    }
    cRes.x = 6.1;
    return cRes;
}

CDPoint GetQuadRegion(CDPrimitive cPrim, double dLineWidth, PDPoint pPoints)
{
    CDPrimitive cPrimBez;
    cPrimBez.cPt1 = cPrim.cPt1;
    cPrimBez.cPt2 = (cPrim.cPt1 + 2.0*cPrim.cPt2)/3.0;
    cPrimBez.cPt3 = (cPrim.cPt3 + 2.0*cPrim.cPt2)/3.0;
    cPrimBez.cPt4 = cPrim.cPt3;

    return GetBezRegion(cPrimBez, dLineWidth, pPoints);
}

CDPoint GetBoundsRegion(CDPrimitive cPrim, double dLineWidth, PDPoint pPoints)
{
    double dx = 3.0*dLineWidth;
    pPoints[0].x = cPrim.cPt1.x - dx;
    pPoints[0].y = cPrim.cPt1.y - dx;
    pPoints[1].x = cPrim.cPt1.x + dx;
    pPoints[1].y = cPrim.cPt1.y - dx;
    pPoints[2].x = cPrim.cPt1.x + dx;
    pPoints[2].y = cPrim.cPt1.y + dx;
    pPoints[3].x = cPrim.cPt1.x - dx;
    pPoints[3].y = cPrim.cPt1.y + dx;
    CDPoint cRes = {4.1, 0.0};
    return cRes;
}

CDPoint GetCenterRegion(CDPrimitive cPrim, double dScale, PDPoint pPoints)
{
    double dx = 10.0/dScale;
    pPoints[0].x = cPrim.cPt1.x - dx;
    pPoints[0].y = cPrim.cPt1.y - dx;
    pPoints[1].x = cPrim.cPt1.x + dx;
    pPoints[1].y = cPrim.cPt1.y - dx;
    pPoints[2].x = cPrim.cPt1.x + dx;
    pPoints[2].y = cPrim.cPt1.y + dx;
    pPoints[3].x = cPrim.cPt1.x - dx;
    pPoints[3].y = cPrim.cPt1.y + dx;
    CDPoint cRes = {4.1, 0.0};
    return cRes;
}

CDPoint GetArrowRegion(CDPrimitive cPrim, double dLineWidth, PDPoint pPoints)
{
    CDPoint cDist = cPrim.cPt3 - cPrim.cPt2;
    double dx = 1.1*GetNorm(cDist) + 2.0*dLineWidth;
    if(dx < 1.0) dx = 1.0;

    pPoints[0].x = cPrim.cPt2.x - dx;
    pPoints[0].y = cPrim.cPt2.y - dx;
    pPoints[1].x = cPrim.cPt2.x + dx;
    pPoints[1].y = cPrim.cPt2.y - dx;
    pPoints[2].x = cPrim.cPt2.x + dx;
    pPoints[2].y = cPrim.cPt2.y + dx;
    pPoints[3].x = cPrim.cPt2.x - dx;
    pPoints[3].y = cPrim.cPt2.y + dx;

    CDPoint cRes = {4.1, 0};
    return cRes;
}

CDPoint GetLabRegion(CDPrimitive cPrim, double dLineWidth, PDPoint pPoints)
{
    double dPi2 = M_PI/2.0;
    double dco = cos(cPrim.cPt2.x - dPi2);
    double dsi = sin(cPrim.cPt2.x - dPi2);

    CDPoint cPt, cDir;
    cDir.x = dco;
    cDir.y = dsi;

    cPt.x = 30.0;
    cPt.y = 1.0;
    pPoints[0] = cPrim.cPt1 + Rotate(cPt, cDir, true);
    cPt.x = -30.0;
    cPt.y = 1.0;
    pPoints[1] = cPrim.cPt1 + Rotate(cPt, cDir, true);
    cPt.x = -30.0;
    cPt.y = -20.0;
    pPoints[2] = cPrim.cPt1 + Rotate(cPt, cDir, true);
    cPt.x = 30.0;
    cPt.y = -20.0;
    pPoints[3] = cPrim.cPt1 + Rotate(cPt, cDir, true);

    CDPoint cRes = {4.1, 0};
    return cRes;
}

CDPoint GetPrimRegion(CDPrimitive cPrim, double dLineWidth, double dScale,
    PDPoint pPoints1, PDPoint pPoints2)
{
    CDPoint cRes = {0, 0};
    if(!pPoints1)
    {
        switch(cPrim.iType)
        {
        case 1:
            cRes.x = 4.1;
            break;
        case 2:
            cRes.x = 16.1;
            break;
        case 3:
            cRes.x = 8.1;
            cRes.y = 8.1;
            break;
        case 4:
        case 5:
            cRes.x = 6.1;
            break;
        case 6:
        case 7:
            cRes.x = 4.1;
            break;
        case 9:
        case 10:
            cRes.x = 4.1;
            break;
        }
        return cRes;
    }

    switch(cPrim.iType)
    {
    case 1:
        cRes = GetLineRegion(cPrim, dLineWidth, pPoints1);
        break;
    case 2:
        cRes = GetArcRegion(cPrim, dLineWidth, pPoints1);
        break;
    case 3:
        cRes = GetCircRegion(cPrim, dLineWidth, pPoints1, pPoints2);
        break;
    case 4:
        cRes = GetQuadRegion(cPrim, dLineWidth, pPoints1);
        break;
    case 5:
        cRes = GetBezRegion(cPrim, dLineWidth, pPoints1);
        break;
    case 6:
        cRes = GetBoundsRegion(cPrim, dLineWidth, pPoints1);
        break;
    case 7:
        cRes = GetCenterRegion(cPrim, dScale, pPoints1);
        break;
    case 9:
        cRes = GetArrowRegion(cPrim, dLineWidth, pPoints1);
        break;
    case 10:
        cRes = GetLabRegion(cPrim, dLineWidth, pPoints1);
        break;
    }
    return cRes;
}

