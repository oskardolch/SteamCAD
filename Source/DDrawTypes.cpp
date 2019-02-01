#include "DDrawTypes.hpp"
#include "DMath.hpp"
#include <math.h>
#include "DLine.hpp"
#include "DCircle.hpp"
#include "DEllipse.hpp"
#include "DPrimitive.hpp"
#include "DArcElps.hpp"
#include "DHyper.hpp"
#include "DParabola.hpp"
#include "DSpline.hpp"
#include "DEvolv.hpp"
#include <malloc.h>
#include <string.h>

// for debugging purpose only
/*#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
extern HWND g_hStatus;*/
// -----


void SwapBytes(unsigned char *pDest, unsigned char *pSrc, int iLen, bool bSwap)
{
    if(bSwap)
    {
        for(int i = 0; i < iLen; i++) pDest[i] = pSrc[iLen - i - 1];
    }
    else for(int i = 0; i < iLen; i++) pDest[i] = pSrc[i];
}

// CDObject

CDObject::CDObject(CDDrawType iType, double dWidth)
{
    m_iDataSize = 0;
    m_iDataLen = 0;
    m_pGroupObjects = NULL;

    if(iType == dtGroup)
    {
        m_iDataSize = 16;
        m_pGroupObjects = (PDObject*)malloc(m_iDataSize*sizeof(PDObject));
    }

    m_iType = iType;
    m_iSubType = dstNone;
    m_pInputPoints = new CDPointList();
    m_pUndoPoints = new CDPointList();
    m_pCachePoints = new CDPointList();
    m_pCrossPoints = new CDRefList();
    m_pPrimitive = new CDPrimObject();
    m_pDimens = new CDPtrList();
    m_iPrimitiveRequested = 0;
    m_bSelected = false;
    m_cLines[0].bIsSet = false;
    m_cLines[1].bIsSet = false;
    m_cBounds[0].bIsSet = false;
    m_cBounds[1].bIsSet = false;
    m_cLineStyle.dWidth = dWidth;
    m_cLineStyle.dPercent = 0.0;
    m_cLineStyle.bCapType = 0;
    m_cLineStyle.iSegments = 0;
    for(int i = 0; i < 6; i++) m_cLineStyle.dPattern[i] = 0.0;
    m_dMovedDist = 0.0;
    m_bFirstDimSet = false;
    m_cTmpDim.psLab = m_sTmpDimBuf;
    m_sTmpDimBuf[0] = 0;
    m_bSnapTo = true;
}

CDObject::~CDObject()
{
    if(m_iDataSize > 0)
    {
        ClearGroups();
        free(m_pGroupObjects);
    }

    PDDimension pDim;
    for(int i = 0; i < m_pDimens->GetCount(); i++)
    {
        pDim = (PDDimension)m_pDimens->GetItem(i);
        if(pDim->psLab) free(pDim->psLab);
        free(pDim);
    }
    delete m_pDimens;

    delete m_pPrimitive;
    delete m_pCrossPoints;
    delete m_pCachePoints;
    delete m_pUndoPoints;
    delete m_pInputPoints;
}

void CDObject::ClearGroups()
{
    for(int i = 0; i < m_iDataLen; i++)
    {
        delete m_pGroupObjects[i];
    }
    m_iDataLen = 0;
}

void CDObject::AddChild(CDObject *pObj)
{
    if(m_iDataLen >= m_iDataSize)
    {
        m_iDataSize += 16;
        m_pGroupObjects = (PDObject*)realloc(m_pGroupObjects, m_iDataSize*sizeof(PDObject));
    }
    m_pGroupObjects[m_iDataLen++] = pObj;
}

void CDObject::SetSubType(CDDrawSubType iSubType)
{
    if(m_iSubType == iSubType) return;
    ClearGroups();
    m_iSubType = iSubType;

    PDObject pChild;
    switch(m_iSubType)
    {
    case dstRectangle:
        pChild = new CDObject(dtLine, m_cLineStyle.dWidth);
        AddChild(pChild);
        pChild = new CDObject(dtLine, m_cLineStyle.dWidth);
        AddChild(pChild);
        pChild = new CDObject(dtLine, m_cLineStyle.dWidth);
        AddChild(pChild);
        pChild = new CDObject(dtLine, m_cLineStyle.dWidth);
        AddChild(pChild);
        break;
    default:
        break;
    }
}

bool CDObject::AddPoint(double x, double y, char iCtrl, bool bFromGui)
{
    bool bRes = false;
    int iInputLines = 0;
    if(m_cLines[0].bIsSet) iInputLines++;
    if(m_cLines[1].bIsSet) iInputLines++;

    switch(m_iType)
    {
    case dtLine:
        bRes = AddLinePoint(x, y, iCtrl, m_pInputPoints);
        break;
    case dtCircle:
        bRes = AddCirclePoint(x, y, iCtrl, m_pInputPoints, m_cLines);
        break;
    case dtEllipse:
        bRes = AddEllipsePoint(x, y, iCtrl, m_pInputPoints, iInputLines);
        break;
    case dtArcEllipse:
        bRes = AddArcElpsPoint(x, y, iCtrl, m_pInputPoints, iInputLines);
        break;
    case dtHyperbola:
        bRes = AddHyperPoint(x, y, iCtrl, m_pInputPoints, iInputLines);
        break;
    case dtParabola:
        bRes = AddParabPoint(x, y, iCtrl, m_pInputPoints, iInputLines);
        break;
    case dtSpline:
        bRes = AddSplinePoint(x, y, iCtrl, m_pInputPoints);
        break;
    case dtEvolvent:
        bRes = AddEvolvPoint(x, y, iCtrl, m_pInputPoints, iInputLines);
        break;
    case dtGroup:
        bRes = AddGroupPoint(x, y, iCtrl);
        break;
    }

    return bRes;
}

bool CDObject::AddRectanglePoint(double x, double y, char iCtrl)
{
    m_pInputPoints->AddPoint(x, y, iCtrl);
    if(m_pInputPoints->GetCount(-1) < 2)
    {
        m_pGroupObjects[0]->AddPoint(x, y, 0, false);
        m_pGroupObjects[0]->AddPoint(x + 1.0, y, 0, false);
        m_pGroupObjects[1]->AddPoint(x, y, 0, false);
        m_pGroupObjects[1]->AddPoint(x, y + 1.0, 0, false);
        m_pGroupObjects[2]->AddPoint(x, y, 0, false);
        m_pGroupObjects[2]->AddPoint(x - 1.0, y, 0, false);
        m_pGroupObjects[3]->AddPoint(x, y, 0, false);
        m_pGroupObjects[3]->AddPoint(x, y - 1.0, 0, false);
    }
    else
    {
/*        CDInputPoint cFirstPt = m_pInputPoints->GetPoint(0, -1);
        m_pGroupObjects[0]->AddPoint(cFirstPt.cPoint.x, y, 0, false);
        m_pGroupObjects[1]->AddPoint(cFirstPt.cPoint.x, cFirstPt.cPoint.y, 0, false);
        m_pGroupObjects[2]->AddPoint(x, cFirstPt.cPoint.y, 0, false);
        m_pGroupObjects[3]->AddPoint(cFirstPt.cPoint.x, cFirstPt.cPoint.y, 0, false);*/
    }
    return (m_pInputPoints->GetCount(-1) == 2);
}

bool CDObject::AddGroupPoint(double x, double y, char iCtrl)
{
    switch(m_iSubType)
    {
    case dstRectangle:
        return AddRectanglePoint(x, y, iCtrl);
    default:
        return false;
    }
}

void CDObject::RemoveLastPoint()
{
    int iCnt = m_pInputPoints->GetCount(-1);
    if(iCnt < 1) return;

    CDInputPoint cInPt = m_pInputPoints->GetPoint(iCnt - 1, -1);
    m_pUndoPoints->AddPoint(cInPt.cPoint.x, cInPt.cPoint.y, cInPt.iCtrl);
    m_pInputPoints->Remove(iCnt - 1, -1);
}

void CDObject::Undo()
{
    RemoveLastPoint();
}

void CDObject::Redo()
{
    int iCnt = m_pUndoPoints->GetCount(-1);
    if(iCnt < 1) return;

    CDInputPoint cInPt = m_pUndoPoints->GetPoint(iCnt - 1, -1);
    m_pInputPoints->AddPoint(cInPt.cPoint.x, cInPt.cPoint.y, cInPt.iCtrl);
    m_pUndoPoints->Remove(iCnt - 1, -1);
}

bool CDObject::BuildGroupCache(CDLine cTmpPt, int iMode)
{
    if(m_iDataLen < 1) return false;

    PDObject pObj = m_pGroupObjects[0];
    bool bRes = pObj->BuildCache(cTmpPt, iMode);
    if(iMode == 2)
    {
        pObj->GetDynValue(cTmpPt.cOrigin, 2, &m_dMovedDist);
        double dMoveDist;
        for(int i = 1; i < m_iDataLen; i++)
        {
            pObj = m_pGroupObjects[i];
            bRes &= pObj->BuildCache(cTmpPt, iMode);
            pObj->GetDynValue(cTmpPt.cOrigin, 2, &dMoveDist);
            if(fabs(dMoveDist) < fabs(m_dMovedDist)) m_dMovedDist = dMoveDist;
        }
    }
    else
    {
        for(int i = 1; i < m_iDataLen; i++)
        {
            pObj = m_pGroupObjects[i];
            bRes &= pObj->BuildCache(cTmpPt, iMode);
        }
    }
    return bRes;
}

bool CDObject::BuildCache(CDLine cTmpPt, int iMode)
{
    switch(m_iType)
    {
    case dtLine:
        return BuildLineCache(cTmpPt, iMode, m_pInputPoints, m_pCachePoints, &m_dMovedDist);
    case dtCircle:
        return BuildCircCache(cTmpPt, iMode, m_pInputPoints, m_pCachePoints, m_cLines, &m_dMovedDist);
    case dtEllipse:
        return BuildEllipseCache(cTmpPt, iMode, m_pInputPoints, m_pCachePoints, m_cLines, &m_dMovedDist);
    case dtArcEllipse:
        return BuildArcElpsCache(cTmpPt, iMode, m_pInputPoints, m_pCachePoints, m_cLines, &m_dMovedDist);
    case dtHyperbola:
        return BuildHyperCache(cTmpPt, iMode, m_pInputPoints, m_pCachePoints, m_cLines, &m_dMovedDist);
    case dtParabola:
        return BuildParabCache(cTmpPt, iMode, m_pInputPoints, m_pCachePoints, m_cLines, &m_dMovedDist);
    case dtSpline:
        return BuildSplineCache(cTmpPt, iMode, m_pInputPoints, m_pCachePoints, &m_dMovedDist);
    case dtEvolvent:
        return BuildEvolvCache(cTmpPt, iMode, m_pInputPoints, m_pCachePoints, m_cLines, &m_dMovedDist);
    case dtGroup:
        return BuildGroupCache(cTmpPt, iMode);
    default:
        return false;
    }
}

void CDObject::AddGroupSegment(double dStart, double dEnd, PDRect pRect)
{
}

void CDObject::AddCurveSegment(double dStart, double dEnd, PDRect pRect)
{
    switch(m_iType)
    {
    case dtLine:
        AddLineSegment(dStart, dEnd, m_pCachePoints, m_pPrimitive, pRect);
        break;
    case dtCircle:
        AddCircSegment(dStart, dEnd, m_pCachePoints, m_pPrimitive, pRect);
        break;
    case dtEllipse:
        AddElpsSegment(dStart, dEnd, m_pCachePoints, m_pPrimitive, pRect);
        break;
    case dtArcEllipse:
        AddArcElpsSegment(dStart, dEnd, m_pCachePoints, m_pPrimitive, pRect);
        break;
    case dtHyperbola:
        AddHyperSegment(dStart, dEnd, m_pCachePoints, m_pPrimitive, pRect);
        break;
    case dtParabola:
        AddParabSegment(dStart, dEnd, m_pCachePoints, m_pPrimitive, pRect);
        break;
    case dtSpline:
        AddSplineSegment(dStart, dEnd, m_pCachePoints, m_pPrimitive, pRect);
        break;
    case dtEvolvent:
        AddEvolvSegment(dStart, dEnd, m_pCachePoints, m_pPrimitive, pRect);
        break;
    case dtGroup:
        AddGroupSegment(dStart, dEnd, pRect);
        break;
    }
}

void CDObject::AddPatSegment(double dStart, int iStart, double dEnd, int iEnd,
    PDPoint pBnds, PDRect pRect)
{
    double dPatScale = 1.0;
    double dPatStart2 = m_cLineStyle.dPattern[0]/2.0;
    int iRep;

    double dDist = dEnd - dStart;
    double dPatLen = 0;
    double dPerLen = 0;
    for(int i = 0; i < m_cLineStyle.iSegments; i++)
        dPatLen += m_cLineStyle.dPattern[i];

    if(dStart > dEnd)
    {
        dPerLen = pBnds->y - pBnds->x;
        dDist = dPerLen - dStart + dEnd;
    }

    if((iStart > 0) && (iEnd > 0))
    {
        if(iStart > 1) dDist -= dPatStart2;
        if(iEnd > 1) dDist -= dPatStart2;

        double dRep = dDist/dPatLen;
        if(dRep < 0.8)
        {
            AddCurveSegment(dStart, dEnd, pRect);
            return;
        }

        iRep = Round(dRep);
        if(dStart > dEnd) dDist = dPerLen - dStart + dEnd;
        else dDist = dEnd - dStart;
        double dStretchDist = iRep*dPatLen;
        if(iStart > 1) dStretchDist += dPatStart2;
        if(iEnd > 1) dStretchDist += dPatStart2;

        dPatScale = dDist/dStretchDist;
    }
    else iRep = dDist/dPatLen + 1;

    double d1, d2;
    int i;
    if(iStart > 0)
    {
        i = 0;
        d1 = dStart;
        d2 = d1 + dPatScale*m_cLineStyle.dPattern[i++];
        if(iStart < 2) d2 = d1 + dPatScale*dPatStart2;
        AddCurveSegment(d1, d2, pRect);

        d1 = d2 + dPatScale*m_cLineStyle.dPattern[i++];
        for(int j = 0; j < iRep; j++)
        {
            while(i < m_cLineStyle.iSegments)
            {
                if(d1 > pBnds->y) d1 -= dPerLen;
                d2 = d1 + dPatScale*m_cLineStyle.dPattern[i++];
                AddCurveSegment(d1, d2, pRect);
                d1 = d2 + dPatScale*m_cLineStyle.dPattern[i++];
            }
            i = 0;
        }
        if(d1 > pBnds->y) d1 -= dPerLen;
        d2 = d1 + dPatScale*m_cLineStyle.dPattern[0];
        if((iEnd > 0) && (iEnd < 2)) d2 = d1 + dPatScale*dPatStart2;
        AddCurveSegment(d1, d2, pRect);
    }
    else // iEnd should be > 0
    {
        d2 = dEnd;
        d1 = d2 - dPatScale*m_cLineStyle.dPattern[0];
        if(iEnd < 2) d1 = d2 - dPatScale*dPatStart2;
        AddCurveSegment(d1, d2, pRect);

        for(int j = 0; j < iRep; j++)
        {
            i = m_cLineStyle.iSegments;
            while(i > 0)
            {
                d2 = d1 - dPatScale*m_cLineStyle.dPattern[--i];
                d1 = d2 - dPatScale*m_cLineStyle.dPattern[--i];
                AddCurveSegment(d1, d2, pRect);
            }
        }
    }
}

int CDObject::AddDimenPrimitive(int iPos, PDDimension pDim, PDPrimObject pPrimitive, PDRect pRect)
{
    CDPrimitive cPrim;
    cPrim.iType = 9;
    CDPoint cDir, bPt1;

    if(pDim->iArrowType1 > 0)
    {
        cPrim.cPt1.x = pDim->iArrowType1;
        cPrim.cPt1.y = iPos;
        if(!GetNativeRefPoint(pDim->dRef1, &cPrim.cPt2)) return 0;
        if(!GetNativeRefDir(pDim->dRef1, &cDir)) return 0;

        if(pDim->dRef1 > pDim->dRef2) cDir *= -1.0;

        bPt1 = pDim->cArrowDim1;
        if(pDim->iArrowType1 == 3) cPrim.cPt3 = cPrim.cPt2 + bPt1;
        else
        {
            if(pDim->iArrowType1 == 5) bPt1.x *= -1.0;
            cPrim.cPt3 = cPrim.cPt2 + Rotate(bPt1, cDir, true);
            bPt1.y *= -1.0;
            if(pDim->iArrowType1 > 3) bPt1.x *= -1.0;
            cPrim.cPt4 = cPrim.cPt2 + Rotate(bPt1, cDir, true);
        }
        pPrimitive->AddPrimitive(cPrim);
    }

    if(pDim->iArrowType2 > 0)
    {
        cPrim.cPt1.x = pDim->iArrowType2;
        cPrim.cPt1.y = iPos;
        if(!GetNativeRefPoint(pDim->dRef2, &cPrim.cPt2)) return 0;
        if(!GetNativeRefDir(pDim->dRef2, &cDir)) return 0;

        if(pDim->dRef1 < pDim->dRef2) cDir *= -1.0;

        bPt1 = pDim->cArrowDim2;
        if(pDim->iArrowType2 == 3) cPrim.cPt3 = cPrim.cPt2 + bPt1;
        else
        {
            if(pDim->iArrowType2 == 5) bPt1.x *= -1.0;
            cPrim.cPt3 = cPrim.cPt2 + Rotate(bPt1, cDir, true);
            bPt1.y *= -1.0;
            if(pDim->iArrowType2 > 3) bPt1.x *= -1.0;
            cPrim.cPt4 = cPrim.cPt2 + Rotate(bPt1, cDir, true);
        }
        pPrimitive->AddPrimitive(cPrim);
    }

    cPrim.iType = 10;
    cPrim.cPt1 = pDim->cLabelPos.cPoint;
    cPrim.cPt2.x = pDim->cLabelPos.dOrientation;
    cPrim.cPt2.y = iPos;
    pPrimitive->AddPrimitive(cPrim);
    return 0;
}

