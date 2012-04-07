///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_SKYRENDERER_H__
#define __NYX_SKYRENDERER_H__

//
//  Forward declarations.
//
class Camera;
class GraphicsDevice;

//
//  Renders the skybox.
//
class SkyRenderer : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] graphicsDevice
    //          Parent GraphicsDevice instance.
    //
    SkyRenderer(GraphicsDevice& graphicsDevice);

    //
    //  Destructor.
    //
    ~SkyRenderer();

    //
    //  Sets the camera position.
    //
    void SetCamera(const Camera& camera);

    //
    //  Draws the skybox.
    //
    void Draw();

private:
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
        boost::intrusive_ptr<ID3D11ShaderResourceView> skyTextureView;
        boost::intrusive_ptr<ID3D11SamplerState> samplerState;
    };
    struct ShaderConstants
    {
        float4x4 viewMatrix;
        float4x4 projectionMatrix;
    };
    struct Vertex 
    {
        float xyz[3];
        float uv[2];
    };

    static const Vertex SkyboxVertices[24];
    static const uint16_t SkyboxIndices[36];
    static std::weak_ptr<SharedProperties> m_sharedWeakPtr;
    std::shared_ptr<SharedProperties> m_shared;
    GraphicsDevice& m_graphicsDevice;
    ShaderConstants m_constants;
    boost::intrusive_ptr<ID3D11Buffer> m_constantBuffer;
};

#endif  // __NYX_SKYRENDERER_H__
