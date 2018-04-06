#include "DParser.hpp"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include "DMath.hpp"
#include "DDataTypes.hpp"

CDUnitList::CDUnitList()
{
    m_iDataLen = 0;
    m_iDataSize = 16;
    m_pData = (PDUnit)malloc(m_iDataSize*sizeof(CDUnit));
}

CDUnitList::~CDUnitList()
{
    free(m_pData);
}

int CDUnitList::GetCount(int iType)
{
    if(iType < 0) return m_iDataLen;
    int iRes = 0;
    for(int i = 0; i < m_iDataLen; i++)
    {
        if(m_pData[i].iUnitType == iType) iRes++;
    }
    return iRes;
}

void CDUnitList::AddUnit(CDUnit cUnit)
{
    if(m_iDataLen >= m_iDataSize)
    {
        m_iDataSize += 16;
        m_pData = (PDUnit)realloc(m_pData, m_iDataSize*sizeof(CDUnit));
    }
    memcpy(&m_pData[m_iDataLen++], &cUnit, sizeof(CDUnit));
}

int CDUnitList::FindPos(int iType, int iPos)
{
    if(iType < 0) return iPos;
    int iRes = -1;
    int i = 0;
    while((i < m_iDataLen) && (iRes < iPos))
    {
        if(m_pData[i++].iUnitType == iType) iRes++;
    }
    if(iRes < iPos) return -1;
    return i - 1;
}

PDUnit CDUnitList::GetUnit(int iType, int iPos)
{
    int i = FindPos(iType, iPos);
    if(i < 0) return NULL;
    return &m_pData[i];
}

PDUnit CDUnitList::FindUnit(const char *psAbbrev)
{
    int i = 0;
    bool bFound = false;
    PDUnit pUnit;
    while(!bFound && (i < m_iDataLen))
    {
        pUnit = &m_pData[i++];
        bFound = ((strcmp(pUnit->sAbbrev, psAbbrev) == 0) ||
           (strcmp(pUnit->sAbbrev2, psAbbrev) == 0));
    }
    return bFound ? pUnit : NULL;
}


bool IsOperChar(char *psNumber)
{
    return (*psNumber == '+') || (*psNumber == '-') || (*psNumber == '*') ||
        (*psNumber == '/');
}

bool IsPI(char *psNumber)
{
    return (*psNumber == 'p') && (*(psNumber + 1) == 'i');
}

bool IsNumChar(char *psNumber)
{
    bool bRes = ((*psNumber >= '0') && (*psNumber <= '9')) ||
        (*psNumber == '.') || (*psNumber == ',') || IsOperChar(psNumber) ||
        (IsPI(psNumber)) || (*psNumber == ' ');
    return bRes;
}

char* GetNextUnit(char *psNumber)
{
    char *pBuf = psNumber;
    while((*pBuf != 0) && IsNumChar(pBuf))
    {
        if(IsPI(pBuf)) pBuf++;
        pBuf++;
    }
    return pBuf;
}

char* GetNextNumber(char *psNumber, PDUnitList pUnits, PDUnit *ppUnit)
{
    *ppUnit = NULL;
    char *pBuf = psNumber;
    while((*pBuf != 0) && !IsNumChar(pBuf)) pBuf++;

    char sUnitAbbrev[8];
    int iLen = pBuf - psNumber;
    if(iLen > 7) iLen = 7;
    strncpy(sUnitAbbrev, psNumber, iLen);
    sUnitAbbrev[iLen] = 0;

    *ppUnit = pUnits->FindUnit(sUnitAbbrev);
    return pBuf;
}

char* GetNextOper(char *psNumber)
{
    char *pBuf = psNumber;
    while((*pBuf != 0) && !IsOperChar(pBuf)) pBuf++;
    return pBuf;
}

struct CNumOper
{
    char cOper;
    double dNumber;
};

