//
// Created by baifeng on 2021/11/16.
//

#include "menu.h"
#include "common/loadres.h"
#include "common/game.h"
#include "common/widget_ex.h"

using namespace mge;

int _colorIndex = 9;

class WeaponIconCell : public CellWidget {
public:
    WeaponIconCell() {
        SDL_Color color[7] = {
                {255, 0, 0, 255},
                {0, 255, 0, 255},
                {0, 0, 255, 255},
                {255, 255, 0, 255},
                {0, 255, 255, 255},
                {255, 0, 255, 255},
                {0, 0, 0, 255},
        };
        auto mask = Ptr(new mge::MaskWidget(color[_colorIndex++ % 7]));
        addChild(mask);
        _mask = mask->to<MaskWidget>();
    }
private:
    void onModifySize(Vector2f const& size) override {
        //_mask->setPositionX(10);
        _mask->setSize(size-Vector2f{20, 0});
    }
private:
    MaskWidget* _mask;
};

class WeaponListView : public TableWidget, public WidgetDataSource {
public:
    WeaponListView() {
        auto icon_texture = res::load_texture(_game.renderer(), "assets/survival/icon-box.png");
        auto image = New<ImageWidget>(icon_texture);
        addChild(image);

        setAnchor(0.5f, 0.5f);
        setPosition(size().x * 0.5f, size().y * 0.35f);
        setSize(80, 80);
        setDirection(Horizontal);
        setDataSource(this);
        //reload_data(true);

        _cursor->setVisible(false);
    }
private:
    size_t numberOfCellsInWidget(TableWidget* sender) override {
        return 10;
    }
    Vector2i cellSizeForIndex(TableWidget* sender, size_t index) override {
        return {100, 80};
    }
    Widget::Ptr cellWidgetAtIndex(TableWidget* sender, size_t index) override {
        auto cell = sender->dequeueCell();
        if (cell == nullptr) {
            cell.reset(new WeaponIconCell);
        }
        return cell;
    }
};

WeaponSelectWidget::WeaponSelectWidget() {
    auto mask = Ptr(new mge::MaskWidget({255, 255, 255, 255}));
    addChild(mask);

    auto table = New<WeaponListView>();
    addChild(table);
    _weaponView = table->to<WeaponListView>();

    auto font = res::load_ttf_font("assets/fonts/prstart.ttf", 22);
    font->setColor({0, 0, 0, 255});
    auto label = TTFLabel::New("", font, {0.5f, 0.5f});
    label->setPosition(size().x * 0.5f, size().y * 0.65f);
    label->setName("label");
    addChild(label);

    _weaponView->connect(_weaponView->DID_SCROLL, [this](Widget* sender) {
        auto label = this->find("label")->to<TTFLabel>();
        label->setString(std::string("ITEM ") + std::to_string(_weaponView->getCursorIndex()+1));
    });
    _weaponView->reload_data(true);
}

void WeaponSelectWidget::onButtonDown(int key) {
    if (key == KeyCode::LEFT) {
        _weaponView->startMoveCursor(_weaponView->MOVE_PREV);
    } else if (key == KeyCode::RIGHT) {
        _weaponView->startMoveCursor(_weaponView->MOVE_NEXT);
    }
}

void WeaponSelectWidget::onButtonUp(int key) {
    if (key == KeyCode::LEFT) {
        _weaponView->stopMoveCursor();
    } else if (key == KeyCode::RIGHT) {
        _weaponView->stopMoveCursor();
    }
}