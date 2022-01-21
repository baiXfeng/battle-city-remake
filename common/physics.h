//
// Created by baifeng on 2021/12/1.
//

#ifndef SDL2_UI_PHYSICS_H
#define SDL2_UI_PHYSICS_H

#include "macro.h"
#include "vector2.h"
#include <box2d/box2d.h>
#include <SDL.h>

mge_begin

#ifdef M_PI
    static long double const PI = M_PI;
#else
    static long double const PI = 3.14159265358979323846264338327950288;
#endif
    static float const MET2PIX = 32.0f;
    static float const PIX2MET = 1.0f / MET2PIX;
    static float const RAD2DEG = 180.0f / PI;
    static float const DEG2RAD = PI / 180.0f;

    // 屏幕坐标转物理空间坐标
    float getMetricFromPixel(const int iPixel) noexcept;
    // 物理空间坐标转屏幕坐标
    float getPixelFromMetric(const float iMetric) noexcept;
    // 屏幕坐标转物理空间坐标
    b2Vec2 getMetricPositionFromPixel(Vector2f const& iPixel);
    // 物理空间坐标转屏幕坐标空间
    Vector2f getPixelPositionFromMetric(const b2Vec2& iMetrics);

    class b2BodySugar {
    public:
        b2BodySugar();
        b2BodySugar(b2Body* body);
        virtual ~b2BodySugar() {}
    public:
        static b2Body* CreateBody(b2World* world, b2BodyType type);
        void destroy();
        void reset(b2Body* body);
        b2Fixture* addBoxShape(Vector2i const& screen_size);
        b2Fixture* addBoxShape(Vector2f const& screen_pos, Vector2i const& screen_size);
        b2Fixture* addCircleShape(float screen_radius);
        b2Fixture* addCircleShape(Vector2f const& screen_pos, float screen_radius);
        void addEdgeShape(Vector2f const& start_pos, Vector2f const& end_pos);
        void setRestitution(float restitution);
        void setTransform(Vector2f const& screen_pos, float angle = 0.0f);
        void setLinearVelocity(Vector2f const& screen_velocity);
        void enableRotation(bool e = true);
    public:
        Vector2f getPixelPosition() const;
        float getPixelAngle() const;
        b2Body* body() const;
    private:
        b2Body* _body;
    };

    class b2WorldSugar {
    public:
        static b2World* CreateWorld(Vector2f const& g = {});
        static void update(b2World* world, float delta);
    };

    class PhysicDrawner : public b2Draw {
    public:
        PhysicDrawner() {
            SetFlags(b2Draw::e_shapeBit);
        }
        void DrawPolygon(const b2Vec2 *iVertices, int32 iVertexCount,
                         const b2Color &iColor) override;
        void DrawSolidPolygon(const b2Vec2 *iVertices, int32 iVertexCount,
                              const b2Color &iColor) override;
        void DrawCircle(const b2Vec2 &center, float radius,
                        const b2Color &color) override;
        void DrawSolidCircle(const b2Vec2 &center, float radius, const b2Vec2 &axis,
                             const b2Color &color) override;
        void DrawSegment(const b2Vec2 &p1, const b2Vec2 &p2,
                         const b2Color &color) override;
        void DrawTransform(const b2Transform &xf) override;
        void DrawPoint(const b2Vec2 &p, float size, const b2Color &color) override;
    };

mge_end

#endif //SDL2_UI_PHYSICS_H