int CDObject::BuildPrimitives(CDLine cTmpPt, int iMode, PDRect pRect, int iTemp, PDFileAttrs pAttrs)
{
//wchar_t buf[128];
//SendMessage(g_hStatus, SB_SETTEXT, 2, (LPARAM)L"");
    int iRes = 0;

    PDPrimObject plPrimitive = m_pPrimitive;
    if(iTemp > 1) plPrimitive = new CDPrimObject();
    plPrimitive->Clear();
    CDRect cRect = *pRect;
    cRect.cPt1.x -= m_cLineStyle.dWidth;
    cRect.cPt1.y -= m_cLineStyle.dWidth;
    cRect.cPt2.x += m_cLineStyle.dWidth;
    cRect.cPt2.y += m_cLineStyle.dWidth;

    CDPoint cBnds;
    double dExt = m_cLineStyle.dPercent*m_cLineStyle.dWidth/200.0;

    switch(m_iType)
    {
    case dtLine:
        iRes = BuildLinePrimitives(cTmpPt, iMode, &cRect, m_pInputPoints, m_pCachePoints,
            plPrimitive, m_cBounds, dExt, &m_dMovedDist, &cBnds);
        break;
    case dtCircle:
        iRes = BuildCircPrimitives(cTmpPt, iMode, &cRect, m_pInputPoints, m_pCachePoints,
            plPrimitive, m_cLines, m_cBounds, dExt, &m_dMovedDist, &cBnds);
        break;
    case dtEllipse:
        iRes = BuildEllipsePrimitives(cTmpPt, iMode, &cRect, m_pInputPoints, m_pCachePoints,
            plPrimitive, m_cLines, m_cBounds, dExt, &m_dMovedDist, &cBnds, iTemp == 1);
        break;
    case dtArcEllipse:
        iRes = BuildArcElpsPrimitives(cTmpPt, iMode, &cRect, m_pInputPoints, m_pCachePoints,
            plPrimitive, m_cLines, m_cBounds, dExt, &m_dMovedDist, &cBnds);
        break;
    case dtHyperbola:
        iRes = BuildHyperPrimitives(cTmpPt, iMode, &cRect, m_pInputPoints, m_pCachePoints,
            plPrimitive, m_cLines, m_cBounds, dExt, &m_dMovedDist, &cBnds, iTemp == 1);
        break;
    case dtParabola:
        iRes = BuildParabPrimitives(cTmpPt, iMode, &cRect, m_pInputPoints, m_pCachePoints,
            plPrimitive, m_cLines, m_cBounds, dExt, &m_dMovedDist, &cBnds, iTemp == 1);
        break;
    case dtSpline:
        iRes = BuildSplinePrimitives(cTmpPt, iMode, &cRect, m_pInputPoints, m_pCachePoints,
            plPrimitive, m_cBounds, dExt, &m_dMovedDist, &cBnds, iTemp == 1);
        break;
    case dtEvolvent:
        iRes = BuildEvolvPrimitives(cTmpPt, iMode, &cRect, m_pInputPoints, m_pCachePoints,
            plPrimitive, m_cLines, m_cBounds, dExt, &m_dMovedDist, &cBnds, iTemp == 1);
        break;
    case dtGroup:
        iRes = 0;
        break;
    }

    int nCrs = m_pCrossPoints->GetCount();

    if((iTemp < 1) && (iRes > 0) && (m_cLineStyle.iSegments > 0))
    {
        plPrimitive->ClearLines();

        double dStart, dEnd;
        int iStart, iEnd;
        double d1, d2;

        int iClosed = IsClosed();

        if(m_cBounds[0].bIsSet)
        {
            if(!GetPointRefDist(m_cBounds[0].dRef, &d1)) return iRes;

            dStart = d1;
            dEnd = d1;
            iStart = 2;
            iEnd = 0;
            if(m_iType == 7) iEnd = 2;
            else if(iClosed > 0) iEnd = 1;

            int iFirst = 0;
            if(m_cBounds[1].bIsSet)
            {
                if(!GetPointRefDist(m_cBounds[1].dRef, &d2)) return iRes;
                if(d1 > d2) iFirst = m_pCrossPoints->GetIndex(m_cBounds[0].dRef);
                if(iFirst < 0) iFirst = 0;
            }

            if(nCrs > 0)
            {
                iStart = 1;
                GetPointRefDist(m_pCrossPoints->GetPoint(iFirst), &dEnd);
                if(fabs(dEnd - d1) > g_dPrec)
                {
                    AddPatSegment(d1, 2, dEnd, 1, &cBnds, pRect);
                    dStart = dEnd;
                }

                for(int i = iFirst + 1; i < nCrs; i++)
                {
                    dStart = dEnd;
                    GetPointRefDist(m_pCrossPoints->GetPoint(i), &dEnd);
                    AddPatSegment(dStart, 1, dEnd, 1, &cBnds, pRect);
                }

                if(iFirst > 0)
                {
                    for(int i = 0; i < iFirst; i++)
                    {
                        dStart = dEnd;
                        GetPointRefDist(m_pCrossPoints->GetPoint(i), &dEnd);
                        AddPatSegment(dStart, 1, dEnd, 1, &cBnds, pRect);
                    }
                }
            }

            if(m_cBounds[1].bIsSet)
            {
                if(nCrs > 0)
                {
                    if(fabs(dEnd - d2) > g_dPrec)
                        AddPatSegment(dEnd, 1, d2, 2, &cBnds, pRect);
                }
                else AddPatSegment(d1, 2, d2, 2, &cBnds, pRect);
            }
            else
            {
                if(iClosed > 0)
                {
                    dStart = dEnd;
                    dEnd = d1 + cBnds.y - cBnds.x;
                    AddPatSegment(dStart, iStart, dEnd, 2, &cBnds, pRect);
                }
                else AddPatSegment(dEnd, iStart, cBnds.y, iEnd, &cBnds, pRect);
            }
        }
        else if(m_cBounds[1].bIsSet)
        {
            if(!GetPointRefDist(m_cBounds[1].dRef, &d2)) return iRes;

            iStart = 0;
            if(m_iType > 6) iStart = 2;

            if(nCrs > 0)
            {
                GetPointRefDist(m_pCrossPoints->GetPoint(0), &dEnd);
                AddPatSegment(cBnds.x, iStart, dEnd, 1, &cBnds, pRect);

                for(int i = 1; i < nCrs; i++)
                {
                    dStart = dEnd;
                    GetPointRefDist(m_pCrossPoints->GetPoint(i), &dEnd);
                    AddPatSegment(dStart, 1, dEnd, 1, &cBnds, pRect);
                }
                iStart = 1;
                if(fabs(dEnd - d2) > g_dPrec)
                    AddPatSegment(dEnd, 1, d2, 2, &cBnds, pRect);
            }
            else
            {
                dEnd = d2;
                AddPatSegment(cBnds.x, iStart, dEnd, 2, &cBnds, pRect);
            }
        }
        else if(iClosed > 0)
        {
            if(nCrs > 0)
            {
                GetPointRefDist(m_pCrossPoints->GetPoint(0), &dStart);
                for(int i = 1; i < nCrs; i++)
                {
                    GetPointRefDist(m_pCrossPoints->GetPoint(i), &dEnd);
                    AddPatSegment(dStart, 1, dEnd, 1, &cBnds, pRect);
                    dStart = dEnd;
                }
                GetPointRefDist(m_pCrossPoints->GetPoint(0), &dEnd);
                if(nCrs < 2) dEnd += (cBnds.y - cBnds.x);
            }
            else
            {
                dStart = cBnds.x;
                dEnd = cBnds.y;
            }
            AddPatSegment(dStart, 1, dEnd, 1, &cBnds, pRect);
        }
        else
        {
            iStart = 0;
            iEnd = 0;
            if(m_iType > 6) iStart = 2;
            if(m_iType == 7) iEnd = 2;

            if(nCrs > 0)
            {
                GetPointRefDist(m_pCrossPoints->GetPoint(0), &dEnd);
            }
            else dEnd = 0.0;
            if(dEnd > cBnds.x + g_dPrec)
            {
                AddPatSegment(cBnds.x, iStart, dEnd, 1, &cBnds, pRect);
                iStart = 1;
            }
            for(int i = 1; i < nCrs; i++)
            {
                dStart = dEnd;
                GetPointRefDist(m_pCrossPoints->GetPoint(i), &dEnd);
                AddPatSegment(dStart, 1, dEnd, 1, &cBnds, pRect);
            }
            dStart = dEnd;
            if(nCrs > 0) iStart = 1;
            if(cBnds.y > dStart + g_dPrec)
                AddPatSegment(dStart, iStart, cBnds.y, iEnd, &cBnds, pRect);
        }
    }

    CDPrimitive cPrim;

    if((iTemp < 1) && (iRes > 0))
    {
        cPrim.iType = 6;
        cPrim.cPt1 = 0;
        if(m_cBounds[0].bIsSet)
        {
            if(GetNativeRefPoint(m_cBounds[0].dRef, &cPrim.cPt2)) cPrim.cPt1.x = 1.0;
        }
        if(m_cBounds[1].bIsSet)
        {
            if(GetNativeRefPoint(m_cBounds[1].dRef, &cPrim.cPt3)) cPrim.cPt1.y = 1.0;
        }
        CropPrimitive(cPrim, &cRect, plPrimitive);
    }

    if((iTemp < 1) && (iRes > 0) && (nCrs > 0))
    {
        cPrim.iType = 8;
        for(int i = 0; i < nCrs; i++)
        {
            if(GetNativeRefPoint(m_pCrossPoints->GetPoint(i), &cPrim.cPt1))
                plPrimitive->AddPrimitive(cPrim);
        }
    }

    if((iTemp < 2) && (iRes > 0))
    {
        int nDim = m_pDimens->GetCount();
        PDDimension pDim;
        for(int i = 0; i < nDim; i++)
        {
            pDim = (PDDimension)m_pDimens->GetItem(i);
            AddDimenPrimitive(i, pDim, plPrimitive, &cRect);
        }

        if((iMode == 4) && m_bFirstDimSet && pAttrs)
        {
            CDLine cPtX;
            m_cTmpDim.dRef1 = m_dFirstDimen;
            GetDistFromPt(cTmpPt.cOrigin, cTmpPt.cOrigin, true, &cPtX, NULL);

            m_cTmpDim.dRef2 = cPtX.dRef;
            m_cTmpDim.iArrowType1 = pAttrs->iArrowType;
            m_cTmpDim.iArrowType2 = pAttrs->iArrowType;
            m_cTmpDim.cArrowDim1 = pAttrs->cArrowDim;
            m_cTmpDim.cArrowDim2 = pAttrs->cArrowDim;
            m_cTmpDim.dFontSize = pAttrs->dFontSize;
            m_cTmpDim.bFontAttrs = pAttrs->bFontAttrs;
            strcpy(m_cTmpDim.psFontFace, pAttrs->sFontFace);
            m_cTmpDim.bSelected = false;

            double d1, d2, d3, dRef3;
            GetPointRefDist(m_cTmpDim.dRef1, &d1);
            GetPointRefDist(m_cTmpDim.dRef2, &d2);
            d3 = (d1 + d2)/2.0;

            GetNativeReference(d3, &dRef3);

            CDPoint cPt1, cDir, cNorm;
            GetNativeRefPoint(dRef3, &cPt1);
            GetNativeRefDir(dRef3, &cDir);
            cNorm = GetNormal(cDir);
            double dAng = atan2(cNorm.y, cNorm.x);
            if(dAng < -g_dPrec)
            {
                cNorm *= -1.0;
                dAng += M_PI;
            }
            else if(dAng < g_dPrec)
            {
                dAng = 0.0;
                cNorm.x = 1.0;
                cNorm.y = 0.0;
            }

            m_cTmpDim.cLabelPos.cPoint = cPt1 - pAttrs->dBaseLine*cNorm;
            m_cTmpDim.cLabelPos.dOrientation = dAng;

            switch(m_iType)
            {
            case dtLine:
                strcpy(m_cTmpDim.psLab, pAttrs->sLengthMask);
                break;
            case dtCircle:
                strcpy(m_cTmpDim.psLab, pAttrs->sAngleMask);
                break;
            default:
                strcpy(m_cTmpDim.psLab, "???");
                break;
            }
            AddDimenPrimitive(-1, &m_cTmpDim, plPrimitive, &cRect);
        }
    }

    if(iTemp > 1) delete plPrimitive;
    return iRes;
}

void CDObject::GetFirstPrimitive(PDPrimitive pPrim, double dScale, int iDimen)
{
    m_iPrimitiveRequested = 0;
    GetNextPrimitive(pPrim, dScale, iDimen);
    return;
}

void CDObject::GetNextPrimitive(PDPrimitive pPrim, double dScale, int iDimen)
{
    pPrim->iType = 0;
    int iDimCount = m_pPrimitive->GetCount();
    int i = m_iPrimitiveRequested;
    bool bFound = false;
    CDPrimitive cStPrim;
    int iPos;

    while(!bFound && (i < iDimCount))
    {
        cStPrim = m_pPrimitive->GetPrimitive(i++);
        if(iDimen < -1) bFound = (cStPrim.iType < 9);
        else
        {
            if(cStPrim.iType == 9)
            {
                iPos = Round(cStPrim.cPt1.y);
                bFound = (iPos == iDimen);
            }
            else if(cStPrim.iType == 10)
            {
                iPos = Round(cStPrim.cPt2.y);
                bFound = (iPos == iDimen);
            }
        }
    }

    if(bFound)
    {
        pPrim->iType = cStPrim.iType;
        if(dScale < -g_dPrec)
        {
            if(cStPrim.iType != 9) pPrim->cPt1 = -dScale*cStPrim.cPt1;
            else pPrim->cPt1 = cStPrim.cPt1;
            if(cStPrim.iType != 10) pPrim->cPt2 = -dScale*cStPrim.cPt2;
            else pPrim->cPt2 = cStPrim.cPt2;
            pPrim->cPt3 = -dScale*cStPrim.cPt3;
            pPrim->cPt4 = -dScale*cStPrim.cPt4;
        }
        else
        {
            if(cStPrim.iType != 9) pPrim->cPt1 = Round(dScale*cStPrim.cPt1);
            else pPrim->cPt1 = cStPrim.cPt1;
            if(cStPrim.iType != 10) pPrim->cPt2 = Round(dScale*cStPrim.cPt2);
            else pPrim->cPt2 = cStPrim.cPt2;
            pPrim->cPt3 = Round(dScale*cStPrim.cPt3);
            pPrim->cPt4 = Round(dScale*cStPrim.cPt4);
        }
    }
    m_iPrimitiveRequested = i;
    return;
}

int CDObject::GetBSplineParts()
{
    int iRes = 0;
    int iCount = m_pPrimitive->GetCount();
    if(iCount < 1) return iRes;

    CDPrimitive cPrim1, cPrim2 = m_pPrimitive->GetPrimitive(0);
    bool b2set = (cPrim2.iType == 4);
    if(b2set) iRes++;

    double dDist;

    for(int i = 1; i < iCount; i++)
    {
        cPrim1 = m_pPrimitive->GetPrimitive(i);
        if(cPrim1.iType != 4)
        {
            b2set = false;
        }
        else if(b2set)
        {
            dDist = GetDist(cPrim2.cPt3, cPrim1.cPt1);
            if(dDist > g_dPrec) iRes++;
            cPrim2 = cPrim1;
        }
        else
        {
            iRes++;
            b2set = true;
            cPrim2 = cPrim1;
        }
    }

    return iRes;
}

bool CDObject::GetBSplines(int iParts, double dScale, int *piCtrls, double **ppdKnots, PDPoint *ppPoints)
{
    int iCount = m_pPrimitive->GetCount();
    int *piPairs = (int*)malloc(2*iParts*sizeof(int));

    int iCurPair = -1;
    CDPrimitive cPrim1, cPrim2 = m_pPrimitive->GetPrimitive(0);
    bool b2set = (cPrim2.iType == 4);
    if(b2set)
    {
        iCurPair = 0;
        piPairs[2*iCurPair] = 0;
        piPairs[2*iCurPair + 1] = -1;
    }

    double dDist;

    for(int i = 1; i < iCount; i++)
    {
        cPrim1 = m_pPrimitive->GetPrimitive(i);
        if(cPrim1.iType != 4)
        {
            if(b2set) piPairs[2*iCurPair + 1] = i;
            b2set = false;
        }
        else if(b2set)
        {
            dDist = GetDist(cPrim2.cPt3, cPrim1.cPt1);
            if(dDist > g_dPrec)
            {
                piPairs[2*iCurPair + 1] = i;
                iCurPair++;
                piPairs[2*iCurPair] = i;
                piPairs[2*iCurPair + 1] = -1;
            }
            cPrim2 = cPrim1;
        }
        else
        {
            iCurPair++;
            piPairs[2*iCurPair] = i;
            piPairs[2*iCurPair + 1] = -1;
            b2set = true;
            cPrim2 = cPrim1;
        }
    }

    CDPoint cPt1, cPt2;
    double dDom, du;
    double *pdt;

    if((IsClosed() > 0) && (iParts == 1))
    {
        if(piPairs[1] < 0) piPairs[1] = iCount;
        piCtrls[0] = piPairs[1] - piPairs[0];
        ppPoints[0] = (PDPoint)malloc(piCtrls[0]*sizeof(CDPoint));
        ppdKnots[0] = (double*)malloc((piCtrls[0] + 4)*sizeof(double));

        pdt = (double*)malloc((piCtrls[0])*sizeof(double));

        cPrim1 = m_pPrimitive->GetPrimitive(piPairs[0]);
        cPt2 = cPrim1.cPt2;

        ppdKnots[0][0] = 0.0;
        ppdKnots[0][1] = 0.0;
        ppdKnots[0][2] = 0.0;

        cPrim1 = m_pPrimitive->GetPrimitive(piPairs[1] - 1);
        cPt2 = cPrim1.cPt2;

        for(int i = piPairs[0]; i < piPairs[1]; i++)
        {
            cPt1 = cPt2;
            cPrim1 = m_pPrimitive->GetPrimitive(i);
            cPt2 = cPrim1.cPt2;
            ppPoints[0][i - piPairs[0]] = -dScale*cPt2;

            dDom = GetDist(cPt1, cPt2);
            du = GetDist(cPt1, cPrim1.cPt1);
            pdt[i - piPairs[0]] = du/dDom;
        }
        if(piCtrls[0] > 4)
        {
            for(int j = 1; j < piCtrls[0] - 3; j++)
            {
                du = pdt[j]/(1.0 - pdt[j - 1]*(1 - pdt[j]));
                pdt[j] = du;
            }
        }
        ppdKnots[0][piCtrls[0] - 1] = pdt[piCtrls[0] - 4];
        if(piCtrls[0] > 4)
        {
            for(int j = piCtrls[0] - 5; j >= 0; j--)
            {
                ppdKnots[0][j + 3] = ppdKnots[0][j + 4]*pdt[j];
            }
        }
        ppdKnots[0][piCtrls[0]] = 1.0;
        ppdKnots[0][piCtrls[0] + 1] = 1.0 + ppdKnots[0][3];
        ppdKnots[0][piCtrls[0] + 2] = 1.0 + ppdKnots[0][4];

        ppdKnots[0][0] = ppdKnots[0][piCtrls[0] - 2] - 1.0;
        ppdKnots[0][1] = ppdKnots[0][piCtrls[0] - 1] - 1.0;
//for(int j = 0; j < piCtrls[0] + 3; j++)
//printf("%f\n", ppdKnots[0][j]);

        free(pdt);
        free(piPairs);
        return true;
    }

    if(piPairs[2*iCurPair + 1] < 0) piPairs[2*iCurPair + 1] = iCount;

    for(int i = 0; i < iParts; i++)
    {
        piCtrls[i] = piPairs[2*i + 1] - piPairs[2*i] + 2;
        ppPoints[i] = (PDPoint)malloc(piCtrls[i]*sizeof(CDPoint));
        cPrim1 = m_pPrimitive->GetPrimitive(piPairs[2*i]);
        ppPoints[i][0] = -dScale*cPrim1.cPt1;
        cPrim1 = m_pPrimitive->GetPrimitive(piPairs[2*i + 1] - 1);
        ppPoints[i][piCtrls[i] - 1] = -dScale*cPrim1.cPt3;

        pdt = (double*)malloc((piCtrls[i] - 3)*sizeof(double));

        ppdKnots[i] = (double*)malloc((piCtrls[i] + 3)*sizeof(double));
        ppdKnots[i][0] = 0.0;
        ppdKnots[i][1] = 0.0;
        ppdKnots[i][2] = 0.0;

        cPrim1 = m_pPrimitive->GetPrimitive(piPairs[2*i]);
        cPt2 = cPrim1.cPt2;

        for(int j = piPairs[2*i]; j < piPairs[2*i + 1]; j++)
        {
            cPt1 = cPt2;
            cPrim1 = m_pPrimitive->GetPrimitive(j);
            cPt2 = cPrim1.cPt2;
            ppPoints[i][j - piPairs[2*i] + 1] = -dScale*cPt2;

            if(j > piPairs[2*i])
            {
                dDom = GetDist(cPt1, cPt2);
                du = GetDist(cPt1, cPrim1.cPt1);
                pdt[j - piPairs[2*i] - 1] = du/dDom;
            }
        }
        if(piCtrls[i] > 4)
        {
            for(int j = 1; j < piCtrls[i] - 3; j++)
            {
                du = pdt[j]/(1.0 - pdt[j - 1]*(1 - pdt[j]));
                pdt[j] = du;
            }
        }
        ppdKnots[i][piCtrls[i] - 1] = pdt[piCtrls[i] - 4];
        if(piCtrls[i] > 4)
        {
            for(int j = piCtrls[i] - 5; j >= 0; j--)
            {
                ppdKnots[i][j + 3] = ppdKnots[i][j + 4]*pdt[j];
            }
        }
        ppdKnots[i][piCtrls[i]] = 1.0;
        ppdKnots[i][piCtrls[i] + 1] = ppdKnots[i][piCtrls[i]];
        ppdKnots[i][piCtrls[i] + 2] = ppdKnots[i][piCtrls[i]];
        free(pdt);
    }

    free(piPairs);
    return false;
}

