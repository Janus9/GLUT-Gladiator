#ifndef _SHADER_H
#define _SHADER_H

#include <_common.h>

class _shader {
    public:
        _shader();
        virtual ~_shader();

        /**
         * Loads a shader file
         * 
         * @param fileName Directory of shader file to load
         * 
         * @return string of data containing the shader code
         */
        string loadFile(const string &fileName);

        /**
         * Loads a given shader of vertex or fragment into the GPU and returns an ID
         * 
         * @param source string source of the shader 
         * 
         * @param mode What kind (vertex/fragment) shader is being loaded
         * 
         * @return unsigned int of the shader ID
         */
        uint32_t loadShader(const string &source, uint32_t mode);

        /**
         * Initializes the the vertex and fragment shaders
         * 
         * @param vertShader Vertex Shader directory
         * 
         * @param fragShader Fragment Shader directory
         */
        void initShader(const string &vertShader, const string &fragShader);

        // Cleans the shaders
        void cleanUp();

        // Uses the setup program
        void useProgram();

        // Returns the program
        uint32_t getProgram();

        // Sets program to 0
        void clearProgram();
    protected:
    private:
        uint32_t vs, fs; // Handlers
        uint32_t program;
};

#endif // _SHADER_H