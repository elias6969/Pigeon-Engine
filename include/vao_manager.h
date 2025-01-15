#ifndef VAO_MANAGER_H
#define VAO_MANAGER_H

#include <glad/glad.h>
#include <vector>

class VAOManager {
public:
    // Constructor
    VAOManager() : VAO(0), VBO(0) {}

    // Destructor
    ~VAOManager() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    // Method to initialize the VAO and VBO
    void setupVAO(const std::vector<float>& vertices, const std::vector<int>& attributeSizes) {
        // Generate and bind VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Generate and bind VBO
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Calculate stride and set vertex attributes
        int stride = 0;
        for (int size : attributeSizes) {
            stride += size;
        }

        stride *= sizeof(float);

        // Enable and define vertex attributes
        int offset = 0;
        for (size_t i = 0; i < attributeSizes.size(); ++i) {
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attributeSizes[i], GL_FLOAT, GL_FALSE, stride, (void*)(offset * sizeof(float)));
            offset += attributeSizes[i];
        }

        // Unbind VAO
        glBindVertexArray(0);
    }

    // Getter for VAO
    unsigned int getVAO() const {
        return VAO;
    }

private:
    unsigned int VAO, VBO;
};

#endif // VAO_MANAGER_H
