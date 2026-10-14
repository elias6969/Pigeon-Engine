#include "ui/fontManager.h"
#include <core/Variables.h>

#include <iostream>
#include <vector>

FontManager::FontManager()
{
    if (FT_Init_FreeType(&ft))
    {
        std::cerr
            << "ERROR::FREETYPE: Could not init FreeType Library"
            << std::endl;
        return;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const std::string displayFile = PathManager::fontpath + "Irish_Grover/irish.ttf";
    const std::string bodyFile =
        PathManager::fontpath + "IBM_Plex_Sans/IBMPlexSans-Regular.ttf";
    std::cout << "FONT PATH: " << displayFile << '\n';
    std::cout << "BODY FONT PATH: " << bodyFile << '\n';

    if (!LoadAtlas(ft, displayFile, displayCharacters))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load display font" << std::endl;
        FT_Done_FreeType(ft);
        return;
    }

    if (!LoadAtlas(ft, bodyFile, bodyCharacters))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load body font, using display face"
                  << std::endl;
        bodyCharacters = displayCharacters;
    }

    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    const unsigned char whitePixel = 255;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    shader.LoadShaders(
        (PathManager::shaderPath + "font.vs").c_str(),
        (PathManager::shaderPath + "font.fs").c_str());

    shader.use();
    shader.setInt("text", 0);
    shader.setFloat("textAlpha", 1.0f);
    shader.setMat4("projection", projection);

    overlayShader.LoadShaders(
        (PathManager::shaderPath + "font.vs").c_str(),
        (PathManager::shaderPath + "overlay.fs").c_str());
    overlayShader.use();
    overlayShader.setInt("image", 0);
    overlayShader.setVec3("tint", glm::vec3(1.0f));
    overlayShader.setFloat("alpha", 1.0f);
    overlayShader.setMat4("projection", projection);
}

