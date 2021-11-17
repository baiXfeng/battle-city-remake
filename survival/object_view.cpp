//
// Created by baifeng on 2021/11/14.
//

#include "object_view.h"
#include "common/loadres.h"
#include "common/game.h"

namespace survival {

    using namespace mge;

    TankView::TankView() {
        auto bodyTex = res::load_texture(_game.renderer(), "assets/survival/tank.png");
        auto bodyView = New<ImageWidget>(bodyTex);
        bodyView->setAnchor(0.5f, 0.44f);
        addChild(bodyView);
        _img[0] = bodyView.get();

        auto weaponTex = res::load_texture(_game.renderer(), "assets/survival/gun.png");
        auto weaponView = New<ImageWidget>(weaponTex);
        weaponView->setAnchor(0.5f, 0.742f);
        weaponView->setPosition(0.0f, -4.0f);
        addChild(weaponView);
        _img[1] = weaponView.get();
    }

    Widget* TankView::body() const {
        return _img[0];
    }

    Widget* TankView::weapon() const {
        return _img[1];
    }
}