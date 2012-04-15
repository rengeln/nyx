///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "GraphicsDevice.h"
#include "RenderContext.h"

RenderContext::RenderContext(GraphicsDevice& graphicsDevice,  
                             boost::intrusive_ptr<ID3D11DeviceContext> context)
: m_graphicsDevice(graphicsDevice),
  m_d3dContext(context)
{
    ID3D11Device& device = m_graphicsDevice.GetD3DDevice();

    //
    //  Default rasterizer state.
    //
    D3D11_RASTERIZER_DESC rsDesc =
    {
        D3D11_FILL_SOLID,                                               //  FillMode
        D3D11_CULL_BACK,                                                //  CullMode
        FALSE,                                                          //  FrontCounterClockwise
        0,                                                              //  DepthBias
        0.0f,                                                           //  DepthBiasClamp
        0.0f,                                                           //  SlopeScaledDepthBias
        FALSE,                                                          //  DepthClipEnable
        FALSE,                                                          //  ScissorEnable
        FALSE,                                                          //  MultisampleEnable
        FALSE                                                           //  AntialiasedLineEnable
    };
    boost::intrusive_ptr<ID3D11RasterizerState> rs;
    D3DCHECK(device.CreateRasterizerState(&rsDesc, AttachPtr(rs)));
    PushRasterizerState(rs);

    //
    //  Default depth-stencil state.
    //
    D3D11_DEPTH_STENCIL_DESC dsDesc = 
    {
        TRUE,                                                           //  DepthEnable
        D3D11_DEPTH_WRITE_MASK_ALL,                                     //  DepthWriteMask
        D3D11_COMPARISON_LESS,                                          //  ComparisonFunc
        FALSE,                                                          //  StencilEnable
        0,                                                              //  StencilReadMask
        0xFF,                                                           //  StencilWriteMask
        {                                                               //  FrontFace
            D3D11_STENCIL_OP_KEEP,                                      //      StencilFailOp
            D3D11_STENCIL_OP_KEEP,                                      //      StencilDepthFailOp
            D3D11_STENCIL_OP_KEEP,                                      //      StencilPassOp
            D3D11_COMPARISON_ALWAYS,                                    //      StencilFunc
        },
        {                                                               //  BackFace
            D3D11_STENCIL_OP_KEEP,                                      //      StencilFailOp
            D3D11_STENCIL_OP_KEEP,                                      //      StencilDepthFailOp
            D3D11_STENCIL_OP_KEEP,                                      //      StencilPassOp
            D3D11_COMPARISON_NEVER,                                     //      StencilFunc
        }
    };
    boost::intrusive_ptr<ID3D11DepthStencilState> ds;
    D3DCHECK(device.CreateDepthStencilState(&dsDesc, AttachPtr(ds)));
    PushDepthStencilState(ds);

    //
    //  Default blend state.
    //
    D3D11_BLEND_DESC bsDesc =
    {
        FALSE,                                                          //  AlphaToCoverageEnable
        FALSE,                                                          //  IndependentBlendEnable
        {                                                               //  RenderTarget[0]
            FALSE,                                                      //  BlendEnable
            D3D11_BLEND_ONE,                                            //  SrcBlend
            D3D11_BLEND_ZERO,                                           //  DestBlend
            D3D11_BLEND_OP_ADD,                                         //  BlendOp
            D3D11_BLEND_ONE,                                            //  SrcBlendAlpha
            D3D11_BLEND_ZERO,                                           //  DestBlendAlpha
            D3D11_BLEND_OP_ADD,                                         //  BlendOpAlpha
            D3D11_COLOR_WRITE_ENABLE_ALL                                //  RenderTargetWriteMask
        }
    };
    boost::intrusive_ptr<ID3D11BlendState> bs;
    D3DCHECK(device.CreateBlendState(&bsDesc, AttachPtr(bs)));
    PushBlendState(bs);
}

RenderContext::~RenderContext()
{
}

void RenderContext::PushRasterizerState(boost::intrusive_ptr<ID3D11RasterizerState> rs)
{
    assert(rs);
    m_d3dContext->RSSetState(rs.get());
    m_rasterizerStateStack.push(rs);
}

