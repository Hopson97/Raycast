#include "keyboard.h"

Keyboard::Keyboard()
{
    std::fill(m_keys.begin(), m_keys.end(), false);
}

void Keyboard::update(sf::Event e)
{
    switch (e.type) {
        case sf::Event::KeyPressed:
            m_keys[e.key.code] = true;
            break;

        default:
            break;
    }
}

bool Keyboard::isKeyDown(sf::Keyboard::Key key) const
{
    return m_keys[key];
}

void Keyboard::resetKeys()
{
    std::fill(m_keys.begin(), m_keys.end(), false);
}