
#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>

#include "glad.h" 

// #define GL_GLEXT_PROTOTYPES
// #include <SDL2/SDL_opengl_glext.h>
#include <irrlicht.h>
#include "CLinesBatch.h"
#include <iostream>
#include <typeinfo>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;

static bool ABORT = false;

class CShaderCallBack : public video::IShaderConstantSetCallBack
{

public:
    CShaderCallBack()
    {
    }

    virtual void OnSetConstants(video::IMaterialRendererServices *services, s32 userData, bool updateTransform) override

    {
        if (!services)
            return;

        // Obter matrizes
        core::matrix4 world = services->getVideoDriver()->getTransform(video::ETS_WORLD);
        core::matrix4 view = services->getVideoDriver()->getTransform(video::ETS_VIEW);
        core::matrix4 projection = services->getVideoDriver()->getTransform(video::ETS_PROJECTION);

        // Calcular matriz MVP
        core::matrix4 mvp = projection * view * world;

        // // Enviar para vertex shader
        s32 index = services->getVertexShaderConstantID("uMVPMatrix");
        if (index == -1)
            std::cout << "uMVPMatrix index: " << index << std::endl;
        services->setVertexShaderConstant(index, mvp.pointer(), 16);

        index = services->getVertexShaderConstantID("uWorldMatrix");
        services->setVertexShaderConstant(index, world.pointer(), 16);
        if (index == -1)
            std::cout << "uWorldMatrix index: " << index << std::endl;

        s32 tex = 0;
        s32 idSampler = services->getPixelShaderConstantID("uTexture0");
        services->setPixelShaderConstant(idSampler, &tex, 1);

        int useTex = false;
        s32 idUseTex = services->getPixelShaderConstantID("uUseTexture");
        services->setPixelShaderConstant(idUseTex, &useTex, 1);

        // Luz ambiente + direcional
        core::vector3df lightDir(0.5f, -1.0f, -0.3f); // direcção da luz
        lightDir.normalize();
        float pack[3] = {lightDir.X, lightDir.Y, lightDir.Z};
        index = services->getPixelShaderConstantID("uLightDir");
        if (index == -1)
            std::cout << "uLightDir index: " << index << std::endl;
        services->setPixelShaderConstant(index, &pack[0], 3);

        video::SColorf lightColor(1.0f, 1.0f, 1.0f, 1.0f);
        index = services->getPixelShaderConstantID("uLightColor");
        if (index == -1)
            std::cout << "uLightColor index: " << index << std::endl;
        services->setPixelShaderConstant(index, &lightColor.r, 4);

        video::SColorf ambient(0.2f, 0.2f, 0.2f, 1.0f);
        index = services->getPixelShaderConstantID("uAmbient");
        if (index == -1)
            std::cout << "uAmbient index: " << index << std::endl;
        services->setPixelShaderConstant(index, &ambient.r, 4);
    }
};



class CShaderLinesCallBack : public video::IShaderConstantSetCallBack
{

public:
    CShaderLinesCallBack()
    {
    }

    virtual void OnSetConstants(video::IMaterialRendererServices *services, s32 userData, bool updateTransform) override

    {
        if (!services)
            return;

        // Obter matrizes
        core::matrix4 world = services->getVideoDriver()->getTransform(video::ETS_WORLD);
        core::matrix4 view = services->getVideoDriver()->getTransform(video::ETS_VIEW);
        core::matrix4 projection = services->getVideoDriver()->getTransform(video::ETS_PROJECTION);

        // Calcular matriz MVP
        core::matrix4 mvp = projection * view * world;

        // // Enviar para vertex shader
        s32 index = services->getVertexShaderConstantID("uMVPMatrix");
        if (index == -1)
            std::cout << "uMVPMatrix index: " << index << std::endl;
        services->setVertexShaderConstant(index, mvp.pointer(), 16);

        
    }
};


// Vertex Shader GLSL
const c8 *VertexShader = R"(
 


layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;
layout (location = 3) in vec2 inTexCoord;


	
uniform mat4 uMVPMatrix;
uniform mat4 uWorldMatrix;

