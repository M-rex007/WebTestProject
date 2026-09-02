#pragma once
#include <string>
#include "SDL3/SDL.h"

namespace App {
	class Window
	{
	public:
		// ウィンドウ生成パラメーター
		struct Settings
		{
			std::string title = "WebTestProject"; // ウィンドウタイトル
			int width = 1280;	// 初期幅px
			int height = 720;	// 初期高さpx
		};

		// コンストラクタ: SDLウィンドウとOpenGLコンテキストの生成
		// explicit: Settings型 から Window への暗黙的な型変換を禁止する
		explicit Window(const Settings& settings);

		// デストラクタ: GLコンテキスト → SDLウィンドウの順に開放する
		~Window();

		// SDL ウィンドウへの生ポインタを返す (ImGui 初期化などで必要)
		[[nodiscard]] SDL_Window* get_native_window()     const { return m_window; }

		// OpenGL コンテキストへの生ポインタを返す
		[[nodiscard]] SDL_GLContext  get_native_gl_context() const { return m_gl_context; }

	private:
		// SDL ウィンドウハンドル
		SDL_Window* m_window{ nullptr };

		// OpenGL レンダリングコンテキスト
		SDL_GLContext m_gl_context;
	};
} // namespace App
