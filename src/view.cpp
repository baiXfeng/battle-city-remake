//
// Created by baifeng on 2021/9/29.
//

#include "view.h"
#include "common/loadres.h"
#include "common/game.h"
#include "common/action.h"
#include "common/audio.h"

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

        if (false) {
            auto call = Action::New<CallBackVoid>([&]{
                auto a1 = Action::Ptr(new PushSceneAction<HelloWorld>(true));
                auto a1_delay = Action::Ptr(new Delay(1.0f));
                auto a1_seq = Action::Ptr(new Sequence({a1, a1_delay}));
                auto a2 = Action::Ptr(new CallBackVoid([]() {
                    //printf("开幕.\n");
                }));
                //_game.screen().cut_to(a1_seq, 0.33f);
            });
            auto delay = Action::Ptr(new Delay(2.0f));
            auto seq = Action::Ptr(new Sequence({delay, call}));
            auto action = Action::Ptr(new Repeat(seq));
            root->runAction(action);
        }

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
    GamePadWidget::onEnter();
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
    auto font = res::load_ttf_font(fontName, 16);
    font->setColor({255, 255, 255, 255});

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
        title->setString("2 PLAYER");
        title->setAnchor(0.5f, 0.0f);
        title->setPosition(size().x * 0.5f, size().y * 0.58f);
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
        copyright->setPosition(title->position().x - title->size().x * 0.5f, size().y * 0.9f - 1);
    }

    root->performLayout();
}

void StartView::onEnter() {
    GamePadWidget::onEnter();
    auto root = find("root");
    auto move = Action::New<MoveBy>(root, Vector2f{0.0f, -size().y}, 3.0f);
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
        auto view = new CurtainWidget({128, 128, 128, 255});
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
            sleep_gamepad(1.8f);
            _game.screen().replace<BattleView>();
        }, 1.8f);
    }
}

void SelectLevelView::onButtonUp(int key) {
    if (key == KeyCode::A or key == KeyCode::X or key == KeyCode::B or key == KeyCode::Y) {
        stopAutoAddLevel();
    }
}

void SelectLevelView::addLevel() {
    _level = ++_level >= 35 ? 35 : _level;
    _label->setString("STAGE " + std::to_string(_level));
}

void SelectLevelView::subLevel() {
    _level = --_level <= 1 ? 1 : _level;
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
        auto view = new CurtainWidget({128, 128, 128, 255});
        view->fadeOut(0.3f);
        widget.reset(view);
        addChild(widget);

        auto delay = Action::New<Delay>(0.3f);
        auto call = Action::Ptr(new CallBackSender(view, [&](Widget* sender) {
            sender->removeFromParent();
        }));
        view->runAction(Action::Ptr(new Sequence({delay, call})));
    }

    _game.setRenderColor({128, 128, 128, 255});
    this->performLayout();
}

//=====================================================================================

static int const _mapSize = 13 * 40;

BattleFieldView::BattleFieldView() {

    auto old_size = this->size();
    this->setSize(_mapSize, _mapSize);

    Ptr widget;

    {
        auto view = Ptr(new MaskWidget({0, 0, 0, 255}));
        view->setSize(size());
        addChild(view);
    }
}

//=====================================================================================

static int const _enemy_icon_size = 20;

BattleInfoView::BattleInfoView() {

    Ptr widget;
    int const padding = 18;
    auto const old_size = size();

    setSize(_enemy_icon_size * 2 + padding * 2, _mapSize);

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
        view->setPosition(size().x * 0.5f, _mapSize * 0.85f);
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

    this->setPosition(_mapSize, 0.0f);
}

ImageWidget* BattleInfoView::createEnemyIcon() {
    auto icon = res::load_texture(_game.renderer(), "assets/images/enemy.png");
    //auto sign = res::load_texture(_game.renderer(), "");
    auto widget = new ImageWidget(icon);
    widget->setSize(_enemy_icon_size, _enemy_icon_size);
    return widget;
}

//=====================================================================================

Widget::Ptr firstScene() {
    return Widget::New<BattleView>();
}
