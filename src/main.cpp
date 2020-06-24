#include "keyboard.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <cstring>
#include <iostream>

using Buffer = std::vector<sf::Uint8>;

constexpr unsigned WIDTH = 640;
constexpr unsigned HEIGHT = 480;
constexpr unsigned SIZE = WIDTH * HEIGHT;

void clearBuffer(Buffer& buffer)
{
    std::memset(buffer.data(), 0, buffer.size());
}

void setPixel(Buffer& buffer, unsigned x, unsigned y, sf::Uint8 r, sf::Uint8 g,
              sf::Uint8 b)
{
    auto* ptr = &buffer[(y * WIDTH + x) * 4];
    ptr[0] = r;
    ptr[1] = g;
    ptr[2] = b;
    ptr[3] = 255;
}

int main()
{
    sf::RenderWindow window({WIDTH, HEIGHT}, "Raycast Test");
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    Buffer buffer(WIDTH * HEIGHT * 4);

    sf::Texture texture;
    texture.create(WIDTH, HEIGHT);
    texture.update(buffer.data());

    sf::RectangleShape sprite;
    sprite.setSize({(float)WIDTH, (float)HEIGHT});

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

        clearBuffer(buffer);
        static unsigned x = 0;
        for (unsigned i = 0; i < 100; i++) {
            for (unsigned j = 0; j < 100; j++) {
                setPixel(buffer, j + x, i, 200, 100, 255);
            }
        }

        texture.update(buffer.data());
        sprite.setTexture(&texture);

        window.clear();

        window.draw(sprite);

        window.display();
    }
}