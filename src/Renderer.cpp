#include "Renderer.h"

#include "Constants.h"
#include <cstring>

Renderer::Renderer()
    : pixelBuffer(WINDOW_WIDTH * WINDOW_HEIGHT * 4)
{
    line.emplace_back(sf::Vector2f{0, 0}, sf::Color::Red);
    line.emplace_back(sf::Vector2f{0, 0}, sf::Color::Red);

    texture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
    sprite.setSize({WINDOW_WIDTH, WINDOW_HEIGHT});
}

void Renderer::renderLine(sf::RenderTarget& target, const sf::Vector2f& begin,
                          const sf::Vector2f& end, sf::Color colour)
{
    line[0].position = begin;
    line[1].position = end;
    line[0].color = colour;
    line[1].color = colour;
    target.draw(line.data(), 2, sf::PrimitiveType::Lines);
}

void Renderer::clearPixels()
{
    std::memset(pixelBuffer.data(), 0, pixelBuffer.size());
}

void Renderer::setPixel(int x, int y, sf::Color colour)
{
    if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
        return;
    }
    sf::Uint8* ptr = &pixelBuffer.at((y * WINDOW_WIDTH + x) * 4);
    ptr[0] = colour.r;
    ptr[1] = colour.g;
    ptr[2] = colour.b;
    ptr[3] = 255;
}

void Renderer::render(sf::RenderTarget& target)
{
    sprite.setTexture(&texture);
    texture.update(pixelBuffer.data());
    target.draw(sprite);
}