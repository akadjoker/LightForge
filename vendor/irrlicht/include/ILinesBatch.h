#ifndef __I_LINE_BATCH_RENDERER_H_INCLUDED__
#define __I_LINE_BATCH_RENDERER_H_INCLUDED__

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

       

        class ILineBatchRenderer : public virtual IReferenceCounted
        {

        public:
            virtual void addLine(const core::vector3df &start, const core::vector3df &end, const SColor &color = SColor(255, 255, 255, 255)) = 0;
            virtual void addLineStrip(const core::array<core::vector3df> &points, const SColor &color = SColor(255, 255, 255, 255)) = 0;
            virtual void addLineLoop(const core::array<core::vector3df> &points, const SColor &color = SColor(255, 255, 255, 255)) = 0;
            virtual void addWireBox(const core::aabbox3d<f32> &box, const SColor &color = SColor(255, 255, 255, 255)) = 0;
            virtual void addGrid(const core::vector3df &center, f32 size, u32 divisions, const SColor &color = SColor(255, 255, 255, 255)) = 0;

            virtual void flush() = 0;
            virtual void clear() = 0;
        };

    } // end namespace video
} // end namespace irr

#endif // __C_LINE_BATCH_RENDERER_H_INCLUDED__
