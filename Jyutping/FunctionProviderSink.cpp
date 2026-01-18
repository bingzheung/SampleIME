#include "Private.h"
#include "Jyutping.h"
#include "SearchCandidateProvider.h"

//+---------------------------------------------------------------------------
//
// _InitFunctionProviderSink
//
//----------------------------------------------------------------------------

BOOL CSampleIME::_InitFunctionProviderSink()
{
    ITfSourceSingle* pSourceSingle = nullptr;
    BOOL ret = FALSE;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle)))
    {
        IUnknown* punk = nullptr;
        if (SUCCEEDED(QueryInterface(IID_IUnknown, (void **)&punk)))
        {
            if (SUCCEEDED(pSourceSingle->AdviseSingleSink(_tfClientId, IID_ITfFunctionProvider, punk)))
            {
                if (SUCCEEDED(CSearchCandidateProvider::CreateInstance(&_pITfFnSearchCandidateProvider, (ITfTextInputProcessorEx*)this)))
                {
                    ret = TRUE;
                }
            }
            punk->Release();
        }
        pSourceSingle->Release();
    }
    return ret;
}

//+---------------------------------------------------------------------------
//
// _UninitFunctionProviderSink
//
//----------------------------------------------------------------------------

void CSampleIME::_UninitFunctionProviderSink()
{
    ITfSourceSingle* pSourceSingle = nullptr;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle)))
    {
        pSourceSingle->UnadviseSingleSink(_tfClientId, IID_ITfFunctionProvider);
        pSourceSingle->Release();
    }
}
