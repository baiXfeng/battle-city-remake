//
// Created by baifeng on 2021/6/28.
//

#include "game.h"
#include "fps.h"
#include "widget.h"

#include <memory>
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#if defined(__NS__)
#include <switch.h>
#endif

mge_begin

SDL_Window* sdl_window = nullptr;
SDL_Renderer* sdl_renderer = nullptr;
Vector2i sdl_screen_size{GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT};
bool _quit = false;

//游戏控制器1处理机
SDL_Joystick* _gGameController = nullptr;

Game& Game::instance() {
    static Game i;
    return i;
}

SDL_Renderer* Game::renderer() const {
    return sdl_renderer;
}

SDL_Window* Game::window() const {
    return sdl_window;
}

Game::Delegate* Game::delegate() const {
    return _delegate;
}

void Game::setDelegate(Delegate* p) {
    _delegate = p;
}

void Game::setRenderColor(SDL_Color const& c) {
    _color = c;
}

Game::Game():_delegate(nullptr), _color({0, 0, 0, 255}) {

}

void Game::input() {
    SDL_Event e;
    while( SDL_PollEvent( &e ) != 0 ) {
        if( e.type == SDL_QUIT ) {
            _quit = true;
            continue;
        }
        gamepad().onEvent(e);
        mouse().onEvent(e);
    }
}

void Game::runActionOnMainThread(ActionPtr const& action) {
    screen().runAction(action);
}

int initSDL(std::string const& title);
void finiSDL();
int Game::run() {

    if (!_delegate) {
        printf("no delegate, app exiting...\n");
        return 1;
    }

#if defined(__NS__)
    romfsInit();
#endif

    auto size = _delegate->screenSize();
    sdl_screen_size = {size.x, size.y};
    int ret = initSDL(_delegate->windowTitle());
    if (ret != 0) {
        return ret;
    }

    this->initVariable();
    this->_delegate->init();
    float delta = fps().delta();

    while (!_quit) {
        fps().start();
        this->input();
        _delegate->update(delta);
        SDL_SetRenderDrawColor(sdl_renderer, _color.r, _color.g, _color.b, _color.a );
        SDL_RenderClear(sdl_renderer);
        _delegate->render(sdl_renderer);
        SDL_RenderPresent(sdl_renderer);
        fps().next();
    }

    _delegate->fini();
    this->finiVariable();
    this->clear();
    finiSDL();

#if defined(__NS__)
    romfsExit();
#endif

    return 0;
}

int initSDL(std::string const& windowTitle) {
    char rendername[256] = {0};
    SDL_RendererInfo info;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS);   // Initialize SDL2
    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    TTF_Init();

    //检查操纵杆
    if( SDL_NumJoysticks() < 1 ) {
        printf( "Warning: No joysticks connected!\n" );
    } else {
        //加载操纵杆
        _gGameController = SDL_JoystickOpen( 0 );
        if( _gGameController == nullptr ) {
            printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
        }
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Create an application window with the following settings:
    sdl_window = SDL_CreateWindow(
            windowTitle.c_str(),         //    const char* title
            SDL_WINDOWPOS_CENTERED,  //    int x: initial x position
            SDL_WINDOWPOS_CENTERED,  //    int y: initial y position
            sdl_screen_size.x,                      //    int w: width, in pixels
            sdl_screen_size.y,                      //    int h: height, in pixels
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE        //    Uint32 flags: window options, see docs
    );

    // Check that the window was successfully made
    if(sdl_window==NULL){
        // In the event that the window could not be made...
        std::cout << "Could not create window: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    for (int it = 0; it < SDL_GetNumRenderDrivers(); it++) {
        SDL_GetRenderDriverInfo(it, &info);
        strcat(rendername, info.name);
        strcat(rendername, " ");
    }

    sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_TARGETTEXTURE);
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(sdl_renderer, sdl_screen_size.x, sdl_screen_size.y);

    int result = Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 1024 );
    // Check load
    if ( result != 0 ) {
        std::cout << "Failed to open audio: " << Mix_GetError() << std::endl;
    }

    return 0;
}

void finiSDL() {
    // Close game controller
    SDL_JoystickClose( _gGameController );
    _gGameController = nullptr;

    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

Game::App::App() {
    _game.setDelegate(this);
}

int Game::App::run() {
    return _game.run();
}

mge_end
