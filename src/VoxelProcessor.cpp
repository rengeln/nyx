///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "GraphicsDevice.h"
#include "Profiler.h"
#include "VoxelMesh.h"
#include "VoxelManager.h"
#include "VoxelProcessor.h"

#include "MarchingCubes.inl"

std::weak_ptr<VoxelProcessor::SharedProperties> VoxelProcessor::m_sharedWeakPtr;

VoxelProcessor::VoxelProcessor(GraphicsDevice& graphicsDevice)
: m_graphicsDevice(graphicsDevice),
  m_verticesAreReady(false),
  m_indicesAreReady(false)
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
        //  Create the gen_voxels vertex shader and input layout.
        //
        boost::intrusive_ptr<ID3D10Blob> bytecode, errors;
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/voxel_generator_vs.hlsl",    //  pSrcFile
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
                                        AttachPtr(m_shared->genVoxelsVS)));
        
        D3D11_INPUT_ELEMENT_DESC inputElements[] =
        {
            {
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R32_UINT,                                       //  Format
                0,                                                          //  InputSlot
                0,                                                          //  AlignedByteOffset
                D3D11_INPUT_PER_VERTEX_DATA,                                //  InputSlotClass
                0                                                           //  InstanceDataStepRate
            },
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateInputLayout(
                                        inputElements,
                                        sizeof(inputElements) / sizeof(inputElements[0]),
                                        bytecode->GetBufferPointer(),
                                        bytecode->GetBufferSize(),
                                        AttachPtr(m_shared->genVoxelsLayout)));  

        //
        //  Create the gen_voxels geometry shader.
        //
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/voxel_generator_gs.hlsl",    //  pSrcFile
                                        NULL,                               //  pDefines
                                        NULL,                               //  pInclude
                                        "main",                             //  pFunctionName
                                        "gs_4_0",                           //  pProfile
                                        0,                                  //  Flags1
                                        0,                                  //  Flags2
                                        NULL,                               //  pPump
                                        AttachPtr(bytecode),                //  ppShader
                                        AttachPtr(errors),                  //  ppErrorMsgs
                                        NULL));                             //  pHResult
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateGeometryShader(
                                        bytecode->GetBufferPointer(),
                                        bytecode->GetBufferSize(),
                                        NULL,
                                        AttachPtr(m_shared->genVoxelsGS)));

        //
        //  Create the gen_voxels pixel shader.
        //
        D3DCHECK(D3DX11CompileFromFileA("assets/shaders/voxel_generator_ps.hlsl",    //  pSrcFile
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
                                        AttachPtr(m_shared->genVoxelsPS))); 


        //
        //  Create the list_cells vertex shader and input layout.
        //
        const char* path = "assets/shaders/marching_cubes_list_cells_vs.hlsl";
        D3DCHECK(D3DX11CompileFromFileA(path,                               //  pSrcFile
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
                                        AttachPtr(m_shared->listCellsVS)));

        D3D11_INPUT_ELEMENT_DESC listCellsInputElements[] =
        {
            {
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R32_UINT,                                       //  Format
                0,                                                          //  InputSlot
                0,                                                          //  AlignedByteOffset
                D3D11_INPUT_PER_VERTEX_DATA,                                //  InputSlotClass
                0                                                           //  InstanceDataStepRate
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateInputLayout(
                                        listCellsInputElements,
                                        sizeof(listCellsInputElements) / sizeof(listCellsInputElements[0]),
                                        bytecode->GetBufferPointer(),
                                        bytecode->GetBufferSize(),
                                        AttachPtr(m_shared->listCellsLayout)));

        //
        //  Create the list_cells geometry shader.
        //
        path = "assets/shaders/marching_cubes_list_cells_gs.hlsl";
        D3DCHECK(D3DX11CompileFromFileA(path,                               //  pSrcFile
                                        NULL,                               //  pDefines
                                        NULL,                               //  pInclude
                                        "main",                             //  pFunctionName
                                        "gs_4_0",                           //  pProfile
                                        0,                                  //  Flags1
                                        0,                                  //  Flags2
                                        NULL,                               //  pPump
                                        AttachPtr(bytecode),                //  ppShader
                                        AttachPtr(errors),                  //  ppErrorMsgs
                                        NULL));                             //  pHResult
        
        D3D11_SO_DECLARATION_ENTRY listCellsGSStreamOutDecl[] =
        {
            {
                0,                                                          //  Stream
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                0,                                                          //  StartComponent
                1,                                                          //  ComponentCount
                0                                                           //  OutputSlot
            }
        };
        size_t listCellsGSStreamOutStride = sizeof(uint32_t);
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateGeometryShaderWithStreamOutput(
                                    bytecode->GetBufferPointer(),           //  pShaderBytecode
                                    bytecode->GetBufferSize(),              //  BytecodeLength
                                    listCellsGSStreamOutDecl,           //  pSODeclaration
                                    1,                                      //  NumEntries
                                    &listCellsGSStreamOutStride,        //  pBufferStrides
                                    1,                                      //  NumStrides
                                    0,                                      //  RasterizedStream
                                    NULL,                                   //  pClassLinkage
                                    AttachPtr(m_shared->listCellsGS))); //  ppGeometryShader

        //
        //  Create the list_vertices vertex shader and input layout.
        //
        path = "assets/shaders/marching_cubes_list_vertices_vs.hlsl";
        D3DCHECK(D3DX11CompileFromFileA(path,                               //  pSrcFile
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
                                        AttachPtr(m_shared->listVerticesVS)));

        D3D11_INPUT_ELEMENT_DESC listVerticesInputElements[] =
        {
            {
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R32_UINT,                                       //  Format
                0,                                                          //  InputSlot
                0,                                                          //  AlignedByteOffset
                D3D11_INPUT_PER_VERTEX_DATA,                                //  InputSlotClass
                0                                                           //  InstanceDataStepRate
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateInputLayout(
                                        listVerticesInputElements,
                                        sizeof(listVerticesInputElements) / sizeof(listVerticesInputElements[0]),
                                        bytecode->GetBufferPointer(),
                                        bytecode->GetBufferSize(),
                                        AttachPtr(m_shared->listVerticesLayout)));

        //
        //  Create the list_vertices geometry shader.
        //
        path = "assets/shaders/marching_cubes_list_vertices_gs.hlsl";
        D3DCHECK(D3DX11CompileFromFileA(path,                               //  pSrcFile
                                        NULL,                               //  pDefines
                                        NULL,                               //  pInclude
                                        "main",                             //  pFunctionName
                                        "gs_4_0",                           //  pProfile
                                        0,                                  //  Flags1
                                        0,                                  //  Flags2
                                        NULL,                               //  pPump
                                        AttachPtr(bytecode),                //  ppShader
                                        AttachPtr(errors),                  //  ppErrorMsgs
                                        NULL));                             //  pHResult
        
        D3D11_SO_DECLARATION_ENTRY listVerticesGSStreamOutDecl[] =
        {
            {
                0,                                                          //  Stream
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                0,                                                          //  StartComponent
                1,                                                          //  ComponentCount
                0                                                           //  OutputSlot
            }
        };
        size_t listVerticesGSStreamOutStride = sizeof(uint32_t);
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateGeometryShaderWithStreamOutput(
                                    bytecode->GetBufferPointer(),           //  pShaderBytecode
                                    bytecode->GetBufferSize(),              //  BytecodeLength
                                    listVerticesGSStreamOutDecl,           //  pSODeclaration
                                    1,                                      //  NumEntries
                                    &listVerticesGSStreamOutStride,        //  pBufferStrides
                                    1,                                      //  NumStrides
                                    0,                                      //  RasterizedStream
                                    NULL,                                   //  pClassLinkage
                                    AttachPtr(m_shared->listVerticesGS))); //  ppGeometryShader

        //
        //  Create the gen_vertices vertex shader and layout.
        //
        path = "assets/shaders/marching_cubes_gen_vertices_vs.hlsl";
        D3DCHECK(D3DX11CompileFromFileA(path,                               //  pSrcFile
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
                                        AttachPtr(m_shared->genVerticesVS)));

        D3D11_INPUT_ELEMENT_DESC genVerticesInputElements[] =
        {
            {
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R32_UINT,                                       //  Format
                0,                                                          //  InputSlot
                0,                                                          //  AlignedByteOffset
                D3D11_INPUT_PER_VERTEX_DATA,                                //  InputSlotClass
                0                                                           //  InstanceDataStepRate
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateInputLayout(
                                        genVerticesInputElements,
                                        sizeof(genVerticesInputElements) / sizeof(genVerticesInputElements[0]),
                                        bytecode->GetBufferPointer(),
                                        bytecode->GetBufferSize(),
                                        AttachPtr(m_shared->genVerticesLayout)));

        //
        //  Create the gen_vertices geometry shader.
        //
        path = "assets/shaders/marching_cubes_gen_vertices_gs.hlsl";
        D3DCHECK(D3DX11CompileFromFileA(path,                               //  pSrcFile
                                        NULL,                               //  pDefines
                                        NULL,                               //  pInclude
                                        "main",                             //  pFunctionName
                                        "gs_4_0",                           //  pProfile
                                        0,                                  //  Flags1
                                        0,                                  //  Flags2
                                        NULL,                               //  pPump
                                        AttachPtr(bytecode),                //  ppShader
                                        AttachPtr(errors),                  //  ppErrorMsgs
                                        NULL));                             //  pHResult
        
        D3D11_SO_DECLARATION_ENTRY genVerticesGSStreamOutDecl[] =
        {
            {
                0,                                                          //  Stream
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                0,                                                          //  StartComponent
                3,                                                          //  ComponentCount
                0                                                           //  OutputSlot
            },
            {
                0,                                                          //  Stream
                "NORMAL",                                                   //  SemanticName
                0,                                                          //  SemanticIndex
                0,                                                          //  StartComponent
                1,                                                          //  ComponentCount
                0                                                           //  OutputSlot
            },
            {
                0,                                                          //  Stream
                "TEXCOORD",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                0,                                                          //  StartComponent
                2,                                                          //  ComponentCount
                0                                                           //  OutputSlot
            }
        };
        size_t genVerticesGSStreamOutStride = sizeof(VoxelMesh::Vertex);
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateGeometryShaderWithStreamOutput(
                                    bytecode->GetBufferPointer(),           //  pShaderBytecode
                                    bytecode->GetBufferSize(),              //  BytecodeLength
                                    genVerticesGSStreamOutDecl,             //  pSODeclaration
                                    3,                                      //  NumEntries
                                    &genVerticesGSStreamOutStride,          //  pBufferStrides
                                    1,                                      //  NumStrides
                                    0,                                      //  RasterizedStream
                                    NULL,                                   //  pClassLinkage
                                    AttachPtr(m_shared->genVerticesGS)));   //  ppGeometryShader

        //
        //  Create the splat_vertices vertex shader and layout.
        //
        path = "assets/shaders/marching_cubes_splat_vertices_vs.hlsl";
        D3DCHECK(D3DX11CompileFromFileA(path,                               //  pSrcFile
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
                                        AttachPtr(m_shared->splatVerticesVS)));

        D3D11_INPUT_ELEMENT_DESC splatVerticesInputElements[] =
        {
            {
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R32_UINT,                                       //  Format
                0,                                                          //  InputSlot
                0,                                                          //  AlignedByteOffset
                D3D11_INPUT_PER_VERTEX_DATA,                                //  InputSlotClass
                0                                                           //  InstanceDataStepRate
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateInputLayout(
                                        splatVerticesInputElements,
                                        sizeof(splatVerticesInputElements) / sizeof(splatVerticesInputElements[0]),
                                        bytecode->GetBufferPointer(),
                                        bytecode->GetBufferSize(),
                                        AttachPtr(m_shared->splatVerticesLayout)));

        //
        //  Create the splat_vertices pixel shader.
        //
        path = "assets/shaders/marching_cubes_splat_vertices_ps.hlsl";
        D3DCHECK(D3DX11CompileFromFileA(path,                               //  pSrcFile
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
                                        AttachPtr(m_shared->splatVerticesPS)));


        //
        //  Create the gen_indices vertex shader and input layout.
        //
        path = "assets/shaders/marching_cubes_gen_indices_vs.hlsl";
        D3DCHECK(D3DX11CompileFromFileA(path,                               //  pSrcFile
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
                                        AttachPtr(m_shared->genIndicesVS)));

        D3D11_INPUT_ELEMENT_DESC genIndicesInputElements[] =
        {
            {
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                DXGI_FORMAT_R32_UINT,                                       //  Format
                0,                                                          //  InputSlot
                0,                                                          //  AlignedByteOffset
                D3D11_INPUT_PER_VERTEX_DATA,                                //  InputSlotClass
                0                                                           //  InstanceDataStepRate
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateInputLayout(
                                        genIndicesInputElements,
                                        sizeof(genIndicesInputElements) / sizeof(genIndicesInputElements[0]),
                                        bytecode->GetBufferPointer(),
                                        bytecode->GetBufferSize(),
                                        AttachPtr(m_shared->genIndicesLayout)));

        //
        //  Create the gen_indices geometry shader.
        //
        path = "assets/shaders/marching_cubes_gen_indices_gs.hlsl";
        D3DCHECK(D3DX11CompileFromFileA(path,                               //  pSrcFile
                                        NULL,                               //  pDefines
                                        NULL,                               //  pInclude
                                        "main",                             //  pFunctionName
                                        "gs_4_0",                           //  pProfile
                                        0,                                  //  Flags1
                                        0,                                  //  Flags2
                                        NULL,                               //  pPump
                                        AttachPtr(bytecode),                //  ppShader
                                        AttachPtr(errors),                  //  ppErrorMsgs
                                        NULL));                             //  pHResult
        
        D3D11_SO_DECLARATION_ENTRY genIndicesGSStreamOutDecl[] =
        {
            {
                0,                                                          //  Stream
                "POSITION",                                                 //  SemanticName
                0,                                                          //  SemanticIndex
                0,                                                          //  StartComponent
                1,                                                          //  ComponentCount
                0                                                           //  OutputSlot
            }
        };
        size_t genIndicesGSStreamOutStride = sizeof(uint32_t);
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateGeometryShaderWithStreamOutput(
                                    bytecode->GetBufferPointer(),           //  pShaderBytecode
                                    bytecode->GetBufferSize(),              //  BytecodeLength
                                    genIndicesGSStreamOutDecl,            //  pSODeclaration
                                    1,                                      //  NumEntries
                                    &genIndicesGSStreamOutStride,         //  pBufferStrides
                                    1,                                      //  NumStrides
                                    0,                                      //  RasterizedStream
                                    NULL,                                   //  pClassLinkage
                                    AttachPtr(m_shared->genIndicesGS)));    //  ppGeometryShader


        //
        //  Create the cell buffer used as input for list_cells.
        //
        const size_t MaxCells = CellDimensions.x * CellDimensions.y * CellDimensions.z;
        std::vector<uint32_t> cells(MaxCells);
        for (size_t i = 0; i < MaxCells; i++)
        {
            cells[i] = i;
        }

        D3D11_SUBRESOURCE_DATA cellBufferData =
        {
            cells.data(),                                                   //  pSysMem
            0,                                                              //  SysMemPitch
            0                                                               //  SysMemSlicePitch
        };
        D3D11_BUFFER_DESC cellBufferDesc =
        {
            MaxCells * sizeof(uint32_t),                                    //  ByteWidth
            D3D11_USAGE_IMMUTABLE,                                          //  Usage
            D3D11_BIND_VERTEX_BUFFER,                                       //  BindFlags
            0,                                                              //  CPUAccessFlags
            0,                                                              //  MiscFlags
            0                                                               //  StructureByteStride
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(
                                        &cellBufferDesc,
                                        &cellBufferData,
                                        AttachPtr(m_shared->cellBuffer)));

        //
        //  Create the triangle table buffer and view.
        //
        D3D11_SUBRESOURCE_DATA triTableData =
        {
            TriTable,                                                       //  pSysMem
            0,                                                              //  SysMemPitch
            0                                                               //  SysMemSlicePitch
        };
        D3D11_BUFFER_DESC triTableDesc =
        {
            sizeof(TriTable),                                               //  ByteWidth
            D3D11_USAGE_IMMUTABLE,                                          //  Usage
            D3D11_BIND_SHADER_RESOURCE,                                     //  BindFlags
            0,                                                              //  CPUAccessFlags
            0,                                                              //  MiscFlags
            0                                                               //  StructureByteStride
        };
        boost::intrusive_ptr<ID3D11Buffer> triTableBuffer;
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(
                                        &triTableDesc,
                                        &triTableData,
                                        AttachPtr(triTableBuffer)));

        D3D11_SHADER_RESOURCE_VIEW_DESC triTableViewDesc =
        {
            DXGI_FORMAT_R32_SINT,                                           //  Format
            D3D11_SRV_DIMENSION_BUFFER,                                     //  ViewDimension
            {                                                               //  Buffer
                0,                                                          //      ElementOffset
                sizeof(TriTable) / sizeof(TriTable[0][0]),                  //      ElementWidth
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateShaderResourceView(
                                        triTableBuffer.get(),
                                        &triTableViewDesc,
                                        AttachPtr(m_shared->triTableView)));

        //
        //  Create the sampler state for sampling the density field.
        //
        D3D11_SAMPLER_DESC samplerDesc =
        {
            D3D11_FILTER_MIN_MAG_MIP_LINEAR,                                //  Filter
            D3D11_TEXTURE_ADDRESS_CLAMP,                                    //  AddressU
            D3D11_TEXTURE_ADDRESS_CLAMP,                                    //  AddressV
            D3D11_TEXTURE_ADDRESS_CLAMP,                                    //  AddressW
            0.0f,                                                           //  MipLODBias
            16,                                                             //  MaxAnisotropy
            D3D11_COMPARISON_NEVER,                                         //  ComparisonFunc
            {0, 0, 0, 0},                                                   //  BorderColor
            -FLT_MAX,                                                       //  MinLOD
            FLT_MAX                                                         //  MaxLOD
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateSamplerState(
                                        &samplerDesc,
                                        AttachPtr(m_shared->densitySampler)));

        //
        //  Create the depth/stencil state - when running on D3D10 class hardware
        //  we get runtime errors if we don't explicitly turn off the CellDimensions.z
        //  and stencil tests when doing stream out processing.
        //
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc =
        {
            FALSE,                                                          //  DepthEnable
            D3D11_DEPTH_WRITE_MASK_ZERO,                                    //  DepthWriteMask
            D3D11_COMPARISON_NEVER,                                         //  DepthFunc
            FALSE,                                                          //  StencilEnable
            0,                                                              //  StencilReadMask
            0,                                                              //  StencilWriteMask
            {                                                               //  FrontFace
                D3D11_STENCIL_OP_KEEP,                                      //      StencilFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilDepthFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilPassOp
                D3D11_COMPARISON_NEVER                                      //      StencilFunc
            },
            {                                                               //  BackFace
                D3D11_STENCIL_OP_KEEP,                                      //      StencilFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilDepthFailOp
                D3D11_STENCIL_OP_KEEP,                                      //      StencilPassOp
                D3D11_COMPARISON_NEVER                                      //      StencilFunc
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateDepthStencilState(
                                        &depthStencilDesc,
                                        AttachPtr(m_shared->depthStencilState)));

        //
        //  Create the rasterizer state used for rendering to the vertex map.
        //
        D3D11_RASTERIZER_DESC rasterDesc =
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
                                        &rasterDesc,
                                        AttachPtr(m_shared->rasterizerState)));

        //
        //  Create the permutations buffer for generating noise.
        //
        srand(time(0));
        int shuffledPerms[256];
        memcpy(shuffledPerms, Permutations, sizeof(Permutations));
        for (size_t i = 255; i > 0; --i)
        {
            size_t j = rand() % (i + 1);
            std::swap(shuffledPerms[j], shuffledPerms[i]);
        }
        int permutationTable[512];
        for (uint i = 0; i < 512; i++) {
            permutationTable[i] = shuffledPerms[i & 255];
        }

        D3D11_SUBRESOURCE_DATA permutationBufferData =
        {
            permutationTable,                                               //  pSysMem
            0,                                                              //  SysMemPitch
            0                                                               //  SysMemSlicePitch
        };
        D3D11_BUFFER_DESC permutationBufferDesc =
        {
            sizeof(permutationTable),                                       //  ByteWidth
            D3D11_USAGE_IMMUTABLE,                                          //  Usage
            D3D11_BIND_SHADER_RESOURCE,                                     //  BindFlags
            0,                                                              //  CPUAccessFlags
            0,                                                              //  MiscFlags
            0                                                               //  StructureByteStride
        };
        boost::intrusive_ptr<ID3D11Buffer> permutationsBuffer;
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(&permutationBufferDesc,
                                                              &permutationBufferData,
                                                              AttachPtr(permutationsBuffer)));
    
        D3D11_SHADER_RESOURCE_VIEW_DESC permutationsSRVDesc =
        {
            DXGI_FORMAT_R32_UINT,                                           //  Format
            D3D11_SRV_DIMENSION_BUFFER,                                     //  ViewDimension
            {                                                               //  Buffer
                0,                                                          //      ElementOffset
                512                                                         //      ElementWidth
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateShaderResourceView(
                                                            permutationsBuffer.get(),
                                                            &permutationsSRVDesc,
                                                            AttachPtr(m_shared->permutationsView)));

        //
        //  Create the gradient vectors buffer used for generating noise.
        //
        D3D11_SUBRESOURCE_DATA gradientsBufferData =
        {
            GradientVectors,                                                //  pSysMem
            0,                                                              //  SysMemPitch
            0                                                               //  SysMemSlicePitch
        };
        D3D11_BUFFER_DESC gradientsBufferDesc =
        {
            sizeof(GradientVectors),                                        //  ByteWidth
            D3D11_USAGE_IMMUTABLE,                                          //  Usage
            D3D11_BIND_SHADER_RESOURCE,                                     //  BindFlags
            0,                                                              //  CPUAccessFlags
            0,                                                              //  MiscFlags
            0                                                               //  StructureByteStride
        };
        boost::intrusive_ptr<ID3D11Buffer> gradientsBuffer;
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(&gradientsBufferDesc,
                                                              &gradientsBufferData,
                                                              AttachPtr(gradientsBuffer)));

        D3D11_SHADER_RESOURCE_VIEW_DESC gradientsSRVDesc =
        {
            DXGI_FORMAT_R32G32B32_FLOAT,                                    //  Format
            D3D11_SRV_DIMENSION_BUFFER,                                     //  ViewDimension
            {                                                               //  Buffer
                0,                                                          //      ElementOffset
                12                                                          //      ElementWidth
            }
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateShaderResourceView(
                                                            gradientsBuffer.get(),
                                                            &gradientsSRVDesc,
                                                            AttachPtr(m_shared->gradientsView)));
    }

    //
    //  Create the cell marker buffer used as output for list_cells and input for
    //  list_vertices and gen_indices.
    //
    const size_t MaxCells = CellDimensions.x * CellDimensions.y * CellDimensions.z;
    D3D11_BUFFER_DESC cellMarkerBufferDesc =
    {
        MaxCells * sizeof(uint32_t),                                    //  ByteWidth
        D3D11_USAGE_DEFAULT,                                            //  Usage
        D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER,            //  BindFlags
        0,                                                              //  CPUAccessFlags
        0,                                                              //  MiscFlags
        0                                                               //  StructureByteStride
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(
                                    &cellMarkerBufferDesc,
                                    NULL,
                                    AttachPtr(m_cellMarkerBuffer)));        

    //
    //  Create the marker buffer used as output for list_vertices and input for gen_vertices and splat_vertices.
    //
    const size_t MaxMarkers = MaxCells * 5;
    D3D11_BUFFER_DESC vertexMarkerBufferDesc =
    {
        MaxMarkers * sizeof(uint32_t),                                  //  ByteWidth
        D3D11_USAGE_DEFAULT,                                            //  Usage
        D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER,            //  BindFlags
        0,                                                              //  CPUAccessFlags
        0,                                                              //  MiscFlags
        0                                                               //  StructureByteStride
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(
                                    &vertexMarkerBufferDesc,
                                    NULL,
                                    AttachPtr(m_vertexMarkerBuffer)));

    //
    //  Create the vertex buffer used as output for gen_vertices.
    //
    const size_t MaxVertices = MaxCells * 3;
    D3D11_BUFFER_DESC vertexBufferDesc =
    {
        MaxVertices * sizeof(VoxelMesh::Vertex),                        //  ByteWidth
        D3D11_USAGE_DEFAULT,                                            //  Usage
        D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER,            //  BindFlags
        0,                                                              //  CPUAccessFlags
        0,                                                              //  MiscFlags
        0                                                               //  StructureByteStride
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(
                                    &vertexBufferDesc,
                                    NULL,
                                    AttachPtr(m_vertexBuffer)));

    //
    //  Create the index buffer used as output for gen_indices.
    //
    const size_t MaxIndices = MaxCells * 5;
    D3D11_BUFFER_DESC indexBufferDesc = 
    {
        MaxIndices * sizeof(uint32_t),                                  //  ByteWidth
        D3D11_USAGE_DEFAULT,                                            //  Usage
        D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_INDEX_BUFFER,             //  BindFlags
        0,                                                              //  CPUAccessFlags
        0,                                                              //  MiscFlags
        0                                                               //  StructureByteStride
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(
                                    &indexBufferDesc,
                                    NULL,
                                    AttachPtr(m_indexBuffer)));

    //
    //  Create the vertex map used as output for splat_vertices and input for gen_indices.
    //
    D3D11_TEXTURE2D_DESC vertexMapDesc = 
    {
        CellDimensions.x * 3,                                                          //  Width
        CellDimensions.y * CellDimensions.z,                                                 //  Height
        1,                                                              //  MipLevels
        1,                                                              //  ArraySize
        DXGI_FORMAT_R32_UINT,                                    //  Format
        {                                                               //  SampleDesc
            1,                                                          //      Count
            0                                                           //      Quality
        },
        D3D11_USAGE_DEFAULT,                                            //  Usage
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,          //  BindFlags
        0,                                                              //  CPUAccessFlags
        0                                                               //  MiscFlags
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateTexture2D(
                                    &vertexMapDesc,
                                    NULL,
                                    AttachPtr(m_vertexMap)));

    D3D11_RENDER_TARGET_VIEW_DESC vertexMapRTVDesc =
    {
        DXGI_FORMAT_R32_UINT,                                  //  Format
        D3D11_RTV_DIMENSION_TEXTURE2D,                                  //  ViewDimension
        {                                                               //  Texture3D
            0,                                                          //      MipSlice
        }
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateRenderTargetView(
                                    m_vertexMap.get(),
                                    &vertexMapRTVDesc,
                                    AttachPtr(m_vertexMapRTV)));

    D3D11_SHADER_RESOURCE_VIEW_DESC vertexMapSRVDesc =
    {
        DXGI_FORMAT_R32_UINT,                                  //  Format
        D3D11_SRV_DIMENSION_TEXTURE2D,                                  //  ViewDimension
        {                                                               //  Texture3D
            0,                                                          //      MostDetailedMip
            1,                                                          //      MipLevels
        }
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateShaderResourceView(
                                    m_vertexMap.get(),
                                    &vertexMapSRVDesc,
                                    AttachPtr(m_vertexMapSRV)));

    //
    //  Create the density map texture and views.
    //
    D3D11_TEXTURE3D_DESC densityTextureDesc =
    {
        CellDimensions.x,                                                        //  Width
        CellDimensions.y,                                                       //  Height
        CellDimensions.z,                                                        //  Depth
        1,                                                              //  MipLevels
        DXGI_FORMAT_R32_FLOAT,                                          //  Format
        D3D11_USAGE_DEFAULT,                                            //  Usage
        D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,          //  BindFlags
        0,                                                              //  CPUAccessFlags
        0                                                               //  MiscFlags
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateTexture3D(
                                    &densityTextureDesc,
                                    NULL,
                                    AttachPtr(m_voxelTexture)));

    D3D11_SHADER_RESOURCE_VIEW_DESC voxelSRVDesc =
    {
        DXGI_FORMAT_R32_FLOAT,                                              //  Format
        D3D11_SRV_DIMENSION_TEXTURE3D,                                      //  ViewDimension
        {                                                                   //  Texture3D
            0,                                                              //      MostDetailedMip
            1                                                               //      MipLevels
        }
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateShaderResourceView(
                                    m_voxelTexture.get(),
                                    &voxelSRVDesc,
                                    AttachPtr(m_voxelSRV)));

    D3D11_RENDER_TARGET_VIEW_DESC voxelRTVDesc =
    {
        DXGI_FORMAT_R32_FLOAT,                                          //  Format
        D3D11_RTV_DIMENSION_TEXTURE3D,                                  //  ViewDimension
        {                                                               //  Texture3D
            0,                                                          //      MipSlice
            0,                                                          //      FirstWSlice
        }
    };
    voxelRTVDesc.Texture3D.WSize = -1;
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateRenderTargetView(
                                    m_voxelTexture.get(),
                                    &voxelRTVDesc,
                                    AttachPtr(m_voxelRTV)));

    //
    //  Create the material texture, SRV and RTV.
    //
    D3D11_TEXTURE3D_DESC materialTextureDesc =
    {
        CellDimensions.x,                                                        //  Width
        CellDimensions.y,                                                       //  Height
        CellDimensions.z,                                                        //  Depth
        1,                                                              //  MipLevels
        DXGI_FORMAT_R32G32_UINT,                                        //  Format
        D3D11_USAGE_DEFAULT,                                            //  Usage
        D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,          //  BindFlags
        0,                                                              //  CPUAccessFlags
        0                                                               //  MiscFlags
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateTexture3D(
                                    &materialTextureDesc,
                                    NULL,
                                    AttachPtr(m_materialTexture)));

    D3D11_SHADER_RESOURCE_VIEW_DESC materialSRVDesc =
    {
        DXGI_FORMAT_R32G32_UINT,                                            //  Format
        D3D11_SRV_DIMENSION_TEXTURE3D,                                      //  ViewDimension
        {                                                                   //  Texture3D
            0,                                                              //      MostDetailedMip
            1                                                               //      MipLevels
        }
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateShaderResourceView(
                                    m_materialTexture.get(),
                                    &materialSRVDesc,
                                    AttachPtr(m_materialSRV)));

    D3D11_RENDER_TARGET_VIEW_DESC materialRTVDesc =
    {
        DXGI_FORMAT_R32G32_UINT,                                        //  Format
        D3D11_RTV_DIMENSION_TEXTURE3D,                                  //  ViewDimension
        {                                                               //  Texture3D
            0,                                                          //      MipSlice
            0,                                                          //      FirstWSlice
        }
    };
    materialRTVDesc.Texture3D.WSize = -1;
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateRenderTargetView(
                                    m_materialTexture.get(),
                                    &materialRTVDesc,
                                    AttachPtr(m_materialRTV)));

    //
    //  Create the staging texture for copying the voxel data to the CPU.
    //
    D3D11_TEXTURE3D_DESC stagingTextureDesc = 
    {
        32,                                                             //  Width
        32,                                                             //  Height
        32,                                                             //  Depth
        1,                                                              //  MipLevels
        DXGI_FORMAT_R32_FLOAT,                                          //  Format
        D3D11_USAGE_STAGING,                                            //  Usage
        0,                                                              //  BindFlags
        D3D11_CPU_ACCESS_READ,                                          //  CPUAccessFlags
        0                                                               //  MiscFlags
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateTexture3D(
                                    &stagingTextureDesc,
                                    NULL,
                                    AttachPtr(m_stagingTexture)));

    //
    //  Create the query objects.
    //
    D3D11_QUERY_DESC queryDesc =
    {
        D3D11_QUERY_SO_STATISTICS,                                          //  Query
        0                                                                   //  MiscFlags
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateQuery(
                                        &queryDesc,
                                        AttachPtr(m_vertexQuery)));
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateQuery(
                                        &queryDesc,
                                        AttachPtr(m_indexQuery)));

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

    //
    //  Initialize the shader constants.
    //
    m_shaderConstants.cellCount[0] = CellDimensions.x;
    m_shaderConstants.cellCount[1] = CellDimensions.y;
    m_shaderConstants.cellCount[2] = CellDimensions.z;
    m_shaderConstants.cellCount[3] = 0;
    m_shaderConstants.voxelIDMapSize = XMFLOAT4(static_cast<float>(CellDimensions.x),
                                                static_cast<float>(CellDimensions.y),
                                                static_cast<float>(CellDimensions.z),
                                                1.0f);
    memcpy(m_shaderConstants.edgeCellOffsets, EdgeCellOffsets, sizeof(EdgeCellOffsets));
    memcpy(m_shaderConstants.EdgeXOffsets, EdgeXOffsets, sizeof(EdgeXOffsets));
}

