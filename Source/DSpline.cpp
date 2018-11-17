#include "DSpline.hpp"
#include "DMath.hpp"
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "DPrimitive.hpp"

// for debugging purpose only
/*#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
extern HWND g_hStatus;*/
// -----

bool AddSplinePoint(double x, double y, char iCtrl, PDPointList pPoints)
{
    if(iCtrl > 1)
    {
        int nOffs = pPoints->GetCount(2);
        if(nOffs > 0) pPoints->SetPoint(0, 2, x, y, 2);
        else pPoints->AddPoint(x, y, 2);
        return true;
    }

    if(iCtrl == 1)
    {
        int nCtrl = pPoints->GetCount(1);
        if(nCtrl < 1) pPoints->AddPoint(x, y, 1);
        else pPoints->Remove(0, 1);
        return false;
    }

    pPoints->AddPoint(x, y, 0);
    return false;
}

CDPoint GetThreePointsControl(CDPoint cp1, CDPoint cp2, CDPoint cp3)
{
	double dl1 = GetDist(cp1, cp2);
	double dl2 = GetDist(cp2, cp3);
	double dt = dl1/(dl1 + dl2);

	if(dt < g_dPrec) return cp1;
    if(dt > 1.0 - g_dPrec) return cp3;

	CDPoint cRes = (cp2 - Power2(1.0 - dt)*cp1 - Power2(dt)*cp3)/dt/(1 - dt)/2.0;
	return cRes;
}

void GetMatrix(int iCount, bool bClosed, double *pdt, double *pdRes)
{
	double x1, x2, x3;

	if(bClosed)
	{
		double *pdDiag = pdRes;
		double *pdDiag1 = &pdRes[iCount];
		double *pdDiag2 = &pdRes[2*iCount - 1];
		double *pdLastCol1 = &pdRes[3*iCount - 2];
		double *pdLastCol2 = &pdRes[4*iCount - 4];

		x1 = (1.0 - pdt[0])*(1.0 - pdt[0])/2.0;
		x3 = pdt[0]*pdt[0]/2.0;
		x2 = x1 + 2.0*pdt[0]*(1.0 - pdt[0]) + x3;

		pdDiag[0] = sqrt(x2);
		pdDiag1[0] = x3/pdDiag[0];
		pdLastCol1[0] = x1/pdDiag[0];

		x1 = (1.0 - pdt[1])*(1.0 - pdt[1])/2.0;
		x3 = pdt[1]*pdt[1]/2.0;
		x2 = x1 + 2.0*pdt[1]*(1.0 - pdt[1]) + x3;

		pdDiag2[0] = x1/pdDiag[0];

		pdDiag[1] = sqrt(x2 - pdDiag1[0]*pdDiag2[0]);
		pdDiag1[1] = x3/pdDiag[1];
		pdLastCol1[1] = -pdDiag2[0]*pdLastCol1[0]/pdDiag[1];

		for(int i = 2; i < iCount - 2; i++)
		{
			x1 = (1.0 - pdt[i])*(1.0 - pdt[i])/2.0;
			x3 = pdt[i]*pdt[i]/2.0;
			x2 = x1 + 2.0*pdt[i]*(1.0 - pdt[i]) + x3;

			pdDiag2[i - 1] = x1/pdDiag[i - 1];

			pdDiag[i] = sqrt(x2 - pdDiag1[i - 1]*pdDiag2[i - 1]);
			pdDiag1[i] = x3/pdDiag[i];
			pdLastCol1[i] = -pdDiag2[i - 1]*pdLastCol1[i - 1]/pdDiag[i];
		}
		x1 = (1.0 - pdt[iCount - 2])*(1.0 - pdt[iCount - 2])/2.0;
		x3 = pdt[iCount - 2]*pdt[iCount - 2]/2.0;
		x2 = x1 + 2.0*pdt[iCount - 2]*(1.0 - pdt[iCount - 2]) + x3;

		pdDiag2[iCount - 3] = x1/pdDiag[iCount - 3];

		pdDiag[iCount - 2] = sqrt(x2 - pdDiag1[iCount - 3]*pdDiag2[iCount - 3]);
		pdDiag1[iCount - 2] = (x3 - pdDiag2[iCount - 3]*pdLastCol1[iCount - 3])/pdDiag[iCount - 2];

		x1 = (1.0 - pdt[iCount - 1])*(1.0 - pdt[iCount - 1])/2.0;
		x3 = pdt[iCount - 1]*pdt[iCount - 1]/2.0;
		x2 = x1 + 2.0*pdt[iCount - 1]*(1.0 - pdt[iCount - 1]) + x3;

		pdLastCol2[0] = x3/pdDiag[0];
		double dLastColSum = pdLastCol1[0]*pdLastCol2[0];
		for(int i = 1; i < iCount - 2; i++)
		{
			pdLastCol2[i] = -pdLastCol2[i - 1]*pdDiag1[i - 1]/pdDiag[i];
			dLastColSum += pdLastCol1[i]*pdLastCol2[i];
		}

		pdDiag2[iCount - 2] = (x1 - pdDiag1[iCount - 3]*pdLastCol2[iCount - 3])/pdDiag[iCount - 2];

		dLastColSum += pdDiag1[iCount - 2]*pdDiag2[iCount - 2];
		pdDiag[iCount - 1] = sqrt(x2 - dLastColSum);
	}
	else
	{
		double *pdDiag = pdRes;
		double *pdDiag1 = &pdRes[iCount - 2];
		double *pdDiag2 = &pdRes[2*iCount - 5];

		x3 = pdt[0]*pdt[0]/2.0;
		x2 = 2.0*pdt[0]*(1.0 - pdt[0]) + x3;
		pdDiag[0] = sqrt(x2);
		pdDiag1[0] = x3/pdDiag[0];

		for(int i = 1; i < iCount - 3; i++)
		{
			x1 = (1.0 - pdt[i])*(1.0 - pdt[i])/2.0;
			x3 = pdt[i]*pdt[i]/2.0;
			x2 = x1 + 2.0*pdt[i]*(1.0 - pdt[i]) + x3;

			pdDiag2[i - 1] = x1/pdDiag[i - 1];
			pdDiag[i] = sqrt(x2 - pdDiag1[i - 1]*pdDiag2[i - 1]);
			pdDiag1[i] = x3/pdDiag[i];
		}

		x1 = (1.0 - pdt[iCount - 3])*(1.0 - pdt[iCount - 3])/2.0;
		x2 = x1 + 2.0*pdt[iCount - 3]*(1.0 - pdt[iCount - 3]);
		pdDiag2[iCount - 4] = x1/pdDiag[iCount - 4];
		pdDiag[iCount - 3] = sqrt(x2 - pdDiag1[iCount - 4]*pdDiag2[iCount - 4]);
	}

	return;
}

double AddOffset(PDPoint pTmpPt, int iMode, PDPointList pPoints, PDPointList pCache)
{
    double dRes = 0.0;
    int nOffs = pPoints->GetCount(2);
    if((iMode == 2) || (nOffs > 0))
    {
        CDPoint cPt1;
        if(iMode == 2) cPt1 = *pTmpPt;
        else cPt1 = pPoints->GetPoint(0, 2).cPoint;

        CDLine cPtX;
        double dDist = GetSplineDistFromPt(cPt1, cPt1, pCache, &cPtX);
        double dDistOld = 0.0;

        if((nOffs > 0) && (iMode == 2))
        {
            cPt1 = pPoints->GetPoint(0, 2).cPoint;
            dDistOld = GetSplineDistFromPt(cPt1, cPt1, pCache, &cPtX);
        }

        dRes = dDist - dDistOld;
        if(fabs(dDist) > g_dPrec) pCache->AddPoint(dDist, dDistOld, 2);
    }
    return dRes;
}

