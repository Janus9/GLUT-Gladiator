#include <_lightManager.h>

_lightManager::_lightManager() {
    // Reserve space
    lightPositions.reserve(maxLights);
    lightRadiuses.reserve(maxLights);
    lightIntensities.reserve(maxLights);
    lightColors.reserve(maxLights);
}

_lightManager::~_lightManager() {
    lightPositions.clear();
    lightRadiuses.clear();
    lightIntensities.clear();
    lightColors.clear();

    lightIDMap.clear();

    programMap.clear();
}

bool _lightManager::addLight(const light_config &config) {
    auto it = lightIDMap.find(config.ID);
    if (it != lightIDMap.end()) {
        // Already exists
        return false;
    }
    if (numLightsInUse >= maxLights) {
        // Full capacity
        return false;
    }

    int index = numLightsInUse;
    
    lightIDMap[config.ID] = index;
    lightPositions.push_back(config.pos);
    lightRadiuses.push_back(config.radius);
    lightIntensities.push_back(config.intensity);
    lightColors.push_back(config.color);

    numLightsInUse++;
    return true;
}

bool _lightManager::removeLight(const string &ID) {
    auto it = lightIDMap.find(ID);
    if (it == lightIDMap.end()) {
        // Does not exist
        return false;
    }

    int index = it->second;
    
    lightPositions.erase(lightPositions.begin() + index);
    lightRadiuses.erase(lightRadiuses.begin() + index);
    lightIntensities.erase(lightIntensities.begin() + index);
    lightColors.erase(lightColors.begin() + index);

    lightIDMap.erase(it);

    // Vectors automatically shift down elements from deletion but a hashtable does not, thus we do it manually
    for (auto &pair : lightIDMap) {
        if (pair.second > index) {
            pair.second--;
        }
    }

    numLightsInUse--;
    return true;
}

void _lightManager::applyLights(GLuint program) const {
    auto it = programMap.find(program);
    if (it == programMap.end()) {
        // Program not found -- skip
        return;
    }

    const programs &currentProgram = it->second;

    glUniform1i(currentProgram.u_lightCount,numLightsInUse);
    glUniform2fv(currentProgram.u_lightPos, numLightsInUse, (float*)lightPositions.data());
    glUniform1fv(currentProgram.u_lightRadius, numLightsInUse, lightRadiuses.data());
    glUniform1fv(currentProgram.u_lightIntensity, numLightsInUse, lightIntensities.data());
    glUniform3fv(currentProgram.u_lightColor, numLightsInUse, (float*)lightColors.data());
}

void _lightManager::addProgram(GLuint program) {
    programs newProgram;
    newProgram.u_lightCount = glGetUniformLocation(program,"u_lightCount");
    newProgram.u_lightPos = glGetUniformLocation(program,"u_lightPos");
    newProgram.u_lightRadius = glGetUniformLocation(program,"u_lightRadius");
    newProgram.u_lightIntensity = glGetUniformLocation(program,"u_lightIntensity");
    newProgram.u_lightColor = glGetUniformLocation(program,"u_lightColor");
    
    programMap[program] = newProgram;
}

Vec2f* _lightManager::getLightPosition(const string &ID) {
    auto it = lightIDMap.find(ID);
    if (it == lightIDMap.end()) {
        // Not found
        return nullptr;
    }
    return &lightPositions[it->second];
}

float* _lightManager::getLightRadius(const string &ID) {
    auto it = lightIDMap.find(ID);
    if (it == lightIDMap.end()) {
        // Not found
        return nullptr;
    }
    return &lightRadiuses[it->second];
}

float* _lightManager::getLightIntensity(const string &ID) {
    auto it = lightIDMap.find(ID);
    if (it == lightIDMap.end()) {
        // Not found
        return nullptr;
    }
    return &lightIntensities[it->second];
}

Col3f* _lightManager::getLightColor(const string &ID) {
    auto it = lightIDMap.find(ID);
    if (it == lightIDMap.end()) {
        // Not found
        return nullptr;
    }
    return &lightColors[it->second];
}

int _lightManager::getNumLighsInUse() const {
    return numLightsInUse;
}

bool _lightManager::hasFullCapacity() const {
    return (numLightsInUse >= maxLights);
}