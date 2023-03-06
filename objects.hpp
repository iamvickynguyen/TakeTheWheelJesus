#ifndef INCLUDE_OBJECTS_HPP
#define INCLUDE_OBJECTS_HPP

#include "./json.hpp"
#include <string>
#include <vector>

struct Point {
  int x, y, weight;
  Point() {}
  Point(const int x, const int y, const int weight)
      : x(x), y(y), weight(weight) {}
};

struct Snake {
  std::string id;
  int health;
  std::vector<Point *> body;
  Point *head;
  Point *tail;

  Snake() {}
  Snake(const nlohmann::json snake_data) {
    id = snake_data["id"];
    health = _strtoi(snake_data["health"]);

    for (auto &point : snake_data["body"]) {
      body.push_back(new Point(point["x"], point["y"], 0));
    }

    head = new Point(snake_data["head"]["x"], snake_data["head"]["y"], 0);
    tail = body[body.size() - 1];
  }

  int _strtoi(const std::string n) {
    int result = 0;
    for (auto &c : n)
      result = result * 10 + (c - '0');
    return result;
  }
};

class Board {
public:
  int height, width;
  std::vector<Snake *> other_snakes;
  Snake *mysnake;
  std::vector<Point *> foods;
  std::vector<Point *> hazards;

  Board(const nlohmann::json board) {
    height = board["height"];
    width = board["width"];
    mysnake = new Snake(board["you"]);

    for (auto snake_data : board["snakes"]) {
      Snake *snake = new Snake(snake_data);
      if (snake->id != mysnake->id) {
        other_snakes.push_back(snake);
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