out vec4 vColor;
out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vWorldPos;

void main()
{
    gl_Position = uMVPMatrix * vec4(inPosition, 1.0);

    vColor = inColor;
    vTexCoord = inTexCoord;

    mat3 normalMat = transpose(inverse(mat3(uWorldMatrix)));
    vNormal = normalize(normalMat * inNormal);


    vWorldPos = (uWorldMatrix * vec4(inPosition, 1.0)).xyz;
}

)";

// Fragment Shader GLSL
const c8 *FragmentShader = R"(
 

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vWorldPos;

uniform sampler2D uTexture0;
uniform bool uUseTexture;

// Luz
uniform vec3 uLightDir;    // direção da luz
uniform vec4 uLightColor;  // cor da luz
uniform vec4 uAmbient;     // luz ambiente

out vec4 FragColor;

void main()
{
    vec4 texColor = vec4(1.0);
    if (uUseTexture)
        texColor = texture(uTexture0, vTexCoord);

    // Normalizar
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);

    // Difusa simples
    float NdotL = max(dot(N, L), 0.0);

    vec4 diffuse = uLightColor * NdotL;
    vec4 ambient = uAmbient;

    vec4 finalColor = clamp((ambient + diffuse) * vColor * texColor, 0.0, 1.0);


    FragColor = finalColor;
}

)";

const c8 *VertexShaderLines = R"(
 


layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;


	
uniform mat4 uMVPMatrix;

out vec4 vColor;

void main()
{
    gl_Position = uMVPMatrix * vec4(inPosition, 1.0);

    vColor = inColor;

}

)";

// Fragment Shader GLSL
const c8 *FragmentShaderLines = R"(
 

in vec4 vColor;

out vec4 FragColor;

void main()
{


    FragColor = vColor;
}

)";

class MyEventReceiver : public IEventReceiver
{
public:
    virtual bool OnEvent(const SEvent &event) override
    {
        if (event.EventType == irr::EET_KEY_INPUT_EVENT)
        {
            KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
            if (event.KeyInput.Key == irr::KEY_ESCAPE && !event.KeyInput.PressedDown)
                ABORT = true;
        }
        else if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
        {
        
        }
        return false;
    }

    
    virtual bool IsKeyDown(EKEY_CODE keyCode) const
    {
        return KeyIsDown[keyCode];
    }
 

    MyEventReceiver()
    {
        
    }

private:
    bool KeyIsDown[KEY_KEY_CODES_COUNT];
 
};


const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

