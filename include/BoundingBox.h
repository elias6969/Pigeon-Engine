#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"

class BoundingBox {
private:
    unsigned int VAO, VBO;
    std::vector<float> vertices;
    glm::vec3 min, max;
    Shader shader;

    void setupBuffers();

public:
    BoundingBox(glm::vec3 min, glm::vec3 max);
    ~BoundingBox();

    void update(glm::vec3 newMin, glm::vec3 newMax);
    void render(Camera& camera);
};

#endif // BOUNDING_BOX_H
