// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "SampleIME.h"
#include "CompositionProcessorEngine.h"

BOOL CSampleIME::VerifySampleIMECLSID(_In_ REFCLSID clsid)
{
    if (IsEqualCLSID(clsid, Global::SampleIMECLSID))
    {
        return TRUE;
    }
    return FALSE;
}

//+---------------------------------------------------------------------------
//
// ITfActiveLanguageProfileNotifySink::OnActivated
//
// Sink called by the framework when changes activate language profile.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnActivated(_In_ REFCLSID clsid, _In_ REFGUID guidProfile, _In_ BOOL isActivated)
{
	guidProfile;

    if (FALSE == VerifySampleIMECLSID(clsid))
    {
        return S_OK;
    }

    if (isActivated)
    {
        _AddTextProcessorEngine();
    }

    if (nullptr == _pCompositionProcessorEngine)
    {
        return S_OK;
    }

    if (isActivated)
    {
        _pCompositionProcessorEngine->ShowAllLanguageBarIcons();

        _pCompositionProcessorEngine->ConversionModeCompartmentUpdated(_pThreadMgr);
    }
    else
    {
        _DeleteCandidateList(FALSE, nullptr);

        _pCompositionProcessorEngine->HideAllLanguageBarIcons();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitActiveLanguageProfileNotifySink
//
// Advise a active language profile notify sink.
//----------------------------------------------------------------------------

BOOL CSampleIME::_InitActiveLanguageProfileNotifySink()
{
    Microsoft::WRL::ComPtr<ITfSource> pSource;

    if (FAILED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pSource))))
    {
        return FALSE;
    }

    if (FAILED(pSource->AdviseSink(IID_ITfActiveLanguageProfileNotifySink, (ITfActiveLanguageProfileNotifySink *)this, &_activeLanguageProfileNotifySinkCookie)))
    {
        _activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _UninitActiveLanguageProfileNotifySink
//
// Unadvise a active language profile notify sink.  Assumes we have advised one already.
//----------------------------------------------------------------------------

void CSampleIME::_UninitActiveLanguageProfileNotifySink()
{
    if (_activeLanguageProfileNotifySinkCookie == TF_INVALID_COOKIE)
    {
        return; // never Advised
    }

    Microsoft::WRL::ComPtr<ITfSource> pSource;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pSource))))
    {
        pSource->UnadviseSink(_activeLanguageProfileNotifySinkCookie);
    }

    _activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;
}
