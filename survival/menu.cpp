//
// Created by baifeng on 2021/11/16.
//

#include "menu.h"
#include "common/widget_ex.h"
#include "common/loadres.h"

using namespace mge;

class testCell : public CellWidget {
public:
    testCell() {
        enum {MAX = 6};
        SDL_Color color[MAX] = {
                {255, 255, 255, 255},
                {255, 0, 255, 255},
                {0, 255, 255, 255},
                {255, 255, 0, 255},
                {0, 255, 0, 255},
                {0, 0, 255, 255},
        };
        static int cIdx = 0;
        int index = cIdx++ % MAX;
        auto mask = Ptr(new MaskWidget(color[index]));
        addChild(mask);
        _mask = mask->to<MaskWidget>();

        auto font = res::load_ttf_font("assets/fonts/prstart.ttf", 30);
        auto label = TTFLabel::New("0", font);
        label->setName("label");
        addChild(label);
    }
    void setNumber(int i) {
        auto label = find("label")->to<TTFLabel>();
        label->setString(std::to_string(i), {0, 0, 0, 255});
    }
private:
    void onModifySize(Vector2f const& size) {
        _mask->setSize(_mask->size().x, size.y);
    }
private:
    MaskWidget* _mask;
};

class testTable : public TableWidget, public WidgetDataSource {
public:
    testTable() {
        setDataSource(this);
        reload_data();
    }
protected:
    size_t numberOfCellsInWidget(Widget* sender) override {
        return 100;
    }
    CellWidget* cellWidgetAtIndex(Widget* sender, size_t index) override {
        auto cell = new testCell;
        cell->setNumber(index+1);
        return cell;
    }
    Vector2i cellSizeForIndex(Widget* sender, size_t index) override {
        return {960, 100};
    }
};

WeaponSelectWidget::WeaponSelectWidget() {
    auto mask = Ptr(new mge::MaskWidget({255, 0, 0, 255}));
    addChild(mask);

    _tableView = new testTable;
    addChild(Ptr(_tableView));
}

void WeaponSelectWidget::onButtonDown(int key) {
    if (key == KeyCode::UP) {
        _tableView->moveCursorPrev();
    } else if (key == KeyCode::DOWN) {
        _tableView->moveCursorNext();
    }
}

void WeaponSelectWidget::onButtonUp(int key) {

}