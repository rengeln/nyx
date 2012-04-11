///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Camera.h"
#include "GraphicsDevice.h"
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
    }
}

WaterRenderer::~WaterRenderer()
{
}

void WaterRenderer::GenerateRadialGrid(std::vector<Vertex>& vertices,
                                       std::vector<uint16_t>& indices)
{
    const size_t M = 12, N = 6;
    const float MinRadius = 1.0f, DeltaRadius = 2.0f;

    vertices.reserve(M * N);
    //  First generate the vertices
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < M; ++j)
        {
            float r = MinRadius + (DeltaRadius * (i * i * i * i));
            Vertex v;
            v.xyz = float3(r * cos(2.0f * XM_PI * j / M),
                           0.0f,
                           r * sin(2.0f * XM_PI * j / M));
            vertices.push_back(v);
        }
    }
    //  Next generate the indices

}
