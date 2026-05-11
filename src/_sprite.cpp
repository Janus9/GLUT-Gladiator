#include <_sprite.h>

// -- STATIC MEMBERS -- //

int _sprite::nextID = 0;

// -- PUBLIC -- //

_sprite::_sprite() {
    // ctor
    animationTimer->reset();
    // Sets a sprite to a specific ID
    spriteID = nextID;
    nextID++;
}

_sprite::~_sprite() {
    
}

void _sprite::initSprite(const string &fileName, int _framesX, int _framesY, const sprite_direction &direction, int _FPS) {
    texture->loadTexture(fileName);
    textureID = texture->textID;

    framesX = _framesX;
    framesY = _framesY;
    
    if (framesX == 0 || framesY == 0) {
        cerr << "ERROR: Sprite frames X/Y cannot be 0\n";
        return;
    }

    pixelsX = texture->width / framesX; 
    pixelsY = texture->height / framesY; 

    // cout << "Initialized a sprite for " << fileName << " Pixel Dim: (" << pixelsX << "px, " << pixelsY << "px)\n";

    // Pixel draw stop is an index so sub 1
    pixelDrawStop = pixelsY-1;
    
    defaultDirection = direction;

    setFPS(_FPS);
}

void _sprite::initSprite(const texture_entry &texture, int _framesX, int _framesY, int _layer, int _FPS) {
    framesX = _framesX;
    framesY = _framesY;

    setLayer(_layer);
    setTexture(texture);

     if (framesX == 0 || framesY == 0) {
        cerr << "ERROR: Sprite frames X/Y cannot be 0\n";
        return;
    }

    setFPS(_FPS);
}

void _sprite::createSpriteAction(const sprite_action &action) {
    spriteActions[action.action_name] = action;
}

void _sprite::removeSpriteAction(const string &actionName) {
    spriteActions.erase(actionName);
}

bool _sprite::loadSpriteAction(const string &actionName) {
    if (currentAction != nullptr && currentAction->action_name == actionName) {
        // Sprite action already loaded, no need to load it again
        return false;
    }
    unordered_map<string, sprite_action>::iterator it;
    it = spriteActions.find(actionName); 
    if (it != spriteActions.end()) {
        // Found
        currentAction = &it->second; // Access second cause 1st is the string

        // Set the currentFrame selection to prevent being behind the start if load is done while sprite is activley animating
        currentFrameX = currentAction->frame_column_start_index;
        currentFrameY = currentAction->frame_row_index;
        return true;
    } 
    // Not Found
    cerr << "ERROR: Cannot load sprite action " << actionName << " as it does not exist\n";
    currentAction = nullptr;
    return false;
}

void _sprite::setDirection(const sprite_direction &direction) {
    if (direction != defaultDirection) {
        // Direction is opposite of sprite sheet, requires inverse
        mirrorImage = true;
    } else {
        mirrorImage = false;
    }
}

