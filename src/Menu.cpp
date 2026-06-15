#include <Menu.h>

namespace menu {
    // ============ Menu Manager ============ //
    
    // -- PUBLIC -- //
    
    Manager::Manager() {
    }
    
    Manager::~Manager() {
    }
    
    void Manager::injectContext(const Context &context) {
        if (!context.validate()) {
            SDL_LogError(LOG_MENU_MANAGER, "ERROR: Unable to validate the context");
        }
        sounds = context.sounds;
        scene = context.scene;
    }
    
    void Manager::init() {
        SDL_LogInfo(LOG_MENU_MANAGER, "Initializing the menu manager");
    
        if (!sounds || !scene) {
            SDL_LogWarn(LOG_MENU_MANAGER, "WARNING: Sounds or Scene is nullptr");
        }
    
        //  -- Landing --  //
        menuList[MENU_LANDING].init(MENU_LANDING);
        menuList[MENU_LANDING].addRenderObject({
            "images/menu/landing_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "landing_bg",
            MENU_LANDING,
            MENU_NULL
        });
        menuList[MENU_LANDING].addRenderObject({
            "images/menu/continue_button.png",
            {0.2f, 0.2f},
            {0.5f, 0.2f},
            true,
            "landing_continue_button",
            MENU_LANDING,
            MENU_HOME
        });
    
        // -- Home -- //
        menuList[MENU_HOME].init(MENU_HOME);
        menuList[MENU_HOME].addRenderObject({
            "images/menu/home_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "home_bg",
            MENU_HOME,
            MENU_NULL
        });
        menuList[MENU_HOME].addRenderObject({
            "images/menu/play_button.png",
            {0.2f, 0.2f},
            {0.75f, 0.2f},
            true,
            "home_start_button",
            MENU_HOME,
            MENU_SAVES
        });
        menuList[MENU_HOME].addRenderObject({
            "images/menu/help_button.png",
            {0.2f, 0.2f},
            {0.25f, 0.2f},
            true,
            "home_help_button",
            MENU_HOME,
            MENU_HELP
        });
        // menuList[MENU_HOME].addRenderObject({
        //     "images/menu/landing_button.png",
        //     {0.2f, 0.2f},
        //     {0.85f, 0.2f},
        //     true,
        //     "home_landing_button",
        //     MENU_HOME,
        //     MENU_LANDING
        // });
    
        // -- Saves -- //
        menuList[MENU_SAVES].init(MENU_SAVES);
        menuList[MENU_SAVES].addRenderObject({
            "images/menu/landing_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "saves_bg",
            MENU_SAVES,
            MENU_NULL
        });
        menuList[MENU_SAVES].addRenderObject({
            "images/menu/new_game_button.png",
            {0.2f, 0.2f},
            {0.15f, 0.2f},
            true,
            "saves_generate_button",
            MENU_SAVES,
            MENU_INFO
        });
        menuList[MENU_SAVES].addRenderObject({
            "images/menu/load_game_button.png",
            {0.2f, 0.2f},
            {0.85f, 0.2f},
            true,
            "saves_load_button",
            MENU_SAVES,
            MENU_GAME
        });
    
        // -- Help -- //
        menuList[MENU_HELP].init(MENU_HELP);
        menuList[MENU_HELP].addRenderObject({
            "images/menu/help_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "help_bg",
            MENU_HELP,
            MENU_NULL
        });
        menuList[MENU_HELP].addRenderObject({
            "images/menu/back_button.png",
            {0.2f, 0.2f},
            {0.5f, 0.2f},
            true,
            "help_back_button",
            MENU_HELP,
            MENU_HOME
        });
    
        // -- Pause -- //
        menuList[MENU_PAUSE].init(MENU_PAUSE);
        menuList[MENU_PAUSE].addRenderObject({
            "images/menu/help_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "help_bg",
            MENU_PAUSE,
            MENU_NULL
        });
        menuList[MENU_PAUSE].addRenderObject({
            "images/menu/continue_button.png",
            {0.2f, 0.2f},
            {0.60f, 0.2f},
            true,
            "pause_continue_button",
            MENU_PAUSE,
            MENU_GAME
        });
        menuList[MENU_PAUSE].addRenderObject({
            "images/menu/menu_button.png",
            {0.2f, 0.2f},
            {0.8f, 0.2f},
            true,
            "pausePage_button",
            MENU_PAUSE,
            MENU_HOME
        });
        menuList[MENU_PAUSE].addRenderObject({
            "images/menu/save_button.png",
            {0.2f, 0.2f},
            {0.6f, 0.5f},
            true,
            "pause_save_button",
            MENU_PAUSE,
            MENU_NULL
        });
    
        menuList[MENU_PAUSE].addRenderObject({
            "images/menu/exit_game_button.png",
            {0.2f, 0.2f},
            {0.8f, 0.5f},
            true,
            "exit_game_button",
            MENU_PAUSE,
            MENU_NULL
        });
    
        menuList[MENU_PAUSE].addRenderObject({
            "images/menu/info_button.png",
            {0.2f, 0.2f},
            {0.5f, 0.8f},
            true,
            "pause_info_button",
            MENU_PAUSE,
            MENU_INFO
        });
    
        // -- WIN -- //
        menuList[MENU_WIN].init(MENU_WIN);
        menuList[MENU_WIN].addRenderObject({
            "images/menu/win_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "win_bg",
            MENU_WIN,
            MENU_NULL
        });
        menuList[MENU_WIN].addRenderObject({
            "images/menu/exit_game_button.png",
            {0.2f, 0.2f},
            {0.6f, 0.1f},
            true,
            "exit_game_button",
            MENU_WIN,
            MENU_NULL
        });
    
        // -- LOOSE -- //
        menuList[MENU_LOOSE].init(MENU_LOOSE);
        menuList[MENU_LOOSE].addRenderObject({
            "images/menu/loose_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "loose_bg",
            MENU_LOOSE,
            MENU_NULL
        });
        menuList[MENU_LOOSE].addRenderObject({
            "images/menu/loose_game_button.png",
            {0.2f, 0.2f},
            {0.65f, 0.1f},
            true,
            "exit_game_button",
            MENU_LOOSE,
            MENU_NULL
        });
    
        // -- INFO -- //
        menuList[MENU_INFO].init(MENU_INFO);
        menuList[MENU_INFO].addRenderObject({
            "images/menu/intro_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "info_bg",
            MENU_INFO,
            MENU_NULL
        });
        menuList[MENU_INFO].addRenderObject({
            "images/menu/continue_button.png",
            {0.15f, 0.10f},
            {0.5f, 0.09f},
            true,
            "info_continue_button",
            MENU_INFO,
            MENU_GAME
        });
    }
    
