///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_WATERRENDERER_H__
#define __NYX_WATERRENDERER_H__

//
//  Renders the water plane.
//
class WaterRenderer : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    WaterRenderer(GraphicsDevice& graphicsDevice);

    //
    //  Destructor.
    //
    ~WaterRenderer();

    //
    //  Sets the camera position.
    //
    void SetCamera(const Camera& camera);

    //
    //  Draws the water plane.
    //
    void Draw();

private:
    struct Vertex
    {
        float3 xyz;
    };

    //
    //  Generates the radial grid mesh.
    //
    //  Parameters:
    //      [out] vertices
    //          Vector which will hold the generated vertices.
    //      [out] indices
    //          Vector which will hold the generated indices.
    //
    static void GenerateRadialGrid(std::vector<struct Vertex>& vertices,
                                   std::vector<uint16_t>& indices);

    //
    //  Properties.
    //
    struct SharedProperties
    {
        boost::intrusive_ptr<ID3D11VertexShader> vertexShader;
        boost::intrusive_ptr<ID3D11PixelShader> pixelShader;
        boost::intrusive_ptr<ID3D11InputLayout> inputLayout;
        boost::intrusive_ptr<ID3D11Buffer> vertexBuffer;
        boost::intrusive_ptr<ID3D11Buffer> indexBuffer;
        boost::intrusive_ptr<ID3D11RasterizerState> rasterizerState;
        boost::intrusive_ptr<ID3D11DepthStencilState> depthStencilState;
        boost::intrusive_ptr<ID3D11SamplerState> samplerState;
        boost::intrusive_ptr<ID3D11BlendState> blendState;
        boost::intrusive_ptr<ID3D11ShaderResourceView> noiseTextureView;
        boost::intrusive_ptr<ID3D11SamplerState> noiseSampler;
        uint32_t indexCount;
    };
    struct ShaderConstants
    {
        float4x4 viewMatrix;
        float4x4 projectionMatrix;
        float4   cameraPosition;
        float4   offset;
    };
    static std::weak_ptr<SharedProperties> m_sharedWeakPtr;
    std::shared_ptr<SharedProperties> m_shared;
    GraphicsDevice& m_graphicsDevice;
    ShaderConstants m_shaderConstants;
    boost::intrusive_ptr<ID3D11Buffer> m_constantBuffer;
    float m_time;
};

#endif  //  __NYX_WATERRENDERER_H__
