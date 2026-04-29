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

void _menuManager::initMenuManager(_sounds* sharedSounds, _scene* _scene) {
    cout << "Initializing the menu manager ...\n";

    sounds = sharedSounds;
    scene = _scene;

    //  -- Landing --  //
    menuList[MENU_LANDING].initMenu(MENU_LANDING);
    menuList[MENU_LANDING].addMenuObject({
        "images/menu/landing_page.png",
        {1.0f, 1.0f},
        {0.5f, 0.5f},
        false,
        "landing_bg",
        MENU_LANDING,
        MENU_NULL
    });
    menuList[MENU_LANDING].addMenuObject({
        "images/menu/continue_button.png",
        {0.2f, 0.2f},
        {0.5f, 0.2f},
        true,
        "landing_continue_button",
        MENU_LANDING,
        MENU_HOME
    });

    // -- Home -- //
    menuList[MENU_HOME].initMenu(MENU_HOME);
    menuList[MENU_HOME].addMenuObject({
        "images/menu/home_page.png",
        {1.0f, 1.0f},
        {0.5f, 0.5f},
        false,
        "home_bg",
        MENU_HOME,
        MENU_NULL
    });
    menuList[MENU_HOME].addMenuObject({
        "images/menu/play_button.png",
        {0.2f, 0.2f},
        {0.5f, 0.2f},
        true,
        "home_start_button",
        MENU_HOME,
        MENU_SAVES
    });
    menuList[MENU_HOME].addMenuObject({
        "images/menu/help_button.png",
        {0.2f, 0.2f},
        {0.15f, 0.2f},
        true,
        "home_help_button",
        MENU_HOME,
        MENU_HELP
    });
    menuList[MENU_HOME].addMenuObject({
        "images/menu/landing_button.png",
        {0.2f, 0.2f},
        {0.85f, 0.2f},
        true,
        "home_landing_button",
        MENU_HOME,
        MENU_LANDING
    });

    // -- Saves -- //
    menuList[MENU_SAVES].initMenu(MENU_SAVES);
    menuList[MENU_SAVES].addMenuObject({
        "images/menu/landing_page.png",
        {1.0f, 1.0f},
        {0.5f, 0.5f},
        false,
        "saves_bg",
        MENU_SAVES,
        MENU_NULL
    });
    menuList[MENU_SAVES].addMenuObject({
        "images/menu/generate_button.png",
        {0.2f, 0.2f},
        {0.35f, 0.2f},
        true,
        "saves_generate_button",
        MENU_SAVES,
        MENU_NULL
    });
    menuList[MENU_SAVES].addMenuObject({
        "images/menu/load_button.png",
        {0.2f, 0.2f},
        {0.65f, 0.2f},
        true,
        "saves_load_button",
        MENU_SAVES,
        MENU_NULL
    });

    menuList[MENU_SAVES].addMenuObject({
        "images/menu/start_button.png",
        {0.2f, 0.2f},
        {0.5f, 0.5f},
        true,
        "saves_start_button",
        MENU_SAVES,
        MENU_GAME
    });

    // -- Help -- //
    menuList[MENU_HELP].initMenu(MENU_HELP);
    menuList[MENU_HELP].addMenuObject({
        "images/menu/help_page.png",
        {1.0f, 1.0f},
        {0.5f, 0.5f},
        false,
        "help_bg",
        MENU_HELP,
        MENU_NULL
    });
    menuList[MENU_HELP].addMenuObject({
        "images/menu/back_button.png",
        {0.2f, 0.2f},
        {0.5f, 0.2f},
        true,
        "help_back_button",
        MENU_HELP,
        MENU_HOME
    });

    // -- Pause -- //
    menuList[MENU_PAUSE].initMenu(MENU_PAUSE);
    menuList[MENU_PAUSE].addMenuObject({
        "images/menu/help_page.png",
        {1.0f, 1.0f},
        {0.5f, 0.5f},
        false,
        "help_bg",
        MENU_PAUSE,
        MENU_NULL
    });
    menuList[MENU_PAUSE].addMenuObject({
        "images/menu/continue_button.png",
        {0.2f, 0.2f},
        {0.50f, 0.2f},
        true,
        "pause_continue_button",
        MENU_PAUSE,
        MENU_GAME
    });
    menuList[MENU_PAUSE].addMenuObject({
        "images/menu/menu_button.png",
        {0.2f, 0.2f},
        {0.8f, 0.2f},
        true,
        "pause_menu_button",
        MENU_PAUSE,
        MENU_HOME
    });
    menuList[MENU_PAUSE].addMenuObject({
        "images/menu/save_button.png",
        {0.2f, 0.2f},
        {0.5f, 0.5f},
        true,
        "pause_save_button",
        MENU_PAUSE,
        MENU_NULL
    });
}

