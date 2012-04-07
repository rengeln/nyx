///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_SCENEMANAGER_H__
#define __NYX_SCENEMANAGER_H__

#include "Camera.h"

//
//  Forward declarations.
//
class GraphicsDevice;
class VoxelManager;

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
    //  Returns the VoxelManager.
    //
    VoxelManager& GetVoxelManager();

private:
    //
    //  Properties.
    //
    GraphicsDevice& m_graphicsDevice;
    std::unique_ptr<VoxelManager> m_voxelManager;
    Camera m_camera;
};

inline VoxelManager& SceneManager::GetVoxelManager()
{
    assert(m_voxelManager);
    return *m_voxelManager;
}

#endif  //  __NYX_SCENEMANGER_H__
