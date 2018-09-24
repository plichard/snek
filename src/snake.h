#pragma once
#include "stable.hpp"
#include "main.h"
#include "network.h"


struct SnakeGame {
    int gridSize = 16;
    sf::RectangleShape body_shape;
    sf::RectangleShape food_shape;

    sf::Font hand_font;
    sf::Text pause_text;

    Network network;

    struct Food {
        sf::Vector2i p;
        Food(int x, int y);
    };

    struct Cell {
        Food *food = nullptr;
        void reset();
    };

    enum class Direction { Up, Right, Down, Left };
    static constexpr Direction next_right[4] = {
        Direction::Right, Direction::Down, Direction::Left, Direction::Up};

    static constexpr Direction next_left[4] = {
        Direction::Left, Direction::Up, Direction::Right, Direction::Down};

    enum class GameStatus {
        MainMenu,
        SinglePlayer,
        HostLobby,
        GuestLobby,
        HostMultiPlayer,
        GuestMultiPlayer
    };
    GameStatus game_status;

    struct Player {
        sf::Color color;
        std::vector<sf::Vector2i> body;
        Direction dir;
        std::list<sf::Keyboard::Key> input_buffer;
        bool use_ai = false;
        bool boost = false;
        bool dead = false;

        int moveDelay = 2;
        int moveCounter = 0;

        u32 spawnX;
        u32 spawnY;
        Direction spawn_dir;

        using ID = uint8_t;
        ID id;
    };

    struct Message {
        enum class Type { HeartBeat, RequestJoin, SetNewPlayer, PlayerJoined };

        struct HeartBeat {};

        struct RequestJoin {};

        struct SetNewPlayer {
            Player::ID new_id;
        };

        struct PlayerJoined {
            Player::ID new_id;
            sf::Color color;
        };

        Type header;

        std::variant<HeartBeat, RequestJoin, SetNewPlayer, PlayerJoined> body;
    };

    Player::ID local_id = 0;
    Player::ID unique_player_id = 1;

    std::unordered_map<Player::ID, Player> players;

    Array2D<Cell> world_map;

    std::vector<Food *> food;

	u32 gridRows = 30;
	u32 gridCols = 30;

    int foodRegrow = 20;
    int foodRegrowCount = 0;

    int foodGrowth = 1;
	u32 initialSize = 3;

    bool paused = false;
    bool hasMovedAfterDirectionChange = false;

    //    std::vector<sf::Vector2i> body;

    Direction set_dir(Player &player, Direction d);

    //    Direction dir;
    Player::ID add_player();
    void add_player(Player::ID id);
    void recompute_spawn_points();
    void spawn(Player &player);

    // leave food behind where your body was
    void decompose(Player &player);
    void add_food(int x, int y);
    void remove_food(int x, int y);
    void reset_map();

    bool on_player(const Player &p1, const Player &p2);
    bool on_player(const Player &p1, const Player &p2, int x, int y);

    void init();

    void input_key(sf::Keyboard::Key k, bool down);
    void update(float dt);

    sf::Color get_random_color();

    void main_menu();
    void host_lobby();
    void guest_lobby();
    void single_player();
};