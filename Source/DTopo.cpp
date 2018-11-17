#include "DTopo.hpp"
#include "DMath.hpp"
#include <math.h>

CDPoint& CDPoint::operator+=(const CDPoint& p1)
{
    this->x += p1.x;
    this->y += p1.y;
    return(*this);
}

CDPoint& CDPoint::operator-=(const CDPoint& p1)
{
    this->x -= p1.x;
    this->y -= p1.y;
    return(*this);
}

CDPoint& CDPoint::operator*=(const double& d1)
{
    this->x *= d1;
    this->y *= d1;
    return(*this);
}

CDPoint& CDPoint::operator/=(const double& d1)
{
    this->x /= d1;
    this->y /= d1;
    return(*this);
}

CDPoint& CDPoint::operator=(const double& d1)
{
    this->x = d1;
    this->y = d1;
    return(*this);
}

CDPoint operator+(const CDPoint& p1, const CDPoint& p2)
{
    CDPoint cRes;
    cRes.x = p1.x + p2.x;
    cRes.y = p1.y + p2.y;
    return(cRes);
}

CDPoint operator-(const CDPoint& p1, const CDPoint& p2)
{
    CDPoint cRes;
    cRes.x = p1.x - p2.x;
    cRes.y = p1.y - p2.y;
    return(cRes);
}

double operator*(const CDPoint& p1, const CDPoint& p2)
{
    return(p1.x*p2.x + p1.y*p2.y);
}

CDPoint operator*(const double& d1, const CDPoint& p2)
{
    CDPoint cRes;
    cRes.x = d1*p2.x;
    cRes.y = d1*p2.y;
    return(cRes);
}

CDPoint operator/(const CDPoint& p1, const double& d2)
{
    CDPoint cRes;
    cRes.x = p1.x/d2;
    cRes.y = p1.y/d2;
    return(cRes);
}


double GetNorm(CDPoint cPt)
{
    return sqrt(cPt*cPt);
}

double GetDist(CDPoint cPt1, CDPoint cPt2)
{
    return GetNorm(cPt2 - cPt1);
}

CDPoint VectProd(CDPoint cp1, CDPoint cp2)
{
    CDPoint cRes;
    cRes.x = cp1.y - cp2.y;
    cRes.y = cp2.x - cp1.x;
    return cRes;
}

CDPoint GetNormal(CDPoint cPt1)
{
    CDPoint cRes = {-cPt1.y, cPt1.x};
    return cRes;
}


int MultPolySubt(int iDeg11, int iDeg12, int iDeg21, int iDeg22,
    double *pdCoef11, double *pdCoef12, double *pdCoef21, double *pdCoef22,
    double *pdCoefRes)
{
    double pdCfMult1[5], pdCfMult2[5];
    int iDeg1 = MultiplyPolynoms(iDeg11, iDeg22, pdCoef11, pdCoef22, pdCfMult1);
    int iDeg2 = MultiplyPolynoms(iDeg12, iDeg21, pdCoef12, pdCoef21, pdCfMult2);
    return AddPolynomsMult(iDeg1, iDeg2, pdCfMult1, pdCfMult2, -1.0, pdCoefRes);
}

// BiLinear is a system of two polynomials in the form
// x1 + x2*s + x3*t = 0
// y1 + y2*s + y3*t = 0
int SolveBiLinear(PDPoint ppCoefs, PDPoint ppRes)
{
    double dDet = ppCoefs[1].x*ppCoefs[2].y - ppCoefs[1].y*ppCoefs[2].x;
    if(fabs(dDet) < g_dPrec) return 0;

    ppRes[0].x = (ppCoefs[2].x*ppCoefs[0].y - ppCoefs[2].y*ppCoefs[0].x)/dDet;
    ppRes[0].y = (ppCoefs[1].y*ppCoefs[0].x - ppCoefs[1].x*ppCoefs[0].y)/dDet;
    return 1;
}

