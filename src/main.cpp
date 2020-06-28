#include "Keyboard.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <cassert>
#include <cmath>
#include "Renderer.h"

constexpr float PI = 3.14159;
constexpr float SPEED = 2.0;
constexpr int FOV = 60;

constexpr int MAP_SIZE = 20;
constexpr int TILE_SIZE = 64;
constexpr int MINIMAP_SCALE = 4;
constexpr int MINIMAP_TILE_SIZE = TILE_SIZE / MINIMAP_SCALE;
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

float distance(const sf::Vector2f& vecA, sf::Vector2f& vectB)
{
    float dx = (vecA.x - vectB.x);
    float dy = (vecA.y - vectB.y);
    return std::sqrt(dx * dx + dy * dy);
}

struct Map {
    // clang-format off
    std::vector<int> MAP = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    // clang-format on

    Map()
    {
        assert(MAP.size() == (MAP_SIZE * MAP_SIZE));
    }

    int getTile(int x, int y) const
    {
        if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE)
            return 0;
        return MAP[y * MAP_SIZE + x];
    }

    void setTile(int x, int y, int tile)
    {
        if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE)
            return;
        MAP[y * MAP_SIZE + x] = tile;
    }
};

struct Player {
    float x = 153;
    float y = 221;
    float angle = 0;

    // How much the player moves in the X/ Y direction when moving forwards or back
    float dx = 0;
    float dy = 0;

    sf::CircleShape rayCastSprite;

    Player()
    {
        rayCastSprite.setRadius({5.0f});
        dx = std::cos(angle);
        dy = std::sin(angle);
    }

    void doInput(const Keyboard& keys)
    {
        float a = rads(angle);
        if (keys.isKeyDown(sf::Keyboard::W)) {
            x += dx * SPEED;
            y += dy * SPEED;
        }
        if (keys.isKeyDown(sf::Keyboard::A)) {
            angle -= SPEED;
            if (angle < 0) {
                angle += 360;
            }
            dx = std::cos(a);
            dy = std::sin(a);
        }
        if (keys.isKeyDown(sf::Keyboard::S)) {
            x -= dx * SPEED;
            y -= dy * SPEED;
        }
        if (keys.isKeyDown(sf::Keyboard::D)) {
            angle += SPEED;
            if (angle > 360) {
                angle -= 360;
            }
            dx = std::cos(a);
            dy = std::sin(a);
        }
    }

    void draw(sf::RenderTexture& window)
    {
        rayCastSprite.setPosition(x / MINIMAP_SCALE - 5, y / MINIMAP_SCALE - 5);
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
        player.doInput(keyboard);

        // Update

        // Clear
        window.clear();
        drawBuffer.clearPixels();
        minimapTexture.clear(sf::Color::Transparent);

        // ================= Raycasting starts here ========================
        // Get the starting angle of the ray, that is half the FOV to the "left" of the
        // player's looking angle
        float rayAngle = wrap(player.angle - FOV / 2);
        for (int i = 0; i < WINDOW_WIDTH; i++) {

            
            // These need to be stored for later so they can be compared
            sf::Vector2f horizonatalIntersect;
            sf::Vector2f verticalIntersect;
            // =============== Horizontal line =========================
            // Y Intersection -> Divide the player's Y position by the size of the tiles,
            //  +64 if the ray is looking "down"
            // X Intersection -> Use tan and trig where:
            //  Opp = (Y Intersection - Player Y position)
            //  Theta = Ray's angle
            //  tan(Theta) = Opp / X Intersection so X Intersection = Opp / tan(Theta)
            bool down = rayAngle < 180;
            {
                sf::Vector2f initialIntersect;
                initialIntersect.y = std::floor(player.y / TILE_SIZE) * TILE_SIZE +
                                     (rayAngle < 180 ? TILE_SIZE : -1);
                initialIntersect.x =
                    (initialIntersect.y - player.y) / std::tan(rads(rayAngle)) + player.x;

                // Find distances to the next intersection
                sf::Vector2f distance;
                distance.y = rayAngle < 180 ? TILE_SIZE : -TILE_SIZE;
                distance.x = TILE_SIZE / (rayAngle < 180 ? std::tan(rads(rayAngle))
                                                         : -std::tan(rads(rayAngle)));

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
                initialIntersect.x = std::floor(player.x / TILE_SIZE) * TILE_SIZE +
                                     (left ? -1 : TILE_SIZE);
                initialIntersect.y =
                    (initialIntersect.x - player.x) * std::tan(rads(rayAngle)) + player.y;

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
            float hDist = distance({player.x, player.y}, horizonatalIntersect);
            float vDist = distance({player.x, player.y}, verticalIntersect) - (left ? 1 : 0);
            float dist = 0;
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
                TILE_SIZE / dist * (WINDOW_WIDTH / 2 / std::tan(rads(FOV / 2)));
            int start = (WINDOW_HEIGHT / 2) - height / 2;
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
                    minimapTexture, {player.x / MINIMAP_SCALE, player.y / MINIMAP_SCALE},
                    horizonatalIntersect / (float)MINIMAP_SCALE, {0, 0, 255, 50});
            }
            else {
                drawBuffer.renderLine(
                    minimapTexture, {player.x / MINIMAP_SCALE, player.y / MINIMAP_SCALE},
                    verticalIntersect / (float)MINIMAP_SCALE, {255, 0, 0, 50});
            }
            
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
        drawBuffer.renderLine(minimapTexture,
                            {player.x / MINIMAP_SCALE, player.y / MINIMAP_SCALE},
                            {player.x / MINIMAP_SCALE + player.dx * 25,
                             player.y / MINIMAP_SCALE + player.dy * 25},
                            sf::Color::Yellow);

        minimapTexture.display();
        minimapSprite.setTexture(&minimapTexture.getTexture());
        window.draw(minimapSprite);

        window.display();
    }
}