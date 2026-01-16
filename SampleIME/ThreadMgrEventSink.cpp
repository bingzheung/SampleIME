// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "SampleIME.h"
#include "CandidateListUIPresenter.h"

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnInitDocumentMgr
//
// Sink called by the framework just before the first context is pushed onto
// a document.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnInitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr)
{
    pDocMgr;
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnUninitDocumentMgr
//
// Sink called by the framework just after the last context is popped off a
// document.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnUninitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr)
{
    pDocMgr;
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnSetFocus
//
// Sink called by the framework when focus changes from one document to
// another.  Either document may be NULL, meaning previously there was no
// focus document, or now no document holds the input focus.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus, _In_ ITfDocumentMgr *pDocMgrPrevFocus)
{
    pDocMgrPrevFocus;

    _InitTextEditSink(pDocMgrFocus);

    _UpdateLanguageBarOnSetFocus(pDocMgrFocus);

    //
    // We have to hide/unhide candidate list depending on whether they are
    // associated with pDocMgrFocus.
    //
    if (_pCandidateListUIPresenter)
    {
        Microsoft::WRL::ComPtr<ITfDocumentMgr> pCandidateListDocumentMgr;
        ITfContext* pTfContext = _pCandidateListUIPresenter->_GetContextDocument();
        if ((nullptr != pTfContext) && SUCCEEDED(pTfContext->GetDocumentMgr(&pCandidateListDocumentMgr)))
        {
            if (pCandidateListDocumentMgr.Get() != pDocMgrFocus)
            {
                _pCandidateListUIPresenter->OnKillThreadFocus();
            }
            else
            {
                _pCandidateListUIPresenter->OnSetThreadFocus();
            }
        }
    }

    _pDocMgrLastFocused = pDocMgrFocus;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnPushContext
//
// Sink called by the framework when a context is pushed.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnPushContext(_In_ ITfContext *pContext)
{
    pContext;

    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnPopContext
//
// Sink called by the framework when a context is popped.
//----------------------------------------------------------------------------

STDAPI CSampleIME::OnPopContext(_In_ ITfContext *pContext)
{
    pContext;

    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// _InitThreadMgrEventSink
//
// Advise our sink.
//----------------------------------------------------------------------------

BOOL CSampleIME::_InitThreadMgrEventSink()
{
    Microsoft::WRL::ComPtr<ITfSource> pSource;

    if (FAILED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pSource))))
    {
        return FALSE;
    }

    if (FAILED(pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink *)this, &_threadMgrEventSinkCookie)))
    {
        _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _UninitThreadMgrEventSink
//
// Unadvise our sink.
//----------------------------------------------------------------------------

void CSampleIME::_UninitThreadMgrEventSink()
{
    if (_threadMgrEventSinkCookie == TF_INVALID_COOKIE)
    {
        return;
    }

    Microsoft::WRL::ComPtr<ITfSource> pSource;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pSource))))
    {
        pSource->UnadviseSink(_threadMgrEventSinkCookie);
    }

    _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
}