// BiQuadratic is a system of two polynomials in the form
// x1 + x2*s + x3*t + x4*s^2 + x5*s*t + x6*t^2 = 0
// y1 + y2*s + y3*t + y4*s^2 + y5*s*t + y6*t^2 = 0
int SolveBiQuadratics(PDPoint ppCoefs, bool b01, PDPoint ppRes)
{
    double dPolyX1[3], dPolyY1[3];
    double dPolyX2[2], dPolyY2[2];
    double dPolyX3, dPolyY3;

    dPolyX3 = ppCoefs[5].x;
    dPolyY3 = ppCoefs[5].y;
    dPolyX2[0] = ppCoefs[2].x;
    dPolyY2[0] = ppCoefs[2].y;
    dPolyX2[1] = ppCoefs[4].x;
    dPolyY2[1] = ppCoefs[4].y;
    dPolyX1[0] = ppCoefs[0].x;
    dPolyY1[0] = ppCoefs[0].y;
    dPolyX1[1] = ppCoefs[1].x;
    dPolyY1[1] = ppCoefs[1].y;
    dPolyX1[2] = ppCoefs[3].x;
    dPolyY1[2] = ppCoefs[3].y;

    int iDegX1 = GetPolyDegree(2, dPolyX1);
    int iDegX2 = GetPolyDegree(1, dPolyX2);
    int iDegX3 = GetPolyDegree(0, &dPolyX3);
    int iDegY1 = GetPolyDegree(2, dPolyY1);
    int iDegY2 = GetPolyDegree(1, dPolyY2);
    int iDegY3 = GetPolyDegree(0, &dPolyY3);

    double dPolMult11[3], dPolMult12[2];

    int iDeg11 = MultPolySubt(iDegX1, iDegX3, iDegY1, iDegY3,
        dPolyX1, &dPolyX3, dPolyY1, &dPolyY3, dPolMult11);
    int iDeg12 = MultPolySubt(iDegX2, iDegX3, iDegY2, iDegY3,
        dPolyX2, &dPolyX3, dPolyY2, &dPolyY3, dPolMult12);

    double dPolMult21[4]; //, dPolMult22[3]; dPolMult22 = dPolMult11

    int iDeg21 = MultPolySubt(iDegX1, iDegX2, iDegY1, iDegY2,
        dPolyX1, dPolyX2, dPolyY1, dPolyY2, dPolMult21);
    //int iDeg22 = MultPolySubt(2, 0, 2, 0, dPolyX1, dPolyX3, dPolyY1, dPolyY3, dPolMult22);

    double dPolMultRes[5];
    int iDegRes = MultPolySubt(iDeg11, iDeg12, iDeg21, iDeg11,
        dPolMult11, dPolMult12, dPolMult21, dPolMult11, dPolMultRes);

    int iRoots, iDeg, iRoots2, iSol;
    double dRoots[4], dRoots2[2];
    double dPoly[3];

    if(b01)
    {
        iSol = 0;
        iRoots = SolvePolynom01(iDegRes, dPolMultRes, dRoots);
        for(int i = 0; i < iRoots; i++)
        {
            dPoly[0] = EvaluatePolynom(iDegX1, dPolyX1, dRoots[i]);
            dPoly[1] = EvaluatePolynom(iDegX2, dPolyX2, dRoots[i]);
            dPoly[2] = EvaluatePolynom(iDegX3, &dPolyX3, dRoots[i]);

            iDeg = GetPolyDegree(2, dPoly);
            iRoots2 = SolvePolynom01(iDeg, dPoly, dRoots2);

            ppRes[iSol].x = dRoots[i];
            if(iRoots2 > 0) ppRes[iSol++].y = dRoots2[0];
        }
    }
    else
    {
        iSol = 0;
        iRoots = SolvePolynom(iDegRes, dPolMultRes, dRoots);
        for(int i = 0; i < iRoots; i++)
        {
            dPoly[0] = EvaluatePolynom(iDegX1, dPolyX1, dRoots[i]);
            dPoly[1] = EvaluatePolynom(iDegX2, dPolyX2, dRoots[i]);
            dPoly[2] = EvaluatePolynom(iDegX3, &dPolyX3, dRoots[i]);

            iDeg = GetPolyDegree(2, dPoly);
            iRoots2 = SolvePolynom(iDeg, dPoly, dRoots2);

            ppRes[iSol].x = dRoots[i];
            if(iRoots2 > 0) ppRes[iSol++].y = dRoots2[0];
        }
    }

    return iSol;
}

