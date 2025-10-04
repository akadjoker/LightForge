
#ifndef __I_ARRAY_HARDWARE_BUFFER_H__
#define __I_ARRAY_HARDWARE_BUFFER_H__

#include "IReferenceCounted.h"
#include "IVertexDescriptor.h"

namespace irr
{
    namespace video
    {
        class IHardwareIndexBuffer;
        class IHardwareVertexBuffer;

        class IVertexArray : public virtual IReferenceCounted
        {
        public:
            virtual ~IVertexArray() {}

            virtual IVertexDescriptor *getVertexDescriptor() = 0;

            virtual void bind() = 0;
            virtual void unbind() = 0;

            virtual bool isValid() const = 0;
            virtual void rebuild() = 0;
        };

    } // namespace scene
} // namespace irr

#endif