bool CDObject::IsNearPoint(CDPoint cPt, double dTolerance, int *piDimen)
{
    *piDimen = -2;
    double dDist = dTolerance + 1.0;
    CDLine cPtX;
    cPtX.bIsSet = false;
    switch(m_iType)
    {
    case dtLine:
        dDist = GetLineDistFromPt(cPt, m_pCachePoints, &cPtX);
        break;
    case dtCircle:
        dDist = GetCircDistFromPt(cPt, cPt, true, m_pCachePoints, &cPtX);
        break;
    case dtEllipse:
        dDist = GetElpsDistFromPt(cPt, cPt, 1, m_pCachePoints, &cPtX, m_cBounds);
        break;
    case dtArcEllipse:
        dDist = GetArcElpsDistFromPt(cPt, cPt, 1, m_pCachePoints, &cPtX, m_cBounds);
        break;
    case dtHyperbola:
        dDist = GetHyperDistFromPt(cPt, cPt, 1, m_pCachePoints, &cPtX, m_cBounds);
        break;
    case dtParabola:
        dDist = GetParabDistFromPt(cPt, cPt, 1, m_pCachePoints, &cPtX, m_cBounds);
        break;
    case dtSpline:
        dDist = GetSplineDistFromPt(cPt, cPt, m_pCachePoints, &cPtX);
        break;
    case dtEvolvent:
        dDist = GetEvolvDistFromPt(cPt, cPt, m_pCachePoints, &cPtX);
        break;
    case dtGroup:
        break;
    }

    PDDimension pDim;
    double dDist1;
    CDPoint cPt1, cPt2;
    for(int i = 0; i < m_pDimens->GetCount(); i++)
    {
        pDim = (PDDimension)m_pDimens->GetItem(i);
        GetNativeRefPoint(pDim->dRef1, &cPt1);
        dDist1 = GetDist(cPt, cPt1);
        if(dDist1 < fabs(dDist))
        {
            dDist = dDist1;
            *piDimen = i;
            cPtX.bIsSet = true;
            cPtX.cOrigin = cPt1;
            cPtX.dRef = pDim->dRef1;
        }
        GetNativeRefPoint(pDim->dRef2, &cPt1);
        dDist1 = GetDist(cPt, cPt1);
        if(dDist1 < fabs(dDist))
        {
            dDist = dDist1;
            *piDimen = i;
            cPtX.bIsSet = true;
            cPtX.cOrigin = cPt1;
            cPtX.dRef = pDim->dRef2;
        }
        cPt1.x = -sin(pDim->cLabelPos.dOrientation);
        cPt1.y = cos(pDim->cLabelPos.dOrientation);
        cPt2 = Rotate(cPt - pDim->cLabelPos.cPoint, cPt1, false);
        if(DPtInDRect(cPt2, &pDim->cExt))
        {
            dDist = 0.0;
            *piDimen = i;
            cPtX.bIsSet = true;
            cPtX.cOrigin = pDim->cLabelPos.cPoint;
            cPtX.dRef = (pDim->dRef1 + pDim->dRef2)/2.0;
        }
    }

    if(!cPtX.bIsSet) return false;

    BoundPoint(cPt, &cPtX, &dDist);

    return fabs(dDist) < dTolerance;
}

bool CDObject::GetNativeReference(double dDist, double *pdRef)
{
    switch(m_iType)
    {
    case 1:
        return GetLineReference(dDist, m_pCachePoints, pdRef);
    case 2:
        return GetCircReference(dDist, m_pCachePoints, pdRef);
    case 3:
        return GetElpsReference(dDist, m_pCachePoints, pdRef);
    case 4:
        return GetArcElpsReference(dDist, m_pCachePoints, pdRef);
    case 5:
        return GetHyperReference(dDist, m_pCachePoints, pdRef);
    case 6:
        return GetParabReference(dDist, m_pCachePoints, pdRef);
    case 7:
        return GetSplineReference(dDist, m_pCachePoints, pdRef);
    case 8:
        return GetEvolvReference(dDist, m_pCachePoints, pdRef);
    default:
        return false;
    }
}

bool CDObject::GetNativeRefPoint(double dRef, PDPoint pPt)
{
    switch(m_iType)
    {
    case dtLine:
        return GetLineRefPoint(dRef, m_pCachePoints, pPt);
    case dtCircle:
        return GetCircRefPoint(dRef, m_pCachePoints, pPt);
    case dtEllipse:
        return GetElpsRefPoint(dRef, m_pCachePoints, pPt);
    case dtArcEllipse:
        return GetArcElpsRefPoint(dRef, m_pCachePoints, pPt);
    case dtHyperbola:
        return GetHyperRefPoint(dRef, m_pCachePoints, pPt);
    case dtParabola:
        return GetParabRefPoint(dRef, m_pCachePoints, pPt);
    case dtSpline:
        return GetSplineRefPoint(dRef, m_pCachePoints, pPt);
    case dtEvolvent:
        return GetEvolvRefPoint(dRef, m_pCachePoints, pPt);
    case dtGroup:
        return false;
    default:
        return false;
    }
}

bool CDObject::GetNativeRefDir(double dRef, PDPoint pPt)
{
    switch(m_iType)
    {
    case dtLine:
        return GetLineRefDir(dRef, m_pCachePoints, pPt);
    case dtCircle:
        return GetCircRefDir(dRef, m_pCachePoints, pPt);
    case dtEllipse:
        return GetElpsRefDir(dRef, m_pCachePoints, pPt);
    case dtArcEllipse:
        return GetArcElpsRefDir(dRef, m_pCachePoints, pPt);
    case dtHyperbola:
        return GetHyperRefDir(dRef, m_pCachePoints, pPt);
    case dtParabola:
        return GetParabRefDir(dRef, m_pCachePoints, pPt);
    case dtSpline:
        return GetSplineRefDir(dRef, m_pCachePoints, pPt);
    case dtEvolvent:
        return GetEvolvRefDir(dRef, m_pCachePoints, pPt);
    case dtGroup:
        return false;
    default:
        return false;
    }
}

bool CDObject::IsValidRef(double dRef)
{
    if(IsClosedShape())
    {
        if(!m_cBounds[1].bIsSet) return true;
        if(!m_cBounds[0].bIsSet) return true;

        if(m_cBounds[0].dRef > m_cBounds[1].dRef)
        {
            if((m_cBounds[1].dRef < dRef) && (dRef < m_cBounds[0].dRef))
                return false;
        }
    }

    if(m_cBounds[0].bIsSet)
    {
        if(dRef < m_cBounds[0].dRef) return false;
    }

    if(m_cBounds[1].bIsSet)
    {
        if(dRef > m_cBounds[1].dRef) return false;
    }

    return true;
}

bool CDObject::GetSelected()
{
    return m_bSelected;
}

void CDObject::SetSelected(bool bSelect, bool bInvert, int iDimen, PDPtrList pRegions)
{
    PDDimension pDim;
    bool bChanged = false;
    if(bInvert)
    {
        if(iDimen > -1)
        {
            pDim = (PDDimension)m_pDimens->GetItem(iDimen);
            if(pDim->bSelected == bSelect) pDim->bSelected = !bSelect;
            else pDim->bSelected = bSelect;
        }
        else if(m_bSelected == bSelect) m_bSelected = !bSelect;
        else m_bSelected = bSelect;
        bChanged = true;
    }
    else
    {
        if(iDimen > -1)
        {
            pDim = (PDDimension)m_pDimens->GetItem(iDimen);
            bChanged = (pDim->bSelected != bSelect);
            pDim->bSelected = bSelect;
        }
        else
        {
            bChanged = (m_bSelected != bSelect);
            m_bSelected = bSelect;
            for(int i = 0; i < m_pDimens->GetCount(); i++)
            {
                pDim = (PDDimension)m_pDimens->GetItem(i);
                bChanged |= pDim->bSelected;
                pDim->bSelected = false;
            }
        }
    }
    if(bChanged) AddRegions(pRegions, -1);
}

int CDObject::GetType()
{
    return m_iType;
}

CDLine CDObject::GetLine()
{
    CDLine cRes = {false, {0, 0}, {0, 0}};

    if(m_iType != 1) return cRes;

    int iCnt = m_pCachePoints->GetCount(0);
    if(iCnt < 2) return cRes;

    CDInputPoint cInPt1 = m_pCachePoints->GetPoint(0, 0);
    CDInputPoint cInPt2 = m_pCachePoints->GetPoint(1, 0);

    cRes.bIsSet = true;
    cRes.cOrigin = cInPt1.cPoint;
    cRes.cDirection = cInPt2.cPoint;
    return cRes;
}

CDLine CDObject::GetCircle()
{
    CDLine cRes = {false, {0, 0}, {0, 0}};

    if(m_iType != 2) return cRes;

    int iCnt = m_pCachePoints->GetCount(0);
    if(iCnt < 2) return cRes;

    CDInputPoint cInPt1 = m_pCachePoints->GetPoint(0, 0);
    CDInputPoint cInPt2 = m_pCachePoints->GetPoint(1, 0);

    cRes.bIsSet = true;
    cRes.cOrigin = cInPt1.cPoint;
    cRes.cDirection = cInPt2.cPoint;
    return cRes;
}

void CDObject::SetInputLine(int iIndex, CDLine cLine)
{
    m_cLines[iIndex] = cLine;
}

bool CDObject::HasEnoughPoints()
{
    int iInputLines = 0;
    if(m_cLines[0].bIsSet) iInputLines++;
    if(m_cLines[1].bIsSet) iInputLines++;

    switch(m_iType)
    {
    case dtLine:
        return(HasLineEnoughPoints(m_pInputPoints));
    case dtCircle:
        return(HasCircEnoughPoints(m_pInputPoints, iInputLines));
    case dtEllipse:
        return(HasElpsEnoughPoints(m_pInputPoints, iInputLines));
    case dtArcEllipse:
        return(HasArcElpsEnoughPoints(m_pInputPoints, iInputLines));
    case dtHyperbola:
        return(HasHyperEnoughPoints(m_pInputPoints, iInputLines));
    case dtParabola:
        return(HasParabEnoughPoints(m_pInputPoints, iInputLines));
    case dtSpline:
        return(HasSplineEnoughPoints(m_pInputPoints));
    case dtEvolvent:
        return(HasEvolvEnoughPoints(m_pInputPoints, iInputLines));
    case dtGroup:
        return false;
    default:
        return false;
    }
}

bool CDObject::GetPoint(int iIndex, char iCtrl, PDInputPoint pPoint)
{
    int iCnt = m_pInputPoints->GetCount(iCtrl);
    if(iIndex >= iCnt) return false;
    *pPoint = m_pInputPoints->GetPoint(iIndex, iCtrl);
    return true;
}

void CDObject::SetPoint(int iIndex, char iCtrl, CDInputPoint cPoint)
{
    m_pInputPoints->SetPoint(iIndex, iCtrl, cPoint.cPoint.x, cPoint.cPoint.y, cPoint.iCtrl);
}

bool CDObject::BoundPoint(CDPoint cRefPt, PDLine pPtX, double *pdDist)
{
    if(IsClosedShape())
    {
        bool bRes = false;
        if(m_cBounds[0].bIsSet && m_cBounds[1].bIsSet)
        {
            if(m_cBounds[0].dRef > m_cBounds[1].dRef)
            {
                bRes = ((pPtX->dRef > m_cBounds[1].dRef) && (m_cBounds[0].dRef > pPtX->dRef));
            }
            else bRes = ((pPtX->dRef < m_cBounds[0].dRef) || (m_cBounds[1].dRef < pPtX->dRef));
        }
        if(bRes)
        {
            pPtX->cDirection = 0;
            pPtX->dRef = m_cBounds[0].dRef;
            GetNativeRefPoint(m_cBounds[0].dRef, &pPtX->cOrigin);
            *pdDist = GetDist(cRefPt, pPtX->cOrigin);

            CDPoint cPt2;
            GetNativeRefPoint(m_cBounds[1].dRef, &cPt2);
            double d1 = GetDist(cRefPt, cPt2);
            if(d1 < *pdDist)
            {
                pPtX->dRef = m_cBounds[1].dRef;
                pPtX->cOrigin = cPt2;
                *pdDist = d1;
            }
            return true;
        }
        return false;
    }

    if(m_cBounds[0].bIsSet)
    {
        if(pPtX->dRef < m_cBounds[0].dRef)
        {
            pPtX->cDirection = 0;
            pPtX->dRef = m_cBounds[0].dRef;
            GetNativeRefPoint(m_cBounds[0].dRef, &pPtX->cOrigin);
            *pdDist = GetDist(cRefPt, pPtX->cOrigin);
            return true;
        }
    }
    if(m_cBounds[1].bIsSet)
    {
        if(pPtX->dRef > m_cBounds[1].dRef)
        {
            pPtX->cDirection = 0;
            pPtX->dRef = m_cBounds[1].dRef;
            GetNativeRefPoint(m_cBounds[1].dRef, &pPtX->cOrigin);
            *pdDist = GetDist(cRefPt, pPtX->cOrigin);
            return true;
        }
    }
    return false;
}

double CDObject::GetDistFromPt(CDPoint cPt, CDPoint cRefPt, bool bSnapCenters, PDLine pPtX, int *piDimen)
{
    CDLine cPtX;
    double dRes = -1.0;
    int iMask = 0;
    if(bSnapCenters) iMask = 1;

    switch(m_iType)
    {
    case dtLine:
        dRes = GetLineDistFromPt(cPt, m_pCachePoints, &cPtX);
        break;
    case dtCircle:
        dRes = GetCircDistFromPt(cPt, cRefPt, bSnapCenters, m_pCachePoints, &cPtX);
        break;
    case dtEllipse:
        dRes = GetElpsDistFromPt(cPt, cRefPt, iMask, m_pCachePoints, &cPtX, m_cBounds);
        break;
    case dtArcEllipse:
        dRes = GetArcElpsDistFromPt(cPt, cRefPt, iMask, m_pCachePoints, &cPtX, m_cBounds);
        break;
    case dtHyperbola:
        dRes = GetHyperDistFromPt(cPt, cRefPt, iMask, m_pCachePoints, &cPtX, m_cBounds);
        break;
    case dtParabola:
        dRes = GetParabDistFromPt(cPt, cRefPt, iMask, m_pCachePoints, &cPtX, m_cBounds);
        break;
    case dtSpline:
        dRes = GetSplineDistFromPt(cPt, cRefPt, m_pCachePoints, &cPtX);
        break;
    case dtEvolvent:
        dRes = GetEvolvDistFromPt(cPt, cRefPt, m_pCachePoints, &cPtX);
        break;
    case dtGroup:
        break;
    }

    if(piDimen)
    {
        *piDimen = -2;
        PDDimension pDim;
        double dDist1;
        CDPoint cPt1, cPt2;
        for(int i = 0; i < m_pDimens->GetCount(); i++)
        {
            pDim = (PDDimension)m_pDimens->GetItem(i);
            GetNativeRefPoint(pDim->dRef1, &cPt1);
            dDist1 = GetDist(cPt, cPt1);
            if(dDist1 < fabs(dRes))
            {
                dRes = dDist1;
                *piDimen = i;
                cPtX.bIsSet = true;
                cPtX.cOrigin = cPt1;
                cPtX.dRef = pDim->dRef1;
                cPtX.cDirection = 0;
            }
            GetNativeRefPoint(pDim->dRef2, &cPt1);
            dDist1 = GetDist(cPt, cPt1);
            if(dDist1 < fabs(dRes))
            {
                dRes = dDist1;
                *piDimen = i;
                cPtX.bIsSet = true;
                cPtX.cOrigin = cPt1;
                cPtX.dRef = pDim->dRef2;
                cPtX.cDirection = 0;
            }
            cPt1.x = -sin(pDim->cLabelPos.dOrientation);
            cPt1.y = cos(pDim->cLabelPos.dOrientation);
            cPt2 = Rotate(cPt - pDim->cLabelPos.cPoint, cPt1, false);
            if(DPtInDRect(cPt2, &pDim->cExt))
            {
                dRes = 0.0;
                *piDimen = i;
                cPtX.bIsSet = true;
                cPtX.cOrigin = pDim->cLabelPos.cPoint;
                cPtX.dRef = (pDim->dRef1 + pDim->dRef2)/2.0;
                cPtX.cDirection = 0;
            }
        }
    }

    double dNorm = GetNorm(cPtX.cDirection);
    if(dNorm < g_dPrec)
    {
        if(pPtX) *pPtX = cPtX;
        return dRes;
    }

    if(BoundPoint(cRefPt, &cPtX, &dRes))
    {
        if(pPtX) *pPtX = cPtX;
        return dRes;
    }

    dNorm = GetNorm(cPtX.cDirection);
    if(dNorm < g_dPrec)
    {
        if(pPtX) *pPtX = cPtX;
        return dRes;
    }

    CDPoint cPtTan = GetNormal(cPtX.cDirection);

    CDPoint cPtX2;
    double dRes2 = GetLineProj(cPt, cPtX.cOrigin, cPtTan, &cPtX2);
    if(dRes2 < dRes)
    {
        if(pPtX)
        {
            pPtX->bIsSet = true;
            pPtX->cOrigin = cPtX2;
            pPtX->cDirection = cPtX.cDirection;
            pPtX->dRef = cPtX.dRef;
        }
        return dRes2;
    }

    if(pPtX) *pPtX = cPtX;
    return dRes;
}

CDLineStyle CDObject::GetLineStyle()
{
    return m_cLineStyle;
}

void CDObject::SetLineStyle(int iMask, CDLineStyle cStyle)
{
    //m_cLineStyle = cStyle;
    if(iMask & 1) m_cLineStyle.dWidth = cStyle.dWidth;
    if(iMask & 2) m_cLineStyle.dPercent = cStyle.dPercent;
    if(iMask & 4)
    {
        int n = cStyle.iSegments;
        if(n > 6) n = 6;
        m_cLineStyle.iSegments = n;
        for(int i = 0; i < n; i++) m_cLineStyle.dPattern[i] = cStyle.dPattern[i];
    }
}

