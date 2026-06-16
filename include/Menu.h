#ifndef MENU_H
#define MENU_H

#include <_common.h>
#include <_texture.h>
#include <_shader.h>
#include <_sounds.h>
#include <_scene.h>

// Math library for matrices and vectors etc -- https://github.com/g-truc/glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace menu {

    /**
     * Type of page enum
     */
    enum type {
        PAGE_NULL,
        PAGE_GAME,      // Load game instead of menu
        PAGE_LANDING,
        PAGE_HOME,
        PAGE_HELP,
        PAGE_PAUSE,
        PAGE_SAVES,
        PAGE_WIN,
        PAGE_LOOSE,
        PAGE_INFO,
        PAGE_COUNT // DO NOT MOVE -- KEEP AT BACK
    };
    
    /**
     * Configuration struct for a page
     */
    struct RenderObjectConfig {
        std::string fileName;               /// Image path
        Vec2f size;                         /// Size in % from 0-1 (Ex/ 0.5 is 50%)
        Vec2f pos;                          /// Position of center (bottom-left is (0,0) and top right is (1,1))
        bool hasMouseState;                 /// Condition if object reacts to mouse or not (disable for backgrounds)

        std::string ID;                     /// Unique string ID for lookups
        type parent;                        /// Which page owns this object
        type destination = PAGE_NULL;       /// Which page should we redirect to on mouse event? (Leave as PAGE_NULL if no redirection)
    };
    
    /**
     * Context injection
     */
    struct Context {
        _sounds* sounds;
        _scene* scene;
    
        bool validate() const {
            if (!sounds) {
                SDL_LogError(LOG_MENU_MANAGER, "ERROR: Unable to validate the Sound Manager");
                return false;
            }
            if (!scene) {
                SDL_LogError(LOG_MENU_MANAGER, "ERROR: Unable to validate the Scene");
                return false;
            }
        }
    };
    
    /**
     * Manages the all the pages update/draw etc 
     */
    class Manager {
        public:
            Manager();
            virtual ~Manager();
    
            void injectContext(const Context &context);
    
            /**
             * injectContext MUST be ran prior to initialization
             * 
             * Sets up:
             *  - Landing page
             *  - Home page
             *  - Help page
             *  - Pause page
             */
            void init();
    
            // Draw function -- Loads selected page (or none if inMenu false)
            void draw(const Vec2i &dim);
    
            /**
             * @param dt Deleta Time (in seconds)
             * @param inputState Input state container
             */
            void update(double dt, const InputState &inputState);
    
            // Loads a given page
            void loadPage(type type);
    
            Vec2f getMousePosition() const;
    
            type getLoadedPage() const;
    
            bool loadGameEvent = false;  // If true main reloads back into the game
            bool closeGameEvent = false;
    
            bool loadWorldEvent = false;
            bool saveWorldEvent = false;
            bool generateWorldEvent = false;
            bool unloadWorldEvent = false;
        protected:
        private:
            /**
             * Render Object
             * 
             * Holds data for rendering an object inside a page
             *  - Position
             *  - Size
             *  - Texture
             */
            class RenderObject {
                public:
                    RenderObject();
                    virtual ~RenderObject();
    
                    // Initializes a page object from the given config
                    void init(const RenderObjectConfig &config);
    
                    // Draw function for a given page
                    void draw(const Vec2i &wDim);
    
                    // Update Menu Object
                    void update(double dt, const InputState &inputState);
                    
                    // Returns true if mouse is hovering the object
                    bool getMouseState() const;
    
                    // Returns true on the single frame the mouse first enters the object's AABB
                    bool justEnteredHover() const;
    
                    // Returns the ID of the object
                    std::string getID() const;
    
                    // Get parent of the page object
                    type getParent() const;
    
                    // Get redirection destination of the page
                    type getDestination() const;
    
                    bool operator==(const RenderObject &other) const; 
                protected:
                private:
                    Vec2f size;
                    Vec2f pos;
    
                    bool mouseHovering = false;     // Whether mouse is hovering the object
                    bool prevMouseHovering = false; // Previous frame's hovering state (for hover-enter edge)
                    bool hasMouseState = false;     // Whether object reacts to mouse (disabled on backgrounds)
    
                    void buildVBO();
                    void buildEBO();
                    void buildVAO();
    
                    std::unique_ptr<_texture> texture = std::make_unique<_texture>();
                    GLuint vboID = 0;
                    GLuint eboID = 0;
                    GLuint vaoID = 0;
    
                    std::unique_ptr<_shader> shader = std::make_unique<_shader>();
    
                    GLint u_texture = -1;
                    GLint u_projection = -1;
                    GLint u_view = -1;
                    GLint u_model = -1;
                    GLint u_isHovering = -1;
    
                    std::string ID;
                    
                    type parent;
                    type destination;
            };
    
            /**
             * Page Class
             * 
             * Handles a page containing
             *  - Background
             *  - Buttons (with effects + input handling)
             * 
             *  Composed of page objects.
             */
            class Page {
                public:
                    Page();
                    virtual ~Page();
    
                    // Initializes a page
                    void init(type _type);
    
                    /**
                     * Adds a new page object to the page
                     * 
                     * @param config Configuration for new page (see struct for implementation details)
                     */
                    void addRenderObject(const RenderObjectConfig &config);
    
                    /**
                     * Draws all Render Objects in the page
                     * 
                     * @param wDim Windowm Dimensions (width/height in pixels).
                     */
                    void draw(const Vec2i &wDim);
    
                    // Update page
                    void update(double dt, const InputState &inputState, _sounds* sounds);
                    
                    type redirectTo = PAGE_NULL;   // If not null will redirect on next update by Manager
    
                    bool generateWorldEvent = false;
                    bool loadWorldEvent = false;
                    bool saveGameEvent = false;
                    bool endGameEvent = false;
                    bool unloadWorldEvent = false;
    
                protected:
                private:
                    std::vector<std::unique_ptr<RenderObject>> renderObjects;
    
                    type type;
    
                    double timeSinceRedirect = 0.0; // Timer to prevent redirect spamming
            };
    
            Page pageList[PAGE_COUNT]; // List of pages up to COUNT amount
            type selectedPage = PAGE_LANDING;
    
            _sounds* sounds = nullptr;  // Non-owning; 
            _scene* scene = nullptr;    // Non-owning; 
    
            Vec2f mouseScreenClipPosition;
    };
}

#endif // MENU_H