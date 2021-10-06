//
// Created by baifeng on 2021/9/29.
//

#include "view.h"
#include "common/loadres.h"
#include "common/game.h"
#include "common/action.h"
#include "common/audio.h"
#include "common/collision.h"
#include "const.h"
#include "data.h"
#include "lutok3.h"
#include "battle.h"
#include <assert.h>

std::string fontName = "assets/fonts/prstart.ttf";

class HelloWorld : public GamePadWidget {
public:
    ~HelloWorld() {

    }
    HelloWorld() {
        auto renderer = _game.renderer();
        auto bg = res::load_texture(renderer, "assets/bg.jpg");
        auto root = this;

        auto bgView = Widget::Ptr(new ImageWidget(bg));
        root->addChild(bgView);

        auto iconView = Widget::Ptr(new ImageWidget(bg));
        iconView->setScale(0.5f, 0.5f);
        iconView->setPosition(480, 272);
        iconView->setOpacity(100);
        root->addChild(iconView);

        auto label_ptr = Widget::New<TTFLabel>();
        auto font = res::load_ttf_font(fontName, 14);
        auto label = label_ptr->to<TTFLabel>();
        label->setFont(font);
        label->setString("Hello World!", {255, 255, 255, 255});
        label->setPosition(480, 272);
        label->setAnchor(0.5f, 0.5f);
        root->addChild(label_ptr);

        {
            auto zoom_big = Action::Ptr(new ScaleBy(bgView.get(), {1.0f, 1.0f}, 0.5f));
            auto zone_small = Action::Ptr(new ScaleBy(bgView.get(), {-1.0f, -1.0f}, 0.5f));
            auto action_seq = Action::Ptr(new Sequence({zoom_big, zone_small}));
            auto repeat = Action::Ptr(new Repeat(action_seq));
            bgView->runAction(repeat);
            bgView->setScale({0.5f, 0.5f});
        }

        {
            auto move1 = Action::Ptr(new MoveBy(iconView.get(), {240, 0}, 1.0f));
            auto move2 = Action::Ptr(new MoveBy(iconView.get(), {-240, 0}, 1.0f));
            auto seq = Action::Ptr(new Sequence({move1, move2}));
            auto repeat = Action::Ptr(new Repeat(seq));
            iconView->runAction(repeat);
            iconView->setPosition(480, 272);
            iconView->setAnchor({0.5f, 0.5f});
        }
    }
private:
    void onButtonDown(int key) override {
        if (key == KeyCode::X) {
            _game.screen().replace<HelloWorld>();
            //_game.screen().pop();
        }
    }
};

//=====================================================================================

LogoView::LogoView():_canClick(false) {
    auto bg = res::load_texture(_game.renderer(), "assets/images/logo.png");
    auto image = New<ImageWidget>(bg);
    auto mask = New<MaskWidget>(SDL_Color{0, 0, 0, 255});
    addChild(image);
    addChild(mask);
    _mask = mask->to<MaskWidget>();

    setFinishCall([]{
        _game.screen().replace<StartView>();
    });
    _game.setRenderColor({0, 0, 0, 255});
}

void LogoView::onButtonDown(int key) {
    if (_canClick) {
        this->stopAllActions();
        auto fadeout = Action::Ptr(new ProgressAction(std::bind(&LogoView::onFadeOut, this, std::placeholders::_1), 0.5f));
        auto delay1 = Action::New<Delay>(1.0f);
        auto callback = Action::Ptr(new CallBackVoid(_callback));
        auto action = Action::Ptr(new Sequence({fadeout, delay1, callback}));
        this->runAction(action);
        _canClick = false;
    }
}

void LogoView::setFinishCall(Callback const& cb) {
    _callback = cb;
}

void LogoView::onEnter() {
    auto fadein = Action::Ptr(new ProgressAction(std::bind(&LogoView::onFadeIn, this, std::placeholders::_1), 0.5f));
    auto delay1 = Action::Ptr(new Delay(1.0f));
    auto clickon = Action::New<CallBackVoid>([&]{
        this->_canClick = true;
    });
    auto delay2 = Action::Ptr(new Delay(1.0f));
    auto clickoff = Action::New<CallBackVoid>([&]{
        this->_canClick = false;
    });
    auto fadeout = Action::Ptr(new ProgressAction(std::bind(&LogoView::onFadeOut, this, std::placeholders::_1), 0.5f));
    auto delay3 = Action::New<Delay>(1.0f);
    auto callback = Action::Ptr(new CallBackVoid(_callback));
    auto action = Action::Ptr(new Sequence({fadein, delay1, clickon, delay2, clickoff, fadeout, delay3, callback}));
    this->runAction(action);
}

