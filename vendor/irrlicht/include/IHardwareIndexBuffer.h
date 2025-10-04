// IHardwareIndexBuffer.h
#ifndef __I_HARDWARE_INDEX_BUFFER_H__
#define __I_HARDWARE_INDEX_BUFFER_H__

#include "IHardwareBuffer.h"
#include "IIndexBuffer.h"

namespace irr
{
namespace video
{
    
    class IHardwareIndexBuffer : public IHardwareBuffer
    {
    public:
        virtual ~IHardwareIndexBuffer() {}
        
        //! Get index type
        virtual E_INDEX_TYPE getType() const = 0;
        
        //! Get number of indices
        virtual u32 getNumIndexes() const = 0;
        
        //! Get size of one index
        virtual u32 getIndexSize() const = 0;

        virtual void upload() =0;

        virtual void bind()=0;
		virtual void unbind()=0;
    };
    
 

} // namespace scene
} // namespace irr

#endif