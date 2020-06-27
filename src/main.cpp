#include "keyboard.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <cmath>
#include <cstring>
#include <iostream>

constexpr float PI = 3.14159;
constexpr float SPEED = 2.0;
constexpr int FOV = 60;

constexpr int MAP_SIZE = 10;
constexpr int TILE_SIZE = 64;
constexpr int WINDOW_WIDTH = TILE_SIZE * MAP_SIZE * 2;
constexpr int WINDOW_HEIGHT = TILE_SIZE * MAP_SIZE;

constexpr int PROJECTION_WIDTH = WINDOW_WIDTH / 2;

constexpr int EYE_HEIGHT = 32;

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
    // clang-format off
    const std::vector<int> MAP = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 1, 1, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
    // clang-format on

    int getTile(int x, int y) const
    {
        if (x < 0 || x > MAP_SIZE || y < 0 || y > MAP_SIZE)
            return 0;
        return MAP[y * MAP_SIZE + x];
    }
};

struct Player {
    float x = 153;
    float y = 221;
    float angle = 0;

    // How much the player moves in the X/ Y direction when moving forwards or back
    float dx = 0;
    float dy = 0;

    sf::RectangleShape sprite;

    Player()
    {
        sprite.setSize({10.0f, 10.0f});
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

    void draw(sf::RenderWindow& window)
    {
        sprite.setPosition(x - 5, y - 5);
        window.draw(sprite);
    }
};

struct Drawbuffer {
    std::vector<sf::Uint8> pixels;
    std::vector<sf::Vertex> line;

    Drawbuffer()
        : pixels((WINDOW_WIDTH / 2) * WINDOW_HEIGHT * 4)
    {
        line.emplace_back(sf::Vector2f{0, 0}, sf::Color::Red);
        line.emplace_back(sf::Vector2f{0, 0}, sf::Color::Red);
    }

    // For the minimap
    void drawLine(sf::RenderWindow& window, const sf::Vector2f& begin,
                  const sf::Vector2f& end, sf::Color colour)
    {
        line[0].position = begin;
        line[1].position = end;
        line[0].color = colour;
        line[1].color = colour;
        window.draw(line.data(), 2, sf::PrimitiveType::Lines);
    }

    void clear()
    {
        std::memset(pixels.data(), 0, pixels.size());
    }

    void set(int x, int y, sf::Uint8 red, sf::Uint8 green, sf::Uint8 blue)
    {
        sf::Uint8* ptr = &pixels[(y * WINDOW_WIDTH + x) * 4];
        ptr[0] = red;
        ptr[1] = green;
        ptr[2] = blue;
        ptr[3] = 255;
    }
};

int main()
{
    sf::RenderWindow window({WINDOW_WIDTH, WINDOW_HEIGHT}, "Raycast Test");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    Drawbuffer drawBuffer;
    Map map;
    Player player;

    sf::Texture texture;
    texture.create(WINDOW_WIDTH / 2, WINDOW_HEIGHT);

    sf::RectangleShape sprite;
    sprite.setSize({WINDOW_WIDTH / 2, WINDOW_HEIGHT});
    sprite.move(WINDOW_WIDTH / 2, 0);

    sf::RectangleShape minimapTile;
    minimapTile.setSize({TILE_SIZE, TILE_SIZE});
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

                default:
                    break;
            }
        }
        // Input
        player.doInput(keyboard);

        // Update

        // Clear
        window.clear();
        drawBuffer.clear();

        // Render the minimap
        for (int y = 0; y < MAP_SIZE; y++) {
            for (int x = 0; x < MAP_SIZE; x++) {
                switch (map.getTile(x, y)) {
                    case 1:
                        minimapTile.setFillColor(sf::Color::White);
                        break;

                    default:
                        minimapTile.setFillColor({127, 127, 127});
                        break;
                }

                minimapTile.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                window.draw(minimapTile);
            }
        }
        player.draw(window);

        //
        //  Raycasting starts here
        //

        // Get the starting angle of the ray, that is half the FOV to the "left" of the
        // player's looking angle
        float rayAngle = wrap(player.angle - FOV / 2);
        for (int i = 0; i < 1; i++) {
            //
            // Horizontal line
            //
            // Y Intersection -> Divide the player's Y position by the size of the tiles,
            //  +64 if the ray is looking "down"
            // X Intersection -> Use tan and trig where:
            //  Opp = (Y Intersection - Player Y position)
            //  Theta = Ray's angle
            //  tan(Theta) = Opp / X Intersection so X Intersection = Opp / tan(Theta)
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
                sf::Vector2f next = initialIntersect + distance;
                int gridX = std::floor(next.x / TILE_SIZE);
                int gridY = std::floor(next.y / TILE_SIZE);

                while ((gridX >= 0 && gridX < MAP_SIZE) &&
                       map.getTile(gridX, gridY) == 0) {
                    next += distance;
                    gridX = std::floor(next.x / TILE_SIZE);
                    gridY = std::floor(next.y / TILE_SIZE);
                }

                drawBuffer.drawLine(window, {player.x, player.y}, next, sf::Color::Red);
            }

            //
            // Vertical line
            //
            {
                bool left = rayAngle > 90 && rayAngle < 270;
                sf::Vector2f initialIntersect;
                initialIntersect.x = std::floor(player.x / TILE_SIZE) * TILE_SIZE +
                                     (left ? -1 : TILE_SIZE);
                initialIntersect.y =
                    (initialIntersect.x - player.x) * std::tan(rads(rayAngle)) + player.y;

                // Find distances to the next intersection
                sf::Vector2f distance;
                distance.x = left ? -TILE_SIZE : TILE_SIZE;
                distance.y = TILE_SIZE * (left ? -std::tan(rads(rayAngle))
                                               : std::tan(rads(rayAngle)));

                sf::Vector2f next = initialIntersect + distance;
                int gridX = std::floor(next.x / TILE_SIZE);
                int gridY = std::floor(next.y / TILE_SIZE);

                while ((gridX >= 0 && gridX < MAP_SIZE) &&
                       map.getTile(gridX, gridY) == 0) {
                    next += distance;
                    gridX = std::floor(next.x / TILE_SIZE);
                    gridY = std::floor(next.y / TILE_SIZE);
                }

                drawBuffer.drawLine(window, {player.x, player.y}, next, sf::Color::Blue);
            }

            rayAngle = wrap(rayAngle + (float)FOV / (float)PROJECTION_WIDTH);
        }
        drawBuffer.drawLine(window, {player.x, player.y},
                            {player.x + player.dx * 25, player.y + player.dy * 25},
                            sf::Color::Yellow);

        sprite.setTexture(&texture);
        texture.update(drawBuffer.pixels.data());
        window.draw(sprite);

        window.display();
    }
}