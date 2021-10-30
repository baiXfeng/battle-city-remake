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

    {
        auto mask = Ptr(new MaskWidget({255, 255, 255, 255}));
        addChild(mask);
    }

    auto root = New<Widget>();
    root->setAnchor(0.0f, 0.5f);
    root->setPositionY(size().y * 0.5f);
    addChild(root);

    {
        auto mask = New<MaskWidget>(SDL_Color{0, 0, 0, 255});
        addChild(mask);
        _mask = mask->to<MaskWidget>();
    }

    auto& state = _game.force_get<lutok3::State>("lua");
    std::string key[5] = {
            "AVATAR", "INDIENOVA", "QQ_GROUP", "DISCORD",
    };
    std::vector<std::string> value;
    for (int i = 0; i < 4; ++i) {
        if (state.getGlobal(key[i]) == lutok3::Type::String) {
            value.push_back(state.get());
        } else {
            value.push_back("");
        }
        state.pop();
    }

    if (value[0].size()) {
        auto view = New<ImageWidget>(res::load_texture(_game.renderer(), res::imageName(value[0])));
        view->setAnchor(0.5f, 0.0f);
        view->setPosition(size().x * 0.5f, 0.0f);
        root->addChild(view);
        root->setSize(Vector2f{size().x, view->size().y + 15});
    }

    auto font = res::load_ttf_font(res::fontName("prstart"), 26);
    font->setColor({0, 0, 0, 255});

    for (int i = 0; i < value.size()-1; ++i) {
        int idx = i + 1;
        if (value[idx].empty()) {
            continue;
        }
        auto label = TTFLabel::New(value[idx], font, {0.5f, 0.0f});
        label->setPosition(size().x * 0.5f, root->size().y + 10.0f);
        root->addChild(label);
        root->setSize(root->size() + Vector2f{0.0f, label->size().y + 20});
    }

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
        auto& state = _game.force_get<lutok3::State>("lua");
        std::string github;
        if (state.getGlobal("GITHUB") == lutok3::Type::String) {
            github = (std::string)state.get();
        }
        state.pop();

        auto title = new TTFLabel;
        title->setFont(font);
        title->setString(github);
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
        title->setString("README");
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

    _game.get<int>("level") = _level;

    Ptr widget;

    {
        auto view = new CurtainWidget({115, 115, 115, 255});
        view->fadeIn(_duration);
        widget.reset(view);
        addChild(widget);
        _curtain = view;
    }

    {
        std::string stage = std::string("STAGE ") + std::to_string(_game.force_get<int>("level"));
        auto font = res::load_ttf_font(fontName, 16);
        font->setColor({0, 0, 0, 255});
        auto label = TTFLabel::New(stage, font, {0.5f, 0.5f});
        label->setAnchor(0.5f, 0.5f);
        label->setPosition(size().x * 0.5f, size().y * 0.5f);
        label->setVisible(false);
        addChild(label);
        _label = label->to<TTFLabel>();
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
        auto& player = _game.force_get<PlayerModel>("player_model");
        player.life = Tank::getDefaultLifeMax();
        player.win = false;
        memset(player.killCount, 0, sizeof(player.killCount));
        Tank::resetPlayerScore();

        _game.get<int>("level") = _level;
        _game.screen().replace<BattleView>();
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

BattleView::~BattleView() {
    _game.event().remove(EventID::PAUSE_GAME, this);
    _game.event().remove(EventID::RESUME_GAME, this);
}

BattleView::BattleView() {

    _game.event().add(EventID::PAUSE_GAME, this);
    _game.event().add(EventID::RESUME_GAME, this);

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

        // 幕帘完全打开再开始运作战场
        float delay = Tank::getGlobalFloat("LEVEL_START_DELAY");
        view->enableUpdate(false);
        this->defer(view, [](Widget* sender) {
            sender->enableUpdate(true);
        }, delay);
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
        auto font = res::load_ttf_font(res::fontName("prstart"), 18);
        font->setColor({0, 0, 0, 255});
        auto label = TTFLabel::New("L-MENU", font);
        label->setPosition(15, 15);
        addChild(label);
    }

    {
        auto view = new CurtainWidget({115, 115, 115, 255});
        view->setState(CurtainWidget::ON);
        //view->fadeOut(0.3f);
        view->defer(view, [](Widget* sender){
            sender->to<CurtainWidget>()->fadeOut(0.3f);
            sender->find("stage:label")->setVisible(false);
        }, 1.8f);
        view->defer(view, [](Widget* sender){
            sender->removeFromParent();
        }, 1.8 + 0.3f);
        widget.reset(view);
        addChild(widget);

        {
            std::string stage = std::string("STAGE ") + std::to_string(_game.force_get<int>("level"));
            auto font = res::load_ttf_font(fontName, 16);
            font->setColor({0, 0, 0, 255});
            auto label = TTFLabel::New(stage, font, {0.5f, 0.5f});
            label->setPosition(size().x * 0.5f, size().y * 0.5f);
            label->setName("stage:label");
            view->addChild(label);
        }
    }

    _game.setRenderColor({115, 115, 115, 255});
    this->performLayout();

    auto sound = res::soundName("stage_start");
    _game.audio().loadEffect(sound);
    _game.audio().playEffect(sound);
}