double ParseNumber(char *psNumber, PDUnit pUnit, bool *pbHasPi, bool *pbIsValid)
{
    double dRes = 0.0;
    *pbHasPi = false;
    *pbIsValid = true;

    CNumOper cNumOpers[32];
    int iNumOpers = 0;

    char *pBuf1 = psNumber;
    char *pBuf2 = GetNextOper(pBuf1);
    int i, j, iLen;
    char buf[64];
    bool bPi;
    char cOper = 0;
    float f;

    while(pBuf2 && (iNumOpers < 32))
    {
        iLen = pBuf2 - pBuf1;
        i = 0;
        j = 0;
        bPi = false;
        while((i < iLen) && (j < 63) && !bPi)
        {
            if((pBuf1[i] == 'p') && (pBuf1[i + 1] == 'i')) bPi = true;
            else if(pBuf1[i] == ',') buf[j++] = '.';
            else if(pBuf1[i] != ' ') buf[j++] = pBuf1[i];
            i++;
        }
        buf[j] = 0;

        if(sscanf(buf, "%f", &f) == 1)
        {
            if(!cOper) cOper = '+';
            cNumOpers[iNumOpers].cOper = cOper;
            cNumOpers[iNumOpers++].dNumber = f;

            if(bPi)
            {
                cNumOpers[iNumOpers].cOper = '*';
                cNumOpers[iNumOpers++].dNumber = M_PI;
                *pbHasPi = true;
            }
        }
        else if(bPi)
        {
            if(!cOper)
            {
                if(iNumOpers < 1) cOper = '+';
                else cOper = '*';
            }
            cNumOpers[iNumOpers].cOper = cOper;
            cNumOpers[iNumOpers++].dNumber = M_PI;
            *pbHasPi = true;
        }

        cOper = *pBuf2;
        if(*pBuf2)
        {
            pBuf1 = pBuf2 + 1;
            pBuf2 = GetNextOper(pBuf1);
        }
        else pBuf2 = NULL;
    }

    if(iNumOpers < 1)
    {
        *pbIsValid = false;
        return 0.0;
    }

    if((cNumOpers[0].cOper == '*') || (cNumOpers[0].cOper == '/'))
    {
        *pbIsValid = false;
        return 0.0;
    }

    double dSums[32];
    int iSums = 0;
    cOper = cNumOpers[0].cOper;
    if(cOper == '-') dSums[0] = -cNumOpers[0].dNumber;
    else dSums[0] = cNumOpers[0].dNumber;

    i = 1;
    while(*pbIsValid && (i < iNumOpers))
    {
        cOper = cNumOpers[i].cOper;
        if(cOper == '*')
        {
            dSums[iSums] *= cNumOpers[i].dNumber;
        }
        else if(cOper == '/')
        {
            if(fabs(cNumOpers[i].dNumber) > g_dPrec) dSums[iSums] /= cNumOpers[i].dNumber;
            else *pbIsValid = false;
        }
        else
        {
            iSums++;
            if(cOper == '-') dSums[iSums] = -cNumOpers[i].dNumber;
            else dSums[iSums] = cNumOpers[i].dNumber;
        }
        i++;
    }

    if(!(*pbIsValid)) return 0.0;

    for(i = 0; i <= iSums; i++)
    {
        dRes += dSums[i];
    }

    if(pUnit)
    {
        if(pUnit->iUnitType == 1) dRes *= pUnit->dBaseToUnit;
        else if(pUnit->iUnitType == 2) dRes /= pUnit->dBaseToUnit;
    }
    return dRes;
}

