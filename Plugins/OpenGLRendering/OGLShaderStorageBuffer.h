#pragma once

#include "glad\glad.h"
#include <memory>

/* Thin wrapper around an OpenGL SSBO. Should only be created via the factory down below.*/
namespace NCL {
    namespace Rendering {
        class OGLShaderStorageBuffer {
        public:

            inline GLuint GetID() const { return id; }

            void Bind() const {
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
            }

            void BindTo(GLuint bindingPoint) const {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, id);
            }

            void Unbind() const {
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }

            void UpdateData(const void* data, GLsizeiptr size, GLintptr offset = 0) {
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
                glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
            }

            ~OGLShaderStorageBuffer() {
                if (id) {
                    glDeleteBuffers(1, &id);
                }
            }

            // Prevent copying (SSBOs should be unique)
            OGLShaderStorageBuffer(const OGLShaderStorageBuffer&) = delete;
            OGLShaderStorageBuffer& operator=(const OGLShaderStorageBuffer&) = delete;

            // Allow move semantics
            OGLShaderStorageBuffer(OGLShaderStorageBuffer&& other) noexcept : id(std::exchange(other.id, 0)) {}
            OGLShaderStorageBuffer& operator=(OGLShaderStorageBuffer&& other) noexcept {
                if (this != &other) {
                    glDeleteBuffers(1, &id);
                    id = std::exchange(other.id, 0);
                }
                return *this;
            }

            /* Util RAII class for binding the buffer object within the current scope. */
            class ScopedBind {
            public:
                explicit ScopedBind(const OGLShaderStorageBuffer& buffer, GLuint bindingPoint)
                    : buffer_(buffer) {
                    buffer_.Bind();
                }

                ~ScopedBind() {
                    buffer_.Unbind();
                }

            private:
                const OGLShaderStorageBuffer& buffer_;
            };

        private:

            GLuint id;

            // Private constructor so that only the factory can create instances
            explicit OGLShaderStorageBuffer(GLuint bufferID) : id(bufferID) {}

            friend class SSBFactory;
        };

        /* Basic factory for constructing OpenGL SSBOs. Possibly move to OGLResourceManager.*/
        class SSBFactory {
        public:
            static OGLShaderStorageBuffer Create(GLsizeiptr size, GLenum usage = GL_STATIC_COPY) {
                GLuint bufferID;
                glGenBuffers(1, &bufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
                glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, usage);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
                return OGLShaderStorageBuffer(bufferID);
            }
        };
    }
}