///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "GraphicsDevice.h"
#include "RenderContext.h"
#include "SceneManager.h"
#include "WaterManager.h"
#include "WaterRenderer.h"

WaterManager::WaterManager(SceneManager& sceneManager)
: m_sceneManager(sceneManager),
  m_preventDrawing(false)
{
    m_waterRenderer.reset(new WaterRenderer(sceneManager.GetGraphicsDevice()));

    ID3D11Device& device = sceneManager.GetGraphicsDevice().GetD3DDevice();

    //
    //  Set up render to texture for reflections.
    //
    D3D11_TEXTURE2D_DESC reflectionTextureDesc =
    {
        512,                                                        //  Width
        512,                                                        //  Height
        1,                                                          //  MipLevels
        1,                                                          //  ArraySize
        DXGI_FORMAT_R8G8B8A8_UNORM,                                 //  Format
        {                                                           //  SampleDesc
            1,                                                      //      Count
            0                                                       //      Quality
        },
        D3D11_USAGE_DEFAULT,                                        //  Usage
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,      //  BindFlags
        0,                                                          //  CPUAccessFlags
        0                                                           //  MiscFlags
    };
    boost::intrusive_ptr<ID3D11Texture2D> reflectionTexture;
    D3DCHECK(device.CreateTexture2D(&reflectionTextureDesc,
                                    NULL,
                                    AttachPtr(reflectionTexture)));

    D3D11_SHADER_RESOURCE_VIEW_DESC reflectionSrvDesc =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM,                                 //  Format
        D3D11_SRV_DIMENSION_TEXTURE2D,                              //  ViewDimension
        {                                                           //  Texture2D
            0,                                                      //      MostDetailedMip
            -1                                                      //      MipLevels
        }
    };
    D3DCHECK(device.CreateShaderResourceView(reflectionTexture.get(),
                                             &reflectionSrvDesc,
                                             AttachPtr(m_reflectionSRV)));

    D3D11_RENDER_TARGET_VIEW_DESC reflectionRtvDesc =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM,                                 //  Format
        D3D11_RTV_DIMENSION_TEXTURE2D,                              //  ViewDimension
        {                                                           //  Texture2D
            0                                                       //      MipSlice
        }
    };
    D3DCHECK(device.CreateRenderTargetView(reflectionTexture.get(),
                                           &reflectionRtvDesc,
                                           AttachPtr(m_reflectionRTV)));

    D3D11_TEXTURE2D_DESC reflectionDepthDesc =
    {
        512,                                                        //  Width
        512,                                                        //  Height
        1,                                                          //  MipLevels
        1,                                                          //  ArraySize
        DXGI_FORMAT_D24_UNORM_S8_UINT,                              //  Format
        {                                                           //  SampleDesc
            1,                                                      //      Count
            0                                                       //      Quality
        },
        D3D11_USAGE_DEFAULT,                                        //  Usage
        D3D11_BIND_DEPTH_STENCIL,                                   //  BindFlags
        0,                                                          //  CPUAccessFlags
        0                                                           //  MiscFlags
    };
    boost::intrusive_ptr<ID3D11Texture2D> reflectionDepth;
    D3DCHECK(device.CreateTexture2D(&reflectionDepthDesc,
                                    NULL,
                                    AttachPtr(reflectionDepth))); 
    
    D3D11_DEPTH_STENCIL_VIEW_DESC reflectionDsvDesc =
    {
        DXGI_FORMAT_D24_UNORM_S8_UINT,                              //  Format
        D3D11_DSV_DIMENSION_TEXTURE2D,                              //  ViewDimension
        0,                                                          //  Flags
        {                                                           //  Texture2D
            0                                                       //      MipSlice
        }
    };
    D3DCHECK(device.CreateDepthStencilView(reflectionDepth.get(),
                                           &reflectionDsvDesc,
                                           AttachPtr(m_reflectionDSV)));

    D3D11_RASTERIZER_DESC reflectionRSDesc =
    {
        D3D11_FILL_SOLID,                                               //  FillMode
        D3D11_CULL_BACK,                                                //  CullMode
        TRUE,                                                           //  FrontCounterClockwise
        0,                                                              //  DepthBias
        0.0f,                                                           //  DepthBiasClamp
        0.0f,                                                           //  SlopeScaledDepthBias
        FALSE,                                                          //  DepthClipEnable
        FALSE,                                                          //  ScissorEnable
        FALSE,                                                          //  MultisampleEnable
        FALSE                                                           //  AntialiasedLineEnable
    };
    D3DCHECK(device.CreateRasterizerState(&reflectionRSDesc,
                                          AttachPtr(m_reflectionRS)));
}

WaterManager::~WaterManager()
{
}

void WaterManager::Update()
{
}

void WaterManager::PreDraw(RenderContext& renderContext, const Camera& camera)
{
    ID3D11DeviceContext& context = m_sceneManager.GetGraphicsDevice().GetD3DContext();

    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    context.ClearRenderTargetView(m_reflectionRTV.get(), clearColor);
    context.ClearDepthStencilView(m_reflectionDSV.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    renderContext.PushRenderTarget(m_reflectionRTV, m_reflectionDSV);
    D3D11_VIEWPORT viewport =
    {
        0.0f,                                               //  TopLeftX
        0.0f,                                               //  TopLeftY
        512,                                                //  Width
        512,                                                //  Height
        0.0f,                                               //  MinDepth
        1.0f                                                //  MaxDepth
    };
    renderContext.PushViewport(viewport);
    renderContext.PushRasterizerState(m_reflectionRS.get());

    SceneConstants sceneConstants;
    sceneConstants.projectionMatrix = camera.GetProjectionMatrix();
    sceneConstants.viewMatrix = camera.GetViewMatrix();
    sceneConstants.cameraPos = camera.GetPosition();
    sceneConstants.clipPlane = float4(0, 1.0f, 0, 0);
    sceneConstants.lowDetail = true;
    
    //  Mirror the view matrix along the water plane
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&sceneConstants.viewMatrix),
                    XMMatrixMultiply(XMMatrixReflect(XMVectorSet(0, -1.0f, 0, 0)),
                                     XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&sceneConstants.viewMatrix))));
    float4x4 combinedMatrix = sceneConstants.viewMatrix * sceneConstants.projectionMatrix;
    sceneConstants.frustum = combinedMatrix;

    m_preventDrawing = true;
    m_sceneManager.DrawStage(renderContext, sceneConstants);
    m_preventDrawing = false;

    renderContext.PopRasterizerState();
    renderContext.PopViewport();
    renderContext.PopRenderTarget();
}

void WaterManager::Draw(RenderContext& renderContext,
                        const SceneConstants& sceneConstants)
{
    if (!m_preventDrawing)
    {
        m_waterRenderer->Draw(renderContext,
                              sceneConstants,
                              *m_reflectionSRV);
    }
}
