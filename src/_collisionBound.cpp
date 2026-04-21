#include <_collisionBound.h>

// PRIVATE //

void _collisionBound::setBounds() {
    Vec2f localPos = getPos();

    xmax = localPos.x + size.x/2.0f;
    xmin = localPos.x - size.x/2.0f;
    ymax = localPos.y + size.y/2.0f;
    ymin = localPos.y - size.y/2.0f;
}

// PUBLIC

_collisionBound::_collisionBound() {
    // ctor
}

_collisionBound::~_collisionBound() {
    // dtor
}

void _collisionBound::setLockedPosition(const Vec2f* _pos) {
    if (!_pos) {
        cerr << "ERROR: _pos is nullptr\n";
        return;
    }
    posLocked = _pos;
}

void _collisionBound::freeLockedPosition() {
    posLocked = nullptr;
}

bool _collisionBound::isLocked() const {
    return (posLocked != nullptr); // Nullptr means NOT locked 
}

void _collisionBound::setPosition(const Vec2f &_pos) {
    pos = _pos;
}

void _collisionBound::setPositionOffset(const Vec2f &_posOffset) {
    posOffset = _posOffset;
}

void _collisionBound::setSize(const Vec2f &_size) {
    size = _size;
}

bool _collisionBound::isColliding(_collisionBound &other) {
    setBounds(); // Update bounds 
    other.setBounds(); // Update other box bounds
    if (
        other.xmin < xmax && // Left collision
        other.xmax > xmin && // Right collision
        other.ymin < ymax && // Top collision
        other.ymax > ymin    // Bottom collision
    ) {
        return true;
    } 
    return false;
}

Vec2f _collisionBound::getPenetration(_collisionBound &other) {
    setBounds(); // Update bounds 
    other.setBounds(); // Update other box bounds

    Vec2f localPos = getPos();
    Vec2f otherLocalPos = other.getPos();
    Vec2f penetration = {0.0f, 0.0f};

    float overlapX = std::min(xmax, other.xmax) - std::max(xmin, other.xmin);
    float overlapY = std::min(ymax, other.ymax) - std::max(ymin, other.ymin);

    if (overlapX <= 0.0f || overlapY <= 0.0f)
    {
        return penetration;
    }

    if (localPos.x < otherLocalPos.x)
    {
        penetration.x = overlapX;
    }
    else
    {
        penetration.x = -overlapX;
    }

    if (localPos.y < otherLocalPos.y)
    {
        penetration.y = overlapY;
    }
    else
    {
        penetration.y = -overlapY;
    }

    return penetration;
}

Vec2f _collisionBound::getPos() const {
    Vec2f localPos;
    if (posLocked) {
        localPos = *posLocked;
    } else {
        localPos = pos;
    }
    return localPos + posOffset;
}

void _collisionBound::drawBound() {
    setBounds();
    glPushMatrix();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // Draw the outline of the box in red
        glColor3f(1.0f,0.0f,0.0);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(xmin,ymax); // top-left 
            glVertex2f(xmax,ymax); // top-right
            glVertex2f(xmax,ymin); // bottom-right
            glVertex2f(xmin,ymin); // bottom-left
        glEnd();    

        // Color the center (position)
        Vec2f localPos = getPos();
        glPointSize(3.0f);
        glColor3f(0.0f,0.0f,1.0f);
        glBegin(GL_POINTS);
            glVertex2f(localPos.x,localPos.y); // top-left 
        glEnd();    
        

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    glPopMatrix();
}