VoxelProcessor::~VoxelProcessor()
{
}

void VoxelProcessor::Process(std::shared_ptr<VoxelMesh> geometry,
                             float3 position,
                             float3 size,
                             size_t depth)
{
	ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();

    m_geometryPtr = geometry;

    m_shaderConstants.chunkPosition = XMFLOAT4(position.x,
                                               position.y,
                                               position.z,
                                               1.0f);
    m_shaderConstants.chunkDimensions = size;
    m_shaderConstants.chunkDepth = depth;
    m_shaderConstants.voxelFieldSize = XMFLOAT4(32.0f,
                                                32.0f,
                                                32.0f,
                                                1.0f);
    m_shaderConstants.voxelFieldSubSize = XMFLOAT4(30.0f,
                                                   30.0f,
                                                   30.0f,
                                                   1.0f);
    m_shaderConstants.voxelFieldOffset = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    static Profiler profiler("VoxelProcessor::Generate()");
    profiler.Begin();

    UpdateConstantBuffer();
    GenerateVoxels();
    ListCells();
	ListVertices();
	GenerateVertices();
	SplatVertices();
	GenerateIndices();

    profiler.End();
}

void VoxelProcessor::Update()
{
	ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();

    if (m_geometryPtr)
    {
        if (!m_vertexCount)
        {
            D3D11_QUERY_DATA_SO_STATISTICS stats;
            if (context.GetData(m_vertexQuery.get(),
                                &stats,
                                sizeof(stats),
                                0) == S_OK)
            {
                m_vertexCount = static_cast<size_t>(stats.NumPrimitivesWritten);
                m_verticesAreReady = true;
            }
        }
        if (!m_indexCount)
        {
            D3D11_QUERY_DATA_SO_STATISTICS stats;
            if (context.GetData(m_indexQuery.get(),
                                &stats,
                                sizeof(stats),
                                0) == S_OK)
            {
                m_indexCount = static_cast<size_t>(stats.NumPrimitivesWritten) * 2;
                m_indicesAreReady = true;
            }
        }

        if (m_verticesAreReady && m_indicesAreReady)
        {  
            //
            //  Copy the results from the staging texture to the VoxelField object.
            //
            /*
            D3D11_MAPPED_SUBRESOURCE map;
            context.Map(m_stagingTexture.get(), 0, D3D11_MAP_READ, 0, &map);
            memcpy(m_nodePtr->GetFieldPtr(), map.pData, map.DepthPitch * 32);
            context.Unmap(m_stagingTexture.get(), 0);
            */
            //
            //  Copy the vertex and index buffers to the VoxelMesh object.
            //
            m_geometryPtr->Resize(m_vertexCount, m_indexCount);

            if (m_vertexCount)
            {
                D3D11_BOX vertexSrcBox =
                {
                    0,                                                          //  left
                    0,                                                          //  top
                    0,                                                          //  front
                    m_vertexCount * sizeof(VoxelMesh::Vertex),              //  right
                    1,                                                          //  bottom
                    1                                                           //  back
                };
                context.CopySubresourceRegion(m_geometryPtr->GetVertexBuffer(), //  pDstResource
                                              0,                                //  DstSubresource
                                              0,                                //  DstX
                                              0,                                //  DstY
                                              0,                                //  DstZ
                                              m_vertexBuffer.get(),             //  pSrcResource
                                              0,                                //  SrcSubresource
                                              &vertexSrcBox);                   //  pSrcBox
            }

            if (m_indexCount)
            {
                D3D11_BOX indexSrcBox =
                {
                    0,
                    0,
                    0,
                    m_indexCount * sizeof(uint16_t),
                    1,
                    1
                };
                context.CopySubresourceRegion(m_geometryPtr->GetIndexBuffer(),  //  pDstResource
                                              0,                                //  DstSubresource
                                              0,                                //  DstX
                                              0,                                //  DstY
                                              0,                                //  DstZ
                                              m_indexBuffer.get(),              //  pSrcResource
                                              0,                                //  SrcSubresource
                                              &indexSrcBox);                    //  pSrcBox
            }
            m_geometryPtr->SetReady(true);
            m_geometryPtr.reset();
            m_verticesAreReady = false;
            m_indicesAreReady = false;
        }
    }
}