bool BuildSplineCache(CDLine cTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    double *pdDist)
{
    pCache->ClearAll();

    int nCtrl = pPoints->GetCount(1);
    bool bClosed = nCtrl > 0;

    int nNorm = pPoints->GetCount(0);
    int n = nNorm;

    if(iMode == 1) n++;

    CDInputPoint cInPt;
    CDPoint cPt1, cPt2, cPt3;

    if(bClosed)
    {
        cPt1 = pPoints->GetPoint(0, 1).cPoint;
        pCache->AddPoint(cPt1.x, cPt1.y, 1);
    }

	if(bClosed && (n < 3))
	{
		if(nNorm > 0)
		{
		    cInPt = pPoints->GetPoint(0, 0);
		    pCache->AddPoint(cInPt.cPoint.x, cInPt.cPoint.y, 0);
		}
		if(nNorm > 1)
		{
		    cInPt = pPoints->GetPoint(1, 0);
		    pCache->AddPoint(cInPt.cPoint.x, cInPt.cPoint.y, 0);
		}
		if(iMode == 1) pCache->AddPoint(cTmpPt.cOrigin.x, cTmpPt.cOrigin.y, 0);
		*pdDist = AddOffset(&cTmpPt.cOrigin, iMode, pPoints, pCache);
		return true;
	}

	if(!bClosed && (n < 4))
	{
		if(nNorm > 0)
		{
		    cInPt = pPoints->GetPoint(0, 0);
		    pCache->AddPoint(cInPt.cPoint.x, cInPt.cPoint.y, 0);
		}
		if(nNorm > 2)
		{
		    cInPt = pPoints->GetPoint(0, 0);
		    cPt1 = cInPt.cPoint;
		    cInPt = pPoints->GetPoint(1, 0);
		    cPt2 = cInPt.cPoint;
		    cInPt = pPoints->GetPoint(2, 0);
		    cPt3 = cInPt.cPoint;
			CDPoint cControl = GetThreePointsControl(cPt1, cPt2, cPt3);
			pCache->AddPoint(cControl.x, cControl.y, 0);
		}
		else if((nNorm > 1) && (iMode == 1))
		{
		    cInPt = pPoints->GetPoint(0, 0);
		    cPt1 = cInPt.cPoint;
		    cInPt = pPoints->GetPoint(1, 0);
		    cPt2 = cInPt.cPoint;
		    cPt3 = cTmpPt.cOrigin;
			CDPoint cControl = GetThreePointsControl(cPt1, cPt2, cPt3);
			pCache->AddPoint(cControl.x, cControl.y, 0);
		}

        if(iMode == 1)
        {
		    pCache->AddPoint(cTmpPt.cOrigin.x, cTmpPt.cOrigin.y, 0);
        }
		else if(nNorm > 1)
		{
		    cInPt = pPoints->GetPoint(nNorm - 1, 0);
		    pCache->AddPoint(cInPt.cPoint.x, cInPt.cPoint.y, 0);
		}

		*pdDist = AddOffset(&cTmpPt.cOrigin, iMode, pPoints, pCache);
		return true;
	}

	int iDim = 0;
	if(bClosed) iDim = n;
	else iDim = n - 2;

	double *dt = (double*)malloc(iDim*sizeof(double));
	double dl1, dl2;

	if(bClosed)
	{
	    cPt1 = pPoints->GetPoint(0, 0).cPoint;
        if(iMode == 1) cPt2 = cTmpPt.cOrigin;
        else cPt2 = pPoints->GetPoint(nNorm - 1, 0).cPoint;
		dl1 = GetDist(cPt1, cPt2);
	    cPt2 = pPoints->GetPoint(1, 0).cPoint;
		dl2 = GetDist(cPt1, cPt2);
		dt[0] = dl1/(dl1 + dl2);
		for(int i = 1; i < iDim - 2; i++)
		{
			dl1 = dl2;
			cPt1 = cPt2;
			cPt2 = pPoints->GetPoint(i + 1, 0).cPoint;
			dl2 = GetDist(cPt1, cPt2);
			dt[i] = dl1/(dl1 + dl2);
		}
        cPt1 = cPt2;
        if(iMode == 1) cPt2 = cTmpPt.cOrigin;
        else cPt2 = pPoints->GetPoint(nNorm - 1, 0).cPoint;
        dl1 = dl2;
		dl2 = GetDist(cPt1, cPt2);
        dt[iDim - 2] = dl1/(dl1 + dl2);
        cPt1 = cPt2;
        cPt2 = pPoints->GetPoint(0, 0).cPoint;
        dl1 = dl2;
		dl2 = GetDist(cPt1, cPt2);
		dt[iDim - 1] = dl1/(dl1 + dl2);
	}
	else
	{
	    cPt1 = pPoints->GetPoint(0, 0).cPoint;
	    cPt2 = pPoints->GetPoint(1, 0).cPoint;
		dl1 = GetDist(cPt1, cPt2);
		cPt1 = cPt2;
	    cPt2 = pPoints->GetPoint(2, 0).cPoint;
		dl2 = GetDist(cPt1, cPt2);
		dt[0] = dl1/(dl1 + dl2/2.0);
		for(int i = 1; i < iDim - 1; i++)
		{
			dl1 = dl2;
            cPt1 = cPt2;
            cPt2 = pPoints->GetPoint(i + 2, 0).cPoint;
			dl2 = GetDist(cPt1, cPt2);
			dt[i] = dl1/(dl1 + dl2);
		}
		dl1 = dl2;
        cPt1 = cPt2;
        if(iMode == 1) cPt2 = cTmpPt.cOrigin;
        else cPt2 = pPoints->GetPoint(iDim + 1, 0).cPoint;
		dl2 = GetDist(cPt1, cPt2);
		dt[iDim - 1] = dl1/(dl1 + 2.0*dl2);
	}

	int iMatSize = 0;
	if(bClosed) iMatSize = 5*n - 6; // n + 2*(n - 1) + 2*(n - 2)
	else iMatSize = 3*n - 8; // (n - 2) + 2*(n - 3)

	double *pdMatrix = (double*)malloc(iMatSize*sizeof(double));

	GetMatrix(n, bClosed, dt, pdMatrix);

	double *dx = (double*)malloc(iDim*sizeof(double));
	double *dy = (double*)malloc(iDim*sizeof(double));
	double *dx2 = (double*)malloc(iDim*sizeof(double));
	double *dy2 = (double*)malloc(iDim*sizeof(double));

	if(bClosed)
	{
		double *pdDiag = pdMatrix;
		double *pdDiag1 = &pdMatrix[n];
		double *pdDiag2 = &pdMatrix[2*n - 1];
		double *pdLastCol1 = &pdMatrix[3*n - 2];
		double *pdLastCol2 = &pdMatrix[4*n - 4];

	    cPt1 = pPoints->GetPoint(0, 0).cPoint;
		dx[0] = cPt1.x/pdDiag[0];
		dy[0] = cPt1.y/pdDiag[0];
		for(int i = 1; i < iDim - 1; i++)
		{
            cPt1 = pPoints->GetPoint(i, 0).cPoint;
			dx[i] = (cPt1.x - pdDiag2[i - 1]*dx[i - 1])/pdDiag[i];
			dy[i] = (cPt1.y - pdDiag2[i - 1]*dy[i - 1])/pdDiag[i];
		}

        if(iMode == 1) cPt1 = cTmpPt.cOrigin;
        else cPt1 = pPoints->GetPoint(nNorm - 1, 0).cPoint;
		dx[iDim - 1] = cPt1.x - pdDiag2[iDim - 2]*dx[iDim - 2];
		dy[iDim - 1] = cPt1.y - pdDiag2[iDim - 2]*dy[iDim - 2];
		for(int i = 0; i < iDim - 2; i++)
		{
			dx[iDim - 1] -= (dx[i]*pdLastCol2[i]);
			dy[iDim - 1] -= (dy[i]*pdLastCol2[i]);
		}
		dx[iDim - 1] /= pdDiag[iDim - 1];
		dy[iDim - 1] /= pdDiag[iDim - 1];

		dx2[iDim - 1] = dx[iDim - 1]/pdDiag[iDim - 1];
		dy2[iDim - 1] = dy[iDim - 1]/pdDiag[iDim - 1];
		dx2[iDim - 2] = (dx[iDim - 2] - pdDiag1[iDim - 2]*dx2[iDim - 1])/pdDiag[iDim - 2];
		dy2[iDim - 2] = (dy[iDim - 2] - pdDiag1[iDim - 2]*dy2[iDim - 1])/pdDiag[iDim - 2];

		for(int i = iDim - 3; i >= 0; i--)
		{
			dx2[i] = (dx[i] - pdDiag1[i]*dx2[i + 1] - pdLastCol1[i]*dx2[iDim - 1])/pdDiag[i];
			dy2[i] = (dy[i] - pdDiag1[i]*dy2[i + 1] - pdLastCol1[i]*dy2[iDim - 1])/pdDiag[i];
		}

		for(int i = 0; i < iDim; i++)
		{
		    pCache->AddPoint(dx2[i], dy2[i], 0);
		}
	}
	else
	{
		double *pdDiag = pdMatrix;
		double *pdDiag1 = &pdMatrix[n - 2];
		double *pdDiag2 = &pdMatrix[2*n - 5];

        cPt1 = pPoints->GetPoint(0, 0).cPoint;
        cPt2 = pPoints->GetPoint(1, 0).cPoint;
		dx[0] = (cPt2.x - cPt1.x*Power2(1.0 - dt[0]))/pdDiag[0];
		dy[0] = (cPt2.y - cPt1.y*Power2(1.0 - dt[0]))/pdDiag[0];
		for(int i = 1; i < iDim - 1; i++)
		{
            cPt1 = pPoints->GetPoint(i + 1, 0).cPoint;
			dx[i] = (cPt1.x - pdDiag2[i - 1]*dx[i - 1])/pdDiag[i];
			dy[i] = (cPt1.y - pdDiag2[i - 1]*dy[i - 1])/pdDiag[i];
		}
        cPt1 = pPoints->GetPoint(iDim, 0).cPoint;
        if(iMode == 1) cPt2 = cTmpPt.cOrigin;
        else cPt2 = pPoints->GetPoint(iDim + 1, 0).cPoint;
		dx[iDim - 1] = ((cPt1.x - cPt2.x*dt[n - 3]*dt[n - 3]) -
			pdDiag2[iDim - 2]*dx[iDim - 2])/pdDiag[iDim - 1];
		dy[iDim - 1] = ((cPt1.y - cPt2.y*dt[n - 3]*dt[n - 3]) -
			pdDiag2[iDim - 2]*dy[iDim - 2])/pdDiag[iDim - 1];

		dx2[iDim - 1] = dx[iDim - 1]/pdDiag[iDim - 1];
		dy2[iDim - 1] = dy[iDim - 1]/pdDiag[iDim - 1];

		for(int i = iDim - 2; i >= 0; i--)
		{
			dx2[i] = (dx[i] - pdDiag1[i]*dx2[i + 1])/pdDiag[i];
			dy2[i] = (dy[i] - pdDiag1[i]*dy2[i + 1])/pdDiag[i];
		}

        cPt1 = pPoints->GetPoint(0, 0).cPoint;
        pCache->AddPoint(cPt1.x, cPt1.y, 0);
		for(int i = 0; i < iDim; i++)
		{
            pCache->AddPoint(dx2[i], dy2[i], 0);
		}
		if(iMode == 1) cPt1 = cTmpPt.cOrigin;
        else cPt1 = pPoints->GetPoint(nNorm - 1, 0).cPoint;
        pCache->AddPoint(cPt1.x, cPt1.y, 0);
	}

	free(pdMatrix);

	free(dt);

	free(dy2);
	free(dx2);
	free(dy);
	free(dx);

	*pdDist = AddOffset(&cTmpPt.cOrigin, iMode, pPoints, pCache);
    return true;
}