bool CDObject::GetRestrictPoint(CDPoint cPt, int iMode, bool bRestrictSet, double dRestrictValue,
    PDPoint pSnapPt)
{
    *pSnapPt = cPt;

    if(!bRestrictSet) return false;

    switch(m_iType)
    {
    case dtLine:
        return GetLineRestrictPoint(cPt, iMode, dRestrictValue, pSnapPt, m_pCachePoints);
    case dtCircle:
        return GetCircleRestrictPoint(cPt, iMode, dRestrictValue, pSnapPt, m_pCachePoints,
            m_pInputPoints, m_cLines);
    case dtEllipse:
        return GetElpsRestrictPoint(cPt, iMode, dRestrictValue, pSnapPt, m_pCachePoints);
    case dtArcEllipse:
        return GetArcElpsRestrictPoint(cPt, iMode, dRestrictValue, pSnapPt, m_pCachePoints);
    case dtHyperbola:
        return GetHyperRestrictPoint(cPt, iMode, dRestrictValue, pSnapPt, m_pCachePoints);
    case dtParabola:
        return GetParabRestrictPoint(cPt, iMode, dRestrictValue, pSnapPt, m_pCachePoints);
    case dtSpline:
        return GetSplineRestrictPoint(cPt, iMode, dRestrictValue, pSnapPt, m_pCachePoints);
    case dtEvolvent:
        return GetEvolvRestrictPoint(cPt, iMode, dRestrictValue, pSnapPt, m_pCachePoints);
    case dtGroup:
        return false;
    default:
        return false;
    }
}

CDObject* CDObject::Copy()
{
    PDObject pRes = new CDObject(m_iType, m_cLineStyle.dWidth);
    pRes->SetInputLine(0, m_cLines[0]);
    pRes->SetInputLine(1, m_cLines[1]);
    CDInputPoint cInPt;

    for(int i = 0; i < m_pInputPoints->GetCount(-1); i++)
    {
        cInPt = m_pInputPoints->GetPoint(i, -1);
        pRes->AddPoint(cInPt.cPoint.x, cInPt.cPoint.y, cInPt.iCtrl, false);
    }
    pRes->SetBound(0, m_cBounds[0]);
    pRes->SetBound(1, m_cBounds[1]);
    pRes->SetLineStyle(7, m_cLineStyle);

    double dRef;
    for(int i = 0; i < m_pCrossPoints->GetCount(); i++)
    {
        dRef = m_pCrossPoints->GetPoint(i);
        pRes->AddCrossPoint(dRef);
    }

    return pRes;
}

bool CDObject::IsClosedShape()
{
    switch(m_iType)
    {
    case dtCircle:
    case dtEllipse:
    case dtArcEllipse:
        return true;
    case dtSpline:
        if(m_pInputPoints->GetCount(1) < 1) return false;
        return true;
    default:
        return false;
    }
}

int CDObject::IsClosed()
{
    switch(m_iType)
    {
    case dtCircle:
    case dtEllipse:
    case dtArcEllipse:
        if(m_cBounds[0].bIsSet)
        {
            if(m_cBounds[1].bIsSet) return 0;
            return 1;
        }
        return 2;
    case dtSpline:
        if(m_pInputPoints->GetCount(1) < 1) return 0;
        if(m_cBounds[0].bIsSet)
        {
            if(m_cBounds[1].bIsSet) return 0;
            return 1;
        }
        return 2;
    default:
        return 0;
    }
}

void CDObject::SetBound(int iIndex, CDLine cBound)
{
    m_cBounds[iIndex].bIsSet = cBound.bIsSet;
    m_cBounds[iIndex].dRef = cBound.dRef;
    if(IsClosedShape() && !m_cBounds[0].bIsSet && m_cBounds[1].bIsSet)
    {
        m_cBounds[0] = m_cBounds[1];
        m_cBounds[1].bIsSet = false;
    }

    int n = m_pCrossPoints->GetCount();
    double dRef;
    while(n > 0)
    {
        dRef = m_pCrossPoints->GetPoint(--n);
        if(!IsValidRef(dRef)) m_pCrossPoints->Remove(n);
    }
}

void CDObject::SetBound(int iIndex, CDRefPoint cBound)
{
    m_cBounds[iIndex] = cBound;
    if(IsClosedShape() && !m_cBounds[0].bIsSet && m_cBounds[1].bIsSet)
    {
        m_cBounds[0] = m_cBounds[1];
        m_cBounds[1].bIsSet = false;
    }

    int n = m_pCrossPoints->GetCount();
    double dRef;
    while(n > 0)
    {
        dRef = m_pCrossPoints->GetPoint(--n);
        if(!IsValidRef(dRef)) m_pCrossPoints->Remove(n);
    }
}

bool CDObject::Split(CDPoint cPt, double dDist, PDRect pRect, CDObject** ppNewObj, PDPtrList pRegions)
{
    *ppNewObj = NULL;

    CDLine cPtX, cLn;
    cLn.bIsSet = false;
    double d1 = fabs(GetDistFromPt(cPt, cPt, false, &cPtX, NULL));
    if(d1 > dDist) return false;
    if(!cPtX.bIsSet) return false;

    int iClosed = IsClosed();
    if(iClosed == 2)
    {
        SetBound(0, cPtX);
        BuildPrimitives(cLn, 0, pRect, 0, NULL);
        AddRegions(pRegions, 6);
        return true;
    }

    PDObject pNewObj = NULL;

    if(m_cBounds[0].bIsSet)
    {
        if(m_cBounds[1].bIsSet)
        {
            pNewObj = Copy();
            SetBound(1, cPtX);
            pNewObj->SetBound(0, cPtX);
        }
        else
        {
            pNewObj = Copy();
            SetBound(1, cPtX);
            pNewObj->SetBound(0, cPtX);
            if(iClosed > 0) pNewObj->SetBound(1, m_cBounds[0]);
        }
    }
    else
    {
        if(m_cBounds[1].bIsSet)
        {
            pNewObj = Copy();
            SetBound(0, cPtX);
            pNewObj->SetBound(1, cPtX);
            //cPtX.bIsSet = false;
            //pNewObj->SetBound(0, cPtX);
        }
        else if(iClosed < 1)
        {
            pNewObj = Copy();
            SetBound(0, cPtX);
            pNewObj->SetBound(1, cPtX);
        }
        else SetBound(0, cPtX);
    }

    PDDimension pDim;
    int n = m_pDimens->GetCount();
    for(int i = n - 1; i >= 0; i--)
    {
        pDim = (PDDimension)m_pDimens->GetItem(i);
        if(m_cBounds[0].bIsSet && m_cBounds[1].bIsSet)
        {
            if(m_cBounds[0].dRef < m_cBounds[1].dRef)
            {
                if(pDim->dRef1 < m_cBounds[0].dRef - g_dPrec)
                {
                    m_pDimens->Remove(i);
                    if(pDim->dRef2 < m_cBounds[0].dRef + g_dPrec) pNewObj->AddDimenPtr(pDim);
                    else free(pDim);
                }
                else if(pDim->dRef2 > m_cBounds[1].dRef + g_dPrec)
                {
                    m_pDimens->Remove(i);
                    if(pDim->dRef1 > m_cBounds[1].dRef - g_dPrec) pNewObj->AddDimenPtr(pDim);
                    else free(pDim);
                }
            }
            /*else
            {
                if(pDim->dRef1 < m_cBounds[1].dRef - g_dPrec)
                {
                    m_pDimens->Remove(i);
                    if(pDim->dRef2 < m_cBounds[1].dRef + g_dPrec) pNewObj->AddDimenPtr(pDim);
                    else free(pDim);
                }
                if(pDim->dRef2 > m_cBounds[0].dRef + g_dPrec)
                {
                    m_pDimens->Remove(i);
                    if(pDim->dRef1 > m_cBounds[0].dRef - g_dPrec) pNewObj->AddDimenPtr(pDim);
                    else free(pDim);
                }
            }*/
        }
        else if(m_cBounds[0].bIsSet)
        {
            if(pDim->dRef1 < m_cBounds[0].dRef - g_dPrec)
            {
                m_pDimens->Remove(i);
                if(pDim->dRef2 < m_cBounds[0].dRef + g_dPrec) pNewObj->AddDimenPtr(pDim);
                else free(pDim);
            }
        }
        else if(m_cBounds[1].bIsSet)
        {
            if(pDim->dRef2 > m_cBounds[1].dRef + g_dPrec)
            {
                m_pDimens->Remove(i);
                if(pDim->dRef1 > m_cBounds[1].dRef - g_dPrec) pNewObj->AddDimenPtr(pDim);
                else free(pDim);
            }
        }
    }

    cPtX.bIsSet = false;
    BuildPrimitives(cLn, 0, pRect, 0, NULL);
    AddRegions(pRegions, -1);
    pNewObj->BuildCache(cPtX, 0);
    pNewObj->BuildPrimitives(cLn, 0, pRect, 0, NULL);
    pNewObj->SetSelected(true, false, -1, pRegions);

    *ppNewObj = pNewObj;
    return true;
}

bool CDObject::Extend(CDPoint cPt, double dDist, PDRect pRect, PDPtrList pRegions)
{
    double d1;
    bool bRes = false;
    CDPoint cPt1;
    if(m_cBounds[0].bIsSet)
    {
        GetNativeRefPoint(m_cBounds[0].dRef, &cPt1);
        d1 = GetDist(cPt1, cPt);
        if(d1 < dDist)
        {
            m_cBounds[0].bIsSet = false;
            bRes = true;
        }
    }
    if(m_cBounds[1].bIsSet)
    {
        GetNativeRefPoint(m_cBounds[1].dRef, &cPt1);
        d1 = GetDist(cPt1, cPt);
        if(d1 < dDist)
        {
            m_cBounds[1].bIsSet = false;
            bRes = true;
        }
    }
    if(bRes)
    {
        if(IsClosedShape() && !m_cBounds[0].bIsSet && m_cBounds[1].bIsSet)
        {
            m_cBounds[0] = m_cBounds[1];
            m_cBounds[1].bIsSet = false;
        }
        CDLine cLn;
        cLn.bIsSet = false;
        BuildPrimitives(cLn, 0, pRect, 0, NULL);
        AddRegions(pRegions, -1);
    }
    return bRes;
}

void CDObject::SavePoint(FILE *pf, bool bSwapBytes, CDPoint cPoint)
{
    unsigned char buf[16], *pbuf;

    pbuf = (unsigned char*)&cPoint.x;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    pbuf = (unsigned char*)&cPoint.y;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);
}

void CDObject::SaveInputPoint(FILE *pf, bool bSwapBytes, CDInputPoint cInPoint)
{
    unsigned char buf = cInPoint.iCtrl;
    fwrite(&buf, 1, 1, pf);
    SavePoint(pf, bSwapBytes, cInPoint.cPoint);
}

void CDObject::SaveReference(FILE *pf, bool bSwapBytes, double dRef)
{
    unsigned char buf[16], *pbuf;
    pbuf = (unsigned char*)&dRef;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);
}

void CDObject::SaveRefPoint(FILE *pf, bool bSwapBytes, CDRefPoint cRefPoint)
{
    unsigned char buf = cRefPoint.bIsSet;
    fwrite(&buf, 1, 1, pf);
    SaveReference(pf, bSwapBytes, cRefPoint.dRef);
}

void CDObject::SaveLine(FILE *pf, bool bSwapBytes, CDLine cLine)
{
    unsigned char buf[16], *pbuf;

    buf[0] = cLine.bIsSet;
    fwrite(buf, 1, 1, pf);
    SavePoint(pf, bSwapBytes, cLine.cOrigin);
    SavePoint(pf, bSwapBytes, cLine.cDirection);

    pbuf = (unsigned char*)&cLine.dRef;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);
}

void CDObject::SaveLineStyle(FILE *pf, bool bSwapBytes, CDLineStyle cLineStyle)
{
    unsigned char buf[16], *pbuf;

    pbuf = (unsigned char*)&cLineStyle.dWidth;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    pbuf = (unsigned char*)&cLineStyle.dPercent;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    unsigned long ulVal = cLineStyle.iSegments;
    pbuf = (unsigned char*)&ulVal;
    SwapBytes(buf, pbuf, 4, bSwapBytes);
    fwrite(buf, 1, 4, pf);

    for(int i = 0; i < 6; i++)
    {
        pbuf = (unsigned char*)&cLineStyle.dPattern[i];
        SwapBytes(buf, pbuf, 8, bSwapBytes);
        fwrite(buf, 1, 8, pf);
    }
}

void CDObject::SaveDimension(FILE *pf, bool bSwapBytes, PDDimension pDim)
{
    unsigned char buf[16], *pbuf;

    pbuf = (unsigned char*)&pDim->dRef1;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    pbuf = (unsigned char*)&pDim->dRef2;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    buf[0] = (unsigned char)pDim->iArrowType1;
    fwrite(buf, 1, 1, pf);

    buf[0] = (unsigned char)pDim->iArrowType2;
    fwrite(buf, 1, 1, pf);

    SavePoint(pf, bSwapBytes, pDim->cArrowDim1);
    SavePoint(pf, bSwapBytes, pDim->cArrowDim2);

    pbuf = (unsigned char*)&pDim->dFontSize;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    buf[0] = pDim->bFontAttrs;
    fwrite(buf, 1, 1, pf);

    buf[0] = (unsigned char)strlen(pDim->psFontFace);
    fwrite(buf, 1, 1, pf);
    fwrite(pDim->psFontFace, 1, buf[0], pf);

    SavePoint(pf, bSwapBytes, pDim->cLabelPos.cPoint);

    pbuf = (unsigned char*)&pDim->cLabelPos.dOrientation;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    unsigned long ulVal = 0;
    if(pDim->psLab) ulVal = strlen(pDim->psLab);
    pbuf = (unsigned char*)&ulVal;
    SwapBytes(buf, pbuf, 4, bSwapBytes);
    fwrite(buf, 1, 4, pf);
    if(pDim->psLab) fwrite(pDim->psLab, 1, ulVal, pf);
}

void CDObject::SaveToFile(FILE *pf, bool bSwapBytes)
{
    unsigned char buf[16], *pbuf;

    buf[0] = m_iType;
    fwrite(buf, 1, 1, pf);

    SaveLine(pf, bSwapBytes, m_cLines[0]);
    SaveLine(pf, bSwapBytes, m_cLines[1]);
    SaveRefPoint(pf, bSwapBytes, m_cBounds[0]);
    SaveRefPoint(pf, bSwapBytes, m_cBounds[1]);

    SaveLineStyle(pf, bSwapBytes, m_cLineStyle);

    unsigned long lCnt = m_pInputPoints->GetCount(-1);
    pbuf = (unsigned char*)&lCnt;
    SwapBytes(buf, pbuf, 4, bSwapBytes);
    fwrite(buf, 1, 4, pf);

    CDInputPoint cInPt;
    for(unsigned int i = 0; i < lCnt; i++)
    {
        cInPt = m_pInputPoints->GetPoint(i, -1);
        SaveInputPoint(pf, bSwapBytes, cInPt);
    }

    lCnt = m_pCrossPoints->GetCount();
    pbuf = (unsigned char*)&lCnt;
    SwapBytes(buf, pbuf, 4, bSwapBytes);
    fwrite(buf, 1, 4, pf);

    double dRef;
    for(unsigned int i = 0; i < lCnt; i++)
    {
        dRef = m_pCrossPoints->GetPoint(i);
        SaveReference(pf, bSwapBytes, dRef);
    }

    lCnt = m_pDimens->GetCount();
    pbuf = (unsigned char*)&lCnt;
    SwapBytes(buf, pbuf, 4, bSwapBytes);
    fwrite(buf, 1, 4, pf);

    PDDimension pDim;
    for(unsigned int i = 0; i < lCnt; i++)
    {
        pDim = (PDDimension)m_pDimens->GetItem(i);
        SaveDimension(pf, bSwapBytes, pDim);
    }
}

void CDObject::LoadPoint(FILE *pf, bool bSwapBytes, PDPoint pPoint)
{
    unsigned char buf[16], *pbuf;

    pPoint->x = 0.0;
    fread(buf, 1, 8, pf);
    pbuf = (unsigned char*)&pPoint->x;
    SwapBytes(pbuf, buf, 8, bSwapBytes);

    pPoint->y = 0.0;
    fread(buf, 1, 8, pf);
    pbuf = (unsigned char*)&pPoint->y;
    SwapBytes(pbuf, buf, 8, bSwapBytes);
}

void CDObject::LoadInputPoint(FILE *pf, bool bSwapBytes, PDInputPoint pInPoint)
{
    unsigned char buf;
    fread(&buf, 1, 1, pf);
    pInPoint->iCtrl = buf;
    LoadPoint(pf, bSwapBytes, &pInPoint->cPoint);
}

void CDObject::LoadReference(FILE *pf, bool bSwapBytes, double *pdRef)
{
    unsigned char buf[16], *pbuf;

    fread(buf, 1, 8, pf);
    pbuf = (unsigned char*)pdRef;
    SwapBytes(pbuf, buf, 8, bSwapBytes);
}

void CDObject::LoadRefPoint(FILE *pf, bool bSwapBytes, PDRefPoint pRefPoint)
{
    unsigned char buf;
    fread(&buf, 1, 1, pf);
    pRefPoint->bIsSet = buf;
    pRefPoint->dRef = 0.0;
    LoadReference(pf, bSwapBytes, &pRefPoint->dRef);
}

void CDObject::LoadLine(FILE *pf, bool bSwapBytes, PDLine pLine)
{
    unsigned char buf[16], *pbuf;

    fread(buf, 1, 1, pf);
    pLine->bIsSet = buf[0];
    pLine->cOrigin.x = 0.0;
    pLine->cOrigin.y = 0.0;
    LoadPoint(pf, bSwapBytes, &pLine->cOrigin);
    pLine->cDirection.x = 0.0;
    pLine->cDirection.y = 0.0;
    LoadPoint(pf, bSwapBytes, &pLine->cDirection);

    pLine->dRef = 0.0;
    fread(buf, 1, 8, pf);
    pbuf = (unsigned char*)&pLine->dRef;
    SwapBytes(pbuf, buf, 8, bSwapBytes);
}

void CDObject::LoadLineStyle(FILE *pf, bool bSwapBytes, PDLineStyle pLineStyle)
{
    unsigned char buf[16], *pbuf;

    pLineStyle->dWidth = 0.0;
    fread(buf, 1, 8, pf);
    pbuf = (unsigned char*)&pLineStyle->dWidth;
    SwapBytes(pbuf, buf, 8, bSwapBytes);

    pLineStyle->dPercent = 0.0;
    fread(buf, 1, 8, pf);
    pbuf = (unsigned char*)&pLineStyle->dPercent;
    SwapBytes(pbuf, buf, 8, bSwapBytes);

    fread(buf, 1, 4, pf);
    unsigned long ulVal = 0;
    pbuf = (unsigned char*)&ulVal;
    SwapBytes(pbuf, buf, 4, bSwapBytes);
    pLineStyle->iSegments = ulVal;

    for(int i = 0; i < 6; i++)
    {
        pLineStyle->dPattern[i] = 0.0;
        fread(buf, 1, 8, pf);
        pbuf = (unsigned char*)&pLineStyle->dPattern[i];
        SwapBytes(pbuf, buf, 8, bSwapBytes);
    }
}