void _sprite::drawSprite() {
    if (hidden || ocapacity == 0) return; // Skip draw due to hidden image
    if (pixelsX <= 0 || pixelsY <= 0) {
        // No need to handle drawing an image of 0 size
        return;
    }
    
    // Discard nearly-transparent pixels so they don't write to the depth buffer
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    glPushMatrix();
        texture->bindTexture();
        glColor4f(color.r, color.g, color.b, ocapacity);
        
        glTranslatef(pos.x, pos.y, 0.0f);
        
        // OpenGL is normal 0,0 on bottom-left but SOIL invertes it for us making top-left the 0,0
        
        // https://www.desmos.com/calculator/n7b7d52qam

        // Pixels indexed means that 128th pixel is index 127 (as example)
        // Floats not int for division (int division truncates to 0 or 1 in this case)
        float pixelsXIndexed = pixelsX-1;
        float pixelsYIndexexd = pixelsY-1;

        float frameWidth = 1.0f/framesX;
        float frameHeight = 1.0f/framesY;

        // top-left
        float u0 = currentFrameX * (frameWidth);
        float v0 = currentFrameY * (frameHeight);
        
        // bottom-right
        float u1 = (currentFrameX+1) * (frameWidth);
        float v1 = (currentFrameY+1) * (frameHeight);

        if (mirrorImage) {
            // Flips the image to face the opposite direction (swap the X-component tex points)
            float temp = u0;
            u0 = u1;
            u1 = temp;
        }
        
        // Adjust the top drawing portion to adjust to the cutoff point. (think % a 50% cutoff point means we move 1/2 a distance down to start from 1/2 frame height) 
        v0 += (frameHeight * (pixelDrawStart/pixelsYIndexexd)); 
        // Similar code but the Y value decreases as the pixelDrawStop decreases so we subtract by # of pixels per frame
        v1 += (frameHeight * ((pixelDrawStop-pixelsYIndexexd)/pixelsYIndexexd)); 

        // We dont use glScale here we just tell the vertex to be larger
        float scaledPixelsX = pixelsX * scale.x;

        // For the image cuttoff we have to adjust its placement otherwise it stretches and moves the image itself. We want it to NOT move but be cutoff
        float drawTopHalf = (pixelsY/2.0f-1) - pixelDrawStart;
        float drawBottomHalf = pixelDrawStop - (pixelsY/2.0f-1);

        drawTopHalf *= scale.y;
        drawBottomHalf *= scale.y;
        
        // Adjusts the pivot layout from editor layot (top-left is 0,0) to openGL world coords
        float adjustedPivotX = (pivotPoint.x) - (pixelsX)/2.0f;
        float adjustedPivotY = -((pivotPoint.y) - (pixelsY)/2.0f);

        adjustedPivotX *= scale.x;
        adjustedPivotY *= scale.y;

        // Translate image by offset -- uses world coords not rot
        glTranslatef(negatef(offsetPoint.x,mirrorImage),offsetPoint.y,0.0f);

        // To get a rotation offset we do translate to pivot, rot, then translate back
        // Mirror means negate
        glTranslatef(negatef(adjustedPivotX,mirrorImage),adjustedPivotY,0.0f);
        glRotatef(negatef(rot,mirrorImage),0,0,1); // Negate rot if mirrored
        glTranslatef(-negatef(adjustedPivotX,mirrorImage),-adjustedPivotY,0.0f);

        glBegin(GL_QUADS);
            // bottom-left
            glTexCoord2f(u0, v1); glVertex2f(-scaledPixelsX/2.0f, -drawBottomHalf);
            // bottom-right
            glTexCoord2f(u1, v1); glVertex2f(scaledPixelsX/2.0, -drawBottomHalf);
            // top-right
            glTexCoord2f(u1, v0); glVertex2f(scaledPixelsX/2.0f, drawTopHalf);
            // top-left
            glTexCoord2f(u0, v0); glVertex2f(-scaledPixelsX/2.0f, drawTopHalf);
        glEnd();
       
        if (DEBUG_showPivotPoint || DEBUG_showOrigin) {
            //cout << "Adjusted Pivot Point: (" << adjustedPivotX << ", " << adjustedPivotY << ")\n";
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glPointSize(5.0f);
            
            if (DEBUG_showPivotPoint) {
                glColor3f(1.0f,0.0f,0.0f);
                glBegin(GL_POINTS);
                    glVertex2f(negatef(adjustedPivotX,mirrorImage),adjustedPivotY);
                glEnd();
            }

            if (DEBUG_showOrigin) {
                glColor3f(0.0f,1.0f,0.0f);
                glBegin(GL_POINTS);
                    glVertex2f(0,0);
                glEnd();
            }

            glEnable(GL_LIGHTING);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
        }
    glPopMatrix();
    glDisable(GL_ALPHA_TEST);

    // Check for animation frame change
    if (animationTimer->getSeconds() > fpsDelay && playingAnimation && currentAction != nullptr) {
        animationTimer->reset();
        if(reverseAnimation) {
            currentFrameX--;
            if (currentFrameX < currentAction->frame_column_start_index) {
                currentFrameX = currentAction->frame_column_stop_index;
            }
        } else {
            currentFrameX++;
            if (currentFrameX > currentAction->frame_column_stop_index) {
                currentFrameX = currentAction->frame_column_start_index;
            }
        }
        if (singleActionInProgress && currentFrameX == currentAction->frame_column_start_index) {
            // If single action and were at the beginning (after++) means we already looped. 
            stopAnimation();
        } 
        if (debugging) {
            cout << "DRAW for sprite ID: " << spriteID << "\n";
            cout << "-------------------------------\n";
            cout << "Frame Pos: (" << currentFrameX << ", " << currentFrameY << ")\n"; 
            cout << "Pivot Point: (" << pivotPoint.x << "px, " << pivotPoint.y << "px)\n";
        }
    }
}

