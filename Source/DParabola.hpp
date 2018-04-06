#ifndef _DPARABOLA_HPP_
#define _DPARABOLA_HPP_

#include "DDataTypes.hpp"

bool AddParabPoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines);
bool BuildParabCache(PDPoint pTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    PDLine pLines, double *pdDist);
int BuildParabPrimitives(PDPoint pTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly = false);
double GetParabDistFromPt(CDPoint cPt, CDPoint cRefPt, PDPointList pCache, PDLine pPtX);
bool HasParabEnoughPoints(PDPointList pPoints, int iInputLines);
double GetParabRadiusAtPt(CDPoint cPt, PDPointList pCache, PDLine pPtR, bool bNewPt);
bool GetParabPointRefDist(double dRef, PDPointList pCache, double *pdDist);
void AddParabSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect);
bool GetParabRefPoint(double dRef, PDPointList pCache, PDPoint pPt);
bool GetParabRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
    PDPointList pCache);
bool GetParabRefDir(double dRef, PDPointList pCache, PDPoint pPt);
bool GetParabReference(double dDist, PDPointList pCache, double *pdRef);

#endif
