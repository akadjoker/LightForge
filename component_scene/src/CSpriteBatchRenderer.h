#ifndef __C_SPRITE_BATCH_RENDERER_H_INCLUDED__
#define __C_SPRITE_BATCH_RENDERER_H_INCLUDED__

#include "IVideoDriver.h"
#include "SMaterial.h"
#include "S3DVertex.h"
#include "irrArray.h"
#include "vector3d.h"
#include "SColor.h"
#include "CMeshBuffer.h"
#include "IMesh.h"
#include "ICameraSceneNode.h"

namespace irr
{
    namespace video
    {

        class CSpriteBatchRenderer
        {
        public:
            enum BillboardMode
            {
                // Sprite orientado para a câmara (usa right/up da câmara)
                BILLBOARD_CAMERA_FACING,
                // Sprite num plano fixo (XY local) – útil p/ HUD 3D ou manual
                BILLBOARD_AXIS_ALIGNED
            };

        private:
            IVideoDriver *Driver;

            u32 MaxSprites;
            u32 CurrentVertexCount; // em vértices
            u32 CurrentIndexCount;  // em índices

            scene::IVertexBuffer *VertexBuffer;
            scene::IIndexBuffer *IndexBuffer;
            scene::IMeshBuffer *MeshBuffer;

            SMaterial Material;
            ITexture *CurrentTexture;
            bool AutoFlush;

            BillboardMode Mode;

        public:
            CSpriteBatchRenderer(IVideoDriver *driver, u32 maxSprites = 8192);
            ~CSpriteBatchRenderer();

            // Config
            void setTexture(ITexture *tex); // troca de textura => flush automático
            void setShader(s32 shader);     // MaterialType
            void setAutoFlush(bool v) { AutoFlush = v; }
            void setBillboardMode(BillboardMode m) { Mode = m; }

            // Adiciona um sprite no mundo (center), tamanho (metade = halfSize), rotação opcional
            // uv: retângulo [0..1] no atlas
            void addSpriteWorld(const core::vector3df &center,
                                f32 halfWidth, f32 halfHeight,
                                const SColor &color,
                                const core::rect<f32> &uv,
                                scene::ICameraSceneNode *cam,
                                f32 rotationDegrees = 0.0f);

            // Versão “plano fixo” (eixos fornecidos)
            void addSpriteWorldAxisAligned(const core::vector3df &center,
                                           const core::vector3df &axisRight,
                                           const core::vector3df &axisUp,
                                           f32 halfWidth, f32 halfHeight,
                                           const SColor &color,
                                           const core::rect<f32> &uv,
                                           f32 rotationDegrees = 0.0f);

            // Renderiza o batch acumulado
            void flush();

            // Limpa sem desenhar
            void clear();

            // Info
            bool isEmpty() const { return CurrentVertexCount == 0; }
            bool isFull() const { return CurrentVertexCount + 4 > MaxSprites * 4; }

        private:
            void initializeBuffers();
            void updateBuffers();
            void checkFlush();
            void addQuad(const core::vector3df v[4],
                         const SColor &c,
                         const core::vector2df t[4]);

            // helpers
            static void buildBillboardBasis(scene::ICameraSceneNode *cam,
                                            core::vector3df &right,
                                            core::vector3df &up);
            static void rotateAxes(core::vector3df &r, core::vector3df &u, f32 deg);
        };

    } // namespace video
} // namespace irr

#endif // __C_SPRITE_BATCH_RENDERER_H_INCLUDED__
