#ifndef _MENU_MANAGER_H
#define _MENU_MANAGER_H

#include <_common.h>
#include <_texture.h>
#include <_shader.h>

// Math library for matrices and vectors etc -- https://github.com/g-truc/glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum menu_type {
    MENU_LANDING,
    MENU_HOME,
    MENU_HELP,
    MENU_PAUSE,
    MENU_COUNT // DO NOT MOVE -- KEEP AT BACK
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
         */
        void initMenuManager();

        // Draw function -- Loads selected menu (or none if inMenu false)
        void drawMenuManager();

        // Update selected function
        void updateMenuManager(double dt);

        // Loads a given menu
        void loadMenu(menu_type type);

        // Sets window dimensions for menu drawing
        static void setWindowDimensions(const Vec2i &dim);

        bool inMenu = true; // Wether menu is loaded or not
    protected:
    private:
        /**
         * Menu Object
         * 
         * Holds data for rendering an object inside a menu
         *  - Position
         *  - Size
         *  - Texture
         *  
         * Handles events for mouse hover/click etc TODO
         */
        class _menuObject {
            public:
                _menuObject();
                virtual ~_menuObject();

                /**
                 * @param fileName File image to load
                 * @param size Size of the menu object (in % of window. Ex/ 0.5 is 50% width of the window)
                 * @param pos Position of the menu object's center (bottom-left: (0,0) and top-right: (1,1))
                 */
                void initMenuObject(const string &fileName, const Vec2f &_size, const Vec2f &_pos);

                // Draw function for a given menu
                void drawMenuObject(const Vec2i &wDim);

                // Update Menu Object
                void updateMenuObject(double dt);
            protected:
            private:
                Vec2f size;
                Vec2f pos;

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
                GLint u_model = -1;
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
                 * @param fileName File image to load
                 * @param size Size of the menu object (in % of window. Ex/ 0.5 is 50% width of the window)
                 * @param pos Position of the menu object's center (bottom-left: (0,0) and top-right: (1,1))
                 */
                void addMenuObject(const string &fileName, const Vec2f &_size, const Vec2f &_pos);

                /**
                 * Draws all menuObjects in the menu
                 * 
                 * @param wDim Windowm Dimensions (width/height in pixels).
                 */
                void drawMenu(const Vec2i &wDim);

                // Update menu
                void updateMenu(double dt);
            protected:
            private:
                vector<unique_ptr<_menuObject>> menuObjects;

                menu_type type;
        };

        _menu menuList[MENU_COUNT]; // List of menus up to COUNT amount
        menu_type selectedMenu = MENU_LANDING;
        
        static Vec2i windowDimensions;
};

#endif // _MENU_MANAGER_H