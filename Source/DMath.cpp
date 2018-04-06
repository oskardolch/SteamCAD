#include "DMath.hpp"
#include <math.h>
#include <string.h>

double Power2(double x)
{
	return(x*x);
}

double Power3(double x)
{
	return(x*x*x);
}

double Power4(double x)
{
	return(Power2(x*x));
}

double PowerN(int n, double x)
{
    if(n == 0) return 1.0;
    double dDir = 1.0;
    if(x < 0)
    {
        dDir = -1.0;
        x *= dDir;
    }
    return dDir*exp(n*log(x));
}

// Polynomial with degree 0 is stil considered as polynomial - constant
// Even if the absolute coefficient is zero for degree 0, we will consider
// it as a polynomial of degree 0 and a_0 = 0
int GetPolyDegree(int iDeg, double *pdCoefs)
{
	while((iDeg > 0) && (fabs(pdCoefs[iDeg]) < g_dPrec))
	{
		iDeg--;
	}
	return(iDeg);
}

int ReducePoly(int iDeg, double *pdCoefs)
{
    while((iDeg > 0) && (fabs(pdCoefs[0]) < g_dPrec))
    {
        memmove(pdCoefs, &pdCoefs[1], iDeg*sizeof(double));
        iDeg--;
    }
    return iDeg;
}

// we will return root for any number and any degree
// it is up the caller to handle even roots from negative numbers
double RootN(double x, int n)
{
	bool bneg = x < -g_dPrec;
	if(bneg) x = -x;
	if(x < g_dPrec) return(0);
	double dlog = log(x)/((double)n);
	x = exp(dlog);
	if(bneg) x = -x;
	return(x);
}

// we will implement very simple algorithm to sort up to four
// numbers. Solution of equations of order greater than four will
// be sorted automatically
void SortFour(int iCnt, double *pnums)
{
	double dmin;
	int imin;
	for(int i = 0; i < iCnt; i++)
	{
		dmin = pnums[i];
		imin = i;
		for(int j = i + 1; j < iCnt; j++)
		{
			if(pnums[j] < dmin)
			{
				imin = j;
				dmin = pnums[j];
			}
		}
		if(i != imin)
		{
			pnums[imin] = pnums[i];
			pnums[i] = dmin;
		}
	}
	return;
}

// and we implement a method, which will select the ordered roots in the
// interval (0, 1) only
int CutRoots01(int iCnt, double *pdRoots)
{
	int i = 0;
	while((i < iCnt) && (pdRoots[i] < -g_dPrec)) i++;
	if(i == iCnt) return(0); // all roots are less than zero
	if(i > 0)
	{
		for(int j = 0; j < iCnt - i; j++)
		{
			pdRoots[j] = pdRoots[i + j];
		}
		iCnt -= i;
	}
	i = 0;
	while((i < iCnt) && (pdRoots[i] < (1 + g_dPrec))) i++;
	return(i);
}


int SolveConstant(double *pdCoefs, double *pdRoots)
{
	return(fabs(pdCoefs[0]) < g_dPrec ? -1 : 0);
}

int SolveLinear(double *pdCoefs, double *pdRoots)
{
	pdRoots[0] = -pdCoefs[0]/pdCoefs[1];
	return(1);
}

int SolveLinear01(double *pdCoefs, double *pdRoots)
{
	pdRoots[0] = -pdCoefs[0]/pdCoefs[1];
	if(pdRoots[0] < -g_dPrec) return(0);
	if(pdRoots[0] < g_dPrec) pdRoots[0] = 0.0;
	if(pdRoots[0] > 1 + g_dPrec) return(0);
	if(pdRoots[0] > 1 - g_dPrec) pdRoots[0] = 1.0;
	return(1);
}

int SolveQuadratic(double *pdCoefs, double *pdRoots)
{
	if(fabs(pdCoefs[0]) < g_dPrec)
	{
		pdRoots[0] = 0.0;
		return(SolveLinear(&pdCoefs[1], &pdRoots[1]) + 1);
	}

	double disc = Power2(pdCoefs[1]) - 4*pdCoefs[0]*pdCoefs[2];
	double denom = 2.0*pdCoefs[2];
	if(disc < -g_dPrec) return(0);
	if(disc < g_dPrec)
	{
		pdRoots[0] = -pdCoefs[1]/denom;
		return(1);
	}
	disc = sqrt(disc);
	pdRoots[0] = (-pdCoefs[1] - disc)/denom;
	pdRoots[1] = (-pdCoefs[1] + disc)/denom;
	SortFour(2, pdRoots);
	return(2);
}

