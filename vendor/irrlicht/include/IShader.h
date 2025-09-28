
#ifndef __I_SHADER_H_INCLUDED__
#define __I_SHADER_H_INCLUDED__


#include "IReferenceCounted.h"
#include "IShaderConstantSetCallBack.h"
#include "irrString.h"
#include "IMaterialRendererServices.h"


namespace irr
{
    namespace video
    {
        class IShader : public virtual IReferenceCounted, public IShaderConstantSetCallBack
        {
        public:
            virtual ~IShader() {}
            virtual const core::stringc &getName() const = 0;
            virtual s32 getMaterialType() const = 0;
        };

    }
}

#endif