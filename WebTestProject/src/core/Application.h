#pragma once
#include "SDL3/SDL.h"
#include "memory"

namespace App {
	
	class Window;

	class Application
	{
	public:
		// コンストラクタ: SDLの基本初期化
		Application();

		// デストラクタ: ImGuiとSDLの終了処理
		~Application();

		// メインループ
		// @return OSに返す終了コード
		int run();

		// メインループの終了処理 (m_runningをfalseにする)
		void stop();

	private:
		// 終了ステータスコード (0 = 正常)
		int m_exit_status = 0;

		// メインループ継続フラグ
		bool m_running = true;

		// SDLウィンドウ管理オブジェクト (遅延構築のため unique_ptr)
		std::unique_ptr<Window> m_window{ nullptr };
	};

}// namespace App
