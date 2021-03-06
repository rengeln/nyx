///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Camera.h"
#include "GraphicsDevice.h"
#include "LineRenderer.h"
#include "Profiler.h"
#include "SceneManager.h"
#include "VoxelMesh.h"
#include "VoxelManager.h"
#include "VoxelProcessor.h"
#include "VoxelRenderer.h"

VoxelManager::VoxelManager(GraphicsDevice& graphicsDevice,
                           size_t treeDepth,
                           float3 nodeDimensions,
                           float radius)
: m_graphicsDevice(graphicsDevice),
  m_treeDepth(treeDepth),
  m_radius(radius)
{
    assert(m_treeDepth <= MaxTreeDepth);

    m_oldCamera.SetPosition(float3::Replicate(FLT_MAX));

    m_voxelRenderer.reset(new VoxelRenderer(m_graphicsDevice));
    m_lineRenderer.reset(new LineRenderer(m_graphicsDevice));

    std::fill(m_nodeDimensions.begin(), m_nodeDimensions.end(), float3::Replicate(-1.0f));
    std::fill(m_splitDistances.begin(), m_splitDistances.end(), -1.0f);
    std::fill(m_unsplitDistances.begin(), m_unsplitDistances.end(), -1.0f);
    std::fill(m_fadeOutStartDistances.begin(), m_fadeOutStartDistances.end(), -1.0f);
    std::fill(m_fadeOutEndDistances.begin(), m_fadeOutEndDistances.end(), -1.0f);

    m_nodeDimensions[0] = nodeDimensions;
    for (size_t i = 1; i < m_treeDepth; ++i)
    {
        m_nodeDimensions[i].x = m_nodeDimensions[i - 1].x / 2.0f;
        m_nodeDimensions[i].y = m_nodeDimensions[i - 1].y / 2.0f;
        m_nodeDimensions[i].z = m_nodeDimensions[i - 1].z / 2.0f;
    }


    for (size_t i = 0; i < m_treeDepth; ++i)
    {
        //  Split and fade distances are factors of the node size
        //  Just use the x dimension.
        m_splitDistances[i] = m_nodeDimensions[i].x * 3.0f;
        m_unsplitDistances[i] = m_nodeDimensions[i].x * 3.25f;
        m_fadeOutStartDistances[i] = m_nodeDimensions[i].x * 2.9f;
        m_fadeOutEndDistances[i] = m_nodeDimensions[i].x * 2.8f;
    }

    m_mustSortNodes = false;
    for (size_t i = 0; i < 4; i++)
    {
        m_voxelProcessorArray[i].reset(new VoxelProcessor(m_graphicsDevice));
    }
}

VoxelManager::~VoxelManager()
{
}

