#include "Keyboard.h"
#include "Renderer.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>

constexpr float PI = 3.14159;
constexpr int FOV = 60;

constexpr int MAP_SIZE = 20;
constexpr int MINIMAP_TILE_SIZE = 32;
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

float rads(float degs)
{
    return degs * PI / 180.f;
}

float wrap(float angle)
{
    if (angle < 0) {
        return angle + 360;
    }
    else if (angle > 360) {
        return angle - 360;
    }
    else {
        return angle;
    }
}

struct Map {
    std::vector<int> map;

    Map()
    {
        std::ifstream inFile("map.txt");
        std::string line;
        map.reserve(MAP_SIZE * MAP_SIZE);
        while (std::getline(inFile, line)) {
            for (auto c : line) {
                map.push_back(c - '0');
            }
        }
        assert(map.size() == (MAP_SIZE * MAP_SIZE));
    }

    int getTile(int x, int y) const
    {
        if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE)
            return 0;
        return map[y * MAP_SIZE + x];
    }

    void setTile(int x, int y, int tile)
    {
        if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE)
            return;
        map[y * MAP_SIZE + x] = tile;
    }
};

struct Player {
    sf::Vector2f position{2, 2};
    sf::Vector2f delta;
    float angle = 0;
    float turnSpeed = 64.0f;
    float moveSpeed = 3.0f;

    sf::CircleShape rayCastSprite;

    Player()
    {
        rayCastSprite.setRadius({5.0f});
        delta.x = std::cos(angle);
        delta.y = std::sin(angle);
    }

    void doInput(const Keyboard& keys, float dt)
    {
        float a = rads(angle);
        if (keys.isKeyDown(sf::Keyboard::W)) {
            position += delta * moveSpeed * dt;
        }

        else if (keys.isKeyDown(sf::Keyboard::S)) {
            position -= delta * moveSpeed * dt;
        }

        if (keys.isKeyDown(sf::Keyboard::A)) {
            angle -= turnSpeed * dt;
            if (angle < 0) {
                angle += 360;
            }
            delta.x = std::cos(a);
            delta.y = std::sin(a);
        }
        else if (keys.isKeyDown(sf::Keyboard::D)) {
            angle += turnSpeed * dt;
            if (angle > 360) {
                angle -= 360;
            }
            delta.x = std::cos(a);
            delta.y = std::sin(a);
        }
    }

    void draw(sf::RenderTexture& window)
    {
        rayCastSprite.setPosition(position.x * MINIMAP_TILE_SIZE - 5,
                                  position.y * MINIMAP_TILE_SIZE - 5);
        window.draw(rayCastSprite);
    }
};

