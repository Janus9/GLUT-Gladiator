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
        eventCallback = context.callback;
    }
    
    void Manager::init() {
        SDL_LogInfo(LOG_MENU_MANAGER, "Initializing the menu manager");
    
        if (!sounds || !scene) {
            SDL_LogWarn(LOG_MENU_MANAGER, "WARNING: Sounds or Scene is nullptr");
        }
    
        //  -- Landing --  //
        pageList[PAGE_LANDING].init(PAGE_LANDING);
        pageList[PAGE_LANDING].addRenderObject({
            "images/menu/landing_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "landing_bg",
            PAGE_LANDING,
            PAGE_NULL
        });
        pageList[PAGE_LANDING].addRenderObject({
            "images/menu/continue_button.png",
            {0.2f, 0.2f},
            {0.5f, 0.2f},
            true,
            "landing_continue_button",
            PAGE_LANDING,
            PAGE_HOME
        });
    
        // -- Home -- //
        pageList[PAGE_HOME].init(PAGE_HOME);
        pageList[PAGE_HOME].addRenderObject({
            "images/menu/home_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "home_bg",
            PAGE_HOME,
            PAGE_NULL
        });
        pageList[PAGE_HOME].addRenderObject({
            "images/menu/play_button.png",
            {0.2f, 0.2f},
            {0.75f, 0.2f},
            true,
            "home_start_button",
            PAGE_HOME,
            PAGE_SAVES
        });
        pageList[PAGE_HOME].addRenderObject({
            "images/menu/help_button.png",
            {0.2f, 0.2f},
            {0.25f, 0.2f},
            true,
            "home_help_button",
            PAGE_HOME,
            PAGE_HELP
        });
        // pageList[PAGE_HOME].addRenderObject({
        //     "images/menu/landing_button.png",
        //     {0.2f, 0.2f},
        //     {0.85f, 0.2f},
        //     true,
        //     "home_landing_button",
        //     PAGE_HOME,
        //     PAGE_LANDING
        // });
    
        // -- Saves -- //
        pageList[PAGE_SAVES].init(PAGE_SAVES);
        pageList[PAGE_SAVES].addRenderObject({
            "images/menu/landing_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "saves_bg",
            PAGE_SAVES,
            PAGE_NULL
        });
        pageList[PAGE_SAVES].addRenderObject({
            "images/menu/new_game_button.png",
            {0.2f, 0.2f},
            {0.15f, 0.2f},
            true,
            "saves_generate_button",
            PAGE_SAVES,
            PAGE_INFO
        });
        pageList[PAGE_SAVES].addRenderObject({
            "images/menu/load_game_button.png",
            {0.2f, 0.2f},
            {0.85f, 0.2f},
            true,
            "saves_load_button",
            PAGE_SAVES,
            PAGE_INFO
        });
    
        // -- Help -- //
        pageList[PAGE_HELP].init(PAGE_HELP);
        pageList[PAGE_HELP].addRenderObject({
            "images/menu/help_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "help_bg",
            PAGE_HELP,
            PAGE_NULL
        });
        pageList[PAGE_HELP].addRenderObject({
            "images/menu/back_button.png",
            {0.2f, 0.2f},
            {0.5f, 0.2f},
            true,
            "help_back_button",
            PAGE_HELP,
            PAGE_HOME
        });
    
        // -- Pause -- //
        pageList[PAGE_PAUSE].init(PAGE_PAUSE);
        pageList[PAGE_PAUSE].addRenderObject({
            "images/menu/help_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "help_bg",
            PAGE_PAUSE,
            PAGE_NULL
        });
        pageList[PAGE_PAUSE].addRenderObject({
            "images/menu/continue_button.png",
            {0.2f, 0.2f},
            {0.60f, 0.2f},
            true,
            "pause_continue_button",
            PAGE_PAUSE,
            PAGE_GAME
        });
        pageList[PAGE_PAUSE].addRenderObject({
            "images/menu/menu_button.png",
            {0.2f, 0.2f},
            {0.8f, 0.2f},
            true,
            "pause_menu_button",
            PAGE_PAUSE,
            PAGE_HOME
        });
        pageList[PAGE_PAUSE].addRenderObject({
            "images/menu/save_button.png",
            {0.2f, 0.2f},
            {0.6f, 0.5f},
            true,
            "pause_save_button",
            PAGE_PAUSE,
            PAGE_NULL
        });
    
        pageList[PAGE_PAUSE].addRenderObject({
            "images/menu/exit_game_button.png",
            {0.2f, 0.2f},
            {0.8f, 0.5f},
            true,
            "exit_game_button",
            PAGE_PAUSE,
            PAGE_NULL
        });
    
        pageList[PAGE_PAUSE].addRenderObject({
            "images/menu/info_button.png",
            {0.2f, 0.2f},
            {0.5f, 0.8f},
            true,
            "pause_info_button",
            PAGE_PAUSE,
            PAGE_INFO
        });
    
        // -- WIN -- //
        pageList[PAGE_WIN].init(PAGE_WIN);
        pageList[PAGE_WIN].addRenderObject({
            "images/menu/win_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "win_bg",
            PAGE_WIN,
            PAGE_NULL
        });
        pageList[PAGE_WIN].addRenderObject({
            "images/menu/exit_game_button.png",
            {0.2f, 0.2f},
            {0.6f, 0.1f},
            true,
            "exit_game_button",
            PAGE_WIN,
            PAGE_NULL
        });
    
        // -- LOOSE -- //
        pageList[PAGE_LOOSE].init(PAGE_LOOSE);
        pageList[PAGE_LOOSE].addRenderObject({
            "images/menu/loose_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "loose_bg",
            PAGE_LOOSE,
            PAGE_NULL
        });
        pageList[PAGE_LOOSE].addRenderObject({
            "images/menu/loose_game_button.png",
            {0.2f, 0.2f},
            {0.65f, 0.1f},
            true,
            "exit_game_button",
            PAGE_LOOSE,
            PAGE_NULL
        });
    
        // -- INFO -- //
        pageList[PAGE_INFO].init(PAGE_INFO);
        pageList[PAGE_INFO].addRenderObject({
            "images/menu/intro_page.png",
            {1.0f, 1.0f},
            {0.5f, 0.5f},
            false,
            "info_bg",
            PAGE_INFO,
            PAGE_NULL
        });
        pageList[PAGE_INFO].addRenderObject({
            "images/menu/continue_button.png",
            {0.15f, 0.10f},
            {0.5f, 0.09f},
            true,
            "info_continue_button",
            PAGE_INFO,
            PAGE_GAME
        });
    }
    
    void Manager::draw(const Vec2i &dim) {
        pageList[selectedPage].draw(dim);
    }
    
    void Manager::update(double dt, const InputState &inputState) {
        Page &page = pageList[selectedPage];
        mouseScreenClipPosition = inputState.mouseScreenClipPos;
    
        page.update(dt,inputState,sounds);
        
        // Events are not destroyed here, main.cpp must handle them
        if (!page.eventQueue.empty()) {
            Event &event = page.eventQueue.front();
            SDL_LogDebug(LOG_MENU_MANAGER, "Loaded event: %s from the event queue",event.ID.c_str());

            // Handle event callback onto main.cpp
            if (eventCallback) {
                eventCallback(event);
            }

            if (event.redirectTo != PAGE_NULL) {
                const type targetPage = event.redirectTo;
                SDL_LogDebug(LOG_MENU_MANAGER, "Redirecting to page: %i", static_cast<int>(targetPage));
               
                loadPage(targetPage);     // Load page
            }

            page.eventQueue.pop(); // Remove the event
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
    
    
    // ============ Render Object ============ //
    
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
        
        ID = config.ID;
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
        return ID;
    }
    
    type Manager::RenderObject::getParent() const {
        return parent;
    }
        
    type Manager::RenderObject::getDestination() const {
        return destination;
    }
    
    
    bool Manager::RenderObject::operator==(const RenderObject &other) const {
        return ID == other.ID;
    }
    
    // ============ PAGE ============ //
    
    
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
        renderObjects.push_back(move(newObject));
    }
    
    void Manager::Page::draw(const Vec2i &wDim) {
        for (int i = 0; i < renderObjects.size(); i++) {
            renderObjects[i]->draw(wDim);
        }
    }
    
    void Manager::Page::update(double dt, const InputState &inputState, sound::Engine* sounds) {
        timeSinceRedirect += dt;
        for (int i = 0; i < renderObjects.size(); i++) {
            RenderObject* renderObject = renderObjects[i].get();
            renderObject->update(dt, inputState);
            if (renderObject->justEnteredHover()) {
                sounds->playSound("MENU_HOVER");
            }
            if (renderObject->getMouseState() && inputState.LMB && timeSinceRedirect > 0.5) {
                SDL_LogDebug(LOG_MENU_PAGE, "Mouse clicked on ID: %s", renderObject->getID().c_str());
                
                sounds->playSound("MENU_CLICK");
                
                Event event {
                    .ID = renderObject->getID(),
                    .redirectTo = renderObject->getDestination()
                };
                eventQueue.push(event);

                timeSinceRedirect = 0.0;
            }
        }
    }
}

