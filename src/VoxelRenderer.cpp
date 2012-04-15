///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Camera.h"
#include "GraphicsDevice.h"
#include "RenderContext.h"
#include "SceneManager.h"
#include "VoxelMesh.h"
#include "VoxelRenderer.h"

std::weak_ptr<VoxelRenderer::SharedProperties> VoxelRenderer::m_sharedWeakPtr;

VoxelRenderer::VoxelRenderer(GraphicsDevice& graphicsDevice)
: m_graphicsDevice(graphicsDevice)
{
    if (!m_sharedWeakPtr.expired())
    {
        m_shared = m_sharedWeakPtr.lock();
    }
    else
    {
        m_shared = std::make_shared<SharedProperties>();
        m_sharedWeakPtr = std::weak_ptr<SharedProperties>(m_shared);

        //
        //  Create the vertex shader and input layout.
        //
        boost::intrusive_ptr<ID3D10Blob> bytecode, errors;
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/voxel_mesh_vs.hlsl",//  pSrcFile
                                        NULL,                               //  pDefines
                                        NULL,                               //  pInclude
                                        "main",                             //  pFunctionName
                                        "vs_4_0",                           //  pProfile
                                        0,                                  //  Flags1
                                        0,                                  //  Flags2
                                        NULL,                               //  pPump
                                        AttachPtr(bytecode),                //  ppShader
                                        AttachPtr(errors),                  //  ppErrorMsgs
                                        NULL));                             //  pHResult
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateVertexShader(
                                        bytecode->GetBufferPointer(),
                                        bytecode->GetBufferSize(),
                                        NULL,
                                        AttachPtr(m_shared->vertexShader)));
        
        D3D11_INPUT_ELEMENT_DESC inputElements[] =
        {
            {
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R32G32B32_FLOAT,                                //  Format
                0,                                                          //  InputSlot
                0,                                                          //  AlignedByteOffset
                D3D11_INPUT_PER_VERTEX_DATA,                                //  InputSlotClass
                0                                                           //  InstanceDataStepRate
            },
            {
                "NORMAL",                                                   //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R32_UINT,                                       //  Format
                0,                                                          //  InputSlot
                12,                                                         //  AlignedByteOffset
                D3D11_INPUT_PER_VERTEX_DATA,                                //  InputSlotClass
                0                                                           //  InstanceDataStepRate
            },
            {
                "TEXCOORD",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R32G32_UINT,                                    //  Format
                0,                                                          //  InputSlot
                16,                                                         //  AlignedByteOffset
                D3D11_INPUT_PER_VERTEX_DATA,                                //  InputSlotClass
                0                                                           //  InstanceDataStepRate
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateInputLayout(
                                        inputElements,
                                        sizeof(inputElements) / sizeof(inputElements[0]),
                                        bytecode->GetBufferPointer(),
                                        bytecode->GetBufferSize(),
                                        AttachPtr(m_shared->inputLayout)));

        //
        //  Create the pixel shader.
        //
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/voxel_mesh_ps.hlsl",//  pSrcFile
                                        NULL,                               //  pDefines
                                        NULL,                               //  pInclude
                                        "main",                             //  pFunctionName
                                        "ps_4_0",                           //  pProfile
                                        0,                                  //  Flags1
                                        0,                                  //  Flags2
                                        NULL,                               //  pPump
                                        AttachPtr(bytecode),                //  ppShader
                                        AttachPtr(errors),                  //  ppErrorMsgs
                                        NULL));                             //  pHResult
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreatePixelShader(
                                        bytecode->GetBufferPointer(),
                                        bytecode->GetBufferSize(),
                                        NULL,
                                        AttachPtr(m_shared->pixelShader)));
    

        //
        //  Create the state objects.
        //
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = 
        {
            TRUE,                                                           //  DepthEnable
            D3D11_DEPTH_WRITE_MASK_ALL,                                     //  DepthWriteMask
            D3D11_COMPARISON_LESS,                                          //  ComparisonFunc
            TRUE,                                                           //  StencilEnable
            0,                                                              //  StencilReadMask
            0xFF,                                                           //  StencilWriteMask
            {                                                               //  FrontFace
                D3D11_STENCIL_OP_KEEP,                                      //      StencilFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilDepthFailOp
                D3D11_STENCIL_OP_INCR_SAT,                                  //      StencilPassOp
                D3D11_COMPARISON_ALWAYS,                                    //      StencilFunc
            },
            {                                                               //  BackFace
                D3D11_STENCIL_OP_KEEP,                                      //      StencilFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilDepthFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilPassOp
                D3D11_COMPARISON_NEVER,                                     //      StencilFunc
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateDepthStencilState(
                                        &depthStencilDesc,
                                        AttachPtr(m_shared->depthStencilState)));

        D3D11_DEPTH_STENCIL_DESC transparentDepthStencilDesc = 
        {
            TRUE,                                                           //  DepthEnable
            D3D11_DEPTH_WRITE_MASK_ZERO,                                    //  DepthWriteMask
            D3D11_COMPARISON_LESS,                                          //  ComparisonFunc
            FALSE,                                                          //  StencilEnable
            0,                                                              //  StencilReadMask
            0,                                                              //  StencilWriteMask
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
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateDepthStencilState(
                                        &depthStencilDesc,
                                        AttachPtr(m_shared->transparentDepthStencilState)));

        D3D11_DEPTH_STENCIL_DESC fillGapsDepthStencilDesc =
        {
            TRUE,                                                           //  DepthEnable
            D3D11_DEPTH_WRITE_MASK_ALL,                                     //  DepthWriteMask
            D3D11_COMPARISON_LESS,                                          //  ComparisonFunc
            TRUE,                                                           //  StencilEnable
            0xFF,                                                           //  StencilReadMask
            0,                                                              //  StencilWriteMask
            {                                                               //  FrontFace
                D3D11_STENCIL_OP_KEEP,                                      //      StencilFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilDepthFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilPassOp
                D3D11_COMPARISON_EQUAL                                      //      StencilFunc
            },
            {                                                               //  BackFace
                D3D11_STENCIL_OP_KEEP,                                      //      StencilFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilDepthFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilPassOp
                D3D11_COMPARISON_NEVER                                      //      StencilFunc
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateDepthStencilState(&fillGapsDepthStencilDesc,
                                                                         AttachPtr(m_shared->fillGapsDepthStencilState)));


        D3D11_BLEND_DESC blendDesc =
        {
            FALSE,                                                           //  AlphaToCoverageEnable
            FALSE,                                                          //  IndependentBlendEnable
            {{                                                              //  RenderTarget[0]
                TRUE,                                                       //      BlendEnable
                D3D11_BLEND_SRC_ALPHA,                                      //      SrcBlend
                D3D11_BLEND_INV_SRC_ALPHA,                                  //      DestBlend
                D3D11_BLEND_OP_ADD,                                         //      BlendOp
                D3D11_BLEND_ZERO,                                           //      SrcBlendAlpha
                D3D11_BLEND_ZERO,                                           //      DestBlendAlpha
                D3D11_BLEND_OP_ADD,                                         //      BlendOpAlpha
                D3D11_COLOR_WRITE_ENABLE_ALL                                //      RenderTargetWriteMask
            }}
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBlendState(&blendDesc,
                                                                  AttachPtr(m_shared->blendState)));
        //
        //  Load the textures.
        //
        const char* paths[] =
        {
            "assets/textures/rock.jpg",
            "assets/textures/dark_grass.png",
            "assets/textures/Grass_1.png",
            "assets/textures/dirt.jpg",
            "assets/textures/pjrock21.jpg",
            "assets/textures/darkrock.png",
            "assets/textures/sand.png"
        };
        for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++)
        {
            D3DCHECK(D3DX11CreateShaderResourceViewFromFileA(&m_graphicsDevice.GetD3DDevice(),
                                                             paths[i],
                                                             NULL,
                                                             NULL,
                                                             AttachPtr(m_shared->textureViews[i]),
                                                             NULL));
        }

        //
        //  Create the sampler state.
        //
        D3D11_SAMPLER_DESC samplerDesc =
        {
            D3D11_FILTER_MIN_MAG_MIP_LINEAR,                                //  Filter
            D3D11_TEXTURE_ADDRESS_WRAP,                                     //  AddressU
            D3D11_TEXTURE_ADDRESS_WRAP,                                     //  AddressV
            D3D11_TEXTURE_ADDRESS_WRAP,                                     //  AddressW
            0.0f,                                                           //  MaxLODBias
            0,                                                              //  MaxAnisotropy
            D3D11_COMPARISON_ALWAYS,                                        //  ComparisonFunc
            {0, 0, 0, 0},                                                   //  BorderColor
            -FLT_MAX,                                                       //  MinLOD
            FLT_MAX                                                         //  MaxLOD
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateSamplerState(&samplerDesc,
                                                                    AttachPtr(m_shared->samplerState)));
    }

    //
    //  Create the constant buffer.
    //
    D3D11_BUFFER_DESC constantBufferDesc =
    {
        sizeof(ShaderConstants),                                            //  ByteWidth
        D3D11_USAGE_DYNAMIC,                                                //  Usage
        D3D11_BIND_CONSTANT_BUFFER,                                         //  BindFlags
        D3D11_CPU_ACCESS_WRITE,                                             //  CPUAccessFlags
        0,                                                                  //  MiscFlags
        0                                                                   //  StructureByteSize
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(
                                        &constantBufferDesc,
                                        NULL,
                                        AttachPtr(m_constantBuffer)));
}

