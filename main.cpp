#include "main.h"
#include "network.h"
#include "snake.h"
#include <stable.hpp>

sf::RenderWindow *window = nullptr;

namespace {

sf::Font message_font;
sf::Text message_text;

uint message_character_size = 14;
uint max_message = 10;
bool console_input_focused = false;
std::string console_input_text;
sf::RectangleShape console_input_rectangle;
int mouseX = 0;
int mouseY = 0;
bool leftPressed = false;
bool leftReleased = false;
float dt = 0.0f;

std::mutex message_mutex;

struct Message {
    static float SHOW_DELAY;
    static float FADE_DELAY;

    std::string text;
    float show_delay = 5.0f;
    Message(std::string text) : text(std::move(text)), show_delay(SHOW_DELAY) {}
};
float Message::SHOW_DELAY = 5.0f;
float Message::FADE_DELAY = 1.0f;

std::list<Message> messages;

} // namespace

namespace ui {

float PushButton::PRESS_TIMER = 0.2f;

std::unordered_map<std::string, PushButton> push_button_map;
std::unordered_map<std::string, ToggleButton> toggle_button_map;
sf::Text label_text;

bool push_button(int x, int y, const std::string &label) {

    PushButton *button;
    if (auto it = push_button_map.find(label); it != push_button_map.end()) {
        button = &it->second;
        button->x = x;
        button->y = y;
    } else {
        button = &push_button_map.insert({label, {}}).first->second;
        button->x = x;
        button->y = y;
        button->label.setString(label);
        button->label.setFont(message_font);
        button->label.setCharacterSize(30);
    }

    assert(button);
    button->label.setPosition(x, y);

    const auto BB = button->label.getGlobalBounds();

    if (BB.contains(mouseX, mouseY)) {
        button->label.setFillColor({100, 255, 100, 255});
        button->hover = true;
    } else {
        button->label.setFillColor({255, 255, 255, 255});
        button->hover = false;
    }

    bool pressed = false;

    if (button->hover && leftPressed) {
        button->press_timer = PushButton::PRESS_TIMER;
        pressed = true;
    }

    if (button->press_timer > 0) {
        button->label.setOutlineColor(
            {0, 255, 0,
             static_cast<sf::Uint8>(255 * button->press_timer /
                                    button->PRESS_TIMER)});
        button->label.setOutlineThickness(2);
        button->press_timer -= dt;
    } else {
        button->label.setOutlineColor({0, 0, 0, 0});
        button->label.setOutlineThickness(0);
    }

    window->draw(button->label);

    return pressed;
}

bool toggle_button(int x, int y, const std::string &label) {

    ToggleButton *button = nullptr;
    if (auto it = toggle_button_map.find(label);
        it != toggle_button_map.end()) {
        button = &it->second;
        button->x = x;
        button->y = y;
    } else {
        button = &toggle_button_map.insert({label, {}}).first->second;
        button->x = x;
        button->y = y;
        button->label.setString(label);
        button->label.setFont(message_font);
        button->label.setCharacterSize(30);
    }

    assert(button);
    button->label.setPosition(x, y);

    const auto BB = button->label.getGlobalBounds();

    if (BB.contains(mouseX, mouseY)) {
        button->hover = true;
        button->label.setOutlineColor({255, 255, 255, 255});
        button->label.setOutlineThickness(1);
    } else {
        button->hover = false;
        button->label.setOutlineColor({0, 0, 0, 0});
        button->label.setOutlineThickness(0);
    }

    if (button->hover && leftPressed) {
        button->active = !button->active;
    }

    if (button->active) {
        button->label.setFillColor({100, 255, 100, 255});
    } else {
        button->label.setFillColor({255, 100, 100, 255});
    }

    window->draw(button->label);

    return button->active;
}

void label(int x, int y, const std::string &fmt, ...) {
    static char buffer[255];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt.c_str(), args);
    va_end(args);
    label_text.setFillColor({255, 255, 255, 255});
    label_text.setCharacterSize(message_character_size);
    label_text.setString(buffer);
    label_text.setPosition(x, y);
    window->draw(label_text);
}

} // namespace ui

