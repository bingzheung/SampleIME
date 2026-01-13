// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "BaseWindow.h"
#include "ShadowWindow.h"

#define SHADOW_ALPHANUMBER (2)

//+---------------------------------------------------------------------------
//
// _Create
//
//----------------------------------------------------------------------------

BOOL CShadowWindow::_Create(ATOM atom, DWORD dwExStyle, DWORD dwStyle, _In_opt_ CBaseWindow* pParent, int wndWidth, int wndHeight)
{
    if (!CBaseWindow::_Create(atom, dwExStyle, dwStyle, pParent, wndWidth, wndHeight))
    {
        return FALSE;
    }

    return _Initialize();
}

//+---------------------------------------------------------------------------
//
// _WindowProcCallback
//
// Shadow window proc.
//----------------------------------------------------------------------------

LRESULT CALLBACK CShadowWindow::_WindowProcCallback(_In_ HWND wndHandle, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        HDC dcHandle;
        PAINTSTRUCT ps;

        dcHandle = BeginPaint(wndHandle, &ps);

        HBRUSH hBrush = CreateSolidBrush(_color);
        FillRect(dcHandle, &ps.rcPaint, hBrush);
        DeleteObject(hBrush);

        EndPaint(wndHandle, &ps);
    }
    return 0;

    case WM_SETTINGCHANGE:
        _OnSettingChange();
        break;
    }

    return DefWindowProc(wndHandle, uMsg, wParam, lParam);
}

//+---------------------------------------------------------------------------
//
// _OnSettingChange
//
//----------------------------------------------------------------------------

void CShadowWindow::_OnSettingChange()
{
    _InitSettings();

    DWORD dwWndStyleEx = GetWindowLong(_GetWnd(), GWL_EXSTYLE);

    if (_isGradient)
    {
        SetWindowLong(_GetWnd(), GWL_EXSTYLE, (dwWndStyleEx | WS_EX_LAYERED));
    }
    else
    {
        SetWindowLong(_GetWnd(), GWL_EXSTYLE, (dwWndStyleEx & ~WS_EX_LAYERED));
    }

    _AdjustWindowPos();
    _InitShadow();
}

//+---------------------------------------------------------------------------
//
// _OnOwnerWndMoved
//
//----------------------------------------------------------------------------

void CShadowWindow::_OnOwnerWndMoved(BOOL isResized)
{
    if (IsWindow(_GetWnd()) && _IsWindowVisible())
    {
        _AdjustWindowPos();
        if (isResized)
        {
            _InitShadow();
        }
    }
}


//+---------------------------------------------------------------------------
//
// _Show
//
//----------------------------------------------------------------------------

void CShadowWindow::_Show(BOOL isShowWnd)
{
    _OnOwnerWndMoved(TRUE);
    CBaseWindow::_Show(isShowWnd);
}

//+---------------------------------------------------------------------------
//
// _Initialize
//
//----------------------------------------------------------------------------