void LogoView::onFadeIn(float v) {
    _mask->setOpacity(255 * (1-v));
}

void LogoView::onFadeOut(float v) {
    _mask->setOpacity(255 * v);
}

//=====================================================================================

StartView::StartView():_index(0), _canSelect(false) {

    _game.setRenderColor({0, 0, 0, 255});

    auto font = res::load_ttf_font(fontName, 18);
    font->setColor({255, 255, 255, 255});

    _game.gamepad().sleep(0.0f);

    auto root = Widget::New<WindowWidget>();
    root->setName("root");
    root->setPosition(0, size().y);
    addChild(root);

    Widget::Ptr widget;

    {
        auto title = new TTFLabel;
        title->setFont(font);
        title->setString("I-    00 HI- 20000");
        title->setPosition(15, 15);
        widget.reset(title);
        root->addChild(widget);
    }

    {
        auto title = new TTFLabel;
        title->setFont(font);
        title->setString("github.com/baiXfeng");
        title->setAnchor(1.0f, 0.0f);
        title->setPosition(size().x-15, 15);
        widget.reset(title);
        root->addChild(widget);
    }

    {
        auto texture = res::load_texture(_game.renderer(), "assets/images/battle_city.png");
        auto imageView = Widget::New<ImageWidget>(texture);
        imageView->setAnchor(0.5f, 0.0f);
        imageView->setPosition(size().x*0.5f, 100);
        root->addChild(imageView);
    }

    {
        auto title = new TTFLabel;
        title->setFont(font);
        title->setString("1 PLAYER");
        title->setAnchor(0.5f, 0.0f);
        title->setPosition(size().x * 0.5f, size().y * 0.52f);
        widget.reset(title);
        root->addChild(widget);

        _position.push_back({
                size().x * 0.39f,
                title->position().y + title->size().y * 0.5f
        });
    }

    {
        auto animate = new FrameAnimationWidget;
        animate->setFrames({
            res::load_texture(_game.renderer(), "assets/images/tank_player1_right_c0_t1.png"),
            res::load_texture(_game.renderer(), "assets/images/tank_player1_right_c0_t2.png")
        });
        animate->setAnchor(0.5f, 0.5f);
        animate->setPosition(_position[0]);
        animate->play(0.15f);
        animate->setName("tank");
        widget.reset(animate);
        root->addChild(widget);
    }

    {
        auto title = new TTFLabel;
        title->setFont(font);
        title->setString("TIP ME");
        title->setAnchor(0.5f, 0.0f);
        title->setPosition(size().x * 0.5f, size().y * 0.6f);
        widget.reset(title);
        root->addChild(widget);

        _position.push_back({
                size().x * 0.39f,
                title->position().y + title->size().y * 0.5f
        });
    }

    {
        auto texture = res::load_texture(_game.renderer(), "assets/images/namcot.png");
        auto imageView = Widget::New<ImageWidget>(texture);
        imageView->setAnchor(0.5f, 0.0f);
        imageView->setPosition(size().x*0.5f, size().y * 0.82f);
        root->addChild(imageView);
    }

    {
        auto texture = res::load_texture(_game.renderer(), "assets/images/copyright.png");
        auto imageView = Widget::New<ImageWidget>(texture);
        imageView->setAnchor(1.0f, 0.0f);
        imageView->setName("copyright");
        root->addChild(imageView);
    }

    {
        auto title = new TTFLabel;
        title->setFont(font);
        title->setString(" 1980 1985 NAMCO LTD. ALL RIGHTS RESERVED");
        title->setAnchor(0.5f, 0.0f);
        title->setPosition(size().x * 0.52f, size().y * 0.9f);
        widget.reset(title);
        root->addChild(widget);

        auto copyright = root->find("copyright");
        copyright->setPosition(title->position().x - title->size().x * 0.5f, size().y * 0.9f);
    }

    root->performLayout();
}

void StartView::onEnter() {
    auto root = find("root");
    auto move = Action::New<MoveBy>(root, Vector2f{0.0f, -size().y}, 4.0f);
    auto callback = Action::New<CallBackVoid>([&]{
        this->_canSelect = true;
    });
    auto action = Action::Ptr(new Sequence({move, callback}));
    root->runAction(action);
}

