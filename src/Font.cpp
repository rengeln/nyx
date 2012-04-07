///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#include "Prefix.h"
#include "Font.h"
#include "GraphicsDevice.h"

std::weak_ptr<Font::SharedProperties> Font::m_sharedWeakPtr;

Font::Font(GraphicsDevice& graphicsDevice, const char* path, size_t size)
: m_graphicsDevice(graphicsDevice)
{
    assert(size > 0);

    if (!m_sharedWeakPtr.expired())
    {
        m_shared = m_sharedWeakPtr.lock();
    }
    else
    {
        m_shared = std::make_shared<SharedProperties>();
        m_sharedWeakPtr = std::weak_ptr<SharedProperties>(m_shared);
    
        FTCHECK(FT_Init_FreeType(&m_shared->freetype));
    }

    FTCHECK(FT_New_Face(m_shared->freetype,
                        path,
                        0,
                        &m_face));
    FTCHECK(FT_Set_Pixel_Sizes(m_face, 0, 256));

    Bin startingBin =
    {
        uint2(0, 0),
        uint2(BufferWidth, BufferHeight)
    };
    m_bins.push_back(startingBin);

    m_glyphBuffer.resize(BufferWidth * BufferHeight, 0);


    //
    //  Create the texture.
    //
    D3D11_TEXTURE2D_DESC textureDesc =
    {
        BufferWidth,                                                        //  Width
        BufferHeight,                                                       //  Height
        1,                                                                  //  MipLevels
        1,                                                                  //  ArraySize
        DXGI_FORMAT_A8_UNORM,                                               //  Format
        {                                                                   //  SampleDesc
            1,                                                              //      Count
            0                                                               //      Quality
        },
        D3D11_USAGE_DYNAMIC,                                                //  Usage
        D3D11_BIND_SHADER_RESOURCE,                                         //  BindFlags
        D3D11_CPU_ACCESS_WRITE,                                             //  CPUAccessFlags
        0                                                                   //  MiscFlags
    };
    D3DCHECK(m_graphicsDevice.GetD3DDevice().CreateTexture2D(&textureDesc,
                                                             NULL,
                                                             AttachPtr(m_texture)));

    for (size_t i = 'A'; i < 'z'; i++)
    {
        LoadGlyph(i);
    }
    Update();
}

Font::~Font()
{
}

Font::Glyph& Font::LoadGlyph(uint32_t c)
{
    uint32_t glyphIndex = FT_Get_Char_Index(m_face, c);
    FTCHECK(FT_Load_Glyph(m_face, glyphIndex, FT_LOAD_NO_BITMAP));
    FTCHECK(FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL));

    size_t width = m_face->glyph->bitmap.width + 2,
           height = m_face->glyph->bitmap.rows + 2;

    size_t glyphX, glyphY;
    for (auto i = m_bins.begin(); i != m_bins.end(); i++)
    {
        if (i->size.x >= width && i->size.y >= height)
        {
            //  Place the glyph at the top-left corner of the bin
            glyphX = i->xy.x;
            glyphY = i->xy.y;

            //  Split the bin
            Bin leftSplit =
            {
                uint2(i->xy.x, i->xy.y + height),
                uint2(width, i->size.y - height)
            };
            Bin rightSplit =
            {
                uint2(i->xy.x + width, i->xy.y),
                uint2(i->size.x - width, i->size.y)
            };

            m_bins.erase(i);
            if (leftSplit.size.y > 0)
            {
                m_bins.push_back(leftSplit);
            }
            if (rightSplit.size.x > 0)
            {
                m_bins.push_back(rightSplit);
            }
            break;
        }
    }
    //  For now crash if we fail to find a bin, this is an indication that a
    //  larger buffer is needed. (Better behavior would be to erase -everything- and
    //  start from scratch with a new buffer, then re-add all glyphs that were
    //  previously loaded.)

    std::vector<int2> df;
    df.resize((m_face->glyph->bitmap.width + 32) + (m_face->glyph->bitmap.rows + 32));

    //  Copy the glyph bitmap into the buffer
    for (size_t i = 0; i < m_face->glyph->bitmap.rows; i++)
    {
        memcpy(m_glyphBuffer.data() + ((glyphY + i + 1) * BufferWidth) + glyphX + 1,
               m_face->glyph->bitmap.buffer + (i * m_face->glyph->bitmap.pitch),
               m_face->glyph->bitmap.width);
    }
    m_dirty = true;

    //  Normalize the texture coordinates
    Glyph glyph;
    glyph.uv.x = static_cast<float>(glyphX + 1) / static_cast<float>(BufferWidth);
    glyph.uv.y = static_cast<float>(glyphY + 1) / static_cast<float>(BufferHeight);
    glyph.size.x = static_cast<float>(width) / static_cast<float>(BufferWidth);
    glyph.size.y = static_cast<float>(height) / static_cast<float>(BufferHeight);

    auto result = m_glyphMap.insert(std::make_pair(c, glyph));
    return (result.first)->second;
}

void Font::Update()
{
    D3D11_MAPPED_SUBRESOURCE map;
    D3DCHECK(m_graphicsDevice.GetD3DContext().Map(m_texture.get(),
                                                  0,
                                                  D3D11_MAP_WRITE_DISCARD,
                                                  0,
                                                  &map));
    for (size_t i = 0; i < BufferHeight; i++)
    {
        memcpy(reinterpret_cast<char*>(map.pData) + (map.RowPitch * i),
               m_glyphBuffer.data() + (BufferWidth * i),
               BufferWidth);
    }
    m_graphicsDevice.GetD3DContext().Unmap(m_texture.get(), 0);
}
