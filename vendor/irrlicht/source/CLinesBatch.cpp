

#include "IVideoDriver.h"
#include "CLinesBatch.h"
#include "SMaterial.h"
#include "S3DVertex.h"
#include "irrArray.h"
#include "vector3d.h"
#include "SColor.h"
#include "CMeshBuffer.h"
#include "SMesh.h"
#include "IMesh.h"

namespace irr
{
    namespace video
    {

        CLineBatchRenderer::CLineBatchRenderer(IVideoDriver *driver, u32 maxLines)
            : Driver(driver), MaxVertices(maxLines * 2), MaxIndices(maxLines * 2),
              AutoFlush(true), MaxLines(maxLines), VertexBuffer(nullptr), IndexBuffer(nullptr), CurrentVertexCount(0),
              CurrentIndexCount(0),
              MeshBuffer(nullptr)
        {
            if (!Driver)
            {
                return;
            }

            Material.Lighting = false;
            Material.BackfaceCulling = false;
            Material.FrontfaceCulling = false;
            Material.ZWriteEnable = true;
            Material.ZBuffer = ECFN_LESSEQUAL;
            Material.AntiAliasing = EAAM_LINE_SMOOTH;
            Material.Thickness = 1.0f;

            initializeBuffers();
        }

        CLineBatchRenderer::~CLineBatchRenderer()
        {
            delete MeshBuffer;
        }

        void CLineBatchRenderer::initializeBuffers()
        {
            if (!Driver)
                return;

            MeshBuffer =nullptr;// new irr::scene::CMeshBuffer<video::S3DLinesVertex>(Driver->getVertexDescriptor(7));

            IndexBuffer = MeshBuffer->getIndexBuffer();
            VertexBuffer = MeshBuffer->getVertexBuffer(0);

            IndexBuffer->reallocate(MaxIndices);
            VertexBuffer->reallocate(MaxVertices);

            MeshBuffer->setPrimitiveType(scene::EPT_LINES);
        }

        void CLineBatchRenderer::addVertex(const core::vector3df &pos, const SColor &color)
        {
            if (!MeshBuffer || CurrentVertexCount >= MaxLines * 2)
                return;

            video::S3DLinesVertex vertex(pos, color);
            VertexBuffer->addVertex(&vertex);
            MeshBuffer->getBoundingBox().addInternalPoint(pos);
            CurrentVertexCount++;
        }

        void CLineBatchRenderer::addIndex(u16 index)
        {
            if (!MeshBuffer || CurrentIndexCount >= MaxLines * 2)
                return;

            IndexBuffer->addIndex(index);
            ++CurrentIndexCount;
        }

        void CLineBatchRenderer::addLine(const core::vector3df &start, const core::vector3df &end, const SColor &color)
        {

            if (!Driver || isFull())
            {
                if (AutoFlush && !isEmpty())
                    flush();
                else
                    return;
            }

            u16 startIndex = CurrentVertexCount;

            addVertex(start, color);
            addVertex(end, color);

            addIndex(startIndex);
            addIndex(startIndex + 1);

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

            addLine(edges[5], edges[1], color);
            addLine(edges[1], edges[3], color);
            addLine(edges[3], edges[7], color);
            addLine(edges[7], edges[5], color);
            addLine(edges[0], edges[2], color);
            addLine(edges[2], edges[6], color);
            addLine(edges[6], edges[4], color);
            addLine(edges[4], edges[0], color);
            addLine(edges[1], edges[0], color);
            addLine(edges[3], edges[2], color);
            addLine(edges[7], edges[6], color);
            addLine(edges[5], edges[4], color);

        }

        void CLineBatchRenderer::addGrid(const core::vector3df &center, f32 size, u32 divisions,const SColor &color)
        {
            f32 step = size / divisions;
            f32 halfSize = size * 0.5f;

            // Linhas paralelas ao eixo X (variando Z)
            for (u32 i = 0; i <= divisions; ++i)
            {
                f32 z = center.Z - halfSize + i * step;
                addLine(core::vector3df(center.X - halfSize, center.Y, z),
                        core::vector3df(center.X + halfSize, center.Y, z), color);
            }

            // Linhas paralelas ao eixo Z (variando X)
            for (u32 i = 0; i <= divisions; ++i)
            {
                f32 x = center.X - halfSize + i * step;
                addLine(core::vector3df(x, center.Y, center.Z - halfSize),
                        core::vector3df(x, center.Y, center.Z + halfSize), color);
            }
        }
        void CLineBatchRenderer::addAxes(const core::vector3df &center, f32 length)
        {
            // Eixo X - Vermelho
            addLine(center, center + core::vector3df(length, 0, 0), SColor(255, 255, 0, 0));

            // Eixo Y - Verde
            addLine(center, center + core::vector3df(0, length, 0), SColor(255, 0, 255, 0));

            // Eixo Z - Azul
            addLine(center, center + core::vector3df(0, 0, length), SColor(255, 0, 0, 255));
        }

