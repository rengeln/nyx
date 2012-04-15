///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_GRAPHICSDEVICE_H__
#define __NYX_GRAPHICSDEVICE_H__

//
//  Forward declarations.
//
class RenderContext;

//
//  Encapsulates the graphics engine.
//
class GraphicsDevice : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] hwnd
    //          Render window.
    //
    GraphicsDevice(HWND hwnd);

    //
    //  Destructor.
    //
    ~GraphicsDevice();

    //
    //  Begins per-frame rendering operations.
    //
    void Begin();

    //
    //  Ends per-frame rendering operations.
    //
    void End();

    //
    //  Returns the Direct3D device.
    //
    ID3D11Device& GetD3DDevice();

    //
    //  Returns the Direct3D immediate context.
    //
    ID3D11DeviceContext& GetD3DContext();

    //
    //  Returns the D3DInclude implementation.
    //
    ID3D10Include& GetD3DInclude();

    //
    //  Returns the primary RenderContext.
    //
    RenderContext& GetRenderContext();

private:
    //
    //  Properties.
    //
    boost::intrusive_ptr<IDXGIFactory> m_factory;
    boost::intrusive_ptr<IDXGIAdapter> m_adapter;
    boost::intrusive_ptr<ID3D11Device> m_device;
    boost::intrusive_ptr<ID3D11DeviceContext> m_context;
    boost::intrusive_ptr<IDXGISwapChain> m_swapChain;
    boost::intrusive_ptr<ID3D11RenderTargetView> m_renderTargetView;
    boost::intrusive_ptr<ID3D11DepthStencilView> m_depthStencilView;
    D3D_FEATURE_LEVEL m_featureLevel;
    std::unique_ptr<RenderContext> m_renderContext;
};

inline ID3D11Device& GraphicsDevice::GetD3DDevice()
{
    assert(m_device);
    return *m_device;
}

inline ID3D11DeviceContext& GraphicsDevice::GetD3DContext()
{
    assert(m_context);
    return *m_context;
}

inline RenderContext& GraphicsDevice::GetRenderContext()
{
    assert(m_renderContext);
    return *m_renderContext;
}

#endif  // __NYX_GRAPHICSDEVICE_H__
