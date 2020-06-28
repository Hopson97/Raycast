#include "Keyboard.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>

#include "Constants.h"
#include "Renderer.h"

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

sf::Vector2f rotate(const sf::Vector2f& curr, float rad)
{
    return {curr.x * std::cos(rad) - curr.y * std::sin(rad),
            curr.x * std::sin(rad) + curr.y * std::cos(rad)};
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
    sf::Vector2f pos = {153, 221};
    sf::Vector2f dir = {-1, 0};
    sf::Vector2f plane = {0, 0.66};

    float rotSpeed = 5;
    float moveSpeed = 5;

    // Perpendicular vector to the direction

    sf::CircleShape rayCastSprite;

    Player()
    {
        rayCastSprite.setRadius(5.0f);
    }

    void doInput(const Keyboard& keys)
    {
        float rad = rads(rotSpeed);
        if (keys.isKeyDown(sf::Keyboard::W)) {
            pos += dir * moveSpeed;
        }
        if (keys.isKeyDown(sf::Keyboard::A)) {
            dir = rotate(dir, -rad);
            plane = rotate(plane, -rad);
        }
        if (keys.isKeyDown(sf::Keyboard::S)) {
            pos -= dir * moveSpeed;
        }
        if (keys.isKeyDown(sf::Keyboard::D)) {
            dir = rotate(dir, rad);
            plane = rotate(plane, rad);
        }
    }

    void draw(sf::RenderTexture& window)
    {
        rayCastSprite.setPosition(pos.x / MINIMAP_SCALE - 5, pos.y / MINIMAP_SCALE - 5);
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

        // DO RAYCASTING HERE

        // Actually render the walls
        drawBuffer.render(window);

        // Render the minimap
        for (int y = 0; y < MAP_SIZE; y++) {
            for (int x = 0; x < MAP_SIZE; x++) {
                switch (map.getTile(x, y)) {
                    case 1:
                        minimapTile.setFillColor({255, 0, 0, 200});
                        break;

                    case 2:
                        minimapTile.setFillColor({0, 255, 0, 200});
                        break;

                    case 3:
                        minimapTile.setFillColor({0, 0, 255, 200});
                        break;

                    default:
                        minimapTile.setFillColor({127, 127, 127, 200});
                        break;
                }
                minimapTile.setPosition(x * MINIMAP_TILE_SIZE, y * MINIMAP_TILE_SIZE);
                window.draw(minimapTile);
            }
        }
        drawBuffer.renderLine(minimapTexture,
                            {player.pos.x / MINIMAP_SCALE, player.pos.y / MINIMAP_SCALE},
                            {player.pos.x / MINIMAP_SCALE + player.dir.x * 25,
                             player.pos.y / MINIMAP_SCALE + player.dir.y * 25},
                            sf::Color::Yellow);

        drawBuffer.renderLine(minimapTexture,
                            {player.pos.x / MINIMAP_SCALE - player.plane.x * 25,
                             player.pos.y / MINIMAP_SCALE - player.plane.y * 25},
                            {player.pos.x / MINIMAP_SCALE + player.plane.x * 25,
                             player.pos.y / MINIMAP_SCALE + player.plane.y * 25},
                            sf::Color::Red);

        drawBuffer.renderLine(
            minimapTexture, {player.pos.x / MINIMAP_SCALE, player.pos.y / MINIMAP_SCALE},
            {player.pos.x / MINIMAP_SCALE + player.dir.x * 100 + player.plane.x * 100,
             player.pos.y / MINIMAP_SCALE + player.dir.y * 100 + player.plane.y * 100},
            sf::Color::Blue);

        drawBuffer.renderLine(
            minimapTexture, {player.pos.x / MINIMAP_SCALE, player.pos.y / MINIMAP_SCALE},
            {player.pos.x / MINIMAP_SCALE + player.dir.x * 100 - player.plane.x * 100,
             player.pos.y / MINIMAP_SCALE + player.dir.y * 100 - player.plane.y * 100},
            sf::Color::Blue);
        player.draw(minimapTexture);

        minimapTexture.display();
        minimapSprite.setTexture(&minimapTexture.getTexture());
        window.draw(minimapSprite);

        window.display();
    }
}