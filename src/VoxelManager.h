///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_VOXELMANAGER_H__
#define __NYX_VOXELMANAGER_H__

//
//  Forward declarations.
//
class Camera;
class GraphicsDevice;
class VoxelProcessor;
class VoxelMesh;

class VoxelManager : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] graphicsDevice
    //          Parent GraphicsDevice instance.
    //      [in] treeDepth
    //          Maximum octree depth;
    //      [in] nodeDimensions
    //          Worldspace dimensions of a root node in the octree.
    //      [in] radius
    //          Visual radius around the camera to load nodes.
    //
    VoxelManager(GraphicsDevice& graphicsDevice,
                 size_t treeDepth,
                 float3 nodeDimensions,
                 float radius);

    //
    //  Destructor.
    //
    ~VoxelManager();

    //
    //  Updates the simulation.
    //
    void Update();

    //
    //  Updates the node tree based on a camera position.
    //
    void SetCamera(const Camera& camea);

    //
    //  Draws the voxel world.
    //
    //  This doesn't need to be the same camera that SetCamera() was called
    //  with, although if it is very far away then artifacts will likely result.
    //
    void Draw(const Camera& camera);

    //
    //  Draws the voxel bounding boxes.
    //
    void DrawBoundingBoxes(const Camera& camera);

    //
    //  Processes pending nodes.
    //
    void ProcessNodes();

private:
    //
    //  An individual node in the tree.
    //
    struct Node
    {
        std::array<std::shared_ptr<Node>, 8> children;
        float3 position;
        float3 size;
        int depth;
        uint64_t id;
        float distance;
        std::shared_ptr<VoxelMesh> geometry;
        float alpha;
        Node* parent;
        bool visible;
    };

    //
    //  Packs a node ID.
    //
    static uint64_t MakeNodeId(int16_t baseX,
                               int16_t baseZ,
                               int16_t baseY,
                               size_t depth,
                               size_t subX,
                               size_t subY,
                               size_t subZ);

    //
    //  Updates an individual node.
    //
    void UpdateNode(Node& node, const Camera& camera);

    //
    //  Calculates a node's visibility.
    //
    void CalculateNodeVisibility(Node& node, const Camera& camera);

    //
    //  Splits a node.
    //
    void SplitNode(Node& node);

    //
    //  Unsplits a node.
    //
    void UnsplitNode(Node& node);

    //
    //  Draws a node.
    //
    void DrawNode(Node& node);
    void DrawNodeBoundingBox(Node& node);

    //
    //  Creates a new node.
    //
    std::shared_ptr<Node> CreateNode(uint64_t id, Node* parent);

    //
    //  Enqueues a node for processing.
    //
    void ProcessNode(std::shared_ptr<Node> node);

    //
    //  Compares two nodes to determine processing priority.
    //
    static bool CompareNodePriority(const Node& lhs, const Node& rhs);

    //
    //  Properties.
    //
    static const size_t MaxTreeDepth = 8;
    GraphicsDevice& m_graphicsDevice;
    std::unique_ptr<VoxelRenderer> m_voxelRenderer;
    std::unique_ptr<LineRenderer> m_lineRenderer;
    uint3 m_cellsPerNode;
    size_t m_treeDepth;
    float m_radius;
    std::array<float3, MaxTreeDepth> m_nodeDimensions;
    std::array<float, MaxTreeDepth + 1> m_splitDistances;
    std::array<float, MaxTreeDepth + 1> m_unsplitDistances;
    std::array<float, MaxTreeDepth + 1> m_fadeOutStartDistances;
    std::array<float, MaxTreeDepth + 1> m_fadeOutEndDistances;
    std::array<std::unique_ptr<VoxelProcessor>, 4> m_voxelProcessorArray;
    std::map<uint64_t, std::shared_ptr<Node>> m_nodeMap;
    std::vector<std::shared_ptr<Node>> m_pendingNodes;
    bool m_mustSortNodes;
};

inline uint64_t VoxelManager::MakeNodeId(int16_t baseX, 
                                         int16_t baseZ,
                                         int16_t baseY,
                                         size_t depth, 
                                         size_t subX,
                                         size_t subY,
                                         size_t subZ)
{
    uint64_t baseX64 = *reinterpret_cast<uint16_t*>(&baseX),
             baseZ64 = *reinterpret_cast<uint16_t*>(&baseZ),
             baseY64 = *reinterpret_cast<uint16_t*>(&baseY);
    return (baseX64 << 48) |
           (baseZ64 << 32) |
           (baseY64 << 16) |
           ((depth & 0x0F) << 12) |
           ((subX & 0x0F) << 8) |
           ((subY & 0x0F) << 4) |
           ((subZ & 0x0F) << 0);
}

inline bool VoxelManager::CompareNodePriority(const Node& lhs, const Node& rhs)
{
    if (lhs.visible != rhs.visible)
    {
        return lhs.visible > rhs.visible;
    }
    if (lhs.depth != rhs.depth)
    {
        return lhs.depth < rhs.depth;
    }
    else
    {
        return lhs.distance < rhs.distance;
    }
}


#endif  // __NYX_VOXELMANAGER_H__
