#ifndef _DEXPDXF_HPP_
#define _DEXPDXF_HPP_

#include "DDrawTypes.hpp"
#include "DParser.hpp"

#ifdef _WIN32_WINNT
void ExportDXFFile(FILE *pFile, PDataList pDrawData, PDUnitList pUnits);
#else
void ExportDXFFile(char *sFileName, PDataList pDrawData, PDUnitList pUnits);
#endif

#endif