int ParseInputString(char *psNumber, PDUnitList pUnits, double *pdResult)
{
    if(!(*psNumber)) return -1;

    int iRes = 0;
    double dRes = 0.0;

    char sBuf[64];
    PDUnit pUnit;
    char *pBuf1 = psNumber;
    char *pBuf2 = GetNextUnit(pBuf1);
    int iLen;
    bool bIsValid, bHasPi;

    while(*pBuf2 && (iRes > -1))
    {
        iLen = pBuf2 - pBuf1;
        if(iLen > 63) iLen = 63;
        strncpy(sBuf, pBuf1, iLen);
        sBuf[iLen] = 0;

        pBuf1 = pBuf2;
        pBuf2 = GetNextNumber(pBuf1, pUnits, &pUnit);
        if(pUnit)
        {
            if((iRes > 0) && (iRes < 3))
            {
               if(pUnit->iUnitType != iRes) iRes = -1;
            }
            else iRes = pUnit->iUnitType;
        }

        dRes += ParseNumber(sBuf, pUnit, &bHasPi, &bIsValid);
        if(bIsValid)
        {
            if(bHasPi && (iRes == 0)) iRes = 3;
            pBuf1 = pBuf2;
            pBuf2 = GetNextUnit(pBuf1);
        }
        else iRes = -1;
    }

    if(*pBuf1 && !(*pBuf2))
    {
        if((iRes == 0) || (iRes == 3))
        {
            iLen = pBuf2 - pBuf1;
            if(iLen > 63) iLen = 63;
            strncpy(sBuf, pBuf1, iLen);
            sBuf[iLen] = 0;
            dRes += ParseNumber(sBuf, NULL, &bHasPi, &bIsValid);
            if(bIsValid)
            {
                if(bHasPi) iRes = 3;
            }
            else iRes = -1;
        }
        else iRes = -1;
    }

    *pdResult = dRes;
    return iRes;
}

const char* GetEscapeOpening(const char *psBuf)
{
    bool bFound = false;
    while(!bFound)
    {
        if(*psBuf == '[')
        {
            if(*(psBuf + 1) != '[') bFound = true;
            else psBuf++;
        }
        else if(*psBuf == '{')
        {
            if(*(psBuf + 1) != '{') bFound = true;
            else psBuf++;
        }
        else bFound = (*psBuf == 0);
        if(!bFound) psBuf++;
    }
    if(!bFound) return NULL;
    if(*psBuf == 0) return NULL;
    return psBuf;
}

const char* GetEscapeClosing(const char *psBuf, char cType)
{
    char cCloseChar = '}';
    if(cType == '[') cCloseChar = ']';

    bool bFound = false;
    while(!bFound)
    {
        if(*psBuf == cCloseChar)
        {
            if(*(psBuf + 1) != cCloseChar) bFound = true;
            else psBuf++;
        }
        else bFound = (*psBuf == 0);
        if(!bFound) psBuf++;
    }
    if(!bFound) return NULL;
    if(*psBuf == 0) return NULL;
    return psBuf;
}

int GetPlainMaskLen(const char *psMask)
{
    int iLen = 0;
    while(*psMask != 0)
    {
        if((*psMask == '[') || (*psMask == ']') || (*psMask == '{') || (*psMask == '}'))
            psMask++;
        if(*psMask != 0)
        {
            psMask++;
            iLen++;
        }
    }
    return iLen;
}

void CopyPlainMask(char *sDest, const char *sSrc)
{
    while(*sSrc != 0)
    {
        if((*sSrc == '[') || (*sSrc == ']') || (*sSrc == '{') || (*sSrc == '}'))
            sSrc++;
        if(*sSrc != 0)
        {
            *sDest = *sSrc;
            sSrc++;
            sDest++;
        }
    }
    *sDest = 0;
}

bool IsUnitTerm(const char *sBuf)
{
    return (*sBuf == ':') || (*sBuf == ']') || (*sBuf == '}') || (*sBuf == ' ');
}

PDUnit GetUnitAtBuf(const char *sBuf, PDUnitList pUnits)
{
    PDUnit pRes = NULL;
    while(*sBuf == ' ') sBuf++;
    if(((*sBuf == 'd') || (*sBuf == 'D')) && IsUnitTerm(sBuf + 1))
        pRes = pUnits->FindUnit("mm");
    else if(((*sBuf == 'r') || (*sBuf == 'R')) && IsUnitTerm(sBuf + 1))
        pRes = pUnits->FindUnit("deg");
    else
    {
        char buf[8];
        int i = 0;
        while(!IsUnitTerm(sBuf) && (i < 7))
        {
            buf[i++] = *sBuf;
            sBuf++;
        }
        buf[i] = 0;
        pRes = pUnits->FindUnit(buf);
    }
    return pRes;
}