int main()
{
    sf::RenderWindow window({WINDOW_WIDTH, WINDOW_HEIGHT}, "Raycast Test");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    // For the minimap
    sf::RenderTexture minimapTexture;
    minimapTexture.create(MINIMAP_TILE_SIZE * MAP_SIZE, MINIMAP_TILE_SIZE * MAP_SIZE);
    sf::RectangleShape minimapSprite;
    minimapSprite.setSize(
        {(float)MINIMAP_TILE_SIZE * MAP_SIZE, (float)MINIMAP_TILE_SIZE * MAP_SIZE});

    Renderer drawBuffer;
    Map map;
    Player player;

    sf::RectangleShape minimapTile;
    minimapTile.setSize({MINIMAP_TILE_SIZE, MINIMAP_TILE_SIZE});
    minimapTile.setFillColor(sf::Color::White);
    minimapTile.setOutlineColor(sf::Color::Black);
    minimapTile.setOutlineThickness(1);

    sf::Clock timer;

    Keyboard keyboard;
    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            keyboard.update(e);
            switch (e.type) {
                case sf::Event::Closed:
                    window.close();
                    break;

                // Minimap editing
                case sf::Event::MouseButtonReleased: {
                    auto mouseEvent = e.mouseButton;
                    int x = mouseEvent.x;
                    int y = mouseEvent.y;
                    if (x > 0 && x < MAP_SIZE * MINIMAP_TILE_SIZE && y > 0 &&
                        y < MAP_SIZE * MINIMAP_TILE_SIZE) {
                        x /= MINIMAP_TILE_SIZE;
                        y /= MINIMAP_TILE_SIZE;
                        // toggle the tile
                        map.setTile(x, y, !map.getTile(x, y));
                    }
                    break;
                }

                default:
                    break;
            }
        }
        // Input
        player.doInput(keyboard, timer.restart().asSeconds());

        // Update

        // Clear
        window.clear();
        drawBuffer.clearPixels();
        minimapTexture.clear(sf::Color::Transparent);

        // ================= Raycasting starts here ========================
        // Get the starting angle of the ray, that is half the FOV to the "left" of the
        // player's looking angle
        float rayAngle = wrap(player.angle - FOV / 2);
        for (int x = 0; x < 1; x++) {
            float radians = rads(rayAngle);

            int tileX = std::floor(player.position.x);
            int tileY = std::floor(player.position.y);

            float deltaX = 1.0f - (player.position.x - tileX);
            float deltaY = player.position.y - tileY;

            if (rayAngle > 90 && rayAngle < 270) {
                deltaX -= 1;
            }
            if (rayAngle < 180) {
                deltaY -= 1;
            }

            // Distance from the player to the first horizontal intersect
            sf::Vector2f hIntersect = {-deltaY / std::tan(radians), -deltaY};

            // Distance from the player to the first vertical intersect
            sf::Vector2f vIntersect = {deltaX, deltaX * std::tan(radians)};

            // How much to step to hit the next vertical grid line
            sf::Vector2f vStep = {rayAngle > 90 && rayAngle < 270 ? -1.0f : 1.0f,
                                  rayAngle > 90 && rayAngle < 270 ? -std::tan(radians)
                                                                  : std::tan(radians)};

            // How much to step to hit the next horizontal grid line
            sf::Vector2f hStep = {rayAngle < 180 ? 1.0f / std::tan(radians)
                                                 : 1.0f / -std::tan(radians),
                                  rayAngle < 180 ? 1.0f : -1.0f};

            auto pos = player.position + hIntersect;
            int gridX = std::floor(hIntersect.x);
            int gridY = std::floor(hIntersect.y);
            while ((gridX >= 0 && gridX < MAP_SIZE) && map.getTile(gridX, gridY) == 0) {
                pos += hStep;
                gridX = std::floor(hIntersect.x);
                gridY = std::floor(hIntersect.y);
            }

            drawBuffer.renderLine(minimapTexture,
                                  player.position * (float)MINIMAP_TILE_SIZE,
                                  {(pos) * (float)MINIMAP_TILE_SIZE}, sf::Color::Blue);

            drawBuffer.renderLine(
                minimapTexture, player.position * (float)MINIMAP_TILE_SIZE,
                {(player.position.x + vIntersect.x) * MINIMAP_TILE_SIZE,
                 (player.position.y + vIntersect.y) * MINIMAP_TILE_SIZE},
                sf::Color::Red);

            rayAngle = wrap(rayAngle + (float)FOV / (float)WINDOW_WIDTH);

            /*

                        // These need to be stored for later so they can be compared
                        sf::Vector2f horizonatalIntersect;
                        sf::Vector2f verticalIntersect;
                        // =============== Horizontal line =========================
                        // Y Intersection -> Divide the player's Y position by the size of
               the tiles,
                        //  +64 if the ray is looking "down"
                        // X Intersection -> Use tan and trig where:
                        //  Opp = (Y Intersection - Player Y position)
                        //  Theta = Ray's angle
                        //  tan(Theta) = Opp / X Intersection so X Intersection = Opp /
               tan(Theta) bool down = rayAngle < 180;
                        {
                            sf::Vector2f initialIntersect;
                            initialIntersect.y = std::floor(player.position.y / TILE_SIZE)
               * TILE_SIZE + (rayAngle < 180 ? TILE_SIZE : -1); initialIntersect.x =
                                (initialIntersect.y - player.position.y) /
               std::tan(rads(rayAngle)) + player.position.x;

                            // Find distances to the next intersection
                            sf::Vector2f distance;
                            distance.y = rayAngle < 180 ? TILE_SIZE : -TILE_SIZE;
                            distance.x = TILE_SIZE / (rayAngle < 180 ?
               std::tan(rads(rayAngle)) : -std::tan(rads(rayAngle)));

                            int gridX = std::floor(initialIntersect.x / TILE_SIZE);
                            int gridY = std::floor(initialIntersect.y / TILE_SIZE);
                            sf::Vector2f next = initialIntersect;
                            while ((gridX >= 0 && gridX < MAP_SIZE) &&
                                   map.getTile(gridX, gridY) == 0) {
                                next += distance;
                                gridX = std::floor(next.x / TILE_SIZE);
                                gridY = std::floor(next.y / TILE_SIZE);
                            }
                            horizonatalIntersect = next;
                        }

                        bool left = rayAngle > 90 && rayAngle < 270;
                        // =============== Vertical line =========================
                        {

                            sf::Vector2f initialIntersect;
                            initialIntersect.x = std::floor(player.position.x / TILE_SIZE)
               * TILE_SIZE + (left ? -1 : TILE_SIZE); initialIntersect.y =
                                (initialIntersect.x - player.position.x) *
               std::tan(rads(rayAngle)) + player.position.y;

                            sf::Vector2f distance;
                            distance.x = left ? -TILE_SIZE : TILE_SIZE;
                            distance.y = TILE_SIZE * (left ? -std::tan(rads(rayAngle))
                                                           : std::tan(rads(rayAngle)));

                            sf::Vector2f next = initialIntersect;
                            int gridX = std::floor(next.x / TILE_SIZE);
                            int gridY = std::floor(next.y / TILE_SIZE);

                            while ((gridX >= 0 && gridX < MAP_SIZE) &&
                                   map.getTile(gridX, gridY) == 0) {
                                next += distance;
                                gridX = std::floor(next.x / TILE_SIZE);
                                gridY = std::floor(next.y / TILE_SIZE);
                            }
                            verticalIntersect = next;
                        }

                        // Find the shortest distance (And draw a ray on the minimap)
                        float hDist = distance({player.position.x, player.position.y},
               horizonatalIntersect); float vDist = distance({player.position.x,
               player.position.y}, verticalIntersect) - (left ? 1 : 0); float dist = 0;
                        sf::Color colour;
                        if (hDist < vDist) {
                            dist = hDist;
                            colour = {255, 153, 51};
                        }
                        else {
                            dist = vDist;
                            colour = {255, 204, 102};
                        }

                        // Fix the fisheye effect (not quite right...)
                        dist = std::cos(rads(FOV / 2)) * dist;

                        // Draw the walls
                        float height =
                            TILE_SIZE / dist * (WINDOW_WIDTH / 2 / std::tan(rads(FOV /
               2))); int start = (WINDOW_HEIGHT / 2) - height / 2;
                        // Draw the ceiling, then the wall, then the floor
                        // This is done by drawing vertical lines in a loop
                        for (int y = 0; y < start; y++) {
                            drawBuffer.setPixel(i, y, {135, 206, 235});
                        }
                        for (int y = start; y < start + height; y++) {
                            drawBuffer.setPixel(i, y, {colour.r, colour.g, colour.b});
                        }
                        for (int y = start + height; y < WINDOW_HEIGHT; y++) {
                            drawBuffer.setPixel(i, y, {0, 153, 51});
                        }

                        // Find the next ray angle
                        rayAngle = wrap(rayAngle + (float)FOV / (float)WINDOW_WIDTH);

                        // Draw rays for the mini map
                        if (hDist < vDist) {
                            drawBuffer.renderLine(
                                minimapTexture, {player.position.x / MINIMAP_SCALE,
               player.position.y / MINIMAP_SCALE}, horizonatalIntersect /
               (float)MINIMAP_SCALE, {0, 0, 255, 50});
                        }
                        else {
                            drawBuffer.renderLine(
                                minimapTexture, {player.position.x / MINIMAP_SCALE,
               player.position.y / MINIMAP_SCALE}, verticalIntersect /
               (float)MINIMAP_SCALE, {255, 0, 0, 50});
                        }
                        */
        }

        // Actually render the walls
        drawBuffer.render(window);

        // Render the minimap
        for (int y = 0; y < MAP_SIZE; y++) {
            for (int x = 0; x < MAP_SIZE; x++) {
                switch (map.getTile(x, y)) {
                    case 1:
                        minimapTile.setFillColor({255, 255, 255, 200});
                        break;

                    default:
                        minimapTile.setFillColor({127, 127, 127, 200});
                        break;
                }
                minimapTile.setPosition(x * MINIMAP_TILE_SIZE, y * MINIMAP_TILE_SIZE);
                window.draw(minimapTile);
            }
        }
        player.draw(minimapTexture);
        drawBuffer.renderLine(
            minimapTexture,
            {player.position.x * MINIMAP_TILE_SIZE,
             player.position.y * MINIMAP_TILE_SIZE},
            {player.position.x * MINIMAP_TILE_SIZE + player.delta.x * 25,
             player.position.y * MINIMAP_TILE_SIZE + player.delta.y * 25},
            sf::Color::Yellow);

        minimapTexture.display();
        minimapSprite.setTexture(&minimapTexture.getTexture());
        window.draw(minimapSprite);

        window.display();
    }
}