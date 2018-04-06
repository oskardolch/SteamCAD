#include "XMLUtils.hpp"
#include <wchar.h>
#include <winnt.h>

#define DIID_IXMLDOMElement IID_IXMLDOMElement
#define DIID_IXMLDOMNode IID_IXMLDOMNode
#define DIID_IXMLDOMDocument IID_IXMLDOMDocument

const CLSID CLASS_DOMDocument =
    {0x2933BF90,0x7B36,0x11D2,{0xB2,0x0E,0x00,0xC0,0x4F,0x98,0x3E,0x60}};

IXMLDOMDocument* CreateDOMDocument()
{
    IXMLDOMDocument *res = NULL;
    CoCreateInstance(CLASS_DOMDocument, NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, DIID_IXMLDOMDocument,
        (void**)&res);
    return(res);
}

BSTR WideCharToBSTR(LPCWSTR wStr)
{
    int slen = wcslen(wStr);
    BSTR res = SysAllocStringLen(NULL, slen);
    wcscpy(res, wStr);
    return(res);
}

LPWSTR BSTRToWideChar(BSTR bStr)
{
    int slen = wcslen(bStr);
    LPWSTR res = (LPWSTR)malloc((slen + 1)*sizeof(wchar_t));
    wcscpy(res, bStr);
    return(res);
}

CXMLReader::CXMLReader(LPCWSTR wsFileName)
{
    m_wsName = (LPWSTR)malloc((wcslen(wsFileName) + 1)*sizeof(wchar_t));
    wcscpy(m_wsName, wsFileName);
    m_pDoc = CreateDOMDocument();

    VARIANTARG V;
    VariantInit(&V);
    V.vt = VT_BSTR;
    V.bstrVal = WideCharToBSTR(wsFileName);

    VARIANT_BOOL ob;
    m_pDoc->load(V, &ob);
    VariantClear(&V);
    m_bOpened = ob;

    m_pRoot = NULL;
    m_pDoc->get_documentElement(&m_pRoot);
    return;
}

CXMLReader::~CXMLReader()
{
    if(m_pRoot) m_pRoot->Release();
    m_pDoc->Release();
    free(m_wsName);
    return;
}

IXMLDOMElement* CXMLReader::OpenSection(LPCWSTR wsName)
{
    BSTR WS = WideCharToBSTR(wsName);
    IXMLDOMNode* N = NULL;
    IXMLDOMElement* res = NULL;

    if(m_bOpened) m_pRoot->selectSingleNode(WS, &N);
    SysFreeString(WS);

    if(N)
    {
        N->QueryInterface(DIID_IXMLDOMElement, (void**)&res);
        N->Release();
    };
    return(res);
}

IXMLDOMElement* CXMLReader::OpenSubSection(IXMLDOMElement* pParent, LPCWSTR wsName)
{
    BSTR WS = WideCharToBSTR(wsName);
    IXMLDOMNode* N = NULL;
    IXMLDOMElement* res = NULL;

    if(m_bOpened) pParent->selectSingleNode(WS, &N);
    SysFreeString(WS);

    if(N)
    {
        N->QueryInterface(DIID_IXMLDOMElement, (void**)&res);
        N->Release();
    };
    return(res);
}

IXMLDOMElement* CXMLReader::GetFirstNode(IXMLDOMElement* pParent, LPWSTR *pwsNodeName)
{
    *pwsNodeName = NULL;
    IXMLDOMNode *N1 = NULL;
    IXMLDOMElement *res = NULL;
    BSTR wnodename = NULL;

    HRESULT hres = -1;
    if(pParent) hres = pParent->get_firstChild(&N1);
    else hres = m_pRoot->get_firstChild(&N1);

    if(N1 && (hres == S_OK))
    {
        hres = N1->QueryInterface(DIID_IXMLDOMElement, (void**)&res);
        if(hres != S_OK) res = NULL;
        else
        {
            N1->get_nodeName(&wnodename);
            *pwsNodeName = BSTRToWideChar(wnodename);
            SysFreeString(wnodename);
        }
        N1->Release();
    }
    return(res);
}

