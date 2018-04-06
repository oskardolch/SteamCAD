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
int MergeBounds(double da1, double da2, double db1, double db2, double *pdBnds);
int AddBoundCurve(double da, double db, double dr, CurveFunc pFunc, CurveFunc pFuncDer,
    double dt1, double dt2, CDPoint cOrig, CDPoint cMainDir, PDRect pRect,
    PDPrimObject pPrimList);
int AddBoundQuadCurve(double da, double db, double dr, CurveFunc pFunc, CurveFunc pFuncDer,
    double dt1, double dt2, CDPoint cOrig, CDPoint cMainDir, PDRect pRect,
    PDPrimObject pPrimList);
CDPoint GetCurveRefAtDist(double da, double db, double dr, double dDist,
    CurveFunc pFunc, CurveFunc pFuncDer);
CDPoint GetPrimRegion(CDPrimitive cPrim, double dLineWidth, double dScale,
    PDPoint pPoints1, PDPoint pPoints2);

#endif
