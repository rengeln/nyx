///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_VOXELMESH_H__
#define __NYX_VOXELMESH_H__

//
//  Forward declarations.
//
class GraphicsDevice;

class VoxelMesh : public boost::noncopyable
{
public:
    //
    //  Vertex format.
    //
    struct Vertex
    {
        XMFLOAT3 position;
        int32_t normal;
        uint64_t material;
    };

    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] graphicsDevice
    //          Parent GraphicsDevice instance.
    //
    VoxelMesh(GraphicsDevice& graphicsDevice);

    //
    //  Destructor.
    //
    ~VoxelMesh();

    //
    //  Resizes the vertex and index buffers.
    //
    //  Parameters:
    //      [in] vertexCount
    //          New size for the vertex buffer.
    //      [in] indexCount
    //          New size for the index buffer.
    //
    void Resize(size_t vertexCount, size_t indexCount);

    //
    //  Returns the number of items in the vertex buffer.
    //
    size_t GetVertexCount() const;

    //
    //  Returns the number of items in the index buffer.
    //
    size_t GetIndexCount() const;

    //
    //  Returns a pointer to the vertex buffer, or null if there is no vertex data.
    //
    ID3D11Buffer* GetVertexBuffer() const;

    //
    //  Returns a pointer to the index buffer, or null if there is no index data.
    //
    ID3D11Buffer* GetIndexBuffer() const;

    //
    //  Checks if the geometry is ready.
    //
    bool IsReady() const;

    //
    //  Toggles the ready flag.
    //
    void SetReady(bool ready);

private:
    //
    //  Properties.
    //
    GraphicsDevice& m_graphicsDevice;
    boost::intrusive_ptr<ID3D11Buffer> m_vertexBuffer;
    boost::intrusive_ptr<ID3D11Buffer> m_indexBuffer;
    size_t m_vertexCount;
    size_t m_indexCount;
    bool m_ready;
};

inline size_t VoxelMesh::GetVertexCount() const
{
    return m_vertexCount;
}

inline size_t VoxelMesh::GetIndexCount() const
{
    return m_indexCount;
}

inline ID3D11Buffer* VoxelMesh::GetVertexBuffer() const
{
    return m_vertexBuffer.get();
}

inline ID3D11Buffer* VoxelMesh::GetIndexBuffer() const
{
    return m_indexBuffer.get();
}

inline bool VoxelMesh::IsReady() const
{
    return m_ready;
}


#endif  // __NYX_VOXELMESH_H__
