#ifndef _DSPLINE_HPP_
#define _DSPLINE_HPP_

#include "DDataTypes.hpp"

bool AddSplinePoint(double x, double y, char iCtrl, PDPointList pPoints);
bool BuildSplineCache(CDLine cTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    double *pdDist);
int BuildSplinePrimitives(CDLine cTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly = false);
double GetSplineDistFromPt(CDPoint cPt, CDPoint cRefPt, PDPointList pCache, PDLine pPtX);
bool HasSplineEnoughPoints(PDPointList pPoints);
double GetSplineRadiusAtPt(CDLine cPtX, PDPointList pCache, PDLine pPtR, bool bNewPt);
bool GetSplinePointRefDist(double dRef, PDPointList pCache, double *pdDist);
void AddSplineSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect);
bool GetSplineRefPoint(double dRef, PDPointList pCache, PDPoint pPt);
bool GetSplineRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
    PDPointList pCache);
bool GetSplineRefDir(double dRef, PDPointList pCache, PDPoint pPt);
bool GetSplineReference(double dDist, PDPointList pCache, double *pdRef);

#endif
