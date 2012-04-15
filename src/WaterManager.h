///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_WATERMANAGER_H__
#define __NYX_WATERMANAGER_H__

class SceneManager;
class WaterRenderer;
 
class WaterManager : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] sceneManager
    //          Parent SceneManager instance.
    //
    WaterManager(SceneManager& sceneManager);

    //
    //  Destructor.
    //
    ~WaterManager();

    //
    //  Updates the water simulation.
    //
    void Update();

    //
    //  Draws the water.
    //
    void Draw(RenderContext& renderContext,
              const SceneConstants& sceneConstants);

    //
    //  Performs pre-drawing tasks, such as generating the reflection map.
    //
    void PreDraw(RenderContext& renderContext,
                 const Camera& camera);

private:
    //
    //  Properties.
    //
    SceneManager& m_sceneManager;
    boost::intrusive_ptr<ID3D11RenderTargetView> m_reflectionRTV;
    boost::intrusive_ptr<ID3D11ShaderResourceView> m_reflectionSRV;
    boost::intrusive_ptr<ID3D11DepthStencilView> m_reflectionDSV;
    boost::intrusive_ptr<ID3D11RasterizerState> m_reflectionRS;
    std::unique_ptr<WaterRenderer> m_waterRenderer;
    bool m_preventDrawing;
};

#endif  //  __NYX_WATERMANAGER_H__