bool VoxelProcessor::IsReady()
{
    return m_geometryPtr == nullptr;
}

void VoxelProcessor::GenerateVoxels()
{
    static Profiler profiler("VoxelProcessor::GenerateVoxels");
    profiler.Begin();

    ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();

    //
    //  Execute the render op.
    //
    ID3D11Buffer* vertexBufferPtr = m_shared->cellBuffer.get();
    float clearColor[4] = {0.0f, 0.0, 0.0f, 0.0f};
    size_t offset = 0,
           stride = sizeof(uint32_t);
    ID3D11Buffer* constantBufferPtr = m_constantBuffer.get();
    ID3D11ShaderResourceView* shaderResourceViewPtrs[] =
    {
        m_shared->gradientsView.get(),
        m_shared->permutationsView.get()
    };
    D3D11_VIEWPORT viewport = 
    {
        0.0f,                                                           //  TopLeftX
        0.0f,                                                           //  TopLeftY
        32.0f,                                                          //  Width
        32.0f,                                                          //  Height
        0.0f,                                                           //  MinDepth
        1.0f,                                                           //  MaxDepth
    };
    ID3D11RenderTargetView* renderTargetViewPtrs[] =
    {
        m_voxelRTV.get(),
        m_materialRTV.get()
    };

    context.IASetVertexBuffers(0, 1, &vertexBufferPtr, &stride, &offset);
    context.IASetIndexBuffer(NULL, DXGI_FORMAT_R16_UINT, 0);
    context.IASetInputLayout(m_shared->genVoxelsLayout.get());
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context.VSSetShader(m_shared->genVoxelsVS.get(), NULL, 0);
    context.GSSetShader(m_shared->genVoxelsGS.get(), NULL, 0);
    context.PSSetShader(m_shared->genVoxelsPS.get(), NULL, 0);
    context.PSSetShaderResources(0, 2, shaderResourceViewPtrs);
    context.PSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.RSSetState(m_shared->rasterizerState.get());
    context.RSSetViewports(1, &viewport);
    context.OMSetDepthStencilState(m_shared->depthStencilState.get(), 0);
    context.OMSetRenderTargets(2, renderTargetViewPtrs, NULL);
    context.ClearRenderTargetView(m_voxelRTV.get(), clearColor);
    context.DrawInstanced(1, 32 * 32 * 32, 0, 0);

    ID3D11RenderTargetView* nullPtr = NULL;
    context.OMSetRenderTargets(1, &nullPtr, NULL);
    //
    //  Copy the results to the staging texture.
    //
    context.CopyResource(m_stagingTexture.get(), m_voxelTexture.get());

    profiler.End();
}