void CDObject::LoadDimension(FILE *pf, bool bSwapBytes, PDDimension pDim)
{
    unsigned char buf[16], *pbuf;

    pDim->dRef1 = 0.0;
    fread(buf, 1, 8, pf);
    pbuf = (unsigned char*)&pDim->dRef1;
    SwapBytes(pbuf, buf, 8, bSwapBytes);

    pDim->dRef2 = 0.0;
    fread(buf, 1, 8, pf);
    pbuf = (unsigned char*)&pDim->dRef2;
    SwapBytes(pbuf, buf, 8, bSwapBytes);

    fread(buf, 1, 1, pf);
    pDim->iArrowType1 = buf[0];

    fread(buf, 1, 1, pf);
    pDim->iArrowType2 = buf[0];

    pDim->cArrowDim1.x = 0.0;
    pDim->cArrowDim1.y = 0.0;
    LoadPoint(pf, bSwapBytes, &pDim->cArrowDim1);
    pDim->cArrowDim2.x = 0.0;
    pDim->cArrowDim2.y = 0.0;
    LoadPoint(pf, bSwapBytes, &pDim->cArrowDim2);

    pDim->dFontSize = 0.0;
    fread(buf, 1, 8, pf);
    pbuf = (unsigned char*)&pDim->dFontSize;
    SwapBytes(pbuf, buf, 8, bSwapBytes);

    fread(buf, 1, 1, pf);
    pDim->bFontAttrs = buf[0];

    fread(buf, 1, 1, pf);
    fread(pDim->psFontFace, 1, buf[0], pf);
    pDim->psFontFace[buf[0]] = 0;

    pDim->cLabelPos.cPoint.x = 0.0;
    pDim->cLabelPos.cPoint.y = 0.0;
    LoadPoint(pf, bSwapBytes, &pDim->cLabelPos.cPoint);

    pDim->cLabelPos.dOrientation = 0.0;
    fread(buf, 1, 8, pf);
    pbuf = (unsigned char*)&pDim->cLabelPos.dOrientation;
    SwapBytes(pbuf, buf, 8, bSwapBytes);

    unsigned long ulVal = 0;
    fread(buf, 1, 4, pf);
    pbuf = (unsigned char*)&ulVal;
    SwapBytes(pbuf, buf, 4, bSwapBytes);

    pDim->psLab = NULL;
    if(ulVal > 0)
    {
        pDim->psLab = (char*)malloc((ulVal + 1)*sizeof(char));
        fread(pDim->psLab, 1, ulVal, pf);
        pDim->psLab[ulVal] = 0;
    }

    pDim->bSelected = false;
    pDim->cExt.cPt1 = 0;
    pDim->cExt.cPt2 = 0;
}

bool CDObject::ReadFromFile(FILE *pf, bool bSwapBytes)
{
    unsigned char buf[16], *pbuf;

    LoadLine(pf, bSwapBytes, &m_cLines[0]);
    LoadLine(pf, bSwapBytes, &m_cLines[1]);
    LoadRefPoint(pf, bSwapBytes, &m_cBounds[0]);
    LoadRefPoint(pf, bSwapBytes, &m_cBounds[1]);

    LoadLineStyle(pf, bSwapBytes, &m_cLineStyle);

    unsigned long lCnt = 0;
    fread(buf, 1, 4, pf);
    pbuf = (unsigned char*)&lCnt;
    SwapBytes(pbuf, buf, 4, bSwapBytes);

    CDInputPoint cInPt = {0, {0.0, 0.0}};
    for(unsigned int i = 0; i < lCnt; i++)
    {
        LoadInputPoint(pf, bSwapBytes, &cInPt);
        m_pInputPoints->AddPoint(cInPt.cPoint.x, cInPt.cPoint.y, cInPt.iCtrl);
    }

    fread(buf, 1, 4, pf);
    pbuf = (unsigned char*)&lCnt;
    SwapBytes(pbuf, buf, 4, bSwapBytes);

    double dRef = 0.0;
    for(unsigned int i = 0; i < lCnt; i++)
    {
        LoadReference(pf, bSwapBytes, &dRef);
        m_pCrossPoints->AddPoint(dRef);
    }

    fread(buf, 1, 4, pf);
    pbuf = (unsigned char*)&lCnt;
    SwapBytes(pbuf, buf, 4, bSwapBytes);

    PDDimension pDim;
    for(unsigned int i = 0; i < lCnt; i++)
    {
        pDim = (PDDimension)malloc(sizeof(CDDimension));
        LoadDimension(pf, bSwapBytes, pDim);
        m_pDimens->Add(pDim);
    }
    return true;
}

double CDObject::GetRadiusAtPt(CDLine cPtX, PDLine pPtR, bool bNewPt)
{
    switch(m_iType)
    {
    case dtLine:
        return GetLineRadiusAtPt(cPtX.cOrigin, m_pCachePoints, pPtR, bNewPt);
    case dtCircle:
        return GetCircRadiusAtPt(cPtX.cOrigin, m_pCachePoints, pPtR, bNewPt);
    case dtEllipse:
        return GetElpsRadiusAtPt(cPtX.cOrigin, m_pCachePoints, pPtR, bNewPt, m_pInputPoints, m_cLines);
    case dtArcEllipse:
        return GetArcElpsRadiusAtPt(cPtX.cOrigin, m_pCachePoints, pPtR, bNewPt, m_pInputPoints, m_cLines);
    case dtHyperbola:
        return GetHyperRadiusAtPt(cPtX.cOrigin, m_pCachePoints, pPtR, bNewPt);
    case dtParabola:
        return GetParabRadiusAtPt(cPtX.cOrigin, m_pCachePoints, pPtR, bNewPt);
    case dtSpline:
        return GetSplineRadiusAtPt(cPtX, m_pCachePoints, pPtR, bNewPt);
    case dtEvolvent:
        return GetEvolvRadiusAtPt(cPtX, m_pCachePoints, pPtR, bNewPt);
    case dtGroup:
        return -1.0;
    default:
        return -1.0;
    }
}

bool CDObject::GetDynValue(CDPoint cPt, int iMode, double *pdVal)
{
    bool bRes = false;
    if(iMode == 2)
    {
        *pdVal = m_dMovedDist;
        bRes = true;
    }
    else
    {
        switch(m_iType)
        {
        case dtLine:
            bRes = GetLineAngle(m_pCachePoints, pdVal);
            break;
        case dtCircle:
            bRes = GetCirceRad(m_pCachePoints, pdVal);
            break;
        default:
            break;
        }
    }
    return bRes;
}

void CDObject::BuildRound(CDObject *pObj1, CDObject *pObj2, CDPoint cPt, bool bRestSet, double dRad)
{
    m_pInputPoints->ClearAll();

    CDLine cPtX1, cPtX2;
    double d1 = pObj1->GetDistFromPt(cPt, cPt, false, &cPtX1, NULL);
    double d2 = pObj2->GetDistFromPt(cPt, cPt, false, &cPtX2, NULL);

    double dr = fabs(d1);
    if(bRestSet) dr = dRad;
    else if(fabs(d2) > dr) dr = fabs(d2);

    int i = 0;
    int iMaxIter = 16;
    bool bFound = (fabs(fabs(d1) - dr) < g_dPrec) && (fabs(fabs(d2) - dr) < g_dPrec);
    int iX;
    CDPoint cPt1, cPt2, cX = cPt;
    CDPoint cDir1, cDir2;

    while(!bFound && (i < iMaxIter))
    {
        i++;
        if(cPtX1.bIsSet && cPtX2.bIsSet)
        {
            cDir1 = GetNormal(cPtX1.cDirection);
            cDir2 = GetNormal(cPtX2.cDirection);
            if(d1 > 0) cPt1 = cPtX1.cOrigin + dr*cPtX1.cDirection;
            else cPt1 = cPtX1.cOrigin - dr*cPtX1.cDirection;
            if(d2 > 0) cPt2 = cPtX2.cOrigin + dr*cPtX2.cDirection;
            else cPt2 = cPtX2.cOrigin - dr*cPtX2.cDirection;
            iX = LineXLine(cPt1, cDir1, cPt2, cDir2, &cX);

            if(iX > 0)
            {
                d1 = pObj1->GetDistFromPt(cX, cX, false, &cPtX1, NULL);
                d2 = pObj2->GetDistFromPt(cX, cX, false, &cPtX2, NULL);
                bFound = (fabs(fabs(d1) - dr) < g_dPrec) && (fabs(fabs(d2) - dr) < g_dPrec);
            }
            else i = iMaxIter;
        }
        else i = iMaxIter;
    }

    if(bFound)
    {
        m_pInputPoints->AddPoint(cX.x, cX.y, 1);
        m_pInputPoints->AddPoint(cX.x + dr, cX.y, 0);

        double dDist;
        CDLine cPtX;
        BuildCircCache(cPtX, 0, m_pInputPoints, m_pCachePoints, m_cLines, &dDist);

        cPt2 = cPtX1.cOrigin;
        GetCircDistFromPt(cPt2, cPt2, false, m_pCachePoints, &cPtX1);
        cPt2 = cPtX2.cOrigin;
        GetCircDistFromPt(cPt2, cPt2, false, m_pCachePoints, &cPtX2);

        if(d1 < 0) cDir1 *= -1.0;
        if(d2 < 0) cDir2 *= -1.0;
        cPt1 = Rotate(cDir2, cDir1, false);
        if(cPt1.y < 0)
        {
            SetBound(0, cPtX2);
            SetBound(1, cPtX1);
        }
        else
        {
            SetBound(0, cPtX1);
            SetBound(1, cPtX2);
        }
    }
}

void RotateLine(PDLine pLine, CDPoint cOrig, CDPoint cRot)
{
    CDPoint cDir;
    double dNorm;
    if(pLine->bIsSet)
    {
        cDir = pLine->cOrigin - cOrig;
        dNorm = GetNorm(cDir);
        if(dNorm > g_dPrec)
            pLine->cOrigin = cOrig + Rotate(dNorm*cRot, cDir/dNorm, true);
        pLine->cDirection = Rotate(cRot, pLine->cDirection, true);
    }
}

bool CDObject::RotatePoints(CDPoint cOrig, double dRot, int iDimFlag)
{
    bool bRes = false;
    CDLine cPtX;

    BuildCache(cPtX, 0);

    int iCnt;
    PDDimension pDim;
    if(iDimFlag == 1)
    {
        iCnt = m_pDimens->GetCount();
        for(int i = 0; i < iCnt; i++)
        {
            pDim = (PDDimension)m_pDimens->GetItem(i);
            if(pDim->bSelected)
            {
                pDim->cLabelPos.dOrientation += dRot;
                bRes = true;
            }
        }
        BuildCache(cPtX, 0);
        return bRes;
    }

    CDPoint bPt1, bPt2;
    bool b1 = m_cBounds[0].bIsSet;
    bool b2 = m_cBounds[1].bIsSet;

    m_cBounds[0].bIsSet = false;
    m_cBounds[1].bIsSet = false;

    if(b1) GetNativeRefPoint(m_cBounds[0].dRef, &bPt1);
    if(b2) GetNativeRefPoint(m_cBounds[1].dRef, &bPt2);

    iCnt = m_pInputPoints->GetCount(-1);
    CDPoint cDir, cPt1, cPt2;
    double dNorm;
    CDInputPoint cInPt;
    cPt1.x = cos(dRot);
    cPt1.y = sin(dRot);
    for(int i = 0; i < iCnt; i++)
    {
        cInPt = m_pInputPoints->GetPoint(i, -1);
        cDir = cInPt.cPoint - cOrig;
        dNorm = GetNorm(cDir);
        if(dNorm > g_dPrec)
        {
            cPt2 = cOrig + Rotate(dNorm*cPt1, cDir/dNorm, true);
            m_pInputPoints->SetPoint(i, -1, cPt2.x, cPt2.y, cInPt.iCtrl);
        }
    }
    RotateLine(&m_cLines[0], cOrig, cPt1);
    RotateLine(&m_cLines[1], cOrig, cPt1);

    BuildCache(cPtX, 0);

    if(b1)
    {
        cDir = bPt1 - cOrig;
        dNorm = GetNorm(cDir);
        if(dNorm > g_dPrec) cPt2 = cOrig + Rotate(dNorm*cPt1, cDir/dNorm, true);
        else cPt2 = bPt1;

        GetDistFromPt(cPt2, cPt2, false, &cPtX, NULL);
        m_cBounds[0].dRef = cPtX.dRef;
    }

    if(b2)
    {
        cDir = bPt2 - cOrig;
        dNorm = GetNorm(cDir);
        if(dNorm > g_dPrec) cPt2 = cOrig + Rotate(dNorm*cPt1, cDir/dNorm, true);
        else cPt2 = bPt2;

        GetDistFromPt(cPt2, cPt2, false, &cPtX, NULL);
        m_cBounds[1].dRef = cPtX.dRef;
    }

    m_cBounds[0].bIsSet = b1;
    m_cBounds[1].bIsSet = b2;

    if(iDimFlag > 1)
    {
        iCnt = m_pDimens->GetCount();
        for(int i = 0; i < iCnt; i++)
        {
            pDim = (PDDimension)m_pDimens->GetItem(i);
            pDim->cLabelPos.dOrientation += dRot;
        }
    }
    return true;
}

bool CDObject::MovePoints(CDPoint cDir, double dDist, int iDimFlag)
{
    bool bRes = false;
    CDPoint dx = dDist*cDir;
    int iCnt;
    PDDimension pDim;
    CDLine cPtX;
    if(iDimFlag == 1)
    {
        iCnt = m_pDimens->GetCount();
        for(int i = 0; i < iCnt; i++)
        {
            pDim = (PDDimension)m_pDimens->GetItem(i);
            if(pDim->bSelected)
            {
                pDim->cLabelPos.cPoint += dx;
                bRes = true;
            }
        }
        BuildCache(cPtX, 0);
        return bRes;
    }

    iCnt = m_pInputPoints->GetCount(-1);
    CDInputPoint cInPt;
    for(int i = 0; i < iCnt; i++)
    {
        cInPt = m_pInputPoints->GetPoint(i, -1);
        m_pInputPoints->SetPoint(i, -1, cInPt.cPoint.x + dx.x, cInPt.cPoint.y + dx.y, cInPt.iCtrl);
    }
    if(m_cLines[0].bIsSet) m_cLines[0].cOrigin += dx;
    if(m_cLines[1].bIsSet) m_cLines[1].cOrigin += dx;
    if(iDimFlag > 1)
    {
        iCnt = m_pDimens->GetCount();
        for(int i = 0; i < iCnt; i++)
        {
            pDim = (PDDimension)m_pDimens->GetItem(i);
            pDim->cLabelPos.cPoint += dx;
        }
    }
    BuildCache(cPtX, 0);
    return true;
}

void MirrorLine(PDLine pLine, CDLine cLine)
{
    CDPoint cPt1;
    if(pLine->bIsSet)
    {
        pLine->cOrigin = Mirror(pLine->cOrigin, cLine);
        cPt1 = Rotate(pLine->cDirection, cLine.cDirection, false);
        cPt1.y *= -1.0;
        pLine->cDirection = Rotate(cPt1, cLine.cDirection, true);
    }
}

void CDObject::SwapBounds()
{
    CDRefPoint cLn1 = m_cBounds[0];
    m_cBounds[0] = m_cBounds[1];
    m_cBounds[1] = cLn1;
}

void CDObject::MirrorPoints(CDLine cLine)
{
    CDLine cPtX;

    BuildCache(cPtX, 0);

    CDPoint bPt1, bPt2;
    bool b1 = m_cBounds[0].bIsSet;
    bool b2 = m_cBounds[1].bIsSet;

    m_cBounds[0].bIsSet = false;
    m_cBounds[1].bIsSet = false;

    if(b1) GetNativeRefPoint(m_cBounds[0].dRef, &bPt1);
    if(b2) GetNativeRefPoint(m_cBounds[1].dRef, &bPt2);

    int iCnt = m_pInputPoints->GetCount(-1);
    CDInputPoint cInPt;
    CDPoint cPt1;
    for(int i = 0; i < iCnt; i++)
    {
        cInPt = m_pInputPoints->GetPoint(i, -1);
        cPt1 = Mirror(cInPt.cPoint, cLine);
        m_pInputPoints->SetPoint(i, -1, cPt1.x, cPt1.y, cInPt.iCtrl);
    }
    MirrorLine(&m_cLines[0], cLine);
    MirrorLine(&m_cLines[1], cLine);
    m_pCrossPoints->Clear();
    BuildCache(cPtX, 0);

    if(b1)
    {
        cPt1 = Mirror(bPt1, cLine);
        GetDistFromPt(cPt1, cPt1, false, &cPtX, NULL);
        m_cBounds[0].dRef = cPtX.dRef;
    }

    if(b2)
    {
        cPt1 = Mirror(bPt2, cLine);
        GetDistFromPt(cPt1, cPt1, false, &cPtX, NULL);
        m_cBounds[1].dRef = cPtX.dRef;
    }

    m_cBounds[0].bIsSet = b1;
    m_cBounds[1].bIsSet = b2;

    if((m_iType == 1) && (b1 || b2))
    {
        cInPt = m_pInputPoints->GetPoint(0, -1);
        if(cInPt.iCtrl == 1) SwapBounds();
    }

    if(((m_iType != 1) && (m_iType != 7)) ||
       (b1 && b2 && (m_cBounds[1].dRef < m_cBounds[0].dRef)))
    {
        SwapBounds();
    }
}

bool CDObject::AddCrossPoint(CDPoint cPt, double dDist, PDPtrList pRegions)
{
    CDLine cPtX;
    double d1 = GetDistFromPt(cPt, cPt, false, &cPtX, NULL);
    if(d1 > dDist) return false;
    int i = m_pCrossPoints->GetIndex(cPtX.dRef);

    double dRef;
    if(i < 0) m_pCrossPoints->AddPoint(cPtX.dRef);
    else
    {
        dRef = m_pCrossPoints->GetPoint(i);
        if(cPtX.dRef > dRef - 0.001) m_pCrossPoints->Remove(i);
        else m_pCrossPoints->InsertPoint(i, cPtX.dRef);
    }

    double dLWidth = fabs(m_cLineStyle.dWidth);
    if(dLWidth < 1.0) dLWidth = 1.0;

    PDPolygon pPoly = (PDPolygon)malloc(sizeof(CDPolygon));
    pPoly->iPoints = 4;
    pPoly->pPoints = (PDPoint)malloc(4*sizeof(CDPoint));
    pPoly->pPoints[0].x = cPtX.cOrigin.x - 4.0*dLWidth;
    pPoly->pPoints[0].y = cPtX.cOrigin.y - 4.0*dLWidth;
    pPoly->pPoints[1].x = cPtX.cOrigin.x + 4.0*dLWidth;
    pPoly->pPoints[1].y = cPtX.cOrigin.y - 4.0*dLWidth;
    pPoly->pPoints[2].x = cPtX.cOrigin.x + 4.0*dLWidth;
    pPoly->pPoints[2].y = cPtX.cOrigin.y + 4.0*dLWidth;
    pPoly->pPoints[3].x = cPtX.cOrigin.x - 4.0*dLWidth;
    pPoly->pPoints[3].y = cPtX.cOrigin.y + 4.0*dLWidth;
    pRegions->Add(pPoly);

    return true;
}

bool CDObject::AddCrossPoint(double dRef)
{
    m_pCrossPoints->AddPoint(dRef);
    return true;
}

bool CDObject::GetPointRefDist(double dRef, double *pdDist)
{
    switch(m_iType)
    {
    case dtLine:
        return GetLinePointRefDist(dRef, m_pCachePoints, pdDist);
    case dtCircle:
        return GetCircPointRefDist(dRef, m_pCachePoints, pdDist);
    case dtEllipse:
        return GetElpsPointRefDist(dRef, m_pCachePoints, pdDist);
    case dtArcEllipse:
        return GetArcElpsPointRefDist(dRef, m_pCachePoints, pdDist);
    case dtHyperbola:
        return GetHyperPointRefDist(dRef, m_pCachePoints, pdDist);
    case dtParabola:
        return GetParabPointRefDist(dRef, m_pCachePoints, pdDist);
    case dtSpline:
        return GetSplinePointRefDist(dRef, m_pCachePoints, pdDist);
    case dtEvolvent:
        return GetEvolvPointRefDist(dRef, m_pCachePoints, pdDist);
    case dtGroup:
        return false;
    default:
        return false;
    }
}

