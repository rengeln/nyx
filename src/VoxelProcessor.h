///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_VOXELPROCESSOR_H__
#define __NYX_VOXELPROCESSOR_H__

//
//  Forward declarations.
//
class GraphicsDevice;
class VoxelManager;
class VoxelGeometry;

class VoxelProcessor : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] graphicsDevice
    //          Parent GraphicsDevice instance.
    //
    VoxelProcessor(GraphicsDevice& graphicsDevice);

    //
    //  Destructor.
    //
    ~VoxelProcessor();

    //
    //  Processes a node.
    //
    //  Parameters:
    //      [out] geometry
    //          Geometry to process.
    //      [in] position
    //          Position of the chunk.
    //      [in] size
    //          Size of the chunk.
    //
    void Process(std::shared_ptr<VoxelGeometry> geometry,
                 float3 position,
                 float3 size,
                 size_t depth);

    //
    //  Updates the processor.
    //
    void Update();

    //
    //  Checks if the processor is ready to accept another chunk.
    //
    bool IsReady();

private:
    //
    //  Updates the constant buffer.
    //
    void UpdateConstantBuffer();

    //
    //  Runs the gen_voxels pass.
    //
    void GenerateVoxels();

    //
    //  Runs the list_cells pass.
    //
    void ListCells();

	//
	//	Runs the list_vertices pass.
	//
	void ListVertices();

	//
	//	Runs the gen_vertices pass.
	//
	void GenerateVertices();

	//
	//	Runs the splat vertices pass.
	//
	void SplatVertices();

	//
	//	Runs the gen_indices pass.
	//
	void GenerateIndices();

    //
    //  Resources shared among all VoxelProcessor instances.
    //
    struct SharedProperties
    {
        boost::intrusive_ptr<ID3D11InputLayout> genVoxelsLayout;
        boost::intrusive_ptr<ID3D11VertexShader> genVoxelsVS;
        boost::intrusive_ptr<ID3D11GeometryShader> genVoxelsGS;
        boost::intrusive_ptr<ID3D11PixelShader> genVoxelsPS;
        
        boost::intrusive_ptr<ID3D11InputLayout> listCellsLayout;
        boost::intrusive_ptr<ID3D11VertexShader> listCellsVS;
        boost::intrusive_ptr<ID3D11GeometryShader> listCellsGS;

        boost::intrusive_ptr<ID3D11InputLayout> listVerticesLayout;
        boost::intrusive_ptr<ID3D11VertexShader> listVerticesVS;
        boost::intrusive_ptr<ID3D11GeometryShader> listVerticesGS;

        boost::intrusive_ptr<ID3D11InputLayout> genVerticesLayout;
        boost::intrusive_ptr<ID3D11VertexShader> genVerticesVS;
        boost::intrusive_ptr<ID3D11GeometryShader> genVerticesGS;

        boost::intrusive_ptr<ID3D11InputLayout> splatVerticesLayout;
        boost::intrusive_ptr<ID3D11VertexShader> splatVerticesVS;
        boost::intrusive_ptr<ID3D11GeometryShader> splatVerticesGS;
        boost::intrusive_ptr<ID3D11PixelShader> splatVerticesPS;

        boost::intrusive_ptr<ID3D11InputLayout> genIndicesLayout;
        boost::intrusive_ptr<ID3D11VertexShader> genIndicesVS;
        boost::intrusive_ptr<ID3D11GeometryShader> genIndicesGS;

        boost::intrusive_ptr<ID3D11Buffer> cellBuffer;
        boost::intrusive_ptr<ID3D11ShaderResourceView> triTableView;
        boost::intrusive_ptr<ID3D11SamplerState> densitySampler;
        boost::intrusive_ptr<ID3D11DepthStencilState> depthStencilState;
        boost::intrusive_ptr<ID3D11RasterizerState> rasterizerState;
        boost::intrusive_ptr<ID3D11ShaderResourceView> permutationsView;
        boost::intrusive_ptr<ID3D11ShaderResourceView> gradientsView;
    };

    //
    //  Shader constants for the marching cubes geometry shader.
    //
    struct ShaderConstants
    {
        XMFLOAT4 chunkPosition;
        float3 chunkDimensions;
        uint chunkDepth;
        size_t cellCount[4];
        XMFLOAT4 voxelFieldSize;
        XMFLOAT4 voxelFieldSubSize;
        XMFLOAT4 voxelFieldOffset;
        XMFLOAT4 voxelIDMapSize;
        size_t edgeCellOffsets[12][4];
        size_t EdgeXOffsets[12][4];
    };

    //
    //  Properties.
    //
    static const int EdgeTable[256];
    static const int TriTable[256][16];
    static std::weak_ptr<SharedProperties> m_sharedWeakPtr;
    static const uint3 CellDimensions;
    size_t m_width;
    size_t m_height;
    size_t m_depth;
    GraphicsDevice& m_graphicsDevice;
    std::shared_ptr<SharedProperties> m_shared;
    ShaderConstants m_shaderConstants;
    size_t m_vertexCount;
	size_t m_indexCount;
    bool m_verticesAreReady;
    bool m_indicesAreReady;
    std::shared_ptr<VoxelGeometry> m_geometryPtr;

    boost::intrusive_ptr<ID3D11Buffer> m_cellMarkerBuffer;
    boost::intrusive_ptr<ID3D11Buffer> m_vertexMarkerBuffer;
    boost::intrusive_ptr<ID3D11Buffer> m_vertexBuffer;
    boost::intrusive_ptr<ID3D11Buffer> m_indexBuffer;
    boost::intrusive_ptr<ID3D11Texture2D> m_vertexMap;
    boost::intrusive_ptr<ID3D11RenderTargetView> m_vertexMapRTV;
    boost::intrusive_ptr<ID3D11ShaderResourceView> m_vertexMapSRV;
    boost::intrusive_ptr<ID3D11Buffer> m_constantBuffer;
    boost::intrusive_ptr<ID3D11Query> m_vertexQuery;
    boost::intrusive_ptr<ID3D11Query> m_indexQuery;
    boost::intrusive_ptr<ID3D11Texture3D> m_voxelTexture;
    boost::intrusive_ptr<ID3D11ShaderResourceView> m_voxelSRV;
    boost::intrusive_ptr<ID3D11RenderTargetView> m_voxelRTV;
    boost::intrusive_ptr<ID3D11Texture3D> m_materialTexture;
    boost::intrusive_ptr<ID3D11ShaderResourceView> m_materialSRV;
    boost::intrusive_ptr<ID3D11RenderTargetView> m_materialRTV;
    boost::intrusive_ptr<ID3D11Texture3D> m_stagingTexture;
};

#endif  // __NYX_VOXELPROCESSOR_H__
