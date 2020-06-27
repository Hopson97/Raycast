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

constexpr int MAP_SIZE = 10;
constexpr int TILE_SIZE = 64;
constexpr int WINDOW_WIDTH =  TILE_SIZE * MAP_SIZE * 2;
constexpr int WINDOW_HEIGHT = TILE_SIZE * MAP_SIZE;

constexpr int EYE_HEIGHT = 32;

float rads(float degs) {
    return degs * PI / 180.f;
}

struct Player {
    float x = 153;
    float y = 221;
    float angle = 0;

    // How much the player moves in the X/ Y direction when moving forwards or back
    float dx = 0;
    float dy = 0;

    sf::RectangleShape sprite;
    std::vector<sf::Vertex> line;

    Player()
    {
        sprite.setSize({10.0f, 10.0f});
        line.emplace_back(sf::Vector2f{0, 0}, sf::Color::Red);
        line.emplace_back(sf::Vector2f{0, 0}, sf::Color::Red);
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

        line[0].position = {x, y};
        line[1].position = {x + dx * 25, y + dy * 25};

        window.draw(line.data(), 2, sf::PrimitiveType::Lines);
    }
};

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
        return MAP[y * MAP_SIZE + x];
    }
};

struct Drawbuffer {
    std::vector<sf::Uint8> pixels;

    Drawbuffer()
        : pixels((WINDOW_WIDTH / 2) * WINDOW_HEIGHT * 4)
    {
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

        // Do ray cast stuff
        

        sprite.setTexture(&texture);
        texture.update(drawBuffer.pixels.data());
        window.draw(sprite);

        window.display();
    }
}