void _sprite::updateSprite(double dt) {
    // Check for animation frame change
    if (animationTimer->getSeconds() > fpsDelay && playingAnimation && currentAction != nullptr) {
        animationTimer->reset();
        if(reverseAnimation) {
            currentFrameX--;
            if (currentFrameX < currentAction->frame_column_start_index) {
                currentFrameX = currentAction->frame_column_stop_index;
            }
        } else {
            currentFrameX++;
            if (currentFrameX > currentAction->frame_column_stop_index) {
                currentFrameX = currentAction->frame_column_start_index;
            }
        }
        if (singleActionInProgress && currentFrameX == currentAction->frame_column_start_index) {
            // If single action and were at the beginning (after++) means we already looped. 
            stopAnimation();
        } 
        if (debugging) {
            cout << "DRAW for sprite ID: " << spriteID << "\n";
            cout << "-------------------------------\n";
            cout << "Frame Pos: (" << currentFrameX << ", " << currentFrameY << ")\n"; 
            cout << "Pivot Point: (" << pivotPoint.x << "px, " << pivotPoint.y << "px)\n";
        }
    }
}

bool _sprite::startAnimation() {
    // If were already animating, preform early return to avoid reseting animation over and over
    if (playingAnimation) return false;
    if (currentAction == nullptr) {
        // If FPS is 0 then we prevent divide by 0 errors and block animations by setting the delay to an impossibly high value
        cerr << "ERROR: Cannot activate animation as the currentAction is not selected\n";
        playingAnimation = false;
        return false;
    }
    playingAnimation = true;
    // Start from "start" index of the sprite action
    currentFrameX = currentAction->frame_column_start_index; 
    currentFrameY = currentAction->frame_row_index;
    return true;
}

void _sprite::stopAnimation() {
    playingAnimation = false;
    // Set our current frame to the set idle frame
    currentFrameX = idleFrameX;
    currentFrameY = idleFrameY;
    singleActionInProgress = false;
}

void _sprite::playAction(const string &actionName) {
    currentAction = nullptr; // Unload current action to force loadSpriteAction to start on beginning frame
    stopAnimation();
    loadSpriteAction(actionName);
    startAnimation();
    singleActionInProgress = true;
}

void _sprite::setIdleFrame(int _idleFrameX, int _idleFrameY) {
    if (_idleFrameX < 0 || _idleFrameX >= framesX || _idleFrameY < 0 || _idleFrameY >= framesY) {
        // Frame is out of bounds and going to cause undefined behavior -- guard against it and default to 0,0
        cerr << "ERROR: Idle frame is set out of bounds of tile atlas\n"
             << "- FrameX Set: " << _idleFrameX << "Min: 0 and Max: " << framesX-1 << "\n"
             << "- FrameY Set: " << _idleFrameY << "Min: 0 and Max: " << framesY-1 << "\n";        
        idleFrameX = 0;
        idleFrameY = 0;
        return;
    }
    idleFrameX = _idleFrameX;
    idleFrameY = _idleFrameY;
}

int _sprite::getNumAction() const {
    return spriteActions.size();
}

void _sprite::setFPS(int _FPS) {
    if (_FPS <= 0) {
        FPS = 0;
        fpsDelay = numeric_limits<double>::max();
        cerr << "ERROR: FPS value of " << _FPS << "is less than or equal to 0. Must be positive and non-zero. Animation will not play until resolved.\n";
        return;
    }
    FPS = _FPS;
    fpsDelay = 1.0 / FPS;
}

int _sprite::getFPS() const {
    return FPS;
}