        void CLineBatchRenderer::addCube(const core::vector3df &center, f32 size, const SColor &color)
        {
            f32 half = size * 0.5f;

            core::vector3df vertices[8] = {
                center + core::vector3df(-half, -half, -half), // 0: inferior-esquerda-trás
                center + core::vector3df(half, -half, -half),  // 1: inferior-direita-trás
                center + core::vector3df(half, half, -half),   // 2: superior-direita-trás
                center + core::vector3df(-half, half, -half),  // 3: superior-esquerda-trás
                center + core::vector3df(-half, -half, half),  // 4: inferior-esquerda-frente
                center + core::vector3df(half, -half, half),   // 5: inferior-direita-frente
                center + core::vector3df(half, half, half),    // 6: superior-direita-frente
                center + core::vector3df(-half, half, half)    // 7: superior-esquerda-frente
            };

            // Arestas do cubo (12 linhas)
            u16 edges[24] =
                {
                    // Face traseira
                    0, 1, 1, 2, 2, 3, 3, 0,
                    // Face frontal
                    4, 5, 5, 6, 6, 7, 7, 4,
                    // Conectar frente e trás
                    0, 4, 1, 5, 2, 6, 3, 7};

            // Desenhar todas as arestas
            for (int i = 0; i < 24; i += 2)
            {
                addLine(vertices[edges[i]], vertices[edges[i + 1]], color);
            }
        }

        void CLineBatchRenderer::addCylinder(const core::vector3df &center, f32 radius, f32 height, const SColor &color, u32 segments)
        {
            f32 halfHeight = height * 0.5f;
            core::vector3df topCenter = center + core::vector3df(0, halfHeight, 0);
            core::vector3df bottomCenter = center + core::vector3df(0, -halfHeight, 0);

            core::array<core::vector3df> topVertices;
            core::array<core::vector3df> bottomVertices;

            for (u32 seg = 0; seg <= segments; seg++)
            {
                f32 theta = 2.0f * core::PI * seg / segments;
                f32 x = radius * cos(theta);
                f32 z = radius * sin(theta);

                topVertices.push_back(topCenter + core::vector3df(x, 0, z));
                bottomVertices.push_back(bottomCenter + core::vector3df(x, 0, z));
            }

            for (u32 seg = 0; seg < segments; seg++)
            {
                addLine(topVertices[seg], topVertices[seg + 1], color);
            }

            for (u32 seg = 0; seg < segments; seg++)
            {
                addLine(bottomVertices[seg], bottomVertices[seg + 1], color);
            }

            for (u32 seg = 0; seg < segments; seg += segments / 8)
            {
                addLine(topVertices[seg], bottomVertices[seg], color);
            }
        }
        void CLineBatchRenderer::addWireSphere(const core::vector3df &center, f32 radius, u32 segments, u32 rings, const SColor &color)
        {

            core::array<core::vector3df> vertices;
            vertices.reallocate((rings + 1) * (segments + 1));

            for (u32 ring = 0; ring <= rings; ring++)
            {
                f32 phi = core::PI * ring / rings; // Ângulo vertical (0 a PI)
                f32 y = cos(phi);
                f32 ringRadius = sin(phi);

                for (u32 seg = 0; seg <= segments; seg++)
                {
                    f32 theta = 2.0f * core::PI * seg / segments; // Ângulo horizontal (0 a 2PI)
                    f32 x = ringRadius * cos(theta);
                    f32 z = ringRadius * sin(theta);

                    core::vector3df vertex = center + core::vector3df(x, y, z) * radius;
                    vertices.push_back(vertex);
                }
            }

            // Desenhar linhas horizontais (anéis)
            for (u32 ring = 0; ring <= rings; ring++)
            {
                for (u32 seg = 0; seg < segments; seg++)
                {
                    u32 current = ring * (segments + 1) + seg;
                    u32 next = ring * (segments + 1) + (seg + 1);

                    addLine(vertices[current], vertices[next], color);
                }
            }

            // Desenhar linhas verticais (meridianos)
            for (u32 seg = 0; seg <= segments; seg++)
            {
                for (u32 ring = 0; ring < rings; ring++)
                {
                    u32 current = ring * (segments + 1) + seg;
                    u32 below = (ring + 1) * (segments + 1) + seg;

                    addLine(vertices[current], vertices[below], color);
                }
            }
        }

        void CLineBatchRenderer::updateBuffers()
        {
            if (!VertexBuffer || !IndexBuffer || !MeshBuffer)
                return;

            MeshBuffer->recalculateBoundingBox();
        }

        void CLineBatchRenderer::render()
        {
            flush();
        }

        void CLineBatchRenderer::flush()
        {
            if (!Driver || isEmpty())
                return;

            updateBuffers();

            // Desenhar
            if (MeshBuffer)
            {
                // printf("darw lines %u %u \n", CurrentVertexCount, CurrentIndexCount);
                Driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
                Driver->setMaterial(Material);
                Driver->drawMeshBuffer(MeshBuffer);
            }

            clear();
        }

        void CLineBatchRenderer::clear()
        {
            if (!VertexBuffer || !IndexBuffer || !MeshBuffer)
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

        void CLineBatchRenderer::setShader(s32 shader)
        {
            Material.MaterialType = (video::E_MATERIAL_TYPE)shader;
        }

    } // end namespace video
} // end namespace irr

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