void VoxelManager::SetCamera(const Camera& camera)
{
    static Profiler profiler("VoxelManager::SetCamera()");
    profiler.Begin();

    //  If the camera's position has not changed significantly since the last
    //  call to SetCamera, don't bother searching for new nodes to add. (Still
    //  run update on all existing nodes.)
    float delta = Dot(camera.GetPosition() - m_oldCamera.GetPosition(),
                      camera.GetPosition() - m_oldCamera.GetPosition());
    if (delta > 10.0f)
    {
        m_oldCamera = camera;
        //  Translate the camera's position into top-level node indices
        float3 worldPos = camera.GetPosition();
        int32_t indexX = static_cast<int16_t>((worldPos.x - (m_nodeDimensions[0].x / 2.0f)) / m_nodeDimensions[0].x),
                indexZ = static_cast<int16_t>((worldPos.z - (m_nodeDimensions[0].z / 2.0f)) / m_nodeDimensions[0].z);
   
        int visualRadius = static_cast<int>(m_radius / m_nodeDimensions[0].x);
        //  Ensure all nodes within the visual radius are active
        for (int16_t x = indexX - visualRadius; x <= indexX + visualRadius; x++)
        {
            for (int16_t z = indexZ - visualRadius; z <= indexZ + visualRadius; z++)
            {
                for (int16_t y = -1; y < 3; ++y)
                {
                    float3 pos(static_cast<float>(x) * m_nodeDimensions[0].x,
                               0,
                               static_cast<float>(z) * m_nodeDimensions[0].z);
                    float3 cpos = float3(camera.GetPosition().x,
                                         0,
                                         camera.GetPosition().z);
                    float dist = Dot(pos - cpos,
                                     pos - cpos);
                    if (dist < m_radius * m_radius)
                    {
                        uint64_t nodeId = (static_cast<uint64_t>(x & 0xFFFF) << 48) |
                                          (static_cast<uint64_t>(z & 0xFFFF) << 32) |
                                          (static_cast<uint64_t>(y & 0xFFFF) << 16);

                        if (m_nodeMap.find(nodeId) == m_nodeMap.end())
                        {
                            auto node = CreateNode(nodeId, nullptr);
                            m_nodeMap[nodeId] = node;
                            ProcessNode(node);
                        }
                    }
                }
            }
        }
    }

    for (auto i = m_nodeMap.begin(); i != m_nodeMap.end();)
    {
        auto j = i++;
        UpdateNode(*j->second, camera);
        if (j->second->distance > (m_radius * 1.25f))
        {
            m_nodeMap.erase(j);
        }
    }

    profiler.End();
}

void VoxelManager::Update()
{
    static Profiler profiler("VoxelManager::Update()");
    profiler.Begin();
    ProcessNodes();
    profiler.End();
}

void VoxelManager::Draw(RenderContext& renderContext,
                        const SceneConstants& sceneConstants)
{
    static Profiler profiler("VoxelManager::Draw()");
    profiler.Begin();
    for (auto i = m_nodeMap.begin(); i != m_nodeMap.end(); i++)
    {
        CalculateNodeVisibility(*i->second, sceneConstants.frustum);
        DrawNode(*i->second, !sceneConstants.lowDetail);
    }
    m_voxelRenderer->Flush(renderContext, sceneConstants);
    profiler.End();
}

void VoxelManager::DrawBoundingBoxes(RenderContext& renderContext,
                                     const SceneConstants& sceneConstants)
{
    static Profiler profiler("VoxelManager::DrawBoundingBoxes()");
    profiler.Begin();
    for (auto i = m_nodeMap.begin(); i != m_nodeMap.end(); i++)
    {
        CalculateNodeVisibility(*i->second, sceneConstants.frustum);
        DrawNodeBoundingBox(*i->second);
    }
    m_lineRenderer->Flush(renderContext, sceneConstants);
    profiler.End();
}

void VoxelManager::ProcessNodes()
{
    static Profiler profiler("VoxelManager::ProcessNodes()");
    profiler.Begin();

    if (m_pendingNodes.size())
    {
        char buf[64];
        sprintf_s(buf, "%d pending nodes\n", m_pendingNodes.size());
        OutputDebugStringA(buf);
    }
    if (m_mustSortNodes)
    {
        std::sort(m_pendingNodes.begin(), 
                  m_pendingNodes.end(),
                  [] (std::shared_ptr<Node>& lhs, std::shared_ptr<Node>& rhs)
                  {
                      return CompareNodePriority(*lhs, *rhs);
                  });
    }
    
    for (size_t i = 0; i < 4; i++)
    {
        m_voxelProcessorArray[i]->Update();
    }
    
    while (!m_pendingNodes.empty())
    {
        std::shared_ptr<Node> node = *m_pendingNodes.begin();
        if (node.unique())
        {
            // If this is the only pointer, the node was deleted before it was processed
            m_pendingNodes.erase(m_pendingNodes.begin());
            continue;
        }
        size_t processor = 0;
        while (processor < 4)
        {
            if (m_voxelProcessorArray[processor]->IsReady())
            {
                break;
            }
            processor++;
        }
        if (processor == 4)
        {
            break;
        }

        m_voxelProcessorArray[processor]->Process(node->geometry,
                                                  node->position,
                                                  node->size,
                                                  node->depth);
        m_pendingNodes.erase(m_pendingNodes.begin());
    }

    profiler.End();
}