void VoxelProcessor::ListCells()
{
    ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();
    
    static Profiler profiler("VoxelProcessor::ListCells()");
    profiler.Begin();

    //
    //  Stage 0 (list cells)
    //
    size_t offset = 0,
            stride = sizeof(uint32_t);
    ID3D11Buffer* cellBufferPtr = m_shared->cellBuffer.get();
    ID3D11Buffer* cellMarkerBufferPtr = m_cellMarkerBuffer.get();
    ID3D11ShaderResourceView* shaderResourceViewPtrs[] =
    {
        m_voxelSRV.get()
    };
    ID3D11SamplerState* densitySamplerPtr = m_shared->densitySampler.get();
    ID3D11Buffer* constantBufferPtr = m_constantBuffer.get();
    size_t cellCount = m_shaderConstants.cellCount[0] *
                        m_shaderConstants.cellCount[1] *
                        m_shaderConstants.cellCount[2];

    context.IASetVertexBuffers(0, 1, &cellBufferPtr, &stride, &offset);
    context.IASetIndexBuffer(0, DXGI_FORMAT_R16_UINT, 0);
    context.IASetInputLayout(m_shared->listCellsLayout.get());
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context.SOSetTargets(1, &cellMarkerBufferPtr, &offset);
    context.VSSetShader(m_shared->listCellsVS.get(), NULL, 0);
    context.VSSetShaderResources(0, 1, shaderResourceViewPtrs);
    context.VSSetSamplers(0, 1, &densitySamplerPtr);
    context.VSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.GSSetShader(m_shared->listCellsGS.get(), NULL, 0);
    context.PSSetShader(NULL, NULL, 0);
    context.OMSetRenderTargets(0, NULL, NULL);
    context.OMSetDepthStencilState(m_shared->depthStencilState.get(), 0);
    context.Draw(cellCount, 0);
    context.SOSetTargets(0, NULL, NULL);

    profiler.End();
}

