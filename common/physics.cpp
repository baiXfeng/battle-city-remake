//
// Created by baifeng on 2021/12/1.
//

#include "physics.h"
#include "game.h"
#include <SDL2_gfxPrimitives.h>
#include <vector>

mge_begin

    //=====================================================================================

    float getMetricFromPixel(const int iPixel) noexcept {
        return iPixel * PIX2MET;
    }

    float getPixelFromMetric(const float iMetric) noexcept {
        return iMetric * MET2PIX;
    }

    b2Vec2 getMetricPositionFromPixel(Vector2f const& iPixel) {
        return {getMetricFromPixel(iPixel.x), getMetricFromPixel(iPixel.y)};
    }

    Vector2f getPixelPositionFromMetric(const b2Vec2& iMetrics) {
        return {getPixelFromMetric(iMetrics.x), getPixelFromMetric(iMetrics.y)};
    }

    //=====================================================================================

    b2BodySugar::b2BodySugar(): _body(nullptr) {}
    b2BodySugar::b2BodySugar(b2Body* body): _body(body) {}

    b2Body* b2BodySugar::CreateBody(b2World* world, b2BodyType type) {
        b2BodyDef aBodyDef;
        aBodyDef.type = type;
        return world->CreateBody(&aBodyDef);
    }

    void b2BodySugar::destroy() {
        if (_body) {
            _body->GetWorld()->DestroyBody(_body);
            _body = nullptr;
        }
    }

    void b2BodySugar::reset(b2Body* body) {
        _body = body;
    }

    void b2BodySugar::addBoxShape(Vector2i const& screen_size) {
        b2PolygonShape aRectShape;
        aRectShape.SetAsBox(getMetricFromPixel(screen_size.x) / 2.0f, getMetricFromPixel(screen_size.y) / 2.0f);
        b2FixtureDef def;
        def.shape = &aRectShape;
        def.density = 1.0f;
        _body->CreateFixture(&def);
    }

    void b2BodySugar::addBoxShape(Vector2f const& screen_pos, Vector2i const& screen_size) {
        b2PolygonShape aRectShape;
        aRectShape.SetAsBox(
                getMetricFromPixel(screen_size.x) / 2.0f,
                getMetricFromPixel(screen_size.y) / 2.0f,
                getMetricPositionFromPixel(screen_pos),
                0.0f
        );
        b2FixtureDef def;
        def.shape = &aRectShape;
        def.density = 1.0f;
        _body->CreateFixture(&def);
    }

    void b2BodySugar::addCircleShape(float screen_radius) {
        b2CircleShape aCircleShape;
        aCircleShape.m_radius = getMetricFromPixel(screen_radius);
        b2FixtureDef def;
        def.shape = &aCircleShape;
        def.density = 1.0f;
        _body->CreateFixture(&def);
    }

    void b2BodySugar::addCircleShape(Vector2f const& screen_pos, float screen_radius) {
        b2CircleShape aCircleShape;
        aCircleShape.m_radius = getMetricFromPixel(screen_radius);
        aCircleShape.m_p = getMetricPositionFromPixel(screen_pos);
        b2FixtureDef def;
        def.shape = &aCircleShape;
        def.density = 1.0f;
        _body->CreateFixture(&def);
    }

    void b2BodySugar::addEdgeShape(Vector2f const& start_pos, Vector2f const& end_pos) {
        b2EdgeShape aEdgeShape;
        aEdgeShape.SetTwoSided(getMetricPositionFromPixel(start_pos), getMetricPositionFromPixel(end_pos));
        b2FixtureDef def;
        def.shape = &aEdgeShape;
        def.density = 1.0f;
        _body->CreateFixture(&def);
    }

    void b2BodySugar::setRestitution(float restitution) {
        for (auto fix = _body->GetFixtureList(); fix; fix = fix->GetNext()) {
            fix->SetRestitution(restitution);
        }
    }

    void b2BodySugar::setTransform(Vector2f const& screen_pos, float angle) {
        _body->SetTransform(getMetricPositionFromPixel(screen_pos), angle  * DEG2RAD);
    }

    void b2BodySugar::setLinearVelocity(Vector2f const& screen_velocity) {
        _body->SetLinearVelocity(getMetricPositionFromPixel(screen_velocity));
    }

    void b2BodySugar::enableRotation(bool e) {
        _body->SetFixedRotation(!e);
    }

    Vector2f b2BodySugar::getPixelPosition() const {
        return getPixelPositionFromMetric(_body->GetPosition());
    }

    float b2BodySugar::getPixelAngle() const {
        return _body->GetAngle() * RAD2DEG;
    }

    b2Body* b2BodySugar::body() const {
        return _body;
    }

    //=====================================================================================

    b2World* b2WorldSugar::CreateWorld(Vector2f const& g) {
        return new b2World({g.x, g.y});
    }

    void b2WorldSugar::update(b2World* world, float delta) {
        world->Step(delta, 6.0f, 2.0f);
    }

    //=====================================================================================

    void drawPolygon(const std::vector<Sint16>& iXVertices,
                     const std::vector<Sint16>& iYVertices,
                     const int iNumberVertices, const SDL_Color& iColor) {
        polygonRGBA(_game.renderer(), iXVertices.data(), iYVertices.data(), iNumberVertices,
                    iColor.r, iColor.g, iColor.b, iColor.a);
    }

    void drawSolidPolygon(const std::vector<Sint16>& iXVertices,
                          const std::vector<Sint16>& iYVertices,
                          const int iNumberVertices, const SDL_Color& iColor) {
        filledPolygonRGBA(_game.renderer(), iXVertices.data(), iYVertices.data(),
                          iNumberVertices, iColor.r, iColor.g, iColor.b, iColor.a);
    }

    SDL_Color convertColor(const b2Color& iColor) {
        return SDL_Color{
                static_cast<Uint8>(iColor.r * 255), static_cast<Uint8>(iColor.g * 255),
                static_cast<Uint8>(iColor.g * 255), static_cast<Uint8>(iColor.a * 255)};
    }

    std::pair<std::vector<Sint16>, std::vector<Sint16>> buildGraphicVertices(const b2Vec2* iMetricVertices, const int32 iVertexCount) {
        Vector2f aRect;
        std::vector<Sint16> aVerticesPixelX;
        std::vector<Sint16> aVerticesPixelY;
        aVerticesPixelX.reserve(iVertexCount);
        aVerticesPixelY.reserve(iVertexCount);

        for (int32 i = 0; i < iVertexCount; ++i) {
            const b2Vec2& aVertex = iMetricVertices[i];
            aRect = getPixelPositionFromMetric(aVertex);

            aVerticesPixelX.emplace_back(aRect.x);
            aVerticesPixelY.emplace_back(aRect.y);
        }

        return make_pair(std::move(aVerticesPixelX), std::move(aVerticesPixelY));
    }

    void PhysicDrawner::DrawPolygon(const b2Vec2* iVertices, int32 iVertexCount, const b2Color& iColor) {
        const auto aVerticesPixel = buildGraphicVertices(iVertices, iVertexCount);
        const auto aColor = convertColor(iColor);

        drawPolygon(aVerticesPixel.first, aVerticesPixel.second, iVertexCount, aColor);
    }

    void PhysicDrawner::DrawSolidPolygon(const b2Vec2* iVertices, int32 iVertexCount, const b2Color& iColor) {
        const auto aVerticesPixel = buildGraphicVertices(iVertices, iVertexCount);
        const auto aColor = convertColor(iColor);

        drawSolidPolygon(aVerticesPixel.first, aVerticesPixel.second, iVertexCount, aColor);
    }

    void PhysicDrawner::DrawCircle(const b2Vec2& center, float radius, const b2Color& color) {
        const auto aPosition = getPixelPositionFromMetric(center);
        const auto aColor = convertColor(color);
        const Sint16 aRadiusPixel = getPixelFromMetric(radius);

        circleRGBA(_game.renderer(), aPosition.x, aPosition.y, aRadiusPixel, aColor.r, aColor.g, aColor.b, aColor.a);
    }

    void PhysicDrawner::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) {
        const auto aPosition = getPixelPositionFromMetric(center);
        const auto aColor = convertColor(color);
        const Sint16 aRadiusPixel = getPixelFromMetric(radius);

        filledCircleRGBA(_game.renderer(), aPosition.x, aPosition.y, aRadiusPixel, aColor.r, aColor.g, aColor.b, aColor.a);
    }

    void PhysicDrawner::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {}

    void PhysicDrawner::DrawTransform(const b2Transform& xf) {}

    void PhysicDrawner::DrawPoint(const b2Vec2& p, float size, const b2Color& color) {}

mge_end