void StartView::onButtonDown(int key) {
    if (key == KeyCode::START) {
        if (_canSelect) {
            onStart(_index);
        } else {
            auto root = find("root");
            root->setPosition(0, 0);
            root->stopAllActions();
            _canSelect = true;
        }
    } else if (key == KeyCode::SELECT) {
        if (_canSelect) {
            auto tank = gfind("tank");
            tank->setPosition(_position[_index = ++_index % _position.size()]);
        }
    }
}

void StartView::onStart(int index) {
    if (index == 0) {
        // 选关
        auto widget = New<SelectLevelView>();
        addChild(widget);
    }
}

//=====================================================================================

SelectLevelView::SelectLevelView():_level(1), _duration(0.3f) {

    Ptr widget;

    {
        auto view = new CurtainWidget({115, 115, 115, 255});
        view->fadeIn(_duration);
        widget.reset(view);
        addChild(widget);
        _curtain = view;
    }

    {
        auto font = res::load_ttf_font(fontName, 16);
        font->setColor({0, 0, 0, 255});
        auto label = new TTFLabel;
        label->setFont(font);
        label->setAnchor(0.5f, 0.5f);
        label->setPosition(size().x * 0.5f, size().y * 0.5f);
        label->setString("STAGE 1");
        label->setVisible(false);
        widget.reset(label);
        addChild(widget);
        _label = label;
    }

    defer([&]{
        _label->setVisible(true);
    }, _duration);

    _game.get<int>("level") = _level;
}

void SelectLevelView::onButtonDown(int key) {
    if (!_label->visible()) {
        return;
    }
    if (key == KeyCode::A or key == KeyCode::X) {
        addLevel();
        autoAddLevel(true);
    } else if (key == KeyCode::B or key == KeyCode::Y) {
        subLevel();
        autoAddLevel(false);
    } else if (key == KeyCode::SELECT) {
        // 回退
        float duration = _duration * 0.8f;
        _label->setVisible(false);
        _curtain->fadeOut(duration);
        defer([&]{
            this->removeFromParent();
        }, duration);
        sleep_gamepad(duration);
    } else if (key == KeyCode::START) {
        auto sound = "assets/sounds/stage_start.ogg";
        _game.audio().loadEffect(sound);
        _game.audio().playEffect(sound);
        sleep_gamepad(10.0f);
        defer([&]{
            sleep_gamepad(0.0f);
            _game.screen().replace<BattleView>();
        }, 1.8f);
        _game.get<int>("level") = _level;
    }
}

void SelectLevelView::onButtonUp(int key) {
    if (key == KeyCode::A or key == KeyCode::X or key == KeyCode::B or key == KeyCode::Y) {
        stopAutoAddLevel();
    }
}

void SelectLevelView::addLevel() {
    int const level_max = _game.get<int>("level_max");
    _level = ++_level > level_max ? 1 : _level;
    _label->setString("STAGE " + std::to_string(_level));
}

void SelectLevelView::subLevel() {
    int const level_max = _game.get<int>("level_max");
    _level = --_level <= 0 ? level_max : _level;
    _label->setString("STAGE " + std::to_string(_level));
}

void SelectLevelView::autoAddLevel(bool add) {
    auto delay = Action::New<Delay>(0.5f);
    auto call = Action::New<CallBackVoid>(std::bind(add ? &SelectLevelView::addLevel : &SelectLevelView::subLevel, this));
    auto delay1 = Action::New<Delay>(0.05f);
    auto action_seq = Action::Ptr(new Sequence({call, delay1}));
    auto repeat = Action::Ptr(new Repeat(action_seq));
    auto seq = Action::Ptr(new Sequence({delay, repeat}));
    auto action = Action::New<Repeat>(seq);
    action->setName("auto-add-level");
    stopAutoAddLevel();
    runAction(action);
}

void SelectLevelView::stopAutoAddLevel() {
    stopAction("auto-add-level");
}

//=====================================================================================

BattleView::BattleView() {
    Ptr widget;

    auto root = New<WindowWidget>();
    addChild(root);

    Vector2f root_size;

    {
        auto view = new BattleFieldView;
        widget.reset(view);
        root->addChild(widget);
        root_size.x += view->size().x;
        root_size.y = view->size().y;
    }

    {
        auto view = new BattleInfoView;
        widget.reset(view);
        root->addChild(widget);
        root_size.x += view->size().x;
        root_size.y = view->size().y;
    }

    root->setSize(root_size);
    root->setPosition((size().x - root_size.x) * 0.5f, (size().y - root_size.y) * 0.5f);

    {
        auto view = new CurtainWidget({115, 115, 115, 255});
        view->fadeOut(0.3f);
        widget.reset(view);
        addChild(widget);

        auto delay = Action::New<Delay>(0.3f);
        auto call = Action::Ptr(new CallBackSender(view, [&](Widget* sender) {
            sender->removeFromParent();
        }));
        view->runAction(Action::Ptr(new Sequence({delay, call})));
    }

    auto font = res::load_ttf_font(res::fontName("prstart"), 18);
    font->setColor({0, 0, 0, 255});
    auto label = TTFLabel::New("L-CHEAT", font);
    label->setPosition(15, 15);
    addChild(label);

    _game.setRenderColor({115, 115, 115, 255});
    this->performLayout();
}