void RenderContext::PopRasterizerState()
{
    assert(m_rasterizerStateStack.size() > 1);
    m_rasterizerStateStack.pop();
    m_d3dContext->RSSetState(m_rasterizerStateStack.top().get());
}

void RenderContext::PushDepthStencilState(boost::intrusive_ptr<ID3D11DepthStencilState> ds)
{
    assert(ds);
    m_d3dContext->OMSetDepthStencilState(ds.get(), 0);
    m_depthStencilStateStack.push(ds);
}

void RenderContext::PopDepthStencilState()
{
    assert(m_depthStencilStateStack.size() > 1);
    m_depthStencilStateStack.pop();
    m_d3dContext->OMSetDepthStencilState(m_depthStencilStateStack.top().get(), 0);
}

void RenderContext::PushBlendState(boost::intrusive_ptr<ID3D11BlendState> bs)
{
    assert(bs);
    float factors[] = {1.0f, 1.0f, 1.0f, 1.0f};
    m_d3dContext->OMSetBlendState(bs.get(), factors, 0xFFFFFFFF);
    m_blendStateStack.push(bs);
}

void RenderContext::PopBlendState()
{
    assert(m_blendStateStack.size() > 1);
    m_blendStateStack.pop();
    float factors[] = {1.0f, 1.0f, 1.0f, 1.0f};
    m_d3dContext->OMSetBlendState(m_blendStateStack.top().get(), factors, 0xFFFFFFFF);
}

void RenderContext::PushRenderTarget(boost::intrusive_ptr<ID3D11RenderTargetView> rtv,
                                     boost::intrusive_ptr<ID3D11DepthStencilView> dsv)
{
    assert(rtv);
    ID3D11RenderTargetView* rtvs[] = 
    {
        rtv.get()
    };
    m_d3dContext->OMSetRenderTargets(1, rtvs, dsv.get());
    m_renderTargetStack.push(std::make_pair(rtv, dsv));
}

void RenderContext::PopRenderTarget()
{
    assert(m_renderTargetStack.size() > 0);
    m_renderTargetStack.pop();
    if (m_renderTargetStack.empty())
    {
        ID3D11RenderTargetView* nullRtv = nullptr;
        ID3D11DepthStencilView* nullDsv = nullptr;
        m_d3dContext->OMSetRenderTargets(1, &nullRtv, nullDsv);
    }
    else
    {
        ID3D11RenderTargetView* rtvs[] =
        {
            m_renderTargetStack.top().first.get()
        };
        m_d3dContext->OMSetRenderTargets(1, rtvs, m_renderTargetStack.top().second.get());
    }
}

void RenderContext::PushViewport(D3D11_VIEWPORT vp)
{
    m_d3dContext->RSSetViewports(1, &vp);
    m_viewportStack.push(vp);
}

void RenderContext::PopViewport()
{
    m_viewportStack.pop();
    if (!m_viewportStack.empty())
    {
        m_d3dContext->RSSetViewports(1, &m_viewportStack.top());
    }
}

void RenderContext::Apply()
{
    m_d3dContext->RSSetState(m_rasterizerStateStack.top().get());
    m_d3dContext->OMSetDepthStencilState(m_depthStencilStateStack.top().get(), 0);
    float factors[] = {1.0f, 1.0f, 1.0f, 1.0f};
    m_d3dContext->OMSetBlendState(m_blendStateStack.top().get(), factors, 0xFFFFFFFF);
    if (m_renderTargetStack.empty())
    {
        ID3D11RenderTargetView* nullRtv = nullptr;
        ID3D11DepthStencilView* nullDsv = nullptr;
        m_d3dContext->OMSetRenderTargets(1, &nullRtv, nullDsv);
    }
    else
    {
        ID3D11RenderTargetView* rtvs[] =
        {
            m_renderTargetStack.top().first.get()
        };
        m_d3dContext->OMSetRenderTargets(1, rtvs, m_renderTargetStack.top().second.get());
    }
    if (!m_viewportStack.empty())
    {
        m_d3dContext->RSSetViewports(1, &m_viewportStack.top());
    }
}