double CDObject::GetNearestCrossPoint(CDPoint cPt, PDPoint pPt)
{
    double d1, d2;
    int iCnt = m_pCrossPoints->GetCount();
    if(iCnt < 1) return -1.0;

    CDPoint cPt1;
    GetNativeRefPoint(m_pCrossPoints->GetPoint(0), &cPt1);
    d1 = GetDist(cPt1, cPt);
    *pPt = cPt1;

    for(int i = 1; i < iCnt; i++)
    {
        GetNativeRefPoint(m_pCrossPoints->GetPoint(i), &cPt1);
        d2 = GetDist(cPt1, cPt);
        if(d2 < d1)
        {
            d1 = d2;
            *pPt = cPt1;
        }
    }
    return d1;
}

double CDObject::GetNearestBoundPoint(CDPoint cPt, PDPoint pPt)
{
    CDPoint cPt1;
    double dRes = -1.0;
    if(m_cBounds[0].bIsSet)
    {
        GetNativeRefPoint(m_cBounds[0].dRef, &cPt1);
        *pPt = cPt1;
        dRes = GetDist(cPt, cPt1);
    }

    if(m_cBounds[1].bIsSet)
    {
        GetNativeRefPoint(m_cBounds[1].dRef, &cPt1);
        double d1 = GetDist(cPt, cPt1);
        if((dRes > -0.5) && (d1 < dRes))
        {
            dRes = d1;
            *pPt = cPt1;
        }
    }

    return dRes;
}

bool CDObject::AddDimen(CDPoint cPt, double dDist, PDRect pRect, PDFileAttrs pAttrs)
{
    CDLine cPtX;
    double d1 = GetDistFromPt(cPt, cPt, true, &cPtX, NULL);
    if(d1 > dDist) return false;

    if(!m_bFirstDimSet)
    {
        m_bFirstDimSet = true;
        m_dFirstDimen = cPtX.dRef;
        return false;
    }

    PDDimension pDim = (PDDimension)malloc(sizeof(CDDimension));
    pDim->dRef1 = m_dFirstDimen;
    pDim->dRef2 = cPtX.dRef;
    pDim->iArrowType1 = pAttrs->iArrowType;
    pDim->iArrowType2 = pAttrs->iArrowType;
    pDim->cArrowDim1 = pAttrs->cArrowDim;
    pDim->cArrowDim2 = pAttrs->cArrowDim;
    pDim->dFontSize = pAttrs->dFontSize;
    pDim->bFontAttrs = pAttrs->bFontAttrs;
    pDim->cExt.cPt1 = 0;
    pDim->cExt.cPt2 = 0;
    pDim->bSelected = false;
    strcpy(pDim->psFontFace, pAttrs->sFontFace);

    double d2, d3, dRef3;
    GetPointRefDist(pDim->dRef1, &d1);
    GetPointRefDist(pDim->dRef2, &d2);
    d3 = (d1 + d2)/2.0;

    GetNativeReference(d3, &dRef3);

    CDPoint cPt1, cDir, cNorm;
    GetNativeRefPoint(dRef3, &cPt1);
    GetNativeRefDir(dRef3, &cDir);
    cNorm = GetNormal(cDir);
    double dAng = atan2(cNorm.y, cNorm.x);
    if(dAng < -g_dPrec)
    {
        cNorm *= -1.0;
        dAng += M_PI;
    }
    else if(dAng < g_dPrec)
    {
        dAng = 0.0;
        cNorm.x = 1.0;
        cNorm.y = 0.0;
    }

    pDim->cLabelPos.cPoint = cPt1 - pAttrs->dBaseLine*cNorm;
    pDim->cLabelPos.dOrientation = dAng;

    char sBuf[64];
    switch(m_iType)
    {
    case dtLine:
        strcpy(sBuf, pAttrs->sLengthMask);
        break;
    case dtCircle:
        strcpy(sBuf, pAttrs->sAngleMask);
        break;
    default:
        strcpy(sBuf, "???");
    }
    int iLen = strlen(sBuf) + 1;
    pDim->psLab = (char*)malloc(iLen*sizeof(char));
    strcpy(pDim->psLab, sBuf);

    m_pDimens->Add(pDim);
    m_bFirstDimSet = false;
    return true;
}

void CDObject::AddDimenPtr(PDDimension pDimen)
{
    m_pDimens->Add(pDimen);
}

int CDObject::GetDimenCount()
{
    return m_pDimens->GetCount();
}

PDDimension CDObject::GetDimen(int iPos)
{
    if(iPos < -1) return NULL;
    if(iPos < 0) return &m_cTmpDim;
    return (PDDimension)m_pDimens->GetItem(iPos);
}

int CDObject::PreParseDimText(int iPos, char *sBuf, int iBufLen, double dScale, PDUnitList pUnits)
{
    sBuf[0] = 0;
    PDDimension pDim;
    if(iPos < 0) pDim = &m_cTmpDim;
    else pDim = (PDDimension)m_pDimens->GetItem(iPos);

    if(!pDim->psLab)
    {
        if(iBufLen < 1) return 1;
        *sBuf = 0;
        return 0;
    }

    const char *s1 = GetEscapeOpening(pDim->psLab);
    if(!s1)
    {
        int iLen = GetPlainMaskLen(pDim->psLab) + 1;
        if(iBufLen < iLen) return iLen;

        CopyPlainMask(sBuf, pDim->psLab);
        return 0;
    }

    PDUnit pUnit = GetUnitAtBuf(s1 + 1, pUnits);
    if(!pUnit) return -1;

    double dVal = -1.0;
    if(pUnit->iUnitType == 1)
    {
        double d1, d2;
        GetPointRefDist(pDim->dRef1, &d1);
        GetPointRefDist(pDim->dRef2, &d2);
        dVal = fabs(d2 - d1);
        if(*s1 == '[') dVal /= dScale;
    }
    else if(pUnit->iUnitType == 2)
    {
        CDPoint cDir1, cDir2, cPt1;
        GetNativeRefDir(pDim->dRef1, &cDir1);
        GetNativeRefDir(pDim->dRef2, &cDir2);
        cPt1 = Rotate(cDir1, cDir2, false);
        dVal = fabs(180.0*atan2(cPt1.y, cPt1.x)/M_PI);
    }
    else return -1;

    return PreParseValue(pDim->psLab, pUnits, dVal, dScale, sBuf, iBufLen);
}

int CDObject::GetUnitMask(int iUnitType, char *psBuf, PDUnitList pUnits)
{
    PDDimension pDim = NULL;
    int iRes = 0;
    int iLen;

    bool bFound = false;
    int i = 0;
    while(!bFound && (i < m_pDimens->GetCount()))
    {
        pDim = (PDDimension)m_pDimens->GetItem(i++);
        if(pDim->psLab)
        {
            iLen = strlen(pDim->psLab);
            if(iLen > 0)
            {
                if(*pDim->psLab == '*') iLen--;
            }
            if((iLen > 0) && (iLen < 64))
            {
                iRes = GuessMaskUnitType(pDim->psLab, pUnits);
                bFound = (iRes == iUnitType);
            }
        }
    }

    if(bFound && pDim)
    {
        if(*pDim->psLab == '*') strcpy(psBuf, &pDim->psLab[1]);
        else strcpy(psBuf, pDim->psLab);
        return 0;
    }

    return -1;
}

bool CDObject::ChangeUnitMask(int iUnitType, char *psMask, PDUnitList pUnits)
{
    PDDimension pDim = NULL;
    int iMaskLen = strlen(psMask);
    bool bStar;
    int iLen, iNewLen, iRes;
    bool bRes = false;

    for(int i = 0; i < m_pDimens->GetCount(); i++)
    {
        pDim = (PDDimension)m_pDimens->GetItem(i);
        if(pDim->psLab)
        {
            iLen = strlen(pDim->psLab);
            if(iLen > 0)
            {
                bStar = (*pDim->psLab == '*');
                if(bStar) iLen--;
            }
            if((iLen > 0) && (iLen < 64))
            {
                iRes = GuessMaskUnitType(pDim->psLab, pUnits);
                if(iRes == iUnitType)
                {
                    free(pDim->psLab);
                    pDim->psLab = NULL;
                    iNewLen = iMaskLen;
                    if(bStar) iNewLen++;
                    if(iNewLen > 0)
                    {
                        pDim->psLab = (char*)malloc((iNewLen + 1)*sizeof(char));
                        if(bStar)
                        {
                            *pDim->psLab = '*';
                            if(iMaskLen > 0) strcpy(&pDim->psLab[1], psMask);
                            else pDim->psLab[1] = 0;
                        }
                        else strcpy(pDim->psLab, psMask);
                    }
                    bRes = true;
                }
            }
        }
    }
    return bRes;
}

void CDObject::Rescale(double dRatio, bool bWidths, bool bPatterns, bool bArrows, bool bLabels)
{
    if(m_cLines[0].bIsSet) m_cLines[0].cOrigin *= dRatio;
    if(m_cLines[1].bIsSet) m_cLines[1].cOrigin *= dRatio;

    bool bBnds[2];
    bBnds[0] = m_cBounds[0].bIsSet;
    bBnds[1] = m_cBounds[1].bIsSet;

    CDPoint cBnds[2];
    if(bBnds[0]) GetNativeRefPoint(m_cBounds[0].dRef, &cBnds[0]);
    if(bBnds[1]) GetNativeRefPoint(m_cBounds[1].dRef, &cBnds[1]);
    m_cBounds[0].bIsSet = false;
    m_cBounds[1].bIsSet = false;

    CDPoint cPt;
    PDPointList pCrossPoints = new CDPointList();
    for(int i = 0; i < m_pCrossPoints->GetCount(); i++)
    {
        GetNativeRefPoint(m_pCrossPoints->GetPoint(i), &cPt);
        pCrossPoints->AddPoint(cPt.x, cPt.y, 0);
    }

    PDDimension pDim;
    PDPointList pDimens = new CDPointList();
    for(int i = 0; i < m_pDimens->GetCount(); i++)
    {
        pDim = (PDDimension)m_pDimens->GetItem(i);
        GetNativeRefPoint(pDim->dRef1, &cPt);
        pDimens->AddPoint(cPt.x, cPt.y, 0);
        GetNativeRefPoint(pDim->dRef2, &cPt);
        pDimens->AddPoint(cPt.x, cPt.y, 0);
    }

    m_pUndoPoints->ClearAll();
    m_pCachePoints->ClearAll();

    CDInputPoint cInPt;
    for(int i = 0; i < m_pInputPoints->GetCount(-1); i++)
    {
        cInPt = m_pInputPoints->GetPoint(i, -1);
        cInPt.cPoint *= dRatio;
        m_pInputPoints->SetPoint(i, -1, cInPt.cPoint.x, cInPt.cPoint.y, cInPt.iCtrl);
    }

    CDLine cLine;
    BuildCache(cLine, 0);

    for(int i = 0; i < m_pDimens->GetCount(); i++)
    {
        pDim = (PDDimension)m_pDimens->GetItem(i);

        cInPt = pDimens->GetPoint(2*i, -1);
        cInPt.cPoint *= dRatio;
        GetDistFromPt(cInPt.cPoint, cInPt.cPoint, false, &cLine, NULL);
        pDim->dRef1 = cLine.dRef;

        cInPt = pDimens->GetPoint(2*i + 1, -1);
        cInPt.cPoint *= dRatio;
        GetDistFromPt(cInPt.cPoint, cInPt.cPoint, false, &cLine, NULL);
        pDim->dRef2 = cLine.dRef;

        pDim->cLabelPos.cPoint *= dRatio;
        if(bArrows)
        {
            pDim->cArrowDim1 *= dRatio;
            pDim->cArrowDim2 *= dRatio;
        }
        if(bLabels) pDim->dFontSize *= dRatio;
    }
    delete pDimens;

    m_pCrossPoints->Clear();
    for(int i = 0; i < pCrossPoints->GetCount(-1); i++)
    {
        cInPt = pCrossPoints->GetPoint(i, -1);
        cInPt.cPoint *= dRatio;
        GetDistFromPt(cInPt.cPoint, cInPt.cPoint, false, &cLine, NULL);
        m_pCrossPoints->AddPoint(cLine.dRef);
    }
    delete pCrossPoints;

    if(bBnds[0])
    {
        cBnds[0] *= dRatio;
        GetDistFromPt(cBnds[0], cBnds[0], false, &cLine, NULL);
        m_cBounds[0].dRef = cLine.dRef;
        m_cBounds[0].bIsSet = true;
    }
    if(bBnds[1])
    {
        cBnds[1] *= dRatio;
        GetDistFromPt(cBnds[1], cBnds[1], false, &cLine, NULL);
        m_cBounds[1].dRef = cLine.dRef;
        m_cBounds[1].bIsSet = true;
    }

    if(bWidths) m_cLineStyle.dWidth *= dRatio;

    if(bPatterns)
    {
        for(int i = 0; i < 2*m_cLineStyle.iSegments; i++) m_cLineStyle.dPattern[i] *= dRatio;
        for(int i = 2*m_cLineStyle.iSegments; i < 6; i++) m_cLineStyle.dPattern[i] = 0.0;
    }
}

CDPoint CDObject::GetPointToDir(CDPoint cPoint, double dAngle, CDPoint cPtTarget)
{
    CDPoint cRes = cPoint;

    int iIter = 4, i = 0;
    CDLine cPtX;
    CDPoint cMainDir, cDir1, cDir2, cPt1;

    CDLine cRad;
    double dd = GetDistFromPt(cPtTarget, cPtTarget, false, &cPtX, NULL);
    double dr = GetRadiusAtPt(cPtX, &cRad, false);
    double dta = tan(dAngle);
    double dta2 = 1.0 + Power2(dta);
    double dv, du, dtf, df;
    double dd1;
    double dPi2Prec = M_PI/2.0 + g_dPrec;

    if(cRad.bIsSet)
    {
        dv = (sqrt(Power2(dr) + dta2*(Power2(dd) + 2.0*dr*dd)) - dr)/dta2;
        du = dv*dta;
        dtf = du/(dr + dv);

        df = atan(dtf);
        if(fabs(dAngle) < dPi2Prec) df *= -1.0;
        cDir1.x = cos(df);
        cDir1.y = sin(df);
        cDir2 = Rotate(cDir1, cRad.cDirection, true);
        cRes = cRad.cOrigin + dr*cDir2;
    }
    else
    {
        GetNativeRefDir(cPtX.dRef, &cMainDir);
        cDir1.x = cos(dAngle);
        cDir1.y = sin(dAngle);
        cRad.cDirection = GetNormal(cMainDir);
        cDir2 = Rotate(cDir1, cRad.cDirection, true);
        LineXLine(cRes, cMainDir, cPtTarget, cDir2, &cPt1);
        cRes = cPt1;
    }

    GetDistFromPt(cRes, cRes, false, &cPtX, NULL);

    if(cPtX.bIsSet)
    {
        cDir1 = cPtTarget - cPtX.cOrigin;
        cDir2 = Rotate(cDir1, cRad.cDirection, false);
        df = atan2(cDir2.y, cDir2.x);
    }
    else i = iIter;

    while((i < iIter) && (fabs(df - dAngle) > g_dPrec))
    {
        dr = GetRadiusAtPt(cPtX, &cRad, false);

        if(cRad.bIsSet)
        {
            cMainDir = cPtTarget - cRad.cOrigin;
            dd1 = GetNorm(cMainDir);
            cRad.cDirection = cMainDir/dd1;
            dd = dd1 - dr;
            dv = (sqrt(Power2(dr) + dta2*(Power2(dd) + 2.0*dr*dd)) - dr)/dta2;
            du = dv*dta;
            dtf = du/(dr + dv);

            df = atan(dtf);
            if(fabs(dAngle) < dPi2Prec) df *= -1.0;
            cDir1.x = cos(df);
            cDir1.y = sin(df);
            cDir2 = Rotate(cDir1, cRad.cDirection, true);
            cRes = cRad.cOrigin + dr*cDir2;
        }
        else
        {
            GetNativeRefDir(cPtX.dRef, &cMainDir);
            cDir1.x = cos(dAngle);
            cDir1.y = sin(dAngle);
            cRad.cDirection = GetNormal(cMainDir);
            cDir2 = Rotate(cDir1, cRad.cDirection, true);
            LineXLine(cRes, cMainDir, cPtTarget, cDir2, &cPt1);
            cRes = cPt1;
        }

        GetDistFromPt(cRes, cRes, false, &cPtX, NULL);

        if(cPtX.bIsSet)
        {
            cDir1 = cPtTarget - cPtX.cOrigin;
            cDir2 = Rotate(cDir1, cRad.cDirection, false);
            df = atan2(cDir2.y, cDir2.x);
            i++;
        }
        else i = iIter;
    }

    return cPtX.cOrigin;
}

void CDObject::GetDimFontAttrs(int iPos, PDFileAttrs pAttrs)
{
    if(iPos < 0) return;

    PDDimension pDim = (PDDimension)m_pDimens->GetItem(iPos);
    pAttrs->dFontSize = pDim->dFontSize;
    pAttrs->bFontAttrs = pDim->bFontAttrs;
    strcpy(pAttrs->sFontFace, pDim->psFontFace);
}

bool CDObject::DeleteSelDimens(PDRect pRect, PDPtrList pRegions)
{
    bool bRes = false;
    PDDimension pDim;
    int i = m_pDimens->GetCount();
    while(i > 0)
    {
        pDim = (PDDimension)m_pDimens->GetItem(--i);
        if(pDim->bSelected)
        {
            bRes = true;
            if(pDim->psLab) free(pDim->psLab);
            free(pDim);
            m_pDimens->Remove(i);
            AddRegions(pRegions, 9);
            AddRegions(pRegions, 10);
        }
    }
    if(bRes && pRect)
    {
        CDLine cLn;
        cLn.bIsSet = false;
        BuildPrimitives(cLn, 0, pRect, 0, NULL);
    }
    return bRes;
}

bool CDObject::GetSelectedDimen(PDDimension pDimen)
{
    bool bFound = false;
    int i = 0;
    int n = m_pDimens->GetCount();
    PDDimension pDim1;
    while(!bFound && (i < n))
    {
        pDim1 = (PDDimension)m_pDimens->GetItem(i++);
        bFound = pDim1->bSelected;
    }
    if(bFound) CopyDimenAttrs(pDimen, pDim1);
    return bFound;
}

bool CDObject::SetSelectedDimen(PDDimension pDimen, PDPtrList pRegions)
{
    bool bRes = false;
    int n = m_pDimens->GetCount();
    PDDimension pDim1;
    for(int i = 0; i < n; i++)
    {
        pDim1 = (PDDimension)m_pDimens->GetItem(i);
        if(pDim1->bSelected)
        {
            CopyDimenAttrs(pDim1, pDimen);
            AddRegions(pRegions, 9);
            AddRegions(pRegions, 10);
            bRes = true;
        }
    }
    return bRes;
}

bool CDObject::GetSnapTo()
{
    return m_bSnapTo;
}

void CDObject::SetSnapTo(bool bSnap)
{
    m_bSnapTo = bSnap;
}

