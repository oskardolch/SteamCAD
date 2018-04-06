#ifndef _DTOPO_HPP_
#define _DTOPO_HPP_

typedef struct CDPoint
{
    double x;
    double y;
	CDPoint& operator+=(const CDPoint& p1);
	CDPoint& operator-=(const CDPoint& p1);
	CDPoint& operator*=(const double& d1);
	CDPoint& operator/=(const double& d1);
    CDPoint& operator=(const double& d1);
} *PDPoint;

double operator*(const CDPoint& p1, const CDPoint& p2);
CDPoint operator+(const CDPoint& p1, const CDPoint& p2);
CDPoint operator-(const CDPoint& p1, const CDPoint& p2);
CDPoint operator*(const double& d1, const CDPoint& p2);
CDPoint operator/(const CDPoint& p1, const double& d2);

typedef CDPoint (*DFunc2D)(CDPoint cParam1, CDPoint cParam2, CDPoint cArg);

double GetNorm(CDPoint cPt);
double GetDist(CDPoint cPt1, CDPoint cPt2);
CDPoint VectProd(CDPoint cp1, CDPoint cp2);
CDPoint GetNormal(CDPoint cPt1);

int LineXLine(bool b01, CDPoint cPt1, CDPoint cDir1, CDPoint cPt2, CDPoint cDir2, PDPoint pRes);
int CircXLine(bool b01, CDPoint p11, double dRad, CDPoint p21, CDPoint p22, PDPoint pRes);
int QuadXLine(CDPoint p11, CDPoint p12, CDPoint p13,
    CDPoint p21, CDPoint p22, PDPoint pRes, double *pdTs);
int BezXLine(CDPoint p11, CDPoint p12, CDPoint p13, CDPoint p14,
    CDPoint p21, CDPoint p22, PDPoint pRes, double *pdTs);

double GetCircOrigin(CDPoint cp1, CDPoint cp2, CDPoint cp3, PDPoint pRes);

int BiQuadricXLine(double *pdCoefs, CDPoint p21, CDPoint p22, PDPoint pRes);
// UBLine = Unbound line
int UBLineXLine(double *pdCoefs, CDPoint p21, CDPoint p22, PDPoint pRes);
void SubstituteBiQuad(double *pdCoefs, CDPoint cRot, CDPoint cShift);

#endif
