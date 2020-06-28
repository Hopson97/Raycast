#include "keyboard.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>

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

struct Drawbuffer {
    std::vector<sf::Uint8> pixels;
    std::vector<sf::Vertex> line;

    Drawbuffer()
        : pixels((WINDOW_WIDTH)*WINDOW_HEIGHT * 4)
    {
        line.emplace_back(sf::Vector2f{0, 0}, sf::Color::Red);
        line.emplace_back(sf::Vector2f{0, 0}, sf::Color::Red);
    }

    // For the minimap
    void drawLine(sf::RenderTarget& target, const sf::Vector2f& begin,
                  const sf::Vector2f& end, sf::Color colour)
    {
        line[0].position = begin;
        line[1].position = end;
        line[0].color = colour;
        line[1].color = colour;
        target.draw(line.data(), 2, sf::PrimitiveType::Lines);
    }

    void clear()
    {
        std::memset(pixels.data(), 0, pixels.size());
    }

    void set(int x, int y, sf::Uint8 red, sf::Uint8 green, sf::Uint8 blue)
    {
        if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
            return;
        }
        sf::Uint8* ptr = &pixels.at((y * WINDOW_WIDTH + x) * 4);
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

    // For the minimap
    sf::RenderTexture minimapTexture;
    minimapTexture.create(MINIMAP_TILE_SIZE * MAP_SIZE, MINIMAP_TILE_SIZE * MAP_SIZE);
    sf::RectangleShape minimapSprite;
    minimapSprite.setSize(
        {(float)MINIMAP_TILE_SIZE * MAP_SIZE, (float)MINIMAP_TILE_SIZE * MAP_SIZE});

    Drawbuffer drawBuffer;
    Map map;
    Player player;

    sf::Texture texture;
    texture.create(WINDOW_WIDTH, WINDOW_HEIGHT);

    sf::RectangleShape rayCastSprite;
    rayCastSprite.setSize({WINDOW_WIDTH, WINDOW_HEIGHT});

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
        drawBuffer.clear();
        minimapTexture.clear(sf::Color::Transparent);

        // Actually render the walls
        rayCastSprite.setTexture(&texture);
        texture.update(drawBuffer.pixels.data());
        window.draw(rayCastSprite);

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
        drawBuffer.drawLine(minimapTexture,
                            {player.pos.x / MINIMAP_SCALE, player.pos.y / MINIMAP_SCALE},
                            {player.pos.x / MINIMAP_SCALE + player.dir.x * 25,
                             player.pos.y / MINIMAP_SCALE + player.dir.y * 25},
                            sf::Color::Yellow);

        drawBuffer.drawLine(minimapTexture,
                            {player.pos.x / MINIMAP_SCALE - player.plane.x * 25, 
                             player.pos.y / MINIMAP_SCALE - player.plane.y * 25},
                            {player.pos.x / MINIMAP_SCALE + player.plane.x * 25,
                             player.pos.y / MINIMAP_SCALE + player.plane.y * 25},
                            sf::Color::Red);

        drawBuffer.drawLine(minimapTexture,
                            {player.pos.x / MINIMAP_SCALE, player.pos.y / MINIMAP_SCALE},
                            {player.pos.x / MINIMAP_SCALE + player.dir.x * 100 + player.plane.x * 100,
                             player.pos.y / MINIMAP_SCALE + player.dir.y * 100 + player.plane.y * 100},
                            sf::Color::Blue);

        drawBuffer.drawLine(minimapTexture,
                            {player.pos.x / MINIMAP_SCALE, player.pos.y / MINIMAP_SCALE},
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