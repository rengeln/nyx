///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_LINERENDERER_H__
#define __NYX_LINERENDERER_H__

//
//  Forward declarations.
//
class Camera;
class GraphicsDevice;
class RenderContext;
class SceneConstants;

class LineRenderer : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] graphicsDevice
    //          Parent GraphicsDevice instance.
    //
    LineRenderer(GraphicsDevice& graphicsDevice);

    //
    //  Destructor.
    //
    ~LineRenderer();

    //
    //  Draws a line segment.
    //
    //  Parameters:
    //      [in] p0
    //          First point.
    //      [in] p1
    //          Second point.
    //      [in] color
    //          Line color.
    //
    void DrawLine(float3 p0, float3 p1, float4 color);

    //
    //  Draws an axis-aligned box.
    //
    //  Parameters:
    //      [in] p0
    //          Minimum corner.
    //      [in] p1
    //          Maximum corner.
    //      [in] color
    //          Line color.
    //
    void DrawBox(box3f, float4 color);

    //
    //  Flushes all queued drawing operations.
    //
    //  Parameters:
    //      [in] renderContext
    //          Render context.
    //      [in] sceneConstants
    //          Scene constants.
    //
    void Flush(RenderContext& renderContext,
               const SceneConstants& sceneConstants);

private:
    //
    //  Vertex type.
    //
    struct Vertex
    {
        float3 position;
        ubyte4 color;
    };

    //
    //  Properties.
    //
    struct SharedProperties
    {
        boost::intrusive_ptr<ID3D11InputLayout> inputLayout;
        boost::intrusive_ptr<ID3D11VertexShader> vertexShader;
        boost::intrusive_ptr<ID3D11PixelShader> pixelShader;
        boost::intrusive_ptr<ID3D11Buffer> constantBuffer;
        boost::intrusive_ptr<ID3D11RasterizerState> rasterizerState;
        boost::intrusive_ptr<ID3D11DepthStencilState> depthStencilState;
        boost::intrusive_ptr<ID3D11Buffer> vertexBuffer;
    };
    struct ShaderConstants
    {
        float4x4 projectionViewMatrix;
    };
    static std::weak_ptr<SharedProperties> m_sharedWeakPtr;
    GraphicsDevice& m_graphicsDevice;
    std::shared_ptr<SharedProperties> m_shared;
    boost::intrusive_ptr<ID3D11Buffer> m_vertexBuffer;
    std::vector<Vertex> m_vertices;
    uint m_vertexBufferSize;
    ShaderConstants m_constants;
};

#endif  // __NYX_LINERENDERER_H__
