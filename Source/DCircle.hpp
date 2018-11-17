#ifndef _DCIRCLE_HPP_
#define _DCIRCLE_HPP_

#include "DDataTypes.hpp"

bool AddCirclePoint(double x, double y, char iCtrl, PDPointList pPoints, PDLine pLines);
bool BuildCircCache(CDLine cTmpPt, int iMode, PDPointList pPoints, PDPointList pCache, PDLine pLines,
    double *pdMovedDist);
int BuildCircPrimitives(CDLine cTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdMovedDist, PDPoint pDrawBnds);
double GetCircDistFromPt(CDPoint cPt, CDPoint cRefPt, bool bSnapCenters, PDPointList pCache, PDLine pPtX);
bool HasCircEnoughPoints(PDPointList pPoints, int iInputLines);
bool GetCircleRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
    PDPointList pCache, PDPointList pPoints, PDLine pLines);
double GetCircRadiusAtPt(CDPoint cPt, PDPointList pCache, PDLine pPtR, bool bNewPt);
bool GetCirceRad(PDPointList pCache, double *pdVal);
bool GetCircPointRefDist(double dRef, PDPointList pCache, double *pdDist);
void AddCircSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect);
bool GetCircRefPoint(double dRef, PDPointList pCache, PDPoint pPt);
bool GetCircRefDir(double dRef, PDPointList pCache, PDPoint pPt);
bool GetCircReference(double dDist, PDPointList pCache, double *pdRef);

#endif