//=====================================================================================

static int const _enemy_icon_size = 20;

BattleInfoView::~BattleInfoView() {
    _game.event().remove(EventID::ENEMY_NUMBER_CHANGED, this);
    _game.event().remove(EventID::PLAYER1_NUMBER_CHANGED, this);
}

void BattleInfoView::onEvent(Event const& e) {
    if (e.Id() == EventID::ENEMY_NUMBER_CHANGED) {
        auto value = e.data<int>();
        onEnemyNumberChanged(value);
    } else if (e.Id() == EventID::PLAYER1_NUMBER_CHANGED) {
        auto value = e.data<int>();
        onPlayerNumberChanged(value);
    }
}

void BattleInfoView::onEnemyNumberChanged(int n) {

}

void BattleInfoView::onPlayerNumberChanged(int n) {

}

BattleInfoView::BattleInfoView() {

    _game.event().add(EventID::ENEMY_NUMBER_CHANGED, this);
    _game.event().add(EventID::PLAYER1_NUMBER_CHANGED, this);

    Ptr widget;
    int const padding = 18;
    auto const old_size = size();

    setSize(_enemy_icon_size * 2 + padding * 2, Tile::MAP_SIZE);

    if (false) {
        auto view = Ptr(new MaskWidget({255, 255, 0, 120}));
        view->setSize(size());
        addChild(view);
    }

    {
        float x = 0, y = 0;
        for (int i = 0; i < 20; ++i) {
            auto view = createEnemyIcon();
            view->setPosition(padding+x+view->size().x*(i%2), padding+2+y+view->size().y*(i/2));
            widget.reset(view);
            addChild(widget);
        }
    }

    auto font = res::load_ttf_font(fontName, _enemy_icon_size);
    font->setColor({0, 0, 0, 255});

    {
        auto label = new TTFLabel;
        label->setFont(font);
        label->setString("IP");
        label->setAnchor(0.5f, 1.0f);
        label->setPosition(size().x * 0.5f, size().y * 0.6f);
        widget.reset(label);
        addChild(widget);
    }

    {
        auto icon = res::load_texture(_game.renderer(), "assets/images/lives.png");
        auto view = New<ImageWidget>(icon);
        view->setAnchor(1.0f, 0.0f);
        view->setPosition(size().x * 0.5f, size().y * 0.6f);
        view->setSize(_enemy_icon_size, _enemy_icon_size);
        addChild(view);
    }

    {
        auto label = new TTFLabel;
        label->setFont(font);
        label->setString("1");
        label->setAnchor(0.0f, 0.0f);
        label->setPosition(size().x * 0.5f, size().y * 0.6f);
        widget.reset(label);
        addChild(widget);
    }

    {
        auto bg = res::load_texture(_game.renderer(), "assets/images/flag.png");
        auto view = New<ImageWidget>(bg);
        view->setAnchor(0.5f, 1.0f);
        view->setPosition(size().x * 0.5f, Tile::MAP_SIZE * 0.85f);
        view->setSize(_enemy_icon_size * 2, _enemy_icon_size * 2);
        addChild(view);
    }

    {
        auto label = new TTFLabel;
        label->setFont(font);
        label->setString("1");
        label->setAnchor(0.0f, 0.0f);
        label->setPosition(size().x * 0.5f, size().y * 0.85f);
        widget.reset(label);
        addChild(widget);
    }

    this->setPosition(Tile::MAP_SIZE, 0.0f);
}

ImageWidget* BattleInfoView::createEnemyIcon() {
    auto icon = res::load_texture(_game.renderer(), "assets/images/enemy.png");
    //auto sign = res::load_texture(_game.renderer(), "");
    auto widget = new ImageWidget(icon);
    widget->setSize(_enemy_icon_size, _enemy_icon_size);
    return widget;
}

//=====================================================================================

