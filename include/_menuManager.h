#ifndef _MENU_MANAGER_H
#define _MENU_MANAGER_H

#include <_common.h>
#include <_texture.h>
#include <_shader.h>
#include <_sounds.h>
#include <_scene.h>

// Math library for matrices and vectors etc -- https://github.com/g-truc/glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum menu_type {
    MENU_NULL,
    MENU_GAME, // Load game instead of menu
    MENU_LANDING,
    MENU_HOME,
    MENU_HELP,
    MENU_PAUSE,
    MENU_SAVES,
    MENU_WIN,
    MENU_LOOSE,
    MENU_INFO,
    MENU_COUNT // DO NOT MOVE -- KEEP AT BACK
};

/**
 * @param fileName Filename for image
 * @param size Size in % from 0-1 (Ex/ 0.5 is 50%)
 * @param pos Position of center (bottom-left is (0,0) and top right is (1,1))
 * @param hasMouseState Condition if object reacts to mouse or not (disable for backgrounds)
 * @param ID Unique string ID to lookup object by
 * @param parent Which menu owns this object
 * @param destination Which menu should we redirect to on mouse event? (Leave as MENU_NULL if no redirection)
 */
struct menu_object_config {
    string fileName;
    Vec2f size;
    Vec2f pos;
    bool hasMouseState;

    string ID;
    menu_type parent;
    menu_type destination = MENU_NULL;
};

class _menuManager {
    public:
        _menuManager();
        virtual ~_menuManager();

        /**
         * Sets up:
         *  - Landing menu
         *  - Home menu
         *  - Help menu
         *  - Pause menu
         *
         * @param sounds Shared sound engine used for MENU_HOVER / MENU_CLICK SFX. Non-owning; may be nullptr to disable audio.
         */
        void initMenuManager(_sounds* sounds, _scene* _scene);

        // Draw function -- Loads selected menu (or none if inMenu false)
        void drawMenuManager();

        /**
         * @param dt Deleta Time (in seconds)
         * @param mousePos Position of the mouse in the window
         * @param mouseClicked LMB click condition
         */
        void updateMenuManager(double dt, const Vec2f &mousePos, bool mouseClicked);

        // Loads a given menu
        void loadMenu(menu_type type);

        Vec2f getMenuMousePosition() const;

        menu_type getLoadedMenu() const;

        bool loadGame = false;  // If true main runs the scene initGL once, then resets the state

        // Sets window dimensions for menu drawing
        static void setWindowDimensions(const Vec2i &dim);

        bool closeGameEvent = false;
    protected:
    private:
        /**
         * Menu Object
         * 
         * Holds data for rendering an object inside a menu
         *  - Position
         *  - Size
         *  - Texture
         */
        class _menuObject {
            public:
                _menuObject();
                virtual ~_menuObject();

                // Initializes a menu object from the given config
                void initMenuObject(const menu_object_config &config);

                // Draw function for a given menu
                void drawMenuObject(const Vec2i &wDim);

                // Update Menu Object
                void updateMenuObject(double dt, const Vec2f &mousePos);
                
                // Returns true if mouse is hovering the object
                bool getMouseState() const;

                // Returns true on the single frame the mouse first enters the object's AABB
                bool justEnteredHover() const;

                // Returns the ID of the object
                string getID() const;

                // Get parent of the menu object
                menu_type getParent() const;

                // Get redirection destination of the menu
                menu_type getDestination() const;

                bool operator==(const _menuObject &other) const; 
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

                unique_ptr<_texture> texture = make_unique<_texture>();
                GLuint vboID = 0;
                GLuint eboID = 0;
                GLuint vaoID = 0;

                unique_ptr<_shader> shader = make_unique<_shader>();

                GLint u_texture = -1;
                GLint u_projection = -1;
                GLint u_view = -1;
                GLint u_model = -1;
                GLint u_isHovering = -1;

                string menuObjectID;
                
                menu_type parent;
                menu_type destination;
        };

        /**
         * Menu Class
         * 
         * Handles a menu containing
         *  - Background
         *  - Buttons (with effects + input handling)
         * 
         *  Composed of menu objects.
         */
        class _menu {
            public:
                _menu();
                virtual ~_menu();

                // Initializes a menu
                void initMenu(menu_type _type);

                /**
                 * Adds a new menu object to the menu
                 * 
                 * @param config Configuration for new menu (see struct for implementation details)
                 */
                void addMenuObject(const menu_object_config &config);

                /**
                 * Draws all menuObjects in the menu
                 * 
                 * @param wDim Windowm Dimensions (width/height in pixels).
                 */
                void drawMenu(const Vec2i &wDim);

                // Update menu
                void updateMenu(double dt, const Vec2f &mousePos, bool mouseClicked, _sounds* sounds);
                
                menu_type redirectTo = MENU_NULL;   // If not null will redirect on next update by menuManager

                bool generateWorldEvent = false;
                bool loadWorldEvent = false;
                bool saveGameEvent = false;
                bool endGameEvent = false;
            protected:
            private:
                vector<unique_ptr<_menuObject>> menuObjects;

                menu_type type;

                double timeSinceRedirect = 0.0; // Timer to prevent redirect spamming
        };

        _menu menuList[MENU_COUNT]; // List of menus up to COUNT amount
        menu_type selectedMenu = MENU_LANDING;

        _sounds* sounds = nullptr; // Non-owning; provided by main via initMenuManager
        _scene* scene = nullptr; // Non-owning; provided by main via initMenuManager

        Vec2f mouseScreenClipPosition;

        static Vec2i windowDimensions;
};

#endif // _MENU_MANAGER_H