void VoxelProcessor::ListVertices()
{
    ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();

    static Profiler profiler("VoxelProcessor::ListVertices()");
    profiler.Begin();

    //
    //  Stage 1 (list triangles)
    //
    size_t offset = 0,
            stride = sizeof(uint32_t);
    ID3D11Buffer* cellMarkerBufferPtr = m_cellMarkerBuffer.get();
    ID3D11Buffer* vertexMarkerBufferPtr = m_vertexMarkerBuffer.get();
    ID3D11ShaderResourceView* shaderResourceViewPtrs[] =
    {
        m_shared->triTableView.get(),
        m_voxelSRV.get()
    };
    ID3D11SamplerState* densitySamplerPtr = m_shared->densitySampler.get();
    ID3D11Buffer* constantBufferPtr = m_constantBuffer.get();
    size_t cellCount = m_shaderConstants.cellCount[0] *
                        m_shaderConstants.cellCount[1] *
                        m_shaderConstants.cellCount[2];

    context.IASetVertexBuffers(0, 1, &cellMarkerBufferPtr, &stride, &offset);
    context.IASetIndexBuffer(0, DXGI_FORMAT_R16_UINT, 0);
    context.IASetInputLayout(m_shared->listVerticesLayout.get());
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context.SOSetTargets(1, &vertexMarkerBufferPtr, &offset);
    context.VSSetShader(m_shared->listVerticesVS.get(), NULL, 0);
    context.GSSetShader(m_shared->listVerticesGS.get(), NULL, 0);
    context.GSSetShaderResources(0, 2, shaderResourceViewPtrs);
    context.GSSetSamplers(0, 1, &densitySamplerPtr);
    context.GSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.PSSetShader(NULL, NULL, 0);
    context.OMSetRenderTargets(0, NULL, NULL);
    context.OMSetDepthStencilState(m_shared->depthStencilState.get(), 0);
    context.DrawAuto();
    context.SOSetTargets(0, NULL, NULL);

    profiler.End();
}

