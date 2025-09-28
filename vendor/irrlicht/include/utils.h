#pragma once

#include "plane3d.h"
#include "vector3d.h"
#include "line3d.h"
#include "aabbox3d.h"
#include "matrix4.h"
#include "quaternion.h"
#include "IVideoDriver.h"

namespace irr
{
namespace core
{

         static core::vector3df lerp(const core::vector3df& a, const core::vector3df& b, float t);

		static core::vector3df slerp(const core::vector3df& a, const core::vector3df& b, float t);

		static core::vector3df reflect(const core::vector3df& inDirection, const core::vector3df& normal);

		static float angle(const core::vector3df& a, const core::vector3df& b);

		static core::vector3df project(const core::vector3df& vector, const core::vector3df& onNormal);

		static core::vector3df projectOnPlane(const core::vector3df& vector, const core::vector3df& planeNormal);

		static bool isParallel(const core::vector3df& a, const core::vector3df& b);

		static core::vector3df quaternionToEuler(const core::quaternion& q);

		static core::quaternion getQuaternionFromAxes(const core::vector3df& xAxis, const core::vector3df& yAxis, const core::vector3df& zAxis);

} // end namespace core
} // end namespace irr