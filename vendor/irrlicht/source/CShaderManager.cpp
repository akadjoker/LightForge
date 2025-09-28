#include "CShaderManager.h"
#include "IShader.h"
#include "CShader.h"
#include "IVideoDriver.h"
#include "IFileSystem.h"
#include "IGPUProgrammingServices.h"
#include "IMaterialRenderer.h"
#include "IMaterialRendererServices.h"
#include "SMaterial.h"
#include "IReadFile.h"
#include "irrOS.h"

namespace irr
{
    namespace video
    {

        CShaderManager::CShaderManager(IVideoDriver *driver, io::IFileSystem *fs)
        {
            Driver = driver;
            FileSystem = fs;
            if (FileSystem)
                FileSystem->grab();
            GPU = Driver->getGPUProgrammingServices();
            os::Printer::log("Shader Manager created");
        }

        CShaderManager::~CShaderManager()
        {
            clear();
            if (FileSystem)
                FileSystem->drop();
            FileSystem = nullptr;
            Driver = nullptr;
            GPU = nullptr;
            os::Printer::log("Shader Manager destroyed");
        }

        void CShaderManager::clear()
        {
            auto it = Shaders.getIterator();
            for (; !it.atEnd(); it++)
            {
                auto *shader = it.getNode()->getValue();
                if (shader)
                {
                    delete shader;
                }
            }
            Shaders.clear();
        }

        core::stringc CShaderManager::loadShaderFromFile(const core::stringc &filename)
        {
            if (!FileSystem)
                return "";

            io::IReadFile *file = FileSystem->createAndOpenFile(filename);
            if (!file)
                return "";

            const u32 size = file->getSize();
            if (size == 0)
            {
                file->drop();
                return "";
            }

            char *buffer = new char[size + 1];
            file->read(buffer, size);
            buffer[size] = 0;

            core::stringc result(buffer);
            delete[] buffer;
            file->drop();

            return result;
        }

        IShader *CShaderManager::createShaderFromFiles(const core::stringc &name, const c8 *vertexFile, const c8 *fragmentFile)
        {
            if (!Driver || !GPU)
            {
                os::Printer::log("No video driver or GPU programming services available, cannot create shader", name, ELL_ERROR);
                return nullptr;
            }

            auto *shaderIt = Shaders.find(name);
            if (shaderIt)
            {
                os::Printer::log("Shader with name already exists", name, ELL_WARNING);
                return shaderIt->getValue();
            }

            core::stringc vertexSource = loadShaderFromFile(vertexFile);
            core::stringc fragmentSource = loadShaderFromFile(fragmentFile);

            if (vertexSource.size() == 0 || fragmentSource.size() == 0)
                return nullptr;

            return createShader(name, vertexSource.c_str(), fragmentSource.c_str());
        }
        IShader *CShaderManager::getShader(const core::stringc &name)
        {
            auto *shaderIt = Shaders.find(name);
            if (shaderIt)
                return shaderIt->getValue();
            return nullptr;
        }

        bool CShaderManager::removeShader(const core::stringc &name)
        {
            auto *shaderIt = Shaders.find(name);
            if (shaderIt)
            {
                auto *shader = shaderIt->getValue();
                if (shader)
                {
                    delete shader;
                }
                Shaders.remove(name);
                return true;
            }
            return false;
        }

        u32 CShaderManager::getShaderCount() const
        {
            return Shaders.size();
        }

        IShader *CShaderManager::createShader(const core::stringc &name, const c8 *vertexSource, const c8 *fragmentSource)
        {
            if (!Driver || !GPU)
            {
                os::Printer::log("No video driver or GPU programming services available, cannot create shader", name, ELL_ERROR);
                return nullptr;
            }

            auto *shaderIt = Shaders.find(name);
            if (shaderIt)
            {
                os::Printer::log("Shader with name already exists", name, ELL_WARNING);
                return shaderIt->getValue();
            }

            CShader *shader = new CShader(name);
            s32 materialType = -1;

            s32 shaderMaterial = GPU->addHighLevelShaderMaterial(
                vertexSource,   "main", video::EVST_VS_4_0,   // Vertex shader
                fragmentSource, "main", video::EPST_PS_4_0, // Fragment shader
                shader,                                    // Callback
                video::EMT_SOLID, 0                         // Base material
            );
            if (shaderMaterial == -1)
            {
                os::Printer::log("Could not add high level shader material", name, ELL_ERROR);
                delete shader;
                return nullptr;
            }

            shader->MaterialType = shaderMaterial;
            Shaders.insert(name, shader);

            return shader;
        }

    }
}