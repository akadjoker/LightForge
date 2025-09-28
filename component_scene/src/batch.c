#ifndef __C_LINE_BATCH_RENDERER_H_INCLUDED__
#define __C_LINE_BATCH_RENDERER_H_INCLUDED__

#include "IVideoDriver.h"
#include "SMaterial.h"
#include "S3DVertex.h"
#include "irrArray.h"
#include "vector3d.h"
#include "SColor.h"

namespace irr
{
    namespace video
    {

        struct SLineVertex
        {
            core::vector3df position;
            SColor color;

            SLineVertex() {}
            SLineVertex(const core::vector3df &pos, const SColor &col)
                : position(pos), color(col) {}
        };

        class CLineBatchRenderer
        {
        private:
            IVideoDriver *Driver;
            core::array<SLineVertex> Vertices;
            core::array<u16> Indices;
            SMaterial Material;
            u32 MaxVertices;
            u32 MaxIndices;
            bool AutoFlush;

            // Buffers para otimização
            scene::IVertexBuffer *VertexBuffer;
            scene::IIndexBuffer *IndexBuffer;
            scene::IMeshBuffer *MeshBuffer;
            scene::IVertexDescriptor *VertexDescriptor;

        public:
            CLineBatchRenderer(IVideoDriver * driver, u32 maxLines = 10000);
            ~CLineBatchRenderer();

            // Adiciona uma linha ao batch
            void addLine(const core::vector3df &start, const core::vector3df &end,
                         const SColor &color = SColor(255, 255, 255, 255));

            // Adiciona múltiplas linhas conectadas (line strip)
            void addLineStrip(const core::array<core::vector3df> &points,
                              const SColor &color = SColor(255, 255, 255, 255));

            // Adiciona um loop de linhas (line loop)
            void addLineLoop(const core::array<core::vector3df> &points,
                             const SColor &color = SColor(255, 255, 255, 255));

            // Adiciona uma caixa wireframe
            void addWireBox(const core::aabbox3d<f32> &box,
                            const SColor &color = SColor(255, 255, 255, 255));

            // Adiciona um grid
            void addGrid(const core::vector3df &center, f32 size, u32 divisions,
                         const SColor &color = SColor(255, 255, 255, 255));

            // Renderiza todas as linhas acumuladas
            void flush();

            // Limpa o batch sem renderizar
            void clear();

            // Configurações
            void setAutoFlush(bool autoFlush) { AutoFlush = autoFlush; }
            void setLineWidth(f32 width) { Material.Thickness = width; }
            void setMaterial(const SMaterial &material) { Material = material; }
            SMaterial &getMaterial() { return Material; }

            // Informações
            u32 getVertexCount() const { return Vertices.size(); }
            u32 getLineCount() const { return Indices.size() / 2; }
            bool isEmpty() const { return Vertices.empty(); }
            bool isFull() const { return Vertices.size() >= MaxVertices || Indices.size() >= MaxIndices; }

        private:
            void initializeBuffers();
            void updateBuffers();
            void checkFlush();
        };

        // Implementação

        CLineBatchRenderer::CLineBatchRenderer(IVideoDriver * driver, u32 maxLines)
            : Driver(driver), MaxVertices(maxLines * 2), MaxIndices(maxLines * 2),
              AutoFlush(true), VertexBuffer(nullptr), IndexBuffer(nullptr),
              MeshBuffer(nullptr), VertexDescriptor(nullptr)
        {
            if (!Driver)
                return;

            // Configurar material padrão para linhas
            Material.Lighting = false;
            Material.BackfaceCulling = false;
            Material.FrontfaceCulling = false;
            Material.ZWriteEnable = true;
            Material.ZBuffer = ECFN_LESSEQUAL;
            Material.AntiAliasing = EAAM_LINE_SMOOTH;
            Material.Thickness = 1.0f;

            // Reservar memória
            Vertices.reallocate(MaxVertices);
            Indices.reallocate(MaxIndices);

            initializeBuffers();
        }

        CLineBatchRenderer::~CLineBatchRenderer()
        {
            if (VertexBuffer)
                VertexBuffer->drop();
            if (IndexBuffer)
                IndexBuffer->drop();
            if (MeshBuffer)
                MeshBuffer->drop();
            if (VertexDescriptor)
                VertexDescriptor->drop();
        }

        void CLineBatchRenderer::initializeBuffers()
        {
            if (!Driver)
                return;

            // Criar vertex descriptor para linhas
            VertexDescriptor = Driver->createVertexDescriptor("LineVertex");
            if (VertexDescriptor)
            {
                VertexDescriptor->addAttribute("inPosition", 3, EVAT_FLOAT, EVAS_POSITION, 0);
                VertexDescriptor->addAttribute("inColor", 4, EVAT_UBYTE, EVAS_COLOR, 0);
            }

            // Criar buffers
            VertexBuffer = Driver->createVertexBuffer(VertexDescriptor);
            if (VertexBuffer)
            {
                VertexBuffer->reallocate(MaxVertices);
                VertexBuffer->setHardwareMappingHint(scene::EHM_DYNAMIC);
            }

            IndexBuffer = Driver->createIndexBuffer();
            if (IndexBuffer)
            {
                IndexBuffer->reallocate(MaxIndices);
                IndexBuffer->setHardwareMappingHint(scene::EHM_DYNAMIC);
            }

            // Criar mesh buffer
            MeshBuffer = Driver->createMeshBuffer(VertexDescriptor, VertexBuffer, IndexBuffer);
            if (MeshBuffer)
            {
                MeshBuffer->setPrimitiveType(scene::EPT_LINES);
            }
        }

