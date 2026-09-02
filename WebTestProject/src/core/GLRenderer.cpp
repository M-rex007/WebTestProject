#include "GLRenderer.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace Renderer {

    // -------------------------------------------------------------------------
    // 公開インターフェイス
    // -------------------------------------------------------------------------

    // FBO を初期化する。GL コンテキストが有効な状態で呼ぶこと
    bool GLRenderer::init(int width, int height) {
        return createFBO(width, height);
    }

    // FBO をバインドし、3D 描画の準備をする
    // ・バインドすることで以降の glDraw* が FBO へ書き込まれる
    // ・ビューポートを FBO サイズに合わせる
    // ・背景色 (深い紺色) でカラーと深度バッファをクリアする
    // ・深度テストを有効にする (奥にあるポリゴンが手前に見えないようにする)
    void GLRenderer::bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_width, m_height);
        glClearColor(0.05f, 0.05f, 0.12f, 1.0f); // 背景: 深い紺色
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
    }

    // FBO をアンバインドして、描画先をデフォルトバックバッファに戻す
    // 深度テストも無効化して後続の ImGui 描画に影響を与えないようにする
    void GLRenderer::unbind() {
        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // FBO をリサイズする
    // サイズが変わっていなければ早期リターンして不要な再生成を避ける
    // 変わっている場合は既存リソースを解放してから新しいサイズで再生成する
    void GLRenderer::resize(int width, int height) {
        if (width == m_width && height == m_height) return;
        cleanup();
        createFBO(width, height);
    }

    // -------------------------------------------------------------------------
    // シェーダー読み込み (static)
    // -------------------------------------------------------------------------

    // 頂点シェーダーとフラグメントシェーダーをファイルから読み込んでコンパイル・リンクする
    // 戻り値: シェーダープログラム ID (失敗時は 0)
    GLuint GLRenderer::loadShaderProgram(const std::string& vertPath,
        const std::string& fragPath)
    {
        const std::string vertSrc = readFile(vertPath);
        const std::string fragSrc = readFile(fragPath);

        if (vertSrc.empty() || fragSrc.empty()) {
            std::cerr << "[GLRenderer] シェーダーファイルの読み込みに失敗しました\n"
                << "  vert: " << vertPath << "\n"
                << "  frag: " << fragPath << "\n";
            return 0;
        }

        // 各シェーダーをコンパイルする
        const GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
        const GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
        if (!vert || !frag) {
            glDeleteShader(vert);
            glDeleteShader(frag);
            return 0;
        }

        // プログラムオブジェクトを生成してシェーダーをリンクする
        const GLuint program = glCreateProgram();
        glAttachShader(program, vert);
        glAttachShader(program, frag);
        glLinkProgram(program);

        // リンク後はシェーダーオブジェクト単体は不要になるため削除する
        glDeleteShader(vert);
        glDeleteShader(frag);

        // リンク結果を確認する
        GLint ok = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[512];
            glGetProgramInfoLog(program, sizeof(log), nullptr, log);
            std::cerr << "[GLRenderer] シェーダーリンクエラー:\n" << log << "\n";
            glDeleteProgram(program);
            return 0;
        }

        return program;
    }

    // -------------------------------------------------------------------------
    // プライベート
    // -------------------------------------------------------------------------

    // FBO・カラーテクスチャ・深度 RBO を生成してアタッチする
    // 1. カラーテクスチャ: RGBA8 フォーマット。ImGui::Image() に渡す描画先
    // 2. 深度 RBO: 24bit 深度値の格納専用バッファ。テクスチャより軽量
    // 3. FBO: 上記 2 つをアタッチしてオフスクリーン描画先として機能する
    bool GLRenderer::createFBO(int w, int h) {
        m_width = w;
        m_height = h;

        // 1. カラーテクスチャを生成する (GL_RGBA8: 各チャンネル 8bit)
        glGenTextures(1, &m_color_texture);
        glBindTexture(GL_TEXTURE_2D, m_color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr); // 初期データは空
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 2. 深度 RBO を生成する
        // テクスチャとして読み出す必要がないため、軽量な RBO で十分
        glGenRenderbuffers(1, &m_depth_renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth_renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);

        // 3. FBO を生成してカラーテクスチャと深度 RBO をアタッチする
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, m_color_texture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, m_depth_renderbuffer);

        // FBO の完全性を検証する
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "[GLRenderer] FBO が不完全です\n";
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }

    // FBO・テクスチャ・RBO を解放して ID を 0 にリセットする
    // resize() やデストラクタから呼ばれる
    void GLRenderer::cleanup() {
        if (m_fbo) { glDeleteFramebuffers(1, &m_fbo);               m_fbo = 0; }
        if (m_color_texture) { glDeleteTextures(1, &m_color_texture);      m_color_texture = 0; }
        if (m_depth_renderbuffer) { glDeleteRenderbuffers(1, &m_depth_renderbuffer); m_depth_renderbuffer = 0; }
        m_width = m_height = 0;
    }

    // ファイルパスを受け取り、内容を std::string として返す
    // 失敗した場合は空文字列を返す
    std::string GLRenderer::readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    // 指定した種別のシェーダーをコンパイルする
    // 成功したらシェーダー ID を返し、失敗したら 0 を返す
    GLuint GLRenderer::compileShader(GLenum type, const std::string& src) {
        const GLuint shader = glCreateShader(type);
        const char* cstr = src.c_str();
        glShaderSource(shader, 1, &cstr, nullptr);
        glCompileShader(shader);

        GLint ok = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512];
            glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
            const char* typeName =
                (type == GL_VERTEX_SHADER) ? "頂点" : "フラグメント";
            std::cerr << "[GLRenderer] " << typeName
                << "シェーダーコンパイルエラー:\n" << log << "\n";
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

} // namespace Renderer
