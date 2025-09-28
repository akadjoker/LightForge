#include "utils.h"

namespace irr
{
    namespace core
    {

        // reference from Unity Engine
        // https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector3.cs

        core::vector3df lerp(const core::vector3df &a, const core::vector3df &b, float t)
        {
            t = core::clamp(t, 0.0f, 1.0f);

            return core::vector3df(
                a.X + (b.X - a.X) * t,
                a.Y + (b.Y - a.Y) * t,
                a.Z + (b.Z - a.Z) * t);
        }

        core::vector3df slerp(const core::vector3df &a, const core::vector3df &b, float t)
        {
            // https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
            // Dot product - the cosine of the angle between 2 vectors.
            float dot = a.dotProduct(b);

            // Clamp it to be in the range of Acos()
            // This may be unnecessary, but floating point
            // precision can be a fickle mistress.
            dot = core::clamp(dot, -1.0f, 1.0f);

            // Acos(dot) returns the angle between start and end,
            // And multiplying that by percent returns the angle between
            // start and the final result.
            float theta = acosf(dot) * t;
            core::vector3df relativeVec = b - a * dot;
            relativeVec.normalize();

            // Orthonormal basis
            // The final result.
            core::vector3df ret = (a * cosf(theta)) + (relativeVec * sinf(theta));
            return ret;
        }

        core::vector3df reflect(const core::vector3df &inDirection, const core::vector3df &normal)
        {
            float factor = -2.0f * normal.dotProduct(inDirection);
            return core::vector3df(
                factor * normal.X + inDirection.X,
                factor * normal.Y + inDirection.Y,
                factor * normal.Z + inDirection.Z);
        }

        float angle(const core::vector3df &a, const core::vector3df &b)
        {
            float denominator = (float)sqrtf(a.getLengthSQ() * b.getLengthSQ());
            if (denominator < FLT_EPSILON)
                return 0.0f;

            float dot = a.dotProduct(b) / denominator;
            return ((float)acosf(dot));
        }

        core::vector3df project(const core::vector3df &vector, const core::vector3df &onNormal)
        {
            float sqrMag = onNormal.dotProduct(onNormal);
            if (sqrMag < FLT_EPSILON)
                return core::vector3df();
            else
            {
                float dot = vector.dotProduct(onNormal);
                return core::vector3df(onNormal.X * dot / sqrMag,
                                       onNormal.Y * dot / sqrMag,
                                       onNormal.Z * dot / sqrMag);
            }
        }

        core::vector3df projectOnPlane(const core::vector3df &vector, const core::vector3df &planeNormal)
        {
            float sqrMag = planeNormal.dotProduct(planeNormal);
            if (sqrMag < FLT_EPSILON)
                return core::vector3df();
            else
            {
                float dot = vector.dotProduct(planeNormal);
                return core::vector3df(vector.X - planeNormal.X * dot / sqrMag,
                                       vector.Y - planeNormal.Y * dot / sqrMag,
                                       vector.Z - planeNormal.Z * dot / sqrMag);
            }
        }

#define kEpsilon 1.0e-5f
#define equivalent(x, y, fudge) ((x > y) ? (x - y <= fudge) : (y - x <= fudge))

        bool isParallel(const core::vector3df &v1, const core::vector3df &v2)
        {
            core::vector3df a, b;
            a = v1;
            a.normalize();

            b = v2;
            b.normalize();

            float dotPrd = fabs(a.dotProduct(b));
            return equivalent(dotPrd, 1.0f, kEpsilon);
        }

        core::vector3df quaternionToEuler(const core::quaternion &q)
        {
            core::vector3df angles;
            q.toEuler(angles);

            const float epsilon = 0.001f;
            if ((fabsf(angles.X - core::PI) < epsilon || fabsf(angles.X + core::PI) < epsilon) &&
                (fabsf(angles.Z - core::PI) < epsilon || fabsf(angles.Z + core::PI) < epsilon))
            {
                angles.X = 0.0f;
                angles.Y = core::PI - angles.Y;
                angles.Z = 0.0f;
            }

            if ((fabsf(angles.Z - core::PI) < epsilon || fabsf(angles.Z + core::PI) < epsilon) &&
                (fabsf(angles.Y) < epsilon))
            {
                angles.X = -angles.X;
                angles.Y = 0.0f;
                angles.Z = 0.0f;
            }

            return angles;
        }

        core::quaternion getQuaternionFromAxes(const core::vector3df &xAxis, const core::vector3df &yAxis, const core::vector3df &zAxis)
        {
            // copilot
            // Can you write function get quaternion from 3 vector axis 3d in C
            core::quaternion q;

            // Compute the trace of the matrix
            float trace = xAxis.X + yAxis.Y + zAxis.Z;

            if (trace > 0.0f)
            {
                float s = sqrtf(trace + 1.0f) * 2.0f;
                q.W = 0.25f * s;
                q.X = (yAxis.Z - zAxis.Y) / s;
                q.Y = (zAxis.X - xAxis.Z) / s;
                q.Z = (xAxis.Y - yAxis.X) / s;
            }
            else if ((xAxis.X > yAxis.Y) && (xAxis.X > zAxis.Z))
            {
                float s = sqrtf(1.0f + xAxis.X - yAxis.Y - zAxis.Z) * 2.0f;
                q.W = (yAxis.Z - zAxis.Y) / s;
                q.X = 0.25f * s;
                q.Y = (yAxis.X + xAxis.Y) / s;
                q.Z = (zAxis.X + xAxis.Z) / s;
            }
            else if (yAxis.Y > zAxis.Z)
            {
                float s = sqrtf(1.0f + yAxis.Y - xAxis.X - zAxis.Z) * 2.0f;
                q.W = (zAxis.X - xAxis.Z) / s;
                q.X = (yAxis.X + xAxis.Y) / s;
                q.Y = 0.25f * s;
                q.Z = (zAxis.Y + yAxis.Z) / s;
            }
            else
            {
                float s = sqrtf(1.0f + zAxis.Z - xAxis.X - yAxis.Y) * 2.0f;
                q.W = (xAxis.Y - yAxis.X) / s;
                q.X = (zAxis.X + xAxis.Z) / s;
                q.Y = (zAxis.Y + yAxis.Z) / s;
                q.Z = 0.25f * s;
            }

            return q;
        }

    } // end namespace core
} // end namespace irr