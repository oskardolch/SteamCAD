#ifndef _DHYPER_HPP_
#define _DHYPER_HPP_

#include "DDataTypes.hpp"

bool AddHyperPoint(double x, double y, char iCtrl, PDPointList pPoints, int iInputLines);
bool BuildHyperCache(CDLine cTmpPt, int iMode, PDPointList pPoints, PDPointList pCache,
    PDLine pLines, double *pdDist);
int BuildHyperPrimitives(CDLine cTmpPt, int iMode, PDRect pRect, PDPointList pPoints,
    PDPointList pCache, PDPrimObject pPrimList, PDLine pLines, PDRefPoint pBounds, double dOffset,
    double *pdDist, PDPoint pDrawBnds, bool bQuadsOnly = false);
double GetHyperDistFromPt(CDPoint cPt, CDPoint cRefPt, int iSrchMask, PDPointList pCache, PDLine pPtX, PDRefPoint pBounds);
bool HasHyperEnoughPoints(PDPointList pPoints, int iInputLines);
double GetHyperRadiusAtPt(CDPoint cPt, PDPointList pCache, PDLine pPtR, bool bNewPt);
bool GetHyperPointRefDist(double dRef, PDPointList pCache, double *pdDist);
void AddHyperSegment(double d1, double d2, PDPointList pCache, PDPrimObject pPrimList, PDRect pRect);
bool GetHyperRefPoint(double dRef, PDPointList pCache, PDPoint pPt);
bool GetHyperRestrictPoint(CDPoint cPt, int iMode, double dRestrictValue, PDPoint pSnapPt,
    PDPointList pCache);
bool GetHyperRefDir(double dRef, PDPointList pCache, PDPoint pPt);
bool GetHyperReference(double dDist, PDPointList pCache, double *pdRef);
int GetHyperNumParts(PDPointList pCache, PDRefPoint pBounds);
bool HyperRemovePart(bool bDown, PDPointList pCache, PDRefPoint pBounds);

#endif