    void Manager::draw(const Vec2i &dim) {
        menuList[selectedPage].draw(dim);
    }
    
    void Manager::update(double dt, const InputState &inputState) {
        Page* page = &menuList[selectedPage];
        mouseScreenClipPosition = inputState.mouseScreenClipPos;
    
        page->update(dt,inputState,sounds);
    
        // Generate World Event
        if (page->generateWorldEvent) {
            SDL_LogInfo(LOG_MENU_MANAGER, "Generate world event");
            page->generateWorldEvent = false;
            generateWorldEvent = true;
        }
        
        // Load World Event
        if (page->loadWorldEvent) {
            SDL_LogInfo(LOG_MENU_MANAGER, "Load world event");
            page->loadWorldEvent = false;
            loadWorldEvent = true;
        }
    
        // Save Game Event
        if (page->saveGameEvent) {
            SDL_LogInfo(LOG_MENU_MANAGER, "Save world event");
            page->saveGameEvent = false;
            saveWorldEvent = true;
        }
    
        // End Game Event
        if (page->endGameEvent) {
            SDL_LogInfo(LOG_MENU_MANAGER, "End game event");
            page->endGameEvent = false;
            closeGameEvent = true;
        }
    
        // Unload World Event
        if (page->unloadWorldEvent) {
            SDL_LogInfo(LOG_MENU_MANAGER, "Unload world event");
            page->unloadWorldEvent = false;
            unloadWorldEvent = true;
    
        }
    
        if (page->redirectTo != MENU_NULL) {
            const type targetPage = page->redirectTo;
            page->redirectTo = MENU_NULL;   // Reset page redirection state
            SDL_LogDebug(LOG_MENU_MANAGER, "Redirecting to page: %i", static_cast<int>(targetPage));
            if (targetPage == MENU_GAME) {
                if (!scene->isInitialized()) {
                    SDL_LogError(LOG_MENU_MANAGER, "ERROR: Cannot redirect to GAME as scene is not initialized");
                    return; // Scene must be initialized if were trying to load the game
                } 
                loadGameEvent = true;
                if (sounds) sounds->playSfx("GAME_START");
                scene->gameUnPausedEvent = true;
                // if (sounds) sounds->playBackgroundMusic("sounds/gameplay_music.wav", 0.2f);
            }
            loadPage(targetPage);     // Load page
        }
    }
    
