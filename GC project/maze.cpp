// MazeGame_GLUT.cpp
// Build: g++ MazeGame_GLUT.cpp -o MazeGame_GLUT -lGL -lGLU -lglut -std=c++17

#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <stdint.h>
#include <deque>

using Grid = std::vector<std::string>;

struct Level
{
    Grid grid;
};

class MazeGame
{
public:
    MazeGame(int tileSize = 48)
        : tileSize(tileSize), currentLevel(0), score(0),
          levelTimeLimit(40), moving(false), moveSpeed(6.0f),
          window(0), windowWidth(0), windowHeight(0), gameOver(false),
          lionTexture(0), lionTextureLoaded(false)
    {
        levels = {
            {{"########################################",
              "#S...................#...........B.....#",
              "#.##.#.#####.#######.#.#######.#####..#",
              "#.#..#.....#.....#...#.......#.....#..#",
              "#.#.#####..###.#.#.###.#####.#####.#..#",
              "#.#.....#..#...#.#.....#...#.#.....#..#",
              "#.#####.#..#.###.#######.#.#.#.###.#..#",
              "#.....#.#..#.....#.......#.#.#...#.#..#",
              "###.#.#.########.#.#######.#.###.#.#..#",
              "#...#.#.....#...#.#.......#.#...#.#...#",
              "#.#.#######.#.#.#.#######.#.###.#.###.#",
              "#.#.........#.#.#.........#.....#...E.#",
              "########################################"}},
            {{"########################################",
              "#S.....#..B............................#",
              "#.###..#.####.#######.#####.#.#######..#",
              "#...#..#....#.......#.....#.#.....#....#",
              "###.#.###.###.#####.#####.#.###.#.###..#",
              "#...#.....#...#...#.....#.#.#...#...#..#",
              "#.#####.###.#.#.#.#####.#.#.#.#####.#..#",
              "#.....#.....#.#.#.....#.#.#.#.....#.#..#",
              "#.###.#######.#.#####.#.#.#.#####.#.#..#",
              "#.#...#.....#.#.....#.#.#.......#.#.#..#",
              "#.#.#####.###.#####.#.#.#.#####.#.#.E..#",
              "#.#.......#.........#.#.#.......#.#....#",
              "########################################"}},
            // New Level 3
            {{"########################################",
              "#S...#.............#####..........#....#",
              "#.#.#.#######.###.#...#..######..#..B..#",
              "#.#.#.....#...#.#.#.#.#..#....#..####..#",
              "#.#.#####.#.###.#.#.#.####.##.#.....#..#",
              "#.#.....#.#.....#.#.#......#..#####.#..#",
              "#.#####.#.#######.#.########..#.....#..#",
              "#.....#.#.........#..........###.###.#.#",
              "#####.#.###################......#.#.#.#",
              "#.....#.#.................########.#.#.#",
              "#.#####.#.#################......#.#.#.#",
              "#.....#.#.................#..E...#...#.#",
              "########################################"}},
            // New Level 4
            {{"########################################",
              "#S........#.........#........#.......#.#",
              "#.#######.#.#######.#.#####.#.#####.#.#",
              "#.#.....#.#.....#...#.....#.#.#.....#.#",
              "#.#.###.#.#####.#.#######.#.#.#####.#.#",
              "#.#.#...#.....#.#.......#.#.#.....#.#.#",
              "#.#.#.#######.#.#######.#.#.#####.#.#.#",
              "#.#.#.......#.#.....#...#.#.#.....#.#.#",
              "#.#.#######.#.#####.#.###.#.#.###.#.#.#",
              "#.#.......#.#.....#.#...#.#.#.#...#.#.#",
              "#.#######.#.#####.#.###.#.#.#.#.###.#.#",
              "#.....B...#.......#.....#...#.#.....E.#",
              "########################################"}}};

        int rows = (int)levels[0].grid.size();
        int cols = (int)levels[0].grid[0].size();
        windowWidth = cols * tileSize;
        windowHeight = rows * tileSize + hudHeight; // HUD area (top)

        restartLevel();

        lastTime = clockNow();
    }

    void start(int argc, char **argv)
    {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA);
        glutInitWindowSize(windowWidth, windowHeight);
        window = glutCreateWindow("Maze Run (GLUT)");
        glutFullScreen(); // FULLSCREEN MODE
        glutDisplayFunc(displayStatic);
        glutReshapeFunc(reshapeStatic);
        glutKeyboardFunc(keyboardStatic);
        glutSpecialFunc(specialStatic);
        glutIdleFunc(idleStatic);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0, 0, 0, 1);
        lastTime = clockNow();
        glutMainLoop();
    }