void CDObject::AddRegions(PDPtrList pRegions, int iPrimType)
{
    PDPoint pPts1, pPts2;
    CDPoint cDim;
    int i1, i2;
    CDPrimitive cPrim;
    PDPolygon pPoly;
    double dScale = pRegions->GetDblVal();
    double dLWidth = fabs(m_cLineStyle.dWidth);
    if(dLWidth < 1.0) dLWidth = 1.0;

    for(int i = 0; i < m_pPrimitive->GetCount(); i++)
    {
        cPrim = m_pPrimitive->GetPrimitive(i);
        if((iPrimType < 0) || (iPrimType == cPrim.iType))
        {
            cDim = GetPrimRegion(cPrim, dLWidth, dScale, NULL, NULL);
            i1 = cDim.x;
            i2 = cDim.y;
            pPts1 = NULL;
            pPts2 = NULL;
            if(i1 > 0) pPts1 = (PDPoint)malloc(i1*sizeof(CDPoint));
            if(i2 > 0) pPts2 = (PDPoint)malloc(i2*sizeof(CDPoint));

            cDim = GetPrimRegion(cPrim, dLWidth, dScale, pPts1, pPts2);
            i1 = cDim.x;
            i2 = cDim.y;
            if(i1 > 0)
            {
                pPoly = (PDPolygon)malloc(sizeof(CDPolygon));
                pPoly->iPoints = i1;
                pPoly->pPoints = pPts1;
                pRegions->Add(pPoly);
            }
            else if(pPts1) free(pPts1);
            if(i2 > 0)
            {
                pPoly = (PDPolygon)malloc(sizeof(CDPolygon));
                pPoly->iPoints = i2;
                pPoly->pPoints = pPts2;
                pRegions->Add(pPoly);
            }
            else if(pPts2) free(pPts2);
        }
    }
}

void CDObject::SetAuxInt(int iVal)
{
    m_iAuxInt = iVal;
}

int CDObject::GetAuxInt()
{
    return m_iAuxInt;
}

int CDObject::GetNumParts()
{
    switch(m_iType)
    {
    case dtEllipse:
        return GetElpsNumParts(m_pCachePoints, m_cBounds);
    case dtArcEllipse:
        return GetArcElpsNumParts(m_pCachePoints, m_cBounds);
    case dtHyperbola:
        return GetHyperNumParts(m_pCachePoints, m_cBounds);
    case dtParabola:
        return GetParabNumParts(m_pCachePoints, m_cBounds);
    default:
        return 0;
    }
    return 0;
}

bool CDObject::RemovePart(bool bDown, PDRefPoint pBounds)
{
    CDRefPoint cBounds[2];
    cBounds[0] = pBounds[0];
    cBounds[1] = pBounds[1];

    bool bRes = false;
    switch(m_iType)
    {
    case dtEllipse:
        bRes = ElpsRemovePart(bDown, m_pCachePoints, cBounds);
        break;
    case dtArcEllipse:
        bRes = ArcElpsRemovePart(bDown, m_pCachePoints, cBounds);
        break;
    case dtHyperbola:
        bRes = HyperRemovePart(bDown, m_pCachePoints, cBounds);
        break;
    case dtParabola:
        bRes = ParabRemovePart(bDown, m_pCachePoints, cBounds);
        break;
    default:
        break;
    }
    if(bRes)
    {
        m_cBounds[0] = cBounds[0];
        m_cBounds[1] = cBounds[1];
    }
    return bRes;
}

CDObject* CDObject::SplitPart(PDRect pRect, PDPtrList pRegions)
{
    CDRefPoint cBounds[2];
    cBounds[0] = m_cBounds[0];
    cBounds[1] = m_cBounds[1];

    PDObject pNewObj = NULL;
    if(RemovePart(false, cBounds))
    {
        CDLine cLn;
        cLn.bIsSet = false;

        BuildPrimitives(cLn, 0, pRect, 0, NULL);
        AddRegions(pRegions, -1);

        pNewObj = Copy();
        pNewObj->BuildCache(cLn, 0);
        if(pNewObj->RemovePart(true, cBounds))
        {
            pNewObj->BuildPrimitives(cLn, 0, pRect, 0, NULL);
            pNewObj->SetSelected(true, false, -1, pRegions);
        }
        else
        {
            delete pNewObj;
            pNewObj = NULL;
        }
    }
    return pNewObj;
}


// CDataList

CDataList::CDataList()
{
    m_iDataLen = 0;
    m_iDataSize = 16;
    m_ppObjects = (PDObject*)malloc(m_iDataSize*sizeof(PDObject));
    m_bHasChanged = false;
}

CDataList::~CDataList()
{
    for(int i = 0; i < m_iDataLen; i++)
    {
        delete m_ppObjects[i];
    }
    free(m_ppObjects);
}

int CDataList::GetCount()
{
    return m_iDataLen;
}

void CDataList::Add(PDObject pObject)
{
    CDLine cPtX;
    if(!pObject->BuildCache(cPtX, 0)) return;
    if(m_iDataLen >= m_iDataSize)
    {
        m_iDataSize += 16;
        m_ppObjects = (PDObject*)realloc(m_ppObjects, m_iDataSize*sizeof(PDObject));
    }
    m_ppObjects[m_iDataLen++] = pObject;
    m_bHasChanged = true;
    return;
}

void CDataList::Remove(int iIndex, bool bFree)
{
    if(bFree) delete m_ppObjects[iIndex];

    m_iDataLen--;
    if(iIndex < m_iDataLen)
    {
        memmove(&m_ppObjects[iIndex], &m_ppObjects[iIndex + 1],
            (m_iDataLen - iIndex)*sizeof(PDObject));
    }
}

PDObject CDataList::GetItem(int iIndex)
{
    return m_ppObjects[iIndex];
}

void CDataList::BuildAllPrimitives(PDRect pRect, bool bResolvePatterns)
{
    CDLine cLn;
    cLn.bIsSet = false;
    int iTemp = 0;
    if(!bResolvePatterns) iTemp = 1;
    for(int i = 0; i < m_iDataLen; i++)
    {
        m_ppObjects[i]->BuildPrimitives(cLn, 0, pRect, iTemp, NULL);
    }
}

PDObject CDataList::SelectByPoint(CDPoint cPt, double dDist, int *piDimen)
{
    int i = 0;
    bool bFound = false;
    while(!bFound && (i < m_iDataLen))
    {
        bFound = m_ppObjects[i++]->IsNearPoint(cPt, dDist, piDimen);
    }
    return bFound ? m_ppObjects[i - 1] : NULL;
}

PDObject CDataList::SelectLineByPoint(CDPoint cPt, double dDist)
{
    int i = 0;
    bool bFound = false;
    PDObject pObj = NULL;
    int iDimen;
    while(!bFound && (i < m_iDataLen))
    {
        pObj = m_ppObjects[i++];
        bFound = (pObj->GetType() == 1) && pObj->IsNearPoint(cPt, dDist, &iDimen);
    }
    return bFound ? pObj : NULL;
}

void CDataList::ClearSelection(PDPtrList pRegions)
{
    for(int i = 0; i < m_iDataLen; i++)
    {
        m_ppObjects[i]->SetSelected(false, false, -1, pRegions);
    }
}

int CDataList::GetNumOfSelectedLines()
{
    int iRes = 0;
    for(int i = 0; i < m_iDataLen; i++)
    {
        if(m_ppObjects[i]->GetSelected() && (m_ppObjects[i]->GetType() == 1)) iRes++;
    }
    return iRes;
}

PDObject CDataList::GetSelectedLine(int iIndex)
{
    PDObject pRes = NULL;
    int i = 0;
    int iCurSelLine = -1;
    while((i < m_iDataLen) && (iCurSelLine < iIndex))
    {
        pRes = m_ppObjects[i++];
        if(pRes->GetSelected() && (pRes->GetType() == 1)) iCurSelLine++;
    }

    if(iCurSelLine >= iIndex) return pRes;

    return NULL;
}

int CDataList::GetNumOfSelectedCircles()
{
    int iRes = 0;
    for(int i = 0; i < m_iDataLen; i++)
    {
        if(m_ppObjects[i]->GetSelected() && (m_ppObjects[i]->GetType() == 2)) iRes++;
    }
    return iRes;
}

PDObject CDataList::GetSelectedCircle(int iIndex)
{
    PDObject pRes = NULL;
    int i = 0;
    int iCurSelLine = -1;
    while((i < m_iDataLen) && (iCurSelLine < iIndex))
    {
        pRes = m_ppObjects[i++];
        if(pRes->GetSelected() && (pRes->GetType() == 2)) iCurSelLine++;
    }

    if(iCurSelLine >= iIndex) return pRes;

    return NULL;
}

int CDataList::GetTangSnap(CDPoint cPt, double dDist, bool bNewPt, PDLine pSnapPt, PDObject pObj, PDObject pDynObj)
{
    CDLine cPtX1;
    cPtX1.dRef = 0.0;
    pObj->GetDistFromPt(cPt, cPt, false, &cPtX1, NULL);
    if(!cPtX1.bIsSet) return 0;

    CDLine cRad1, cRad2;
    double d1 = pObj->GetRadiusAtPt(cPtX1, &cRad1, false);
    double d2 = pDynObj->GetRadiusAtPt(cPtX1, &cRad2, bNewPt);
    //int iType1 = pObj->GetType();
    int iType2 = pDynObj->GetType();
    CDInputPoint cInPt1;
    double d3;

    double dcos = cRad1.cDirection*cRad2.cDirection;
    if(fabs(dcos) < 0.1)
    {
        if(iType2 == 1)
        {
            if(!pDynObj->GetPoint(0, 0, &cInPt1)) return 0;
            pObj->GetDistFromPt(cInPt1.cPoint, cPt, false, &cPtX1, NULL);
            if(cPtX1.bIsSet)
            {
                d3 = GetDist(cPtX1.cOrigin, cPt);
                if(fabs(d3) < dDist)
                {
                    *pSnapPt = cPtX1;
                    return 1;
                }
            }
        }
        return 0;
    }

    double dsin = Deter2(cRad1.cDirection, cRad2.cDirection);
    if(fabs(dsin) < 0.1)
    {
        bool bFound = fabs(dsin) < g_dPrec;
        int i = 0;
        int iIterMax = 16;
        CDPoint cPt1, cPt2;
        double dDir;

        if(cRad1.bIsSet)
        {
            cPt2 = cPt;
            while(cRad1.bIsSet && !bFound && (i < iIterMax))
            {
                if(cRad2.bIsSet) pObj->GetDistFromPt(cRad2.cOrigin, cPt2, false, &cPtX1, NULL);
                else
                {
                    dDir = 1.0;
                    cPt1 = cPt2 - cRad1.cOrigin;
                    dsin = GetNorm(cPt1);
                    if(dsin > g_dPrec)
                    {
                        if(cPt1*cRad2.cDirection/dsin < -0.5) dDir = -1.0;
                    }
                    cPt1 = cRad1.cOrigin + dDir*d1*cRad2.cDirection;
                    pObj->GetDistFromPt(cPt1, cPt1, false, &cPtX1, NULL);
                }

                i++;
                d1 = pObj->GetRadiusAtPt(cPtX1, &cRad1, false);
                d2 = pDynObj->GetRadiusAtPt(cPtX1, &cRad2, bNewPt);
                dsin = Deter2(cRad1.cDirection, cRad2.cDirection);
                bFound = (fabs(dsin) < g_dPrec);
                cPt2 = cPtX1.cOrigin;
            }

            if(bFound && (GetDist(cPt, cPtX1.cOrigin) < dDist))
            {
                *pSnapPt = cPtX1;
                return 1;
            }
        }
        else if(cRad2.bIsSet)
        {
            cPt2 = cPt;
            while(cRad2.bIsSet && !bFound && (i < iIterMax))
            {
                dDir = 1.0;
                if((cPt2 - cRad2.cOrigin)*cRad1.cDirection < -0.5) dDir = -1.0;
                cPt1 = cRad2.cOrigin + dDir*d2*cRad1.cDirection;
                pObj->GetDistFromPt(cPt1, cPt1, false, &cPtX1, NULL);

                i++;
                d1 = pObj->GetRadiusAtPt(cPtX1, &cRad1, false);
                d2 = pDynObj->GetRadiusAtPt(cPtX1, &cRad2, bNewPt);
                dsin = Deter2(cRad1.cDirection, cRad2.cDirection);
                bFound = (fabs(dsin) < g_dPrec);
                cPt2 = cPtX1.cOrigin;
            }

            if(bFound && (GetDist(cPt, cPtX1.cOrigin) < dDist))
            {
                *pSnapPt = cPtX1;
                return 1;
            }
        }
    }

    return 0;
}

int CDataList::GetSnapPoint(int iSnapMask, CDPoint cPt, double dDist, PDLine pSnapPt, PDObject pDynObj)
{
//wchar_t buf[128];
//swprintf(buf, L"Tol: %f", dDist);
//SendMessage(g_hStatus, SB_SETTEXT, 2, (LPARAM)L"");

    pSnapPt->bIsSet = false;

    int i = 0;
    bool bFound1 = false;
    CDLine cPtSnap1;
    double dDist1, dblDist;
    PDObject pObj1 = NULL;
    dblDist = 2.0*dDist;
    int iDim;

    while(!bFound1 && (i < m_iDataLen))
    {
        pObj1 = m_ppObjects[i++];
        if(pObj1->GetSnapTo())
        {
            dDist1 = fabs(pObj1->GetDistFromPt(cPt, cPt, true, &cPtSnap1, &iDim));
            bFound1 = cPtSnap1.bIsSet && (dDist1 < dblDist);
        }
    }

    if(bFound1 && (GetNorm(cPtSnap1.cDirection) < 0.5))
    {
        *pSnapPt = cPtSnap1;
        return 1;
    }

    double dDist2;
    bool bFound2 = false;
    CDLine cPtSnap2;
    PDObject pObj2 = NULL;

    while(!bFound2 && (i < m_iDataLen))
    {
        pObj2 = m_ppObjects[i++];
        if(pObj2->GetSnapTo())
        {
            dDist2 = fabs(pObj2->GetDistFromPt(cPt, cPt, true, &cPtSnap2, &iDim));
            bFound2 = cPtSnap2.bIsSet && (dDist2 < dblDist);
        }
    }

    if(!(bFound1 || bFound2)) return 0;

    if(bFound2 && (GetNorm(cPtSnap2.cDirection) < 0.5))
    {
        *pSnapPt = cPtSnap2;
        return 1;
    }

    *pSnapPt = cPtSnap1;

    if(!bFound2)
    {
        CDPoint cPt2;

        if((iSnapMask == 1) && pObj1->GetSelected())
        {
            dDist2 = pObj1->GetNearestCrossPoint(cPt, &cPt2);
            if((dDist2 > -0.5) && (dDist2 < dblDist))
            {
                pSnapPt->bIsSet = true;
                pSnapPt->cOrigin = cPt2;
                pSnapPt->cDirection = 0;
                return 1;
            }
        }

        if(pDynObj)
        {
            int iRes = GetTangSnap(cPt, dblDist, iSnapMask != 2, pSnapPt, pObj1, pDynObj);
            if(iRes > 0) return iRes;
        }

        dDist2 = pObj1->GetNearestBoundPoint(cPt, &cPt2);
        if((dDist2 > -0.5) && (dDist2 < dblDist))
        {
            pSnapPt->bIsSet = true;
            pSnapPt->cOrigin = cPt2;
            pSnapPt->cDirection = 0;
            return 1;
        }

        if(dDist1 < dDist)
        {
            return 1;
        }

        i = 0;
        while(!bFound2 && (i < m_iDataLen))
        {
            pObj2 = m_ppObjects[i++];
            if(pObj2->GetSnapTo())
            {
                dDist2 = fabs(pObj2->GetDistFromPt(cPt, cPt, true, &cPtSnap2, &iDim));
                bFound2 = dDist2 < dDist;
            }
        }

        if(bFound2)
        {
            *pSnapPt = cPtSnap2;
            return 1;
        }
        return 0;
    }

    double df1, df2;
    df1 = GetNorm(cPtSnap1.cDirection);
    df2 = GetNorm(cPtSnap2.cDirection);

    if(df1 < g_dPrec) return 1;
    if(df2 < g_dPrec)
    {
        *pSnapPt = cPtSnap2;
        return 1;
    }

    CDPoint cX, cDir = {0, 0};
    int iX = LineXLine(cPtSnap1.cOrigin, GetNormal(cPtSnap1.cDirection),
        cPtSnap2.cOrigin, GetNormal(cPtSnap2.cDirection), &cX);
    if(iX < 1) return 1;

    int iIterMax = 16;
    int iIter = 0;
    double dDist3 = GetDist(cPt, cX);
    dDist1 = fabs(pObj1->GetDistFromPt(cX, cX, true, &cPtSnap1, NULL));
    dDist2 = fabs(pObj2->GetDistFromPt(cX, cX, true, &cPtSnap2, NULL));
    bFound2 = dDist3 < dblDist;

    df1 = GetNorm(cPtSnap1.cDirection);
    df2 = GetNorm(cPtSnap2.cDirection);
    double dIncl;

    if(df1 < g_dPrec)
    {
        *pSnapPt = cPtSnap1;
        return 1;
    }
    if(df2 < g_dPrec)
    {
        *pSnapPt = cPtSnap2;
        return 1;
    }

    bool bDoIter = bFound2;
    while(bDoIter)
    {
        iIter++;
        dIncl = 0.0;
        iX = LineXLine(cPtSnap1.cOrigin, GetNormal(cPtSnap1.cDirection),
            cPtSnap2.cOrigin, GetNormal(cPtSnap2.cDirection), &cX);
        if(iX > 0)
        {
            dDist3 = GetDist(cPt, cX);
            dDist1 = fabs(pObj1->GetDistFromPt(cX, cX, true, &cPtSnap1, NULL));
            dDist2 = fabs(pObj2->GetDistFromPt(cX, cX, true, &cPtSnap2, NULL));
            bFound2 = dDist3 < dblDist;

            df1 = GetNorm(cPtSnap1.cDirection);
            df2 = GetNorm(cPtSnap2.cDirection);
            if(df1 < g_dPrec)
            {
                cX = cPtSnap1.cOrigin;
                cDir = cPtSnap1.cDirection;
                iIter = iIterMax;
            }
            else if(df2 < g_dPrec)
            {
                cX = cPtSnap2.cOrigin;
                cDir = cPtSnap2.cDirection;
                iIter = iIterMax;
            }
            else dIncl = fabs(cPtSnap1.cDirection*cPtSnap2.cDirection);
        }
        else bFound2 = false;

        bDoIter = bFound2 && (iIter < iIterMax) &&
            ((dDist1 > g_dRootPrec) || (dDist2 > g_dRootPrec) || (iIter < 4) ||
             ((dIncl > 1.0 - g_dPrec) && (dIncl < 1.0 - g_dRootPrec)));
        /*if(!bDoIter && bFound2 && (iIter < iIterMax))
        {
            // do one more iteration
            iIter = iIterMax - 1;
            bDoIter = true;
        }*/
    }

//swprintf(buf, L"%d", iIter);
//SendMessage(g_hStatus, SB_SETTEXT, 2, (LPARAM)buf);
    if(bFound2)
    {
        pSnapPt->bIsSet = true;
        pSnapPt->cOrigin = cX;
        pSnapPt->cDirection = 0;
    }

    return 1;
}

bool CDataList::DeleteSelected(CDataList *pUndoList, PDRect pRect, PDPtrList pRegions)
{
    bool bRes = false;
    PDObject pObj;
    for(int i = m_iDataLen - 1; i >= 0; i--)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            pObj->AddRegions(pRegions, -1);
            bRes = true;
            pUndoList->Add(pObj);
            m_iDataLen--;
            if(i < m_iDataLen)
            {
                memmove(&m_ppObjects[i], &m_ppObjects[i + 1], (m_iDataLen - i)*sizeof(PDObject));
            }
        }
        else bRes |= pObj->DeleteSelDimens(pRect, pRegions);
    }
    if(bRes) m_bHasChanged = true;
    return bRes;
}

int CDataList::GetSelectCount()
{
    int iRes = 0;
    PDObject pObj;
    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected()) iRes++;
    }
    return iRes;
}

