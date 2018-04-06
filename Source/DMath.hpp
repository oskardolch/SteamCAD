#ifndef _DMATH_HPP_
#define _DMATH_HPP_

const double g_dPrec = 0.000001;
const double g_dRootPrec = 0.0000000001;
const double g_dCrossPrec = 0.000000000000000000001;
const double g_dBezCirc = 0.55196764827565714118;

typedef struct CDPoint3
{
    double x;
    double y;
    double z;
} *PDPoint3;

double operator*(const CDPoint3& p1, const CDPoint3& p2);

double Power2(double x);
double Power3(double x);
double Power4(double x);
double PowerN(int n, double x);

int SolvePolynom(int iDeg, double *pdCoefs, double *pdRoots);
int SolvePolynom01(int iDeg, double *pdCoefs, double *pdRoots);

int GetPolyDegree(int iDeg, double *pdCoefs);
int ReducePoly(int iDeg, double *pdCoefs);
int MultiplyPolynoms(int iDeg1, int iDeg2, double *pCoefs1, double *pCoefs2,
	double *pCoefsRes);
int AddPolynoms(int iDeg1, int iDeg2, double *pCoefs1, double *pCoefs2,
	double *pCoefsRes);
int AddPolynomsMult(int iDeg1, int iDeg2, double *pCoefs1, double *pCoefs2,
	double xmul, double *pCoefsRes);
int AddPolynomsMult2(int iDeg1, int iDeg2, double *pCoefs1, double *pCoefs2,
	double xmul1, double xmul2, double *pCoefsRes);
double EvaluatePolynom(int iDeg, double *pCoefs, double x);

bool Solve3x3Matrix(CDPoint3 cMat1, CDPoint3 cMat2, CDPoint3 cMat3, CDPoint3 cB,
    PDPoint3 pSol);

#endif