void VoxelProcessor::GenerateVertices()
{
    static Profiler profiler("VoxelProcessor::GenerateVertices()");
    profiler.Begin();

    ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();
	m_vertexCount = 0;

    //
    //  Stage 2 - generate vertices.
    //
    context.Begin(m_vertexQuery.get());

    size_t offset = 0,
            stride = sizeof(uint32_t);
    ID3D11ShaderResourceView* shaderResourceViewPtrs[] =
    {
        m_voxelSRV.get(),
        m_materialSRV.get()
    };
    ID3D11SamplerState* densitySamplerPtr = m_shared->densitySampler.get();
    ID3D11Buffer* constantBufferPtr = m_constantBuffer.get();
    ID3D11Buffer* vertexMarkerBufferPtr = m_vertexMarkerBuffer.get();
    ID3D11Buffer* vertexBufferPtr = m_vertexBuffer.get();

    context.IASetVertexBuffers(0, 1, &vertexMarkerBufferPtr, &stride, &offset);
    context.IASetIndexBuffer(0, DXGI_FORMAT_R16_UINT, 0);
    context.IASetInputLayout(m_shared->genVerticesLayout.get());
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context.SOSetTargets(1, &vertexBufferPtr, &offset);
    context.VSSetShader(m_shared->genVerticesVS.get(), NULL, 0);
    context.VSSetShaderResources(0, 2, shaderResourceViewPtrs);
    context.VSSetSamplers(0, 1, &densitySamplerPtr);
    context.VSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.GSSetShader(m_shared->genVerticesGS.get(), NULL, 0);
    context.PSSetShader(NULL, NULL, 0);
    context.OMSetDepthStencilState(m_shared->depthStencilState.get(), 0);
    context.DrawAuto();
    context.VSSetConstantBuffers(0, 0, NULL);
    context.VSSetShaderResources(0, 0, NULL);
    context.SOSetTargets(0, NULL, NULL);
    context.End(m_vertexQuery.get());

    profiler.End();

    m_vertexCount = 0;
}