class FrameAnimationAction : public Action {
public:
    typedef std::vector<Texture::Ptr> Frames;
public:
    FrameAnimationAction(ImageWidget* target, Frames const& arr, float duration):
    _textures(arr),
    _target(target),
    _index(0),
    _timer(0.0f),
    _timer_max(duration / arr.size()) {}
private:
    State Step(float dt) override {
        if ((_timer += dt) >= _timer_max) {
            _timer -= _timer_max;
            if (++_index >= _textures.size()) {
                _index = 0;
            }
            _target->setTexture(_textures[_index]);
            _target->setSize(Tile::SIZE, Tile::SIZE);
        }
        return RUNNING;
    }
private:
    int _index;
    float _timer;
    float _timer_max;
    ImageWidget* _target;
    Frames _textures;
};

TileView::TileView(TYPE t):
ImageWidget(nullptr),
_type(NONE),
_data(std::make_shared<TileData>()) {
    set_userdata(_data.get());
    setType(t);
}

void TileView::setType(TYPE t) {
    if (_type == t) {
        return;
    }
    Texture::Ptr texture;
    _type = t;
    _data->layer = 1;
    _data->type = _type;
    _action->clear();
    switch (t) {
        case BASE:
            texture = res::load_texture(_game.renderer(), "assets/images/base.png");
            setTexture(texture);
            break;
        case BRICK_0:
        case BRICK_1:
        case BRICK_2:
        case BRICK_3:
        {
            texture = res::load_texture(_game.renderer(), "assets/images/wall_brick.png");
            int half_width = Tile::SIZE >> 1;
            int half_height = Tile::SIZE >> 1;
            SDL_Rect srcrect[4] = {
                    {0, 0, half_width, half_height},
                    {half_width, 0, half_width, half_height},
                    {0, half_height, half_width, half_height},
                    {half_width, half_height, half_width, half_height},
            };
            setTexture(texture, srcrect[t-BRICK_0]);
            setSize(half_width, half_height);
            return;
        }
            break;
        case STEEL_0:
        case STEEL_1:
        case STEEL_2:
        case STEEL_3:
        {
            texture = res::load_texture(_game.renderer(), "assets/images/wall_steel.png");
            int half_width = Tile::SIZE >> 1;
            int half_height = Tile::SIZE >> 1;
            SDL_Rect srcrect[4] = {
                    {0, 0, half_width, half_height},
                    {half_width, 0, half_width, half_height},
                    {0, half_height, half_width, half_height},
                    {half_width, half_height, half_width, half_height},
            };
            setTexture(texture, srcrect[t-STEEL_0]);
            setSize(half_width, half_height);
            return;
        }
            break;
        case TREES:
            texture = res::load_texture(_game.renderer(), "assets/images/trees.png");
            setTexture(texture);
            _data->layer = 2;
            break;
        case ICE_FLOOR:
            texture = res::load_texture(_game.renderer(), "assets/images/ice_floor.png");
            setTexture(texture);
            _data->layer = 0;
            break;
        case WATER:
        {
            FrameAnimationAction::Frames frames = {
                    res::load_texture(_game.renderer(), "assets/images/water_1.png"),
                    res::load_texture(_game.renderer(), "assets/images/water_2.png"),
            };
            setTexture(frames[0]);
            runAction(Action::Ptr(new FrameAnimationAction(this, frames, 1.0f)));
            setSize(Tile::SIZE, Tile::SIZE);
            _data->layer = 0;
            return;
        }
            break;
        default:
            break;
    }
    setSize(Tile::SIZE, Tile::SIZE);
}

int TileView::type() const {
    return _type;
}

void TileView::update(float delta) {
    if (not _action->empty()) {
        _action->update(delta);
    }
    if (_dirty) {
        this->modifyLayout();
    }
}

void TileView::draw(SDL_Renderer* renderer) {
    if (!_visible) {
        return;
    }
    ImageWidget::onDraw(renderer);
}

void TileView::onDirty() {

}

//=====================================================================================

TankView::TankView(TYPE t, TexturesArray const& array):
_type(t),
_texArr(array),
_data(std::make_shared<TileData>()),
_dir(MAX) {
    setAnchor(0.5f, 0.5f);
    set_userdata(_data.get());
    _data->type = _type;
    _data->layer = 1;
    auto mask = Ptr(new MaskWidget({255, 0, 0, 255}));
    mask->setSize(Tile::SIZE, Tile::SIZE);
    addChild(mask);
}

void TankView::move(Direction dir) {
    _dir = dir;
    setFrames(_texArr[dir]);
    play(0.15f);
    Vector2f speed[4] = {
            {0.0f, -100.0f},
            {100.0f, 0.0f},
            {0.0f, 100.0f},
            {-100.0f, 0.0f},
    };
    _move = speed[dir] * 3.5f;
}