bool FontManager::LoadAtlas(FT_Library library, const std::string &path,
                            std::map<char, Characterforfont> &out)
{
    FT_Face loaded{};
    if (FT_New_Face(library, path.c_str(), 0, &loaded))
        return false;

    FT_Set_Pixel_Sizes(loaded, 0, static_cast<unsigned int>(glyphPixelSize));

    for (unsigned char c = 0; c < 128; ++c)
    {
        if (FT_Load_Char(loaded, c, FT_LOAD_RENDER))
        {
            std::cerr
                << "ERROR::FREETYPE: Failed to load glyph: "
                << static_cast<int>(c)
                << std::endl;
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            loaded->glyph->bitmap.width,
            loaded->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            loaded->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Characterforfont character = {
            texture,
            glm::ivec2(loaded->glyph->bitmap.width, loaded->glyph->bitmap.rows),
            glm::ivec2(loaded->glyph->bitmap_left, loaded->glyph->bitmap_top),
            static_cast<unsigned int>(loaded->glyph->advance.x)};

        out.insert(std::pair<char, Characterforfont>(static_cast<char>(c), character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(loaded);
    return true;
}

void FontManager::SetScreenSize(float width, float height)
{
    if (width <= 0.0f || height <= 0.0f)
        return;

    if (width == screenWidth && height == screenHeight)
        return;

    screenWidth = width;
    screenHeight = height;
    projection = glm::ortho(0.0f, screenWidth, 0.0f, screenHeight);
    shader.use();
    shader.setMat4("projection", projection);
    overlayShader.use();
    overlayShader.setMat4("projection", projection);
}

void FontManager::SetFont(FontFace face)
{
    activeFace = face;
}

const std::map<char, Characterforfont> &FontManager::ActiveCharacters() const
{
    if (activeFace == FontFace::Body && !bodyCharacters.empty())
        return bodyCharacters;
    return displayCharacters;
}

void FontManager::BeginOverlayPass()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    shader.use();
    shader.setMat4("projection", projection);
    shader.setInt("text", 0);
}

void FontManager::EndOverlayPass()
{
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
}

void FontManager::DrawGlyphQuad(float x, float y, float w, float h, unsigned int textureId)
{
    float vertices[6][4] = {
        {x,     y + h, 0.0f, 0.0f},
        {x,     y,     0.0f, 1.0f},
        {x + w, y,     1.0f, 1.0f},
        {x,     y + h, 0.0f, 0.0f},
        {x + w, y,     1.0f, 1.0f},
        {x + w, y + h, 1.0f, 0.0f}
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

float FontManager::MeasureTextWidth(const std::string &text, float scale,
                                    float tracking) const
{
    float width = 0.0f;
    const auto &glyphs = ActiveCharacters();
    for (size_t i = 0; i < text.size(); ++i)
    {
        auto it = glyphs.find(text[i]);
        if (it == glyphs.end())
            continue;
        width += (it->second.Advance >> 6) * scale;
        if (i + 1 < text.size())
            width += tracking * scale;
    }
    return width;
}

float FontManager::LineHeight(float scale) const
{
    return glyphPixelSize * scale * 1.2f;
}

void FontManager::RenderRect(
    float x,
    float y,
    float width,
    float height,
    const glm::vec3 &color,
    float alpha)
{
    BeginOverlayPass();
    shader.setVec3("textColor", color);
    shader.setFloat("textAlpha", alpha);
    DrawGlyphQuad(x, y, width, height, whiteTexture);
    EndOverlayPass();
}

void FontManager::RenderTexture(
    unsigned int texture,
    float x,
    float y,
    float width,
    float height,
    const glm::vec3 &tint,
    float alpha)
{
    if (texture == 0 || width <= 0.0f || height <= 0.0f)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    overlayShader.use();
    overlayShader.setMat4("projection", projection);
    overlayShader.setInt("image", 0);
    overlayShader.setVec3("tint", tint);
    overlayShader.setFloat("alpha", alpha);
    DrawGlyphQuad(x, y, width, height, texture);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
}

void FontManager::RenderText(
    const std::string &text,
    float x,
    float y,
    float scale,
    const glm::vec3 &color,
    float tracking,
    float alpha)
{
    if (text.empty())
        return;

    BeginOverlayPass();
    shader.setVec3("textColor", color);
    shader.setFloat("textAlpha", alpha);
    glBindVertexArray(VAO);

    const auto &glyphs = ActiveCharacters();
    for (size_t i = 0; i < text.size(); ++i)
    {
        auto it = glyphs.find(text[i]);
        if (it == glyphs.end())
            continue;

        const Characterforfont &ch = it->second;
        const float xpos = x + ch.Bearing.x * scale;
        const float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        DrawGlyphQuad(xpos, ypos, ch.Size.x * scale, ch.Size.y * scale, ch.TextureID);
        x += (ch.Advance >> 6) * scale;
        if (i + 1 < text.size())
            x += tracking * scale;
    }

    EndOverlayPass();
}

void FontManager::RenderTextWrapped(
    const std::string &text,
    float x,
    float y,
    float scale,
    const glm::vec3 &color,
    float maxWidth,
    float tracking,
    float alpha)
{
    if (text.empty())
        return;

    const float lineHeight = LineHeight(scale);
    std::vector<std::string> lines;
    std::string currentLine;

    auto flushLine = [&]() {
        lines.push_back(currentLine);
        currentLine.clear();
    };

    std::string word;
    for (size_t i = 0; i <= text.size(); ++i)
    {
        const bool atEnd = i == text.size();
        const char c = atEnd ? ' ' : text[i];

        if (c == '\n')
        {
            if (!word.empty())
            {
                currentLine += currentLine.empty() ? word : " " + word;
                word.clear();
            }
            flushLine();
            continue;
        }

        if (c == ' ' || atEnd)
        {
            if (!word.empty())
            {
                const std::string candidate =
                    currentLine.empty() ? word : currentLine + " " + word;
                if (!currentLine.empty() &&
                    MeasureTextWidth(candidate, scale, tracking) > maxWidth)
                {
                    flushLine();
                    currentLine = word;
                }
                else
                {
                    currentLine = candidate;
                }
                word.clear();
            }
            continue;
        }

        word.push_back(c);
    }

    if (!currentLine.empty() || lines.empty())
        lines.push_back(currentLine);

    float cursorY = y;
    for (const std::string &line : lines)
    {
        RenderText(line, x, cursorY, scale, color, tracking, alpha);
        cursorY -= lineHeight;
    }
}
