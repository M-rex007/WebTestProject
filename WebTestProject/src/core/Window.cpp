#include "Window.h"
#include "SDL3\SDL.h"
#include <iostream>

namespace App {

    // コンストラクタ: ウィンドウと OpenGL コンテキストを生成する
    Window::Window(const Settings& settings)
    {
        // ウィンドウフラグを設定する
        // RESIZABLE      : ユーザーがウィンドウをリサイズ可能にする
        // HIGH_PIXEL_DENSITY : 高 DPI (HiDPI / Retina) ディスプレイに対応する
        // OPENGL         : OpenGL レンダリングを有効にする
        SDL_WindowFlags window_flags =
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL;

        // SDL ウィンドウを生成する
        m_window = SDL_CreateWindow(
            settings.title.c_str(), settings.width, settings.height, window_flags);
        if (!m_window) {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return;
        }

        // OpenGL コンテキストのバージョンをリクエストする
        // バージョン 3.3 コアプロファイルを要求する (Compatibility プロファイルは使用しない)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        // OpenGL コンテキストを生成する
        // コンテキストは GPU 上の描画状態・リソースを保持するオブジェクト
        m_gl_context = SDL_GL_CreateContext(m_window);
        if (!m_gl_context) {
            std::cerr << "OpenGL Context creation failed: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(m_window);
            SDL_Quit();
            return;
        }

        // 生成したコンテキストをこのスレッドのカレントコンテキストに設定する
        // glXxx() 関数はカレントコンテキストに対して動作する
        SDL_GL_MakeCurrent(m_window, m_gl_context);

        // VSync を有効にする
        // ディスプレイのリフレッシュレートに合わせてバッファスワップを待機し
        // ティアリング (画面の縦断面のズレ) を防ぐ
        SDL_GL_SetSwapInterval(1);

    } // Window()

    // デストラクタ: リソースを逆順に解放する
    // GL コンテキストを先に破棄し、その後にウィンドウを破棄する
    // (ウィンドウを先に破棄するとコンテキストが無効になる可能性があるため)
    Window::~Window()
    {
        if (m_gl_context) {
            SDL_GL_DestroyContext(m_gl_context); // OpenGL コンテキストを解放
        }
        if (m_window) {
            SDL_DestroyWindow(m_window);         // SDL ウィンドウを解放
        }
    } // ~Window()

} // namespace App