bool ValidateMask(const char *psMask, PDUnitList pUnits)
{
    const char *s1 = psMask;
    const char *s2 = GetEscapeOpening(s1);
    const char *s3 = NULL;
    if(s2) s3 = GetEscapeClosing(s2 + 1, *s2);

    bool bEnd = !(s2 || s3);
    if(bEnd) return true;

    bool bValid = s2 && s3;
    if(!bValid) return false;

    char cFirstEscape = *s2;
    PDUnit pUnit = GetUnitAtBuf(s2 + 1, pUnits);
    if(!pUnit) return false;

    int iFirstType = pUnit->iUnitType;

    while(bValid && !bEnd)
    {
        s1 = s3 + 1;
        s2 = GetEscapeOpening(s1);
        if(s2) s3 = GetEscapeClosing(s2 + 1, *s2);
        else s3 = NULL;

        if(!(s2 || s3)) bEnd = true;
        else if(!(s2 && s3)) bValid = false;
        else if(*s2 != cFirstEscape) bValid = false;
        else
        {
            pUnit = GetUnitAtBuf(s2 + 1, pUnits);
            if(!pUnit) bValid = false;
            else if(pUnit->iUnitType != iFirstType) bValid = false;
        }
    }

    return bValid;
}

int ParseNumBuf(char *psMask, char *psBuf, double dVal, PDUnit pUnit, double *pdRest)
{
    psBuf[0] = 0;
    char *sCol = strchr(psMask, ':');

    if(sCol)
    {
        char sNumBuf[8];
        char *pBuf1 = sCol + 1;
        int iLen, i;

        double dVal1 = dVal;

        bool bFrac = (*pBuf1 == 'f') || (*pBuf1 == 'F');

        if(bFrac)
        {
            if(pUnit->iUnitType == 1) dVal /= pUnit->dBaseToUnit;
            else dVal *= pUnit->dBaseToUnit;

            int b, c;
            c = Round(dVal);
            dVal1 = dVal - c;

            if(fabs(dVal1) < 0.0001)
            {
                sprintf(psBuf, "%d", c);
            }
            else
            {
                bool bFound = false;
                i = 0;
                double a, d;
                c = (int)dVal;
                d = dVal - c;
                while(!bFound && (i < 64))
                {
                    i++;
                    a = d*i;
                    b = Round(a);
                    bFound = fabs(a - b) < 0.0001;
                }
                if(b > 0) sprintf(psBuf, "%d_%d/%d", c, b, i);
                else sprintf(psBuf, "%d", c);
            }
            *pdRest = 0;
        }
        else
        {
            iLen = strlen(pBuf1);
            if(iLen > 7) iLen = 7;

            i = 0;
            while((*pBuf1 >= '0') && (*pBuf1 <= '9') && (i < iLen))
            {
                sNumBuf[i++] = *pBuf1;
                pBuf1++;
            }

            sNumBuf[i] = 0;
            int iPrec = 0;
            if(sscanf(sNumBuf, "%d", &iPrec) == 1)
            {
                if(iPrec < 0) iPrec = 0;
            }

            if(iPrec > 0)
            {
                double dPrec = 0.1;
                for(int i = 1; i < iPrec; i++) dPrec /= 10.0;

                if(pUnit->iUnitType == 1) dVal /= pUnit->dBaseToUnit;
                else dVal *= pUnit->dBaseToUnit;

                dVal1 = dVal - Round(dVal);
                if(fabs(dVal1) < dPrec)
                {
                    dVal = Round(dVal);
                    iPrec = 0;
                }
                char sMask[16];
                sprintf(sMask, "%%.%df", iPrec);
                sprintf(psBuf, sMask, dVal);
                *pdRest = 0.0;
            }
            else
            {
                int iNum;
                double dVal2;
                *pdRest = 0.0;
                if(pUnit->iUnitType == 1)
                {
                    dVal2 = dVal/pUnit->dBaseToUnit;
                    iNum = Round(dVal2);
                    dVal1 = dVal2 - iNum;
                    if(fabs(dVal1) > 0.0001)
                    {
                        iNum = (int)dVal2;
                        *pdRest = dVal - iNum*pUnit->dBaseToUnit;
                    }
                }
                else
                {
                    dVal2 = dVal*pUnit->dBaseToUnit;
                    iNum = Round(dVal2);
                    dVal1 = dVal2 - iNum;
                    if(fabs(dVal1) > 0.0001)
                    {
                        iNum = (int)dVal2;
                        *pdRest = (dVal2 - iNum)/pUnit->dBaseToUnit;
                    }
                }
                sprintf(psBuf, "%d", iNum);
            }
        }
    }
    else
    {
        if(pUnit->iUnitType == 1) dVal /= pUnit->dBaseToUnit;
        else dVal *= pUnit->dBaseToUnit;
        sprintf(psBuf, "%f", dVal);
        *pdRest = 0.0;
    }
    return strlen(psBuf);
}