VoxelRenderer::~VoxelRenderer()
{
}

void VoxelRenderer::Draw(const VoxelMesh& geometry, float3 position)
{
    RenderOp op =
    {
        &geometry,
        position,
        0,
        1.0f,
        true
    };
    m_renderOps.push_back(op);
}

void VoxelRenderer::DrawGapFiller(const VoxelMesh& geometry, float3 position)
{
    RenderOp op =
    {
        &geometry,
        position,
        0,
        1.0f,
        true
    };
    m_gapFillerRenderOps.push_back(op);    
}

void VoxelRenderer::DrawTransparent(const VoxelMesh& geometry, float3 position, float alpha)
{
    RenderOp op =
    {
        &geometry,
        position,
        0,
        alpha,
        false
    };
    m_transparentRenderOps.push_back(op);    
}

void VoxelRenderer::Flush(RenderContext& renderContext,
                          const SceneConstants& sceneConstants)
{
    //  Set up global shader constants
    m_constants.projectionViewMatrix = sceneConstants.viewMatrix *
                                       sceneConstants.projectionMatrix;
    m_constants.clipPlane = sceneConstants.clipPlane;

    //  Calculate camera distances for all queued render ops
    float3 cameraPos = sceneConstants.cameraPos;
    for (size_t i = 0; i < m_renderOps.size(); ++i)
    {
        float3 diff = m_renderOps[i].position - cameraPos;
        m_renderOps[i].distance2 = Dot(diff, diff);
    }
    for (size_t i = 0; i < m_gapFillerRenderOps.size(); ++i)
    {
        float3 diff = m_gapFillerRenderOps[i].position - cameraPos;
        m_gapFillerRenderOps[i].distance2 = Dot(diff, diff);
    }
    for (size_t i = 0; i < m_transparentRenderOps.size(); ++i)
    {
        float3 diff = m_transparentRenderOps[i].position - cameraPos;
        m_transparentRenderOps[i].distance2 = Dot(diff, diff);
    }
    
    //  Sort by camera distance (front to back for opaque render ops,
    //  back to front for transparent)
    auto transparentSorter = [](const RenderOp& lhs, const RenderOp& rhs)
    {
        return lhs.distance2 > rhs.distance2;
    };
    auto opaqueSorter = [](const RenderOp& lhs, const RenderOp& rhs)
    {
        return lhs.distance2 < rhs.distance2;
    };
    std::sort(m_renderOps.begin(), m_renderOps.end(), opaqueSorter);
    std::sort(m_gapFillerRenderOps.begin(), m_gapFillerRenderOps.end(), opaqueSorter);
    std::sort(m_transparentRenderOps.begin(), m_transparentRenderOps.end(), transparentSorter);

    //  Finally time to draw them
    renderContext.PushDepthStencilState(m_shared->depthStencilState);
    for (size_t i = 0; i < m_renderOps.size(); i++)
    {
        Draw(m_renderOps[i]);
    }
    m_renderOps.clear();
    renderContext.PopDepthStencilState();

    renderContext.PushDepthStencilState(m_shared->fillGapsDepthStencilState);
    for (size_t i = 0; i < m_gapFillerRenderOps.size(); ++i)
    {
        Draw(m_gapFillerRenderOps[i]);
    }
    m_gapFillerRenderOps.clear();
    renderContext.PopDepthStencilState();

    renderContext.PushDepthStencilState(m_shared->transparentDepthStencilState);
    renderContext.PushBlendState(m_shared->blendState);
    for (size_t i = 0; i < m_transparentRenderOps.size(); i++)
    {
        Draw(m_transparentRenderOps[i]);
    }
    m_transparentRenderOps.clear();
    renderContext.PopBlendState();
    renderContext.PopDepthStencilState();
}