void draw_console_input() {
    float dy = message_font.getLineSpacing(message_character_size) + 2 * 2;
    console_input_rectangle.setFillColor({255, 255, 255, 64});
    console_input_rectangle.setOutlineColor({255, 255, 255, 255});
    console_input_rectangle.setSize({400, dy});
    console_input_rectangle.setPosition(5, window->getSize().y - dy);
    window->draw(console_input_rectangle);
}

void draw_messages() {
    std::lock_guard scope_guard(message_mutex);
    int y = 0;
    message_text.setCharacterSize(message_character_size);
    bool first_message = true;

    for (auto &msg : messages) {
        msg.show_delay -= dt;

        if (msg.show_delay > 0.0f) {
            message_text.setFillColor({255, 255, 255, 255});
        } else if (msg.show_delay < 0.0f &&
                   msg.show_delay > -Message::FADE_DELAY) {
            message_text.setFillColor(
                {255, 255, 255,
                 static_cast<sf::Uint8>(255 + 255 * msg.show_delay)});
        } else {
            continue;
        }
        message_text.setString(msg.text);
        if (first_message) {
            y = window->getSize().y -
                console_input_rectangle.getLocalBounds().height -
                message_text.getLocalBounds().height - 7;
            first_message = false;
        } else {
            y -= message_text.getLocalBounds().height + 5;
        }
        message_text.setPosition(5, y);
        window->draw(message_text);
    }
}

// Entity *entity;
// Entity *EntityID::operator->() {
//    if (entity->id.uid == uid) {
//        return entity;
//    }
//    return nullptr;
//}

// EntityID::operator bool() { return entity->id.uid == uid; }

#include <utility>

int main() {
    SnakeGame snake;

    window = new sf::RenderWindow({1900, 1000}, "Games");
    window->setVerticalSyncEnabled(true);

    message_font.loadFromFile("resources/fonts/Inconsolata-Regular.ttf");
    message_text.setFont(message_font);
    ui::label_text.setFont(message_font);

    snake.init();

    auto tp1 = std::chrono::high_resolution_clock::now();
    auto tp2 = std::chrono::high_resolution_clock::now();

    while (window->isOpen()) {
        sf::Event ev;
        while (window->pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                window->close();
            }

            else if (ev.type == sf::Event::MouseMoved) {
                mouseX = ev.mouseMove.x;
                mouseY = ev.mouseMove.y;
            }

            else if (ev.type == sf::Event::MouseButtonPressed) {
                if (ev.mouseButton.button == sf::Mouse::Left) {
                    leftPressed = true;
                }
            }

            else if (ev.type == sf::Event::MouseButtonReleased) {
                if (ev.mouseButton.button == sf::Mouse::Left) {
                    leftReleased = true;
                }
            }

            else if (ev.type == sf::Event::TextEntered) {
                // printf("key = %d\n", ev.text.unicode);
                auto code = ev.text.unicode;

                if (code == 13) {
                    console_input_focused = !console_input_focused;
                }

            } else {
                if (ev.type == sf::Event::KeyPressed) {
                    if (ev.key.code == sf::Keyboard::Q) {
                        window->close();
                    } else {
                        snake.input_key(ev.key.code, true);
                    }
                } else if (ev.type == sf::Event::KeyReleased) {
                    if (ev.key.code == sf::Keyboard::Q) {
                        //                        window->close();
                    } else {
                        snake.input_key(ev.key.code, false);
                    }
                }
            }
        }
        window->clear();

        tp2 = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration<float>(tp2 - tp1).count();
        tp1 = tp2;
        snake.update(dt);

        draw_messages();

        if (console_input_focused)
            draw_console_input();

        window->display();

        leftReleased = false;
        leftPressed = false;
    }

    return 0;
}

void add_message(std::string fmt, ...) {
    static char buffer[255];
    std::lock_guard scope_guard(message_mutex);
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    //    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    oss << std::put_time(&tm, "[%H:%M:%S] ");
    auto time_str = oss.str();
    fmt = time_str + fmt;
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt.c_str(), args);
    va_end(args);
    messages.emplace_front(buffer);
    if (messages.size() > max_message) {
        messages.pop_back();
    }
}