void BattleView::onEvent(const Event &e) {

    if (e.Id() == EventID::PAUSE_GAME) {

        this->pauseAllActions();

    } else if (e.Id() == EventID::RESUME_GAME) {

        this->resumeAllActions();
    }
}

//=====================================================================================

static int const _enemy_icon_size = 20;

BattleInfoView::~BattleInfoView() {
    _game.event().remove(EventID::ENEMY_NUMBER_CHANGED, this);
    _game.event().remove(EventID::PLAYER_LIFE_CHANGED, this);
}

void BattleInfoView::onEvent(Event const& e) {
    if (e.Id() == EventID::ENEMY_NUMBER_CHANGED) {
        auto value = e.data<int>();
        onEnemyNumberChanged(value);
    } else if (e.Id() == EventID::PLAYER_LIFE_CHANGED) {
        auto value = e.data<int>();
        onPlayerNumberChanged(value);
    }
}

void BattleInfoView::onEnemyNumberChanged(int n) {
    for (int i = 0; i < 20; ++i) {
        _enemy[i]->setVisible(i < n);
    }
}

void BattleInfoView::onPlayerNumberChanged(int n) {
    _playerLife->setString(std::to_string(n));
}

BattleInfoView::BattleInfoView():
_playerLife(nullptr) {

    _game.event().add(EventID::ENEMY_NUMBER_CHANGED, this);
    _game.event().add(EventID::PLAYER_LIFE_CHANGED, this);

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
            _enemy[i] = widget;
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
        _playerLife = label;
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
    auto widget = new ImageWidget(icon);
    widget->setSize(_enemy_icon_size, _enemy_icon_size);
    return widget;
}

//=====================================================================================

