#ifndef RENDERER_H
#define RENDERER_H

#include <_common.h>

#include <glm/glm.hpp>                      // Core library
#include <glm/gtc/matrix_transform.hpp>     // Matrix ops like transform, scale, ortho, etc
#include <glm/gtc/type_ptr.hpp>             // Send GLM datatypes (matrix) to GPU

namespace renderer {

    struct RenderItemConfig {
        std::string ID;     
        int maxRenderCount; 
    };

    class Engine {
        public:
            Engine();
            virtual ~Engine();

            // Render Item //

            bool addRenderItem(const RenderItemConfig &config);
            bool addFloatAttribute(const std::string &ID, int size);
            bool updateFloatMesh(
                const std::string &ID, 
                const std::vector<float> &verticies, 
                size_t startByteOffset
            );
            bool submitDrawRange(
                const std::string &ID,
                size_t renderItemCount,
                size_t byteOffset
            );
            
            // Render Batch //

            bool addRenderBatch(const std::string ID);
            bool addShader(
                const std::string &ID,
                const std::string &vShaderPath, 
                const std::string &fShaderPath
            );
            void addUniform1i(
                const std::string &ID,
                const std::string &name,
                int* data
            );
            void addUniform1f(
                const std::string &ID,
                const std::string &name,
                float* data
            );
            void addUniformMatrix4fv(
                const std::string &ID,
                const std::string &name,
                glm::mat4* data
            );
            
            // Flushes out all render items using the provided batchID to apply
            void flush(const std::string ID);

        protected:
        private:
    };
}

#endif // RENDERER_H