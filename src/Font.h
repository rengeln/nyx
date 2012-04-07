///////////////////////////////////////////////////////////////////////////////
//	Copyright (C) 2011 Robert Engeln (engeln@gmail.com) All rights reserved.
//	See accompanying LICENSE file for full license information.
///////////////////////////////////////////////////////////////////////////////

#ifndef __NYX_FONT_H__
#define __NYX_FONT_H__

class GraphicsDevice;

class Font : public boost::noncopyable
{
public:
    //
    //  Constructor.
    //
    //  Parameters:
    //      [in] graphicsDevice
    //          Parent GraphicsDevice instance.
    //      [in] font
    //          Path to the font file.
    //      [in] size
    //          Face size.
    //
    Font(GraphicsDevice& graphicsDevice, const char* path, size_t size);

    //
    //  Destructor.
    //
    ~Font();

private:
    struct Glyph
    {
        float2 uv;
        float2 size;
    };
    struct Bin
    {
        uint2 xy;
        uint2 size;
    };

    //
    //  Loads a glyph.
    //
    Glyph& LoadGlyph(uint32_t c);

    //
    //  Updates the texture.
    //
    void Update();

    //
    //  Properties.
    //
    struct SharedProperties
    {
        FT_Library freetype;
    };
    static std::weak_ptr<SharedProperties> m_sharedWeakPtr;
    static const size_t BufferWidth = 2048,
                        BufferHeight = 1024;
    std::shared_ptr<SharedProperties> m_shared;
    GraphicsDevice& m_graphicsDevice;
    FT_Face m_face;
    std::vector<uint8_t> m_glyphBuffer;
    boost::intrusive_ptr<ID3D11Texture2D> m_texture;
    std::vector<Bin> m_bins;
    std::map<uint32_t, Glyph> m_glyphMap;
    bool m_dirty;
};

#endif  // __NYX_FONT_H__
