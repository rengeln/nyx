///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Camera.h"
#include "GraphicsDevice.h"
#include "Noise.h"
#include "WaterRenderer.h"

std::weak_ptr<WaterRenderer::SharedProperties> WaterRenderer::m_sharedWeakPtr;

WaterRenderer::WaterRenderer(GraphicsDevice& graphicsDevice)
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
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/water_vs.hlsl",     //  pSrcFile
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
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/water_ps.hlsl",     //  pSrcFile
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
        //  Create the renderer state objects.
        //
        D3D11_RASTERIZER_DESC rasterizerDesc =
        {
            D3D11_FILL_SOLID,                                               //  FillMode
            D3D11_CULL_BACK,                                                //  CullMode
            TRUE,                                                           //  FrontCounterClockwise
            0.0f,                                                           //  DepthBias
            0.0f,                                                           //  DepthBiasClamp
            0.0f,                                                           //  SlopeScaledDepthBias
            FALSE,                                                          //  DepthClipEnable
            FALSE,                                                          //  ScissorEnable
            FALSE,                                                          //  MultisampleEnable
            FALSE                                                           //  AntialiasedLineEnable
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateRasterizerState(
                                        &rasterizerDesc,
                                        AttachPtr(m_shared->rasterizerState)));

        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = 
        {
            TRUE,                                                           //  DepthEnable
            D3D11_DEPTH_WRITE_MASK_ALL,                                     //  DepthWriteMask
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
                                        AttachPtr(m_shared->depthStencilState)));

        //
        //  Create the blend state object.
        //
        D3D11_BLEND_DESC blendDesc =
        {
            FALSE,                                                          //  AlphaToCoverageEnable
            FALSE,                                                          //  IndependentBlendEnable
            {{                                                              //  RenderTarget[0]
                FALSE,                                                       //      BlendEnable
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
        //  Create the vertex and index buffers.
        //
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
        GenerateRadialGrid(vertices, indices);
        m_shared->indexCount = indices.size();

        D3D11_SUBRESOURCE_DATA vertexBufferData =
        {
            vertices.data(),                                                //  pSysMem
            0,                                                              //  SysMemPitch
            0                                                               //  SysMemSlicePitch
        };
        D3D11_BUFFER_DESC vertexBufferDesc =
        {
            vertices.size() * sizeof(Vertex),                               //  ByteWidth
            D3D11_USAGE_IMMUTABLE,                                          //  Usage
            D3D11_BIND_VERTEX_BUFFER,                                       //  BindFlags
            0,                                                              //  CPUAccessFlags
            0,                                                              //  MiscFlags
            0                                                               //  StructureByteStride
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(&vertexBufferDesc,
                                                              &vertexBufferData,
                                                              AttachPtr(m_shared->vertexBuffer)));

        D3D11_SUBRESOURCE_DATA indexBufferData =
        {
            indices.data(),                                                 //  pSysMem
            0,                                                              //  SysMemPitch
            0                                                               //  SysMemSlicePitch
        };
        D3D11_BUFFER_DESC indexBufferDesc =
        {
            indices.size() * sizeof(uint16_t),                              //  ByteWidth
            D3D11_USAGE_IMMUTABLE,                                          //  Usage
            D3D11_BIND_INDEX_BUFFER,                                        //  BindFlags
            0,                                                              //  CPUAccessFlags
            0,                                                              //  MiscFlags
            0                                                               //  StructureByteStride
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(&indexBufferDesc,
                                                              &indexBufferData,
                                                              AttachPtr(m_shared->indexBuffer)));

        //
        //  Create the noise texture.
        //
        std::vector<float> noiseData(256 * 256);
        Noise n;
        n.Fill(noiseData.data(), 256, 256, 1, 0.1f, 0.1f, 1.0f);

        D3D11_SUBRESOURCE_DATA noiseTextureData =
        {
            noiseData.data(),                                               //  pSysMem
            sizeof(float) * 256,                                            //  SysMemPitch
            0,                                                              //  SysMemSlicePitch
        };
        D3D11_TEXTURE2D_DESC noiseTextureDesc =
        {
            256,                                                            //  Width
            256,                                                            //  Height
            1,                                                              //  MipLevels
            1,                                                              //  ArraySize
            DXGI_FORMAT_R32_FLOAT,                                          //  Format
            {                                                               //  SampleDesc
                1,                                                          //      Count
                0                                                           //      Quality
            },
            D3D11_USAGE_IMMUTABLE,                                          //  Usage
            D3D11_BIND_SHADER_RESOURCE,                                     //  BindFlags
            0,                                                              //  CPUAccessFlags
            0                                                               //  MiscFlags
        };
        boost::intrusive_ptr<ID3D11Texture2D> noiseTexture;
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateTexture2D(&noiseTextureDesc,
                                                                 &noiseTextureData,
                                                                 AttachPtr(noiseTexture)));

        D3D11_SHADER_RESOURCE_VIEW_DESC noiseSRVDesc =
        {
            DXGI_FORMAT_R32_FLOAT,                                          //  Format
            D3D11_SRV_DIMENSION_TEXTURE2D,                                  //  ViewDimension
            {                                                               //  Texture2D
                0,                                                          //      MostDetailedMip
                -1                                                          //      MipLevels
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateShaderResourceView(noiseTexture.get(),
                                                                          &noiseSRVDesc,
                                                                          AttachPtr(m_shared->noiseTextureView)));

        //
        //  Create the sampler for the noise texture.
        //
        D3D11_SAMPLER_DESC noiseSamplerDesc =
        {
            D3D11_FILTER_MIN_MAG_MIP_LINEAR,                                //  Filter
            D3D11_TEXTURE_ADDRESS_MIRROR,                                     //  AddressU
            D3D11_TEXTURE_ADDRESS_MIRROR,                                     //  AddressV
            D3D11_TEXTURE_ADDRESS_MIRROR,                                     //  AddressW
            0.0f,                                                           //  MaxLODBias
            0,                                                              //  MaxAnisotropy
            D3D11_COMPARISON_ALWAYS,                                        //  ComparisonFunc
            {0, 0, 0, 0},                                                   //  BorderColor
            -FLT_MAX,                                                       //  MinLOD
            FLT_MAX                                                         //  MaxLOD
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateSamplerState(&noiseSamplerDesc,
                                                                    AttachPtr(m_shared->noiseSampler)));    
                                                                 
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
    m_time = 0;
}

WaterRenderer::~WaterRenderer()
{
}

void WaterRenderer::GenerateRadialGrid(std::vector<Vertex>& vertices,
                                       std::vector<uint16_t>& indices)
{
    const size_t M = 256, N = 64;
    const float MinRadius = 1.0f, DeltaRadius = 0.05f;

    vertices.reserve(1 + (M * N));

    Vertex c;
    c.xyz = float3(0, 1000.0f, 0);
    vertices.push_back(c);

    //  indices for the inner ring
    for (size_t j = 0; j < M; ++j)
    {
        indices.push_back(0);
        indices.push_back(1 + j);
    }
    
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < M; ++j)
        {
            float r = MinRadius + (DeltaRadius * (i * i * i));
            Vertex v;
            v.xyz = float3(r * cos(2.0f * XM_PI * j / M),
                           1000.0f,
                           r * sin(2.0f * XM_PI * j / M));
            vertices.push_back(v);

            if (i < N - 1)
            {
                indices.push_back(1 + (i * M) + j);
                indices.push_back(1 + ((i + 1) * M) + j);
            }
        }
    }
}