void SubQuad(double dt1, double dt2, PDPrimitive pQuad)
{
    double dt = dt2 - dt1;

    CDPoint cp11 = pQuad->cPt1;
    CDPoint cp12 = 2.0*(pQuad->cPt2 - pQuad->cPt1);
    CDPoint cp13 = pQuad->cPt3 - 2.0*pQuad->cPt2 + pQuad->cPt1;

    CDPoint cp21 = cp11 + dt1*cp12 + Power2(dt1)*cp13;
    CDPoint cp22 = dt*(cp12 + 2.0*dt1*cp13);
    CDPoint cp23 = Power2(dt)*cp13;

    pQuad->cPt1 = cp21;
    pQuad->cPt2 = pQuad->cPt1 + cp22/2.0;
    pQuad->cPt3 = cp23 + 2.0*pQuad->cPt2 - pQuad->cPt1;
}

int CropQuad(CDPrimitive cQuad, double dr, PDRect pRect, PDPrimObject pPrimList)
{
    if(fabs(dr) < g_dPrec)
    {
        return CropPrimitive(cQuad, pRect, pPrimList);
    }

    CDPrimitive cPrim;
    cPrim.iType = 4;

    CDPoint cDir1, cDir2;
    cDir1 = GetQuadNormal(&cQuad, 0.0);
    cPrim.cPt1 = GetQuadPoint(&cQuad, 0.0) + dr*cDir1;
    cDir2 = GetQuadNormal(&cQuad, 0.5);
    cPrim.cPt3 = GetQuadPoint(&cQuad, 0.5) + dr*cDir2;
    LineXLine(cPrim.cPt1, GetNormal(cDir1), cPrim.cPt3, GetNormal(cDir2), &cPrim.cPt2);
    int k1 = CropPrimitive(cPrim, pRect, pPrimList);

    cDir1 = cDir2;
    cPrim.cPt1 = cPrim.cPt3;
    cDir2 = GetQuadNormal(&cQuad, 1.0);
    cPrim.cPt3 = GetQuadPoint(&cQuad, 1.0) + dr*cDir2;
    LineXLine(cPrim.cPt1, GetNormal(cDir1), cPrim.cPt3, GetNormal(cDir2), &cPrim.cPt2);
    int k2 = CropPrimitive(cPrim, pRect, pPrimList);

    int iRes = k1;
    if(k2 < iRes) iRes = k2;
    return iRes;
}

int CropBez(CDPrimitive cQuad, double dr, PDRect pRect, PDPrimObject pPrimList)
{
    if(fabs(dr) < g_dPrec)
    {
        return CropPrimitive(cQuad, pRect, pPrimList);
    }

    CDPrimitive cPrim;
    cPrim.iType = 5;

    CDPoint cDir1, cDir2, cHypPts[5];
    double dt;
    for(int j = 0; j < 5; j++)
    {
        dt = j/4.0;

        cDir1 = GetQuadNormal(&cQuad, dt);
        cHypPts[j] = GetQuadPoint(&cQuad, dt) + dr*cDir1;
    }

    cDir2 = GetNormal(cDir1);
    cDir1 = GetNormal(GetQuadNormal(&cQuad, 0.0));

    ApproxLineSeg(5, cHypPts, &cDir1, &cDir2, &cPrim);
    return CropPrimitive(cPrim, pRect, pPrimList);
}

double GetQuadBufLength(CDPrimitive cQuad, double dr, double dt1, double dt2)
{
    if(fabs(dr) < g_dPrec) return GetQuadLength(&cQuad, dt1, dt2);

    int iDiv = (int)4*(dt2 - dt1);
    if(iDiv < 1) iDiv = 1;

    double dt = dt1;
    double dRes = 0.0;
    //double dSeg = (dt2 - dt1)/(double)iDiv;

    CDPoint cDir2 = GetQuadNormal(&cQuad, dt);
    CDPoint cDir1;

    CDPrimitive cQuad1;
    cQuad1.cPt3 = GetQuadPoint(&cQuad, dt) + dr*cDir2;

    for(int i = 0; i < iDiv; i++)
    {
        cDir1 = cDir2;
        cQuad1.cPt1 = cQuad1.cPt3;

        dt = dt1 + (i + 1)*(dt2 - dt1)/iDiv;
        cDir2 = GetQuadNormal(&cQuad, dt);
        cQuad1.cPt3 = GetQuadPoint(&cQuad, dt) + dr*cDir2;

        LineXLine(cQuad1.cPt1, GetNormal(cDir1), cQuad1.cPt3, GetNormal(cDir2), &cQuad1.cPt2);
        dRes += GetQuadLength(&cQuad1, 0.0, 1.0);
    }
    return dRes;
}

CDPrimitive CompactQuad(CDPrimitive cQuad)
{
    CDPrimitive cRes;
    cRes.cPt1 = cQuad.cPt1;
    cRes.cPt2 = 2.0*(cQuad.cPt2 - cQuad.cPt1);
    cRes.cPt3 = cQuad.cPt3 - 2.0*cQuad.cPt2 + cQuad.cPt1;
    return cRes;
}

CDPrimitive GetQuadDeriv(CDPrimitive cCompQuad)
{
    CDPrimitive cRes;
    cRes.cPt1 = cCompQuad.cPt2;
    cRes.cPt2 = 2.0*cCompQuad.cPt3;
    return cRes;
}

CDPoint EvalCompQuad(CDPrimitive cCompQuad, double dt)
{
    CDPoint cRes = cCompQuad.cPt1 + dt*cCompQuad.cPt2 + Power2(dt)*cCompQuad.cPt3;
    return cRes;
}

double Bound01(double dx, bool *pbIn01)
{
    *pbIn01 = false;
    if(dx < -g_dPrec) return 0.0;
    if(dx > 1.0 + g_dPrec) return 1.0;

    *pbIn01 = true;
    if(dx < g_dPrec) return 0.0;
    if(dx > 1.0 - g_dPrec) return 1.0;
    return dx;
}

