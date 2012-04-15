///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "GraphicsDevice.h"
#include "RenderContext.h"
#include "SceneManager.h"
#include "SkyRenderer.h"
#include "VoxelManager.h"
#include "WaterManager.h"

SceneManager::SceneManager(GraphicsDevice& graphicsDevice)
: m_graphicsDevice(graphicsDevice)
{
    m_voxelManager.reset(new VoxelManager(m_graphicsDevice,
                                          4,
                                          float3(2048.0f, 2048.0f, 2048.0f),
                                          24000.0f));
    m_skyRenderer.reset(new SkyRenderer(m_graphicsDevice));
    m_waterManager.reset(new WaterManager(*this));

    // FIXME refactor into RenderContext class
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
    m_graphicsDevice.GetRenderContext().Apply();

    //  Do pre passes
    m_waterManager->PreDraw(m_graphicsDevice.GetRenderContext(), m_camera);

    //  Main render pass
    SceneConstants sceneConstants;
    sceneConstants.projectionMatrix = m_camera.GetProjectionMatrix();
    sceneConstants.viewMatrix = m_camera.GetViewMatrix();
    sceneConstants.cameraPos = m_camera.GetPosition();
    sceneConstants.clipPlane = float4(1.0f, 1.0f, 1.0f, FLT_MAX);   // clip nothing
    sceneConstants.frustum = m_camera.GetFrustum();
    sceneConstants.lowDetail = false;
    m_graphicsDevice.Begin();
    DrawStage(m_graphicsDevice.GetRenderContext(), sceneConstants);
    m_graphicsDevice.End();
}

void SceneManager::DrawStage(RenderContext& renderContext,
                             const SceneConstants& sceneConstants)
{
    m_skyRenderer->Draw(m_graphicsDevice.GetRenderContext(), sceneConstants);
    m_voxelManager->Draw(m_graphicsDevice.GetRenderContext(), sceneConstants);
    m_waterManager->Draw(m_graphicsDevice.GetRenderContext(), sceneConstants);
}