void WaterRenderer::SetCamera(const Camera& camera)
{
    m_shaderConstants.viewMatrix = camera.GetViewMatrix();
    m_shaderConstants.projectionMatrix = camera.GetProjectionMatrix();
    m_shaderConstants.cameraPosition = camera.GetPosition();
}

void WaterRenderer::Draw()
{
    ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();

    m_time += 0.1f;
    m_shaderConstants.offset.x = m_time * 1.35f;
    //m_shaderConstants.offset = float4::Replicate(0);
    
    //
    //  Update the constant buffer.
    //
    D3D11_MAPPED_SUBRESOURCE map;
    D3DCHECK(m_graphicsDevice.GetD3DContext().Map(m_constantBuffer.get(),   //  pResource
                                                 0,                         //  Subresource
                                                 D3D11_MAP_WRITE_DISCARD,   //  MapType
                                                 0,                         //  MapFlags
                                                 &map));                    //  pMappedResource
    memcpy(map.pData, &m_shaderConstants, sizeof(m_shaderConstants));
    m_graphicsDevice.GetD3DContext().Unmap(m_constantBuffer.get(), 0);

    //
    //  Execute the render op.
    //
    ID3D11Buffer* vertexBufferPtr = m_shared->vertexBuffer.get();
    size_t offset = 0,
           stride = sizeof(Vertex);
    ID3D11ShaderResourceView* srvs[] =
    {
        m_shared->noiseTextureView.get()
    };
    ID3D11SamplerState* samplers[] =
    {
        m_shared->noiseSampler.get()
    };

    context.IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
    context.IASetIndexBuffer(m_shared->indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
    context.IASetInputLayout(m_shared->inputLayout.get());
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    
    ID3D11Buffer* constantBufferPtr = m_constantBuffer.get();
    context.VSSetShaderResources(0, sizeof(srvs) / sizeof(srvs[0]), srvs);
    context.VSSetSamplers(0, sizeof(samplers) / sizeof(samplers[0]), samplers);
    context.VSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.VSSetShader(m_shared->vertexShader.get(), NULL, 0);
    
    context.GSSetShader(NULL, NULL, 0);

    context.PSSetShader(m_shared->pixelShader.get(), NULL, 0);
    context.PSSetShaderResources(0, sizeof(srvs) / sizeof(srvs[0]), srvs);
    context.PSSetSamplers(0, sizeof(samplers) / sizeof(samplers[0]), samplers);
    
    context.RSSetState(m_shared->rasterizerState.get());

    const float blendFactor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    context.OMSetBlendState(m_shared->blendState.get(), NULL, 0xFFFFFFFF);
    context.OMSetDepthStencilState(m_shared->depthStencilState.get(), 0);
    
    context.DrawIndexed(m_shared->indexCount, 0, 0);

    context.OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
}