bool GetQuadPtProj(CDPoint cPt, CDPoint cRefPt, CDPrimitive cQuad, double *pdProj)
{
    CDPoint cDir1 = cQuad.cPt2 - cQuad.cPt1;
    CDPoint cDir2 = cQuad.cPt3 - cQuad.cPt2;
    double d1 = GetNorm(cDir1);
    double d2 = GetNorm(cDir2);
    bool bIn01;
    CDLine cPtX;

    if(d1 < g_dPrec)
    {
        if(d2 < g_dPrec)
        {
            *pdProj = 0.0;
            return true;
        }

        GetPtDistFromLineSeg(cPt, cQuad.cPt1, cQuad.cPt3, &cPtX);
        *pdProj = Bound01(cPtX.dRef, &bIn01);
        return bIn01;
    }

    if(d2 < g_dPrec)
    {
        GetPtDistFromLineSeg(cPt, cQuad.cPt1, cQuad.cPt3, &cPtX);
        *pdProj = Bound01(cPtX.dRef, &bIn01);
        return bIn01;
    }

    cDir1 /= d1;
    cDir2 /= d2;

    CDPrimitive cCompQuad = CompactQuad(cQuad);
    CDPrimitive cCompDeriv = GetQuadDeriv(cCompQuad);

    double dPoly11[3], dPoly12[3];
    double dPoly21[2], dPoly22[2];

    dPoly11[0] = cCompQuad.cPt1.x;
    dPoly11[1] = cCompQuad.cPt2.x;
    dPoly11[2] = cCompQuad.cPt3.x;

    dPoly12[0] = cCompQuad.cPt1.y;
    dPoly12[1] = cCompQuad.cPt2.y;
    dPoly12[2] = cCompQuad.cPt3.y;

    dPoly21[0] = cCompDeriv.cPt1.x;
    dPoly21[1] = cCompDeriv.cPt2.x;

    dPoly22[0] = cCompDeriv.cPt1.y;
    dPoly22[1] = cCompDeriv.cPt2.y;

    double dPoly31[4], dPoly32[4];
    int iDeg31 = MultiplyPolynoms(2, 1, dPoly11, dPoly21, dPoly31);
    int iDeg32 = MultiplyPolynoms(2, 1, dPoly12, dPoly22, dPoly32);

    double dPoly41[4];
    int iDeg41 = AddPolynoms(iDeg31, iDeg32, dPoly31, dPoly32, dPoly41);

    dPoly41[0] -= (cPt.x*dPoly21[0] + cPt.y*dPoly22[0]);
    dPoly41[1] -= (cPt.x*dPoly21[1] + cPt.y*dPoly22[1]);

    double dRoots[3];
    int iRoots = SolvePolynom01(iDeg41, dPoly41, dRoots);

    if(iRoots < 1) return false;

    double dt = dRoots[0];
    double dtMin = dt;
    CDPoint cPt2 = EvalCompQuad(cCompQuad, dt);
    d1 = GetDist(cRefPt, cPt2);

    for(int i = 1; i < iRoots; i++)
    {
        dt = dRoots[i];
        cPt2 = EvalCompQuad(cCompQuad, dt);
        d2 = GetDist(cRefPt, cPt2);
        if(d2 < d1)
        {
            d1 = d2;
            dtMin = dt;
        }
    }

    CDPoint cDir = GetQuadNormal(&cQuad, dtMin);
    cDir2 = cPt - EvalCompQuad(cCompQuad, dtMin);
    d2 = GetNorm(cDir2);
    cDir += cDir2/d2;

    int i = 0;
    while(i < 4)
    {
        dPoly11[0] = (cCompQuad.cPt1.x - cPt.x)*cDir.y - (cCompQuad.cPt1.y - cPt.y)*cDir.x;
        dPoly11[1] = cCompQuad.cPt2.x*cDir.y - cCompQuad.cPt2.y*cDir.x;
        dPoly11[2] = cCompQuad.cPt3.x*cDir.y - cCompQuad.cPt3.y*cDir.x;

        iRoots = SolvePolynom01(2, dPoly11, dRoots);
        if(iRoots > 0)
        {
            dtMin = dRoots[0];
            if(iRoots > 1)
            {
                cPt2 = EvalCompQuad(cCompQuad, dtMin);
                d1 = GetDist(cPt2, cRefPt);
                cPt2 = EvalCompQuad(cCompQuad, dRoots[1]);
                d2 = GetDist(cPt2, cRefPt);
                if(d2 < d1) dtMin = dRoots[1];
            }
            cDir = GetQuadNormal(&cQuad, dtMin);
            cDir2 = cPt - EvalCompQuad(cCompQuad, dtMin);
            d2 = GetNorm(cDir2);
            cDir += cDir2/d2;
        }
        else
        {
            cDir = -1.0*GetQuadNormal(&cQuad, dtMin);
            cDir += cDir2/d2;
        }
        i++;
    }

    *pdProj = dtMin;
    return true;
}

double GetSplinePtProj(CDPoint cPt, CDPoint cRefPt, PDPointList pCache)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return -1.0;

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);

    CDPrimitive cQuad;
    double d1 = 0.0, d2;
    CDPoint bPt1, bPt2, bPt3;
    CDLine cPtX;
    bool b01;

    if(iCnt < 3)
    {
        bPt1 = pCache->GetPoint(0, 0).cPoint;
        bPt2 = pCache->GetPoint(1, 0).cPoint;
        GetPtDistFromLineSeg(cPt, bPt1, bPt2, &cPtX);
        return Bound01(cPtX.dRef, &b01);
    }

    bPt1 = pCache->GetPoint(0, 0).cPoint;
    bPt2 = pCache->GetPoint(1, 0).cPoint;
    bPt3 = pCache->GetPoint(2, 0).cPoint;
    if(bClosed) cQuad.cPt1 = (bPt1 + bPt2)/2.0;
    else cQuad.cPt1 = bPt1;
    cQuad.cPt2 = bPt2;
    cQuad.cPt3 = (bPt2 + bPt3)/2.0;

    bool bMinSet = false;
    int iSeg = 0;
    double dProj1 = -1.0, dProj2;

    if(!bClosed)
    {
        dProj1 = 0.0;
        d1 = GetDist(bPt1, cRefPt);
        bMinSet = true;
    }

    for(int i = 3; i < iCnt; i++)
    {
        b01 = GetQuadPtProj(cPt, cRefPt, cQuad, &dProj2);
        if(b01)
        {
            bPt1 = GetQuadPoint(&cQuad, dProj2);
            d2 = GetDist(bPt1, cRefPt);
            if(!bMinSet || (d2 < d1))
            {
                d1 = d2;
                dProj1 = (double)iSeg + dProj2;
                bMinSet = true;
            }
        }
        iSeg++;

        bPt2 = bPt3;
        bPt3 = pCache->GetPoint(i, 0).cPoint;
        cQuad.cPt1 = cQuad.cPt3;
        cQuad.cPt2 = bPt2;
        cQuad.cPt3 = (bPt2 + bPt3)/2.0;
    }

    if(bClosed)
    {
        b01 = GetQuadPtProj(cPt, cRefPt, cQuad, &dProj2);
        if(b01)
        {
            bPt1 = GetQuadPoint(&cQuad, dProj2);
            d2 = GetDist(bPt1, cRefPt);
            if(!bMinSet || (d2 < d1))
            {
                d1 = d2;
                dProj1 = (double)iSeg + dProj2;
                bMinSet = true;
            }
        }
        iSeg++;

        bPt2 = bPt3;
        bPt3 = pCache->GetPoint(0, 0).cPoint;
        cQuad.cPt1 = cQuad.cPt3;
        cQuad.cPt2 = bPt2;
        cQuad.cPt3 = (bPt2 + bPt3)/2.0;

        b01 = GetQuadPtProj(cPt, cRefPt, cQuad, &dProj2);
        if(b01)
        {
            bPt1 = GetQuadPoint(&cQuad, dProj2);
            d2 = GetDist(bPt1, cRefPt);
            if(!bMinSet || (d2 < d1))
            {
                d1 = d2;
                dProj1 = (double)iSeg + dProj2;
                bMinSet = true;
            }
        }
        iSeg++;

        bPt2 = bPt3;
        bPt3 = pCache->GetPoint(1, 0).cPoint;
        cQuad.cPt1 = cQuad.cPt3;
        cQuad.cPt2 = bPt2;
        cQuad.cPt3 = (bPt2 + bPt3)/2.0;
    }
    else cQuad.cPt3 = bPt3;

    b01 = GetQuadPtProj(cPt, cRefPt, cQuad, &dProj2);
    if(b01)
    {
        bPt1 = GetQuadPoint(&cQuad, dProj2);
        d2 = GetDist(bPt1, cRefPt);
        if(!bMinSet || (d2 < d1))
        {
            d1 = d2;
            dProj1 = (double)iSeg + dProj2;
            bMinSet = true;
        }
    }

    if(!bClosed)
    {
        d2 = GetDist(bPt3, cRefPt);
        if(d2 < d1) dProj1 = (double)(iSeg + 1);
    }
    return dProj1;
}

