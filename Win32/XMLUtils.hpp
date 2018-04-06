#ifndef _XMLUTILS_HPP_
#define _XMLUTILS_HPP_

#include <msxml.h>

class CXMLReader
{
private:
    LPWSTR m_wsName;
    IXMLDOMDocument* m_pDoc;
    IXMLDOMElement* m_pRoot;
    BOOL m_bOpened;
public:
    CXMLReader(LPCWSTR wsFileName);
    ~CXMLReader();
    IXMLDOMElement* OpenSection(LPCWSTR wsName);
    IXMLDOMElement* OpenSubSection(IXMLDOMElement* pParent, LPCWSTR wsName);
    IXMLDOMElement* GetFirstNode(IXMLDOMElement* pParent, LPWSTR *pwsNodeName);
    IXMLDOMElement* GetNextNode(IXMLDOMElement* pPrevNode, LPWSTR *pwsNodeName);
    BOOL GetByteValue(IXMLDOMElement* pSec, LPCWSTR wsName, BYTE *pbValue);
    BOOL GetIntValue(IXMLDOMElement* pSec, LPCWSTR wsName, int *piValue);
    BOOL GetLongValue(IXMLDOMElement* pSec, LPCWSTR wsName, long *plValue);
    BOOL GetDoubleValue(IXMLDOMElement* pSec, LPCWSTR wsName, double *pdValue);
    BOOL GetStringValue(IXMLDOMElement* pSec, LPCWSTR wsName, LPWSTR *pwsValue);
    BOOL GetStringValueBuf(IXMLDOMElement* pSec, LPCWSTR wsName, LPWSTR wsValue, int iBufSize);
};

class CXMLWritter
{
private:
    LPWSTR m_wsName;
    IXMLDOMDocument* m_pDoc;
    IXMLDOMElement* m_pRoot;
    void InsertNode(IDispatch* pParent, IDispatch* pNode);
public:
    CXMLWritter(LPCWSTR wsFileName);
    ~CXMLWritter();
    void Save();
    void WriteComment(LPCWSTR wsValue);
    void CreateRoot(LPCWSTR wsName);
    IXMLDOMElement* CreateSection(LPCWSTR wsName);
    IXMLDOMElement* CreateSubSection(IXMLDOMElement* pParent, LPCWSTR wsName);
    void AddByteValue(IXMLDOMElement* pSec, LPCWSTR wsName, BYTE bValue);
    void AddIntValue(IXMLDOMElement* pSec, LPCWSTR wsName, int iValue);
    void AddLongValue(IXMLDOMElement* pSec, LPCWSTR wsName, long lValue);
    void AddDoubleValue(IXMLDOMElement* pSec, LPCWSTR wsName, double dValue);
    void AddStringValue(IXMLDOMElement* pSec, LPCWSTR wsName, LPCWSTR wsValue);
};

#endif
