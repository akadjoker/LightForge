#ifndef __C_SHADER_H_INCLUDED__
#define __C_SHADER_H_INCLUDED__


#include "IShaderManager.h"
#include "IVideoDriver.h"
#include "IFileSystem.h"
#include "irrMap.h"
#include "irrString.h"

namespace irr
{
    namespace video
    {
        class IMaterialRendererServices;
        class IVideoDriver;
        class SMaterial;
        class IGPUProgrammingServices;
        class IShader;
  

        class CShaderManager : public IShaderManager
        {
        public:
            CShaderManager(IVideoDriver *driver, io::IFileSystem *fs);
            virtual ~CShaderManager();
            virtual IShader *createShader(const  core::stringc &name,
                                          const  c8* vertexSource,
                                          const  c8* fragmentSource) override;
            virtual IShader *createShaderFromFiles(const  core::stringc &name,
                                                   const  c8* vertexFile,
                                                   const  c8* fragmentFile) override;
            virtual IShader *getShader(const  core::stringc &name) override;
            virtual bool removeShader(const   core::stringc &name) override;
            virtual u32 getShaderCount() const override;

            virtual void clear() override;

        private:
            core::map<core::stringc, IShader *> Shaders;
            IVideoDriver *Driver;
            io::IFileSystem *FileSystem;
            IGPUProgrammingServices *GPU;
            core::stringc loadShaderFromFile(const core::stringc& filename);
        };
    } // end namespace video
} // end namespace irr

#endif