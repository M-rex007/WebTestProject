#pragma once
#include <glad/glad.h>
#include <string>

namespace Renderer {

    // OpenGL フレームバッファオブジェクト (FBO) を管理するクラス
    // 3D シーンをオフスクリーンのテクスチャへ描画し、
    // ImGui::Image() 経由で UI ウィンドウ内に貼り付けるために使用する
    class GLRenderer
    {
    public:
        GLRenderer() = default;
        ~GLRenderer() { cleanup(); } // デストラクタで FBO・テクスチャ・RBO を解放

        // コピー禁止 (OpenGL オブジェクトはコピーできないため)
        GLRenderer(const GLRenderer&)            = delete;
        GLRenderer& operator=(const GLRenderer&) = delete;

        // FBO とアタッチメントを初期化する
        // OpenGL コンテキストが有効になった後に呼び出すこと
        // 戻り値: 成功なら true
        bool init(int width, int height);

        // FBO をバインドして描画先を切り替える
        // 呼び出し後は glDraw* が FBO のカラーテクスチャへ書き込まれる
        // 内部で glClear・glEnable(GL_DEPTH_TEST) も行う
        void bind();

        // FBO をアンバインドして描画先をデフォルトバックバッファに戻す
        void unbind();

        // ウィンドウリサイズ時に FBO をリサイズする
        // サイズが変わっていなければ何もしない
        void resize(int width, int height);

        // ImGui::Image() に渡すカラーテクスチャの ID を返す
        GLuint getColorTexture() const { return m_color_texture; }

        int getWidth()  const { return m_width;  }
        int getHeight() const { return m_height; }

        // 頂点シェーダーとフラグメントシェーダーをファイルから読み込み、
        // コンパイル・リンクしてシェーダープログラムを返す
        // 失敗した場合は 0 を返す
        static GLuint loadShaderProgram(const std::string& vertPath,
                                        const std::string& fragPath);

    private:
        GLuint m_fbo              = 0; // フレームバッファオブジェクト
        GLuint m_color_texture    = 0; // カラーアタッチメント (ImGui に渡すテクスチャ)
        GLuint m_depth_renderbuffer = 0; // 深度バッファ (RBO: Renderbuffer Object)
        int    m_width  = 0;
        int    m_height = 0;

        // FBO・テクスチャ・RBO をまとめて生成する
        bool createFBO(int w, int h);

        // FBO・テクスチャ・RBO をまとめて解放する
        void cleanup();

        // ファイルパスからシェーダーソースを文字列として読み込む
        static std::string readFile(const std::string& path);

        // 指定した種別 (GL_VERTEX_SHADER / GL_FRAGMENT_SHADER) のシェーダーをコンパイルする
        static GLuint compileShader(GLenum type, const std::string& src);
    };

} // namespace Renderer