int SolveCubic(double *pdCoefs, double *pdRoots)
{
	if(fabs(pdCoefs[0]) < g_dPrec)
	{
		pdRoots[0] = 0.0;
		int iRes = SolveQuadratic(&pdCoefs[1], &pdRoots[1]) + 1;
		SortFour(iRes, pdRoots);
		return(iRes);
	}

	double b3a = -pdCoefs[2]/pdCoefs[3]/3.0;
	double p3, q2;
	p3 = Power2(b3a) + (2.0*pdCoefs[2]*b3a + pdCoefs[1])/pdCoefs[3]/3.0;
	q2 = (Power3(b3a) + (pdCoefs[2]*Power2(b3a) + pdCoefs[1]*b3a +
		pdCoefs[0])/pdCoefs[3])/2.0;
	double disc = Power2(q2) + Power3(p3);
	double dres;

	if(disc < -g_dPrec) // three real roots
	{
		if(fabs(p3) < g_dPrec)
		{
			pdRoots[0] = b3a;
			return(1);
		}
		double u = sqrt(-4.0*p3);
		double cos3 = -8.0*q2/Power3(u);
		double phi = acos(cos3);
		pdRoots[0] = u*cos(phi/3.0) + b3a;
		pdRoots[1] = u*cos((2.0*M_PI - phi)/3.0) + b3a;
		pdRoots[2] = u*cos((2.0*M_PI + phi)/3.0) + b3a;
		SortFour(3, pdRoots);
		return(3);
	}
	if(disc < g_dPrec) // two real roots
	{
		dres = RootN(q2, 3);
		pdRoots[0] = dres + b3a;
		dres *= -2.0;
		pdRoots[1] = dres + b3a;
		SortFour(2, pdRoots);
		return(2);
	}
	// one real root
	disc = sqrt(disc);
	dres = RootN(-q2 + disc, 3) + RootN(-q2 - disc, 3);
	pdRoots[0] = dres + b3a;
	return(1);
}

int SolveQuartic(double *pdCoefs, double *pdRoots)
{
	if(fabs(pdCoefs[0]) < g_dPrec)
	{
		pdRoots[0] = 0.0;
		int iRes = SolveCubic(&pdCoefs[1], &pdRoots[1]) + 1;
		SortFour(iRes, pdRoots);
		return(iRes);
	}

	double b4a = -pdCoefs[3]/pdCoefs[4]/4.0;
	double p = 6.0*Power2(b4a) + (3.0*pdCoefs[3]*b4a + pdCoefs[2])/pdCoefs[4];
	double q = 4.0*Power3(b4a) +
		(3.0*pdCoefs[3]*Power2(b4a) + 2.0*pdCoefs[2]*b4a + pdCoefs[1])/pdCoefs[4];
	double r = Power4(b4a) + (pdCoefs[3]*Power3(b4a) + pdCoefs[2]*Power2(b4a) +
		pdCoefs[1]*b4a + pdCoefs[0])/pdCoefs[4];

	double dcub[4];
	double dcr[3];
	int irts;
	int iRes;
	double amax;

	if(fabs(q) < g_dPrec) // q = 0; so we can handle it as quadratic
	{
		dcub[0] = r;
		dcub[1] = p;
		dcub[2] = 1;
		irts = SolveQuadratic(dcub, dcr);
		if(irts > 0)
		{
			if(dcr[0] < -g_dPrec)
			{
				dcr[0] = dcr[1];
				irts--;
			}
		}
		if(irts > 0)
		{
			if(dcr[irts - 1] < -g_dPrec) irts--;
		}
		iRes = 0;
		for(int i = 0; i < irts; i++)
		{
			if(dcr[i] < g_dPrec)
			{
				pdRoots[iRes++] = b4a;
			}
			else
			{
			    amax = sqrt(dcr[i]);
				pdRoots[iRes] = b4a + amax;
				pdRoots[iRes + 1] = b4a - amax;
				iRes += 2;
			}
		}
		if(iRes > 1) SortFour(iRes, pdRoots);
		return(iRes);
	}

	dcub[0] = -Power2(q);
	dcub[1] = Power2(p) - 4.0*r;
	dcub[2] = 2.0*p;
	dcub[3] = 1.0;
	irts = SolveCubic(dcub, dcr);

	if(irts < 1) return(0); // some problem
	amax = dcr[0];
	for(int i = 1; i < irts; i++)
	{
		if(dcr[i] > amax) amax = dcr[i];
	}
	if(amax < -g_dPrec) return(0); // again some hardly believable problem
	if(amax < g_dPrec) return(0); // a = 0 means q = 0 which should not occure

	amax = sqrt(amax);

	dcub[0] = (p + Power2(amax) - q/amax)/2.0;
	dcub[1] = amax;
	dcub[2] = 1;
	iRes = SolveQuadratic(dcub, dcr);
	for(int i = 0; i < iRes; i++)
	{
		pdRoots[i] = dcr[i] + b4a;
	}
	dcub[0] += q/amax;
	dcub[1] = -amax;
	irts = SolveQuadratic(dcub, dcr);
	for(int i = 0; i < irts; i++)
	{
		pdRoots[iRes + i] = dcr[i] + b4a;
	}
	iRes += irts;
	if(iRes > 1) SortFour(iRes, pdRoots);
	return(iRes);
}