void TankView::turn(Direction dir) {
    setTexture(_texArr[dir].front());
}

void TankView::stop(Direction dir) {
    if (dir == UP or dir == DOWN) {
        _move.y = 0.0f;
    } else if (dir == LEFT or dir == RIGHT) {
        _move.x = 0.0f;
    } else {
        _move = {0, 0};
    }
}

void TankView::onUpdate(float delta) {
    setPosition( position() + _move * delta );
    limitPosition();
}

void TankView::limitPosition() {
    int half_width = int(size().x) >> 1;
    int half_height = int(size().y) >> 1;
    if (position().y < half_height and _move.y < 0.0f) {
        setPositionY(half_height);
        _move.y = 0.0f;
    } else if (position().y > parent()->size().y - half_height and _move.y > 0.0f) {
        setPositionY(parent()->size().y - half_height);
        _move.y = 0.0f;
    }
    if (position().x < half_width and _move.x < 0.0f) {
        setPositionX(half_width);
        _move.x = 0.0f;
    } else if (position().x > parent()->size().x - half_width and _move.x > 0.0f) {
        setPositionX(parent()->size().x - half_width);
        _move.x = 0.0f;
    }
}

void TankView::onDirty() {
    setSize(Tile::SIZE, Tile::SIZE);
}

//=====================================================================================

void TileBuilder::gen(TileArray& r, AddTileList const& list) {
    typedef Tile::Type Type;
    for (auto& tile : list) {
        switch (tile.type) {
            case Type::BASE:
                gen(r, "base", {tile.x, tile.y});
                break;
            case Type::BRICK:
                gen(r, "brick", {tile.x, tile.y});
                break;
            case Type::STEEL:
                gen(r, "steel", {tile.x, tile.y});
                break;
            case Type::WATERS:
                gen(r, "water", {tile.x, tile.y});
                break;
            case Type::TREES:
                gen(r, "trees", {tile.x, tile.y});
                break;
            case Type::ICE_FLOOR:
                gen(r, "ice-floor", {tile.x, tile.y});
                break;
            default:
                break;
        }
    }
}

void TileBuilder::gen(TileArray& result, std::string const& type, Vector2i const& position) {
    if (type == "water") {
        gen_tile(result, TileType::WATER, position);
    } else if (type == "trees") {
        gen_tile(result, TileType::TREES, position);
    } else if (type == "ice-floor") {
        gen_tile(result, TileType::ICE_FLOOR, position);
    } else if (type == "base") {
        gen_tile(result, TileType::BASE, position);
    } else if (type == "brick") {
        get_block(result, TileType::BRICK_0, position);
    } else if (type == "steel") {
        get_block(result, TileType::STEEL_0, position);
    }
}

void TileBuilder::gen_tile(TileArray& r, TileType t, Vector2i const& position) {
    auto widget = Widget::New<TileView>(t);
    widget->setPosition(position.to<float>());
    widget->performLayout();
    r.push_back(widget);
}

void TileBuilder::get_block(TileArray& r, TileType begin, Vector2i const& position) {
    static int const block_size = Tile::SIZE >> 1;
    Vector2i offset[4] = {
            {0, 0},
            {block_size >> 1, 0},
            {0, block_size >> 1},
            {block_size >> 1, block_size >> 1},
    };
    for (int i = 0; i < 4; ++i) {
        gen_tile(r, TileType(begin + i), position + offset[i]);
    }
}

//=====================================================================================

void TankBuilder::gen(TankArray& r, TankType t, Vector2f const& position) {
    TankView::TexturesArray texArray;
    gen_textures(texArray, t);
    auto view = new TankView(t, texArray);
    view->setPosition(position + Vector2f{Tile::SIZE >> 1, Tile::SIZE >> 1});
    view->performLayout();
    r.push_back(Widget::Ptr(view));
}

void TankBuilder::gen_textures(TexturesArray& array, TankType t) {
    array.clear();
    array.resize(Direction::MAX);
    auto& up = array[Direction::UP];
    auto& right = array[Direction::RIGHT];
    auto& down = array[Direction::DOWN];
    auto& left = array[Direction::LEFT];
    switch (t) {
        case TankType::PLAYER_1:
        {

        }
            break;
        default:
            break;
    }
}

//=====================================================================================

