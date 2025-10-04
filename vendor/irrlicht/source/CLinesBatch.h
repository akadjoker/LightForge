#ifndef __C_LINE_BATCH_RENDERER_H_INCLUDED__
#define __C_LINE_BATCH_RENDERER_H_INCLUDED__

#include "IVideoDriver.h"
#include "ILinesBatch.h"
#include "SMaterial.h"
#include "S3DVertex.h"
#include "irrArray.h"
#include "vector3d.h"
#include "SColor.h"

namespace irr
{

    namespace video
    {

        class CLineBatchRenderer : public ILineBatchRenderer
        {
        private:
            IVideoDriver *Driver;

            u32 MaxVertices;
            u32 MaxIndices;
            bool AutoFlush;
            u32 MaxLines;

            u32 CurrentVertexCount;
            u32 CurrentIndexCount;

            SMaterial Material;

            // Buffers para otimização
            scene::IVertexBuffer *VertexBuffer;
            scene::IIndexBuffer *IndexBuffer;
            scene::IMeshBuffer *MeshBuffer;

        public:
            CLineBatchRenderer(IVideoDriver *driver, u32 maxLines = 10000);
            ~CLineBatchRenderer();

            void addLine(const core::vector3df &start, const core::vector3df &end,
                         const SColor &color = SColor(255, 255, 255, 255)) _IRR_OVERRIDE_;

            void addAxes(const core::vector3df &center, f32 length);

            void addLineStrip(const core::array<core::vector3df> &points,
                              const SColor &color = SColor(255, 255, 255, 255)) _IRR_OVERRIDE_;

            void addLineLoop(const core::array<core::vector3df> &points,
                             const SColor &color = SColor(255, 255, 255, 255)) _IRR_OVERRIDE_;

            void addWireBox(const core::aabbox3d<f32> &box,
                            const SColor &color = SColor(255, 255, 255, 255)) _IRR_OVERRIDE_;

            void addGrid(const core::vector3df &center, f32 size, u32 divisions,
                         const SColor &color = SColor(255, 255, 255, 255)) _IRR_OVERRIDE_;

            void addWireSphere(const core::vector3df &center, f32 radius, u32 segments, u32 rings, const SColor &color) _IRR_OVERRIDE_;

            void addCube(const core::vector3df &center, f32 size, const SColor &color) _IRR_OVERRIDE_;

            void addCylinder(const core::vector3df &center, f32 radius, f32 height, const SColor &color, u32 segments);

            void flush() _IRR_OVERRIDE_;

            void render() _IRR_OVERRIDE_;

            void clear() _IRR_OVERRIDE_;
            void setShader(s32 shader) _IRR_OVERRIDE_;

            void addVertex(const core::vector3df &pos, const SColor &color) _IRR_OVERRIDE_;
            void addIndex(u16 index) _IRR_OVERRIDE_;

 
            void setAutoFlush(bool autoFlush) { AutoFlush = autoFlush; }

            u32 getVertexCount() const { return CurrentVertexCount; }
            u32 getLineCount() const { return CurrentIndexCount / 2; }
            bool isEmpty() const { return CurrentVertexCount == 0; }
            bool isFull() const { return CurrentVertexCount >= MaxLines * 2 - 2; }

        private:
            void initializeBuffers();
            void updateBuffers();
            void checkFlush();
        };

    } // end namespace video
} // end namespace irr

#endif // __C_LINE_BATCH_RENDERER_H_INCLUDED__
