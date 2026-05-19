#include "_scene.h"

_scene::_scene() : rng(random_device{}())
{
    // ctor
}

_scene::~_scene()
{
    // dtor
    delete myLight;
    myLight = nullptr;

    delete myModel;
    myModel = nullptr;

    delete myWorld;
    myWorld = nullptr;

    delete fpsTimer;
    fpsTimer = nullptr;

    delete interactionTimer;
    interactionTimer = nullptr;

    delete testUnit;
    testUnit = nullptr;

    delete hud;
    hud = nullptr;

    delete sh;
    sh = nullptr;
}

void _scene::setSounds(_sounds *sounds)
{
    soundManager = sounds;
}

GLint _scene::initGL()
{
    cout << "Running Scene initGL Initialization ... \n";

    glClearColor(0.0, 0.0, 0.0, 1.0); // Intended to change the background color. 0001 is black
    glClearDepth(1.0);                // Gives depth to the environment by having color both in the front and back. Depth-test value
    glEnable(GL_DEPTH_TEST);          // Will ensure the depth of the z-coordinate is accurate through enabling depth-test
    glDepthFunc(GL_LEQUAL);           // Depth-test is less than or equal to--> true for less or equal

    // its 2D w/ pixels so no need for lighting
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void _scene::initScene(bool loadWorld)
{
    if (sceneInitialized) {
        cout << "WARNING: Scene already initialized, skipping\n";
        return;
    }
    // -- RNG SETUP -- //    
    if (!loadWorld) {
        seed = std::chrono::system_clock::now().time_since_epoch().count(); 
        rng = mt19937(seed);
    }

    // -- CLASS INIT -- //

    cout << "Running Scene Class Initialization ... \n";

    inputTimer.reset();
    fpsTimer->reset();
    interactionTimer->reset();
    fireRateTimer.reset();
    
    myWorld->initWorld(loadWorld,lightManager.get());         // Initialize the world

    // PICKUPS //
    pickupManager->initPickupManager("images/pickups/pickup_sheet.png",6,player.get(),lightManager.get());
    // HP 
    health_pickup.imageIndex = 0;
    health_pickup.size = 8.0f;

    health_pickup.health = 15.0f;
    health_pickup.maxHealth = 0.0f;
    health_pickup.ammo = 0.0f;
    health_pickup.speed = 0.0f;
    health_pickup.fireRate = 0.0f;
    health_pickup.xp = 0.0f;

    // Ammo 
    ammo_pickup.imageIndex = 1;
    ammo_pickup.size = 12.0f;

    ammo_pickup.health = 0.0f;
    ammo_pickup.maxHealth = 0.0f;
    ammo_pickup.ammo = 15.0f;
    ammo_pickup.speed = 0.0f;
    ammo_pickup.fireRate = 0.0f;
    ammo_pickup.xp = 0.0f;

    // Speed 
    speed_pickup.imageIndex = 2;
    speed_pickup.size = 8.0f;

    speed_pickup.health = 0.0f;
    speed_pickup.maxHealth = 0.0f;
    speed_pickup.ammo = 0.0f;
    speed_pickup.speed = 5.0f;
    speed_pickup.fireRate = 0.0f;
    speed_pickup.xp = 0.0f;

    // Max HP 
    max_health_pickup.imageIndex = 3;
    max_health_pickup.size = 12.0f;

    max_health_pickup.health = 0.0f;
    max_health_pickup.maxHealth = 5.0f;
    max_health_pickup.ammo = 0.0f;
    max_health_pickup.speed = 0.0f;
    max_health_pickup.fireRate = 0.0f;
    max_health_pickup.xp = 0.0f;

    // XP 
    xp_pickup.imageIndex = 4;
    xp_pickup.size = 5.5f;

    xp_pickup.health = 0.0f;
    xp_pickup.maxHealth = 0.0f;
    xp_pickup.ammo = 0.0f;
    xp_pickup.speed = 0.0f;
    xp_pickup.fireRate = 0.0f;
    xp_pickup.xp = 5.0f;

    // Fire Rate 
    fire_rate_pickup.imageIndex = 5;
    fire_rate_pickup.size = 12.0f;

    fire_rate_pickup.health = 0.0f;
    fire_rate_pickup.maxHealth = 0.0f;
    fire_rate_pickup.ammo = 0.0f;
    fire_rate_pickup.speed = 0.0f;
    fire_rate_pickup.fireRate = 5.0f;
    fire_rate_pickup.xp = 0.0f;

    // TEXTURE LOADER //
    if (!loadWorld) setupTextures();

    // -- PLAYER -- //
    player->initPlayer(lightManager.get());
    player->hasGun = true;

    // -- FOB -- //
    FOB->initFob(player.get(),lightManager.get(),soundManager);
    // -- HUD -- //

    hud->addHudText("FPS");
    hud->getHudText("FPS")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("PLAYER_POS");
    hud->getHudText("PLAYER_POS")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("CHUNK_POS");
    hud->getHudText("CHUNK_POS")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("MOUSE_SCREEN");
    hud->getHudText("MOUSE_SCREEN")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("MOUSE_WORLD");
    hud->getHudText("MOUSE_WORLD")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("MOUSE_NORMAL");
    hud->getHudText("MOUSE_NORMAL")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("TILE_NAME");
    hud->getHudText("TILE_NAME")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("CELL_HEALTH");
    hud->getHudText("CELL_HEALTH")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("CHUNK_REDRAW");
    hud->getHudText("CHUNK_REDRAW")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("PLAYER_HEALTH");
    hud->getHudText("PLAYER_HEALTH")->setFont(GLUT_BITMAP_HELVETICA_18);

    hud->addHudText("PLAYER_AMMO");
    hud->getHudText("PLAYER_AMMO")->setFont(GLUT_BITMAP_HELVETICA_18);

    hud->addHudText("PLAYER_RESERVE");
    hud->getHudText("PLAYER_RESERVE")->setFont(GLUT_BITMAP_HELVETICA_18);

    hud->addHudSprite("PROGRESS_BAR");
    hud->getHudSprite("PROGRESS_BAR")->getSprite()->initSprite("images/progress_bar.png", 5, 1, sprite_direction::RIGHT);
    hud->getHudSprite("PROGRESS_BAR")->getSprite()->scale = {1.5f, 1.5f};
    hud->getHudSprite("PROGRESS_BAR")->getSprite()->createSpriteAction(sprite_action("DEFAULT", 0, 0, 4));
    hud->getHudSprite("PROGRESS_BAR")->getSprite()->loadSpriteAction("DEFAULT");
    hud->getHudSprite("PROGRESS_BAR")->getSprite()->setIdleFrame(0, 0);
    hud->getHudSprite("PROGRESS_BAR")->visible = false;

    myLight->setLight(GL_LIGHT0); // The light onto the object from the pointer is set to be the instantiated light from before

    debugTimer.reset();

    drawWorldBenchmark.startBenchmark();
    drawEnemiesBenchmark.startBenchmark();

    if(!loadWorld) enemyManager->initEnemyManager(player.get(), myWorld, bulletManager.get(), soundManager, lightManager.get(), textureManager.get(),pickupManager.get());
    enemyManager->bullet_1 = &turret_bullet;
    enemyManager->bullet_2 = &gatling_bullet;

    enemyManager->health_pickup = health_pickup;
    enemyManager->ammo_pickup = ammo_pickup;
    enemyManager->speed_pickup = speed_pickup;
    enemyManager->max_health_pickup = max_health_pickup;
    enemyManager->fire_rate_pickup = fire_rate_pickup;
    enemyManager->xp_pickup = xp_pickup;

    // -- SOUND EFFECTS -- //
    // Register all SFX up front so first-play decoder stalls are avoided. Tune per-SFX volumes here.
    soundManager->registerSfx("PLAYER_SHOOT", "sounds/player_shoot.mp3", 0.1f); // Done
    soundManager->registerSfx("ENEMY_SHOOT", "sounds/22LR Single Isolated WAV.wav", 0.3f);  // Done
    soundManager->registerSfx("BULLET_HIT_WALL", "sounds/bullet_hit_wall.wav", 0.07f);
    soundManager->registerSfx("BULLET_HIT_UNIT", "sounds/test.m4a", 0.8f);        // This is a clown horn because I don't know when this triggers yet and want to be surprised
    soundManager->registerSfx("ENEMY_DEATH", "sounds/enemy_death.wav", 0.1f);     // Done
    soundManager->registerSfx("PLAYER_HURT", "sounds/player_hurt.mp3", 0.3f);     // Done
    soundManager->registerSfx("PLAYER_DEATH", "sounds/player_death.wav", 0.5f);   // Done
    soundManager->registerSfx("MINE_COMPLETE", "sounds/mine_complete.wav", 0.05f); // Done
    soundManager->registerSfx("MINE_TICK", "sounds/mine_tick.wav", 0.15f);         // Done
    soundManager->registerSfx("ORC_ATTACK", "sounds/orc_attack.mp3", 0.1f);
    soundManager->registerSfx("ORC_HURT", "sounds/orc_hurt.mp3", 0.05f);
    soundManager->registerSfx("ORC_DEATH", "sounds/orc_death.mp3", 0.5f);
    soundManager->registerSfx("VAMPIRE_ATTACK", "sounds/Vampire_Attack.wav", 0.3f);
    soundManager->registerSfx("VAMPIRE_HURT", "sounds/Vampire_Hurt.wav", 0.2f);
    soundManager->registerSfx("VAMPIRE_DEATH", "sounds/Vampire_Death.wav", 0.6f);
    soundManager->registerSfx("GATLING_SHOOT","sounds/MinigunLoop.wav", 0.2f);
    soundManager->registerSfx("GATLING_REV","sounds/MinigunSpinLoop.wav", 0.2f);
    soundManager->registerSfx("FOB_AMBIENT","sounds/machine_mx_2_loop.ogg", 0.4f);

    // Background music is started by _menuManager when the user clicks Start (transition to MENU_GAME),
    // not here, so the fade-in lines up with gameplay starting rather than world generation.

    // -- SHADERS -- //
    // sh->initShader("shaders/V.vs","shaders/F.fs");
    // glUseProgram(sh->program);


    // -- BULLETS -- //
    bulletManager->initBulletManager("images/test_bullet.png", myWorld, player.get(), enemyManager.get(), soundManager, lightManager.get());
    
    // Player //
    player_bullet.amount = 1;
    player_bullet.speed = 512.0f;
    player_bullet.width = 15.0f;
    player_bullet.height = 3.0f;
    player_bullet.lifespan = 3.0f;
    player_bullet.angleOffset = 2.0f;
    player_bullet.penetration = 0.0f;
    player_bullet.damage = 15.0f;

    // Default Turret //
    turret_bullet.amount = 1;
    turret_bullet.speed = 650.0f;
    turret_bullet.width = 23.0f;
    turret_bullet.height = 4.0f;
    turret_bullet.lifespan = 3.0f;
    turret_bullet.angleOffset = 3.0f;
    turret_bullet.penetration = 50;
    turret_bullet.damage = 20.0f;

    // Gatling Turret //
    gatling_bullet.amount = 1;
    gatling_bullet.speed = 400.0f;
    gatling_bullet.width = 12.0f;
    gatling_bullet.height = 3.0f;
    gatling_bullet.lifespan = 3.0f;
    gatling_bullet.angleOffset = 7.0f;
    gatling_bullet.penetration = 0;
    gatling_bullet.damage = 7.5f;

    // -- ENEMY CONFIGS -- //
    // Default Turret //
    default_turret_config.type = ENEMY_TURRET;
    default_turret_config.team = _team::ENEMY;
    default_turret_config.maxHP = 75.0f;
    default_turret_config.fireRate = 300.0f;
    default_turret_config.slewRate = 90.0f;
    default_turret_config.detectionRadius = 256.0f;

    // Gatling Turret //
    gatling_turret_config.type = ENEMY_GATLING;
    gatling_turret_config.team = _team::ENEMY;
    gatling_turret_config.maxHP = 400.0f;
    gatling_turret_config.fireRate = 1500.0f;
    gatling_turret_config.slewRate = 55.0f;
    gatling_turret_config.detectionRadius = 350.0f;

    // Orc //
    orc_config.type = ENEMY_ORC;
    orc_config.team = _team::ENEMY;
    orc_config.maxHP = 100.0f;
    orc_config.fireRate = 0.0f; // Not needed
    orc_config.slewRate = 0.0f; // Not needed
    orc_config.detectionRadius = 200.0f;

    // Vampire (boss) //
    vampire_config.type = ENEMY_VAMPIRE;
    vampire_config.team = _team::ENEMY;
    vampire_config.maxHP = 600.0f;
    vampire_config.fireRate = 0.0f; // Not needed
    vampire_config.slewRate = 0.0f; // Not needed
    vampire_config.detectionRadius = 300.0f;

    // Vampire minions //
    vampire_minion1_config.type = ENEMY_VAMPIRE_MINION1;
    vampire_minion1_config.team = _team::ENEMY;
    vampire_minion1_config.maxHP = 80.0f;
    vampire_minion1_config.fireRate = 0.0f;
    vampire_minion1_config.slewRate = 0.0f;
    vampire_minion1_config.detectionRadius = 300.0f;

    vampire_minion2_config.type = ENEMY_VAMPIRE_MINION2;
    vampire_minion2_config.team = _team::ENEMY;
    vampire_minion2_config.maxHP = 80.0f;
    vampire_minion2_config.fireRate = 0.0f;
    vampire_minion2_config.slewRate = 0.0f;
    vampire_minion2_config.detectionRadius = 300.0f;

    if(!loadWorld) {
        player->fireRate = 400.0f;
        player->magCapacity = 30;
        player->magLevel = 30;
        player->reserveCapacity = 200;
        player->reserveLevel = 200;
        player->reloadSpeed = 2.5f;

        player->setHealth(200.0f);
        player->setMaxHealth(200.0f);
        
        /**
         * World is 128 x 128 chunks (32,768 x 32,768 world units)
         * Player spawns in bounds: -15,000 to -12,000 AND 12,000 to 15,000 
         * 
         */
    
        // Find spawn 
        const float numChunks = NUM_CHUNKS;
        // Total chunk area to length/width * num tiles * 16 units per tile / 2 since 0,0 is center
        float bounds = sqrt(numChunks) * 16 * 16 * 0.5;
        uniform_real_distribution<float> player_pos_neg_dist(-1.0f,1.0f);           // Coin flip for positive vs negative side
        uniform_real_distribution<float> player_pos_dist_neg(-15000.0f,-12000.0f);  // Distribution for negative side
        uniform_real_distribution<float> player_pos_dist_pos(12000.0f,15000.0f);    // Distribution for positive side
        bool lookingForSpawn = true;
        while (lookingForSpawn)
        {
            // Assign X //
            if (player_pos_neg_dist(rng) > 0.0f) {
                spawnPos.x = player_pos_dist_pos(rng);
            } else {
                spawnPos.x = player_pos_dist_neg(rng);
            }
    
            // Assign Y //
            if (player_pos_neg_dist(rng) > 0.0f) {
                spawnPos.y = player_pos_dist_pos(rng);
            } else {
                spawnPos.y = player_pos_dist_neg(rng);
            }
    
            _cell *spawnCell = myWorld->getCellAtWorld(spawnPos);
            if (spawnCell && myWorld->isCellWall(spawnCell))
            {
                // Is a wall, retry
                continue;
            }
            player->pos = spawnPos;
            lookingForSpawn = false;
        }
        player->spawnPos = spawnPos;
    }
    FOB->pos = player->spawnPos;
    
    // -- LIGHT CONFIGS -- //
    player_light.ID = "PLAYER_LIGHT";
    player_light.active = true;
    player_light.pos = player->pos; 
    player_light.radius = 400.0f;
    player_light.intensity = 1.0f;
    player_light.color = {1.0f, 1.0f, 0.8f};
    lightManager->addLight(player_light);

    fob_light.ID = "FOB_LIGHT";
    fob_light.active = true;
    fob_light.pos = FOB->pos; 
    fob_light.radius = 400.0f;
    fob_light.intensity = 1.0f;
    fob_light.color = {0.8f, 0.8f, 1.0f};
    lightManager->addLight(fob_light);

    boss_light.ID = "BOSS_LIGHT";
    boss_light.active = true;
    boss_light.pos = {0.0f,0.0f}; 
    boss_light.radius = 600.0f;
    boss_light.intensity = 1.0f;
    boss_light.color = {0.8f, 1.0f, 1.0f};
    lightManager->addLight(boss_light);


    const int number_default_turrets = 500;
    const int number_gatling_turrets = 75;
    const int number_orcs = 600;
    const int number_vampire_minions = 200;  // Spawn naturally in world
    const int number_vampire_boss_minions = 25;  // Spawn near the boss

    // Dont spawn enemies when world is loaded
    if (!loadWorld) {
        // Spawn default turrets //
        uniform_real_distribution<float> turret_pos_dist(-14000, 14000);
        for (int i = 0; i < number_default_turrets; i++)
        {
            bool lookingForTurretSpawn = true;
            while (lookingForTurretSpawn)
            {
                Vec2f spawnTurretPos = {turret_pos_dist(rng), turret_pos_dist(rng)};
                _cell *spawnTurretCell = myWorld->getCellAtWorld(spawnTurretPos);
                if (spawnTurretCell && myWorld->isCellWall(spawnTurretCell))
                {
                    // Is a wall, retry
                    continue;
                }
                enemyManager->addEnemy(spawnTurretPos,default_turret_config);
                lookingForTurretSpawn = false;
            }
        }
    
        // Spawn gatling turrets //
        uniform_real_distribution<float> gatling_pos_dist(-4000, 4000);
        for (int i = 0; i < number_gatling_turrets; i++)
        {
            bool lookingForGatlingSpawn = true;
            while (lookingForGatlingSpawn)
            {
                Vec2f spawnGatlingPos = {gatling_pos_dist(rng), gatling_pos_dist(rng)};
                _cell *spawnGatlingCell = myWorld->getCellAtWorld(spawnGatlingPos);
                if (spawnGatlingCell && myWorld->isCellWall(spawnGatlingCell))
                {
                    // Is a wall, retry
                    continue;
                }
                enemyManager->addEnemy(spawnGatlingPos,gatling_turret_config);
                lookingForGatlingSpawn = false;
            }
        }

        // Spawn Orcs //
        uniform_real_distribution<float> orc_pos_dist(-12000, 12000);
        for (int i = 0; i < number_orcs; i++)
        {
            bool lookingForOrcSpawn = true;
            while (lookingForOrcSpawn)
            {
                Vec2f spawnOrcPos = {orc_pos_dist(rng), orc_pos_dist(rng)};
                _cell *spawnOrcCell = myWorld->getCellAtWorld(spawnOrcPos);
                if (spawnOrcCell && myWorld->isCellWall(spawnOrcCell))
                {
                    // Is a wall, retry
                    continue;
                }
                enemyManager->addEnemy(spawnOrcPos,orc_config);
                lookingForOrcSpawn = false;
            }
        }

        // Spawn Vampire Minions //
        uniform_real_distribution<float> vamp_mini_pos_dist(-6000, 6000);
        uniform_real_distribution<float> vamp_type_dist(0.0f, 1.0f);

        for (int i = 0; i < number_vampire_minions; i++)
        {
            bool lookingForVampMiniSpawn = true;
            while (lookingForVampMiniSpawn)
            {
                Vec2f spawnVampMiniPos = {vamp_mini_pos_dist(rng), vamp_mini_pos_dist(rng)};
                _cell *spawnVampMiniCell = myWorld->getCellAtWorld(spawnVampMiniPos);
                if (spawnVampMiniCell && myWorld->isCellWall(spawnVampMiniCell))
                {
                    // Is a wall, retry
                    continue;
                }

                // Two vampire types, randomly spreads both
                if (vamp_type_dist(rng) > 0.5f) {
                    enemyManager->addEnemy(spawnVampMiniPos,vampire_minion1_config);
                } else {
                    enemyManager->addEnemy(spawnVampMiniPos,vampire_minion2_config);
                }

                lookingForVampMiniSpawn = false;
            }
        }

        // Spawn Boss //
        enemyManager->addEnemy({0.0f,0.0f},vampire_config);
        
        // Spawn Boss Minions //
        uniform_real_distribution<float> vamp_mini_boss_pos_dist(-300, 300);
        for (int i = 0; i < number_vampire_boss_minions; i++) {
            Vec2f spawnVampMiniPos = {vamp_mini_boss_pos_dist(rng), vamp_mini_boss_pos_dist(rng)};
            if (vamp_type_dist(rng) > 0.5f) {
                enemyManager->addEnemy(spawnVampMiniPos,vampire_minion1_config);
            } else {
                enemyManager->addEnemy(spawnVampMiniPos,vampire_minion2_config);
            }
        }

        // Spawn Pickups //
        const int num_hp_pickups = 350;
        const int num_ammo_pickups = 350;
        const int num_speed_pickups = 80;
        const int num_max_hp_pickups = 80;
        const int num_fire_rate_pickups = 80;

        uniform_real_distribution<float> hp_pos_dist(-10000, 10000);
        uniform_real_distribution<float> ammo_pos_dist(-10000, 10000);
        uniform_real_distribution<float> speed_pos_dist(-6500, 6500);
        uniform_real_distribution<float> max_hp_pos_dist(-5000, 5000);
        uniform_real_distribution<float> fire_rate_pos_dist(-2000, 2000);

        // Hp Pickup Distribution //
        for (int i = 0; i < num_hp_pickups; i++)
        {
            bool lookingSpawn = true;
            while (lookingSpawn)
            {
                Vec2f pos = {hp_pos_dist(rng), hp_pos_dist(rng)};
                _cell *cell = myWorld->getCellAtWorld(pos);
                if (cell && myWorld->isCellWall(cell)) continue;
                pickupManager->addPickup(pos,health_pickup);
                lookingSpawn = false;
            }
        }

        // Ammo Pickup Distribution //
        for (int i = 0; i < num_ammo_pickups; i++)
        {
            bool lookingSpawn = true;
            while (lookingSpawn)
            {
                Vec2f pos = {ammo_pos_dist(rng), ammo_pos_dist(rng)};
                _cell *cell = myWorld->getCellAtWorld(pos);
                if (cell && myWorld->isCellWall(cell)) continue;
                pickupManager->addPickup(pos,ammo_pickup);
                lookingSpawn = false;
            }
        }

        // Speed Pickup Distribution //
        for (int i = 0; i < num_speed_pickups; i++)
        {
            bool lookingSpawn = true;
            while (lookingSpawn)
            {
                Vec2f pos = {speed_pos_dist(rng), speed_pos_dist(rng)};
                _cell *cell = myWorld->getCellAtWorld(pos);
                if (cell && myWorld->isCellWall(cell)) continue;
                pickupManager->addPickup(pos,speed_pickup);
                lookingSpawn = false;
            }
        }

        // Max HP Pickup Distribution //
        for (int i = 0; i < num_max_hp_pickups; i++)
        {
            bool lookingSpawn = true;
            while (lookingSpawn)
            {
                Vec2f pos = {max_hp_pos_dist(rng), max_hp_pos_dist(rng)};
                _cell *cell = myWorld->getCellAtWorld(pos);
                if (cell && myWorld->isCellWall(cell)) continue;
                pickupManager->addPickup(pos,max_health_pickup);
                lookingSpawn = false;
            }
        }

        // Fire Rate Pickup Distribution //
        for (int i = 0; i < num_fire_rate_pickups; i++)
        {
            bool lookingSpawn = true;
            while (lookingSpawn)
            {
                Vec2f pos = {fire_rate_pos_dist(rng), fire_rate_pos_dist(rng)};
                _cell *cell = myWorld->getCellAtWorld(pos);
                if (cell && myWorld->isCellWall(cell)) continue;
                pickupManager->addPickup(pos,fire_rate_pickup);
                lookingSpawn = false;
            }
        }
    }    

    // player->setAction(PLAYER_ACTION_IDLE_GUN, PLAYER_FACE_S);

    sceneInitialized = true;
}

bool _scene::saveSceneToFile(const string &fileName) {
    cout << "Exporting game to save file: " << fileName << ".gg_world\n";
    ofstream file(fileName + ".gg_world", ios::binary);     // Output as binary file
    if (!file) {
        cerr << "ERROR: Cannot create output file for: " << fileName << "\n";
        return false;
    }
    // Header Data Write //

    const char header[2] = {'G','G'};   // Header ("GG")
    file.write(header,2);
    /**
     * write() requires a const char* input for the data to write. 
     * We use reinterpret_cast because it tells the compiler to "pretend" its raw bytes and not whatever datatype it is.
     * unlike other casts, this does NOT change the data, only how its interpreted.
     * This cast is safe with primative data types but NOT with others (structs, classes etc) 
     * 
     * We pass with & because we want the memory address
     */
    file.write(reinterpret_cast<const char*>(&seed),sizeof(seed));  // Seed

    auto now = chrono::system_clock::now();
    auto duration = now.time_since_epoch(); 
    const uint64_t seconds = chrono::duration_cast<chrono::seconds>(duration).count();
    file.write(reinterpret_cast<const char*>(&seconds),sizeof(seconds)); // Time Stamp    

    const uint32_t version_id = WORLD_SAVE_VERSION;
    file.write(reinterpret_cast<const char*>(&version_id),sizeof(version_id));  // Version ID
    const float game_id = GAME_VERSION;
    file.write(reinterpret_cast<const char*>(&game_id),sizeof(game_id));  // Game Version
    const int numStartingChunks = NUM_CHUNKS;
    file.write(reinterpret_cast<const char*>(&numStartingChunks),sizeof(numStartingChunks)); // Chunk Count

    // Chunk Data Write //
    
    const char data_header[4] = {'W','R','L','D'};
    file.write(data_header,4); // Chunk Data Header ("WRLD")

    vector<chunk_serial_data> world_data = myWorld->exportSerializeWorld();
    if (!world_data.empty()) {
        cout << "Writing world data:\n"
             << " - Number of chunks: " << world_data.size() << "\n"
             << " - Size of world: " << world_data.size() * sizeof(chunk_serial_data) << " bytes\n";
        // This writes the contents of the entire vector to file
        file.write(reinterpret_cast<const char*>(world_data.data()), world_data.size() * sizeof(chunk_serial_data));

        if (!file) {
            cout << "ERROR: Save failed to write the world data\n";
            return false;
        }
    } else {
        cout << "ERROR: Size of world data is 0\n";
    }

    // Enemy Data Write //

    vector<enemy_serial_data> enemy_data = enemyManager->exportSerializedEnemies();
    if (enemy_data.empty()) {
        cout << "ERROR: Enemy data is empty\n";
        return false;
    }

    cout << "Writing enemy data:\n"
         << " - Number of enemies: " << enemy_data.size() << "\n"
         << " - Size of enemies: " << enemy_data.size() * sizeof(enemy_serial_data) << " bytes\n";

    const char enemy_header[4] = {'E','N','M','Y'};
    file.write(enemy_header,4); // Enemy Data Header ("ENMY")

    const uint32_t number_enemies = static_cast<uint32_t>(enemy_data.size());
    file.write(reinterpret_cast<const char*>(&number_enemies),sizeof(number_enemies)); // Enemy Count

    file.write(reinterpret_cast<const char*>(enemy_data.data()), enemy_data.size() * sizeof(enemy_serial_data)); // Enemy Data
    if (!file) {
        cout << "ERROR: Save failed to write the enemy data\n";
        return false;
    }

    // Player Data Write //

    cout << "Writing player data:\n"
         << " - Size of player: " << sizeof(player_serial_data) << " bytes\n";

    const char player_header[4] = {'P','L','Y','R'};
    file.write(player_header,4); // Player Data Header ("PLYR")

    player_serial_data player_data = player->exportSerializedPlayer();
    file.write(reinterpret_cast<const char*>(&player_data), sizeof(player_data));

    if (!file) {
        cout << "ERROR: Save failed to write the player data\n";
        return false;
    }

    cout << "Finished game saving!\n"
         << "Save Size: " << file.tellp() << " bytes\n";

    return true;
}

bool _scene::loadSceneFromFile(const string &fileName) {
    cout << "Starting game import from: " << fileName + ".gg_world\n";

    ifstream file(fileName + ".gg_world", ios::binary);
    if (!file) {
        cerr << "ERROR: Cannot open file: " << fileName << "\n";
        return false;
    }

    char header[2];
    file.read(header,2);
    if (header[0] != 'G' || header[1] != 'G') {
        cout << "ERROR: Invalid file header\n";
        return false;
    }

    uint32_t save_seed = 0;
    file.read(reinterpret_cast<char*>(&save_seed), sizeof(save_seed));      // Seed
    seed = save_seed;
    rng = mt19937(seed);    // Set rng engine to seed

    uint64_t time_stamp = 0;
    file.read(reinterpret_cast<char*>(&time_stamp), sizeof(time_stamp));    // Time Stamp

    uint32_t version_id = 0;
    file.read(reinterpret_cast<char*>(&version_id), sizeof(version_id));    // Version ID
    if (version_id != WORLD_SAVE_VERSION) {
        cout << "WARNING: World file version of " << version_id << " does not match current version of "
             << WORLD_SAVE_VERSION << " continuing with load but may fail\n";
    }    

    float game_id = 0;
    file.read(reinterpret_cast<char*>(&game_id), sizeof(game_id));    // Version ID
    if (game_id < GAME_VERSION) {
        cout << "WARNING: Game file version of " << game_id << " does not match loaded version of the game "
             << WORLD_SAVE_VERSION << " continuing with load but may fail\n";
    }  

    int32_t chunk_count = 0;
    file.read(reinterpret_cast<char*>(&chunk_count), sizeof(chunk_count));  // Chunk Count
    const int numStartingChunks = NUM_CHUNKS;
    if (chunk_count != numStartingChunks) {
        cout << "ERROR: Chunk Count of save " << fileName << " for " << chunk_count << "does not match count of " << numStartingChunks << "\n";
        return false;
    }

    // Read Chunk Data //
    
    char data_header[4];
    file.read(data_header,4);    // Chunk Data Header
    if (data_header[0] != 'W' || data_header[1] != 'R' || data_header[2] != 'L' || data_header[3] != 'D') {
        cout << "ERROR: Invalid chunk data header\n";
        return false;
    }

    vector<chunk_serial_data> world_data;
    world_data.resize(chunk_count);
    file.read(reinterpret_cast<char*>(world_data.data()), world_data.size() * sizeof(chunk_serial_data));

    if (!file) {
        cout << "ERROR: Unable to read chunk data\n";
        return false;
    }

    if (world_data.empty()) {
        cout << "ERROR: World data read is empty\n";
        return false;
    }

    cout << "Read world data:\n"
         << " - Number of chunks: " << world_data.size() << "\n"
         << " - Size of world: " << world_data.size() * sizeof(chunk_serial_data) << " bytes\n";

    myWorld->importSerializeWorld(world_data);

    // Read Enemy Data //

    char enemy_header[4];
    file.read(enemy_header,4);    // Enemy Data Header
    if (enemy_header[0] != 'E' || enemy_header[1] != 'N' || enemy_header[2] != 'M' || enemy_header[3] != 'Y') {
        cout << "ERROR: Invalid enemy data header\n";
        return false;
    }

    uint32_t enemy_count = 0;
    file.read(reinterpret_cast<char*>(&enemy_count), sizeof(enemy_count));  // Enemy Count

    cout << "Enemie count read: " << enemy_count << "\n";

    if (enemy_count == 0) {
        cout << "WARNING: Enemy count is 0\n";
    }

    // Setup enemy manager before adding enemies
    setupTextures();
    enemyManager->initEnemyManager(player.get(), myWorld, bulletManager.get(), soundManager, lightManager.get(), textureManager.get(), pickupManager.get());

    vector<enemy_serial_data> enemy_data;
    enemy_data.resize(enemy_count);
    file.read(reinterpret_cast<char*>(enemy_data.data()), enemy_data.size() * sizeof(enemy_serial_data));

    enemyManager->importSerializedEnemies(enemy_data);

    if (!file) {
        cout << "ERROR: Unable to read chunk data\n";
        return false;
    }

    if (enemy_data.empty()) {
        cout << "ERROR: Enemy data read is empty\n";
        return false;
    }

    cout << "Read enemy data:\n"
         << " - Number of enemies: " << enemy_data.size() << "\n"
         << " - Size of enemies: " << enemy_data.size() * sizeof(enemy_serial_data) << " bytes\n";

    // Read Player Data //

    char player_header[4];
    file.read(player_header,4);    // Enemy Data Header
    if (player_header[0] != 'P' || player_header[1] != 'L' || player_header[2] != 'Y' || player_header[3] != 'R') {
        cout << "ERROR: Invalid player data header\n";
        return false;
    }

    player_serial_data player_data;
    file.read(reinterpret_cast<char*>(&player_data),sizeof(player_data));

    player->importSerializedPlayer(player_data);

    cout << "Read player data:\n"
         << " - Size of player: " << sizeof(player_data) << " bytes\n";


    cout << "Finished game import from: " << fileName + ".gg_world\n"
         << " - Save Size: " << file.tellg() << " bytes\n";

    return true;
}

void _scene::reSize(GLint width, GLint height)
{
    this->width = width;
    this->height = height;

    // These should be moved to main at some point
    _hud::setHudViewportDimensions(width, height);


    // This needs to be cleaned up eventually //
    float offset = 40.0f;
    float spacing = 20.0f;

    // Set hud positions
    if (hud->getHudText("FPS"))
        hud->getHudText("FPS")->position = {20.0f, height - offset};
    if (hud->getHudText("FPS"))
        offset += spacing;
    if (hud->getHudText("PLAYER_POS"))
        hud->getHudText("PLAYER_POS")->position = {20.0f, height - offset};
    if (hud->getHudText("PLAYER_POS"))
        offset += spacing;
    if (hud->getHudText("CHUNK_POS"))
        hud->getHudText("CHUNK_POS")->position = {20.0f, height - offset};
    if (hud->getHudText("CHUNK_POS"))
        offset += spacing;
    if (hud->getHudText("MOUSE_SCREEN"))
        hud->getHudText("MOUSE_SCREEN")->position = {20.0f, height - offset};
    if (hud->getHudText("MOUSE_SCREEN"))
        offset += spacing;
    if (hud->getHudText("MOUSE_WORLD"))
        hud->getHudText("MOUSE_WORLD")->position = {20.0f, height - offset};
    if (hud->getHudText("MOUSE_WORLD"))
        offset += spacing;
    if (hud->getHudText("MOUSE_NORMAL"))
        hud->getHudText("MOUSE_NORMAL")->position = {20.0f, height - offset};
    if (hud->getHudText("MOUSE_NORMAL"))
        offset += spacing;
    if (hud->getHudText("TILE_NAME"))
        hud->getHudText("TILE_NAME")->position = {20.0f, height - offset};
    if (hud->getHudText("TILE_NAME"))
        offset += spacing;
    if (hud->getHudText("CELL_HEALTH"))
        hud->getHudText("CELL_HEALTH")->position = {20.0f, height - offset};
    if (hud->getHudText("CELL_HEALTH"))
        offset += spacing;
    if (hud->getHudText("CHUNK_REDRAW"))
        hud->getHudText("CHUNK_REDRAW")->position = {20.0f, height - offset};
    if (hud->getHudText("CHUNK_REDRAW"))
        offset += spacing;
    if (hud->getHudText("PLAYER_HEALTH"))
        hud->getHudText("PLAYER_HEALTH")->position = {20.0f, 0 + 40.0f};
    if (hud->getHudText("PLAYER_RESERVE"))
        hud->getHudText("PLAYER_RESERVE")->position = {20.0f, 0 + 80.0f};
    if (hud->getHudText("PLAYER_AMMO"))
        hud->getHudText("PLAYER_AMMO")->position = {20.0f, 0 + 120.0f};
    if (hud->getHudSprite("PROGRESS_BAR"))
        hud->getHudSprite("PROGRESS_BAR")->position = {width / 2.0, 100.0f};

    Logger.LogInfo("Resizing window to width: " + std::to_string(width) + " and height: " + std::to_string(height), LOG_BOTH);
    GLfloat aspectRatio = (GLfloat)width / (GLfloat)height; // Intended to keep track of window resize
    glViewport(0, 0, width, height);                        // Integer values taken in to take in view. Setting Viewport
    glMatrixMode(GL_PROJECTION);                            // Turns the projection into a matrix. Initiate the projection
    glLoadIdentity();                                       // Keep value's axes (matrix * identity matrix = matrix). Initialize the matrix with identity matrix

    applyCamera();

    glMatrixMode(GL_MODELVIEW); // Inputs into matrix depend on the placement of objects in the matrix overview. Initiate model and view matrix

    glLoadIdentity(); // Keep value's axes (matrix * identity matrix = matrix). Initialize the matrix with identity matrix
}

// Scene is running in a loop
void _scene::drawScene()
{
    glLoadIdentity(); // Whichever state the scene is in it will stay there
    applyCamera();    // Apply camera transformations

    drawWorldBenchmark.startBenchmark();
    myWorld->drawWorld(left, right, top, bottom); // Draw the world
    drawWorldBenchmark.clickBenchmark();

    
    drawEnemiesBenchmark.startBenchmark();
    enemyManager->drawEnemies();
    drawEnemiesBenchmark.clickBenchmark();
    
    bulletManager->drawBulletManager();
    
    pickupManager->drawPickups();

    FOB->drawFob();
    
    if (!gameEnded) {
        
        player->drawPlayer();
        hud->drawHud();
    }

    // For FPS measuring
    frameCount++;

    if (fpsTimer->getMilliseconds() > fpsPrintInterval)
    {
        debugPrintFPS();
    }
}

void _scene::updateAudio(double dt)
{
    soundManager->updateFadeIn(dt);
}

// Maps an aim vector (player-relative, +y = up) to one of 8 player face directions
// by binning the angle into 45-degree sectors.
static player_face faceFromAim(const Vec2f &aim)
{
    // GetRotationAngle returns: N=0, NW=90, S=180, E=270 (CCW from north).
    float angle = GetRotationAngle({0.0f, 0.0f}, aim);
    int sector = (int)floorf((angle + 22.5f) / 45.0f) & 7;
    static const player_face faces[8] = {
        PLAYER_FACE_N,
        PLAYER_FACE_NW,
        PLAYER_FACE_W,
        PLAYER_FACE_SW,
        PLAYER_FACE_S,
        PLAYER_FACE_SE,
        PLAYER_FACE_E,
        PLAYER_FACE_NE,
    };
    return faces[sector];
}

// Runs in loop 60 times per second. dt is in ms.
void _scene::updateScene(double dt, bool *keysArray)
{
    // Copy data of keys array into keys
    keysPtr = keysArray;

    soundManager->setListenerPos(player->pos);

    enemyManager->updateEnemies(dt);
    bulletManager->updateBulletManager(dt);
    myWorld->updateWorld(dt);
    player->updatePlayer(dt);
    FOB->updateFob(dt);
    pickupManager->updatePickups(dt);

    if (enemyManager->bossKilledEvent) {
        enemyManager->bossKilledEvent = false;
        gameEnded = true;
        gameWon = true;
        cout << "GAME WON!\n";
    }

    if (player->playerLoseEvent) {
        player->playerLoseEvent = false;
        gameEnded = true;
        gameWon = false;
        cout << "GAME LOST\n";
    }

    if (gameEnded) {
        player_light.intensity -= 0.15 * dt;
        boss_light.intensity -= 0.15 * dt;
        fob_light.intensity = 0.0f;

        *lightManager->getLightIntensity("PLAYER_LIGHT") = player_light.intensity;
        *lightManager->getLightIntensity("BOSS_LIGHT") = boss_light.intensity;

        if (!player->isRealDead) {
            *lightManager->getLightIntensity("FOB_LIGHT") = fob_light.intensity;
        }

        gameEndedTimeElapsed += dt;
    }

    if (gameEnded && !playEndSongEvent) {
        playEndSongEvent = true;
        if (gameWon) {
            soundManager->playBackgroundMusic("sounds/win_music.wav",0.075f);
        } else {
            soundManager->playBackgroundMusic("sounds/loose_music.ogg",0.075f);
        }
    }

    const float playerSpeed = player->movementSpeed;

    if (gameUnPausedEvent) {
        cout << "Game un-paused!\n";
        gameUnPausedEvent = false;

        const float distance = player->pos.distance({0.0f,0.0f});
        if (distance > 8000.0f) {
            player->playerLevelEvent = PLAYER_EVENT_LEVEL_OUTER;
        } else if (distance < 8000.0f && distance > 3000.0f) {
            player->playerLevelEvent = PLAYER_EVENT_LEVEL_MIDDLE;
        } else if (distance < 3000.0f && distance > 400.0f) {
            player->playerLevelEvent = PLAYER_EVENT_LEVEL_CENTER;
        } else {
            player->playerLevelEvent = PLAYER_EVENT_LEVEL_BOSS;
        }
    }

    switch (player->playerLevelEvent) {
        case PLAYER_EVENT_LEVEL_OUTER:
            cout << "Player entered level: OUTER\n";
            soundManager->playBackgroundMusic("sounds/level_outer.wav",0.3f);
            player->playerLevelEvent = PLAYER_EVENT_LEVEL_NONE;
            break;
        case PLAYER_EVENT_LEVEL_MIDDLE:
            cout << "Player entered level: MIDDLE\n";
            soundManager->playBackgroundMusic("sounds/level_middle.wav",0.3f);
            player->playerLevelEvent = PLAYER_EVENT_LEVEL_NONE;
            break;
        case PLAYER_EVENT_LEVEL_CENTER:
            cout << "Player entered level: CENTER\n";
            soundManager->playBackgroundMusic("sounds/level_center.ogg",0.3f);
            player->playerLevelEvent = PLAYER_EVENT_LEVEL_NONE;
            break;
        case PLAYER_EVENT_LEVEL_BOSS:
            cout << "Player entered level: BOSS\n";
            soundManager->playBackgroundMusic("sounds/level_boss.wav",0.15f);
            player->playerLevelEvent = PLAYER_EVENT_LEVEL_NONE;
            break;
    }

    // Check for mouse events
    if (LMB && hoveredCell && hoveredChunk && myWorld->isCellWall(hoveredCell))
    {
        if (interactionTimer->getSeconds() > miningSpeed / 5.0f)
        {
            hud->getHudSprite("PROGRESS_BAR")->getSprite()->iterateFrame();
            myWorld->damageCell(hoveredCell, 25.0f);
            if (!hoveredCell->isAlive())
            {
                cout << "BLOCK HAS BEEN MINED \n";
                hud->getHudSprite("PROGRESS_BAR")->getSprite()->stopAnimation(); // Mining finished, reset progress bar animation
                soundManager->playSfx("MINE_COMPLETE");
            }
            else
            {
                soundManager->playSfx("MINE_TICK");
            }
            interactionTimer->reset();
        }
    }
    else
    {
        interactionTimer->reset();
    }

    // Update mouse
    float mouseScreenYInverted = height - mouseScreenPos.y;
    // Bottom-left is (-1,-1) and top right is (1,1)
    mouseNormalPos = {mouseScreenPos.x / (width * 0.5f) - 1, mouseScreenYInverted / (height * 0.5f) - 1};

    // cout << "Mouse Normal Pos: " << mouseNormalPos.toString() << "\n";

    float worldWidth = right - left;
    float worldHeight = top - bottom;

    // Where the mouse is in world position (for selecting tiles etc)
    mouseWorldPos = {mouseScreenPos.x / width * worldWidth + left, mouseScreenYInverted / height * worldHeight + bottom};

    _cell *_playerCell = myWorld->getCellAtWorld(player->pos);
    _chunk *_playerChunk = myWorld->getChunkAtWorld(player->pos);

    // if (_playerCell != playerCell) {
    //     if (playerCell && playerChunk) {
    //         playerCell->outlined = false;
    //         playerChunk->vboDirty = true;
    //     }

    //     playerCell = _playerCell;
    //     playerChunk = _playerChunk;

    //     playerCell->outlined = true;
    //     playerChunk->vboDirty = true;
    // }

    playerCell = _playerCell;
    playerChunk = _playerChunk;

    /**
     * X X X
     * X P X
     * X X X
     *
     * P = player
     * X = tile (false means tile has no collision while true mean tile has collision)
     *
     */
    bool occupationTable[9] = {false};
    /**
     * Used for movement, we check from occupation table IF movement causes issues
     *
     *   0
     * 1 P 2
     *   3
     *
     */
    bool collisionTable[4] = {false};
    for (int i = 0; i < 9; i++)
    {
        // Establish tiles right around player
        int x = i % 3 - 1;
        int y = i / 3 - 1;
        Vec2f offset(16 * x, -16 * y);
        occupationTable[i] = myWorld->getTileAtWorld(player->pos + offset)->hasCollision;
    }

    if (myWorld->getTileAtWorld(player->pos + Vec2f(0.0f, playerSpeed * dt))->hasCollision)
    {
        // Next potential movement yields use moving into upper tile
        collisionTable[0] = true;
    }
    if (myWorld->getTileAtWorld(player->pos - Vec2f(0.0f, playerSpeed * dt))->hasCollision)
    {
        // Next potential movement yields use moving into lower tile
        collisionTable[3] = true;
    }
    if (myWorld->getTileAtWorld(player->pos - Vec2f(playerSpeed * dt, 0.0f))->hasCollision)
    {
        // Next potential movement yields use moving into left tile
        collisionTable[1] = true;
    }
    if (myWorld->getTileAtWorld(player->pos + Vec2f(playerSpeed * dt, 0.0f))->hasCollision)
    {
        // Next potential movement yields use moving into right tile
        collisionTable[2] = true;
    }

    // cout << "Collision Table: " << collisionTable[0] << ", " << collisionTable[1] << ", " << collisionTable[2] << ", " << collisionTable[3] << "\n";

    bool W = keysPtr['W'];
    bool A = keysPtr['A'];
    bool S = keysPtr['S'];
    bool D = keysPtr['D'];
    bool SPACE = keysPtr[VK_SPACE];

    if (SPACE && player->magLevel > 0 && !player->isReloading())
    {
        player->isShooting = true;
    }
    else
    {
        player->isShooting = false;
    }

    player->isMoving = false;

    if (!cameraFree && !player->isDead())
    {
        // Double input -- diagonol checks //
        if (W && A)
        {
            face = PLAYER_FACE_NW;
            player->isMoving = true;
        }

        if (W && D)
        {
            face = PLAYER_FACE_NE;
            player->isMoving = true;
        }

        if (S && A)
        {
            face = PLAYER_FACE_SW;
            player->isMoving = true;
        }

        if (S && D)
        {
            face = PLAYER_FACE_SE;
            player->isMoving = true;
        }

        // Prevents single inputs when were already doing a double input
        if (!player->isMoving)
        {
            // Sigle input checks //
            if (W && !collisionTable[0])
            {
                face = PLAYER_FACE_N;
                player->isMoving = true;
            }

            if (A && !collisionTable[1])
            {
                face = PLAYER_FACE_W;
                player->isMoving = true;
            }

            if (S && !collisionTable[3])
            {
                face = PLAYER_FACE_S;
                player->isMoving = true;
            }

            if (D && !collisionTable[2])
            {
                face = PLAYER_FACE_E;
                player->isMoving = true;
            }
        }
    }

    // Player walk action //
    if (player->isMoving && !player->isDead())
    {
        // Walking
        if (player->hasGun)
        {
            if (player->isShooting)
            {
                face = faceFromAim(mouseNormalPos);
                action = PLAYER_ACTION_WALK_SHOOT;
            }
            else
            {
                action = PLAYER_ACTION_WALK_GUN;
            }
        }
        else
        {
            action = PLAYER_ACTION_WALK;
        }
    }
    else if (!player->isDead())
    {
        // Idle
        if (player->hasGun)
        {
            if (player->isShooting)
            {
                face = faceFromAim(mouseNormalPos);
                action = PLAYER_ACTION_IDLE_SHOOT;
            }
            else
            {
                action = PLAYER_ACTION_IDLE_GUN;
            }
        }
        else
        {
            action = PLAYER_ACTION_IDLE;
        }
    }

    // Player Shoot Action //
    if (player->playerShootEvent && !player->isDead())
    {
        player->playerShootEvent = false;
        player->setAnimationFPS(player->fireRate / 60);
        Vec2f offsetPos;
        if (face == PLAYER_FACE_E)
        {
            offsetPos = {4.0f, 8.0f};
        }
        else
        {
            offsetPos = {-4.0f, 8.0f};
        }
        bulletManager->spawnBulletEffect(player->pos + offsetPos, mouseWorldPos, _team::FRIENDLY, player_bullet);
        soundManager->playSfx("PLAYER_SHOOT");
    } else {
        player->setAnimationFPS(12);
    }
        
    // Player Dead Action //
    if (player->isDead())
    {
        if (!playerDeathSfxFired)
        {
            soundManager->playSfx("PLAYER_DEATH");
            playerDeathSfxFired = true;
        }
        player->handlePlayerDeath(face);
    }
    else
    {
        playerDeathSfxFired = false; // Reset so a future respawn retriggers the SFX on next death
    }

    if (player->isReloading()) {
        player->setAnimationFPS(8.0f / player->reloadSpeed);
        
        if (player->isMoving) {
            // Walk reload
            action = PLAYER_ACTION_WALK_RELOAD;
        } else {
            // Idle reload
            action = PLAYER_ACTION_IDLE_RELOAD;
        }
    } else {
        player->setAnimationFPS(12);
    }

    player->setAction(action, face);

    if (cameraFree)
    {
        if (W)
            cameraY += cameraSpeed * dt;
        if (A)
            cameraX -= cameraSpeed * dt;
        if (S)
            cameraY -= cameraSpeed * dt;
        if (D)
            cameraX += cameraSpeed * dt;
    }
    else if (!player->isDead() && !player->isRealDead && !gameEnded)
    {
        if (W && !collisionTable[0])
            player->pos.y += playerSpeed * dt;
        if (A && !collisionTable[1])
            player->pos.x -= playerSpeed * dt;
        if (S && !collisionTable[3])
            player->pos.y -= playerSpeed * dt;
        if (D && !collisionTable[2])
            player->pos.x += playerSpeed * dt;
    }

    // Set it to camera pos so free-cam has light for testing
    
    if (!player->isRealDead) {
        lightManager->getLightPosition("PLAYER_LIGHT")->x = cameraX;
        lightManager->getLightPosition("PLAYER_LIGHT")->y = cameraY;
    } else {
        player_light.intensity = 0.0;   // Disable player light
    }

    if (debugTimer.getMilliseconds() > debugPrintInterval)
    {
        debugPrint();
        debugTimer.reset();
    }

    playerChunkPos = myWorld->worldToChunkPos(player->pos);

    hud->getHudText("FPS")->setText("FPS: " + to_string(sceneFPS));
    hud->getHudText("PLAYER_POS")->setText("Position: " + player->pos.toString());
    hud->getHudText("CHUNK_POS")->setText("Chunk Position: " + playerChunkPos.toString());
    hud->getHudText("MOUSE_SCREEN")->setText("Mouse Screen Coords: " + mouseScreenPos.toString());
    hud->getHudText("MOUSE_WORLD")->setText("Mouse World Coords: " + mouseWorldPos.toString());
    hud->getHudText("MOUSE_NORMAL")->setText("Mouse Normal Coords: " + mouseNormalPos.toString());
    hud->getHudText("PLAYER_HEALTH")->setText("HP: " + to_string(player->getHealth()));
    hud->getHudText("PLAYER_AMMO")->setText("Ammo: " + to_string(player->magLevel) + " / " + to_string(player->magCapacity));
    hud->getHudText("PLAYER_RESERVE")->setText("Reserve: " + to_string(player->reserveLevel) + " / " + to_string(player->reserveCapacity));
    // const _tile* tile = myWorld->getTileAtWorld(player->pos);

    const _tile *tile = myWorld->getTileAtWorld(Vec2f(mouseWorldPos.x, mouseWorldPos.y));
    _cell *cell = myWorld->getCellAtWorld(Vec2f(mouseWorldPos.x, mouseWorldPos.y));
    _chunk *chunk = myWorld->getChunkAtWorld(Vec2f(mouseWorldPos.x, mouseWorldPos.y));

    if (cell && cell != hoveredCell)
    {
        hoveredCell->setOutline(false);

        hoveredCell = cell;
        hoveredChunk = chunk;

        hoveredCell->setOutline(true);
    }
    if (tile)
    {
        hud->getHudText("TILE_NAME")->setText("Selected Tile Name: " + tile->name);
    }
    if (hoveredCell)
    {
        hud->getHudText("CELL_HEALTH")->setText("Selected Cell Health: " + to_string(hoveredCell->getHealth()));
    }
    if (hoveredChunk)
    {
        string text_main = "Chunk Redraw: ";
        string test_con = hoveredChunk->vboDirty ? "TRUE" : "FALSE";
        hud->getHudText("CHUNK_REDRAW")->setText(text_main + test_con);
    }
}

void _scene::debugPrint()
{
    Logger.LogDebug("World drawing took: " + to_string(drawWorldBenchmark.getAverageResult()) + "ms");
    Logger.LogDebug("Enemy drawing took: " + to_string(drawEnemiesBenchmark.getAverageResult()) + "ms");
    cout << "-- LEFT: " << left << "\n"
         << "-- RIGHT: " << right << "\n"
         << "-- TOP: " << top << "\n"
         << "-- BOTTOM: " << bottom << "\n";
    myWorld->debugPrint();
}

void _scene::debugPrintFPS()
{
    sceneFPS = frameCount / (fpsTimer->getMilliseconds() / 1000.0); // Calculate FPS based on frames and time
    // Logger.LogInfo("Current FPS: " + std::to_string(sceneFPS), LOG_CONSOLE);
    frameCount = 0;    // Reset frame count after printing FPS
    fpsTimer->reset(); // Reset the timer for the next FPS calculation
}

void _scene::keyboardHandler(WPARAM wParam)
{
    if (inputTimer.getMilliseconds() > 200) // 200 ms debounce time for toggle keys
    {
        switch (wParam)
        {
        case 192: // "~"
            break;
        case 49: // "1"
            enemyManager->addEnemy(mouseWorldPos, default_turret_config);
            break;
        case 50: // "2"
            enemyManager->addEnemy(mouseWorldPos, gatling_turret_config);
            break;
        case 51: // "3"
            enemyManager->addEnemy(mouseWorldPos, orc_config);
            break;
        case 52: // "4"
            enemyManager->addEnemy(mouseWorldPos, vampire_config);
            break;
        case 53: // "5"
            enemyManager->addEnemy(mouseWorldPos, vampire_minion1_config);
            break;
        case 54: // "6"
            enemyManager->addEnemy(mouseWorldPos, vampire_minion2_config);
            break;
        case 55: // "7"
            pickupManager->addPickup(mouseWorldPos, speed_pickup);
            break;
        case 56: // "8"
            pickupManager->addPickup(mouseWorldPos, max_health_pickup);
            break;
        case 57: // "9"
            pickupManager->addPickup(mouseWorldPos, fire_rate_pickup);
            break;
        case 58: // "0"
            pickupManager->addPickup(mouseWorldPos, xp_pickup);
            break;

        case 97: // "NUM 1"
            break;

        case ' ': // SPACE
            break;
        case 82: // R
            player->procReload();
            break;
        case 221: // "]"
            debugEnabled = !debugEnabled;
            Logger.LogInfo("Toggled debug mode: " + std::string(debugEnabled ? "ON" : "OFF"), LOG_CONSOLE);
            break;
        case 219: // "["
            inputDebugEnabled = !inputDebugEnabled;
            Logger.LogInfo("Toggled input debug mode: " + std::string(inputDebugEnabled ? "ON" : "OFF"), LOG_CONSOLE);
            break;
        case 220: // "\"
            cameraFree = !cameraFree;
            Logger.LogInfo("Toggled camera free mode: " + std::string(cameraFree ? "ON" : "OFF"), LOG_CONSOLE);
            break;
        case 122: // "F11"
            myWorld->DEBUG_displayChunkBorders = !myWorld->DEBUG_displayChunkBorders;
            Logger.LogInfo("Toggled chunk border display: " + std::string(myWorld->DEBUG_displayChunkBorders ? "ON" : "OFF"), LOG_CONSOLE);
            break;
        }
        inputTimer.reset(); // Reset the timer after handling a toggle key
    }
}

void _scene::mouseMove(LPARAM lParam)
{
    mouseScreenPos = {LOWORD(lParam), HIWORD(lParam)};
    if (inputDebugEnabled)
        Logger.LogDebug("Mouse Move at " + mouseScreenPos.toString("px"), LOG_CONSOLE); // Log the position of the mouse when it moves
    // Adjusted to be -width to +width and +height to -height plus offset of the camera
}

int _scene::winMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    // Keypress
    case WM_KEYDOWN:
        if (inputDebugEnabled)
            Logger.LogDebug("Key Pressed: " + std::to_string(wParam), LOG_CONSOLE); // Log the key that was pressed
        keyboardHandler(wParam);
        break;
    // Key release
    case WM_KEYUP:
        if (inputDebugEnabled)
            Logger.LogDebug("Key Released: " + std::to_string(wParam), LOG_CONSOLE); // Log the key that was released
        break;
    // Left Mouse button down
    case WM_LBUTTONDOWN:
    {
        if (inputDebugEnabled)
            Logger.LogDebug("Left Mouse Button Down at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); // Log the position of the mouse when left button is pressed
        LMB = true;
        interactionTimer->reset();
        break;
    }
    // Right Mouse button down
    case WM_RBUTTONDOWN:
        if (inputDebugEnabled)
            Logger.LogDebug("Right Mouse Button Down at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); // Log the position of the mouse when right button is pressed
        break;
    // Left Mouse button up
    case WM_LBUTTONUP:
        if (inputDebugEnabled)
            Logger.LogDebug("Left Mouse Button Up at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); // Log the position of the mouse when left button is released
        LMB = false;
        break;
    // Right Mouse button up
    case WM_RBUTTONUP:
        if (inputDebugEnabled)
            Logger.LogDebug("Right Mouse Button Up at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); // Log the position of the mouse when right button is released
        break;
    // Mouse move
    case WM_MOUSEMOVE:
        mouseMove(lParam);
        break;
    // Mouse wheel
    case WM_MOUSEWHEEL:
        if (inputDebugEnabled)
            Logger.LogDebug("Mouse Wheel: " + std::to_string((short)HIWORD(wParam)) + " at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); // Log the amount of scroll and position of the mouse when the wheel is scrolled
        if ((short)HIWORD(wParam) > 0)
        {
            // Scroll up
            if (cameraZoom < 9.0f)
                cameraZoom++; // Zoom in by increasing the zoom factor
        }
        else if ((short)HIWORD(wParam) < 0)
        {
            // Scroll down
            if (cameraZoom > 3.0f)
                cameraZoom--; // Zoom out by decreasing the zoom factor
        }

        // cout << "Camera Level: " << cameraZoom << "\n";
        break;
    }
    return 0;
}

bool _scene::isInitialized() const {
    return sceneInitialized;
}

// -- PRIVATE -- //

void _scene::applyCamera()
{
    if (!cameraFree)
    {
        // If camera is not free, it will track the player (centered on player)
        cameraX = player->pos.x;
        cameraY = player->pos.y;
    }

    float renderCameraX = cameraX;
    float renderCameraY = cameraY;

    float halfWidth = width * 0.5f / cameraZoom;   // Adjust half-width based on zoom level
    float halfHeight = height * 0.5f / cameraZoom; // Adjust half-height based on zoom level

    left = renderCameraX - halfWidth;
    right = renderCameraX + halfWidth;
    bottom = renderCameraY - halfHeight;
    top = renderCameraY + halfHeight;

    // GLM Matrix Building //

    sceneProjectionMatrix = glm::ortho(left,right,bottom,top);  // Mirrors glOrtho
    sceneViewMatix = glm::mat4(1.0f); // Identity (view is baked into projection for now)
    sceneViewProjectionMatrix = sceneProjectionMatrix * sceneViewMatix;

    _bulletManager::setViewProjectionMatrix(sceneViewProjectionMatrix);
    _particleManager::setViewProjectionMatrix(sceneViewProjectionMatrix);
    _pickupManager::setViewProjectionMatrix(sceneViewProjectionMatrix);
    _enemyManager::setViewProjectionMatrix(sceneViewProjectionMatrix);
    _world::setViewProjectionMatrix(sceneViewProjectionMatrix);
    _world::setCameraPosition({cameraX,cameraY});

    // Legacy Matrix Building //

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // This setup does not use a View matrix and it should -- this should be changed later

    // We use Orthographic projection because its a 2D game and it makes it easy to maintain scale via pixels
    /**
     * This
     */
    glOrtho(
        left, right, // left->right = zoom of 1 means 1 world unit (1 pixel) is visible. Zooming in makes the world bigger and zooming out makes the world smaller
        bottom, top, // top->bottom = zoom of 1 means 1 world unit (1 pixel) is visible. Zooming in makes the world bigger and zooming out makes the world smaller
        -1.0, 1.0    // z index from -1 to 1 is visible (everything else clips and is not visible)
    );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void _scene::setupTextures() {
    // Regular Turret //
    textureManager->addTexture("images/enemy/turret.png");
    // Gatling Turrets //
    textureManager->addTexture("images/enemy/gatling_gun/gatling_base.png");
    textureManager->addTexture("images/enemy/gatling_gun/gatling_turret.png");
    // Orc //
    textureManager->addTexture("images/enemy/orc/orc_walk.png");
    textureManager->addTexture("images/enemy/orc/orc_attack.png");
    textureManager->addTexture("images/enemy/orc/orc_hurt.png");
    textureManager->addTexture("images/enemy/orc/orc_death.png");
    // Vampire (boss) //
    textureManager->addTexture("images/enemy/vampire/Vampire_Idle.png");
    textureManager->addTexture("images/enemy/vampire/Vampire_Walk.png");
    textureManager->addTexture("images/enemy/vampire/Vampire_Attack.png");
    textureManager->addTexture("images/enemy/vampire/Vampire_Hurt.png");
    textureManager->addTexture("images/enemy/vampire/Vampire_Death.png");
    // Vampire minions //
    textureManager->addTexture("images/enemy/vampire/minion/Minion1_Idle.png");
    textureManager->addTexture("images/enemy/vampire/minion/Minion1_Walk.png");
    textureManager->addTexture("images/enemy/vampire/minion/Minion1_Attack.png");
    textureManager->addTexture("images/enemy/vampire/minion/Minion1_Hurt.png");
    textureManager->addTexture("images/enemy/vampire/minion/Minion1_Death.png");
    textureManager->addTexture("images/enemy/vampire/minion/Minion2_Idle.png");
    textureManager->addTexture("images/enemy/vampire/minion/Minion2_Walk.png");
    textureManager->addTexture("images/enemy/vampire/minion/Minion2_Attack.png");
    textureManager->addTexture("images/enemy/vampire/minion/Minion2_Hurt.png");
    textureManager->addTexture("images/enemy/vampire/minion/Minion2_Death.png");

    textureManager->addTexture("images/enemy/enemy_particles.png");
}