#ifndef INCLUDE_OBJECTS_HPP
#define INCLUDE_OBJECTS_HPP

#include "./json.hpp"

enum Objects {
  EMPTY = 0,
  FOOD = 1,
  HAZARD = 2,
  MYHEAD = 10,
  MYBODY = 11,
  MYTAIL = 12,
  OTHERHEAD = 20,
  OTHERBODY = 21,
  OTHERTAIL = 22
};

struct Point {
  int x, y, weight;
  Point() {}
  Point(const int x, const int y, const int weight)
      : x(x), y(y), weight(weight) {}
};

struct Snake {
  std::string id;
  int health;
  std::deque<Point *> body; // head move, tail cuts off
  Point *head;
  Point *tail;
  bool is_alive;

  Snake() {}
  Snake(const nlohmann::json snake_data) : is_alive(true) {
    id = snake_data["id"];
    health = snake_data["health"];

    for (auto &point : snake_data["body"]) {
      body.push_back(new Point(point["x"], point["y"], 0));
    }

    head = body[0];
    tail = body[body.size() - 1];
  }
};

class GameState {
public:
  int height, width;
  std::vector<Snake *> snakes; // 0 index is my snake
  std::vector<Point *> foods;
  std::vector<Point *> hazards;
  std::vector<std::vector<Objects>> grid;

  GameState(const nlohmann::json all_data) {
    nlohmann::json board = all_data["board"];
    nlohmann::json game = all_data["game"];

    height = board["height"];
    width = board["width"];
    grid.assign(height, std::vector<Objects>(width, EMPTY));

    // Allocate max possible to avoid allocation
    foods.resize(height * width);
    hazards.resize(height * width);

    update(all_data);
  }

  void update(nlohmann::json all_data) {
    nlohmann::json board = all_data["board"];
    nlohmann::json game = all_data["game"];

    reset_grid();
    Snake *mysnake = new Snake(all_data["you"]);

    snakes.clear(); // Hopefully doesn't reallocate
    snakes.push_back(mysnake);

    // update my snake
    for (auto &part : mysnake->body) {
      grid[part->y][part->x] = MYBODY;
    }
    grid[mysnake->head->y][mysnake->head->x] = MYHEAD;
    grid[mysnake->tail->y][mysnake->tail->x] = MYTAIL;

    for (auto snake_data : board["snakes"]) {
      Snake *snake = new Snake(snake_data);
      if (snake->id != mysnake->id) {
        snakes.push_back(snake);

        // update other snake
        for (auto &part : snake->body) {
          grid[part->y][part->x] = OTHERBODY;
        }
        grid[snake->head->y][snake->head->x] = OTHERHEAD;
        grid[snake->tail->y][snake->tail->x] = OTHERTAIL;
      }
    }

    foods.clear(); // Hopefully doesn't reallocate
    for (auto &point : board["food"]) {
      foods.push_back(new Point(point["x"], point["y"], 0));
      grid[point["y"]][point["x"]] = FOOD; // update grid
    }

    hazards.clear(); // Hopefully doesn't reallocate
    for (auto &point : board["hazards"]) {
      hazards.push_back(new Point(point["x"], point["y"], 0));
      grid[point["y"]][point["x"]] = HAZARD; // upate grid
    }
  }

  void reset_grid() {
    for (int i = 0; i < grid.size(); i++) {
      for (int j = 0; j < grid[i].size(); j++) {
        grid[i][j] = EMPTY;
      }
    }
  }

  bool in_bound(const int y, const int x) {
    return (y >= 0 && y < height && x >= 0 && x < width);
  }

  // y and x are in bound
  bool has_food(const int y, const int x) { return (grid[y][x] == FOOD); }

  // y and x are in bound
  // If the square contains a tail of some snake, it's also safe
  bool is_safe(const int y, const int x) {
    return (grid[y][x] == EMPTY || grid[y][x] == FOOD ||
            grid[y][x] == OTHERTAIL || grid[y][x] == MYTAIL);
  }
};

#endif // INCLUDE_OBJECTS_HPP
