#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <vector>

struct Renderer {
    std::vector<sf::Uint8> pixelBuffer;
    std::vector<sf::Vertex> line;

    sf::Texture texture;
    sf::RectangleShape sprite;

    Renderer();

    void renderLine(sf::RenderTarget& target, const sf::Vector2f& begin,
                    const sf::Vector2f& end, sf::Color colour);
    void clearPixels();
    void setPixel(int x, int y, sf::Color colour);

    void render(sf::RenderTarget& target);
};