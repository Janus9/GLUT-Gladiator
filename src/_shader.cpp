#include <_shader.h>

_shader::_shader() {
    // ctor
}

_shader::~_shader() {
    cleanUp();
}

std::string _shader::loadFile(const std::string &fileName) {
    const size_t BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    std::ifstream file(fileName);
    std::string str;
    if (!file) {
        std::cerr << "Error opening file: " << fileName << "\n";
        return str;
    }
    while(file) {
        file.read(buffer,BUFFER_SIZE);  // Read BUFFER_SIZE into buffer
        std::streamsize bytesRead = file.gcount();   // Returns how many bytes read (this is for end when we read partial buffer)
        if (bytesRead > 0) {
            str.append(buffer,(size_t)bytesRead); // Append the buffer, but buffer is fixed size so we specify how many bytes exactly
        }
    }

    return str;
}

uint32_t _shader::loadShader(const std::string &source, uint32_t mode) {
    uint32_t id;

    const char* c_source = source.c_str(); // source must be char* for API 
    char error[1024]; // Error buffer

    id = glCreateShader(mode);

    glShaderSource(id,1,&c_source,NULL); 
    glCompileShader(id);

    glGetShaderInfoLog(id,1024, NULL, error); // Error output for shader compilation

    std::cout << "Shader Compilation Status: " << error << "\n";

    return id;
}

void _shader::initShader(const std::string &vertShader, const std::string &fragShader) {
    std::cout << "Loading Shader: " << vertShader << "\n";

    // Load vertex shader
    std::string source = loadFile(vertShader);
    vs = loadShader(source,GL_VERTEX_SHADER);

    std::cout << "Loading Shader: " << fragShader << "\n";

    // Load fragment shader
    source = loadFile(fragShader);
    fs = loadShader(source,GL_FRAGMENT_SHADER);

    program = glCreateProgram();

    glAttachShader(program,vs);
    glAttachShader(program,fs);

    glLinkProgram(program);

}

void _shader::cleanUp() {
    glDetachShader(program,vs);
    glDetachShader(program,fs);

    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteShader(program);
}

void _shader::useProgram() {
    glUseProgram(program);
}

uint32_t _shader::getProgram() {
    return program;
}

void _shader::clearProgram() {
    glUseProgram(0);
}

