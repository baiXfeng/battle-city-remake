//
// Created by baifeng on 2021/11/16.
//

#include "menu.h"
#include "common/loadres.h"
#include "common/game.h"
#include "common/widget_ex.h"
#include "common/gridmap.h"

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

WeaponSelectView::WeaponSelectView() {
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

void WeaponSelectView::onButtonDown(int key) {
    if (key == KeyCode::LEFT) {
        _weaponView->startMoveCursor(_weaponView->MOVE_PREV);
    } else if (key == KeyCode::RIGHT) {
        _weaponView->startMoveCursor(_weaponView->MOVE_NEXT);
    }
}

void WeaponSelectView::onButtonUp(int key) {
    if (key == KeyCode::LEFT) {
        _weaponView->stopMoveCursor();
    } else if (key == KeyCode::RIGHT) {
        _weaponView->stopMoveCursor();
    }
}

//=====================================================================================

class WorldTileCell : public TileWidget {
public:
    WorldTileCell() {
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

        auto font = res::load_ttf_font("assets/fonts/prstart.ttf", 10);
        auto label = TTFLabel::New("", font, {0.5f, 0.5f});
        addChild(label);
        _label = label->to<TTFLabel>();
    }
    void setPos(Vector2i const& p) {
        _label->setString(std::to_string(p.x) + "," + std::to_string(p.y), {180, 180, 180, 255});
    }
private:
    void onModifySize(Vector2f const& size) override {
        _mask->setSize(size);
        _label->setPosition(size.x*0.5f, size.y*0.5f);
    }
private:
    MaskWidget* _mask;
    TTFLabel* _label;
};

class WorldTileMap : public GridMapWidget, public GridMapDataSource {
public:
    WorldTileMap() {
        setDataSource(this);
    }
private:
    size_t numberOfLayersInWidget(GridMapWidget* sender) override {
        return 1;
    }
    Vector2i sizeOfGridMap(GridMapWidget* sender) override {
        return {30, 30};
    }
    Vector2i sizeOfGridTile(GridMapWidget* sender) override {
        return {56, 56};
    }
    Widget::Ptr tileWidgetAtPosition(GridMapWidget* sender, int layerIndex, Vector2i const& position) override {
        auto cell = sender->dequeueTile(layerIndex);
        if (cell == nullptr) {
            cell = Ptr(new WorldTileCell);
        }
        cell->to<WorldTileCell>()->setPos(position);
        return cell;
    }
};

BattleWorldView::BattleWorldView() {
    addChild(Ptr(_worldMap = new WorldTileMap));

    if (false) {
        _worldMap->setAnchor(0.5f, 0.5f);
        _worldMap->setSize(400, 400);
        _worldMap->setPosition(480, 272);

        auto mask = Ptr(new mge::MaskWidget({0, 0, 0, 140}));
        mask->setSize(400, 400);
        _worldMap->addChild(mask);
    }

    _worldMap->reload_data();

    _game.setRenderColor({255, 255, 255, 255});
}

void BattleWorldView::onButtonDown(int key) {
    if (key == KeyCode::UP) {
        _worldMap->getCamera()->move({0.0f, -300.0f});
    } else if (key == KeyCode::DOWN) {
        _worldMap->getCamera()->move({0.0f, 300.0f});
    } else if (key == KeyCode::LEFT) {
        _worldMap->getCamera()->move({-300.0f, 0.0f});
    } else if (key == KeyCode::RIGHT) {
        _worldMap->getCamera()->move({300.0f, 0.0f});
    } else if (key == KeyCode::A) {
        _worldMap->getCamera()->move({300.0f, 300.0f});
    } else if (key == KeyCode::B) {
        _worldMap->getCamera()->move({-300.0f, 300.0f});
    } else if (key == KeyCode::X) {
        _worldMap->getCamera()->move({300.0f, -300.0f});
    } else if (key == KeyCode::Y) {
        _worldMap->getCamera()->move({-300.0f, -300.0f});
    }
}

void BattleWorldView::onButtonUp(int key) {
    if (key == KeyCode::UP) {
        _worldMap->getCamera()->move({});
    } else if (key == KeyCode::DOWN) {
        _worldMap->getCamera()->move({});
    } else if (key == KeyCode::LEFT) {
        _worldMap->getCamera()->move({});
    } else if (key == KeyCode::RIGHT) {
        _worldMap->getCamera()->move({});
    } else if (key == KeyCode::A) {
        _worldMap->getCamera()->move({});
    } else if (key == KeyCode::B) {
        _worldMap->getCamera()->move({});
    } else if (key == KeyCode::X) {
        _worldMap->getCamera()->move({});
    } else if (key == KeyCode::Y) {
        _worldMap->getCamera()->move({});
    }
}