ScoreView::ScoreView():_total(nullptr), _index(0) {
    _game.setRenderColor({0, 0, 0, 255});

    Ptr widget;

    SDL_Color red = {189, 64, 48, 255};
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {237, 167, 49, 255};

    auto root = New<WindowWidget>();
    root->setPosition(0.0f, 5.0f);
    addChild(root);

    {
        auto label = createLabel("HI-SCORE", red, RIGHT);
        label->setPosition(size().x * 0.5f, size().y * 0.1f);
        widget.reset(label);
        root->addChild(widget);
    }

    {
        auto label = createLabel("20000", yellow, LEFT);
        label->setPosition(size().x * 0.5f + 54, size().y * 0.1f);
        widget.reset(label);
        root->addChild(widget);
    }

    {
        auto label = createLabel("STAGE  1", white, MIDDLE);
        label->setPosition(size().x * 0.5f, size().y * 0.18f);
        widget.reset(label);
        root->addChild(widget);
    }

    {
        auto label = createLabel("I-PLAYER", red, RIGHT);
        label->setPosition(size().x * 0.4f, size().y * 0.26f);
        widget.reset(label);
        root->addChild(widget);
    }

    {
        auto label = createLabel("0", yellow, RIGHT);
        label->setPosition(size().x * 0.4f, size().y * 0.33f);
        widget.reset(label);
        root->addChild(widget);
    }

    Texture::Ptr texture[4] = {
            res::load_texture(_game.renderer(), "assets/images/tank_basic_up_c0_t1.png"),
            res::load_texture(_game.renderer(), "assets/images/tank_fast_up_c0_t1.png"),
            res::load_texture(_game.renderer(), "assets/images/tank_power_up_c0_t1.png"),
            res::load_texture(_game.renderer(), "assets/images/tank_armor_up_c0_t1.png"),
    };
    Texture::Ptr arrow = res::load_texture(_game.renderer(), "assets/images/arrow.png");

    int const heightLine = 54;
    for (int i = 0; i < 4; ++i) {
        {
            auto view = New<ImageWidget>(texture[i]);
            view->setAnchor(0.5f, 0.5f);
            view->setPosition(size().x * 0.5f, i * heightLine + size().y * 0.44f);
            root->addChild(view);
        }

        {
            auto view = New<ImageWidget>(arrow);
            view->setAnchor(0.5f, 0.5f);
            view->setPosition(size().x * 0.47f, i * heightLine + size().y * 0.44f);
            root->addChild(view);
        }

        {
            auto label = createLabel("0", white, RIGHT);
            label->setPosition(size().x * 0.46f, i * heightLine + size().y * 0.44f);
            label->setVisible(false);
            widget.reset(label);
            root->addChild(widget);
            _number.push_back(label);
        }

        {
            auto label = createLabel("PTS", white, RIGHT);
            label->setPosition(size().x * 0.4f, i * heightLine + size().y * 0.44f);
            widget.reset(label);
            root->addChild(widget);
        }

        {
            auto label = createLabel("0", white, RIGHT);
            label->setPosition(size().x * 0.31f, i * heightLine + size().y * 0.44f);
            label->setVisible(false);
            widget.reset(label);
            root->addChild(widget);
            _ops.push_back(label);
        }
    }

    auto mask = New<MaskWidget>(white);
    mask->setSize(size().x * 0.2f, 5.0f);
    mask->setAnchor(0.5f, 0.5f);
    mask->setPosition(size().x * 0.5f, size().y * 0.79f);
    root->addChild(mask);

    {
        auto label = createLabel("0", white, RIGHT);
        label->setPosition(size().x * 0.46f, size().y * 0.83f);
        label->setVisible(false);
        widget.reset(label);
        root->addChild(widget);
        _total = label;
    }

    {
        auto label = createLabel("TOTAL", white, RIGHT);
        label->setPosition(size().x * 0.4f, size().y * 0.83f);
        widget.reset(label);
        root->addChild(widget);
    }

    this->performLayout();
    this->playAnimate();
}

void ScoreView::playAnimate() {
    _index = 0;
    auto delay = Action::New<Delay>(0.5f);
    auto call = Action::New<CallBackVoid>([&]{
        _number[_index]->setVisible(true);
        _ops[_index]->setVisible(true);
        ++_index;
    });
    auto seq = Action::Ptr(new Sequence({delay, call}));
    auto repeat = Action::New<Repeat>(seq, 4);
    auto delay1 = Action::New<Delay>(0.8f);
    auto call1 = Action::New<CallBackVoid>([&]{
        _total->setVisible(true);
    });
    auto delay2 = Action::New<Delay>(1.9f);
    auto call2 = Action::New<CallBackVoid>([&]{
        _game.screen().replace<GameOverView>();
    });
    auto action = Action::Ptr(new Sequence({repeat, delay1, call1, delay2, call2}));
    runAction(action);
}

