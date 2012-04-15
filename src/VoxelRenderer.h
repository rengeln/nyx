///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_VOXELRENDERER_H__
#define __NYX_VOXELRENDERER_H__

//
//  Forward declarations.
//
class Camera;
class GraphicsDevice;
class RenderContext;
class SceneConstants;
class VoxelMesh;

//
//  Implements the voxel mesh renderer.
//
class VoxelRenderer : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] graphicsDevice
    //          Parent GraphicsDevice instance.
    //
    VoxelRenderer(GraphicsDevice& graphicsDevice);

    //
    //  Destructor.
    //
    ~VoxelRenderer();

    //
    //  Draws a voxel mesh.
    //
    //  In reality these functions enqueue the meshes for drawing, which occurs
    //  when Flush() is called.
    //
    void Draw(const VoxelMesh& geometry, float3 position);
    void DrawTransparent(const VoxelMesh& geometry, float3 position, float alpha);
    void DrawGapFiller(const VoxelMesh& geometry, float3 position);

    //
    //  Flushes queued render ops.
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
    struct RenderOp
    {
        const VoxelMesh* geometry;
        float3 position;
        float distance2;
        float alpha;
        bool gapFiller;
    };

    //
    //  Implements the actual drawing of voxel meshes.
    //
    void Draw(const RenderOp& renderOp);

    //
    //  Properties.
    //
    struct SharedProperties
    {
        boost::intrusive_ptr<ID3D11VertexShader> vertexShader;
        boost::intrusive_ptr<ID3D11PixelShader> pixelShader;
        boost::intrusive_ptr<ID3D11InputLayout> inputLayout;
        boost::intrusive_ptr<ID3D11DepthStencilState> depthStencilState;
        boost::intrusive_ptr<ID3D11BlendState> blendState;
        std::array<boost::intrusive_ptr<ID3D11ShaderResourceView>, 8> textureViews;
        boost::intrusive_ptr<ID3D11SamplerState> samplerState;
        boost::intrusive_ptr<ID3D11DepthStencilState> transparentDepthStencilState;
        boost::intrusive_ptr<ID3D11DepthStencilState> fillGapsDepthStencilState;
    };
    struct ShaderConstants
    {
        float4x4 projectionViewMatrix;
        float4x4 worldMatrix;
        float4 alpha;
        float4 clipPlane;
    };
    static std::weak_ptr<SharedProperties> m_sharedWeakPtr;
    std::shared_ptr<SharedProperties> m_shared;
    GraphicsDevice& m_graphicsDevice;
    boost::intrusive_ptr<ID3D11Buffer> m_constantBuffer;
    ShaderConstants m_constants;
    std::vector<RenderOp> m_renderOps;
    std::vector<RenderOp> m_transparentRenderOps;
    std::vector<RenderOp> m_gapFillerRenderOps;
};

#endif  // __NYX_VOXELRENDERER_H__
