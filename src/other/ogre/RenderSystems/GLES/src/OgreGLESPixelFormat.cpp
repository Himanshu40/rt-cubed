/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreGLESPixelFormat.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreBitwise.h"


namespace Ogre  {
    GLenum GLESPixelUtil::getGLOriginFormat(PixelFormat mFormat)
    {
        switch (mFormat)
        {
            case PF_A8:
                return GL_ALPHA;

            case PF_L8:
            case PF_L16:
            case PF_FLOAT16_R:
            case PF_FLOAT32_R:
                return GL_LUMINANCE;
                return GL_LUMINANCE;

            case PF_BYTE_LA:
            case PF_SHORT_GR:
            case PF_FLOAT16_GR:
            case PF_FLOAT32_GR:
                return GL_LUMINANCE_ALPHA;

            case PF_R3G3B2:
            case PF_R5G6B5:
            case PF_FLOAT16_RGB:
            case PF_FLOAT32_RGB:
            case PF_SHORT_RGB:
                return GL_RGB;

            case PF_X8R8G8B8:
            case PF_A8R8G8B8:
            case PF_B8G8R8A8:
            case PF_A1R5G5B5:
            case PF_A4R4G4B4:
            case PF_A2R10G10B10:
                return GL_BGRA_PVR;

            case PF_X8B8G8R8:
            case PF_A8B8G8R8:
            case PF_A2B10G10R10:
            case PF_FLOAT16_RGBA:
            case PF_FLOAT32_RGBA:
            case PF_SHORT_RGBA:
                return GL_RGBA;

#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
            // Formats are in native endian, so R8G8B8 on little endian is
            // BGR, on big endian it is RGB.
            case PF_R8G8B8:
                return GL_RGB;
            case PF_B8G8R8:
                return 0;
#else
            case PF_R8G8B8:
                return 0;
            case PF_B8G8R8:
                return GL_RGB;
#endif
            case PF_DXT1:
            case PF_DXT3:
            case PF_DXT5:
            case PF_B5G6R5:
            default:
                return 0;
        }
    }

    GLenum GLESPixelUtil::getGLOriginDataType(PixelFormat mFormat)
    {
        switch (mFormat)
        {
            case PF_A8:
            case PF_L8:
            case PF_R8G8B8:
            case PF_B8G8R8:
            case PF_BYTE_LA:
                return GL_UNSIGNED_BYTE;
            case PF_R5G6B5:
            case PF_B5G6R5:
                return GL_UNSIGNED_SHORT_5_6_5;
            case PF_L16:
                return GL_UNSIGNED_SHORT;

#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
            case PF_X8B8G8R8:
            case PF_A8B8G8R8:
                return GL_UNSIGNED_INT_8_8_8_8_REV;
            case PF_X8R8G8B8:
            case PF_A8R8G8B8:
                return GL_UNSIGNED_INT_8_8_8_8_REV;
            case PF_B8G8R8A8:
                return GL_UNSIGNED_BYTE;
            case PF_R8G8B8A8:
                return GL_UNSIGNED_BYTE;
#else
            case PF_X8B8G8R8:
            case PF_A8B8G8R8:
                return GL_UNSIGNED_BYTE;
            case PF_X8R8G8B8:
            case PF_A8R8G8B8:
                return GL_UNSIGNED_BYTE;
            case PF_B8G8R8A8:
            case PF_R8G8B8A8:
                return 0;
#endif

            case PF_FLOAT32_R:
            case PF_FLOAT32_GR:
            case PF_FLOAT32_RGB:
            case PF_FLOAT32_RGBA:
                return GL_FLOAT;
            case PF_SHORT_RGBA:
            case PF_SHORT_RGB:
            case PF_SHORT_GR:
                return GL_UNSIGNED_SHORT;

            case PF_A2R10G10B10:
            case PF_A2B10G10R10:
            case PF_FLOAT16_R:
            case PF_FLOAT16_GR:
            case PF_FLOAT16_RGB:
            case PF_FLOAT16_RGBA:
            case PF_R3G3B2:
            case PF_A1R5G5B5:
            case PF_A4R4G4B4:
                // TODO not supported
            default:
                return 0;
        }
    }