TTFLabel* ScoreView::createLabel(std::string const& text, SDL_Color const& c, Alignment const& align) {
    auto font = res::load_ttf_font(fontName, 18);
    font->setColor(c);
    auto label = new TTFLabel;
    label->setFont(font);
    label->setString(text);
    Vector2f anchor[3] = {
            {0.0f, 0.5f},
            {1.0f, 0.5f},
            {0.5f, 0.5f},
    };
    label->setAnchor(anchor[align]);
    return label;
}

//=====================================================================================

auto game_over_sound = "assets/sounds/game_over.ogg";

GameOverView::GameOverView() {
    _game.setRenderColor({0, 0, 0, 255});
    _game.gamepad().sleep(0.0f);

    auto delay = Action::New<Delay>(2.0f);
    auto call = Action::New<CallBackVoid>([]{
        _game.screen().replace<StartView>();
    });
    runAction(Action::Ptr(new Sequence({delay, call})));

    auto icon = res::load_texture(_game.renderer(), "assets/images/game_over.png");
    auto view = New<ImageWidget>(icon);
    view->setPosition(size().x * 0.5f, size().y * 0.5f);
    view->setAnchor(0.5f, 0.5f);
    addChild(view);

    _game.audio().loadEffect(game_over_sound);
    _game.audio().playEffect(game_over_sound);

    performLayout();
}

void GameOverView::onButtonDown(int key) {
    if (key == KeyCode::START) {
        _game.audio().releaseEffect(game_over_sound);
        _game.screen().replace<StartView>();
    }
}

//=====================================================================================

CheatView::CheatView(CallBack const& cb):_index(0), _callback(cb == nullptr ? []{} : cb) {

    sleep_gamepad(0.35f);

    auto mask = Ptr(new MaskWidget({0, 0, 0, 255}));
    mask->setName("CheatView::mask");
    addChild(mask);

    Ptr widget;

    auto scaleto = Action::Ptr(new ScaleTo(mask.get(), {1.0f, 1.0f}, 0.33f));
    mask->setScale(0.01f, 0.01f);
    mask->runAction(scaleto);

    auto root = New<WindowWidget>();
    root->setName("root");
    addChild(root);

    std::string title[2] = {"CHEAT LIST", "BACK TITLE"};
    auto font = res::load_ttf_font(res::fontName("prstart"), 18);
    font->setColor({255, 255, 255, 255});
    for (int i = 0; i < 2; ++i) {
        auto label = TTFLabel::New(title[i], font);
        label->setAnchor(0.0f, 0.5f);
        label->setPosition(80, 60 + i * 40);
        root->addChild(label);

        _position[i] = label->position() - Vector2f{40, 0};
    }

    auto animate = new FrameAnimationWidget;
    animate->setFrames({
        res::load_texture(_game.renderer(), res::imageName("tank_player1_right_c0_t1")),
        res::load_texture(_game.renderer(), res::imageName("tank_player1_right_c0_t2")),
    });
    animate->setAnchor(0.5f, 0.5f);
    animate->setPosition(_position[0]);
    animate->play(0.15f);
    animate->setName("tank");
    widget.reset(animate);
    root->addChild(widget);
    _icon = widget;
}

void CheatView::onButtonDown(int key) {
    if (key == KeyCode::L1 or key == KeyCode::B) {
        auto mask = find("CheatView::mask");
        auto scaleto  = Action::Ptr(new ScaleTo(mask, {0.01f, 0.01f}, 0.3f));
        auto callback = Action::New<CallBackVoid>(_callback);
        auto remove_self = Action::New<CallBackVoid>(std::bind(&CheatView::removeFromParent, this));
        auto action = Action::Ptr(new Sequence({scaleto, callback, remove_self}));
        this->runAction(action);
        find("root")->setVisible(false);
        sleep_gamepad(0.33f);
    } else if (key == KeyCode::UP) {
        _index = --_index <= -1 ? 1 : _index;
        _icon->setPosition(_position[_index]);
    } else if (key == KeyCode::DOWN) {
        _index = ++_index >= 2 ? 0 : _index;
        _icon->setPosition(_position[_index]);
    } else if (key == KeyCode::A) {
        if (_index == 1) {
            _game.screen().replace<StartView>();
        }
    }
}

//=====================================================================================

Widget::Ptr firstScene() {
    return Widget::New<LogoView>();
}
