#include "CSpriteBatchRenderer.h"

namespace irr
{
    namespace video
    {

        using namespace core;
        using namespace scene;

        CSpriteBatchRenderer::CSpriteBatchRenderer(IVideoDriver *driver, u32 maxSprites)
            : Driver(driver),
              MaxSprites(maxSprites),
              CurrentVertexCount(0),
              CurrentIndexCount(0),
              VertexBuffer(nullptr),
              IndexBuffer(nullptr),
              MeshBuffer(nullptr),
              CurrentTexture(nullptr),
              AutoFlush(true),
              Mode(BILLBOARD_CAMERA_FACING)
        {
            if (!Driver)
                return;

            Material.Lighting = false;
            Material.BackfaceCulling = false;
            Material.FrontfaceCulling = false;
            Material.ZWriteEnable = true;
            Material.ZBuffer = ECFN_LESSEQUAL;
            Material.AntiAliasing = EAAM_FULL_BASIC;

            initializeBuffers();
        }

        CSpriteBatchRenderer::~CSpriteBatchRenderer()
        {
            delete MeshBuffer; // meshbuffer owns VB/IB
        }

        void CSpriteBatchRenderer::initializeBuffers()
        {
            if (!Driver)
                return;

            // S3DVertex: pos, normal, color, texcoord
            MeshBuffer = new scene::CMeshBuffer<video::S3DVertex>(Driver->getVertexDescriptor(0));
            MeshBuffer->setPrimitiveType(scene::EPT_TRIANGLES);

            VertexBuffer = MeshBuffer->getVertexBuffer(0);
            IndexBuffer = MeshBuffer->getIndexBuffer();

            // 4 vértices por sprite, 6 índices por sprite
            VertexBuffer->reallocate(MaxSprites * 4);
            IndexBuffer->reallocate(MaxSprites * 6);
        }

        void CSpriteBatchRenderer::setTexture(ITexture *tex)
        {
            if (CurrentTexture == tex)
                return;
            // textura mudou — desenha o que estiver pendente
            if (!isEmpty())
                flush();
            CurrentTexture = tex;
            Material.setTexture(0, CurrentTexture);
        }

        void CSpriteBatchRenderer::setShader(s32 shader)
        {
            Material.MaterialType = (E_MATERIAL_TYPE)shader;
        }

        void CSpriteBatchRenderer::updateBuffers()
        {
            if (!VertexBuffer || !IndexBuffer || !MeshBuffer)
                return;
            MeshBuffer->recalculateBoundingBox();
        }

        void CSpriteBatchRenderer::flush()
        {
            if (!Driver || isEmpty())
                return;

            updateBuffers();

            Driver->setMaterial(Material);
            Driver->drawMeshBuffer(MeshBuffer);

            clear();
        }

        void CSpriteBatchRenderer::clear()
        {
            if (!VertexBuffer || !IndexBuffer || !MeshBuffer)
                return;

            VertexBuffer->set_used(0);
            IndexBuffer->set_used(0);
            CurrentVertexCount = 0;
            CurrentIndexCount = 0;

            MeshBuffer->getBoundingBox().reset(0, 0, 0);
        }

        void CSpriteBatchRenderer::checkFlush()
        {
            if (AutoFlush && isFull())
                flush();
        }

        void CSpriteBatchRenderer::buildBillboardBasis(ICameraSceneNode *cam,
                                                       vector3df &right,
                                                       vector3df &up)
        {
            // Usa a base da câmera (right, up) para billboard
            right = cam->getViewFrustum()->cameraRight; // disponível no frustum
            up = cam->getUpVector();

            right.normalize();
            up.normalize();
        }

        void CSpriteBatchRenderer::rotateAxes(vector3df &r, vector3df &u, f32 deg)
        {
            if (deg == 0.f)
                return;
            const f32 rad = deg * DEGTORAD;
            const f32 cs = cos(rad);
            const f32 sn = sin(rad);
            const vector3df r0 = r, u0 = u;
            // rotação no plano r-u (2D)
            r = r0 * cs + u0 * sn;
            u = -r0 * sn + u0 * cs;
        }