void VoxelProcessor::SplatVertices()
{
    static Profiler profiler("VoxelProcessor::SplatVertices()");
    profiler.Begin();

    ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();

    //
    //  STAGE 2 - Splat the vertex IDs to the vertex ID map texture,
    //            which will be sampled in stage 4 to generate the index buffer.
    //
    D3D11_VIEWPORT viewport = 
    {
        0, 
        0,
        static_cast<float>(m_shaderConstants.cellCount[0] * 3), 
        static_cast<float>(m_shaderConstants.cellCount[1] * m_shaderConstants.cellCount[2]),
        0.0f, 
        1.0f
    };
    context.RSSetViewports(1, &viewport);
    context.RSSetState(m_shared->rasterizerState.get());

    size_t offset = 0, stride = 4;
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    ID3D11RenderTargetView* vertexMapRTVPtr = m_vertexMapRTV.get();
    ID3D11Buffer* vertexMarkerBufferPtr = m_vertexMarkerBuffer.get();
    ID3D11Buffer* constantBufferPtr = m_constantBuffer.get();
    context.IASetVertexBuffers(0, 1, &vertexMarkerBufferPtr, &stride, &offset);
    context.IASetIndexBuffer(0, DXGI_FORMAT_R16_UINT, 0);
    context.IASetInputLayout(m_shared->splatVerticesLayout.get());
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context.VSSetShader(m_shared->splatVerticesVS.get(), NULL, 0);
    context.VSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.GSSetShader(NULL, NULL, 0);
    context.PSSetShader(m_shared->splatVerticesPS.get(), NULL, 0);
    context.OMSetRenderTargets(1, &vertexMapRTVPtr, NULL);
    context.ClearRenderTargetView(vertexMapRTVPtr, clearColor);
    context.DrawAuto();
    context.OMSetRenderTargets(0, NULL, NULL);

    ID3D11RenderTargetView* nullPtr = 0;
    context.OMSetRenderTargets(1, &nullPtr, NULL);

    profiler.End();
}

