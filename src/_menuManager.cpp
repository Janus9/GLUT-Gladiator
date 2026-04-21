#include <_menuManager.h>

// ============ Menu Manager ============ //

// -- STATIC -- //

Vec2i _menuManager::windowDimensions = {0,0};

void _menuManager::setWindowDimensions(const Vec2i &dim) {
    cout << "Menu Manager dimensions resized to (" << dim.x << ", " << dim.y << ")\n";
    windowDimensions = dim;
}

// -- PUBLIC -- //

_menuManager::_menuManager() {
}

_menuManager::~_menuManager() {
}

void _menuManager::initMenuManager() {
    cout << "Initializing the menu manager ...\n";

    menuList[MENU_LANDING].initMenu("images/menu/landing_page.png");
    menuList[MENU_HOME].initMenu("images/menu/home_page.png");
    menuList[MENU_HELP].initMenu("images/menu/help_page.png");
    menuList[MENU_PAUSE].initMenu("images/menu/pause_page.png"); 
}

void _menuManager::drawMenuManager() {
    menuList[selectedMenu].drawMenu(windowDimensions);
}

void _menuManager::updateMenuManager(double dt) {
    menuList[selectedMenu].updateMenu(dt);
}

void _menuManager::loadMenu(menu_type type) {
    selectedMenu = type;
}

// ============ Menu ============ //

_menuManager::_menu::_menu() {
}

_menuManager::_menu::~_menu() {
    if (vboID != 0) {
        glDeleteBuffers(1,&vboID); // tell the GPU to delete the vertex buffer
        vboID = 0;
    }
    if (eboID != 0) {
        glDeleteBuffers(1,&eboID); // tell the GPU to delete the index buffer
        eboID = 0;
    }
    if (vaoID != 0) {
        glDeleteVertexArrays(1,&vaoID); // tell the GPU to delete the array buffer
        vaoID = 0;
    }
}

void _menuManager::_menu::initMenu(const string &fileName) {
    texture->loadTexture(fileName);

    glGenBuffers(1,&vboID);
    glGenBuffers(1,&eboID);
    glGenVertexArrays(1, &vaoID);

    shader->initShader("shaders/menu/vertex.vs","shaders/menu/fragment.fs");

    uint32_t program = shader->getProgram();

    u_texture = glGetUniformLocation(program,"u_texture");
    u_projection = glGetUniformLocation(program,"u_projection");
    u_model = glGetUniformLocation(program,"u_model");

    buildVBO();
    buildEBO();
    buildVAO();
}

void _menuManager::_menu::drawMenu(const Vec2i &wDim) {
    shader->useProgram();

    texture->bindTexture();

    // Projection Matrix - Left, Right, Top, Bottom
    glm::mat4 projection = glm::ortho(0.0f, (float)wDim.x, 0.0f, (float)wDim.y);
    
    // We skip view matrix since its a menu (no camera movement)
    
    // Model Matrix
    glm::mat4 model(1.0f);  // Creates an identity matrix
    // Scales the model to be the size of the window. 
    model = glm::scale(model, glm::vec3((float)wDim.x, (float)wDim.y, 1.0f));

    // For texture in fragment shader
    glUniform1i(u_texture,0);

    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(vaoID);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void _menuManager::_menu::updateMenu(double dt) {

}

void _menuManager::_menu::buildVBO() {
    /**
     * primitives * vertices * 
     */
    float vboData[16];
    int vIndex = 0;

    // Bottom-Left //
    vboData[vIndex++] = 0.0f;
    vboData[vIndex++] = 0.0f;
    vboData[vIndex++] = 0.0f;
    vboData[vIndex++] = 1.0f;
    // Bottom-Right //
    vboData[vIndex++] = 1.0f;
    vboData[vIndex++] = 0.0f;
    vboData[vIndex++] = 1.0f;
    vboData[vIndex++] = 1.0f;
    // Top-Right //
    vboData[vIndex++] = 1.0f;
    vboData[vIndex++] = 1.0f;
    vboData[vIndex++] = 1.0f;
    vboData[vIndex++] = 0.0f;
    // Top-Left //
    vboData[vIndex++] = 0.0f;
    vboData[vIndex++] = 1.0f;
    vboData[vIndex++] = 0.0f;
    vboData[vIndex++] = 0.0f;

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vboData), &vboData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void _menuManager::_menu::buildEBO() {
    uint32_t eboData[6];
    int eIndex = 0;

    // Triangle 1
    eboData[eIndex++] = 0;  // BL 
    eboData[eIndex++] = 1;  // BR
    eboData[eIndex++] = 2;  // TR
    
    // Triangle 2
    eboData[eIndex++] = 0;  // BL
    eboData[eIndex++] = 2;  // TR
    eboData[eIndex++] = 3;  // TL
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(eboData), &eboData, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

void _menuManager::_menu::buildVAO() {
    glBindVertexArray(vaoID);

    glBindBuffer(GL_ARRAY_BUFFER,vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,eboID);

    GLsizei stride = 4 * sizeof(float);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)(0 * sizeof(float)));

    // Texture Coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