        void CSpriteBatchRenderer::addQuad(const vector3df v[4],
                                           const SColor &c,
                                           const core::vector2df t[4])
        {
            if (!MeshBuffer)
                return;

            // baseIndex é o índice de início dos 4 vértices deste quad
            const u16 baseIndex = (u16)CurrentVertexCount;

            // Vértices (v0..v3)
            S3DVertex verts[4] = {
                S3DVertex(v[0], vector3df(0, 0, -1), c, t[0]),
                S3DVertex(v[1], vector3df(0, 0, -1), c, t[1]),
                S3DVertex(v[2], vector3df(0, 0, -1), c, t[2]),
                S3DVertex(v[3], vector3df(0, 0, -1), c, t[3]),
            };

            for (int i = 0; i < 4; ++i)
            {
                VertexBuffer->addVertex(&verts[i]);
                MeshBuffer->getBoundingBox().addInternalPoint(verts[i].Pos);
            }

            // Índices: 2 triângulos (0,1,2) (0,2,3)
            const u16 idx[6] = {
                (u16)(baseIndex + 0),
                (u16)(baseIndex + 1),
                (u16)(baseIndex + 2),
                (u16)(baseIndex + 0),
                (u16)(baseIndex + 2),
                (u16)(baseIndex + 3)};
            for (int i = 0; i < 6; ++i)
                IndexBuffer->addIndex(idx[i]);

            CurrentVertexCount += 4;
            CurrentIndexCount += 6;

            checkFlush();
        }

        void CSpriteBatchRenderer::addSpriteWorldAxisAligned(const vector3df &center,
                                                             const vector3df &axisRight,
                                                             const vector3df &axisUp,
                                                             f32 halfWidth, f32 halfHeight,
                                                             const SColor &color,
                                                             const core::rect<f32> &uv,
                                                             f32 rotationDegrees)
        {
            if (!Driver)
                return;
            if (isFull())
            {
                if (AutoFlush && !isEmpty())
                    flush();
                else
                    return;
            }

            vector3df r = axisRight;
            r.normalize();
            vector3df u = axisUp;
            u.normalize();

            rotateAxes(r, u, rotationDegrees);

            // Canto do quad em torno do centro
            vector3df v[4];
            v[0] = center + (-r * halfWidth) + (-u * halfHeight); // BL
            v[1] = center + (r * halfWidth) + (-u * halfHeight);  // BR
            v[2] = center + (r * halfWidth) + (u * halfHeight);   // TR
            v[3] = center + (-r * halfWidth) + (u * halfHeight);  // TL

            vector2df t[4] = {
                {uv.UpperLeftCorner.X, uv.LowerRightCorner.Y},  // BL
                {uv.LowerRightCorner.X, uv.LowerRightCorner.Y}, // BR
                {uv.LowerRightCorner.X, uv.UpperLeftCorner.Y},  // TR
                {uv.UpperLeftCorner.X, uv.UpperLeftCorner.Y}    // TL
            };

            addQuad(v, color, t);
        }

        void CSpriteBatchRenderer::addSpriteWorld(const vector3df &center,
                                                  f32 halfWidth, f32 halfHeight,
                                                  const SColor &color,
                                                  const core::rect<f32> &uv,
                                                  ICameraSceneNode *cam,
                                                  f32 rotationDegrees)
        {
            if (!cam)
                return;

            vector3df r, u;
            if (Mode == BILLBOARD_CAMERA_FACING)
            {
                buildBillboardBasis(cam, r, u);
            }
            else
            {
                r.set(1, 0, 0);
                u.set(0, 1, 0);
            }

            addSpriteWorldAxisAligned(center, r, u, halfWidth, halfHeight, color, uv, rotationDegrees);
        }

    } // namespace video
} // namespace irr
