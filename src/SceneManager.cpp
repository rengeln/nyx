///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "GraphicsDevice.h"
#include "SceneManager.h"
#include "SkyRenderer.h"
#include "VoxelManager.h"

SceneManager::SceneManager(GraphicsDevice& graphicsDevice)
: m_graphicsDevice(graphicsDevice)
{
    m_voxelManager.reset(new VoxelManager(m_graphicsDevice,
                                          4,
                                          float3(640.0f, 640.0f, 640.0f),
                                          4800.0f));
    m_skyRenderer.reset(new SkyRenderer(m_graphicsDevice));
}

SceneManager::~SceneManager()
{
}

void SceneManager::SetCamera(const Camera& camera)
{
    m_camera = camera;
    m_voxelManager->SetCamera(m_camera);
}

void SceneManager::Update()
{
    m_voxelManager->Update();
}

void SceneManager::Draw()
{
    m_skyRenderer->SetCamera(m_camera);
    m_skyRenderer->Draw();
    m_voxelManager->Draw(m_camera);
}
