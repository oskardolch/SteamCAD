#ifndef _DARCELPS_HPP_
#define _DARCELPS_HPP_

#include "DDataTypes.hpp"

bool AddArcElpsPoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines);
bool BuildArcElpsCache(CDLine cTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    PDLine pLines, double *pdMovedDist);
int BuildArcElpsPrimitives(CDLine cTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdMovedDist, PDPoint pDrawBnds);
double GetArcElpsDistFromPt(CDPoint cPt, CDPoint cRefPt, int iSrchMask, PDPointList pCache, PDLine pPtX, PDRefPoint pBounds);
bool HasArcElpsEnoughPoints(PDPointList pPoints, int iInputLines);
bool GetArcElpsRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
    PDPointList pCache);
double GetArcElpsRadiusAtPt(CDPoint cPt, PDPointList pCache, PDLine pPtR, bool bNewPt,
    PDPointList pPoints, PDLine pLines);
bool GetArcElpsPointRefDist(double dRef, PDPointList pCache, double *pdDist);
void AddArcElpsSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect);
bool GetArcElpsRefPoint(double dRef, PDPointList pCache, PDPoint pPt);
bool GetArcElpsRefDir(double dRef, PDPointList pCache, PDPoint pPt);
bool GetArcElpsReference(double dDist, PDPointList pCache, double *pdRef);
int GetArcElpsNumParts(PDPointList pCache, PDRefPoint pBounds);
bool ArcElpsRemovePart(bool bDown, PDPointList pCache, PDRefPoint pBounds);

#endif
