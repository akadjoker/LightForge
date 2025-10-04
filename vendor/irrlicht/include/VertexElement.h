// VertexElement.h
#ifndef __VERTEX_ELEMENT_H__
#define __VERTEX_ELEMENT_H__

#include "irrTypes.h"

namespace irr
{
namespace video
{
    enum E_VERTEX_ELEMENT_TYPE
    {
        VET_FLOAT1 = 0,
        VET_FLOAT2,
        VET_FLOAT3,
        VET_FLOAT4,
        VET_COLOUR,
        VET_SHORT1,
        VET_SHORT2,
        VET_SHORT3,
        VET_SHORT4,
        VET_UBYTE4,
        VET_UBYTE4_NORM
    };
    
    enum E_VERTEX_ELEMENT_SEMANTIC
    {
        VES_POSITION = 1,
        VES_BLEND_WEIGHTS = 2,
        VES_BLEND_INDICES = 3,
        VES_NORMAL = 4,
        VES_DIFFUSE = 5,
        VES_SPECULAR = 6,
        VES_TEXTURE_COORDINATES = 7,
        VES_BINORMAL = 8,
        VES_TANGENT = 9
    };
    
    struct VertexElement
    {
        //! Which vertex buffer (for multi-stream)
        u16 Source;
        
        //! Offset in bytes from start of vertex
        u32 Offset;
        
        //! Type of element
        E_VERTEX_ELEMENT_TYPE Type;
        
        //! Semantic meaning
        E_VERTEX_ELEMENT_SEMANTIC Semantic;
        
        //! Semantic index (for multiple texcoords, etc)
        u16 Index;
        
        //! Get size of this element in bytes
        u32 getSize() const
        {
            switch(Type)
            {
                case VET_FLOAT1: return sizeof(f32);
                case VET_FLOAT2: return sizeof(f32) * 2;
                case VET_FLOAT3: return sizeof(f32) * 3;
                case VET_FLOAT4: return sizeof(f32) * 4;
                case VET_COLOUR: return sizeof(u32);
                case VET_SHORT1: return sizeof(s16);
                case VET_SHORT2: return sizeof(s16) * 2;
                case VET_SHORT3: return sizeof(s16) * 3;
                case VET_SHORT4: return sizeof(s16) * 4;
                case VET_UBYTE4: return sizeof(u8) * 4;
                case VET_UBYTE4_NORM: return sizeof(u8) * 4;
                default: return 0;
            }
        }
        
        //! Get element count (for glVertexAttribPointer)
        u32 getElementCount() const
        {
            switch(Type)
            {
                case VET_FLOAT1: return 1;
                case VET_FLOAT2: return 2;
                case VET_FLOAT3: return 3;
                case VET_FLOAT4: return 4;
                case VET_COLOUR: return 4;
                case VET_SHORT1: return 1;
                case VET_SHORT2: return 2;
                case VET_SHORT3: return 3;
                case VET_SHORT4: return 4;
                case VET_UBYTE4: return 4;
                case VET_UBYTE4_NORM: return 4;
                default: return 0;
            }
        }
    };

} // namespace scene
} // namespace irr

#endif