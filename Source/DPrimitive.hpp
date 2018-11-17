#ifndef _DPRIMITIVE_HPP_
#define _DPRIMITIVE_HPP_

#include "DDataTypes.hpp"

typedef CDPoint (CurveFunc)(double da, double db, double dt);

int CmpAngle(CDPoint cPt1, CDPoint cPt2);
int CropPrimitive(CDPrimitive cPrim, PDRect pRect, PDPrimObject pPrimList);
double ApproxLineSeg(int iPoints, PDPoint pPoints, PDPoint pStartDir,
	PDPoint pEndDir, PDPrimitive pPrim);
bool PointInArc(CDPoint cPt, CDLine cStart, CDLine cEnd);
CDPoint GetQuadPoint(PDPrimitive pQuad, double dt);
CDPoint GetQuadDir(PDPrimitive pQuad, double dt);
CDPoint GetQuadNormal(PDPrimitive pQuad, double dt);
double GetQuadLength(PDPrimitive pQuad, double t1, double t2);
double GetQuadPointAtDist(PDPrimitive pQuad, double t1, double dDist);
// returns 3 if it is inside, 2 if it coinidents with da2, 1 if it coincidents with da1, 0 otherwise
int RefInBounds(double da1, double da2, double dRef);
int RefInOpenBounds(PDRefPoint pBounds, double dRef);
int MergeBounds(double da1, double da2, double db1, double db2, bool bFullCycle, double *pdBnds);
int AddBoundCurve(double da, double db, double dr, CurveFunc pFunc, CurveFunc pFuncDer,
    double dt1, double dt2, CDPoint cOrig, CDPoint cMainDir, PDRect pRect,
    PDPrimObject pPrimList);
int AddBoundQuadCurve(double da, double db, double dr, CurveFunc pFunc, CurveFunc pFuncDer,
    double dt1, double dt2, CDPoint cOrig, CDPoint cMainDir, PDRect pRect,
    PDPrimObject pPrimList);
CDPoint GetCurveRefAtDist(double da, double db, double dr, double dBreak, double dDist,
    CurveFunc pFunc, CurveFunc pFuncDer, PDRefPoint pBounds);
CDPoint GetPrimRegion(CDPrimitive cPrim, double dLineWidth, double dScale,
    PDPoint pPoints1, PDPoint pPoints2);

#endif
