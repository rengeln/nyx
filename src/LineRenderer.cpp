///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Camera.h"
#include "GraphicsDevice.h"
#include "RenderContext.h"
#include "LineRenderer.h"
#include "SceneManager.h"

std::weak_ptr<LineRenderer::SharedProperties> LineRenderer::m_sharedWeakPtr;

LineRenderer::LineRenderer(GraphicsDevice& graphicsDevice)
: m_graphicsDevice(graphicsDevice),
  m_vertexBufferSize(0)
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
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/line_vs.hlsl",      //  pSrcFile
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
                "COLOR",                                                    //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R8G8B8A8_UNORM,                                 //  Format
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
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/line_ps.hlsl",      //  pSrcFile
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
            D3D11_DEPTH_WRITE_MASK_ALL,                                     //  DepthWriteMask
            D3D11_COMPARISON_LESS,                                          //  ComparisonFunc
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
                                            AttachPtr(m_shared->constantBuffer)));
    }
}

LineRenderer::~LineRenderer()
{
}

void LineRenderer::DrawLine(float3 p0, float3 p1, float4 color)
{
    ubyte4 packedColor = ubyte4(color * 255.0f);
    Vertex v0 = {p0, packedColor};
    Vertex v1 = {p1, packedColor};
    m_vertices.push_back(v0);
    m_vertices.push_back(v1);
}

void LineRenderer::DrawBox(box3f box, float4 color)
{
    ubyte4 packedColor = ubyte4(color * 255.0f);
    Vertex v[] =
    {
        {float3(box.first.x, box.first.y, box.first.z), packedColor}, 
        {float3(box.second.x, box.first.y, box.first.z), packedColor},
        {float3(box.first.x, box.second.y, box.first.z), packedColor},
        {float3(box.second.x, box.second.y, box.first.z), packedColor},
        {float3(box.first.x, box.first.y, box.second.z), packedColor},
        {float3(box.second.x, box.first.y, box.second.z), packedColor},
        {float3(box.first.x, box.second.y, box.second.z), packedColor},
        {float3(box.second.x, box.second.y, box.second.z), packedColor},
        
        {float3(box.first.x, box.first.y, box.first.z), packedColor},
        {float3(box.first.x, box.first.y, box.second.z), packedColor},
        {float3(box.second.x, box.first.y, box.first.z), packedColor},
        {float3(box.second.x, box.first.y, box.second.z), packedColor},
        {float3(box.first.x, box.second.y, box.first.z), packedColor},
        {float3(box.first.x, box.second.y, box.second.z), packedColor},
        {float3(box.second.x, box.second.y, box.first.z), packedColor},
        {float3(box.second.x, box.second.y, box.second.z), packedColor},

        {float3(box.first.x, box.first.y, box.first.z), packedColor},
        {float3(box.first.x, box.second.y, box.first.z), packedColor},
        {float3(box.second.x, box.first.y, box.first.z), packedColor},
        {float3(box.second.x, box.second.y, box.first.z), packedColor},
        {float3(box.first.x, box.first.y, box.second.z), packedColor},
        {float3(box.first.x, box.second.y, box.second.z), packedColor},
        {float3(box.second.x, box.first.y, box.second.z), packedColor},
        {float3(box.second.x, box.second.y, box.second.z), packedColor}
    };
    for (size_t i = 0; i < sizeof(v) / sizeof(v[0]); i++)
    {
        m_vertices.push_back(v[i]);
    }
}

void LineRenderer::Flush(RenderContext& renderContext,
                         const SceneConstants& sceneConstants)
{
    m_constants.projectionViewMatrix = sceneConstants.projectionMatrix *
                                       sceneConstants.viewMatrix;

    ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();

    if (!m_vertices.size())
    {
        return;
    }

    //
    //  Update the constant buffer.
    //
    D3D11_MAPPED_SUBRESOURCE map;
    D3DCHECK(m_graphicsDevice.GetD3DContext().Map(m_shared->constantBuffer.get(),   //  pResource
                                                 0,                                 //  Subresource
                                                 D3D11_MAP_WRITE_DISCARD,           //  MapType
                                                 0,                                 //  MapFlags
                                                 &map));                            //  pMappedResource
    memcpy(map.pData, &m_constants, sizeof(m_constants));
    m_graphicsDevice.GetD3DContext().Unmap(m_shared->constantBuffer.get(), 0);

    //
    //  Update the vertex buffer.
    //
    if (m_vertexBufferSize < m_vertices.size())
    {
        m_vertexBufferSize = m_vertices.size();
        D3D11_SUBRESOURCE_DATA vertexBufferData =
        {
            m_vertices.data(),                                              //  pSysMem
            0,                                                              //  SysMemPitch
            0                                                               //  SysMemSlicePitch
        };
        D3D11_BUFFER_DESC vertexBufferDesc =
        {
            m_vertexBufferSize * sizeof(Vertex),                            //  ByteWidth
            D3D11_USAGE_DYNAMIC,                                            //  Usage
            D3D11_BIND_VERTEX_BUFFER,                                       //  BindFlags
            D3D11_CPU_ACCESS_WRITE,                                         //  CPUAccessFlags
            0,                                                              //  MiscFlags
            0                                                               //  StructureByteStride
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(&vertexBufferDesc,
                                                              &vertexBufferData,
                                                              AttachPtr(m_shared->vertexBuffer)));
    }
    else
    {
        D3D11_MAPPED_SUBRESOURCE map;
        D3DCHECK(m_graphicsDevice.GetD3DContext().Map(m_shared->vertexBuffer.get(),     //  pResource
                                                     0,                                 //  Subresource
                                                     D3D11_MAP_WRITE_DISCARD,           //  MapType
                                                     0,                                 //  MapFlags
                                                     &map));                            //  pMappedResource
        memcpy(map.pData, m_vertices.data(), m_vertices.size() * sizeof(Vertex));
        m_graphicsDevice.GetD3DContext().Unmap(m_shared->vertexBuffer.get(), 0);
    }

    //
    //  Execute the render op.
    //
    ID3D11Buffer* vertexBufferPtr = m_shared->vertexBuffer.get();
    size_t offset = 0,
           stride = sizeof(Vertex);

    context.IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
    context.IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);
    context.IASetInputLayout(m_shared->inputLayout.get());
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    ID3D11Buffer* constantBufferPtr = m_shared->constantBuffer.get();
    context.VSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.VSSetShader(m_shared->vertexShader.get(), NULL, 0);

    context.GSSetShader(NULL, NULL, 0);

    context.PSSetShader(m_shared->pixelShader.get(), NULL, 0);

    renderContext.PushRasterizerState(m_shared->rasterizerState);
    renderContext.PushDepthStencilState(m_shared->depthStencilState);

    context.Draw(m_vertices.size(), 0);

    renderContext.PopRasterizerState();
    renderContext.PopDepthStencilState();

    m_vertices.clear();
}