    void Manager::loadPage(type type) {
        SDL_LogDebug(LOG_MENU_MANAGER, "Loaded page: %i", static_cast<int>(type));
        selectedPage = type;
    }
    
    Vec2f Manager::getMousePosition() const {
        return mouseScreenClipPosition;
    }
    
    type Manager::getLoadedPage() const {
        return selectedPage;
    }
    
    
    // ============ Menu Object ============ //
    
    // -- PUBLIC -- //
    
    Manager::RenderObject::RenderObject() {

    }
    
    Manager::RenderObject::~RenderObject() {
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
    
    void Manager::RenderObject::init(const RenderObjectConfig &config) {
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
    
    void Manager::RenderObject::draw(const Vec2i &wDim) {
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
    
    void Manager::RenderObject::update(double dt, const InputState &inputState) {
        const Vec2f mousePos = inputState.mouseScreenClipPos;
        
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
    
    bool Manager::RenderObject::getMouseState() const {
        return mouseHovering;
    }
    
    bool Manager::RenderObject::justEnteredHover() const {
        return mouseHovering && !prevMouseHovering;
    }
    
    // -- PRIVATE -- //
    
    void Manager::RenderObject::buildVBO() {
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
    
    void Manager::RenderObject::buildEBO() {
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
    
    void Manager::RenderObject::buildVAO() {
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
    
    std::string Manager::RenderObject::getID() const {
        return menuObjectID;
    }
    
    type Manager::RenderObject::getParent() const {
        return parent;
    }
        
    type Manager::RenderObject::getDestination() const {
        return destination;
    }
    
    
    bool Manager::RenderObject::operator==(const RenderObject &other) const {
        return menuObjectID == other.menuObjectID;
    }
    
    // ============ MENU ============ //
    
    
    // -- PUBLIC -- //
    
    Manager::Page::Page() {
        // ctor
    }
    
    Manager::Page::~Page() {
        // dtor
    }
    
    void Manager::Page::init(menu::type _type) {
        type = _type;
    }
    
    void Manager::Page::addRenderObject(const RenderObjectConfig &config) {
        std::unique_ptr<RenderObject> newObject = std::make_unique<RenderObject>();
        newObject->init(config);
        menuObjects.push_back(move(newObject));
    }
    
    void Manager::Page::draw(const Vec2i &wDim) {
        for (int i = 0; i < menuObjects.size(); i++) {
            menuObjects[i]->draw(wDim);
        }
    }
    
    void Manager::Page::update(double dt, const InputState &inputState, _sounds* sounds) {
        timeSinceRedirect += dt;
        for (int i = 0; i < menuObjects.size(); i++) {
            RenderObject* renderObject = menuObjects[i].get();
            renderObject->update(dt, inputState);
            if (renderObject->justEnteredHover()) {
                if (sounds) sounds->playSfx("MENU_HOVER");
            }
            if (renderObject->getMouseState() && inputState.LMB && timeSinceRedirect > 0.5) {
                SDL_LogDebug(LOG_MENU_MANAGER, "Mouse clicked on ID: %s", renderObject->getID().c_str());
                if (renderObject->getID() == "saves_generate_button") {
                    generateWorldEvent = true;
                } else if (renderObject->getID() == "saves_load_button") {
                    loadWorldEvent = true;
                } else if (renderObject->getID() == "pause_save_button") {
                    saveGameEvent = true;
                } else if (renderObject->getID() == "exit_game_button") {
                    endGameEvent = true;
                }
                if (sounds) sounds->playSfx("MENU_CLICK");
                redirectTo = renderObject->getDestination();
                timeSinceRedirect = 0.0;
            }
        }
    }
}

