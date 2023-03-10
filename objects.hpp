#ifndef INCLUDE_OBJECTS_HPP
#define INCLUDE_OBJECTS_HPP

#include "./json.hpp"

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

  Snake() {}
  Snake(const nlohmann::json snake_data) {
    id = snake_data["id"];
    health = snake_data["health"];

    for (auto &point : snake_data["body"]) {
      body.push_back(new Point(point["x"], point["y"], 0));
    }

    head = new Point(snake_data["head"]["x"], snake_data["head"]["y"], 0);
    tail = body[body.size() - 1];
  }

  bool is_alive(const int h, const int w) {
    return (head->x >= 0 && head->x < w && head->y >= 0 && head->y < h);
  }
};

class GameState {
public:
  int height, width;
  std::vector<Snake *> snakes; // 0 index is my snake
  std::vector<Point *> foods;
  std::vector<Point *> hazards;

  GameState(const nlohmann::json board) {
    height = board["height"];
    width = board["width"];
    Snake *mysnake = new Snake(board["you"]);

    snakes.push_back(mysnake);

    for (auto snake_data : board["snakes"]) {
      Snake *snake = new Snake(snake_data);
      if (snake->id != mysnake->id) {
        snakes.push_back(snake);
      }
    }

    for (auto &point : board["food"]) {
      foods.push_back(new Point(point["x"], point["y"], 0));
    }

    for (auto &point : board["hazards"]) {
      hazards.push_back(new Point(point["x"], point["y"], 0));
    }
  }
};

#endif // INCLUDE_OBJECTS_HPP