IXMLDOMElement* CXMLReader::GetNextNode(IXMLDOMElement* pPrevNode, LPWSTR *pwsNodeName)
{
    *pwsNodeName = NULL;
    IXMLDOMNode *N1 = NULL, *N2 = NULL;
    IXMLDOMElement *res = NULL;
    BSTR wnodename = NULL;

    HRESULT hres = pPrevNode->QueryInterface(DIID_IXMLDOMNode, (void**)&N1);
    if(N1 && (hres == S_OK))
    {
        hres = N1->get_nextSibling(&N2);
        if(N2 && (hres == S_OK))
        {
            hres = N2->QueryInterface(DIID_IXMLDOMElement, (void**)&res);
            if(hres != S_OK) res = NULL;
            else
            {
                N2->get_nodeName(&wnodename);
                *pwsNodeName = BSTRToWideChar(wnodename);
                SysFreeString(wnodename);
            }
            N2->Release();
        }
        N1->Release();
    }
    // Caution! We clear the previous node so that it is possible to reuse
    // the same Element variable in a "while" loop
    pPrevNode->Release();
    return(res);
}

BOOL CXMLReader::GetByteValue(IXMLDOMElement* pSec, LPCWSTR wsName, BYTE *pbValue)
{
    BSTR WS = WideCharToBSTR(wsName);
    VARIANT V;

    VariantInit(&V);
    pSec->getAttribute(WS, &V);
    SysFreeString(WS);

    BOOL res = V.vt == VT_BSTR;
    if(res)
    {
        BYTE i;
        VarUI1FromStr(V.bstrVal, 0, 0, &i);
        *pbValue = i;
    };
    VariantClear(&V);
    return(res);
}

BOOL CXMLReader::GetIntValue(IXMLDOMElement* pSec, LPCWSTR wsName, int *piValue)
{
    BSTR WS = WideCharToBSTR(wsName);
    VARIANT V;

    VariantInit(&V);
    pSec->getAttribute(WS, &V);
    SysFreeString(WS);

    BOOL res = V.vt == VT_BSTR;
    if(res)
    {
        short i;
        VarI2FromStr(V.bstrVal, 0, 0, &i);
        *piValue = i;
    };
    VariantClear(&V);
    return(res);
}

BOOL CXMLReader::GetLongValue(IXMLDOMElement* pSec, LPCWSTR wsName, long *plValue)
{
    BSTR WS = WideCharToBSTR(wsName);
    VARIANT V;

    VariantInit(&V);
    pSec->getAttribute(WS, &V);
    SysFreeString(WS);

    BOOL res = V.vt == VT_BSTR;
    if(res) VarI4FromStr(V.bstrVal, 0, 0, plValue);
    VariantClear(&V);
    return(res);
}

BOOL CXMLReader::GetDoubleValue(IXMLDOMElement* pSec, LPCWSTR wsName, double *pdValue)
{
    BSTR WS = WideCharToBSTR(wsName);
    VARIANT V;

    VariantInit(&V);
    pSec->getAttribute(WS, &V);
    SysFreeString(WS);
    BOOL res = V.vt == VT_BSTR;
    if(res) VarR8FromStr(V.bstrVal, 0x007f, LOCALE_NOUSEROVERRIDE, pdValue);
    VariantClear(&V);
    return(res);
}

BOOL CXMLReader::GetStringValue(IXMLDOMElement* pSec, LPCWSTR wsName, LPWSTR *pwsValue)
{
    *pwsValue = NULL;

    BSTR WS = WideCharToBSTR(wsName);
    VARIANT V;

    VariantInit(&V);
    pSec->getAttribute(WS, &V);
    SysFreeString(WS);
    BOOL res = V.vt == VT_BSTR;

    if(res) *pwsValue = BSTRToWideChar(V.bstrVal);
    VariantClear(&V);
    return(res);
}

BOOL CXMLReader::GetStringValueBuf(IXMLDOMElement* pSec, LPCWSTR wsName,
    LPWSTR wsValue, int iBufSize)
{
    wsValue[0] = 0;

    BSTR WS = WideCharToBSTR(wsName);
    VARIANT V;

    VariantInit(&V);
    pSec->getAttribute(WS, &V);
    SysFreeString(WS);
    BOOL res = V.vt == VT_BSTR;
    if(res)
    {
        int slen = wcslen(V.bstrVal);
        if(iBufSize <= slen) slen = iBufSize - 1;
        wcsncpy(wsValue, V.bstrVal, iBufSize);
        wsValue[slen] = 0;
    }
    VariantClear(&V);
    return(res);
}


