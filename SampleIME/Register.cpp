// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"

static const WCHAR RegInfo_Prefix_CLSID[] = L"CLSID\\";
static const WCHAR RegInfo_Key_InProSvr32[] = L"InProcServer32";
static const WCHAR RegInfo_Key_ThreadModel[] = L"ThreadingModel";

static const WCHAR TEXTSERVICE_DESC[] = L"Jyutping";

static const GUID SupportCategories[] = {
    GUID_TFCAT_TIP_KEYBOARD,
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
    GUID_TFCAT_TIPCAP_UIELEMENTENABLED,
    GUID_TFCAT_TIPCAP_SECUREMODE,
    GUID_TFCAT_TIPCAP_COMLESS,
    GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT,
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
};
//+---------------------------------------------------------------------------
//
//  RegisterProfiles
//
//----------------------------------------------------------------------------

BOOL RegisterProfiles()
{
    Microsoft::WRL::ComPtr<ITfInputProcessorProfileMgr> pITfInputProcessorProfileMgr;
    HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pITfInputProcessorProfileMgr));
    if (FAILED(hr))
    {
        return FALSE;
    }

    WCHAR achIconFile[MAX_PATH] = {'\0'};
    DWORD cchA = 0;
    cchA = GetModuleFileName(Global::dllInstanceHandle, achIconFile, MAX_PATH);
    cchA = cchA >= MAX_PATH ? (MAX_PATH - 1) : cchA;
    achIconFile[cchA] = '\0';

    size_t lenOfDesc = 0;
    hr = StringCchLength(TEXTSERVICE_DESC, STRSAFE_MAX_CCH, &lenOfDesc);
    if (hr != S_OK)
    {
        return FALSE;
    }

    hr = pITfInputProcessorProfileMgr->RegisterProfile(Global::SampleIMECLSID,
        TEXTSERVICE_LANGID,
        Global::SampleIMEGuidProfile,
        TEXTSERVICE_DESC,
        static_cast<ULONG>(lenOfDesc),
        achIconFile,
        cchA,
        (UINT)TEXTSERVICE_ICON_INDEX, NULL, 0, TRUE, 0);

    return SUCCEEDED(hr);
}

//+---------------------------------------------------------------------------
//
//  UnregisterProfiles
//
//----------------------------------------------------------------------------

void UnregisterProfiles()
{
    Microsoft::WRL::ComPtr<ITfInputProcessorProfileMgr> pITfInputProcessorProfileMgr;
    HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pITfInputProcessorProfileMgr));
    if (FAILED(hr))
    {
        return;
    }

    pITfInputProcessorProfileMgr->UnregisterProfile(Global::SampleIMECLSID, TEXTSERVICE_LANGID, Global::SampleIMEGuidProfile, 0);
}

//+---------------------------------------------------------------------------
//
//  RegisterCategories
//
//----------------------------------------------------------------------------

BOOL RegisterCategories()
{
    Microsoft::WRL::ComPtr<ITfCategoryMgr> pCategoryMgr;
    HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCategoryMgr));
    if (FAILED(hr))
    {
        return FALSE;
    }

    for (const auto& guid : SupportCategories)
    {
        hr = pCategoryMgr->RegisterCategory(Global::SampleIMECLSID, guid, Global::SampleIMECLSID);
    }

    return SUCCEEDED(hr);
}

//+---------------------------------------------------------------------------
//
//  UnregisterCategories
//
//----------------------------------------------------------------------------

void UnregisterCategories()
{
    Microsoft::WRL::ComPtr<ITfCategoryMgr> pCategoryMgr;
    HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCategoryMgr));
    if (FAILED(hr))
    {
        return;
    }

    for (const auto& guid : SupportCategories)
    {
        pCategoryMgr->UnregisterCategory(Global::SampleIMECLSID, guid, Global::SampleIMECLSID);
    }
}

//+---------------------------------------------------------------------------
//
// RecurseDeleteKey
//
// RecurseDeleteKey is necessary because on NT RegDeleteKey doesn't work if the
// specified key has subkeys
//----------------------------------------------------------------------------

LONG RecurseDeleteKey(_In_ HKEY hParentKey, _In_ LPCTSTR lpszKey)
{
    CRegKey regKey;
    LONG res = 0;
    FILETIME time;
    WCHAR stringBuffer[256] = {'\0'};
    DWORD size = ARRAYSIZE(stringBuffer);

    if (regKey.Open(hParentKey, lpszKey, KEY_READ | KEY_WRITE) != ERROR_SUCCESS)
    {
        return ERROR_SUCCESS;
    }

    res = ERROR_SUCCESS;
    while (RegEnumKeyEx(regKey.GetHKEY(), 0, stringBuffer, &size, NULL, NULL, NULL, &time) == ERROR_SUCCESS)
    {
        stringBuffer[ARRAYSIZE(stringBuffer)-1] = '\0';
        res = RecurseDeleteKey(regKey.GetHKEY(), stringBuffer);
        if (res != ERROR_SUCCESS)
        {
            break;
        }
        size = ARRAYSIZE(stringBuffer);
    }

    if (res == ERROR_SUCCESS)
    {
        regKey.Close();
        res = RegDeleteKey(hParentKey, lpszKey);
    }

    return res;
}

//+---------------------------------------------------------------------------
//
//  RegisterServer
//
//----------------------------------------------------------------------------

BOOL RegisterServer()
{
    CRegKey regKey;
    CRegKey regSubkey;
    WCHAR achIMEKey[ARRAYSIZE(RegInfo_Prefix_CLSID) + CLSID_STRLEN] = {'\0'};
    WCHAR achFileName[MAX_PATH] = {'\0'};

    if (!CLSIDToString(Global::SampleIMECLSID, achIMEKey + ARRAYSIZE(RegInfo_Prefix_CLSID) - 1))
    {
        return FALSE;
    }

    memcpy(achIMEKey, RegInfo_Prefix_CLSID, sizeof(RegInfo_Prefix_CLSID) - sizeof(WCHAR));

    if (regKey.Create(HKEY_CLASSES_ROOT, achIMEKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (regKey.SetStringValue(NULL, TEXTSERVICE_DESC) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (regSubkey.Create(regKey.GetHKEY(), RegInfo_Key_InProSvr32) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    DWORD copiedStringLen = GetModuleFileNameW(Global::dllInstanceHandle, achFileName, ARRAYSIZE(achFileName));
    if (copiedStringLen == 0 || copiedStringLen >= ARRAYSIZE(achFileName))
    {
        return FALSE;
    }

    if (regSubkey.SetStringValue(NULL, achFileName) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    if (regSubkey.SetStringValue(RegInfo_Key_ThreadModel, TEXTSERVICE_MODEL) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
//  UnregisterServer
//
//----------------------------------------------------------------------------

void UnregisterServer()
{
    WCHAR achIMEKey[ARRAYSIZE(RegInfo_Prefix_CLSID) + CLSID_STRLEN] = {'\0'};

    if (!CLSIDToString(Global::SampleIMECLSID, achIMEKey + ARRAYSIZE(RegInfo_Prefix_CLSID) - 1))
    {
        return;
    }

    memcpy(achIMEKey, RegInfo_Prefix_CLSID, sizeof(RegInfo_Prefix_CLSID) - sizeof(WCHAR));

    RecurseDeleteKey(HKEY_CLASSES_ROOT, achIMEKey);
}
