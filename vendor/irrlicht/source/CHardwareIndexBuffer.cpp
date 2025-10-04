

#include "CHardwareIndexBuffer.h"

#include <cstring>

namespace irr
{
    namespace video
    {

        CHardwareIndexBuffer::CHardwareIndexBuffer(E_INDEX_TYPE type, u32 numIndexes, E_USAGE usage)
            : IndexType(type), NumIndexes(numIndexes), Usage(usage), IsLocked(false) 
        {
            u32 indexSize = (type == EIT_16BIT) ? sizeof(u16) : sizeof(u32);
            SizeInBytes = indexSize * numIndexes;
            Data = new u8[SizeInBytes];
            memset(Data, 0, SizeInBytes);
        }

        CHardwareIndexBuffer::~CHardwareIndexBuffer()
        {
            delete[] Data;
        }

        void *CHardwareIndexBuffer::lock(E_LOCK_OPTIONS options)
        {
            if (IsLocked)
                return nullptr;
            IsLocked = true;
            return Data;
        }

        void CHardwareIndexBuffer::unlock()
        {
            if (!IsLocked)
                return;
            IsLocked = false;
        }

        void CHardwareIndexBuffer::readData(u32 offset, u32 length, void *dest)
        {
            if (offset + length > SizeInBytes)
                return;
            memcpy(dest, Data + offset, length);
        }

        void CHardwareIndexBuffer::writeData(u32 offset, u32 length, const void *source, bool discard)
        {
            if (offset + length > SizeInBytes)
                return;
            memcpy(Data + offset, source, length);
        }
         u32 CHardwareIndexBuffer::getId() const 
        {
            return 0;
        }

        

    } // namespace scene
} // namespace irr