void _menuManager::drawMenuManager() {
    menuList[selectedMenu].drawMenu(windowDimensions);
}

void _menuManager::updateMenuManager(double dt, const Vec2f &mousePos, bool mouseClicked) {
    _menu* menu = &menuList[selectedMenu];
    mouseScreenClipPosition = mousePos;

    menu->updateMenu(dt,mousePos,mouseClicked,sounds);
    if (menu->redirectTo != MENU_NULL) {
        if (menu->redirectTo == MENU_GAME) {
            loadGame = true;
        }
        loadMenu(menu->redirectTo);     // Load menu
        menu->redirectTo = MENU_NULL;   // Reset menu redirection state
    }

    if (menu->generateWorldEvent) {
        menu->generateWorldEvent = false;
        scene->initScene(false);            // Setup scene to generate world
        cout << "Generate World Event!\n";
    }
    
    if (menu->loadWorldEvent) {
        menu->loadWorldEvent = false;
        scene->initScene(true);             // Setup scene to load world
        cout << "Load World Event!\n";
    }

    if (menu->saveGameEvent) {
        menu->saveGameEvent = false;
        scene->saveScene();
        cout << "Save Game Event!\n";
    }
}

void _menuManager::loadMenu(menu_type type) {
    selectedMenu = type;
}

Vec2f _menuManager::getMenuMousePosition() const {
    return mouseScreenClipPosition;
}

menu_type _menuManager::getLoadedMenu() const {
    return selectedMenu;
}


// ============ Menu Object ============ //

// -- PUBLIC -- //

_menuManager::_menuObject::_menuObject() {
}

