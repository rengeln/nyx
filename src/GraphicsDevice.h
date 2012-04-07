///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_GRAPHICSDEVICE_H__
#define __NYX_GRAPHICSDEVICE_H__

//
//  Forward declarations.
//
class VoxelRenderer;
class SkyRenderer;
class LineRenderer;

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
    //  Returns the line renderer instance.
    //
    LineRenderer& GetLineRenderer();

    //
    //  Returns the sky renderer instance.
    //
    SkyRenderer& GetSkyRenderer();

    //
    //  Returns the voxel renderer instance.
    //
    VoxelRenderer& GetVoxelRenderer();

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
    D3D11_VIEWPORT m_viewport;
    std::unique_ptr<LineRenderer> m_lineRenderer;
    std::unique_ptr<SkyRenderer> m_skyRenderer;
    std::unique_ptr<VoxelRenderer> m_voxelRenderer;
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

inline LineRenderer& GraphicsDevice::GetLineRenderer()
{
    assert(m_lineRenderer);
    return *m_lineRenderer;
}

inline SkyRenderer& GraphicsDevice::GetSkyRenderer()
{
    assert(m_skyRenderer);
    return *m_skyRenderer;
}

inline VoxelRenderer& GraphicsDevice::GetVoxelRenderer()
{
    assert(m_voxelRenderer);
    return *m_voxelRenderer;
}

#endif  // __NYX_GRAPHICSDEVICE_H__
