#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <map>
#include <string>

#include "rendering/Shader.h"

struct Characterforfont
{
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

enum class FontFace
{
    Display,
    Body
};

class FontManager
{
public:
    FontManager();

    void SetScreenSize(float width, float height);
    void SetFont(FontFace face);

    void RenderText(
        const std::string& text,
        float x,
        float y,
        float scale,
        const glm::vec3& color,
        float tracking = 0.0f,
        float alpha = 1.0f
    );

    void RenderTextWrapped(
        const std::string& text,
        float x,
        float y,
        float scale,
        const glm::vec3& color,
        float maxWidth,
        float tracking = 0.0f,
        float alpha = 1.0f
    );

    void RenderRect(
        float x,
        float y,
        float width,
        float height,
        const glm::vec3& color,
        float alpha
    );

    void RenderTexture(
        unsigned int texture,
        float x,
        float y,
        float width,
        float height,
        const glm::vec3& tint,
        float alpha
    );

    float MeasureTextWidth(const std::string& text, float scale,
                           float tracking = 0.0f) const;
    float LineHeight(float scale) const;

private:
    bool LoadAtlas(FT_Library library, const std::string& path,
                   std::map<char, Characterforfont>& out);
    const std::map<char, Characterforfont>& ActiveCharacters() const;
    void BeginOverlayPass();
    void EndOverlayPass();
    void DrawGlyphQuad(float x, float y, float w, float h, unsigned int textureId);

    std::map<char, Characterforfont> displayCharacters;
    std::map<char, Characterforfont> bodyCharacters;
    FontFace activeFace = FontFace::Display;

    Shader shader;
    Shader overlayShader;

    glm::mat4 projection =
        glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

    float screenWidth = 800.0f;
    float screenHeight = 600.0f;
    float glyphPixelSize = 48.0f;

    FT_Library ft{};
    FT_Face face{};

    unsigned int VAO{};
    unsigned int VBO{};
    unsigned int whiteTexture{};
};
