#pragma once

#include <stdint.h>

namespace donut::math
{

    // a plane equation, so that any point (v) for which (dot(normal, v) == distance) lies on the plane
    struct plane
    {
        float3 normal;
        float distance;

        constexpr plane() : normal(0.f, 0.f, 0.f), distance(0.f) { }
        constexpr plane(const plane &p) : normal(p.normal), distance(p.distance) { }
        constexpr plane(const float3& n, float d) : normal(n), distance(d) { }
        constexpr plane(float x, float y, float z, float d) : normal(x, y, z), distance(d) { }

        plane normalize() const;

        constexpr bool isempty();
    };

    // six planes, normals pointing outside of the volume
    struct frustum
    {
        enum Planes
        {
            NEAR_PLANE = 0,
            FAR_PLANE,
            LEFT_PLANE,
            RIGHT_PLANE,
            TOP_PLANE,
            BOTTOM_PLANE,
            PLANES_COUNT
        };

        enum Corners
        {
            C_LEFT = 0,
            C_RIGHT = 1,
            C_BOTTOM = 0,
            C_TOP = 2,
            C_NEAR = 0,
            C_FAR = 4
        };

        plane planes[PLANES_COUNT];

        frustum() { }

        frustum(const frustum &f);
        frustum(const float4x4 &viewProjMatrix, bool isReverseProjection);

        bool intersectsWith(const float3 &point) const;
        bool intersectsWith(const box3 &box) const;

        static constexpr uint32_t numCorners = 8;
        float3 getCorner(int index) const;

        frustum normalize() const;
        frustum grow(float distance) const;

        bool isempty() const;       // returns true if the frustum trivially rejects all points; does *not* analyze cases when plane equations are mutually exclusive
        bool isopen() const;        // returns true if the frustum has at least one plane that trivially accepts all points
        bool isinfinite() const;    // returns true if the frustum trivially accepts all points

        plane& nearPlane() { return planes[NEAR_PLANE]; }
        plane& farPlane() { return planes[FAR_PLANE]; }
        plane& leftPlane() { return planes[LEFT_PLANE]; }
        plane& rightPlane() { return planes[RIGHT_PLANE]; }
        plane& topPlane() { return planes[TOP_PLANE]; }
        plane& bottomPlane() { return planes[BOTTOM_PLANE]; }

        const plane& nearPlane() const { return planes[NEAR_PLANE]; }
        const plane& farPlane() const { return planes[FAR_PLANE]; }
        const plane& leftPlane() const { return planes[LEFT_PLANE]; }
        const plane& rightPlane() const { return planes[RIGHT_PLANE]; }
        const plane& topPlane() const { return planes[TOP_PLANE]; }
        const plane& bottomPlane() const { return planes[BOTTOM_PLANE]; }

        static frustum empty();    // a frustum that doesn't intersect with any points
        static frustum infinite(); // a frustum that intersects with all points

        static frustum fromBox(const box3& b);
    };

}