void _sprite::setDrawBoundsExact(int _pixelDrawStart, int _pixelDrawStop) {
    if (_pixelDrawStart >= 0) {
        pixelDrawStart = _pixelDrawStart;
    } else {
        cerr << "WARNING: Pixel amount entered was negative for _pixelDrawStart, setting to 0\n";
        pixelDrawStart = 0;
    }

    if (_pixelDrawStop < _pixelDrawStart) {
        cerr << "WARNING: Pixel amount entered was below _pixelDrawStart for _pixelDrawStop which cannot be drawn. Setting to _pixelDrawStart\n";
        pixelDrawStop = pixelDrawStart;
    } else if (_pixelDrawStop > pixelsY-1) {
        cerr << "WARNING: Pixel amount entered was above max of image for _pixelDrawStop which cannot be drawn. Setting to max of image\n";
        pixelDrawStop = pixelsY-1; 
    } else {
        pixelDrawStop = _pixelDrawStop; 
    }
}

void _sprite::resetDrawBounds() {
    pixelDrawStart = 0;
    pixelDrawStop = pixelsY-1;
}

int _sprite::getID() const {
    return spriteID;
}

bool _sprite::iterateFrame() {
    currentFrameX++;
    if (currentFrameX > currentAction->frame_column_stop_index) {
        currentFrameX = currentAction->frame_column_start_index;
    }
    // True if at it
    return currentFrameX == currentAction->frame_column_start_index;
}

Vec2f _sprite::getSize() const {
    return {pixelsX * scale.x, pixelsY * scale.y};
}

bool _sprite::operator==(const _sprite &other) const {
    return spriteID == other.getID();
}

void _sprite::buildSpriteVBO(float* vboData, int &vIndex) const {
    if (hidden || ocapacity == 0) return; // Skip draw due to hidden image
    if (pixelsX <= 0 || pixelsY <= 0) {
        // No need to handle drawing an image of 0 size
        return;
    }

    float frameWidth = 1.0f/framesX;
    float frameHeight = 1.0f/framesY;

    float halfWidth = pixelsX * scale.x * 0.5f;
    float halfHeight = pixelsY * scale.y * 0.5f;

    // top-left
    float u0 = currentFrameX * (frameWidth);
    float v0 = currentFrameY * (frameHeight);
    
    // bottom-right
    float u1 = (currentFrameX+1) * (frameWidth);
    float v1 = (currentFrameY+1) * (frameHeight);

    float rotRad = degreeToRad(rot); // Shaders require radians

    // Quad //
    // Bottom Left
    vboData[vIndex++] = -halfWidth;         // Size X
    vboData[vIndex++] = -halfHeight;        // Size Y
    vboData[vIndex++] = u0;                 // Tex Coord X
    vboData[vIndex++] = v1;                 // Tex Coord Y
    vboData[vIndex++] = pos.x;              // Pos X
    vboData[vIndex++] = pos.y;              // Pos Y
    vboData[vIndex++] = rotRad;             // Angle (Radians)
    // Bottom Right
    vboData[vIndex++] = halfWidth;         
    vboData[vIndex++] = -halfHeight;        
    vboData[vIndex++] = u1;                 
    vboData[vIndex++] = v1;                
    vboData[vIndex++] = pos.x;              
    vboData[vIndex++] = pos.y;              
    vboData[vIndex++] = rotRad;             
    // Top Right
    vboData[vIndex++] = halfWidth;         
    vboData[vIndex++] = halfHeight;        
    vboData[vIndex++] = u1;                 
    vboData[vIndex++] = v0;                
    vboData[vIndex++] = pos.x;              
    vboData[vIndex++] = pos.y;              
    vboData[vIndex++] = rotRad;   
    // Top Left
    vboData[vIndex++] = -halfWidth;         
    vboData[vIndex++] = halfHeight;        
    vboData[vIndex++] = u0;                 
    vboData[vIndex++] = v0;                
    vboData[vIndex++] = pos.x;              
    vboData[vIndex++] = pos.y;              
    vboData[vIndex++] = rotRad;
}
        
GLuint _sprite::getTextureID() const {
    return textureID;
}

void _sprite::setTexture(const texture_entry &texture) {
    textureID = texture.ID;

    if (framesX == 0 || framesY == 0) {
        cerr << "ERROR: Sprite frames X/Y cannot be 0\n";
    }

    pixelsX = texture.width / framesX; 
    pixelsY = texture.height / framesY; 
}

void _sprite::setLayer(int _layer) {
    if (_layer < 0) {
        _layer = 0;
    }
    layer = _layer;
}

int _sprite::getLayer() const {
    return layer;
}