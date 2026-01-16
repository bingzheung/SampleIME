// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "DictionarySearch.h"
#include "SampleIMEBaseStructure.h"

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CDictionarySearch::CDictionarySearch(LCID locale, _In_ CFile *pFile, _In_ CStringRange *pSearchKeyCode) : CDictionaryParser(locale)
{
    _pFile = pFile;
    _pSearchKeyCode = pSearchKeyCode;
    _charIndex = 0;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CDictionarySearch::~CDictionarySearch()
{
}

//+---------------------------------------------------------------------------
//
// FindPhrase
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindPhrase(_Out_ CDictionaryResult **ppdret)
{
    return FindWorker(FALSE, ppdret, FALSE); // NO WILDCARD
}

//+---------------------------------------------------------------------------
//
// FindPhraseForWildcard
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindPhraseForWildcard(_Out_ CDictionaryResult **ppdret)
{
    return FindWorker(FALSE, ppdret, TRUE); // Wildcard
}

//+---------------------------------------------------------------------------
//
// FindConvertedStringForWildcard
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindConvertedStringForWildcard(CDictionaryResult **ppdret)
{
    return FindWorker(TRUE, ppdret, TRUE); // Wildcard
}

//+---------------------------------------------------------------------------
//
// FindWorker
//
//----------------------------------------------------------------------------

BOOL CDictionarySearch::FindWorker(BOOL isTextSearch, _Out_ CDictionaryResult **ppdret, BOOL isWildcardSearch)
{
    DWORD_PTR dwTotalBufLen = GetBufferInWCharLength();        // in char
    if (dwTotalBufLen == 0)
    {
        return FALSE;
    }

    const WCHAR *pwch = GetBufferInWChar();
    DWORD_PTR indexTrace = 0;     // in char
    *ppdret = nullptr;

    while (dwTotalBufLen > 0)
    {
        DWORD_PTR bufLenOneLine = GetOneLine(&pwch[indexTrace], dwTotalBufLen);
        if (bufLenOneLine != 0)
        {
            CParserStringRange keyword;
            if (!ParseLine(&pwch[indexTrace], bufLenOneLine, &keyword))
            {
                return FALSE;    // error
            }

            BOOL match = FALSE;
            if (!isTextSearch)
            {
                if (!isWildcardSearch)
                {
                    match = (CStringRange::Compare(_locale, &keyword, _pSearchKeyCode) == CSTR_EQUAL);
                }
                else
                {
                    match = CStringRange::WildcardCompare(_locale, _pSearchKeyCode, &keyword);
                }
            }
            else
            {
                CSampleImeArray<CParserStringRange> convertedStrings;
                if (!ParseLine(&pwch[indexTrace], bufLenOneLine, &keyword, &convertedStrings))
                {
                    return FALSE;
                }
                if (convertedStrings.Count() == 1)
                {
                    CStringRange* pTempString = convertedStrings.GetAt(0);
                    if (!isWildcardSearch)
                    {
                        match = (CStringRange::Compare(_locale, pTempString, _pSearchKeyCode) == CSTR_EQUAL);
                    }
                    else
                    {
                        match = CStringRange::WildcardCompare(_locale, _pSearchKeyCode, pTempString);
                    }
                }
            }

            if (match)
            {
                *ppdret = new (std::nothrow) CDictionaryResult();
                if (!*ppdret)
                {
                    return FALSE;
                }

                CSampleImeArray<CParserStringRange> valueStrings;
                if (!ParseLine(&pwch[indexTrace], bufLenOneLine, &keyword, &valueStrings))
                {
                    delete *ppdret;
                    *ppdret = nullptr;
                    return FALSE;
                }

                (*ppdret)->_FindKeyCode = keyword;
                (*ppdret)->_SearchKeyCode = *_pSearchKeyCode;

                for (UINT i = 0; i < valueStrings.Count(); i++)
                {
                    CStringRange* findPhrase = (*ppdret)->_FindPhraseList.Append();
                    if (findPhrase)
                    {
                        *findPhrase = *valueStrings.GetAt(i);
                    }
                }

                _charIndex += (indexTrace + bufLenOneLine);
                return TRUE;
            }
        }

        // Seek to next line
        dwTotalBufLen -= bufLenOneLine;
        indexTrace += bufLenOneLine;

        while (dwTotalBufLen > 0 && (pwch[indexTrace] == L'\r' || pwch[indexTrace] == L'\n' || pwch[indexTrace] == L'\0'))
        {
            dwTotalBufLen--;
            indexTrace++;
        }
    }

    return FALSE;
}
