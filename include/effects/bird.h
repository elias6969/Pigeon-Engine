#ifndef BIRDTRAIL_H
#define BIRDTRAIL_H

#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>

class BirdTrail {
public:
    BirdTrail(Shader& shader, unsigned int maxTrailLength, float size);
    void update(const glm::vec2& screenPos);
    void draw();

private:
    void init();

    Shader& shader;
    unsigned int VAO, VBO;
    unsigned int texture;
    float size;
    unsigned int trailLimit;
    std::vector<glm::vec2> trailPositions;
};

#endif
