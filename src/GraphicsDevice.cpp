///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "GraphicsDevice.h"
#include "LineRenderer.h"
#include "Profiler.h"
#include "SkyRenderer.h"
#include "VoxelRenderer.h"

GraphicsDevice::GraphicsDevice(HWND hwnd)
{
    assert(hwnd);

    //
    //  Create the Direct3D device.
    //
    D3DCHECK(CreateDXGIFactory(__uuidof(IDXGIFactory), AttachPtr(m_factory)));
    D3DCHECK(m_factory->EnumAdapters(0, AttachPtr(m_adapter)));

    UINT flags = 0;
#ifdef _DEBUG
    flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3DCHECK(D3D11CreateDevice(m_adapter.get(),             //  pAdapter
                               D3D_DRIVER_TYPE_UNKNOWN,     //  DriverType
                               NULL,                        //  Software
                               flags,                       //  Flags
                               NULL,                        //  pFeatureLevels
                               0,                           //  FeatureLevels
                               D3D11_SDK_VERSION,           //  SDKVersion
                               AttachPtr(m_device),         //  ppDevice
                               &m_featureLevel,             //  pFeatureLevel
                               AttachPtr(m_context)));      //  ppImmediateContext

    //
    //  Create the swap chain.
    //
    RECT wndRect;
    WINCHECK(GetClientRect(hwnd, &wndRect));
    DXGI_SWAP_CHAIN_DESC swapChainDesc =
    {
        {                                                   //  BufferDesc
            wndRect.right - wndRect.left,                   //      Width
            wndRect.bottom - wndRect.top,                   //      Height
            {                                               //      RefreshRate
                60,                                         //          Numerator
                1                                           //          Denominator
            },                                              
            DXGI_FORMAT_R8G8B8A8_UNORM,                     //      Format
            DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,           //      ScanlineOrdering
            DXGI_MODE_SCALING_UNSPECIFIED                   //      Scaling
        },
        {                                                   //  SampleDesc
            1,                                              //      Count
            0,                                              //      Quality
        },
        DXGI_USAGE_BACK_BUFFER |                            //  Usage
        DXGI_USAGE_RENDER_TARGET_OUTPUT,                  
        1,                                                  //  BufferCount
        hwnd,                                               //  OutputWindow
        TRUE,                                               //  Windowed
        DXGI_SWAP_EFFECT_DISCARD,                           //  SwapEffect
        0                                                   //  Flags
    };
    D3DCHECK(m_factory->CreateSwapChain(m_device.get(), 
                                        &swapChainDesc,   
                                        AttachPtr(m_swapChain))); 

    boost::intrusive_ptr<ID3D11Texture2D> backBuffer;
    D3DCHECK(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), AttachPtr(backBuffer)));

    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM,                         //  Format
        D3D11_RTV_DIMENSION_TEXTURE2DMS,                      //  ViewDimension
        {                                                   //  Texture2D
            0                                               //      MipSlice
        }
    };
    D3DCHECK(m_device->CreateRenderTargetView(backBuffer.get(),
                                              &renderTargetViewDesc,
                                              AttachPtr(m_renderTargetView)));

    float clearColor[4] = {0, 0, 0, 0};
    m_context->ClearRenderTargetView(m_renderTargetView.get(), clearColor);

    //
    //  Create the depth/stencil buffer.
    //
    boost::intrusive_ptr<ID3D11Texture2D> depthStencilBuffer;
    D3D11_TEXTURE2D_DESC depthStencilBufferDesc =
    {
        wndRect.right - wndRect.left,                       //  Width
        wndRect.bottom - wndRect.top,                       //  Height
        1,                                                  //  MipLevels
        1,                                                  //  ArraySize
        DXGI_FORMAT_D24_UNORM_S8_UINT ,                     //  Format
        {                                                   //  SampleDesc
            1,                                              //      Count
            0                                               //      Quality
        },
        D3D11_USAGE_DEFAULT,                                //  Usage
        D3D11_BIND_DEPTH_STENCIL,                           //  BindFlags
        0,                                                  //  CPUAccessFlags
        0                                                   //  MiscFlags
    };
    D3DCHECK(m_device->CreateTexture2D(&depthStencilBufferDesc,
                                       NULL,
                                       AttachPtr(depthStencilBuffer)));

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc =
    {   
       DXGI_FORMAT_D24_UNORM_S8_UINT,                      //  Format
        D3D11_DSV_DIMENSION_TEXTURE2DMS,                      //  ViewDimension
        0,                                                  //  Flags
        {                                                   //  Texture2D
            0                                               //      MipSlice
        }
    };
    D3DCHECK(m_device->CreateDepthStencilView(depthStencilBuffer.get(),
                                              &depthStencilViewDesc,
                                              AttachPtr(m_depthStencilView)));
    m_context->ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH, 0, 0);

    //
    //  Set up the viewport.
    //
    ID3D11RenderTargetView* renderTargetViewPtr = m_renderTargetView.get();
    m_context->OMSetRenderTargets(1, &renderTargetViewPtr, m_depthStencilView.get());

    D3D11_VIEWPORT viewport =
    {
        0.0f,                                               //  TopLeftX
        0.0f,                                               //  TopLeftY
        static_cast<float>(wndRect.right - wndRect.left),   //  Width
        static_cast<float>(wndRect.bottom - wndRect.top),   //  Height
        0.0f,                                               //  MinDepth
        1.0f                                                //  MaxDepth
    };
    m_viewport = viewport;
    m_context->RSSetViewports(1, &m_viewport);
}

GraphicsDevice::~GraphicsDevice()
{
}

void GraphicsDevice::Begin()
{
    ID3D11RenderTargetView* renderTargetViewPtr = m_renderTargetView.get();
    m_context->OMSetRenderTargets(1, &renderTargetViewPtr, m_depthStencilView.get());
    m_context->RSSetViewports(1, &m_viewport);
}

void GraphicsDevice::End()
{
    static Profiler profiler("GraphicsDevice::End()\n");
    profiler.Begin();

    m_swapChain->Present(0, 0);
    profiler.End();

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    m_context->ClearRenderTargetView(m_renderTargetView.get(), clearColor);
    m_context->ClearDepthStencilView(m_depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

