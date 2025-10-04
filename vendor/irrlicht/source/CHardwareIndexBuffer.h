// CHardwareIndexBuffer.h
#ifndef __C_HARDWARE_INDEX_BUFFER_H__
#define __C_HARDWARE_INDEX_BUFFER_H__

#include "IHardwareIndexBuffer.h"

namespace irr
{
    namespace video
    {

        class CHardwareIndexBuffer : public IHardwareIndexBuffer
        {
        protected:
            u8 *Data;
            E_INDEX_TYPE IndexType;
            u32 NumIndexes;
            u32 SizeInBytes;
            E_USAGE Usage;
            bool IsLocked;
 

        public:
            CHardwareIndexBuffer(E_INDEX_TYPE type, u32 numIndexes, E_USAGE usage);
            virtual ~CHardwareIndexBuffer();


            virtual u32 getId() const ;

            virtual void bind() =0;
	        virtual void unbind()=0;
            virtual void upload() =0;

            void *lock(E_LOCK_OPTIONS options) override;

            void unlock() override;

            void readData(u32 offset, u32 length, void *dest) override;

            void writeData(u32 offset, u32 length, const void *source, bool discard) override;

            u32 getSizeInBytes() const override { return SizeInBytes; }
            E_USAGE getUsage() const override { return Usage; }
            bool isLocked() const override { return IsLocked; }

            E_INDEX_TYPE getType() const override { return IndexType; }
            u32 getNumIndexes() const override { return NumIndexes; }
            u32 getIndexSize() const override
            {
                return (IndexType == EIT_16BIT) ? sizeof(u16) : sizeof(u32);
            }

   

            //! Get raw data pointer (for GPU upload)
            u8 *getDataPointer() const { return Data; }
        };

       

    } // namespace scene
} // namespace irr

#endif