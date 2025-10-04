
#ifndef __C_HARDWARE_VERTEX_BUFFER_H__
#define __C_HARDWARE_VERTEX_BUFFER_H__

#include "IHardwareVertexBuffer.h"

namespace irr
{
    namespace video
    {
        class CHardwareVertexBuffer : public IHardwareVertexBuffer
        {
        protected:
            u8 *Data;
            u32 VertexSize;
            u32 NumVertices;
            u32 SizeInBytes;
            E_USAGE Usage;
            bool IsLocked;
 

        public:
            CHardwareVertexBuffer(u32 vertexSize, u32 numVertices, E_USAGE usage);

            virtual ~CHardwareVertexBuffer();


            virtual void upload() =0;
            virtual u32 getId() const ;
            void *lock(E_LOCK_OPTIONS options) override;

            void unlock() override;

            void readData(u32 offset, u32 length, void *dest) override;

            void writeData(u32 offset, u32 length, const void *source, bool discard) override;

            u32 getSizeInBytes() const override { return SizeInBytes; }
            E_USAGE getUsage() const override { return Usage; }
            bool isLocked() const override { return IsLocked; }

            u32 getVertexSize() const override { return VertexSize; }
            u32 getNumVertices() const override { return NumVertices; }
 

            //! Get raw data pointer (for GPU upload)
            u8 *getDataPointer() const { return Data; }
        };

    } // namespace scene
} // namespace irr

#endif