CXMLWritter::CXMLWritter(LPCWSTR wsFileName)
{
    m_wsName = (LPWSTR)malloc((wcslen(wsFileName) + 1)*sizeof(wchar_t));
    wcscpy(m_wsName, wsFileName);
    BSTR WS = SysAllocString(L"xml");
    //BSTR WName = SysAllocString(L"version=\"1.0\" encoding=\"windows-1250\"");
    BSTR WName = SysAllocString(L"version=\"1.0\"");

    m_pDoc = CreateDOMDocument();

    IXMLDOMProcessingInstruction* PI = NULL;
    m_pDoc->createProcessingInstruction(WS, WName, &PI);
    SysFreeString(WName);
    SysFreeString(WS);
    InsertNode(m_pDoc, PI);
    PI->Release();

    m_pRoot = NULL;
    return;
}

CXMLWritter::~CXMLWritter()
{
    if(m_pRoot) m_pRoot->Release();
    m_pDoc->Release();
    free(m_wsName);
    return;
}

void CXMLWritter::InsertNode(IDispatch* pParent, IDispatch* pNode)
{
    IXMLDOMNode* ParN = NULL;
    IXMLDOMNode* ChildN = NULL;
    IXMLDOMNode* DummyN = NULL;

    pParent->QueryInterface(DIID_IXMLDOMNode, (void**)&ParN);
    pNode->QueryInterface(DIID_IXMLDOMNode, (void**)&ChildN);
    ParN->appendChild(ChildN, &DummyN);
    if(DummyN) DummyN->Release();
    ChildN->Release();
    ParN->Release();
    return;
}

void CXMLWritter::Save()
{
    VARIANT V;
    V.vt = VT_BSTR;
    V.bstrVal = WideCharToBSTR(m_wsName);
    m_pDoc->save(V);
    VariantClear(&V);
    return;
}

void CXMLWritter::WriteComment(LPCWSTR wsValue)
{
    BSTR WS = WideCharToBSTR(wsValue);
    IXMLDOMComment* Comment = NULL;
    m_pDoc->createComment(WS, &Comment);
    SysFreeString(WS);
    InsertNode(m_pDoc, Comment);
    Comment->Release();
    return;
}

void CXMLWritter::CreateRoot(LPCWSTR wsName)
{
    BSTR WS = WideCharToBSTR(wsName);
    m_pDoc->createElement(WS, &m_pRoot);
    SysFreeString(WS);
    InsertNode(m_pDoc, m_pRoot);
    return;
}

IXMLDOMElement* CXMLWritter::CreateSection(LPCWSTR wsName)
{
    BSTR WS = NULL;
    IXMLDOMText* TN = NULL;
    VARIANT_BOOL hch;

    m_pRoot->hasChildNodes(&hch);
    if(!hch)
    {
        WS = SysAllocString(L"\r\n");
        m_pDoc->createTextNode(WS, &TN);
        SysFreeString(WS);
        InsertNode(m_pRoot, TN);
        TN->Release();
    }

    WS = SysAllocString(L"\t");
    m_pDoc->createTextNode(WS, &TN);
    SysFreeString(WS);
    InsertNode(m_pRoot, TN);
    TN->Release();
    TN = NULL;

    IXMLDOMElement* res = NULL;
    WS = WideCharToBSTR(wsName);
    m_pDoc->createElement(WS, &res);
    SysFreeString(WS);
    InsertNode(m_pRoot, res);

    WS = SysAllocString(L"\r\n");
    m_pDoc->createTextNode(WS, &TN);
    SysFreeString(WS);
    InsertNode(m_pRoot, TN);
    TN->Release();
    return(res);
}