std::shared_ptr<VoxelManager::Node> VoxelManager::CreateNode(uint64_t id,
                                                             Node* parent)
{
    auto node = std::make_shared<Node>();

    node->id = id;
    node->distance = FLT_MAX;
    node->visible = false;
    node->alpha = 1.0f;
    node->parent = parent;

    int base_x = static_cast<int16_t>((id >> 48) & 0xFFFFFF);
    int base_z = static_cast<int16_t>((id >> 32) & 0xFFFFFF);
    int base_y = static_cast<int16_t>((id >> 16) & 0xFFFFFF);
    node->depth = static_cast<uint32_t>((id >> 12) & 0x0F);
    int sub_x = static_cast<uint32_t>((id >> 8) & 0x0F);
    int sub_y = static_cast<uint32_t>((id >> 4) & 0x0F);
    int sub_z = static_cast<uint32_t>((id >> 0) & 0x0F);

    node->position.x = static_cast<float>(base_x) * m_nodeDimensions[0].x;
    node->position.y = static_cast<float>(base_y) * m_nodeDimensions[0].y;
    node->position.z = static_cast<float>(base_z) * m_nodeDimensions[0].z;

    float3 scale = m_nodeDimensions[node->depth];
    node->position.x += static_cast<float>(sub_x) * scale.x;
    node->position.y += static_cast<float>(sub_y) * scale.y;
    node->position.z += static_cast<float>(sub_z) * scale.z;

    node->size = scale;

    node->geometry = std::make_shared<VoxelMesh>(m_graphicsDevice);

    return node;
}

void VoxelManager::UpdateNode(Node& node,
                              const Camera& camera)
{
    float3 cpos = camera.GetPosition();
    float3 center = node.position + (node.size * 0.5f);

#define SQUARE(x) ((x)*(x))
    node.distance = sqrt(SQUARE(cpos.x - center.x) + SQUARE(cpos.z - center.z));
    assert(node.distance >= 0);

    if (!node.children[0] && node.depth < m_treeDepth - 1)
    {
        if (node.distance < m_splitDistances[node.depth])
        {
            SplitNode(node);
        }
    }
    else
    {
        if (node.distance > m_unsplitDistances[node.depth])
        {
            UnsplitNode(node);
        }
    }

    //  Blending distance is based on the center of the parent node in the tree
    if (node.children[0])
    {
        //  If we are split then we may be fading out
        float frac = node.distance - m_fadeOutEndDistances[node.depth];
        node.alpha = min(1.0f, max(0.0f, frac / (m_fadeOutStartDistances[node.depth] - 
                                                 m_fadeOutEndDistances[node.depth])));
    }
    else if (node.depth > 0)
    {
        //  If not split, we may be fading in
        //  Use the parent node's distance for the blending factor
        center = node.parent->position + (node.parent->size * 0.5f);
        float dist = sqrt(SQUARE(cpos.x - center.x) + SQUARE(cpos.z - center.z));
        float frac = m_splitDistances[node.depth - 1] - dist;
        node.alpha = min(1.0f, max(0.0f, frac / (m_splitDistances[node.depth - 1] - 
                                                 m_fadeOutStartDistances[node.depth - 1])));
    }

    if (node.children[0])
    {
        for (size_t i = 0; i < 8; ++i)
        {
            UpdateNode(*node.children[i], camera);
        }
    }
}