int SegXSeg(CDPoint cPt11, CDPoint cPt12, CDPoint cPt21, CDPoint cPt22, PDPoint pRes)
{
    CDPoint cCoefs[3];
    cCoefs[0] = cPt11 - cPt21;
    cCoefs[1] = cPt12 - cPt11;
    cCoefs[2] = cPt21 - cPt22;

    CDPoint cRes;
    int iRes = SolveBiLinear(cCoefs, &cRes);
    if(iRes < 1) return 0;
    if(cRes.x < -g_dPrec) return 0;
    if(cRes.x > g_dPrec) return 0;
    if(cRes.y < -g_dPrec) return 0;
    if(cRes.y > g_dPrec) return 0;

    *pRes = cPt11 + cRes.x*cCoefs[1];
    return 1;
}

int LineXSeg(CDPoint cLnOrg, CDPoint cLnDir, CDPoint cPt1, CDPoint cPt2, PDPoint pRes)
{
    CDPoint cCoefs[3];
    cCoefs[0] = cLnOrg - cPt1;
    cCoefs[1] = cLnDir;
    cCoefs[2] = cPt1 - cPt2;

    CDPoint cRes;
    int iRes = SolveBiLinear(cCoefs, &cRes);
    if(iRes < 1) return 0;
    if(cRes.y < -g_dPrec) return 0;
    if(cRes.y > g_dPrec) return 0;

    *pRes = cLnOrg + cRes.x*cLnDir;
    return 1;
}

int LineXLine(CDPoint cPt1, CDPoint cDir1, CDPoint cPt2, CDPoint cDir2, PDPoint pRes)
{
    CDPoint cCoefs[3];
    cCoefs[0] = cPt1 - cPt2;
    cCoefs[1] = cDir1;
    cCoefs[2] = -1.0*cDir2;

    CDPoint cRes;
    int iRes = SolveBiLinear(cCoefs, &cRes);

    if(iRes > 0) *pRes = cPt1 + cRes.x*cDir1;

    return iRes;
}

int CircXLine(bool b01, CDPoint p11, double dRad, CDPoint p21, CDPoint p22, PDPoint pRes)
{
    double dPolyX1[2], dPolyY1[2];
    dPolyX1[0] = p21.x - p11.x;
    dPolyX1[1] = p22.x - p21.x;
    dPolyY1[0] = p21.y - p11.y;
    dPolyY1[1] = p22.y - p21.y;
    int iDegX1 = GetPolyDegree(1, dPolyX1);
    int iDegY1 = GetPolyDegree(1, dPolyY1);

    double dPolyX2[3], dPolyY2[3];
    int iDegX2 = MultiplyPolynoms(iDegX1, iDegX1, dPolyX1, dPolyX1, dPolyX2);
    int iDegY2 = MultiplyPolynoms(iDegY1, iDegY1, dPolyY1, dPolyY1, dPolyY2);

    double dPoly[3];
    int iDeg = AddPolynomsMult(iDegX2, iDegY2, dPolyX2, dPolyY2, 1.0, dPoly);
    dPoly[0] -= Power2(dRad);

    int iRoots;
    double dRoots[2];

    if(b01) iRoots = SolvePolynom01(iDeg, dPoly, dRoots);
    else iRoots = SolvePolynom(iDeg, dPoly, dRoots);

    int iSol = 0;
    for(int i = 0; i < iRoots; i++)
    {
        pRes[iSol++] = (1.0 - dRoots[i])*p21 + dRoots[i]*p22;
    }

    return iSol;
}

