#include "Application.h"
#include "Window.h"
#include "GLRenderer.h"
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL3/SDL.h>
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <future>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <cmath>


namespace App {

// コンストラクタ
Application::Application() {
	// Constructor implementation
}

// デストラクタ
Application::~Application() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
	SDL_Quit();
}


int Application::run() {

	// ----- SDL 初期化 -----
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
		std::cerr << "SDL init failed: " << SDL_GetError() << "\n";
		return m_exit_status = -1;
	}

	// ----- ウィンドウ生成 -----
	App::Window::Settings window_settings;
	m_window = std::make_unique<App::Window>(window_settings);
	if (!m_window->get_native_window() || !m_window->get_native_gl_context()) {
		std::cerr << "Window/GL context creation failed\n";
		return m_exit_status = -1;
	}

	// ----- GLAD 初期化 -----
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "GLAD init failed\n";
		return m_exit_status = -1;
	}

	// ----- ImGui 初期化 -----
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// 各操作の有効化フラグを設定する
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplSDL3_InitForOpenGL(m_window->get_native_window(), m_window->get_native_gl_context());
	ImGui_ImplOpenGL3_Init("#version 330");


	bool minimized = false;

	// ---- メインループ -----
	m_running = true;
	while (m_running) {

		// SDL イベント処理
		SDL_Event event;
		while (SDL_PollEvent(&event) && m_running) {
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) stop(); // ウィンドウ閉じるボタンで終了
			if (event.type == SDL_EVENT_WINDOW_MINIMIZED) minimized = true; // ウィンドウ最小化時に描画をスキップするためのフラグ
			if (event.type == SDL_EVENT_WINDOW_RESTORED || 
				event.type == SDL_EVENT_WINDOW_EXPOSED) minimized = false; // ウィンドウ復元時に描画を再開するためのフラグ
		}

		// ImGui フレーム開始
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// ImGui 描画データの確定
		ImGui::Render();

		// メインバックバッファへ描画する
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SDL_GL_SwapWindow(m_window->get_native_window());

	}// while (m_running)
}// run()

void Application::stop() {
	m_running = false;
}

}// namespace App