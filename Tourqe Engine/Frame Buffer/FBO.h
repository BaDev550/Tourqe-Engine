#ifndef FBO_H
#define FBO_H

#include <glad/glad.h>

class FBO {
public:
    FBO(int width, int height);
    ~FBO();

    void Bind();   // Bind FBO for rendering
    void Unbind(); // Unbind FBO to revert to the default framebuffer
    GLuint GetTexture() const; // Get the texture to render in ImGui

private:
    GLuint framebuffer;
    GLuint texColorBuffer;
    GLuint rbo;  // Optional: For depth/stencil buffer
    int width, height;

    void CreateFramebuffer();
};

#endif

