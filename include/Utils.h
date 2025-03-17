#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>

// Checks if two positions are colliding based on a given size.
bool CheckCollision(const glm::vec3 &Position1, const glm::vec3 &Position2, float size);

// Sets the screen dimensions.
void ScreenSize(int &screenWidth, int &screenHeight);

// Level of Detail enumeration.
enum LOD {
    BASIC = 0,
    MEDIUM = 1,
    DETAILED = 2
};

#endif // UTILS_H

