#ifndef _LIGHT_MANAGER_H
#define _LIGHT_MANAGER_H

#include <_common.h>

struct light_config {
    string ID;                    /// Unique ID to lookup light by
    Vec2f pos;                    /// Position of the light source (world units)
    float radius;                 /// How far the light shines from source (world units)
    float intensity;              /// How bright the light is (0 - 1 where 0.5 is half-lit) 
    Col3f color;                  /// Color of the light
    
    bool active;                  /// Whether light is shown or not
};

class _lightManager {
    public:
        _lightManager();
        virtual ~_lightManager();

        /**
         * Adds a new light to the manager
         * 
         * @param config Light configuation (see _lightManager.h for documentation)
         * @return True if light was added successfully, false if not (ID not unique or manager full)
         */
        bool addLight(const light_config &config);

        /**
         * Removes a light from the manager
         * 
         * @param ID Unique ID of the light
         * @return True if light was removed successfully, false if not.
         */
        bool removeLight(const string &ID);

        /**
         * Applys all lights in the manager to the provided program
         * 
         * Lights not marked "active" will not be applied
         * 
         * Will not work unless program is added first
         * 
         * @param program Shader program to apply lights to
         */
        void applyLights(GLuint program) const;

        /**
         * Adds a shader program to the lighting system (must be done prior to apply lights)
         * 
         * Overwrites existing program if one already exists
         * 
         * @param program Shader program to add to lighting system
         */
        void addProgram(GLuint program);

        /**
         * Returns a mutable pointer of the lights position (to edit or view)
         * 
         * @param ID Unique ID of the light
         * @return Pointer to the position (nullptr if not found)
         */
        Vec2f* getLightPosition(const string &ID);

        /**
         * Returns a mutable pointer of the lights radius (to edit or view)
         * 
         * @param ID Unique ID of the light
         * @return Pointer to the radius (nullptr if not found)
         */
        float* getLightRadius(const string &ID);

        /**
         * Returns a mutable pointer of the lights intensity (to edit or view)
         * 
         * @param ID Unique ID of the light
         * @return Pointer to the intensity (nullptr if not found)
         */
        float* getLightIntensity(const string &ID);

        /**
         * Returns a mutable pointer of the lights color (to edit or view)
         * 
         * @param ID Unique ID of the light
         * @return Pointer to the color (nullptr if not found)
         */
        Col3f* getLightColor(const string &ID);

        /** Returns number of lights in use */
        int getNumLighsInUse() const;

        /** Returns true if the manager is at full capacity */
        bool hasFullCapacity() const;
    protected:
    private:
        const int maxLights = 16;
        int numLightsInUse = 0;
        vector<Vec2f> lightPositions;
        vector<float> lightRadiuses;
        vector<float> lightIntensities;
        vector<Col3f> lightColors;

        unordered_map<string,int> lightIDMap; // Maps light IDs to an index for constant time lookup  

        struct programs {
            GLint u_lightCount = -1;
            GLint u_lightPos = -1;
            GLint u_lightRadius = -1;
            GLint u_lightIntensity = -1;
            GLint u_lightColor = -1;
        };

        unordered_map<GLuint, programs> programMap;
};


#endif // _LIGHT_MANAGER_H