int QuadXSeg(PDPoint pQuad, CDPoint cPt1, CDPoint cPt2, PDPoint pRes, double *pdTs)
{
    CDPoint cCoefs[4];
    cCoefs[0] = pQuad[0] - cPt1;
    cCoefs[1] = 2.0*(pQuad[1] - pQuad[0]);
    cCoefs[2] = pQuad[2] - 2.0*pQuad[1] + pQuad[0];
    cCoefs[3] = cPt2 - cPt1;

    double pPoly11[3], pPoly12[3];
    for(int i = 0; i < 3; i++)
    {
        pPoly11[i] = cCoefs[i].x;
        pPoly12[i] = cCoefs[i].y;
    }

    int iDeg1, iDeg2, iRoots;
    double dRoots[2], dVal;
    int iSol = 0;

    iDeg1 = GetPolyDegree(2, pPoly11);
    iDeg2 = GetPolyDegree(2, pPoly12);

    if(fabs(cCoefs[3].x) < g_dPrec)
    {
        if(fabs(cCoefs[3].y) < g_dPrec) return 0;

        iRoots = SolvePolynom01(iDeg1, pPoly11, dRoots);

        for(int i = 0; i < iRoots; i++)
        {
            dVal = EvaluatePolynom(iDeg2, pPoly12, dRoots[i])/cCoefs[3].y;
            if((dVal > -g_dPrec) && (dVal < 1.0 + g_dPrec))
            {
                pdTs[iSol] = dRoots[i];
                pRes[iSol++] = cPt1 + dVal*cCoefs[3];
            }
        }

        return iSol;
    }

    if(fabs(cCoefs[3].y) < g_dPrec)
    {
        iRoots = SolvePolynom01(iDeg2, pPoly12, dRoots);

        for(int i = 0; i < iRoots; i++)
        {
            dVal = EvaluatePolynom(iDeg1, pPoly11, dRoots[i])/cCoefs[3].x;
            if((dVal > -g_dPrec) && (dVal < 1.0 + g_dPrec))
            {
                pdTs[iSol] = dRoots[i];
                pRes[iSol++] = cPt1 + dVal*cCoefs[3];
            }
        }

        return iSol;
    }

    double dPolyRes[3];
    int iDegRes = MultPolySubt(iDeg1, iDeg2, 0, 0, pPoly11, pPoly12,
        &cCoefs[3].x, &cCoefs[3].y, dPolyRes);

    iRoots = SolvePolynom01(iDegRes, dPolyRes, dRoots);

    for(int i = 0; i < iRoots; i++)
    {
        dVal = EvaluatePolynom(iDeg1, pPoly11, dRoots[i])/cCoefs[3].x;
        if((dVal > -g_dPrec) && (dVal < 1.0 + g_dPrec))
        {
            pdTs[iSol] = dRoots[i];
            pRes[iSol++] = cPt1 + dVal*cCoefs[3];
        }
    }

    return iSol;
}

int QuadXLine(PDPoint pQuad, CDPoint cLnOrg, CDPoint cLnDir, PDPoint pRes, double *pdTs)
{
    CDPoint cCoefs[3];
    cCoefs[0] = pQuad[0] - cLnOrg;
    cCoefs[1] = 2.0*(pQuad[1] - pQuad[0]);
    cCoefs[2] = pQuad[2] - 2.0*pQuad[1] + pQuad[0];

    double pPoly11[3], pPoly12[3];
    for(int i = 0; i < 3; i++)
    {
        pPoly11[i] = cCoefs[i].x;
        pPoly12[i] = cCoefs[i].y;
    }

    int iDeg1, iDeg2, iRoots;
    double dRoots[2], dVal;
    int iSol = 0;

    iDeg1 = GetPolyDegree(2, pPoly11);
    iDeg2 = GetPolyDegree(2, pPoly12);

    if(fabs(cLnDir.x) < g_dPrec)
    {
        if(fabs(cLnDir.y) < g_dPrec) return 0;

        iRoots = SolvePolynom01(iDeg1, pPoly11, dRoots);

        for(int i = 0; i < iRoots; i++)
        {
            dVal = EvaluatePolynom(iDeg2, pPoly12, dRoots[i])/cLnDir.y;
            pdTs[iSol] = dRoots[i];
            pRes[iSol++] = cLnOrg + dVal*cLnDir;
        }

        return iSol;
    }

    if(fabs(cLnDir.y) < g_dPrec)
    {
        iRoots = SolvePolynom01(iDeg2, pPoly12, dRoots);

        for(int i = 0; i < iRoots; i++)
        {
            dVal = EvaluatePolynom(iDeg1, pPoly11, dRoots[i])/cLnDir.x;
            pdTs[iSol] = dRoots[i];
            pRes[iSol++] = cLnOrg + dVal*cLnDir;
        }

        return iSol;
    }

    double dPolyRes[3];
    int iDegRes = MultPolySubt(iDeg1, iDeg2, 0, 0, pPoly11, pPoly12,
        &cLnDir.x, &cLnDir.y, dPolyRes);

    iRoots = SolvePolynom01(iDegRes, dPolyRes, dRoots);

    for(int i = 0; i < iRoots; i++)
    {
        dVal = EvaluatePolynom(iDeg1, pPoly11, dRoots[i])/cLnDir.x;
        pdTs[iSol] = dRoots[i];
        pRes[iSol++] = cLnOrg + dVal*cLnDir;
    }

    return iSol;
}