BOOL CShadowWindow::_Initialize()
{
    _InitSettings();

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _InitSettings
//
//----------------------------------------------------------------------------

void CShadowWindow::_InitSettings()
{
    HDC dcHandle = GetDC(nullptr);

    // device caps
    int cBitsPixelScreen = GetDeviceCaps(dcHandle, BITSPIXEL);

    _isGradient = cBitsPixelScreen > 8;

    ReleaseDC(nullptr, dcHandle);

    if (_isGradient)
    {
        _color = RGB(0, 0, 0);
        _sizeShift.cx = SHADOW_ALPHANUMBER;
        _sizeShift.cy = SHADOW_ALPHANUMBER;
    }
    else
    {
        _color = RGB(128, 128, 128);
        _sizeShift.cx = 2;
        _sizeShift.cy = 2;
    }
}

//+---------------------------------------------------------------------------
//
// _AdjustWindowPos
//
//----------------------------------------------------------------------------

void CShadowWindow::_AdjustWindowPos()
{
    if (!IsWindow(_GetWnd()))
    {
        return;
    }

    HWND hWndOwner = _pWndOwner->_GetWnd();
    RECT rc = { 0, 0, 0, 0 };

    GetWindowRect(hWndOwner, &rc);
    // Modern shadow: centered on all edges
    SetWindowPos(_GetWnd(), hWndOwner,
        rc.left - SHADOW_ALPHANUMBER,
        rc.top - SHADOW_ALPHANUMBER,
        (rc.right - rc.left) + SHADOW_ALPHANUMBER * 2,
        (rc.bottom - rc.top) + SHADOW_ALPHANUMBER * 2,
        SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}

//+---------------------------------------------------------------------------
//
// _InitShadow
//
//----------------------------------------------------------------------------

#define GETRGBALPHA(_x_, _y_) ((RGBALPHA *)pDIBits + (_y_) * size.cx + (_x_))

void CShadowWindow::_InitShadow()
{
    typedef struct _RGBAPLHA {
        BYTE rgbBlue;
        BYTE rgbGreen;
        BYTE rgbRed;
        BYTE rgbAlpha;
    } RGBALPHA;

    HDC dcScreenHandle = nullptr;
    HDC dcLayeredHandle = nullptr;
    RECT rcWindow = { 0, 0, 0, 0 };
    SIZE size = { 0, 0 };
    BITMAPINFO bitmapInfo;
    HBITMAP bitmapMemHandle = nullptr;
    HBITMAP bitmapOldHandle = nullptr;
    void* pDIBits = nullptr;
    POINT ptSrc = { 0, 0 };
    POINT ptDst = { 0, 0 };
    BLENDFUNCTION Blend;

    if (!_isGradient)
    {
        return;
    }

    SetWindowLong(_GetWnd(), GWL_EXSTYLE, (GetWindowLong(_GetWnd(), GWL_EXSTYLE) | WS_EX_LAYERED));

    _GetWindowRect(&rcWindow);
    size.cx = rcWindow.right - rcWindow.left;
    size.cy = rcWindow.bottom - rcWindow.top;

    if (size.cx <= 0 || size.cy <= 0) return;

    dcScreenHandle = GetDC(nullptr);
    if (dcScreenHandle == nullptr) {
        return;
    }

    dcLayeredHandle = CreateCompatibleDC(dcScreenHandle);
    if (dcLayeredHandle == nullptr) {
        ReleaseDC(nullptr, dcScreenHandle);
        return;
    }

    // create bitmap
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = size.cx;
    bitmapInfo.bmiHeader.biHeight = size.cy;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    bitmapInfo.bmiHeader.biSizeImage = 0;
    bitmapInfo.bmiHeader.biXPelsPerMeter = 100;
    bitmapInfo.bmiHeader.biYPelsPerMeter = 100;
    bitmapInfo.bmiHeader.biClrUsed = 0;
    bitmapInfo.bmiHeader.biClrImportant = 0;

    bitmapMemHandle = CreateDIBSection(dcScreenHandle, &bitmapInfo, DIB_RGB_COLORS, &pDIBits, nullptr, 0);
    if (pDIBits == nullptr || bitmapMemHandle == nullptr) {
        ReleaseDC(nullptr, dcScreenHandle);
        DeleteDC(dcLayeredHandle);
        if (bitmapMemHandle) DeleteObject(bitmapMemHandle);
        return;
    }

    memset(pDIBits, 0, size.cx * size.cy * 4);

    const int radius = 8; // Match the rounded corners of the window

    // Compute shadow alpha with rounded corners
    for (int y = 0; y < size.cy; y++) {
        for (int x = 0; x < size.cx; x++) {
            // Rect of the owner window inside the shadow window
            // [SHADOW_ALPHANUMBER, size.cx - SHADOW_ALPHANUMBER - 1]
            // [SHADOW_ALPHANUMBER, size.cy - SHADOW_ALPHANUMBER - 1]

            float dx = 0;
            float dy = 0;

            if (x < SHADOW_ALPHANUMBER + radius)
                dx = (float)(SHADOW_ALPHANUMBER + radius - x);
            else if (x >= size.cx - SHADOW_ALPHANUMBER - radius)
                dx = (float)(x - (size.cx - SHADOW_ALPHANUMBER - radius) + 1);

            if (y < SHADOW_ALPHANUMBER + radius)
                dy = (float)(SHADOW_ALPHANUMBER + radius - y);
            else if (y >= size.cy - SHADOW_ALPHANUMBER - radius)
                dy = (float)(y - (size.cy - SHADOW_ALPHANUMBER - radius) + 1);

            float dist = 0;
            if (dx > radius && dy > radius) {
                // In the corner regions, beyond the radius
                float rdx = dx - radius;
                float rdy = dy - radius;
                dist = sqrt(rdx * rdx + rdy * rdy);
            }
            else {
                // In the edge or center regions
                dx = 0; dy = 0;
                if (x < SHADOW_ALPHANUMBER) dx = (float)(SHADOW_ALPHANUMBER - x);
                else if (x >= size.cx - SHADOW_ALPHANUMBER) dx = (float)(x - (size.cx - SHADOW_ALPHANUMBER) + 1);
                if (y < SHADOW_ALPHANUMBER) dy = (float)(SHADOW_ALPHANUMBER - y);
                else if (y >= size.cy - SHADOW_ALPHANUMBER) dy = (float)(y - (size.cy - SHADOW_ALPHANUMBER) + 1);
                dist = max(dx, dy);
            }

            if (dist < SHADOW_ALPHANUMBER) {
                float ratio = 1.0f - (dist / SHADOW_ALPHANUMBER);
                BYTE alpha = (BYTE)(120.0f * ratio * ratio);
                RGBALPHA* ppxl = GETRGBALPHA(x, y);
                ppxl->rgbAlpha = alpha;
            }
            else if (dist == 0) {
                RGBALPHA* ppxl = GETRGBALPHA(x, y);
                ppxl->rgbAlpha = 120;
            }
        }
    }

    ptSrc.x = 0;
    ptSrc.y = 0;
    ptDst.x = rcWindow.left;
    ptDst.y = rcWindow.top;
    Blend.BlendOp = AC_SRC_OVER;
    Blend.BlendFlags = 0;
    Blend.SourceConstantAlpha = 255;
    Blend.AlphaFormat = AC_SRC_ALPHA;

    bitmapOldHandle = (HBITMAP)SelectObject(dcLayeredHandle, bitmapMemHandle);

    UpdateLayeredWindow(_GetWnd(), dcScreenHandle, nullptr, &size, dcLayeredHandle, &ptSrc, 0, &Blend, ULW_ALPHA);

    SelectObject(dcLayeredHandle, bitmapOldHandle);

    ReleaseDC(nullptr, dcScreenHandle);
    DeleteDC(dcLayeredHandle);
    DeleteObject(bitmapMemHandle);
}