void VoxelProcessor::GenerateIndices()
{
    static Profiler profiler("VoxelProcessor::GenerateIndices()");
    profiler.Begin();

    ID3D11DeviceContext& context = m_graphicsDevice.GetD3DContext();
	m_indexCount = 0;

    //
    //  STAGE 4 - gen_indices
    //
    size_t offset = 0,
            stride = sizeof(uint32_t);
    ID3D11Buffer* cellMarkerBufferPtr = m_cellMarkerBuffer.get();
    ID3D11Buffer* indexBufferPtr = m_indexBuffer.get();
    ID3D11ShaderResourceView* shaderResourceViewPtrs[] =
    {
        m_shared->triTableView.get(),
        m_vertexMapSRV.get()
    };
    ID3D11Buffer* constantBufferPtr = m_constantBuffer.get();
    size_t cellCount = m_shaderConstants.cellCount[0] *
                        m_shaderConstants.cellCount[1] *
                        m_shaderConstants.cellCount[2];

    context.Begin(m_indexQuery.get());
    context.IASetVertexBuffers(0, 1, &cellMarkerBufferPtr, &stride, &offset);
    context.IASetIndexBuffer(0, DXGI_FORMAT_R16_UINT, 0);
    context.IASetInputLayout(m_shared->genIndicesLayout.get());
    context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context.SOSetTargets(1, &indexBufferPtr, &offset);
    context.VSSetShader(m_shared->genIndicesVS.get(), NULL, 0);
    context.GSSetShader(m_shared->genIndicesGS.get(), NULL, 0);
    context.GSSetShaderResources(0, 2, shaderResourceViewPtrs);
    context.GSSetConstantBuffers(0, 1, &constantBufferPtr);
    context.PSSetShader(NULL, NULL, 0);
    context.OMSetRenderTargets(0, NULL, NULL);
    context.OMSetDepthStencilState(m_shared->depthStencilState.get(), 0);
    context.DrawAuto();
    context.SOSetTargets(0, NULL, NULL);

    ID3D11ShaderResourceView* nullPtrs[] = {0, 0, 0};
    context.GSSetShaderResources(0, 3, nullPtrs);
    context.End(m_indexQuery.get());

    m_indexCount = 0;

    profiler.End();
}

void VoxelProcessor::UpdateConstantBuffer()
{
    D3D11_MAPPED_SUBRESOURCE map;
    D3DCHECK(m_graphicsDevice.GetD3DContext().Map(m_constantBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map));
    memcpy(map.pData, &m_shaderConstants, sizeof(m_shaderConstants));
    m_graphicsDevice.GetD3DContext().Unmap(m_constantBuffer.get(), 0);
}