TTFLabel* createLabel(std::string const& text, SDL_Color const& c, ScoreView::Alignment const& align) {
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

class ScoreItemView : public WindowWidget {
public:
    typedef std::function<void()> Callback;
    ScoreItemView(Texture::Ptr const& texture, Texture::Ptr const& arrow):_killCount(0), _oneScore(0), _currCount(0) {

        SDL_Color white = {255, 255, 255, 255};
        Ptr widget;

        {
            auto view = New<ImageWidget>(texture);
            view->setAnchor(0.5f, 0.5f);
            view->setPosition(size().x * 0.5f, 0.0f);
            this->addChild(view);
        }

        {
            auto view = New<ImageWidget>(arrow);
            view->setAnchor(0.5f, 0.5f);
            view->setPosition(size().x * 0.47f, 0.0f);
            this->addChild(view);
        }

        {
            auto label = createLabel("0", white, ScoreView::RIGHT);
            label->setPosition(size().x * 0.46f, 0.0f);
            label->setVisible(false);
            widget.reset(label);
            this->addChild(widget);
            _number = label;
        }

        {
            auto label = createLabel("PTS", white, ScoreView::RIGHT);
            label->setPosition(size().x * 0.4f, 0.0f);
            widget.reset(label);
            this->addChild(widget);
        }

        {
            auto label = createLabel("0", white, ScoreView::RIGHT);
            label->setPosition(size().x * 0.31f, 0.0f);
            label->setVisible(false);
            widget.reset(label);
            this->addChild(widget);
           _ops = label;
        }
    }
    void setScore(int kill_count, int one_score) {
        _killCount = kill_count;
        _oneScore = one_score;
    }
    void setFinishCall(Callback const& cb) {
        _callback = cb;
    }
    void play() {
        if (_killCount == 0) {
            auto delay = Action::New<Delay>(0.5f);
            auto call = Action::New<CallBackVoid>([&]{
                _number->setVisible(true);
                _ops->setVisible(true);
            });
            auto finish_call = Action::New<CallBackVoid>(_callback);
            runAction(Action::Ptr(new Sequence({delay, call, finish_call})));
        } else {
            defer(std::bind(&ScoreItemView::nextScore, this), 0.5f);
        }
    }
private:
    void nextScore() {
        _number->setVisible(true);
        _ops->setVisible(true);
        _number->setString(std::to_string(++_currCount));
        _ops->setString(std::to_string(_currCount * _oneScore));

        auto sound = res::soundName("score");
        _game.audio().loadEffect(sound);
        _game.audio().playEffect(sound);

        if (_currCount >= _killCount) {
            _callback();
            return;
        }
        auto delay = Action::New<Delay>(0.12f);
        auto call = Action::New<CallBackVoid>(std::bind(&ScoreItemView::nextScore, this));
        runAction(Action::Ptr(new Sequence({delay, call})));
    }
private:
    int _killCount;
    int _oneScore;
    int _currCount;
    TTFLabel* _number;
    TTFLabel* _ops;
    Callback _callback;
};

ScoreView::ScoreView():_total(nullptr) {
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
        auto level = _game.get<int>("level");
        auto label = createLabel("STAGE  " + std::to_string(level), white, MIDDLE);
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
        int score = _game.force_get<int>("player_score");
        auto label = createLabel(std::to_string(score), yellow, RIGHT);
        label->setPosition(size().x * 0.4f, size().y * 0.33f);
        widget.reset(label);
        root->addChild(widget);
    }

    Texture::Ptr texture[4] = {
            res::load_texture(_game.renderer(), res::imageName("tank_basic_up_c0_t1")),
            res::load_texture(_game.renderer(), res::imageName("tank_fast_up_c0_t1")),
            res::load_texture(_game.renderer(), res::imageName("tank_power_up_c0_t1")),
            res::load_texture(_game.renderer(), res::imageName("tank_armor_up_c0_t1")),
    };
    Texture::Ptr arrow = res::load_texture(_game.renderer(), res::imageName("arrow"));

    auto& player = _game.force_get<PlayerModel>("player_model");
    /*player.killCount[0] = 5;
    player.killCount[1] = 7;
    player.killCount[2] = 3;
    player.killCount[3] = 5;
    player.win = true;*/

    int const heightLine = 54;
    std::vector<Ptr> widgets;
    for (int i = 0; i < Tank::TIER_MAX; ++i) {
        auto widget = New<ScoreItemView>(texture[i], arrow);
        auto item = widget->to<ScoreItemView>();
        widget->setPosition(0.0f, i * heightLine + size().y * 0.44f);
        item->setScore(player.killCount[i], (i+1) * 100);
        root->addChild(widget);
        widgets.push_back(widget);
    }
    for (int i = 0; i < widgets.size(); ++i) {
        auto widget = widgets[i];
        auto item = widget->to<ScoreItemView>();
        if (i + 1 < widgets.size()) {
            item->setFinishCall(std::bind(&ScoreItemView::play, widgets[i+1]->to<ScoreItemView>()));
        } else {
            item->setFinishCall(std::bind(&ScoreView::showTotal, this));
        }
    }
    widgets.front()->to<ScoreItemView>()->play();

    auto mask = New<MaskWidget>(white);
    mask->setSize(size().x * 0.2f, 5.0f);
    mask->setAnchor(0.5f, 0.5f);
    mask->setPosition(size().x * 0.5f, size().y * 0.79f);
    root->addChild(mask);

    {
        int total = 0;
        for (int i = 0; i < Tank::TIER_MAX; ++i) {
            total += player.killCount[i];
        }
        auto label = createLabel(std::to_string(total), white, RIGHT);
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
}

void ScoreView::showTotal() {
    auto delay1 = Action::New<Delay>(0.8f);
    auto call1 = Action::New<CallBackVoid>([&]{
        _total->setVisible(true);
    });
    auto delay2 = Action::New<Delay>(1.9f);
    auto call2 = Action::New<CallBackVoid>(std::bind(&ScoreView::onNextScene, this));
    auto action = Action::Ptr(new Sequence({delay1, call1, delay2, call2}));
    runAction(action);
}

void ScoreView::onNextScene() {
    auto& player = _game.get<PlayerModel>("player_model");
    if (player.win) {
        int& level = _game.force_get<int>("level");
        int const& max_level = _game.force_get<int>("level_max");
        level = ++level > max_level ? 1 : level;
        _game.screen().replace<BattleView>();
    } else {
        _game.screen().replace<GameOverView>();
    }
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

    int const TITLE_MAX = 1;
    //std::string title[TITLE_MAX] = {"CHEAT LIST", "BACK TITLE"};
    std::string title[TITLE_MAX] = {"BACK TITLE"};
    auto font = res::load_ttf_font(res::fontName("prstart"), 18);
    font->setColor({255, 255, 255, 255});
    for (int i = 0; i < TITLE_MAX; ++i) {
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
        //_index = --_index <= -1 ? 1 : _index;
        _icon->setPosition(_position[_index]);
    } else if (key == KeyCode::DOWN) {
        //_index = ++_index >= 2 ? 0 : _index;
        _icon->setPosition(_position[_index]);
    } else if (key == KeyCode::A or key == KeyCode::START) {
        //if (_index == 1) {
            _game.screen().replace<StartView>();
        //}
    }
}

//=====================================================================================

BigExplosionView::BigExplosionView() {
    std::vector<Texture::Ptr> frames;
    for (int i = 0; i < 5; ++i) {
        frames.push_back(res::load_texture(_game.renderer(), res::imageName("big_explosion_" + std::to_string(i+1))));
    }
    for (int i = 0; i < 5; ++i) {
        frames.push_back(frames[5-i-1]);
    }
    setFrames(frames);
}

void BigExplosionView::play(FinishCall const& fc) {
    float const duration = 0.5f;
    FrameAnimationWidget::play(duration, false);
    this->defer(this, [](Widget* sender) {
        sender->setVisible(false);
    }, duration * 0.8f);
    auto delay = Action::New<Delay>(duration);
    auto finish_call = Action::New<CallBackVoid>(fc == nullptr ? []{} : fc);
    auto remove_self = Action::New<CallBackVoid>(std::bind(&Widget::removeFromParent, this));
    auto action = Action::Ptr(new Sequence({delay, finish_call, remove_self}));
    this->runAction(action);
}

//=====================================================================================

BulletExplosionView::BulletExplosionView() {
    std::vector<Texture::Ptr> frames;
    for (int i = 0; i < 3; ++i) {
        frames.push_back(res::load_texture(_game.renderer(), res::imageName("bullet_explosion_" + std::to_string(i+1))));
    }
    setFrames(frames);
}

void BulletExplosionView::play() {
    float const duration = 0.15f;
    FrameAnimationWidget::play(duration, false);
    this->defer(this, [](Widget* sender) {
        sender->setVisible(false);
    }, duration * 0.8f);
    this->defer(std::bind(&Widget::removeFromParent, this), duration);
}