int PreParseValue(char *psMask, PDUnitList pUnits, double dVal, double dScale,
    char *psBuf, int iBufSize)
{
    const char *s1 = psMask;
    const char *s2 = GetEscapeOpening(s1);
    const char *s3 = NULL;
    if(s2) s3 = GetEscapeClosing(s2 + 1, *s2);

    bool bValid = s2 && s3;
    if(!bValid) return -1;

    char buf[64], sNumBuf[64];
    int iLen;
    int iMinLen = 0;
    PDUnit pUnit;
    double dValRest = dVal;

    bool bEnd = false;
    while(bValid && !bEnd)
    {
        iLen = s2 - s1;
        if(iLen > 63) iLen = 63;
        strncpy(buf, s1, iLen);
        buf[iLen] = 0;
        iMinLen += GetPlainMaskLen(buf);

        pUnit = GetUnitAtBuf(s2 + 1, pUnits);
        if(!pUnit) bValid = false;
        else
        {
            iLen = s3 - s2 - 1;
            if(iLen > 63) iLen = 63;
            strncpy(buf, s2 + 1, iLen);
            buf[iLen] = 0;
            iMinLen += ParseNumBuf(buf, sNumBuf, dValRest, pUnit, &dValRest);

            s1 = s3 + 1;
            s2 = GetEscapeOpening(s1);
            if(s2) s3 = GetEscapeClosing(s2 + 1, *s2);
            else s3 = NULL;

            if(!(s2 || s3)) bEnd = true;
            else if(!(s2 && s3)) bValid = false;
        }
    }

    if(!bValid) return -1;

    iMinLen += GetPlainMaskLen(s1);
    if(iBufSize <= iMinLen) return iMinLen;

    s1 = psMask;
    s2 = GetEscapeOpening(s1);
    s3 = NULL;
    if(s2) s3 = GetEscapeClosing(s2 + 1, *s2);

    dValRest = dVal;

    bEnd = false;
    iMinLen = 0;

    while(!bEnd)
    {
        iLen = s2 - s1;
        if(iLen > 63) iLen = 63;
        strncpy(buf, s1, iLen);
        buf[iLen] = 0;
        CopyPlainMask(&psBuf[iMinLen], buf);

        pUnit = GetUnitAtBuf(s2 + 1, pUnits);
        iLen = s3 - s2 - 1;
        if(iLen > 63) iLen = 63;
        strncpy(buf, s2 + 1, iLen);
        buf[iLen] = 0;
        ParseNumBuf(buf, sNumBuf, dValRest, pUnit, &dValRest);
        strcat(psBuf, sNumBuf);
        iMinLen = strlen(psBuf);

        s1 = s3 + 1;
        s2 = GetEscapeOpening(s1);
        if(s2) s3 = GetEscapeClosing(s2 + 1, *s2);
        else s3 = NULL;

        if(!(s2 || s3)) bEnd = true;
    }

    CopyPlainMask(&psBuf[iMinLen], s1);
    return 0;
}

int GuessMaskUnitType(char *psMask, PDUnitList pUnits)
{
    const char *s1 = psMask;
    const char *s2 = GetEscapeOpening(s1);
    const char *s3 = NULL;
    if(s2) s3 = GetEscapeClosing(s2 + 1, *s2);

    bool bEnd = !(s2 || s3);
    if(bEnd) return 0;

    bool bValid = s2 && s3;
    if(!bValid) return 0;

    PDUnit pUnit = GetUnitAtBuf(s2 + 1, pUnits);
    if(!pUnit) return 0;

    return pUnit->iUnitType;
}
