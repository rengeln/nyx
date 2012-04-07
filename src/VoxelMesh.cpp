///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "GraphicsDevice.h"
#include "VoxelMesh.h"

VoxelMesh::VoxelMesh(GraphicsDevice& graphicsDevice)
: m_graphicsDevice(graphicsDevice),
  m_vertexCount(0),
  m_indexCount(0),
  m_ready(false)
{
}

VoxelMesh::~VoxelMesh()
{
}

void VoxelMesh::Resize(size_t vertexCount, size_t indexCount)
{
    m_vertexBuffer.reset();
    m_indexBuffer.reset();
    m_vertexCount = vertexCount;
    m_indexCount = indexCount;

    m_vertexBuffer.reset();
    m_indexBuffer.reset();

    m_vertexCount = vertexCount;
    m_indexCount = indexCount;

    if (m_vertexCount)
    {
        //
        //  Recreate the vertex buffer.
        //
        D3D11_BUFFER_DESC vertexBufferDesc =
        {
            m_vertexCount * sizeof(Vertex),                         //  ByteWidth
            D3D11_USAGE_DEFAULT,                                    //  Usage
            D3D11_BIND_VERTEX_BUFFER,                               //  BindFlags
            0,                                                      //  CPUAccessFlags
            0,                                                      //  MiscFlags
            0                                                       //  StructureByteStride
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(&vertexBufferDesc,
                                                              NULL,
                                                              AttachPtr(m_vertexBuffer)));
    }
    if (m_indexCount)
    {
        //
        //  Recreate the index buffer.
        //
        D3D11_BUFFER_DESC indexBufferDesc =
        {
            m_indexCount * sizeof(uint16_t),                        //  ByteWidth
            D3D11_USAGE_DEFAULT,                                    //  Usage
            D3D11_BIND_INDEX_BUFFER,                                //  BindFlags
            0,                                                      //  CPUAccessFlags
            0,                                                      //  MiscFlags
            0                                                       //  StructureByteStride
        };
        D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateBuffer(&indexBufferDesc,
                                                              NULL,
                                                              AttachPtr(m_indexBuffer)));
    }
}

void VoxelMesh::SetReady(bool ready)
{
    m_ready = ready;
}