
#ifndef __I_HARDWARE_BUFFER_H__
#define __I_HARDWARE_BUFFER_H__

#include "IReferenceCounted.h"

namespace irr
{
namespace video
{
	enum E_USAGE
	{
		//! Static buffer - rarely modified
		HBU_STATIC = 1,
		//! Dynamic buffer - frequently modified
		HBU_DYNAMIC = 2,
		//! Write only access
		HBU_WRITE_ONLY = 4,
		//! Combinations
		HBU_STATIC_WRITE_ONLY = HBU_STATIC | HBU_WRITE_ONLY,
		HBU_DYNAMIC_WRITE_ONLY = HBU_DYNAMIC | HBU_WRITE_ONLY
	};
	
	enum E_LOCK_OPTIONS
	{
		//! Normal lock
		HBL_NORMAL,
		//! Discard previous contents
		HBL_DISCARD,
		//! Read only
		HBL_READ_ONLY,
		//! No overwrite (append only)
		HBL_NO_OVERWRITE
	};
	
    class IHardwareBuffer : public virtual IReferenceCounted
    {
    public:
        virtual ~IHardwareBuffer() {}
        
        //! Lock buffer for reading/writing
        virtual void* lock(E_LOCK_OPTIONS options) = 0;
        
        //! Unlock buffer
        virtual void unlock() = 0;
        
        //! Read data from buffer
        virtual void readData(u32 offset, u32 length, void* dest) = 0;
        
        //! Write data to buffer
        virtual void writeData(u32 offset, u32 length, const void* source, bool discardWholeBuffer = false) = 0;
        
        //! Get size in bytes
        virtual u32 getSizeInBytes() const = 0;
        
        //! Get usage flags
        virtual E_USAGE getUsage() const = 0;
        
        //! Check if locked
        virtual bool isLocked() const = 0;

		virtual void upload() =0;

		virtual void bind()=0;
		virtual void unbind()=0;
        
 
 
    };



} // namespace scene
} // namespace irr

#endif