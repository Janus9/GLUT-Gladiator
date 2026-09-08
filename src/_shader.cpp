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
        GG_LOG_ERROR(
            LOG_SHADER,
            "Error opening file: '%s'",
            fileName.c_str()
        );
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
    uint32_t id = glCreateShader(mode);

    const char* c_source = source.c_str(); // source must be char* for API 
    glShaderSource(id,1,&c_source,nullptr); 
    glCompileShader(id);
    
    GLint compileStatus = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);

    GLint logLength = 0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 1) {
        std::vector<char> log(static_cast<size_t>(logLength));
        glGetShaderInfoLog(id, logLength, nullptr, log.data()); // Error output for shader compilation
        
        GG_LOG_ERROR(
            LOG_SHADER, 
            "Shader Compilation Status: '%s'",
            log.data()
        );
    }

    if (compileStatus != GL_TRUE) {
        GG_LOG_ERROR(LOG_SHADER, "Shader compilation failed");
        glDeleteShader(id);
        return 0;
    }

    return id;
}

void _shader::initShader(const std::string &vertShader, const std::string &fragShader) {
    GG_LOG_INFO(
        LOG_SHADER,
        "Loading Vertex Shader: '%s'",
        vertShader.c_str()
    );

    // Load vertex shader
    std::string source = loadFile(vertShader);
    vs = loadShader(source,GL_VERTEX_SHADER);

    GG_LOG_INFO(
        LOG_SHADER,
        "Loading Fragment Shader: '%s'",
        fragShader.c_str()
    );

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

