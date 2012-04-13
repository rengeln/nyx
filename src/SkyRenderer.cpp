///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Camera.h"
#include "GraphicsDevice.h"
#include "SkyRenderer.h"

const SkyRenderer::Vertex SkyRenderer::SkyboxVertices[24] =
{
    //  Left face
    {{-1.0f,  1.0f,  1.0f}, {0.2500f, 0.335f}},
    {{-1.0f, -1.0f,  1.0f}, {0.2500f, 0.665f}},
    {{-1.0f, -1.0f, -1.0f}, {0.0000f, 0.665f}},
    {{-1.0f,  1.0f, -1.0f}, {0.0000f, 0.335f}},

    //  Right face
    {{ 1.0f,  1.0f,  1.0f}, {0.5000f, 0.334f}},
    {{ 1.0f, -1.0f,  1.0f}, {0.5000f, 0.665f}},
    {{ 1.0f, -1.0f, -1.0f}, {0.7500f, 0.665f}},
    {{ 1.0f,  1.0f, -1.0f}, {0.7500f, 0.334f}},

    //  Front face
    {{-1.0f, -1.0f,  1.0f}, {0.2500f, 0.6666f}},
    {{ 1.0f, -1.0f,  1.0f}, {0.5000f, 0.6666f}},
    {{ 1.0f,  1.0f,  1.0f}, {0.5000f, 0.3333f}},
    {{-1.0f,  1.0f,  1.0f}, {0.2500f, 0.3333f}},

    //  Back face
    {{-1.0f, -1.0f, -1.0f}, {1.000f, 0.665f}},
    {{ 1.0f, -1.0f, -1.0f}, {0.7500f, 0.665f}},
    {{ 1.0f,  1.0f, -1.0f}, {0.7500f, 0.335f}},
    {{-1.0f,  1.0f, -1.0f}, {1.000f, 0.335f}},

    //  Top face
    {{-1.0f,  1.0f, -1.0f}, {0.251f, 0.0000f}},
    {{-1.0f,  1.0f,  1.0f}, {0.251f, 0.3333f}},
    {{ 1.0f,  1.0f,  1.0f}, {0.499f, 0.3333f}},
    {{ 1.0f,  1.0f, -1.0f}, {0.499f, 0.0000f}},

    //  Bottom face
    {{-1.0f, -1.0f, -1.0f}, {0.251f, 0.6666f}},
    {{-1.0f, -1.0f,  1.0f}, {0.251f, 1.0000f}},
    {{ 1.0f, -1.0f,  1.0f}, {0.499f, 1.0000f}},
    {{ 1.0f, -1.0f, -1.0f}, {0.499f, 0.6666f}},
};

const uint16_t SkyRenderer::SkyboxIndices[36] = 
{
    //  Left face
    0, 1, 2,
    2, 3, 0,

    //  Right face
    4, 5, 6,
    6, 7, 4,

    //  Front face
    8, 9, 10,
    10, 11, 8,

    //  Back face
    12, 13, 14,
    14, 15, 12,

    //  Top face
    16, 17, 18,
    18, 19, 16,

    //  Bottom face
    20, 21, 22,
    22, 23, 20
};

std::weak_ptr<SkyRenderer::SharedProperties> SkyRenderer::m_sharedWeakPtr;

SkyRenderer::SkyRenderer(GraphicsDevice& graphicsDevice)
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
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/skybox_vs.hlsl",    //  pSrcFile
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
                "TEXCOORD",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R32G32_FLOAT,                                   //  Format
                0,                                                          //  InputSlot
                12,                                                         //  AlignedByteOffset
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
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/skybox_ps.hlsl",    //  pSrcFile
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
            D3D11_CULL_NONE,                                                //  CullMode
            FALSE,                                                          //  FrontCounterClockwise
            0,                                                              //  DepthBias
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
            D3D11_DEPTH_WRITE_MASK_ZERO,                                    //  DepthWriteMask
            D3D11_COMPARISON_LESS_EQUAL,                                    //  ComparisonFunc
            FALSE,                                                          //  StencilEnable
            0,                                                              //  StencilReadMask
            0,                                                              //  StencilWriteMask
            {                                                               //  FrontFace
                D3D11_STENCIL_OP_KEEP,                                      //      StencilFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilDepthFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilPassOp
                D3D11_COMPARISON_NEVER,                                     //      StencilFunc
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
        //  Create the vertex and index buffers.
        //
        D3D11_SUBRESOURCE_DATA vertexBufferData =
        {
            SkyboxVertices,                                                 //  pSysMem
            0,                                                              //  SysMemPitch
            0                                                               //  SysMemSlicePitch
        };
        D3D11_BUFFER_DESC vertexBufferDesc =
        {
            sizeof(SkyboxVertices),                                         //  ByteWidth
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
            SkyboxIndices,                                                  //  pSysMem
            0,                                                              //  SysMemPitch
            0                                                               //  SysMemSlicePitch
        };
        D3D11_BUFFER_DESC indexBufferDesc =
        {
            sizeof(SkyboxIndices),                                          //  ByteWidth
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
        //  Load the sky texture.
        //
        D3DCHECK(D3DX11CreateShaderResourceViewFromFileA(&m_graphicsDevice.GetD3DDevice(),
                                                         "assets/textures/sky.jpg",
                                                         NULL,
                                                         NULL,
                                                         AttachPtr(m_shared->skyTextureView),
                                                         NULL));

        //
        //  Create the sampler state.
        //
        D3D11_SAMPLER_DESC samplerDesc =
        {
            D3D11_FILTER_MIN_MAG_MIP_LINEAR,                                //  Filter
            D3D11_TEXTURE_ADDRESS_CLAMP,                                     //  AddressU
            D3D11_TEXTURE_ADDRESS_CLAMP,                                     //  AddressV
            D3D11_TEXTURE_ADDRESS_CLAMP,                                     //  AddressW
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

SkyRenderer::~SkyRenderer()
{
}

void SkyRenderer::SetCamera(const Camera& camera)
{
    m_constants.viewMatrix = camera.GetViewMatrix();
    m_constants.projectionMatrix = camera.GetProjectionMatrix();
}

void SkyRenderer::Draw()
{
    ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();

    //
    //  Update the constant buffer.
    //
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
    ID3D11Buffer* vertexBufferPtr = m_shared->vertexBuffer.get();
    size_t offset = 0,
           stride = sizeof(Vertex);
    ID3D11ShaderResourceView* shaderResourceViewPtrs[] =
    {
        m_shared->skyTextureView.get()
    };
    ID3D11SamplerState* samplerPtr = m_shared->samplerState.get();

    context.IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
    context.IASetIndexBuffer(m_shared->indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
    context.IASetInputLayout(m_shared->inputLayout.get());
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11Buffer* constantBufferPtr = m_constantBuffer.get();
    context.VSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.VSSetShader(m_shared->vertexShader.get(), NULL, 0);

    context.GSSetShader(NULL, NULL, 0);

    context.PSSetShader(m_shared->pixelShader.get(), NULL, 0);
    context.PSSetShaderResources(0, 1, shaderResourceViewPtrs);
    context.PSSetSamplers(0, 1, &samplerPtr);

    context.RSSetState(m_shared->rasterizerState.get());
    context.OMSetDepthStencilState(m_shared->depthStencilState.get(), 0);

    context.DrawIndexed(36, 0, 0);
}
