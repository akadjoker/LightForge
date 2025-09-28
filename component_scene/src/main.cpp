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

    auto *cube = smgr->addCube(2.0f, nullptr, 20, core::vector3df(0, 0, 0), core::vector3df(0, 0, 0), core::vector3df(1.0f, 1.0f, 1.0f));
    if (cube)
    {
        cube->getComponent<scene::MeshComponent>()->setShaderMaterial(shaderMaterial);
        cube->getComponent<scene::MeshComponent>()->setColor(255,0,0);

    }

    auto *sphere = smgr->addSphere(1.5f, 16, cube, -1, core::vector3df(5, 0, 0), core::vector3df(0, 0, 0), core::vector3df(1.0f, 1.0f, 1.0f));
    if (sphere)
    {
        sphere->getComponent<scene::MeshComponent>()->setShaderMaterial(shaderMaterial);
        sphere->getComponent<scene::MeshComponent>()->setColor(255,0,255);
    }

    camera->setFOV(core::PI / 3.0f);
    camera->setAspectRatio((f32)params.WindowSize.Width / (f32)params.WindowSize.Height);
    camera->setNearValue(0.1f);
    camera->setFarValue(1000.0f);

    u32 frames = 0;
    u32 lastFPS = 0;
    f32 time=0;

    s32 LastAnimationTime = device->getTimer()->getRealTime();

    CLineBatchRenderer lineBatch(driver,80000);
    lineBatch.setShader(shaderLinesMaterial);

    
    u32 bounce=0;

    // Loop principal
    while (device->run() && !ABORT)
    {

        f32 timeDiff = (f32)(device->getTimer()->getRealTime() - LastAnimationTime) / 1000.0f;
        LastAnimationTime = device->getTimer()->getRealTime();
        time+=timeDiff;

        driver->beginScene(true, true, video::SColor(255, 20, 20, 40));

        smgr->update();

        smgr->drawAll();

        
        TransformComponent* transform = cube->getComponent<TransformComponent>();
        
        
        f32 angle = time * 8.0f* core::DEGTORAD;
        transform->rotation.Y = (angle *25.0f) * core::RADTODEG;  
        transform->rotation.Z = (-angle *25.0f) * core::RADTODEG;  
 
        
        transform->position.X = sin(angle)*5.5;
        transform->position.Z = cos(angle)*5.5;



        lineBatch.addGrid(vector3df(0,-1,0), 20.0f, 10, SColor(255,100,100,100));
        lineBatch.addAxes(vector3df(0,0,0), 1.0f);

        lineBatch.addWireBox(sphere->getTransformedBoundingBox(), SColor(255, 255, 40, 128)); 
        lineBatch.addWireBox(cube->getTransformedBoundingBox(), SColor(255, 255, 40, 128)); 

    
        
        lineBatch.addWireSphere(vector3df(0, 0, 0), 1.6f, 16, 8,SColor(255,255,255,0));
 

        lineBatch.addCylinder(vector3df(2, 0, 0),1.0f,5,SColor(255,255,255,0),18);

        lineBatch.addWireBox(camera->getViewFrustum()->getBoundingBox(), SColor(255, 255, 0, 128)); 
   

       


        lineBatch.render();

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

    device->drop();
    return 0;
}