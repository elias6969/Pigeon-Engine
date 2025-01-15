#ifndef PIGEON_H
#define PIGEON_H

namespace Pigeon {

    void initialize();
    void update(float deltaTime);
    void render(float deltaTime);
    void cleanup();
    void run(); // Combines everything and serves as the main execution loop.
} 
#endif // PIGEON_H