int SolvePolynom(int iDeg, double *pdCoefs, double *pdRoots)
{
	int itd = GetPolyDegree(iDeg, pdCoefs);
	if(itd < 1)
	{
		pdRoots[0] = 0.0;
		return(1);
	}

	if(fabs(pdCoefs[0]) < g_dPrec)
	{
		pdRoots[0] = 0.0;
		return(SolvePolynom(itd - 1, &pdCoefs[1], &pdRoots[1]) + 1);
	}

	switch(itd)
	{
		case 0:
			return(SolveConstant(pdCoefs, pdRoots));
		case 1:
			return(SolveLinear(pdCoefs, pdRoots));
		case 2:
			return(SolveQuadratic(pdCoefs, pdRoots));
		case 3:
			return(SolveCubic(pdCoefs, pdRoots));
		case 4:
			return(SolveQuartic(pdCoefs, pdRoots));
	}

	return(0);
}

int SolvePolynom01(int iDeg, double *pdCoefs, double *pdRoots)
{
	int itd = GetPolyDegree(iDeg, pdCoefs);
	int iRes = 0;
	switch(itd)
	{
		case 0:
			return(SolveConstant(pdCoefs, pdRoots));
		case 1:
			return(SolveLinear01(pdCoefs, pdRoots));
		case 2:
			iRes = SolveQuadratic(pdCoefs, pdRoots);
			break;
		case 3:
			iRes = SolveCubic(pdCoefs, pdRoots);
			break;
		case 4:
			iRes = SolveQuartic(pdCoefs, pdRoots);
			break;
	}
	return(CutRoots01(iRes, pdRoots));
}

int MultiplyPolynoms(int iDeg1, int iDeg2, double *pCoefs1, double *pCoefs2,
	double *pCoefsRes)
{
	int n = 1 + iDeg1 + iDeg2;
	int iPos;
	for(int i = 0; i < n; i++)
	{
		pCoefsRes[i] = 0.0;
		for(int j = 0; j < 1 + iDeg2; j++)
		{
			iPos = i - j;
			if((iPos > -1) && (iPos < 1 + iDeg1))
			{
				pCoefsRes[i] += pCoefs1[iPos]*pCoefs2[j];
			}
		}
	}
	return(n - 1);
}

void MultPolyByConst(int iDeg, double *pCoefs, double x)
{
	for(int i = 0; i <= iDeg; i++) pCoefs[i] *= x;
	return;
}

int AddPolynoms(int iDeg1, int iDeg2, double *pCoefs1, double *pCoefs2,
	double *pCoefsRes)
{
	int iRes = iDeg1;
	if(iRes < iDeg2)
	{
		iRes = iDeg2;
		for(int i = 0; i <= iDeg1; i++) pCoefsRes[i] = pCoefs1[i] + pCoefs2[i];
		for(int i = iDeg1 + 1; i <= iRes; i++) pCoefsRes[i] = pCoefs2[i];
	}
	else
	{
		for(int i = 0; i <= iDeg2; i++) pCoefsRes[i] = pCoefs1[i] + pCoefs2[i];
		for(int i = iDeg2 + 1; i <= iRes; i++) pCoefsRes[i] = pCoefs1[i];
	}
	return(iRes);
}

