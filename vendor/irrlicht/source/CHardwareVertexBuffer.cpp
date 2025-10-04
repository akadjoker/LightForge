

#include "CHardwareVertexBuffer.h"
#include <cstring>
namespace irr
{
    namespace video
    {
        CHardwareVertexBuffer::CHardwareVertexBuffer(u32 vertexSize, u32 numVertices, E_USAGE usage)
            : VertexSize(vertexSize), NumVertices(numVertices), Usage(usage), IsLocked(false) 
        {
            SizeInBytes = vertexSize * numVertices;
            Data = new u8[SizeInBytes];
            memset(Data, 0, SizeInBytes);
        }

        CHardwareVertexBuffer::~CHardwareVertexBuffer()
        {
            delete[] Data;
        }

        void *CHardwareVertexBuffer::lock(E_LOCK_OPTIONS options)
        {
            if (IsLocked)
                return nullptr;
            IsLocked = true;
            return Data;
        }

        void CHardwareVertexBuffer::unlock()
        {
            if (!IsLocked)
                return;
            IsLocked = false;
        }

        void CHardwareVertexBuffer::readData(u32 offset, u32 length, void *dest)
        {
            if (offset + length > SizeInBytes)
                return;
            memcpy(dest, Data + offset, length);
        }

        void CHardwareVertexBuffer::writeData(u32 offset, u32 length, const void *source, bool discard)
        {
            if (offset + length > SizeInBytes)
                return;
            memcpy(Data + offset, source, length);
        }

         u32 CHardwareVertexBuffer::getId() const 
        {
            return 0;
        }


    } // namespace scene
} // namespace irr