PDObject CDataList::GetSelected(int iIndex)
{
    int i = 0;
    PDObject pObj = NULL;
    while((i < m_iDataLen) && (iIndex > -1))
    {
        pObj = m_ppObjects[i++];
        if(pObj->GetSelected()) iIndex--;
    }
    return iIndex < 0 ? pObj : NULL;
}

bool CDataList::CutSelected(CDPoint cPt, double dDist, PDRect pRect, PDPtrList pRegions)
{
    bool bRes = false;
    PDObject pObj, pObjNew = NULL;
    int n = m_iDataLen;
    for(int i = 0; i < n; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            if(pObj->Split(cPt, dDist, pRect, &pObjNew, pRegions))
            {
                bRes = true;
                if(pObjNew)
                {
                    Add(pObjNew);
                    pObjNew = NULL;
                }
            }
        }
    }
    if(bRes) m_bHasChanged = true;
    return bRes;
}

bool CDataList::ExtendSelected(CDPoint cPt, double dDist, PDRect pRect, PDPtrList pRegions)
{
    bool bRes = false;
    PDObject pObj;
    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            bRes |= pObj->Extend(cPt, dDist, pRect, pRegions);
        }
    }
    if(bRes) m_bHasChanged = true;
    return bRes;
}

void CDataList::ClearAll()
{
    for(int i = 0; i < m_iDataLen; i++)
    {
        delete m_ppObjects[i];
    }
    m_iDataLen = 0;
    m_bHasChanged = false;
}

void CDataList::SaveToFile(FILE *pf, bool bSwapBytes, bool bSelectOnly)
{
    unsigned char buf[16], *pbuf;
    buf[0] = 3;
    buf[1] = 8;
    buf[2] = 7;
    buf[3] = 0;
    buf[4] = 1;
    buf[5] = 6;

    // magic number
    fwrite(buf, 1, 6, pf);

    buf[0] = 1;

    // version
    fwrite(buf, 1, 1, pf);

    pbuf = (unsigned char*)&m_cFileAttrs.dWidth;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    pbuf = (unsigned char*)&m_cFileAttrs.dHeight;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    pbuf = (unsigned char*)&m_cFileAttrs.dScaleNom;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    pbuf = (unsigned char*)&m_cFileAttrs.dScaleDenom;
    SwapBytes(buf, pbuf, 8, bSwapBytes);
    fwrite(buf, 1, 8, pf);

    unsigned long lDataLen = m_iDataLen;
    if(bSelectOnly) lDataLen = GetSelectCount();
    pbuf = (unsigned char*)&lDataLen;
    SwapBytes(buf, pbuf, 4, bSwapBytes);
    fwrite(buf, 1, 4, pf);

    if(bSelectOnly)
    {
        for(int i = 0; i < m_iDataLen; i++)
        {
            if(m_ppObjects[i]->GetSelected())
                m_ppObjects[i]->SaveToFile(pf, bSwapBytes);
        }
    }
    else
    {
        for(int i = 0; i < m_iDataLen; i++)
        {
            m_ppObjects[i]->SaveToFile(pf, bSwapBytes);
        }
    }
    m_bHasChanged = bSelectOnly;
}

bool CDataList::ReadFromFile(FILE *pf, bool bSwapBytes, bool bClear)
{
    unsigned char buf[16], *pbuf;
    fread(buf, 1, 6, pf);

    bool bMagicOK = (buf[0] == 3) && (buf[1] == 8) && (buf[2] == 7) &&
        (buf[3] == 0) && (buf[4] == 1) && (buf[5] == 6);

    if(!bMagicOK) return false;

    // version
    fread(buf, 1, 1, pf);
    if(buf[0] > 1) return false; // we don't know that version yet

    if(bClear) ClearAll();

    fread(buf, 1, 8, pf);
    if(bClear)
    {
        pbuf = (unsigned char*)&m_cFileAttrs.dWidth;
        SwapBytes(pbuf, buf, 8, bSwapBytes);
    }

    fread(buf, 1, 8, pf);
    if(bClear)
    {
        pbuf = (unsigned char*)&m_cFileAttrs.dHeight;
        SwapBytes(pbuf, buf, 8, bSwapBytes);
    }

    fread(buf, 1, 8, pf);
    if(bClear)
    {
        pbuf = (unsigned char*)&m_cFileAttrs.dScaleNom;
        SwapBytes(pbuf, buf, 8, bSwapBytes);
    }

    fread(buf, 1, 8, pf);
    if(bClear)
    {
        pbuf = (unsigned char*)&m_cFileAttrs.dScaleDenom;
        SwapBytes(pbuf, buf, 8, bSwapBytes);
    }

    fread(buf, 1, 4, pf);
    unsigned long lDataLen = 0;
    pbuf = (unsigned char*)&lDataLen;
    SwapBytes(pbuf, buf, 4, bSwapBytes);

    PDObject pObj;

    for(unsigned int i = 0; i < lDataLen; i++)
    {
        fread(buf, 1, 1, pf);
        pObj = new CDObject((CDDrawType)buf[0], 0.2);
        pObj->ReadFromFile(pf, bSwapBytes);
        Add(pObj);
    }
    m_bHasChanged = !bClear;
    return true;
}

void CDataList::SelectByRectangle(PDRect pRect, int iMode, PDPtrList pRegions)
{
    double x;
    if(pRect->cPt1.x > pRect->cPt2.x)
    {
        x = pRect->cPt1.x;
        pRect->cPt1.x = pRect->cPt2.x;
        pRect->cPt2.x = x;
    }
    if(pRect->cPt1.y > pRect->cPt2.y)
    {
        x = pRect->cPt1.y;
        pRect->cPt1.y = pRect->cPt2.y;
        pRect->cPt2.y = x;
    }

    PDObject pObj;
    CDLine cLn;
    cLn.bIsSet = false;
    int k;
    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        k = pObj->BuildPrimitives(cLn, 0, pRect, 2, NULL);
        if(k == iMode) pObj->SetSelected(true, false, -1, pRegions);
    }

    PDPoint pPts = (PDPoint)malloc(4*sizeof(CDPoint));
    pPts[0].x = pRect->cPt1.x - 1.0;
    pPts[0].y = pRect->cPt1.y - 1.0;
    pPts[1].x = pRect->cPt2.x + 1.0;
    pPts[1].y = pRect->cPt1.y - 1.0;
    pPts[2].x = pRect->cPt2.x + 1.0;
    pPts[2].y = pRect->cPt2.y + 1.0;
    pPts[3].x = pRect->cPt1.x - 1.0;
    pPts[3].y = pRect->cPt2.y + 1.0;

    PDPolygon pPoly = (PDPolygon)malloc(sizeof(CDPolygon));
    pPoly->iPoints = 4;
    pPoly->pPoints = pPts;
    pRegions->Add(pPoly);
}

bool CDataList::RotateSelected(CDPoint cOrig, double dRot, int iCop, PDRect pRect, PDPtrList pRegions)
{
    bool bRes = false;
    double dRotStep = dRot;
    if(iCop > 1) dRotStep = dRot/iCop;
    if(fabs(fabs(dRot) - 2*M_PI) < g_dPrec)
    {
        if(iCop > 1) iCop--;
        else return false;
    }

    PDObject pObj, pObj1;
    CDLine cLn;
    cLn.bIsSet = false;
    int iCurLen = m_iDataLen;
    for(int i = 0; i < iCurLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            bRes = true;
            if(iCop < 1)
            {
                pObj->AddRegions(pRegions, -1);
                pObj->RotatePoints(cOrig, dRot, 2);
                pObj->BuildPrimitives(cLn, 0, pRect, 0, NULL);
                pObj->AddRegions(pRegions, -1);
            }
            else
            {
                for(int j = 0; j < iCop; j++)
                {
                    pObj1 = pObj->Copy();
                    pObj1->RotatePoints(cOrig, (j + 1)*dRotStep, 0);
                    pObj1->BuildPrimitives(cLn, 0, pRect, 0, NULL);
                    pObj1->AddRegions(pRegions, -1);
                    Add(pObj1);
                }
                if(iCop == 1)
                {
                    pObj->SetSelected(false, false, -1, pRegions);
                    pObj1->SetSelected(true, false, -1, pRegions);
                }
            }
        }
        else
        {
            if(pObj->RotatePoints(cOrig, dRot, 1))
            {
                pObj->AddRegions(pRegions, -1);
                bRes = true;
                pObj->BuildPrimitives(cLn, 0, pRect, 0, NULL);
                pObj->AddRegions(pRegions, -1);
            }
        }
    }
    if(bRes) m_bHasChanged = true;
    return bRes;
}

bool CDataList::MoveSelected(CDLine cLine, double dDist, int iCop, PDRect pRect,
    bool bPreserveDir, PDPtrList pRegions)
{
    bool bRes = false;
    double dDistStep = dDist;
    if(iCop > 1) dDistStep = dDist/iCop;

    CDPoint cDir = cLine.cDirection;
    CDLine cLn;
    cLn.bIsSet = false;
    if(!bPreserveDir)
    {
        double dAng = atan2(cDir.y, cDir.x);
        if((dAng > 0.8) || (dAng < -2.4)) cDir *= -1.0;
    }

    PDObject pObj, pObj1;
    int iCurLen = m_iDataLen;
    for(int i = 0; i < iCurLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            bRes = true;
            if(iCop < 1)
            {
                pObj->AddRegions(pRegions, -1);
                pObj->MovePoints(cDir, dDist, 2);
                pObj->BuildPrimitives(cLn, 0, pRect, 0, NULL);
                pObj->AddRegions(pRegions, -1);
            }
            else
            {
                for(int j = 0; j < iCop; j++)
                {
                    pObj1 = pObj->Copy();
                    pObj1->MovePoints(cDir, (j + 1)*dDistStep, 0);
                    pObj1->BuildPrimitives(cLn, 0, pRect, 0, NULL);
                    pObj1->AddRegions(pRegions, -1);
                    Add(pObj1);
                }
                if(iCop == 1)
                {
                    pObj->SetSelected(false, false, -1, pRegions);
                    pObj1->SetSelected(true, false, -1, pRegions);
                }
            }
        }
        else
        {
            if(pObj->MovePoints(cDir, dDist, 1))
            {
                pObj->AddRegions(pRegions, -1);
                bRes = true;
                pObj->BuildPrimitives(cLn, 0, pRect, 0, NULL);
                pObj->AddRegions(pRegions, -1);
            }
        }
    }
    if(bRes) m_bHasChanged = true;
    return bRes;
}

bool CDataList::MirrorSelected(CDLine cLine, PDRect pRect, PDPtrList pRegions)
{
    bool bRes = false;

    PDObject pObj, pObj1;
    CDLine cLn;
    cLn.bIsSet = false;
    int iCurLen = m_iDataLen;
    for(int i = 0; i < iCurLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            pObj->AddRegions(pRegions, -1);
            bRes = true;
            pObj1 = pObj->Copy();
            pObj1->MirrorPoints(cLine);
            pObj1->BuildPrimitives(cLn, 0, pRect, 0, NULL);
            pObj1->AddRegions(pRegions, -1);
            Add(pObj1);
        }
    }
    if(bRes) m_bHasChanged = true;
    return bRes;
}

bool LSPatterMatch(CDLineStyle cStyle1, CDLineStyle cStyle2)
{
    bool bRes = (cStyle1.iSegments == cStyle2.iSegments);
    int i = 0;
    int n = cStyle1.iSegments;
    if(n > 6) n = 6;
    while(bRes && (i < n))
    {
        bRes = (fabs(cStyle1.dPattern[i] - cStyle2.dPattern[i]) < g_dPrec);
        i++;
    }
    return bRes;
}

int CDataList::GetSelectedLineStyle(PDLineStyle pStyle)
{
    int iRes = -1;
    CDLineStyle cSt1, cSt2;
    PDObject pObj;
    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            if(iRes < 0)
            {
                cSt1 = pObj->GetLineStyle();
                iRes = 7;
            }
            else
            {
                cSt2 = pObj->GetLineStyle();
                if(fabs(cSt1.dWidth - cSt2.dWidth) > g_dPrec) iRes &= ~1;
                if(fabs(cSt1.dPercent - cSt2.dPercent) > g_dPrec) iRes &= ~2;
                if(!LSPatterMatch(cSt1, cSt2)) iRes &= ~4;
            }
        }
    }
    if(iRes > -1) *pStyle = cSt1;
    return iRes;
}

bool CDataList::SetSelectedLineStyle(int iMask, PDLineStyle pStyle, PDPtrList pRegions)
{
    bool bRes = false;
    PDObject pObj;
    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            bRes = true;
            pObj->SetLineStyle(iMask, *pStyle);
            pObj->AddRegions(pRegions, -1);
            m_bHasChanged = true;
        }
    }
    return bRes;
}

bool CDataList::SetCrossSelected(CDPoint cPt, double dDist, PDRect pRect, PDPtrList pRegions)
{
    bool bRes = false;
    PDObject pObj;
    CDLine cLn;
    cLn.bIsSet = false;
    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            if(pObj->AddCrossPoint(cPt, dDist, pRegions))
            {
                bRes = true;
                pObj->BuildPrimitives(cLn, 0, pRect, 0, NULL);
            }
        }
    }
    if(bRes) m_bHasChanged = true;
    return bRes;
}

bool CDataList::AddDimen(CDPoint cPt, double dDist, PDRect pRect, PDPtrList pRegions)
{
    bool bFound = false;
    int i = 0;
    PDObject pObj;
    while(!bFound && (i < m_iDataLen))
    {
        pObj = m_ppObjects[i++];
        bFound = pObj->GetSelected();
    }
    if(!bFound) return false;

    bool bRes = pObj->AddDimen(cPt, dDist, pRect, &m_cFileAttrs);
    if(bRes)
    {
        m_bHasChanged = true;
        pObj->AddRegions(pRegions, -1);
    }
    return bRes;
}

bool CDataList::GetChanged()
{
    return m_bHasChanged;
}

void CDataList::SetChanged()
{
    m_bHasChanged = true;
}

void CDataList::SetFileAttrs(PDFileAttrs pFileAttrs, bool bNewFile)
{
    m_cFileAttrs.dWidth = pFileAttrs->dWidth;
    m_cFileAttrs.dHeight = pFileAttrs->dHeight;
    m_cFileAttrs.dScaleNom = pFileAttrs->dScaleNom;
    m_cFileAttrs.dScaleDenom = pFileAttrs->dScaleDenom;
    m_cFileAttrs.iArrowType = pFileAttrs->iArrowType;
    m_cFileAttrs.cArrowDim = pFileAttrs->cArrowDim;
    m_cFileAttrs.dFontSize = pFileAttrs->dFontSize;
    m_cFileAttrs.dBaseLine = pFileAttrs->dBaseLine;
    m_cFileAttrs.bFontAttrs = pFileAttrs->bFontAttrs;
    strcpy(m_cFileAttrs.sFontFace, pFileAttrs->sFontFace);
    strcpy(m_cFileAttrs.sLengthMask, pFileAttrs->sLengthMask);
    strcpy(m_cFileAttrs.sAngleMask, pFileAttrs->sAngleMask);
    if(!bNewFile) m_bHasChanged = true;
}

void CDataList::GetFileAttrs(PDFileAttrs pFileAttrs)
{
    pFileAttrs->dWidth = m_cFileAttrs.dWidth;
    pFileAttrs->dHeight = m_cFileAttrs.dHeight;
    pFileAttrs->dScaleNom = m_cFileAttrs.dScaleNom;
    pFileAttrs->dScaleDenom = m_cFileAttrs.dScaleDenom;
    pFileAttrs->iArrowType = m_cFileAttrs.iArrowType;
    pFileAttrs->cArrowDim = m_cFileAttrs.cArrowDim;
    pFileAttrs->dFontSize = m_cFileAttrs.dFontSize;
    pFileAttrs->dBaseLine = m_cFileAttrs.dBaseLine;
    pFileAttrs->bFontAttrs = m_cFileAttrs.bFontAttrs;
    strcpy(pFileAttrs->sFontFace, m_cFileAttrs.sFontFace);
}

bool CDataList::GetSelectedDimen(PDDimension pDimen)
{
    bool bFound = false;
    int i = 0;
    PDObject pObj;
    while(!bFound && (i < m_iDataLen))
    {
        pObj = m_ppObjects[i++];
        bFound = pObj->GetSelectedDimen(pDimen);
    }
    return bFound;
}

bool CDataList::SetSelectedDimen(PDDimension pDimen, PDPtrList pRegions)
{
    bool bRes = false;
    PDObject pObj;
    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        bRes |= pObj->SetSelectedDimen(pDimen, pRegions);
    }
    if(bRes) m_bHasChanged = true;
    return bRes;
}

void CDataList::GetStatistics(int *piStats)
{
    PDObject pObj;
    int iType;
    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        // 1 line, 2 circle, 3 ellipse, 4 arc ellipse, 5 hyperbola, 6 parabola, 7 spline
        iType = pObj->GetType();
        piStats[iType] += 1;
        piStats[0] += pObj->GetDimenCount();
    }
}

int CDataList::GetUnitMask(int iUnitType, char *psBuf, PDUnitList pUnits)
{
    bool bFound = false;
    int i = 0;
    int iRes = -1;
    PDObject pObj;

    while(!bFound && (i < m_iDataLen))
    {
        pObj = m_ppObjects[i++];
        iRes = pObj->GetUnitMask(iUnitType, psBuf, pUnits);
        bFound = (iRes > -1);
    }
    return iRes;
}

void CDataList::ChangeUnitMask(int iUnitType, char *psMask, PDUnitList pUnits)
{
    PDObject pObj;
    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        m_bHasChanged |= pObj->ChangeUnitMask(iUnitType, psMask, pUnits);
    }
    return;
}

void CDataList::RescaleDrawing(double dNewScaleNom, double dNewScaleDenom, bool bWidths,
    bool bPatterns, bool bArrows, bool bLabels)
{
    double dScaleRatio = dNewScaleNom*m_cFileAttrs.dScaleDenom/(dNewScaleDenom*m_cFileAttrs.dScaleNom);
    if(fabs(dScaleRatio - 1.0) < g_dPrec) return;

    PDObject pObj;
    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        pObj->Rescale(dScaleRatio, bWidths, bPatterns, bArrows, bLabels);
    }

    m_cFileAttrs.dScaleNom = dNewScaleNom;
    m_cFileAttrs.dScaleDenom = dNewScaleDenom;
    m_bHasChanged = true;
    return;
}

bool CDataList::GetSelSnapEnabled()
{
    int iSelTot = 0;
    int iEnabled = 0;
    PDObject pObj;

    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            iSelTot++;
            if(pObj->GetSnapTo()) iEnabled++;
        }
    }

    return 2*iEnabled >= iSelTot;
}

void CDataList::SetSelSnapEnabled(bool bEnable)
{
    PDObject pObj;

    for(int i = 0; i < m_iDataLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected()) pObj->SetSnapTo(bEnable);
    }
}

bool CDataList::BreakSelObjects(PDRect pRect, PDPtrList pRegions)
{
    PDObject pObj, pNewObj;
    int iParts;
    int iLen = m_iDataLen;
    bool bRes = false;

    for(int i = 0; i < iLen; i++)
    {
        pObj = m_ppObjects[i];
        if(pObj->GetSelected())
        {
            iParts = pObj->GetNumParts();
            for(int j = 0; j < iParts; j++)
            {
                pNewObj = pObj->SplitPart(pRect, pRegions);
                if(pNewObj)
                {
                    Add(pNewObj);
                    bRes = true;
                }
            }
        }
    }
    return bRes;
}

