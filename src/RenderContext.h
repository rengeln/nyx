///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_RENDERCONTEXT_H__
#define __NYX_RENDERCONTEXT_H__

class GraphicsDevice;

//
//  Encapsulates a Direct3D rendering context, implementing push and pop
//  behavior for setting rendering state.
//
class RenderContext : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] graphicsDevice
    //          Parent GraphicsDevice instance.
    //      [in] context
    //          Direct3D context to encapsulate.
    //
    RenderContext(GraphicsDevice& graphicsDevice,
                  boost::intrusive_ptr<ID3D11DeviceContext> context);

    //
    //  Destructor.
    //
    ~RenderContext();

    //
    //  Returns the Direct3D device context.
    //
    ID3D11DeviceContext& GetD3DContext();

    //
    //  Reapplies all of the objects at the top of the stack.
    //
    //  This lets RenderContext play nice with legacy code which isn't using it.
    //
    void Apply();

    //
    //  Pushes a rasterizer state and activates it.
    //
    void PushRasterizerState(boost::intrusive_ptr<ID3D11RasterizerState> rs);

    //
    //  Pops the rasterizer state.
    //
    void PopRasterizerState();

    //
    //  Pushes a depth/stencil state and activates it.
    //
    void PushDepthStencilState(boost::intrusive_ptr<ID3D11DepthStencilState> ds);

    //
    //  Pops the depth/stencil state.
    //
    void PopDepthStencilState();

    //
    //  Pushes a blend state and activates it.
    //
    void PushBlendState(boost::intrusive_ptr<ID3D11BlendState> bs);

    //
    //  Pops the blend state.
    //
    void PopBlendState();

    //
    //  Pushes a render target view (with an optional depth/stencil buffer) and activates it.
    //
    void PushRenderTarget(boost::intrusive_ptr<ID3D11RenderTargetView> rtv,
                          boost::intrusive_ptr<ID3D11DepthStencilView> dsv = nullptr);

    //
    //  Pops the render target.
    //
    void PopRenderTarget();

    //
    //  Pushes a viewport and activates it.
    //
    void PushViewport(D3D11_VIEWPORT vp);

    //
    //  Pops the viewport.
    //
    void PopViewport();

private:
    //
    //  Properties.
    //
    GraphicsDevice& m_graphicsDevice;
    boost::intrusive_ptr<ID3D11DeviceContext> m_d3dContext;
    std::stack<boost::intrusive_ptr<ID3D11RasterizerState>> m_rasterizerStateStack;
    std::stack<boost::intrusive_ptr<ID3D11DepthStencilState>> m_depthStencilStateStack;
    std::stack<boost::intrusive_ptr<ID3D11BlendState>> m_blendStateStack;
    std::stack<std::pair<boost::intrusive_ptr<ID3D11RenderTargetView>,
                         boost::intrusive_ptr<ID3D11DepthStencilView>>> m_renderTargetStack;
    std::stack<D3D11_VIEWPORT> m_viewportStack;
};

inline ID3D11DeviceContext& RenderContext::GetD3DContext()
{
    assert(m_d3dContext);
    return *m_d3dContext;
}

#endif  //  __NYX_RENDERCONTEXT_H__
