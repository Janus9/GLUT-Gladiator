#include <_unit.h>

// Static -- shared accross all instances of sprite
int _unit::nextID = 0;

_unit::_unit() {
    // ctor
    unitID = nextID;
    nextID++;
}

_unit::~_unit() {
    // dtor
    unordered_map<string, _sprite*>::iterator it;

    // The map and list hold REFERENCES not the actual data objects. if we clear them w/o deleting all the elements first then
    // we have a mem leak.
    for (it = spriteMap.begin(); it != spriteMap.end(); it++)
    {
        delete it->second;
        it->second = nullptr;
    }

    spriteMap.clear();
    spriteList.clear();
}

void _unit::setupSprite(const string &spriteName) {
    // Look for element in the hashmap first (have to for adjusting Vector) -- Look in map first for O(1) lookup. Not found means we dont run O(n) vector check
    unordered_map<string, _sprite*>::iterator it;
    it = spriteMap.find(spriteName);
    if (it != spriteMap.end()) {
        // Found -- need to find inside vector
        bool found = false;
        for (int i = 0; i < spriteList.size(); i++) {
            if (spriteList[i] == it->second) {
                // The list holds a reference, we must delete before removing the reference from the vector or we get a leak
                delete spriteList[i];
                spriteList[i] = nullptr;
                spriteList.erase(spriteList.begin() + i);
                found = true;
                break;
            }
        }
        if (!found) {
            cerr << "WARNING: Element existed in spriteMap but not spriteList. They are desyncronized\n";
        }
    }
    // Regardless if it was found or not we allow overwrites so we create a new one
    _sprite* newSprite = new _sprite();
    spriteMap[spriteName] = newSprite;  // Hashmap [] overwrites and adds new by default
    spriteList.push_back(newSprite); 
}

void _unit::drawUnit() {
    // Draws all the sprites
    for (auto &sprite : spriteList) {
        if (sprite->hidden) continue; // Skip hidden sprites
        sprite->scale = scale;
        sprite->pos = pos;
        sprite->color = color;
    
        sprite->drawSprite();
    }
    if (DEBUG_display_collision_bounds && collisionBox) {
        collisionBox->drawBound();
    }
}

void _unit::drawUnitSingular() {
    if (singleSprite) {
        singleSprite->scale = scale;
        singleSprite->pos = pos;
        singleSprite->color = color;
    
        singleSprite->drawSprite();
    }
    if (DEBUG_display_collision_bounds && collisionBox) {
        collisionBox->drawBound();
    }
}

void _unit::applyPhysics(double dt) {
    vel += acc * dt;
    pos += vel * dt;
    pos.x = 0.0f; // Player has vec/acc for x. But position is fixed to screen

    // cout << "Acceleration: " << acc.toString() << "\n";
    // cout << "Velocity: " << vel.toString() << "\n";
    // cout << "Position: " << pos.toString() << "\n";
}

_sprite* _unit::getSprite(const string &spriteName) {
    // Look for sprite in hashmap
    unordered_map<string, _sprite*>::iterator it;
    it = spriteMap.find(spriteName);
    if (it != spriteMap.end()) {
        return it->second;
    }
    // Return nullptr when no sprite found
    return nullptr;
}

void _unit::setCollisionBox(const Vec2f &size, const Vec2f &offset) {
    collisionBox = make_unique<_collisionBound>();
    collisionBox->setSize(size);
    collisionBox->setLockedPosition(&pos);
    collisionBox->setPositionOffset(offset);

}

bool _unit::isColliding(const _unit &other) {
    if (collisionBox && other.collisionBox) {
        return collisionBox->isColliding(*other.collisionBox);
    }
}

Vec2f _unit::getPenetration(const _unit &other) {
    if (collisionBox && other.collisionBox) {
        return collisionBox->getPenetration(*other.collisionBox);
    }
}

_collisionBound* _unit::getCollisionBound() const {
    return collisionBox.get();
}

bool _unit::isDead() const {
    return currentHealth <= 0.0f;
}

int _unit::getID() const {
    return unitID;
}

bool _unit::focusOn(const Vec2f &_pos, float speed, float degreeTolerance, _sprite* sprite) {
    _sprite* currentSprite = nullptr;
    if (sprite) {
        currentSprite = sprite;
    } else {
        currentSprite = singleSprite;
    }
    if (!currentSprite) return false; 
    float rotNeeded = GetRotationAngle(pos,_pos);
    if (currentSprite->rot < rotNeeded+5 && currentSprite->rot > rotNeeded-5) return true; 
    // Negative speed defaults to instance look
    if (speed <= 0.0f) {
        currentSprite->rot = rotNeeded;
        return true;
    }
    speed /= 60.0f; // Adjust to degrees/second (breaks if update FPS ever changes)
    if (currentSprite->rot < rotNeeded) {
        currentSprite->rot += speed;
    } else {
        currentSprite->rot -= speed;
    }
    return false;
}

void _unit::setSingleSprite(_sprite* sprite) {
    singleSprite = sprite;
}

_sprite* _unit::getSingleSprite() const {
    return singleSprite;
}

void _unit::clearSingleSprite() {
    singleSprite = nullptr;
}


void _unit::setMaxHealth(float amount) {
    if (amount < 0) {
        maxHealth = 0;
        return;
    }
    maxHealth = amount;
}
        
float _unit::getMaxHealth() const {
    return maxHealth;
}
        
float _unit::getHealth() const {
    return currentHealth;
}

float _unit::getHealthPct() const {
    if (maxHealth <= 0) return 0.0f;
    return currentHealth / maxHealth;
}

void _unit::setHealth(float amount) {
    if (amount < 0) {
        currentHealth = 0;
        return;
    }
    currentHealth = amount;
}

void _unit::impulseDamage(float amount) {
    currentHealth -= amount;
    if (currentHealth < 0) {
        currentHealth = 0;
    }
}
        
void _unit::impulseHealing(float amount) {
    currentHealth += amount;
    if (currentHealth > maxHealth) {
        currentHealth = maxHealth;
    }
}

      
void _unit::resetHealth() {
    currentHealth = maxHealth;
}


bool _unit::operator==(const _unit &other) const {
    return unitID == other.unitID;
}

