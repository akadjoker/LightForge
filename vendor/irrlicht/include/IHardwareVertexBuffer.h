// IHardwareVertexBuffer.h
#ifndef __I_HARDWARE_VERTEX_BUFFER_H__
#define __I_HARDWARE_VERTEX_BUFFER_H__

#include "IHardwareBuffer.h"

namespace irr
{
namespace video
{
    class IHardwareVertexBuffer : public IHardwareBuffer
    {
    public:
        virtual ~IHardwareVertexBuffer() {}
        
        //! Get size of one vertex in bytes
        virtual u32 getVertexSize() const = 0;
        
        //! Get number of vertices
        virtual u32 getNumVertices() const = 0;

        virtual u32 getId() const = 0;

        virtual void bind() =0;
	    virtual void unbind()=0;
    };
    
 

} // namespace scene
} // namespace irr

#endif