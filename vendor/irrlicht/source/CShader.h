
#pragma once




#include "IShader.h"
#include "irrString.h"

namespace irr
{
    namespace video
    {
        class IMaterialRendererServices;
        class SMaterial;
        class CShaderManager;


        class CShader : public IShader
        {
        public:
            CShader(const core::stringc &name );
            virtual ~CShader();
            
            virtual const core::stringc &getName() const override;
            
            virtual s32 getMaterialType() const override;
            
          
            
            virtual void OnSetConstants(IMaterialRendererServices* services, s32 userData, bool updateTransform) override {};
            virtual void OnSetMaterial(const SMaterial& material) override {};

        private:
            friend class CShaderManager;
            core::stringc Name;
      
            s32 MaterialType;
        };

    } // end namespace video
} // end namespace irr

 