IXMLDOMElement* CXMLWritter::CreateSubSection(IXMLDOMElement* pParent, LPCWSTR wsName)
{
    BSTR WS = NULL;
    IXMLDOMText* TN = NULL;
    IXMLDOMNode *N1, *N2;
    IXMLDOMNode *NPar = NULL;
    VARIANT_BOOL hch;

    pParent->hasChildNodes(&hch);
    if(!hch)
    {
        WS = SysAllocString(L"\r\n");
        m_pDoc->createTextNode(WS, &TN);
        SysFreeString(WS);
        InsertNode(pParent, TN);
        TN->Release();
        TN = NULL;
        N1 = NULL;
        pParent->get_parentNode(&NPar);
        if(NPar)
        {
            NPar->get_parentNode(&N1);
        }
        WS = SysAllocString(L"\t");
        while(N1)
        {
            m_pDoc->createTextNode(WS, &TN);
            InsertNode(pParent, TN);
            TN->Release();
            TN = NULL;
            N2 = N1;
            N1 = NULL;
            N2->get_parentNode(&N1);
            N2->Release();
        }
        SysFreeString(WS);
    }
    WS = SysAllocString(L"\t");
    m_pDoc->createTextNode(WS, &TN);
    SysFreeString(WS);
    InsertNode(pParent, TN);
    TN->Release();
    TN = NULL;

    IXMLDOMElement* res = NULL;
    WS = WideCharToBSTR(wsName);
    m_pDoc->createElement(WS, &res);
    SysFreeString(WS);
    InsertNode(pParent, res);

    WS = SysAllocString(L"\r\n");
    m_pDoc->createTextNode(WS, &TN);
    SysFreeString(WS);
    InsertNode(pParent, TN);
    TN->Release();
    TN = NULL;

    N1 = NULL;
    pParent->get_parentNode(&NPar);
    if(NPar)
    {
        NPar->get_parentNode(&N1);
    }

    WS = SysAllocString(L"\t");
    while(N1)
    {
        m_pDoc->createTextNode(WS, &TN);
        InsertNode(pParent, TN);
        TN->Release();
        TN = NULL;
        N2 = N1;
        N1 = NULL;
        N2->get_parentNode(&N1);
        N2->Release();
    }
    SysFreeString(WS);

    return(res);
}

void CXMLWritter::AddByteValue(IXMLDOMElement* pSec, LPCWSTR wsName, BYTE bValue)
{
    BSTR WS = WideCharToBSTR(wsName);
    VARIANT V;
    IXMLDOMAttribute* Attr = NULL;

    m_pDoc->createAttribute(WS, &Attr);
    SysFreeString(WS);
    V.vt = VT_UI1;
    V.bVal = bValue;
	Attr->put_value(V);
    pSec->setAttributeNode(Attr, NULL);
    Attr->Release();
    return;
}

void CXMLWritter::AddIntValue(IXMLDOMElement* pSec, LPCWSTR wsName, int iValue)
{
    BSTR WS = WideCharToBSTR(wsName);
    VARIANT V;
    IXMLDOMAttribute* Attr = NULL;

    m_pDoc->createAttribute(WS, &Attr);
    SysFreeString(WS);
    V.vt = VT_I2;
    V.iVal = iValue;
	Attr->put_value(V);
    pSec->setAttributeNode(Attr, NULL);
    Attr->Release();
    return;
}

void CXMLWritter::AddLongValue(IXMLDOMElement* pSec, LPCWSTR wsName, long lValue)
{
    BSTR WS = WideCharToBSTR(wsName);
    VARIANT V;
    IXMLDOMAttribute* Attr = NULL;

    m_pDoc->createAttribute(WS, &Attr);
    SysFreeString(WS);
    V.vt = VT_I4;
    V.lVal = lValue;
	Attr->put_value(V);
    pSec->setAttributeNode(Attr, NULL);
    Attr->Release();
    return;
}

void CXMLWritter::AddDoubleValue(IXMLDOMElement* pSec, LPCWSTR wsName, double dValue)
{
    BSTR WS = WideCharToBSTR(wsName);
    VARIANT V;
    IXMLDOMAttribute* Attr = NULL;

    m_pDoc->createAttribute(WS, &Attr);
    SysFreeString(WS);
    //V.vt = VT_R8;
    //V.dblVal = Value;
    V.vt = VT_BSTR;
    V.bstrVal = SysAllocStringLen(NULL, 64);
    swprintf(V.bstrVal, L"%2.4f", dValue);
	Attr->put_value(V);
    pSec->setAttributeNode(Attr, NULL);
    Attr->Release();
    return;
}

void CXMLWritter::AddStringValue(IXMLDOMElement* pSec, LPCWSTR wsName, LPCWSTR wsValue)
{
    BSTR WS = WideCharToBSTR(wsName);
    IXMLDOMAttribute* Attr = NULL;

    m_pDoc->createAttribute(WS, &Attr);
    SysFreeString(WS);
    WS = WideCharToBSTR(wsValue);
	Attr->put_text(WS);
    pSec->setAttributeNode(Attr, NULL);
    Attr->Release();
    SysFreeString(WS);
    return;
}