int BezXLine(CDPoint p11, CDPoint p12, CDPoint p13, CDPoint p14,
    CDPoint p21, CDPoint p22, PDPoint pRes, double *pdTs)
{
    CDPoint cCoefs[5];
    cCoefs[0] = p11 - p21;
    cCoefs[1] = 3.0*(p12 - p11);
    cCoefs[2] = 3.0*(p13 - 2.0*p12 + p11);
    cCoefs[3] = p14 - 3.0*p13 + 3.0*p12 - p11;
    cCoefs[4] = p22 - p21;

    double pPoly11[4], pPoly12[4];
    for(int i = 0; i < 4; i++)
    {
        pPoly11[i] = cCoefs[i].x;
        pPoly12[i] = cCoefs[i].y;
    }

    int iDeg1, iDeg2, iRoots;
    double dRoots[3], dVal;
    int iSol = 0;

    iDeg1 = GetPolyDegree(3, pPoly11);
    iDeg2 = GetPolyDegree(3, pPoly12);

    if(fabs(cCoefs[4].x) < g_dPrec)
    {
        if(fabs(cCoefs[4].y) < g_dPrec) return 0;

        iRoots = SolvePolynom01(iDeg1, pPoly11, dRoots);

        for(int i = 0; i < iRoots; i++)
        {
            dVal = EvaluatePolynom(iDeg2, pPoly12, dRoots[i])/cCoefs[4].y;
            pdTs[iSol] = dRoots[i];
            pRes[iSol++] = p21 + dVal*cCoefs[4];
        }

        return iSol;
    }

    if(fabs(cCoefs[4].y) < g_dPrec)
    {
        iRoots = SolvePolynom01(iDeg2, pPoly12, dRoots);

        for(int i = 0; i < iRoots; i++)
        {
            dVal = EvaluatePolynom(iDeg1, pPoly11, dRoots[i])/cCoefs[4].x;
            pdTs[iSol] = dRoots[i];
            pRes[iSol++] = p21 + dVal*cCoefs[4];
        }

        return iSol;
    }

    double dPolyRes[4];
    int iDegRes = MultPolySubt(iDeg1, iDeg2, 0, 0, pPoly11, pPoly12,
        &cCoefs[4].x, &cCoefs[4].y, dPolyRes);

    iRoots = SolvePolynom01(iDegRes, dPolyRes, dRoots);

    for(int i = 0; i < iRoots; i++)
    {
        dVal = EvaluatePolynom(iDeg1, pPoly11, dRoots[i])/cCoefs[4].x;
        pdTs[iSol] = dRoots[i];
        pRes[iSol++] = p21 + dVal*cCoefs[4];
    }

    return iSol;
}