double GetQuadBufPointAtDist(CDPrimitive cQuad, double dr, double t1, double dDist)
{
    if(fabs(dr) < g_dPrec) return GetQuadPointAtDist(&cQuad, t1, dDist);

    int iDiv = (int)4*(1.0 - t1);
    if(iDiv < 1) iDiv = 1;

    double dt = t1;
    double d1;

    CDPoint cDir2 = GetQuadNormal(&cQuad, dt);
    CDPoint cDir1;

    CDPrimitive cQuad1;
    cQuad1.cPt3 = GetQuadPoint(&cQuad, dt) + dr*cDir2;

    bool bFound = false;
    int i = 0;

    while(!bFound && (i < iDiv))
    {
        i++;
        dt = t1 + i*(1.0 - t1)/iDiv;

        cDir1 = cDir2;
        cQuad1.cPt1 = cQuad1.cPt3;

        cDir2 = GetQuadNormal(&cQuad, dt);
        cQuad1.cPt3 = GetQuadPoint(&cQuad, dt) + dr*cDir2;

        LineXLine(cQuad1.cPt1, GetNormal(cDir1), cQuad1.cPt3, GetNormal(cDir2), &cQuad1.cPt2);
        d1 = GetQuadLength(&cQuad1, 0.0, 1.0);
        if(d1 < dDist - g_dPrec) dDist -= d1;
        else bFound = true;
    }

    if(!bFound) return 1.0;

    dt = GetQuadPointAtDist(&cQuad1, 0.0, dDist);
    CDPoint cPt1 = GetQuadPoint(&cQuad1, dt);

    if(!GetQuadPtProj(cPt1, cPt1, cQuad, &dt)) dt = 1.0;
    return dt;
}

int AddSplineSegmentWithBounds(double dT1, double dT2, PDPointList pCache,
    PDPrimObject pPrimList, PDRect pRect)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return 0;

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    CDPoint cDir, cPt1, cPt2;
    double dNorm;

    CDPrimitive cQuad;

    if(iCnt < 3)
    {
        cPt1 = pCache->GetPoint(0, 0).cPoint;
        cPt2 = pCache->GetPoint(1, 0).cPoint;

        cDir = cPt2 - cPt1;
        dNorm = GetNorm(cDir);
        if(dNorm < g_dPrec) return 0;

        cDir = GetNormal(cDir/dNorm);

        cQuad.iType = 1;
        cQuad.cPt1 = (1.0 - dT1)*cPt1 + dT1*cPt2 + dr*cDir;
        cQuad.cPt2 = (1.0 - dT2)*cPt1 + dT2*cPt2 + dr*cDir;
        cQuad.cPt3 = 0;
        cQuad.cPt4 = 0;

        return CropPrimitive(cQuad, pRect, pPrimList);
    }

    cQuad.iType = 4;

    int iCurRes;

    int iStartSeg = (int)(dT1 + g_dPrec);
    double dt1 = dT1 - iStartSeg;
    int iEndSeg = (int)(dT2 + g_dPrec);
    double dt2 = dT2 - iEndSeg;
    if((dt2 < g_dPrec) && (iEndSeg > 0))
    {
        iEndSeg--;
        dt2 = 1.0;
    }

    bool bm1, bm2;

    int k = iStartSeg;
    int iCurSeg = iStartSeg;
    bool bTurnedAround = false;

    bm1 = bClosed || (k > 0);
    CDPoint bPt1, bPt2, bPt3;
    bPt1 = pCache->GetPoint(k++, 0).cPoint;
    if(k > iCnt - 1)
    {
        k = 0;
        bTurnedAround = true;
    }
    bPt2 = pCache->GetPoint(k++, 0).cPoint;
    if(k > iCnt - 1)
    {
        k = 0;
        bTurnedAround = true;
    }
    bPt3 = pCache->GetPoint(k++, 0).cPoint;
    bm2 = bClosed || (k < iCnt);
    if(k > iCnt - 1)
    {
        k = 0;
        bTurnedAround = true;
    }

    if(bm1) cQuad.cPt1 = (bPt1 + bPt2)/2.0;
    else cQuad.cPt1 = bPt1;
    cQuad.cPt2 = bPt2;
    if(bm2) cQuad.cPt3 = (bPt2 + bPt3)/2.0;
    else cQuad.cPt3 = bPt3;

    if(iStartSeg == iEndSeg)
    {
        SubQuad(dt1, dt2, &cQuad);
        return CropBez(cQuad, dr, pRect, pPrimList);
    }

    SubQuad(dt1, 1.0, &cQuad);
    int iRes = CropBez(cQuad, dr, pRect, pPrimList);
    iCurSeg++;
    if(bTurnedAround && (k > 1)) iCurSeg = 0;

    if((dT2 < dT1) && !bTurnedAround)
    {
        while(k < iCnt)
        {
            cQuad.cPt1 = cQuad.cPt3;
            bPt2 = bPt3;
            bPt3 = pCache->GetPoint(k++, 0).cPoint;
            cQuad.cPt2 = bPt2;
            cQuad.cPt3 = (bPt2 + bPt3)/2.0;
            iCurRes = CropBez(cQuad, dr, pRect, pPrimList);
            if(iRes != iCurRes) iRes = 1;
            iCurSeg++;
        }
        k = 0;
    }

    while(iCurSeg < iEndSeg)
    {
        if(k >= iCnt)
        {
            k = 0;
            bTurnedAround = true;
        }
        if(bTurnedAround && (k > 1)) iCurSeg = 0;
        cQuad.cPt1 = cQuad.cPt3;
        bPt2 = bPt3;
        bPt3 = pCache->GetPoint(k++, 0).cPoint;
        cQuad.cPt2 = bPt2;
        if(bClosed || (k < iCnt)) cQuad.cPt3 = (bPt2 + bPt3)/2.0;
        else cQuad.cPt3 = bPt3;

        iCurRes = CropBez(cQuad, dr, pRect, pPrimList);
        if(iRes != iCurRes) iRes = 1;
        iCurSeg++;
    }

    if(k >= iCnt) k = 0;
    cQuad.cPt1 = cQuad.cPt3;
    bPt2 = bPt3;
    bPt3 = pCache->GetPoint(k++, 0).cPoint;
    cQuad.cPt2 = bPt2;
    if(bClosed || (k < iCnt)) cQuad.cPt3 = (bPt2 + bPt3)/2.0;
    else cQuad.cPt3 = bPt3;

    SubQuad(0.0, dt2, &cQuad);
    iCurRes = CropBez(cQuad, dr, pRect, pPrimList);
    if(iRes != iCurRes) iRes = 1;

    return iRes;
}

