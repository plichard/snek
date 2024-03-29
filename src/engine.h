#pragma once
#include "stable_win32.hpp"
extern sf::RenderWindow *window;

struct Input {

    struct KeyPressed {
        sf::Keyboard::Key key;
    };

    struct KeyReleased {
        sf::Keyboard::Key key;
    };

    struct LostFocus {};

    using Event = std::variant<KeyPressed, KeyReleased, LostFocus>;

    std::vector<Event> events;

    template <typename T> void push(const T &value) { events.push_back(value); }

    void clear() { events.clear(); }
};

template <typename T> class LinearInterpolator {
private:
    float from = 0;
    float to = 0;
    float value = 0;

public:
    void update(float dt) {
        if (to > value) {
        }
    }

    void operator=(T v) {}
};

class Timer {
public:
    Timer(float timeout) : timeout_(timeout) {}
    bool operator()(float dt) {
        value_ += dt;
        if (value_ > timeout_) {
            running_ = false;
            return true;
        }
        return false;
    }

    void reset() {
        running_ = true;
        value_ = 0.0f;
    }

private:
    bool running_ = true;
    float value_ = 0.0f;
    float timeout_ = 0.0f;
};

struct Entity;
struct EntityID {
    using Index = size_t;
    using UID = size_t;

    Index index;
    UID uid;

    Entity *operator->();
    operator bool();
};

struct Entity {
    EntityID id;
    int hp;
};

template <typename T> class Array2D {
public:
    Array2D(int w = 0, int h = 0) : w_(w), h_(h), data(w * h) {}

    void resize(int w, int h) {
        w_ = w;
        h_ = h;
        data.resize(w * h);
    }

    T &operator()(int x, int y) { return data[x + y * w_]; }
    const T &operator()(int x, int y) const { return data[x + y * w_]; }
    int w() { return w_; }
    int h() { return h_; }

private:
    int w_ = 0, h_ = 0;
    std::vector<T> data;
};

void add_message(std::string fmt, ...);

namespace ui {

enum class Align { Left, Right, Center };

struct PushButton {
    int x;
    int y;
    sf::Text label;
    bool hover = false;
    float press_timer = 0.0f;
    static float PRESS_TIMER;
};

struct ToggleButton {
    int x;
    int y;
    sf::Text label;
    bool hover = false;
    bool active = false;
};

void label(int x, int y, std::string fmt, ...);
void labelc(int x, int y, const sf::Color &color, std::string fmt, ...);
bool push_button(int x, int y, const std::string &label,
                 ui::Align h_align = ui::Align::Left);
bool toggle_button(int x, int y, const std::string &label, bool *var = nullptr);
void display(sf::RenderWindow *window);

} // namespace ui
