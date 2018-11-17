#ifndef _DELLIPSE_HPP_
#define _DELLIPSE_HPP_

#include "DDataTypes.hpp"

bool AddEllipsePoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines);
bool BuildEllipseCache(CDLine cTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    PDLine pLines, double *pdDist);
int BuildEllipsePrimitives(CDLine cTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly = false);
double GetElpsDistFromPt(CDPoint cPt, CDPoint cRefPt, int iSrchMask, PDPointList pCache, PDLine pPtX, PDRefPoint pBounds);
bool GetElpsRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
    PDPointList pCache);
bool HasElpsEnoughPoints(PDPointList pPoints, int iInputLines);
double GetElpsRadiusAtPt(CDPoint cPt, PDPointList pCache, PDLine pPtR, bool bNewPt,
    PDPointList pPoints, PDLine pLines);
bool GetElpsPointRefDist(double dRef, PDPointList pCache, double *pdDist);
double GetElpsPointAtDist(double da, double db, double dr, double dBreak, double dDist);
void AddElpsSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect);
bool GetElpsRefPoint(double dRef, PDPointList pCache, PDPoint pPt);
bool GetElpsRefDir(double dRef, PDPointList pCache, PDPoint pPt);
bool GetElpsReference(double dDist, PDPointList pCache, double *pdRef);
int GetElpsNumParts(PDPointList pCache, PDRefPoint pBounds);
bool ElpsRemovePart(bool bDown, PDPointList pCache, PDRefPoint pBounds);

#endif
