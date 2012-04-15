///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_SCENEMANAGER_H__
#define __NYX_SCENEMANAGER_H__

#include "Camera.h"
#include "Frustum.h"

//
//  Forward declarations.
//
class GraphicsDevice;
class SkyRenderer;
class RenderContext;
class VoxelManager;
class WaterManager;

//
//  Global rendering constants.
//
class SceneConstants
{
public:
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    Frustum frustum;
    float4 clipPlane;
    float3 cameraPos;
    bool lowDetail;
};

class SceneManager : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] graphicsDevice
    //          GraphicsDevice instance.
    //
    SceneManager(GraphicsDevice& graphicsDevice);

    //
    //  Destructor.
    //
    ~SceneManager();

    //
    //  Sets the camera position.
    //
    void SetCamera(const Camera& camera);

    //
    //  Updates the simulation.
    //
    void Update();

    //
    //  Draws the scene.
    //
    void Draw();

    //
    //  Draws the scene for an individual render stage.
    //
    void DrawStage(RenderContext& renderContext,
                   const SceneConstants& sceneConstants);

    //
    //  Returns the GraphicsDevice instance.
    //
    GraphicsDevice& GetGraphicsDevice();

private:
    //
    //  Properties.
    //
    GraphicsDevice& m_graphicsDevice;
    std::unique_ptr<VoxelManager> m_voxelManager;
    std::unique_ptr<SkyRenderer> m_skyRenderer;
    std::unique_ptr<WaterManager> m_waterManager;
    
    Camera m_camera;
};

inline GraphicsDevice& SceneManager::GetGraphicsDevice()
{
    return m_graphicsDevice;
}

#endif  //  __NYX_SCENEMANGER_H__