_menuManager::_menuObject::~_menuObject() {
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

void _menuManager::_menuObject::initMenuObject(const menu_object_config &config) {
    hasMouseState = config.hasMouseState;

    parent = config.parent;
    destination = config.destination;
    
    menuObjectID = config.ID;
    texture->loadTexture(config.fileName);

    size = config.size;
    pos = config.pos;

    glGenBuffers(1,&vboID);
    glGenBuffers(1,&eboID);
    glGenVertexArrays(1, &vaoID);

    shader->initShader("shaders/menu/vertex.vs","shaders/menu/fragment.fs");

    uint32_t program = shader->getProgram();

    u_texture = glGetUniformLocation(program,"u_texture");
    u_projection = glGetUniformLocation(program,"u_projection");
    u_view = glGetUniformLocation(program,"u_view");
    u_model = glGetUniformLocation(program,"u_model");
    u_isHovering = glGetUniformLocation(program,"u_isHovering");

    buildVBO();
    buildEBO();
    buildVAO();
}

void _menuManager::_menuObject::drawMenuObject(const Vec2i &wDim) {
    shader->useProgram();

    texture->bindTexture();

    // Projection Matrix // - Left, Right, Top, Bottom
    glm::mat4 projection = glm::ortho(0.0f, (float)wDim.x, 0.0f, (float)wDim.y);

    // View Matrix // (identity cause no view movement for now)
    glm::mat4 view(1.0f);   // Creates an indentity 
    
    // Model Matrix //
    glm::mat4 model(1.0f);  // Creates an identity matrix
    // Scales the model to be the size of the window. 
    model = glm::scale(model, glm::vec3((float)wDim.x, (float)wDim.y, 1.0f));

    // For texture in fragment shader
    glUniform1i(u_texture,0);

    glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
    GLint hoveringState;
    if (hasMouseState) {
        if (mouseHovering) {
            hoveringState = 1;
        } else {
            hoveringState = 0;
        }
    } else {
        hoveringState = 0;
    }   
    glUniform1i(u_isHovering,hoveringState);

    glBindVertexArray(vaoID);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

void _menuManager::_menuObject::updateMenuObject(double dt, const Vec2f &mousePos) {
    float halfWidth = size.x * 0.5f;
    float halfHeight = size.y * 0.5f;

    prevMouseHovering = mouseHovering;

    if (hasMouseState) {
        // AABB collision check
        if (mousePos.x > pos.x - halfWidth && 
            mousePos.x < pos.x + halfWidth && 
            mousePos.y > pos.y - halfHeight && 
            mousePos.y < pos.y + halfHeight) 
        {
            mouseHovering = true;    
        } else {
            mouseHovering = false;
        }
    }
}

bool _menuManager::_menuObject::getMouseState() const {
    return mouseHovering;
}

bool _menuManager::_menuObject::justEnteredHover() const {
    return mouseHovering && !prevMouseHovering;
}

// -- PRIVATE -- //

void _menuManager::_menuObject::buildVBO() {
    /**
     * primitives * vertices * 
     */
    float vboData[16];
    int vIndex = 0;

    float halfWidth = size.x * 0.5f;
    float halfHeight = size.y * 0.5f;

    // Bottom-Left //
    vboData[vIndex++] = pos.x - halfWidth;
    vboData[vIndex++] = pos.y - halfHeight;
    vboData[vIndex++] = 0.0f;
    vboData[vIndex++] = 1.0f;
    // Bottom-Right //
    vboData[vIndex++] = pos.x + halfWidth;
    vboData[vIndex++] = pos.y - halfHeight;
    vboData[vIndex++] = 1.0f;
    vboData[vIndex++] = 1.0f;
    // Top-Right //
    vboData[vIndex++] = pos.x + halfWidth;
    vboData[vIndex++] = pos.y + halfHeight;
    vboData[vIndex++] = 1.0f;
    vboData[vIndex++] = 0.0f;
    // Top-Left //
    vboData[vIndex++] = pos.x - halfWidth;
    vboData[vIndex++] = pos.y + halfHeight;
    vboData[vIndex++] = 0.0f;
    vboData[vIndex++] = 0.0f;

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vboData), &vboData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void _menuManager::_menuObject::buildEBO() {
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

void _menuManager::_menuObject::buildVAO() {
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

string _menuManager::_menuObject::getID() const {
    return menuObjectID;
}

menu_type _menuManager::_menuObject::getParent() const {
    return parent;
}
    
menu_type _menuManager::_menuObject::getDestination() const {
    return destination;
}


bool _menuManager::_menuObject::operator==(const _menuObject &other) const {
    return menuObjectID == other.menuObjectID;
}

// ============ MENU ============ //


// -- PUBLIC -- //

_menuManager::_menu::_menu() {
    // ctor
}

_menuManager::_menu::~_menu() {
    // dtor
}

void _menuManager::_menu::initMenu(menu_type _type) {
    type = _type;
}

void _menuManager::_menu::addMenuObject(const menu_object_config &config) {
    unique_ptr<_menuObject> newObject = make_unique<_menuObject>();
    newObject->initMenuObject(config);
    menuObjects.push_back(move(newObject));
}

void _menuManager::_menu::drawMenu(const Vec2i &wDim) {
    for (int i = 0; i < menuObjects.size(); i++) {
        menuObjects[i]->drawMenuObject(wDim);
    }
}

void _menuManager::_menu::updateMenu(double dt, const Vec2f &mousePos, bool mouseClicked, _sounds* sounds) {
    timeSinceRedirect += dt;
    for (int i = 0; i < menuObjects.size(); i++) {
        _menuObject* menuObject = menuObjects[i].get();
        menuObject->updateMenuObject(dt, mousePos);
        if (menuObject->justEnteredHover()) {
            if (sounds) sounds->playSfx("MENU_HOVER");
        }
        if (menuObject->getMouseState() && mouseClicked && timeSinceRedirect > 0.5) {
            cout << "Mouse clicked on ID: " << menuObject->getID() << "\n";
            if (menuObject->getID() == "saves_generate_button") {
                generateWorldEvent = true;
            } else if (menuObject->getID() == "saves_load_button") {
                loadWorldEvent = true;
            } else if (menuObject->getID() == "pause_save_button") {
                saveGameEvent = true;
            }
            if (sounds) sounds->playSfx("MENU_CLICK");
            redirectTo = menuObject->getDestination();
            timeSinceRedirect = 0.0;
        }
    }
}