double GetCircOrigin(CDPoint cp1, CDPoint cp2, CDPoint cp3, PDPoint pRes)
{
    CDPoint cMid1 = (cp1 + cp2)/2;
    CDPoint cMid2 = (cp2 + cp3)/2;

    double dDist = GetDist(cMid1, cMid2);
    if(dDist < g_dPrec) return false;

    CDPoint cDir1 = VectProd(cp1, cp2);
    CDPoint cDir2 = VectProd(cp2, cp3);

    int iX = LineXLine(cMid1, cDir1, cMid2, cDir2, pRes);

    if(iX > 0) return GetDist(cp1, *pRes);

    return -1.0;
}

int BiQuadricXLine(double *pdCoefs, CDPoint p21, CDPoint p22, PDPoint pRes)
{
    double dCoefs[3];
    CDPoint cDir = p22 - p21;
    dCoefs[0] = pdCoefs[0] + pdCoefs[1]*p21.x + pdCoefs[2]*p21.y +
        pdCoefs[3]*Power2(p21.x) + pdCoefs[4]*p21.x*p21.y + pdCoefs[5]*Power2(p21.y);
    dCoefs[1] = pdCoefs[1]*cDir.x + pdCoefs[2]*cDir.y + 2.0*pdCoefs[3]*p21.x*cDir.x +
        pdCoefs[4]*(p21.x*cDir.y + p21.y*cDir.x) + 2.0*pdCoefs[5]*p21.y*cDir.y;
    dCoefs[2] = pdCoefs[3]*Power2(cDir.x) + pdCoefs[4]*cDir.x*cDir.y + pdCoefs[5]*Power2(cDir.y);

    double dRoots[2];
    int iRes = SolvePolynom01(2, dCoefs, dRoots);

    for(int i = 0; i < iRes; i++)
    {
        pRes[i] = p21 + dRoots[i]*cDir;
    }

    return iRes;
}

int UBLineXLine(double *pdCoefs, CDPoint p21, CDPoint p22, PDPoint pRes)
{
    double dCoefs[2];
    CDPoint cDir = p22 - p21;
    dCoefs[0] = pdCoefs[0] + pdCoefs[1]*p21.x + pdCoefs[2]*p21.y;
    dCoefs[1] = pdCoefs[1]*cDir.x + pdCoefs[2]*cDir.y;

    double dRoot;
    int iRes = SolvePolynom01(1, dCoefs, &dRoot);

    if(iRes > 0) *pRes = p21 + dRoot*cDir;

    return iRes;
}

void SubstituteBiQuad(double *pdCoefs, CDPoint cRot, CDPoint cShift)
{
    double dCoefs1[6];
    dCoefs1[0] = pdCoefs[0];
    dCoefs1[1] = pdCoefs[1]*cRot.x - pdCoefs[2]*cRot.y;
    dCoefs1[2] = pdCoefs[1]*cRot.y + pdCoefs[2]*cRot.x;
    dCoefs1[3] = pdCoefs[3]*Power2(cRot.x) - pdCoefs[4]*cRot.x*cRot.y + pdCoefs[5]*Power2(cRot.y);
    dCoefs1[4] = 2.0*pdCoefs[3]*cRot.x*cRot.y + pdCoefs[4]*(Power2(cRot.x) - Power2(cRot.y)) -
        2.0*pdCoefs[5]*cRot.x*cRot.y;
    dCoefs1[5] = pdCoefs[3]*Power2(cRot.y) + pdCoefs[4]*cRot.x*cRot.y + pdCoefs[5]*Power2(cRot.x);

    pdCoefs[0] = dCoefs1[0] - dCoefs1[1]*cShift.x - dCoefs1[2]*cShift.y +
        dCoefs1[3]*Power2(cShift.x) + dCoefs1[4]*cShift.x*cShift.y + dCoefs1[5]*Power2(cShift.y);
    pdCoefs[1] = dCoefs1[1] - 2.0*dCoefs1[3]*cShift.x - dCoefs1[4]*cShift.y;
    pdCoefs[2] = dCoefs1[2] - dCoefs1[4]*cShift.x - 2.0*dCoefs1[5]*cShift.y;
    pdCoefs[3] = dCoefs1[3];
    pdCoefs[4] = dCoefs1[4];
    pdCoefs[5] = dCoefs1[5];
}