int AddSplineSegmentQuadsWithBounds(double dT1, double dT2, PDPointList pCache,
    PDPrimObject pPrimList, PDRect pRect)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return 0;

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    CDPoint cDir, cPt1, cPt2;
    double dNorm;

    CDPrimitive cQuad;

    if(iCnt < 3)
    {
        cPt1 = pCache->GetPoint(0, 0).cPoint;
        cPt2 = pCache->GetPoint(1, 0).cPoint;

        cDir = cPt2 - cPt1;
        dNorm = GetNorm(cDir);
        if(dNorm < g_dPrec) return 0;

        cDir = GetNormal(cDir/dNorm);

        cQuad.iType = 1;
        cQuad.cPt1 = (1.0 - dT1)*cPt1 + dT1*cPt2 + dr*cDir;
        cQuad.cPt2 = (1.0 - dT2)*cPt1 + dT2*cPt2 + dr*cDir;
        cQuad.cPt3 = 0;
        cQuad.cPt4 = 0;

        return CropPrimitive(cQuad, pRect, pPrimList);
    }

    cQuad.iType = 4;

    int iCurRes;

    int iStartSeg = (int)(dT1 + g_dPrec);
    double dt1 = dT1 - iStartSeg;
    int iEndSeg = (int)(dT2 + g_dPrec);
    double dt2 = dT2 - iEndSeg;
    if((dt2 < g_dPrec) && (iEndSeg > 0))
    {
        iEndSeg--;
        dt2 = 1.0;
    }

    bool bm1, bm2;

    int k = iStartSeg;
    int iCurSeg = iStartSeg;
    bool bTurnedAround = false;

    bm1 = bClosed || (k > 0);
    CDPoint bPt1, bPt2, bPt3;
    bPt1 = pCache->GetPoint(k++, 0).cPoint;
    if(k > iCnt - 1)
    {
        k = 0;
        bTurnedAround = true;
    }
    bPt2 = pCache->GetPoint(k++, 0).cPoint;
    if(k > iCnt - 1)
    {
        k = 0;
        bTurnedAround = true;
    }
    bPt3 = pCache->GetPoint(k++, 0).cPoint;
    bm2 = bClosed || (k < iCnt);
    if(k > iCnt - 1)
    {
        k = 0;
        bTurnedAround = true;
    }

    if(bm1) cQuad.cPt1 = (bPt1 + bPt2)/2.0;
    else cQuad.cPt1 = bPt1;
    cQuad.cPt2 = bPt2;
    if(bm2) cQuad.cPt3 = (bPt2 + bPt3)/2.0;
    else cQuad.cPt3 = bPt3;

    if(iStartSeg == iEndSeg)
    {
        SubQuad(dt1, dt2, &cQuad);
        return CropQuad(cQuad, dr, pRect, pPrimList);
    }

    SubQuad(dt1, 1.0, &cQuad);
    int iRes = CropQuad(cQuad, dr, pRect, pPrimList);
    iCurSeg++;
    if(bTurnedAround && (k > 1)) iCurSeg = 0;

    if((dT2 < dT1) && !bTurnedAround)
    {
        while(k < iCnt)
        {
            cQuad.cPt1 = cQuad.cPt3;
            bPt2 = bPt3;
            bPt3 = pCache->GetPoint(k++, 0).cPoint;
            cQuad.cPt2 = bPt2;
            cQuad.cPt3 = (bPt2 + bPt3)/2.0;
            iCurRes = CropQuad(cQuad, dr, pRect, pPrimList);
            if(iRes != iCurRes) iRes = 1;
            iCurSeg++;
        }
        k = 0;
    }

    while(iCurSeg < iEndSeg)
    {
        if(k >= iCnt)
        {
            k = 0;
            bTurnedAround = true;
        }
        if(bTurnedAround && (k > 1)) iCurSeg = 0;
        cQuad.cPt1 = cQuad.cPt3;
        bPt2 = bPt3;
        bPt3 = pCache->GetPoint(k++, 0).cPoint;
        cQuad.cPt2 = bPt2;
        if(bClosed || (k < iCnt)) cQuad.cPt3 = (bPt2 + bPt3)/2.0;
        else cQuad.cPt3 = bPt3;

        iCurRes = CropQuad(cQuad, dr, pRect, pPrimList);
        if(iRes != iCurRes) iRes = 1;
        iCurSeg++;
    }

    if(k >= iCnt) k = 0;
    cQuad.cPt1 = cQuad.cPt3;
    bPt2 = bPt3;
    bPt3 = pCache->GetPoint(k++, 0).cPoint;
    cQuad.cPt2 = bPt2;
    if(bClosed || (k < iCnt)) cQuad.cPt3 = (bPt2 + bPt3)/2.0;
    else cQuad.cPt3 = bPt3;

    SubQuad(0.0, dt2, &cQuad);
    iCurRes = CropQuad(cQuad, dr, pRect, pPrimList);
    if(iRes != iCurRes) iRes = 1;

    return iRes;
}

int BuildSplinePrimitives(CDLine cTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly)
{
    if(iMode > 0) BuildSplineCache(cTmpPt, iMode, pPoints, pCache, pdDist);

    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return 0;

    int nCtrls = pCache->GetCount(1);
    bool bClosed = (nCtrls > 0);

    double dr = dOffset;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr += pCache->GetPoint(0, 2).cPoint.x;

    CDPrimitive cQuad;

    pDrawBnds->x = 0.0;
    pDrawBnds->y = 0.0;

    CDPoint bPt1, bPt2, bPt3;

    double dt1 = 0.0;
    double dt2 = 1.0;

    if(iCnt < 3)
    {
        bPt1 = pCache->GetPoint(0, 0).cPoint;
        bPt2 = pCache->GetPoint(1, 0).cPoint;

        pDrawBnds->y = GetDist(bPt1, bPt2);

        if(pBounds[0].bIsSet) dt1 = pBounds[0].dRef;
        if(pBounds[1].bIsSet) dt2 = pBounds[1].dRef;

        if(bQuadsOnly)
            return AddSplineSegmentQuadsWithBounds(dt1, dt2, pCache, pPrimList, pRect);
        return AddSplineSegmentWithBounds(dt1, dt2, pCache, pPrimList, pRect);
    }

    bPt1 = pCache->GetPoint(0, 0).cPoint;
    bPt2 = pCache->GetPoint(1, 0).cPoint;
    bPt3 = pCache->GetPoint(2, 0).cPoint;
    if(bClosed) cQuad.cPt1 = (bPt1 + bPt2)/2.0;
    else cQuad.cPt1 = bPt1;
    cQuad.cPt2 = bPt2;
    cQuad.cPt3 = (bPt2 + bPt3)/2.0;

    int iQuads = 0;

    for(int i = 3; i < iCnt; i++)
    {
        pDrawBnds->y += GetQuadBufLength(cQuad, dr, 0.0, 1.0);
        iQuads++;

        cQuad.cPt1 = cQuad.cPt3;
        bPt2 = bPt3;
        bPt3 = pCache->GetPoint(i, 0).cPoint;
        cQuad.cPt2 = bPt2;
        cQuad.cPt3 = (bPt2 + bPt3)/2.0;
    }

    if(bClosed)
    {
        pDrawBnds->y += GetQuadBufLength(cQuad, dr, 0.0, 1.0);
        iQuads++;

        cQuad.cPt1 = cQuad.cPt3;
        bPt2 = bPt3;
        bPt3 = pCache->GetPoint(0, 0).cPoint;
        cQuad.cPt2 = bPt2;
        cQuad.cPt3 = (bPt2 + bPt3)/2.0;

        pDrawBnds->y += GetQuadBufLength(cQuad, dr, 0.0, 1.0);
        iQuads++;

        cQuad.cPt1 = cQuad.cPt3;
        bPt2 = bPt3;
        bPt3 = pCache->GetPoint(1, 0).cPoint;
        cQuad.cPt2 = bPt2;
        cQuad.cPt3 = (bPt2 + bPt3)/2.0;
    }
    else cQuad.cPt3 = bPt3;

    pDrawBnds->y += GetQuadBufLength(cQuad, dr, 0.0, 1.0);
    iQuads++;

    dt2 = iQuads;

    if(bClosed)
    {
        if(pBounds[0].bIsSet && pBounds[1].bIsSet)
        {
            dt1 = pBounds[0].dRef;
            dt2 = pBounds[1].dRef;
        }
    }
    else
    {
        if(pBounds[0].bIsSet) dt1 = pBounds[0].dRef;
        if(pBounds[1].bIsSet) dt2 = pBounds[1].dRef;
    }

    if(bQuadsOnly)
        return AddSplineSegmentQuadsWithBounds(dt1, dt2, pCache, pPrimList, pRect);

    return AddSplineSegmentWithBounds(dt1, dt2, pCache, pPrimList, pRect);
}

double GetQuadAtRef(double dRef, bool bClosed, PDPrimitive pQuad, PDPointList pCache)
{
    int iCnt = pCache->GetCount(0);

    CDPoint bPt1, bPt2, bPt3;
    int k = (int)(dRef + g_dPrec);
    double dt = dRef - (double)k;
    if((k > 0) && (dt < g_dPrec))
    {
        k--;
        dt = 1.0;
    }

    bool bm1, bm2;
    bm1 = bClosed || (k > 0);
    bPt1 = pCache->GetPoint(k++, 0).cPoint;
    if(k >= iCnt) k = 0;
    bPt2 = pCache->GetPoint(k++, 0).cPoint;
    if(k >= iCnt) k = 0;
    bPt3 = pCache->GetPoint(k++, 0).cPoint;
    bm2 = bClosed || (k < iCnt);

    if(bm1) pQuad->cPt1 = (bPt1 + bPt2)/2.0;
    else pQuad->cPt1 = bPt1;
    pQuad->cPt2 = bPt2;
    if(bm2) pQuad->cPt3 = (bPt2 + bPt3)/2.0;
    else pQuad->cPt3 = bPt3;

    return dt;
}

