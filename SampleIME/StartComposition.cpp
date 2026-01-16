// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "EditSession.h"
#include "SampleIME.h"

//+---------------------------------------------------------------------------
//
// CStartCompositinoEditSession
//
//----------------------------------------------------------------------------

class CStartCompositionEditSession : public CEditSessionBase
{
public:
    CStartCompositionEditSession(_In_ CSampleIME *pTextService, _In_ ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
    {
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);
};

//+---------------------------------------------------------------------------
//
// ITfEditSession::DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CStartCompositionEditSession::DoEditSession(TfEditCookie ec)
{
    Microsoft::WRL::ComPtr<ITfInsertAtSelection> pInsertAtSelection;
    Microsoft::WRL::ComPtr<ITfRange> pRangeInsert;
    Microsoft::WRL::ComPtr<ITfContextComposition> pContextComposition;
    Microsoft::WRL::ComPtr<ITfComposition> pComposition;

    if (FAILED(_pContext->QueryInterface(IID_PPV_ARGS(&pInsertAtSelection))))
    {
        return S_OK;
    }

    if (FAILED(pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert)))
    {
        return S_OK;
    }

    if (FAILED(_pContext->QueryInterface(IID_PPV_ARGS(&pContextComposition))))
    {
        return S_OK;
    }

    if (SUCCEEDED(pContextComposition->StartComposition(ec, pRangeInsert.Get(), _pTextService, &pComposition)) && (nullptr != pComposition))
    {
        _pTextService->_SetComposition(pComposition.Get());

        // set selection to the adjusted range
        TF_SELECTION tfSelection;
        tfSelection.range = pRangeInsert.Get();
        tfSelection.style.ase = TF_AE_NONE;
        tfSelection.style.fInterimChar = FALSE;

        _pContext->SetSelection(ec, 1, &tfSelection);
        _pTextService->_SaveCompositionContext(_pContext.Get());
    }

    return S_OK;
}

//////////////////////////////////////////////////////////////////////
//
// CSampleIME class
//
//////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
// _StartComposition
//
// this starts the new (std::nothrow) composition at the selection of the current
// focus context.
//----------------------------------------------------------------------------

void CSampleIME::_StartComposition(_In_ ITfContext *pContext)
{
    CStartCompositionEditSession* pStartCompositionEditSession = new (std::nothrow) CStartCompositionEditSession(this, pContext);

    if (nullptr != pStartCompositionEditSession)
    {
        HRESULT hr = S_OK;
        pContext->RequestEditSession(_tfClientId, pStartCompositionEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);

        pStartCompositionEditSession->Release();
    }
}

//+---------------------------------------------------------------------------
//
// _SaveCompositionContext
//
// this saves the context _pComposition belongs to; we need this to clear
// text attribute in case composition has not been terminated on
// deactivation
//----------------------------------------------------------------------------

void CSampleIME::_SaveCompositionContext(_In_ ITfContext *pContext)
{
    assert(_pContext == nullptr);
    _pContext = pContext;
}