private:
    std::vector<Level> levels;
    int currentLevel;
    int tileSize;
    int window, windowWidth, windowHeight;
    int hudHeight = 60;

    int score;
    int levelTimeLimit;
    float levelElapsed;
    bool gameOver;

    std::chrono::steady_clock::time_point lastTime;
    bool moving;
    float moveSpeed;
    std::pair<int, int> playerGrid;
    std::pair<float, float> playerPos;
    std::pair<int, int> moveTargetGrid;

    // running / animation state
    bool humanRunning;
    bool animalRunning;
    float runPhase; // for leg animation
    // animal (background) position and path
    float animalX;
    float animalY;
    float animalSpeed;
    int animalDir;
    float animalStartX;
    float animalEndX;
    bool caughtByAnimal;
    bool timeStarted;
    bool levelFailed;
    // chasing state
    bool animalActive; // becomes true when spawned
    float animalSpawnTimer;
    float animalSpawnDelay;                      // seconds after human starts
    std::deque<std::pair<int, int>> playerTrail; // grid positions the player has visited
    std::pair<int, int> animalGrid;              // animal current grid
    std::pair<float, float> animalPosF;          // animal precise pixel position
    std::pair<int, int> animalTargetGrid;
    bool animalMoving;
    float animalMoveSpeed;
    // texture for lion-like animal
    unsigned int lionTexture;
    bool lionTextureLoaded;

    // helpers to convert
    float gridToPixelX(int gx) const { return gx * tileSize + tileSize / 2.0f; }
    float gridToPixelY(int gy) const { return gy * tileSize + tileSize / 2.0f + hudHeight; }
    std::chrono::steady_clock::time_point clockNow() { return std::chrono::steady_clock::now(); }
    float deltaSeconds()
    {
        auto now = clockNow();
        std::chrono::duration<float> diff = now - lastTime;
        lastTime = now;
        return diff.count();
    }

    void restartLevel()
    {
        moving = false;
        levelElapsed = 0.0f;
        gameOver = false;
        Grid &g = levels[currentLevel].grid;
        for (int y = 0; y < (int)g.size(); ++y)
        {
            for (int x = 0; x < (int)g[y].size(); ++x)
            {
                if (g[y][x] == 'S')
                {
                    playerGrid = {x, y};
                    playerPos = {gridToPixelX(x), gridToPixelY(y)};
                    goto found;
                }
            }
        }
    found:;
        // initialize animation/running state for this level
        humanRunning = false;
        animalRunning = false;
        runPhase = 0.0f;
        caughtByAnimal = false;
        levelFailed = false;
        // do not start the timer immediately on restart; start when player first moves
        timeStarted = false;
        // chasing/trail init
        playerTrail.clear();
        animalActive = false;
        animalSpawnTimer = 0.0f;
        animalSpawnDelay = 3.0f; // spawn 3s after human starts (delayed by +1s)
        animalGrid = {-1, -1};
        animalPosF = {animalX, animalY};
        animalTargetGrid = {-1, -1};
        animalMoving = false;
        animalMoveSpeed = moveSpeed * 0.9f; // slightly slower than player
        int rows = (int)g.size();
        int cols = (int)g[0].size();
        // animal will patrol horizontally across the maze near the top
        animalStartX = gridToPixelX(1);
        animalEndX = gridToPixelX(std::max(2, cols - 2));
        animalX = animalStartX;
        animalY = gridToPixelY(1);
        animalSpeed = tileSize * 1.0f; // pixels per second
        animalDir = 1;
    }

    // Input
    void onKey(unsigned char k)
    {
        if (gameOver)
        {
            if (k == 'r' || k == 'R')
            {
                if (levelFailed)
                {
                    // restart current level only
                    restartLevel();
                }
                else
                {
                    // full reset (e.g., after win)
                    score = 0;
                    currentLevel = 0;
                    restartLevel();
                }
            }
            else if (k == 'q' || k == 'Q')
            {
                glutDestroyWindow(window);
                exit(0);
            }
            return;
        }
        if (!moving)
        {
            if (k == 'r' || k == 'R')
            {
                restartLevel();
                return;
            }
            if (k == 'q' || k == 'Q')
            {
                glutDestroyWindow(window);
                exit(0);
            }
            // WASD
            if (k == 'w' || k == 'W')
                tryMove({0, -1});
            if (k == 's' || k == 'S')
                tryMove({0, 1});
            if (k == 'a' || k == 'A')
                tryMove({-1, 0});
            if (k == 'd' || k == 'D')
                tryMove({1, 0});
        }
    }

    void onSpecial(int key)
    {
        if (gameOver)
            return;
        if (!moving)
        {
            // start the game timer and bump human speed when the user presses an arrow key
            if (!timeStarted && (key == GLUT_KEY_UP || key == GLUT_KEY_DOWN || key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT))
            {
                timeStarted = true;
                animalMoveSpeed = moveSpeed * 0.9f; // slightly slower than player

                // try to load lion texture (one-time)
                if (!lionTextureLoaded)
                {
                    lionTextureLoaded = false;
                    std::string texPath = "lion.bmp";
                    std::ifstream f(texPath, std::ios::binary);
                    if (f)
                    {
                        // naive BMP loader for 24-bit BMP
                        f.seekg(0, std::ios::end);
                        size_t size = (size_t)f.tellg();
                        f.seekg(0);
                        std::vector<unsigned char> data(size);
                        f.read((char *)data.data(), size);
                        // check header
                        if (data.size() > 54 && data[0] == 'B' && data[1] == 'M')
                        {
                            int w = *(int *)&data[18];
                            int h = *(int *)&data[22];
                            int offset = *(int *)&data[10];
                            int bpp = *(short *)&data[28];
                            if (bpp == 24)
                            {
                                // create GL texture
                                glGenTextures(1, &lionTexture);
                                glBindTexture(GL_TEXTURE_2D, lionTexture);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                                // BMP is BGR, bottom-up rows
                                std::vector<unsigned char> img(w * h * 3);
                                int rowSize = ((w * 3 + 3) / 4) * 4;
                                for (int y = 0; y < h; ++y)
                                {
                                    int srcRow = offset + (h - 1 - y) * rowSize;
                                    memcpy(&img[y * w * 3], &data[srcRow], w * 3);
                                }
                                // convert BGR -> RGB since GL_BGR may not be available
                                for (int pi = 0; pi < w * h; ++pi)
                                {
                                    unsigned char b = img[pi * 3 + 0];
                                    unsigned char r = img[pi * 3 + 2];
                                    img[pi * 3 + 0] = r;
                                    img[pi * 3 + 2] = b;
                                }
                                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());
                                lionTextureLoaded = true;
                            }
                        }
                    }
                }
            }

            if (key == GLUT_KEY_UP)
                tryMove({0, -1});
            if (key == GLUT_KEY_DOWN)
                tryMove({0, 1});
            if (key == GLUT_KEY_LEFT)
                tryMove({-1, 0});
            if (key == GLUT_KEY_RIGHT)
                tryMove({1, 0});
        }
    }

    void tryMove(const std::pair<int, int> &dir)
    {
        int nx = playerGrid.first + dir.first;
        int ny = playerGrid.second + dir.second;
        Grid &g = levels[currentLevel].grid;
        if (ny >= 0 && ny < (int)g.size() && nx >= 0 && nx < (int)g[0].size())
        {
            if (g[ny][nx] != '#')
            {
                // if this is the player's first intentional move, start running timer
                if (!humanRunning && playerTrail.empty())
                {
                    humanRunning = true;
                    // start the level timer when the player makes their first intentional move (WASD)
                    timeStarted = true;
                    animalSpawnTimer = 0.0f; // start counting to spawn
                }
                moving = true;
                moveTargetGrid = {nx, ny};
            }
        }
    }

    void update(float dt)
    {
        if (gameOver)
            return;
        // advance level timer only after the player starts the game with an arrow key
        if (timeStarted)
            levelElapsed += dt;

        // advance run phase for animations
        if (humanRunning)
            runPhase += dt * 12.0f;

        // handle animal spawn timing: spawn animalSpawnDelay seconds after human started moving
        if (humanRunning && !animalActive)
        {
            animalSpawnTimer += dt;
            if (animalSpawnTimer >= animalSpawnDelay)
            {
                // activate animal: position it at the player's start or off-left if unknown
                animalActive = true;
                // prefer spawning at maze 'S' position: use the first trail element if available
                if (!playerTrail.empty())
                {
                    animalGrid = playerTrail.front();
                    animalPosF.first = gridToPixelX(animalGrid.first);
                    animalPosF.second = gridToPixelY(animalGrid.second);
                }
                else
                {
                    animalPosF.first = animalX; // fallback
                    animalPosF.second = animalY;
                }
                animalMoving = false;
            }
        }

        if (moving)
        {
            float targetX = gridToPixelX(moveTargetGrid.first);
            float targetY = gridToPixelY(moveTargetGrid.second);
            float dx = targetX - playerPos.first;
            float dy = targetY - playerPos.second;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 1.5f)
            {
                playerPos.first = targetX;
                playerPos.second = targetY;
                playerGrid = moveTargetGrid;
                moving = false;
                onCellEntered(playerGrid);
            }
            else
            {
                float step = (moveSpeed * tileSize) * dt;
                playerPos.first += dx / dist * step;
                playerPos.second += dy / dist * step;
            }
        }

        // animal movement: if active, follow the trail; otherwise idle
        if (animalActive)
        {
            // choose next target grid from the front of playerTrail
            if (!animalMoving && !playerTrail.empty())
            {
                animalTargetGrid = playerTrail.front();
                animalMoving = true;
            }

            if (animalMoving)
            {
                float targetX = gridToPixelX(animalTargetGrid.first);
                float targetY = gridToPixelY(animalTargetGrid.second);
                float dx = targetX - animalPosF.first;
                float dy = targetY - animalPosF.second;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < 1.5f)
                {
                    animalPosF.first = targetX;
                    animalPosF.second = targetY;
                    animalMoving = false;
                    // consume that step so animal follows the footstep
                    if (!playerTrail.empty())
                        playerTrail.pop_front();
                }
                else
                {
                    float step = (animalMoveSpeed * tileSize) * dt;
                    animalPosF.first += dx / dist * step;
                    animalPosF.second += dy / dist * step;
                }
            }
        }
        else
        {
            // fallback patrol while inactive (keep previous simple X motion)
            if (animalRunning)
            {
                animalX += animalDir * animalSpeed * dt;
                if (animalX < animalStartX)
                {
                    animalX = animalStartX;
                    animalDir = 1;
                }
                else if (animalX > animalEndX)
                {
                    animalX = animalEndX;
                    animalDir = -1;
                }
            }
        }

        // collision: animal touches human (only when animal active)
        if (animalActive && !caughtByAnimal)
        {
            float ax = animalPosF.first;
            float ay = animalPosF.second;
            float dx = ax - playerPos.first;
            float dy = ay - playerPos.second;
            float dist2 = dx * dx + dy * dy;
            float catchRadius = tileSize * 0.5f;
            if (dist2 <= catchRadius * catchRadius)
            {
                caughtByAnimal = true;
                gameOver = true;
                levelFailed = true;
                std::cout << "Caught by the animal!\n";
            }
        }

        if ((int)levelElapsed >= levelTimeLimit)
        {
            gameOver = true;
            levelFailed = true;
            std::cout << "Time's up!\n";
        }
    }

    void onCellEntered(const std::pair<int, int> &cell)
    {
        Grid &g = levels[currentLevel].grid;
        char c = g[cell.second][cell.first];
        // record visited grid into trail (for animal to follow)
        playerTrail.push_back(cell);
        // keep trail length reasonable
        if (playerTrail.size() > 512)
            playerTrail.pop_front();
        if (c == 'B')
        {
            score += 50;
            levelElapsed = std::max(0.0f, levelElapsed - 10.0f);
            g[cell.second][cell.first] = '.';
        }
        else if (c == 'E')
        {
            int timeLeft = std::max(0, levelTimeLimit - (int)levelElapsed);
            score += timeLeft * 10;
            if (currentLevel + 1 < (int)levels.size())
            {
                currentLevel++;
                restartLevel();
            }
            else
            {
                gameOver = true;
                std::cout << "You Win! Final Score: " << score << std::endl;
            }
        }
    }

    // Rendering helpers
    void drawRectFilled(float x, float y, float w, float h)
    {
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();
    }

    void drawCircleFilled(float cx, float cy, float r, int segments = 32)
    {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments; ++i)
        {
            float a = (float)i / segments * 2.0f * 3.14159265f;
            glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
        }
        glEnd();
    }

    // Draw a simple human: body, head, and legs animated by runPhase
    void drawHuman(float cx, float cy, float size)
    {
        // drop shadow
        glColor4f(0, 0, 0, 0.25f);
        drawCircleFilled(cx, cy + size * 0.6f, size * 0.25f, 20);

        // body with simple highlight
        glColor3ub(20, 120, 20);
        drawRectFilled(cx - size * 0.15f, cy - size * 0.35f, size * 0.3f, size * 0.45f);
        glColor3ub(60, 180, 60);
        drawRectFilled(cx - size * 0.15f, cy - size * 0.35f, size * 0.08f, size * 0.45f); // left highlight

        // head (slightly shaded)
        glColor3ub(240, 200, 160);
        drawCircleFilled(cx, cy - size * 0.55f, size * 0.18f, 20);
        glColor3ub(200, 160, 120);
        drawCircleFilled(cx - size * 0.04f, cy - size * 0.57f, size * 0.06f, 8); // cheek shade

        // legs animated by runPhase
        float phase = sinf(runPhase) * 0.6f;
        float legLen = size * 0.35f;
        glColor3ub(40, 40, 40);
        drawRectFilled(cx - size * 0.19f - phase * 3.0f, cy + size * 0.15f, size * 0.08f, legLen);
        drawRectFilled(cx + size * 0.07f + phase * 3.0f, cy + size * 0.15f, size * 0.08f, legLen);
    }

    // Draw a simple animal (dog-like) as a rectangle body and circle head, moves along X
    void drawAnimal(float ax, float ay, float scale = 0.6f)
    {
        // drop shadow
        glColor4f(0, 0, 0, 0.25f);
        drawRectFilled(ax - tileSize * scale * 0.5f, ay + tileSize * scale * 0.35f, tileSize * scale, tileSize * 0.12f);

        float bodyW = tileSize * scale;
        float bodyH = tileSize * 0.42f * scale;
        // body with gradient-like effect
        glColor3ub(180, 110, 60);
        drawRectFilled(ax - bodyW / 2.0f, ay - bodyH / 2.0f, bodyW, bodyH);
        glColor3ub(140, 80, 40);
        drawRectFilled(ax - bodyW / 2.0f, ay - bodyH / 2.0f, bodyW * 0.4f, bodyH);

        // head with a lighter forehead
        glColor3ub(120, 70, 30);
        drawCircleFilled(ax + bodyW * 0.46f, ay - bodyH * 0.18f, bodyH * 0.6f, 16);
        glColor3ub(160, 100, 60);
        drawCircleFilled(ax + bodyW * 0.5f, ay - bodyH * 0.22f, bodyH * 0.22f, 8); // snout highlight

        // legs (simple)
        glColor3ub(90, 50, 30);
        drawRectFilled(ax - bodyW * 0.33f, ay + bodyH / 2.0f, bodyW * 0.12f, bodyH * 0.9f);
        drawRectFilled(ax + bodyW * 0.03f, ay + bodyH / 2.0f, bodyW * 0.12f, bodyH * 0.9f);
    }

    // draw a textured quad centered at cx,cy with width w and height h
    void drawTexturedQuad(unsigned int tex, float cx, float cy, float w, float h)
    {
        if (!lionTextureLoaded)
            return;
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1);
        glVertex2f(cx - w / 2, cy - h / 2);
        glTexCoord2f(1, 1);
        glVertex2f(cx + w / 2, cy - h / 2);
        glTexCoord2f(1, 0);
        glVertex2f(cx + w / 2, cy + h / 2);
        glTexCoord2f(0, 0);
        glVertex2f(cx - w / 2, cy + h / 2);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }

    void drawText(const std::string &s, float x, float y)
    {
        glRasterPos2f(x, y);
        for (char c : s)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }

    void drawCenteredTextBig(const std::string &s, float cx, float cy)
    {
        // approximate width: 9 pixels per char at GLUT_BITMAP_HELVETICA_18
        int len = (int)s.size();
        float px = cx - (len * 9.0f) / 2.0f;
        float py = cy - 9.0f;
        glRasterPos2f(px, py);
        for (char c : s)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    void renderScene()
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // top HUD background
        glColor3ub(20, 20, 20);
        drawRectFilled(0, 0, windowWidth, hudHeight);

        Grid &g = levels[currentLevel].grid;
        int rows = (int)g.size();
        int cols = (int)g[0].size();

        // draw tiles
        for (int y = 0; y < rows; ++y)
        {
            for (int x = 0; x < cols; ++x)
            {
                char c = g[y][x];
                float px = x * tileSize + 1;
                float py = y * tileSize + 1 + hudHeight;
                float size = tileSize - 2;

                if (c == '#')
                {
                    glColor3ub(80, 80, 80);
                    drawRectFilled(px, py, size, size);
                }
                else
                {
                    glColor3ub(30, 30, 30);
                    drawRectFilled(px, py, size, size);

                    if (c == 'B')
                    {
                        // pulsing
                        float pulse = 1.0f + 0.15f * sinf(levelElapsed * 6.0f);
                        float r = (tileSize * 0.25f) * pulse;
                        glColor3ub(200, 180, 0);
                        drawCircleFilled(gridToPixelX(x), gridToPixelY(y), r, 32);
                    }
                    else if (c == 'E')
                    {
                        glColor3ub(0, 0, 200);
                        drawRectFilled(px + 3, py + 3, size - 6, size - 6);
                    }
                }
            }
        }

        // draw background animal (behind player) only when active (or faintly hidden before)
        if (animalActive)
        {
            if (lionTextureLoaded)
            {
                float aw = tileSize * 1.0f;
                float ah = tileSize * 1.0f;
                drawTexturedQuad(lionTexture, animalPosF.first, animalPosF.second - tileSize * 0.1f, aw, ah);
            }
            else
            {
                drawAnimal(animalPosF.first, animalPosF.second, 0.6f);
            }
        }

        // draw player as a simple human with bobbing
        float bob = 1.0f + 0.05f * sinf(levelElapsed * 8.0f);
        float humanSize = tileSize * 0.6f * bob;
        drawHuman(playerPos.first, playerPos.second, humanSize);

        // HUD at top-left (render bold by drawing twice)
        glColor3ub(10, 10, 10);
        std::string hud = "Level: " + std::to_string(currentLevel + 1) + "/" + std::to_string(levels.size()) + "   Score: " + std::to_string(score) + "   Time: " + std::to_string(std::max(0, levelTimeLimit - (int)levelElapsed)) + "s";
        drawText(hud, 8.0f + 1.0f, 18.0f + 1.0f); // shadow
        glColor3ub(255, 255, 255);
        drawText(hud, 8.0f, 18.0f);
        // controls small note
        std::string ctrl = "Controls: WASD/Arrow - Move, R - Restart, Q - Quit";
        drawText(ctrl, 8.0f, 34.0f);

        // Game over overlay
        if (gameOver)
        {
            glEnable(GL_BLEND);
            glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
            drawRectFilled(0, 0, windowWidth, windowHeight);
            glDisable(GL_BLEND);

            glColor3ub(255, 255, 255);
            std::string msg;
            if (caughtByAnimal)
            {
                msg = "CAUGHT! The animal got you. Press R to restart or Q to quit";
            }
            else if ((int)levelElapsed >= levelTimeLimit)
            {
                msg = "TIME'S UP! Press R to restart or Q to quit";
            }
            else
            {
                msg = "YOU WIN! Press R to play again or Q to quit";
            }
            drawCenteredTextBig(msg, windowWidth / 2.0f, windowHeight / 2.0f - 10.0f);
        }

        glutSwapBuffers();
    }

    // GLUT static wrappers map to member functions
    static MazeGame *instance()
    {
        static MazeGame *inst = nullptr;
        return inst;
    }

public:
    // For static wrappers we store pointer externally when creating the instance in main
    static MazeGame *setInstance(MazeGame *m)
    {
        MazeGame *old = instance();
        instPtr = m;
        return instPtr;
    }

private:
    static MazeGame *instPtr;

    static void displayStatic()
    {
        instPtr->renderScene();
    }
    static void reshapeStatic(int w, int h)
    {
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, w, h, 0); // top-left origin like window coordinates
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
    static void keyboardStatic(unsigned char key, int x, int y)
    {
        instPtr->onKey(key);
    }
    static void specialStatic(int key, int x, int y)
    {
        instPtr->onSpecial(key);
    }
    static void idleStatic()
    {
        float dt = instPtr->deltaSeconds();
        if (dt > 0.1f)
            dt = 0.1f;
        instPtr->update(dt);
        glutPostRedisplay();
    }
};

MazeGame *MazeGame::instPtr = nullptr;

// ---------- main ----------
int main(int argc, char **argv)
{
    MazeGame *game = new MazeGame(32);
    MazeGame::setInstance(game);
    game->start(argc, argv);
    return 0;
}