int AddPolynomsMult(int iDeg1, int iDeg2, double *pCoefs1, double *pCoefs2,
	double xmul, double *pCoefsRes)
{
	int iRes = iDeg1;
	if(iRes < iDeg2)
	{
		iRes = iDeg2;
		for(int i = 0; i <= iDeg1; i++)
			pCoefsRes[i] = pCoefs1[i] + xmul*pCoefs2[i];
		for(int i = iDeg1 + 1; i <= iRes; i++) pCoefsRes[i] = xmul*pCoefs2[i];
	}
	else
	{
		for(int i = 0; i <= iDeg2; i++)
			pCoefsRes[i] = pCoefs1[i] + xmul*pCoefs2[i];
		for(int i = iDeg2 + 1; i <= iRes; i++) pCoefsRes[i] = pCoefs1[i];
	}
	return(iRes);
}

int AddPolynomsMult2(int iDeg1, int iDeg2, double *pCoefs1, double *pCoefs2,
	double xmul1, double xmul2, double *pCoefsRes)
{
	int iRes = iDeg1;
	if(iRes < iDeg2)
	{
		iRes = iDeg2;
		for(int i = 0; i <= iDeg1; i++)
			pCoefsRes[i] = xmul1*pCoefs1[i] + xmul2*pCoefs2[i];
		for(int i = iDeg1 + 1; i <= iRes; i++) pCoefsRes[i] = xmul2*pCoefs2[i];
	}
	else
	{
		for(int i = 0; i <= iDeg2; i++)
			pCoefsRes[i] = xmul1*pCoefs1[i] + xmul2*pCoefs2[i];
		for(int i = iDeg2 + 1; i <= iRes; i++) pCoefsRes[i] = xmul1*pCoefs1[i];
	}
	return(iRes);
}

double EvaluatePolynom(int iDeg, double *pCoefs, double x)
{
	double dRes = pCoefs[iDeg];
	while(iDeg > 0)
	{
		dRes *= x;
		dRes += pCoefs[--iDeg];
	}
	return(dRes);
}

double operator*(const CDPoint3& p1, const CDPoint3& p2)
{
	return(p1.x*p2.x + p1.y*p2.y + p1.z*p2.z);
}

bool Solve3x3Matrix(CDPoint3 cMat1, CDPoint3 cMat2, CDPoint3 cMat3, CDPoint3 cB,
    PDPoint3 pSol)
{
    double dDet = cMat1.x*cMat2.y*cMat3.z + cMat1.y*cMat2.z*cMat3.x + cMat1.z*cMat2.x*cMat3.y -
        cMat1.z*cMat2.y*cMat3.x - cMat1.y*cMat2.x*cMat3.z - cMat1.x*cMat2.z*cMat3.y;

    if(fabs(dDet) < g_dPrec) return false;

    CDPoint3 cMatInv1, cMatInv2, cMatInv3;

    cMatInv1.x = cMat2.y*cMat3.z - cMat2.z*cMat3.y;
    cMatInv1.y = cMat2.z*cMat3.x - cMat2.x*cMat3.z;
    cMatInv1.z = cMat2.x*cMat3.y - cMat2.y*cMat3.x;

    cMatInv2.x = cMat1.z*cMat3.y - cMat1.y*cMat3.z;
    cMatInv2.y = cMat1.x*cMat3.z - cMat1.z*cMat3.x;
    cMatInv2.z = cMat1.y*cMat3.x - cMat1.x*cMat3.y;

    cMatInv3.x = cMat1.y*cMat2.z - cMat1.z*cMat2.y;
    cMatInv3.y = cMat1.z*cMat2.x - cMat1.x*cMat2.z;
    cMatInv3.z = cMat1.x*cMat2.y - cMat1.y*cMat2.x;

    pSol->x = cMatInv1*cB/dDet;
    pSol->y = cMatInv2*cB/dDet;
    pSol->z = cMatInv3*cB/dDet;

    return true;
}