double GetSplineDistFromPt(CDPoint cPt, CDPoint cRefPt, PDPointList pCache, PDLine pPtX)
{
    pPtX->bIsSet = false;

    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return 0.0;

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

/*wchar_t buf[64];
swprintf(buf, L"%d, %d", pBounds[0].iVal, pBounds[1].iVal);
SendMessage(g_hStatus, SB_SETTEXT, 2, (LPARAM)buf);*/

    CDPrimitive cQuad;
    double d1 = 0.0, d2;
    CDPoint bPt1, bPt2, cDir;

    if(iCnt < 3)
    {
        bPt1 = pCache->GetPoint(0, 0).cPoint;
        bPt2 = pCache->GetPoint(1, 0).cPoint;
        d2 = GetPtDistFromLineSeg(cPt, bPt1, bPt2, pPtX);
        pPtX->cOrigin += dr*pPtX->cDirection;
        return d2 - dr;
    }

    double dProj = GetSplinePtProj(cPt, cRefPt, pCache);
    if(dProj < -0.5)
    {
        if(!bClosed)
        {
            cQuad.cPt1 = pCache->GetPoint(0, 0).cPoint;
            cQuad.cPt2 = pCache->GetPoint(1, 0).cPoint;
            cQuad.cPt3 = pCache->GetPoint(2, 0).cPoint;
            cDir = GetQuadNormal(&cQuad, 0.0);
            bPt1 = cQuad.cPt1 + dr*cDir;
            bPt2 = Rotate(cPt - bPt1, cDir, false);
            d1 = GetDist(cPt, bPt1);
            if(bPt2.x < 0.0) d1 *= -1.0;

            pPtX->bIsSet = true;
            pPtX->cOrigin = bPt1;
            pPtX->cDirection = 0;
            pPtX->dRef = 0.0;

            cQuad.cPt1 = pCache->GetPoint(iCnt - 3, 0).cPoint;
            cQuad.cPt2 = pCache->GetPoint(iCnt - 2, 0).cPoint;
            cQuad.cPt3 = pCache->GetPoint(iCnt - 1, 0).cPoint;
            cDir = GetQuadNormal(&cQuad, 1.0);
            bPt1 = cQuad.cPt1 + dr*cDir;
            bPt2 = Rotate(cPt - bPt1, cDir, false);
            d2 = GetDist(cPt, bPt1);
            if(bPt2.x < 0.0) d2 *= -1.0;

            if(fabs(d2) < fabs(d1))
            {
                pPtX->cOrigin = bPt1;
                pPtX->cDirection = 0;
                pPtX->dRef = (double)iCnt;
                d1 = d2;
            }
            return d1;
        }
        return 0.0;
    }

    double dt = GetQuadAtRef(dProj, bClosed, &cQuad, pCache);

    pPtX->bIsSet = true;
    cDir = GetQuadNormal(&cQuad, dt);
    pPtX->cOrigin = GetQuadPoint(&cQuad, dt) + dr*cDir;
    bPt1 = cPt - pPtX->cOrigin;
    bPt2 = Rotate(bPt1, cDir, false);
    pPtX->cDirection = cDir;
    pPtX->dRef = dProj;

    if((dt < g_dPrec) || (dt > 1.0 - g_dPrec))
    {
        d2 = GetNorm(bPt1);
        if(bPt2.x < 0.0) d2 *= -1.0;
    }
    else d2 = bPt2.x;

    return d2;
}

bool HasSplineEnoughPoints(PDPointList pPoints)
{
    int nNorm = pPoints->GetCount(0);
    int nCtrl = pPoints->GetCount(1);

    bool bRes = (nNorm > 1);
    if(nCtrl > 0) bRes = (nNorm > 2); // spline is closed

    return bRes;
}

double GetSplineRadiusAtPt(CDLine cPtX, PDPointList pCache, PDLine pPtR, bool bNewPt)
{
    pPtR->bIsSet = false;
    pPtR->cOrigin = 0;
    pPtR->cDirection = 0;

    int iCnt = pCache->GetCount(0);

    if(iCnt < 2) return -1.0;

    CDPoint cDir, cQuad2;
    double dNorm;
    CDPrimitive cQuad, cQuad1;

    if(iCnt < 3)
    {
        cQuad.cPt1 = pCache->GetPoint(0, 0).cPoint;
        cQuad.cPt2 = pCache->GetPoint(1, 0).cPoint;
        cDir = cQuad.cPt2 - cQuad.cPt1;
        dNorm = GetNorm(cDir);
        if(dNorm > g_dPrec) pPtR->cDirection = GetNormal(cDir/dNorm);
        return -1.0;
    }

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);
    CDPoint bPt1, bPt2, bPt3;
    double dt = -1.0;

    //double dr = 0.0;
    //int nOffs = pCache->GetCount(2);
    //if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    if(bNewPt)
    {
        if(bClosed)
        {
            bPt1 = pCache->GetPoint(iCnt - 2, 0).cPoint;
            bPt2 = pCache->GetPoint(iCnt - 1, 0).cPoint;
            bPt3 = pCache->GetPoint(0, 0).cPoint;
            cQuad.cPt1 = (bPt1 + bPt2)/2.0;
            cQuad.cPt2 = bPt2;
            cQuad.cPt3 = (bPt2 + bPt3)/2.0;
            if(GetQuadPtProj(cPtX.cOrigin, cPtX.cOrigin, cQuad, &dt))
                dt += (double)(iCnt - 2);
        }
        else
        {
            bPt1 = pCache->GetPoint(iCnt - 3, 0).cPoint;
            bPt2 = pCache->GetPoint(iCnt - 2, 0).cPoint;
            bPt3 = pCache->GetPoint(iCnt - 1, 0).cPoint;
            if(iCnt > 3) cQuad.cPt1 = (bPt1 + bPt2)/2.0;
            else cQuad.cPt1 = bPt1;
            cQuad.cPt2 = bPt2;
            cQuad.cPt3 = bPt3;
            dt = 1.0;
        }
    }
    else
    {
        int k = (int)cPtX.dRef;
        dt = cPtX.dRef - (double)k;
        if(k < 0) k = 0;
        if(k > iCnt - 1) k = 0;
        bPt1 = pCache->GetPoint(k++, 0).cPoint;
        if(k > iCnt - 1) k = 0;
        bPt2 = pCache->GetPoint(k++, 0).cPoint;
        if(k > iCnt - 1) k = 0;
        bPt3 = pCache->GetPoint(k++, 0).cPoint;
        if(bClosed || (k - 3 > 0)) cQuad.cPt1 = (bPt1 + bPt2)/2.0;
        else cQuad.cPt1 = bPt1;
        cQuad.cPt2 = bPt2;
        if(bClosed || (k < iCnt)) cQuad.cPt3 = (bPt2 + bPt3)/2.0;
        else cQuad.cPt3 = bPt3;
    }

    if(dt < 0) return -1.0;
    cQuad1.cPt1 = 2.0*(cQuad.cPt2 - cQuad.cPt1);
    cQuad1.cPt2 = 2.0*(cQuad.cPt3 - cQuad.cPt2);
    cQuad2 = cQuad1.cPt2 - cQuad1.cPt1;

    bPt1 = GetQuadPoint(&cQuad, dt);
    bPt2 = (1.0 - dt)*cQuad1.cPt1 + dt*cQuad1.cPt2;
    bPt3 = cQuad2;

    double dDet = bPt2.x*bPt3.y - bPt2.y*bPt3.x;
    if(fabs(dDet) < g_dPrec)
    {
        cDir = cQuad.cPt3 - cQuad.cPt2;
        dNorm = GetNorm(cDir);
        if(dNorm > g_dPrec) pPtR->cDirection = GetNormal(cDir/dNorm);
        return -1.0;
    }

    double d1 = bPt2*bPt2;
    pPtR->bIsSet = true;
    pPtR->cOrigin.x = bPt1.x - d1*bPt2.y/dDet;
    pPtR->cOrigin.y = bPt1.y + d1*bPt2.x/dDet;
    cDir = cPtX.cOrigin - pPtR->cOrigin;
    dNorm = GetNorm(cDir);
    if(dNorm > g_dPrec) pPtR->cDirection = cDir/dNorm;

    return dNorm;
}

