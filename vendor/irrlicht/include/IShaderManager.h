#ifndef IRRLICHT_SHADER_MANAGER_H
#define IRRLICHT_SHADER_MANAGER_H

#include "EShaderTypes.h"
#include "EMaterialTypes.h"
#include "EPrimitiveTypes.h"
#include "IReferenceCounted.h"
#include "irrString.h"
#include "irrMap.h"

namespace irr
{

    namespace scene
    {
        class ISceneManager;
    }

    namespace io
    {
        class IFileSystem;
    }

    namespace video
    {

        class IShader;

        class IShaderManager : public virtual IReferenceCounted
        {
        public:
            //! Destructor
            virtual ~IShaderManager() {}

            //! Create a shader from source strings
            virtual IShader *createShader(const core::stringc &name,
                                          const c8* vertexSource,
                                          const c8* fragmentSource) = 0;

            //! Create shader from files
            virtual IShader *createShaderFromFiles(const core::stringc &name,
                                                   const c8* vertexFile,
                                                   const c8* fragmentFile) = 0;

            //! Get shader by name
            virtual IShader *getShader(const core::stringc  &name) = 0;

            //! Remove shader by name
            virtual bool removeShader(const core::stringc  &name) = 0;

            //! Get number of loaded shaders
            virtual u32 getShaderCount() const = 0;

         

            //! Clear all shaders
            virtual void clear() = 0;
 
        };

    } // end namespace video

}

#endif