int main()
{
    // Setup do device
    MyEventReceiver receiver;
    SIrrlichtCreationParameters params;
    params.DriverType = video::EDT_OPENGL;
    params.WindowSize = core::dimension2du(1024, 768);
    params.Bits = 32;
    params.AntiAlias = 4; // Anti-aliasing
    params.Stencilbuffer = false;
    params.Vsync = true; // VSync para smoother rendering
    params.EventReceiver = &receiver;

    IrrlichtDevice *device = createDeviceEx(params);
    if (!device)
    {
        std::cerr << "Failed to create device\n";
        return 1;
    }

    video::IVideoDriver *driver = device->getVideoDriver();
    scene::ISceneManager *smgr = device->getSceneManager();

    video::IGPUProgrammingServices *gpu = driver->getGPUProgrammingServices();
    if (!gpu)
    {
        std::cerr << "GPU Programming Services not available!\n";
        device->drop();
        return 2;
    }

  

    CShaderCallBack *shaderCallback = new CShaderCallBack();

    // Criar material com shaders customizadas
    s32 shaderMaterial = gpu->addHighLevelShaderMaterial(
        VertexShader, "main", video::EVST_VS_4_0,   // Vertex shader
        FragmentShader, "main", video::EPST_PS_4_0, // Fragment shader
        shaderCallback,                             // Callback
        video::EMT_SOLID, 0                         // Base material
    );

    shaderCallback->drop();

    if (shaderMaterial == -1)
    {
        std::cerr << "Failed to create shader material!\n";
        device->drop();
        return 3;
    }

    CShaderLinesCallBack * shadeLinesCallback = new CShaderLinesCallBack();
    s32 shaderLinesMaterial = gpu->addHighLevelShaderMaterial(
        VertexShaderLines, "main", video::EVST_VS_4_0,   // Vertex shader
        FragmentShaderLines, "main", video::EPST_PS_4_0, // Fragment shader
        shadeLinesCallback,                             // Callback
        video::EMT_SOLID, 0                         // Base material
    );
    shadeLinesCallback->drop();



    std::cout << "Shader material created successfully! Type: " << shaderMaterial << std::endl;
    std::cout << "Shader lines material created successfully! Type: " << shaderLinesMaterial << std::endl;

    // Verificar se temos geometry creator
    if (!smgr->getGeometryCreator())
    {
        std::cerr << "GeometryCreator not available\n";
        device->drop();
        return 2;
    }

    ICameraSceneNode *camera = smgr->addCameraSceneNode(nullptr, core::vector3df(0, 5, -15), core::vector3df(0, 0, 0));
   //  camera->addComponent<FpsComponent>();
    camera->addComponent<FreeCameraComponent>();


    camera->setFOV(core::PI / 3.0f);
    camera->setAspectRatio((f32)params.WindowSize.Width / (f32)params.WindowSize.Height);
    camera->setNearValue(0.1f);
    camera->setFarValue(1000.0f);

    u32 frames = 0;
    u32 lastFPS = 0;
    f32 time=0;

    s32 LastAnimationTime = device->getTimer()->getRealTime();
 
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) 
    {
        fprintf(stderr, "Falha ao carregar GL com GLAD\n");
        return 1;
    }


     float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned short indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

   video::IHardwareIndexBuffer *ib  =    driver->createIndexBuffer(video::EIT_16BIT,  6 ,video::HBU_STATIC);
   video::IHardwareVertexBuffer *vb =    driver->createVertexBuffer(3 * sizeof(float),4,video::HBU_STATIC);

vb->writeData(/*offsetBytes*/0, /*sizeBytes*/ sizeof(vertices), vertices);
ib->writeData(/*offsetBytes*/0, /*sizeBytes*/ sizeof(indices),  indices);



    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    



    // Loop principal
    while (device->run() && !ABORT)
    {

        f32 timeDiff = (f32)(device->getTimer()->getRealTime() - LastAnimationTime) / 1000.0f;
        LastAnimationTime = device->getTimer()->getRealTime();
        time+=timeDiff;

        driver->beginScene(true, true, video::SColor(255, 20, 20, 40));

        smgr->update();

        smgr->drawAll();


        glUseProgram(shaderProgram);


   vb->bind();
   ib->bind();
   
 glEnableVertexAttribArray(0);
glVertexAttribPointer(
    /*location*/ 0,
    /*size*/     3,
    /*type*/     GL_FLOAT,
    /*norm*/     GL_FALSE,
    /*stride*/   3 * sizeof(float),
    /*offset*/   (void*)0
);

// 6) Draw
glDrawElements(GL_TRIANGLES, /*count*/ 6, /*type*/ GL_UNSIGNED_SHORT, /*offset*/ (void*)0);

// 7) Unbind/cleanup mínimos
glDisableVertexAttribArray(0);
   
   vb->unbind();
   ib->unbind();


        glUseProgram(0);
        

  
 
        driver->endScene();

        // FPS counter
        if (++frames == 60)
        {
            u32 currentFPS = driver->getFPS();
            if (currentFPS != lastFPS)
            {
                core::stringw title = L"Skylicht Engine - Enhanced Demo [";
                title += driver->getName();
                title += L"] FPS: ";
                title += (s32)currentFPS;

                device->setWindowCaption(title.c_str());
                lastFPS = currentFPS;
            }
            frames = 0;
        }
    }

    

    std::cout << "Shutting down...\n";

   ib->drop();
   vb->drop();

    device->drop();
    return 0;
}