void VoxelRenderer::Draw(const RenderOp& op)
{
    ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();

    //
    //  Update the constant buffer.
    //
    m_constants.worldMatrix = float4x4::Translation(op.position);
    m_constants.alpha[0] = op.alpha;

    D3D11_MAPPED_SUBRESOURCE map;
    D3DCHECK(m_graphicsDevice.GetD3DContext().Map(m_constantBuffer.get(),   //  pResource
                                                 0,                         //  Subresource
                                                 D3D11_MAP_WRITE_DISCARD,   //  MapType
                                                 0,                         //  MapFlags
                                                 &map));                    //  pMappedResource
    memcpy(map.pData, &m_constants, sizeof(m_constants));
    m_graphicsDevice.GetD3DContext().Unmap(m_constantBuffer.get(), 0);

    //
    //  Execute the render op.
    //
    ID3D11Buffer* vertexBufferPtr = op.geometry->GetVertexBuffer();
    size_t offset = 0,
           stride = sizeof(VoxelMesh::Vertex);
    ID3D11ShaderResourceView* shaderResourceViewPtrs[] =
    {
        m_shared->textureViews[0].get(),
        m_shared->textureViews[1].get(),
        m_shared->textureViews[2].get(),
        m_shared->textureViews[3].get(),
        m_shared->textureViews[4].get(),
        m_shared->textureViews[5].get(),
        m_shared->textureViews[6].get()
    };
    ID3D11SamplerState* samplerPtr = m_shared->samplerState.get();

    context.IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
    context.IASetIndexBuffer(op.geometry->GetIndexBuffer(), DXGI_FORMAT_R16_UINT, 0);
    context.IASetInputLayout(m_shared->inputLayout.get());
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11Buffer* constantBufferPtr = m_constantBuffer.get();
    context.VSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.VSSetShader(m_shared->vertexShader.get(), NULL, 0);

    context.GSSetShader(NULL, NULL, 0);

    context.PSSetShader(m_shared->pixelShader.get(), NULL, 0);
    context.PSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.PSSetShaderResources(0, 7, shaderResourceViewPtrs);
    context.PSSetSamplers(0, 1, &samplerPtr);

    context.DrawIndexed(op.geometry->GetIndexCount(), 0, 0);  

}