    GLenum GLESPixelUtil::getGLInternalFormat(PixelFormat fmt, bool hwGamma)
    {
        switch (fmt)
        {
            case PF_L8:
                return GL_LUMINANCE;

            case PF_A8:
                return GL_ALPHA;

            case PF_BYTE_LA:
                return GL_LUMINANCE_ALPHA;

            case PF_R8G8B8:
            case PF_B8G8R8:
            case PF_X8B8G8R8:
            case PF_X8R8G8B8:
                if (!hwGamma)
                {
                    return GL_RGB;
                }
            case PF_A8R8G8B8:
            case PF_B8G8R8A8:
                if (!hwGamma)
                {
                    return GL_RGBA;
                }
            case PF_A4L4:
            case PF_L16:
            case PF_A4R4G4B4:
            case PF_R3G3B2:
            case PF_A1R5G5B5:
            case PF_R5G6B5:
            case PF_B5G6R5:
            case PF_A2R10G10B10:
            case PF_A2B10G10R10:
            case PF_FLOAT16_R:
            case PF_FLOAT16_RGB:
            case PF_FLOAT16_GR:
            case PF_FLOAT16_RGBA:
            case PF_FLOAT32_R:
            case PF_FLOAT32_GR:
            case PF_FLOAT32_RGB:
            case PF_FLOAT32_RGBA:
            case PF_SHORT_RGBA:
            case PF_SHORT_RGB:
            case PF_SHORT_GR:
            case PF_DXT1:
            case PF_DXT3:
            case PF_DXT5:
                // TODO not supported
            default:
                return 0;
        }
    }

    GLenum GLESPixelUtil::getClosestGLInternalFormat(PixelFormat mFormat,
                                                   bool hwGamma)
    {
        GLenum format = getGLInternalFormat(mFormat, hwGamma);
        if (format==0)
        {
            if (hwGamma)
            {
                // TODO not supported
                return 0;
            }
            else
            {
                return GL_RGBA;
            }
        }
        else
        {
            return format;
        }
    }

    PixelFormat GLESPixelUtil::getClosestOGREFormat(GLenum fmt)
    {
        switch (fmt)
        {
            case GL_LUMINANCE:
                return PF_L8;
            case GL_ALPHA:
                return PF_A8;
            case GL_LUMINANCE_ALPHA:
                return PF_BYTE_LA;
            case GL_RGB:
                return PF_X8R8G8B8;
            case GL_RGBA:
                return PF_A8R8G8B8;
            case GL_BGRA_PVR:
            default:
                //TODO: not supported
                return PF_A8R8G8B8;
        };
    }

    size_t GLESPixelUtil::getMaxMipmaps(size_t width, size_t height, size_t depth,
                                      PixelFormat format)
    {
        size_t count = 0;

        do {
            if (width > 1)
            {
                width = width / 2;
            }
            if (height > 1)
            {
                height = height / 2;
            }
            if (depth > 1)
            {
                depth = depth / 2;
            }
            /*
            NOT needed, compressed formats will have mipmaps up to 1x1
            if(PixelUtil::isValidExtent(width, height, depth, format))
                count ++;
            else
                break;
            */
            count++;
        } while (!(width == 1 && height == 1 && depth == 1));

        return count;
    }

    size_t GLESPixelUtil::optionalPO2(size_t value)
    {
        const RenderSystemCapabilities *caps =
            Root::getSingleton().getRenderSystem()->getCapabilities();

        if (caps->hasCapability(RSC_NON_POWER_OF_2_TEXTURES))
        {
            return value;
        }
        else
        {
            return Bitwise::firstPO2From((uint32)value);
        }
    }

    PixelBox* GLESPixelUtil::convertToGLformat(const PixelBox &data,
                                             GLenum *outputFormat)
    {
        GLenum glFormat = GLESPixelUtil::getGLOriginFormat(data.format);
        if (glFormat != 0)
        {
            // format already supported
            return new PixelBox(data);
        }

        PixelBox *converted = 0;

        if (data.format == PF_R8G8B8)
        {
            // Convert BGR -> RGB
            converted->format = PF_B8G8R8;
            *outputFormat = GL_RGB;

            converted = new PixelBox(data);
            uint32 *data = (uint32 *) converted->data;
            for (int i = 0; i < converted->getWidth() * converted->getHeight(); i++)
            {
                uint32 *color = data;
                *color = (*color & 0x000000ff) << 16 |
                         (*color & 0x0000FF00) |
                         (*color & 0x00FF0000) >> 16;
                data += 1;
            }
        }

        return converted;
    }
}
