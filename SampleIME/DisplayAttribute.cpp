// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "globals.h"
#include "SampleIME.h"

//+---------------------------------------------------------------------------
//
// _ClearCompositionDisplayAttributes
//
//----------------------------------------------------------------------------

void CSampleIME::_ClearCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext)
{
    Microsoft::WRL::ComPtr<ITfRange> pRangeComposition;
    Microsoft::WRL::ComPtr<ITfProperty> pDisplayAttributeProperty;

    // get the compositon range.
    if (FAILED(_pComposition->GetRange(&pRangeComposition)))
    {
        return;
    }

    // get our the display attribute property
    if (SUCCEEDED(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty)))
    {
        // clear the value over the range
        pDisplayAttributeProperty->Clear(ec, pRangeComposition.Get());
    }
}

//+---------------------------------------------------------------------------
//
// _SetCompositionDisplayAttributes
//
//----------------------------------------------------------------------------

BOOL CSampleIME::_SetCompositionDisplayAttributes(TfEditCookie ec, _In_ ITfContext *pContext, TfGuidAtom gaDisplayAttribute)
{
    Microsoft::WRL::ComPtr<ITfRange> pRangeComposition;
    Microsoft::WRL::ComPtr<ITfProperty> pDisplayAttributeProperty;

    // we need a range and the context it lives in
    if (FAILED(_pComposition->GetRange(&pRangeComposition)))
    {
        return FALSE;
    }

    // get our the display attribute property
    if (SUCCEEDED(pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty)))
    {
        VARIANT var;
        // set the value over the range
        // the application will use this guid atom to lookup the acutal rendering information
        var.vt = VT_I4; // we're going to set a TfGuidAtom
        var.lVal = gaDisplayAttribute;

        return SUCCEEDED(pDisplayAttributeProperty->SetValue(ec, pRangeComposition.Get(), &var));
    }

    return FALSE;
}

//+---------------------------------------------------------------------------
//
// _InitDisplayAttributeGuidAtom
//
// Because it's expensive to map our display attribute GUID to a TSF
// TfGuidAtom, we do it once when Activate is called.
//----------------------------------------------------------------------------

BOOL CSampleIME::_InitDisplayAttributeGuidAtom()
{
    Microsoft::WRL::ComPtr<ITfCategoryMgr> pCategoryMgr;
    HRESULT hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCategoryMgr));

    if (FAILED(hr))
    {
        return FALSE;
    }

    // register the display attribute for input text.
    hr = pCategoryMgr->RegisterGUID(Global::SampleIMEGuidDisplayAttributeInput, &_gaDisplayAttributeInput);
    if (FAILED(hr))
    {
        return FALSE;
    }
    // register the display attribute for the converted text.
    hr = pCategoryMgr->RegisterGUID(Global::SampleIMEGuidDisplayAttributeConverted, &_gaDisplayAttributeConverted);

    return SUCCEEDED(hr);
}