void VoxelManager::CalculateNodeVisibility(Node& node, const Frustum& frustum, bool recursive)
{
    box3f boundingBox(node.position, node.position + node.size);
    node.visible = frustum.Intersects(boundingBox);
    if (recursive && node.visible && node.children[0])
    {
        for (size_t i = 0; i < 8; i++)
        {
            CalculateNodeVisibility(*node.children[i], frustum);
        }
    }
}

void VoxelManager::DrawNode(Node& node, bool recursive)
{
    if (node.geometry->IsReady() && node.visible)
    {
        bool mustDraw = !recursive;
        if (!mustDraw && node.children[0])
        {
            for (size_t i = 0; i < 8; ++i)
            {
                if (!node.children[0]->geometry->IsReady())
                {
                    mustDraw = true;
                    break;
                }
            }
        }

        if (mustDraw || node.alpha == 1.0f)
        {
            m_voxelRenderer->Draw(*node.geometry,
                                  node.position);
        }
        else if (node.alpha > 0)
        {
            m_voxelRenderer->DrawTransparent(*node.geometry,
                                             node.position,
                                             node.alpha);
        }
        
        if (!mustDraw && node.children[0])
        {
            if (!node.children[0]->children[0] && node.alpha < 1.0f)
            {
                m_voxelRenderer->DrawGapFiller(*node.geometry,
                                                node.position);
            }
            for (size_t i = 0; i < 8; ++i)
            {
                DrawNode(*node.children[i], recursive);
            }
        }
    }
}

void VoxelManager::DrawNodeBoundingBox(Node& node)
{
    if (node.visible)
    {
        if (node.children[0])
        {
            for (size_t i = 0; i < 8; ++i)
            {
                DrawNodeBoundingBox(*node.children[i]);
            }
        }
        else
        {
            float4 color = float4(1.0f, 0.0f, 0.0f, 1.0f);
            if (node.depth == 1)
            {
                color = float4(0.0f, 1.0f, 0.0f, 1.0f);
            }
            else if (node.depth == 2)
            {
                color = float4(0.0f, 0.0f, 1.0f, 1.0f);
            }
            else if (node.depth == 3)
            {
                color = float4(1.0f, 1.0f, 0.0f, 1.0f);
            }

            box3f box(node.position, node.position + node.size);
            m_lineRenderer->DrawBox(box, color);
        }
    }
}

void VoxelManager::SplitNode(Node& node)
{
    //  Don't bother splitting an empty node
    if (!node.geometry->GetVertexCount())
    {
        return;
    }

    int baseX = static_cast<int16_t>((node.id >> 48) & 0xFFFFFF);
    int baseZ = static_cast<int16_t>((node.id >> 32) & 0xFFFFFF);
    int baseY = static_cast<int16_t>((node.id >> 16) & 0xFFFFFF);
    int sub_x = static_cast<uint32_t>((node.id >> 8) & 0x0F);
    int sub_y = static_cast<uint32_t>((node.id >> 4) & 0x0F);
    int sub_z = static_cast<uint32_t>((node.id >> 0) & 0x0F);

    size_t index = 0;
    for (size_t subX = 0; subX < 2; subX++)
    {
        for (size_t subY = 0; subY < 2; subY++)
        {
            for (size_t subZ = 0; subZ < 2; subZ++)
            {
                node.children[index] = CreateNode(MakeNodeId(baseX, 
                                                             baseZ,
                                                             baseY,
                                                             node.depth + 1,
                                                             (sub_x * 2) + subX,
                                                             (sub_y * 2) + subY,
                                                             (sub_z * 2) + subZ),
                                                  &node);
                ProcessNode(node.children[index]);
                ++index;
            }
        }
    }
}

void VoxelManager::UnsplitNode(Node& node)
{
    for (size_t i = 0; i < 8; ++i)
    {
        node.children[i].reset();
    }
}

void VoxelManager::ProcessNode(std::shared_ptr<Node> node)
{
    m_pendingNodes.push_back(node);
    m_mustSortNodes = true;
}