bool GetSplinePointRefDist(double dRef, PDPointList pCache, double *pdDist)
{
    int iCnt = pCache->GetCount(0);

    if(iCnt < 2) return false;

    CDPoint cDir;
    CDPrimitive cQuad;

    if(iCnt < 3)
    {
        cQuad.cPt1 = pCache->GetPoint(0, 0).cPoint;
        cQuad.cPt2 = pCache->GetPoint(1, 0).cPoint;
        cDir = cQuad.cPt2 - cQuad.cPt1;
        double dNorm = GetNorm(cDir);
        //if(dNorm < g_dPrec) return false;
        //cDir /= dNorm;
        //cPt1 = Rotate(cPtX.cOrigin - cQuad.cPt1, cDir, false);
//wchar_t buf[64];
//swprintf(buf, L"%f, %f", cPt1.x, cPtX.dVal);
//SendMessage(g_hStatus, SB_SETTEXT, 2, (LPARAM)buf);
        *pdDist = dRef*dNorm;
        return true;
    }

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    double dRes = 0.0;
    CDPoint bPt1, bPt2, bPt3;
    int iVal = (int)dRef;
    double dt = dRef - (double)iVal;
    int k = 0;

    bPt1 = pCache->GetPoint(0, 0).cPoint;
    bPt2 = pCache->GetPoint(1, 0).cPoint;
    bPt3 = pCache->GetPoint(2, 0).cPoint;
    if(bClosed) cQuad.cPt1 = (bPt1 + bPt2)/2.0;
    else cQuad.cPt1 = bPt1;
    cQuad.cPt2 = bPt2;
    cQuad.cPt3 = (bPt2 + bPt3)/2.0;

    while((k < iVal) && (k + 3 < iCnt))
    {
        dRes += GetQuadBufLength(cQuad, dr, 0.0, 1.0);
        bPt2 = bPt3;
        bPt3 = pCache->GetPoint(k + 3, 0).cPoint;
        cQuad.cPt1 = cQuad.cPt3;
        cQuad.cPt2 = bPt2;
        cQuad.cPt3 = (bPt2 + bPt3)/2.0;
        k++;
    }

    if(k + 3 >= iCnt)
    {
        if(bClosed)
        {
            if(k < iVal)
            {
                dRes += GetQuadBufLength(cQuad, dr, 0.0, 1.0);
                bPt2 = bPt3;
                bPt3 = pCache->GetPoint(0, 0).cPoint;
                cQuad.cPt1 = cQuad.cPt3;
                cQuad.cPt2 = bPt2;
                cQuad.cPt3 = (bPt2 + bPt3)/2.0;
                k++;
            }
            if(k < iVal)
            {
                dRes += GetQuadBufLength(cQuad, dr, 0.0, 1.0);
                bPt2 = bPt3;
                bPt3 = pCache->GetPoint(1, 0).cPoint;
                cQuad.cPt1 = cQuad.cPt3;
                cQuad.cPt2 = bPt2;
                cQuad.cPt3 = (bPt2 + bPt3)/2.0;
                k++;
            }
        }
        else cQuad.cPt3 = bPt3;
    }

    if(k == iVal)
    {
        dRes += GetQuadBufLength(cQuad, dr, 0.0, dt);
        *pdDist = dRes;
        return true;
    }
    return false;
}

double GetSplineRefAtDist(double dDist, PDPointList pCache)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return -1.0;

    if(dDist < g_dPrec) return 0.0;

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    CDPoint bPt1, bPt2, bPt3;

    if(iCnt < 3)
    {
        bPt1 = pCache->GetPoint(0, 0).cPoint;
        bPt2 = pCache->GetPoint(1, 0).cPoint;

        double dNorm = GetDist(bPt1, bPt2);
        if(dNorm < g_dPrec) return -1.0;

        return dDist/dNorm;
    }

    bPt1 = pCache->GetPoint(0, 0).cPoint;
    bPt2 = pCache->GetPoint(1, 0).cPoint;
    bPt3 = pCache->GetPoint(2, 0).cPoint;

    CDPrimitive cQuad;
    if(bClosed) cQuad.cPt1 = (bPt1 + bPt2)/2.0;
    else cQuad.cPt1 = bPt1;
    cQuad.cPt2 = bPt2;
    if(bClosed || (iCnt > 3)) cQuad.cPt3 = (bPt2 + bPt3)/2.0;
    else cQuad.cPt3 = bPt3;

    double d1 = GetQuadBufLength(cQuad, dr, 0.0, 1.0);
    bool bFound = (dDist < d1 + g_dPrec);

    int iQuads = 0;
    int i = 3;

    while(!bFound && (i < iCnt))
    {
        dDist -= d1;
        iQuads++;

        cQuad.cPt1 = cQuad.cPt3;
        bPt2 = bPt3;
        bPt3 = pCache->GetPoint(i++, 0).cPoint;
        cQuad.cPt2 = bPt2;
        if(bClosed || (i < iCnt)) cQuad.cPt3 = (bPt2 + bPt3)/2.0;
        else cQuad.cPt3 = bPt3;

        d1 = GetQuadBufLength(cQuad, dr, 0.0, 1.0);
        bFound = (dDist < d1 + g_dPrec);
    }

    if(bClosed)
    {
        if(!bFound)
        {
            dDist -= d1;
            iQuads++;

            cQuad.cPt1 = cQuad.cPt3;
            bPt2 = bPt3;
            bPt3 = pCache->GetPoint(0, 0).cPoint;
            cQuad.cPt2 = bPt2;
            cQuad.cPt3 = (bPt2 + bPt3)/2.0;

            d1 = GetQuadBufLength(cQuad, dr, 0.0, 1.0);
            bFound = (dDist < d1 + g_dPrec);
        }
        if(!bFound)
        {
            dDist -= d1;
            iQuads++;

            cQuad.cPt1 = cQuad.cPt3;
            bPt2 = bPt3;
            bPt3 = pCache->GetPoint(1, 0).cPoint;
            cQuad.cPt2 = bPt2;
            cQuad.cPt3 = (bPt2 + bPt3)/2.0;

            d1 = GetQuadBufLength(cQuad, dr, 0.0, 1.0);
            bFound = (dDist < d1 + g_dPrec);
        }
    }

    if(bFound)
        return (double)iQuads + GetQuadBufPointAtDist(cQuad, dr, 0.0, dDist);

    return (double)(iQuads + 1.0);
}

void AddSplineSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect)
{
    double dt1 = GetSplineRefAtDist(d1, pCache);
    double dt2 = GetSplineRefAtDist(d2, pCache);
    if((dt1 < -0.5) || (dt2 < -0.5)) return;

    AddSplineSegmentWithBounds(dt1, dt2, pCache, pPrimList, pRect);
}

bool GetSplineRefPoint(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return false;

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);

    double dr = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dr = pCache->GetPoint(0, 2).cPoint.x;

    CDPoint bPt1, bPt2;

    if(iCnt < 3)
    {
        bPt1 = pCache->GetPoint(0, 0).cPoint;
        bPt2 = pCache->GetPoint(1, 0).cPoint;

        CDPoint cDir = bPt2 - bPt1;
        double dNorm = GetNorm(cDir);
        if(dNorm < g_dPrec) return false;

        cDir = GetNormal(cDir/dNorm);
        *pPt = (1.0 - dRef)*bPt1 + dRef*bPt2 + dr*cDir;
        return true;
    }

    CDPrimitive cQuad;
    double dt = GetQuadAtRef(dRef, bClosed, &cQuad, pCache);

    CDPoint cDir = GetQuadNormal(&cQuad, dt);
    *pPt = GetQuadPoint(&cQuad, dt) + dr*cDir;
    return true;
}

bool GetSplineRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
    PDPointList pCache)
{
    if(iMode != 2) return false;

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);

    double dDist = 0.0;
    int nOffs = pCache->GetCount(2);
    if(nOffs > 0) dDist = pCache->GetPoint(0, 2).cPoint.y;

    double dRad = dDist + dRestrictValue;

    double dRef = GetSplinePtProj(cPt, cPt, pCache);
    CDPrimitive cQuad;
    double dt = GetQuadAtRef(dRef, bClosed, &cQuad, pCache);

    CDPoint cDir = GetQuadNormal(&cQuad, dt);
    *pSnapPt = GetQuadPoint(&cQuad, dt) + dRad*cDir;
    return true;
}

bool GetSplineRefDir(double dRef, PDPointList pCache, PDPoint pPt)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return false;

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);

    CDPoint bPt1, bPt2;

    if(iCnt < 3)
    {
        bPt1 = pCache->GetPoint(0, 0).cPoint;
        bPt2 = pCache->GetPoint(1, 0).cPoint;

        CDPoint cDir = bPt2 - bPt1;
        double dNorm = GetNorm(cDir);
        if(dNorm < g_dPrec) return false;

        *pPt = cDir/dNorm;
        return true;
    }

    CDPrimitive cQuad;
    double dt = GetQuadAtRef(dRef, bClosed, &cQuad, pCache);

    *pPt = GetQuadDir(&cQuad, dt);
    return true;
}

bool GetSplineReference(double dDist, PDPointList pCache, double *pdRef)
{
    int iCnt = pCache->GetCount(0);
    if(iCnt < 2) return false;

    int nCtrl = pCache->GetCount(1);
    bool bClosed = (nCtrl > 0);

    if(bClosed && (iCnt < 3)) return false;

    *pdRef = GetSplineRefAtDist(dDist, pCache);
    return true;
}