        void CLineBatchRenderer::addLine(const core::vector3df &start, const core::vector3df &end,
                                         const SColor &color)
        {
            if (!Driver || Vertices.size() + 2 > MaxVertices || Indices.size() + 2 > MaxIndices)
            {
                if (AutoFlush && !isEmpty())
                    flush();
                else
                    return;
            }

            u16 startIndex = Vertices.size();

            // Adicionar vértices
            Vertices.push_back(SLineVertex(start, color));
            Vertices.push_back(SLineVertex(end, color));

            // Adicionar índices
            Indices.push_back(startIndex);
            Indices.push_back(startIndex + 1);

            checkFlush();
        }

        void CLineBatchRenderer::addLineStrip(const core::array<core::vector3df> &points,
                                              const SColor &color)
        {
            if (points.size() < 2)
                return;

            for (u32 i = 0; i < points.size() - 1; ++i)
            {
                addLine(points[i], points[i + 1], color);
            }
        }

        void CLineBatchRenderer::addLineLoop(const core::array<core::vector3df> &points,
                                             const SColor &color)
        {
            if (points.size() < 2)
                return;

            addLineStrip(points, color);
            if (points.size() > 2)
                addLine(points[points.size() - 1], points[0], color);
        }

        void CLineBatchRenderer::addWireBox(const core::aabbox3d<f32> &box, const SColor &color)
        {
           

            core::vector3df edges[8];
            box.getEdges(edges);


            draw3DLine(edges[5], edges[1], color);
            draw3DLine(edges[1], edges[3], color);
            draw3DLine(edges[3], edges[7], color);
            draw3DLine(edges[7], edges[5], color);
            draw3DLine(edges[0], edges[2], color);
            draw3DLine(edges[2], edges[6], color);
            draw3DLine(edges[6], edges[4], color);
            draw3DLine(edges[4], edges[0], color);
            draw3DLine(edges[1], edges[0], color);
            draw3DLine(edges[3], edges[2], color);
            draw3DLine(edges[7], edges[6], color);
            draw3DLine(edges[5], edges[4], color);
        }

        void CLineBatchRenderer::addGrid(const core::vector3df &center, f32 size, u32 divisions,
                                         const SColor &color)
        {
            f32 step = size / divisions;
            f32 halfSize = size * 0.5f;

            // Linhas horizontais
            for (u32 i = 0; i <= divisions; ++i)
            {
                f32 z = center.Z - halfSize + i * step;
                addLine(core::vector3df(center.X - halfSize, center.Y, z),
                        core::vector3df(center.X + halfSize, center.Y, z), color);
            }

            // Linhas verticais
            for (u32 i = 0; i <= divisions; ++i)
            {
                f32 x = center.X - halfSize + i * step;
                addLine(core::vector3df(x, center.Y, center.Z - halfSize),
                        core::vector3df(x, center.Y, center.Z + halfSize), color);
            }
        }

        void CLineBatchRenderer::updateBuffers()
        {
            if (!VertexBuffer || !IndexBuffer || !MeshBuffer)
                return;

            // Atualizar vertex buffer
            if (!Vertices.empty())
            {
                VertexBuffer->setVertices(Vertices.pointer(), Vertices.size());
            }

            // Atualizar index buffer
            if (!Indices.empty())
            {
                IndexBuffer->setIndices(Indices.pointer(), Indices.size());
            }

            MeshBuffer->recalculateBoundingBox();
        }

        void CLineBatchRenderer::flush()
        {
            if (!Driver || isEmpty())
                return;

            updateBuffers();

            // Desenhar
            if (MeshBuffer)
            {
                Driver->setMaterial(Material);
                Driver->drawMeshBuffer(MeshBuffer);
            }

            clear();
        }

        void CLineBatchRenderer::clear()
        {
            if (!MeshBuffer)
                return;

            VertexBuffer->set_used(0);
            IndexBuffer->set_used(0);

            CurrentVertexCount = 0;
            CurrentIndexCount = 0;

            MeshBuffer->getBoundingBox().reset(0, 0, 0);
        }

        void CLineBatchRenderer::checkFlush()
        {
            if (AutoFlush && isFull())
                flush();
        }

    } // end namespace video
} // end namespace irr

#endif // __C_LINE_BATCH_RENDERER_H_INCLUDED__

// Exemplo de uso:
/*
#include "CLineBatchRenderer.h"

void drawScene()
{
    // Criar o batch renderer
    CLineBatchRenderer* lineBatch = new CLineBatchRenderer(driver, 5000);

    // Configurar
    lineBatch->setLineWidth(2.0f);
    lineBatch->setAutoFlush(false); // Controle manual do flush

    // Adicionar várias linhas
    lineBatch->addLine(vector3df(0,0,0), vector3df(10,0,0), SColor(255,255,0,0)); // Linha vermelha
    lineBatch->addLine(vector3df(0,0,0), vector3df(0,10,0), SColor(255,0,255,0)); // Linha verde
    lineBatch->addLine(vector3df(0,0,0), vector3df(0,0,10), SColor(255,0,0,255)); // Linha azul

    // Adicionar um grid
    lineBatch->addGrid(vector3df(0,0,0), 20.0f, 10, SColor(255,128,128,128));

    // Adicionar uma caixa wireframe
    aabbox3d<f32> box(-5, -5, -5, 5, 5, 5);
    lineBatch->addWireBox(box, SColor(255,255,255,0));

    // Renderizar tudo de uma vez
    lineBatch->flush();

    delete lineBatch;
}
*/