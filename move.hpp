#ifndef INCLUDE_MOVE_HPP
#define INCLUDE_MOVE_HPP

#include "./objects.hpp"

constexpr double MIN = std::numeric_limits<double>::min();
constexpr double MAX = std::numeric_limits<double>::max();

namespace cppssss {

double calculate_score(GameState &state, const int snake_index) {
  Snake *snake = state.snakes[snake_index];

  if (!(snake->is_alive(state.height, state.width)))
    return MIN;

  // TODO: some better heuristics
  // Calculate best positions to food for now. Maybe reach to most food before
  // others?
  double f = (double)rand() / RAND_MAX;
  return f * 50.0;
}

// We always maximize the score when it's our turn (i.e, snake_index=0)
std::pair<double, char> minimax(GameState &state, int snake_index,
                                char direction, double alpha, double beta,
                                int depth, int max_depth) {
  if (depth == max_depth)
    return {calculate_score(state, i), direction};

  if (snake_index == 0) {
    double curmax = MIN;
    char curmove = move;

    Snake *snake = state.snakes[snake_index];
    std::tuple<int, int, char> moves = {
        {-1, 0, 'd'}, {1, 0, 'u'}, {0, -1, 'l'}, {0, 1, 'r'}};
    for (auto &[y_offset, x_offset, dir] : moves) {
      int y = snake->head->y + y_offset;
      int x = snake->head->x + x_offset;
      if (y >= 0 && y < state.height && x >= 0 && x < state.width) {
        snake->head->y = y;
        snake->head->x = x;

        Point *p = snake->body.back();
        snake->body.pop_back();
        snake->body.push_front(snake->head);

        int new_index = (i + 1) % (state.snakes.size());
        double value =
            minimax(state, new_index, dir, alpha, beta, depth + 1, max_depth);

        if (value > curmax) {
          curmax = value;
          curmove = move;
        }

        // restore
        snake->body.pop_fron();
        snake->body.push_back(p);
        snake->head->y -= y_offset;
        snake->head->x += x_offset;

        alpha = max(alpha, curmax);
        if (beta <= alpha)
          break;
      }

      return {curmax, curmove};
    }
  }

  // TODO: rewrite this, too much duplicate code
  else {
    double curmin = MAX;
    char curmove = move;

    Snake *snake = state.snakes[snake_index];
    std::tuple<int, int, char> moves = {
        {-1, 0, 'd'}, {1, 0, 'u'}, {0, -1, 'l'}, {0, 1, 'r'}};
    for (auto &[y_offset, x_offset, dir] : moves) {
      int y = snake->head->y + y_offset;
      int x = snake->head->x + x_offset;
      if (y >= 0 && y < state.height && x >= 0 && x < state.width) {
        snake->head->y = y;
        snake->head->x = x;

        Point *p = snake->body.back();
        snake->body.pop_back();
        snake->body.push_front(snake->head);

        int new_index = (i + 1) % (state.snakes.size());
        double value =
            minimax(state, new_index, dir, alpha, beta, depth + 1, max_depth);

        if (value < curmin) {
          curmin = value;
          curmove = move;
        }

        // restore
        snake->body.pop_fron();
        snake->body.push_back(p);
        snake->head->y -= y_offset;
        snake->head->x += x_offset;

        beta = min(beta, curmin);
        if (beta <= alpha)
          break;
      }

      return {curmin, curmove};
    }
  }
}

std::string move(const nlohmann::json data) {
  GameState state(data);

  double curmax = MIN;
  char curmove = move;

  std::tuple<int, int, char> moves = {
      {-1, 0, 'd'}, {1, 0, 'u'}, {0, -1, 'l'}, {0, 1, 'r'}};
  for (auto &[y_offset, x_offset, dir] : moves) {
    int y = state->snakes[0]->head->y + y_offset;
    int x = state->snakes[0]->snake->head->x + x_offset;
    if (y >= 0 && y < state.height && x >= 0 && x < state.width) {
      snake->head->y = y;
      snake->head->x = x;

      Point *p = snake->body.back();
      snake->body.pop_back();
      snake->body.push_front(snake->head);

      int new_index = (i + 1) % (state.snakes.size());
      double value = minimax(state, 1, dir, alpha, beta, depth + 1, 20);

      if (value > curmax) {
        curmax = value;
        curmove = move;
      }

      // restore
      snake->body.pop_fron();
      snake->body.push_back(p);
      snake->head->y -= y_offset;
      snake->head->x += x_offset;

      alpha = max(alpha, curmax);
      if (beta <= alpha)
        break;
    }
  }

  if (curmove == 'u')
    return "up";
  if (curmove == 'd')
    return "down";
  if (curmove == 'l')
    return "left";
  return "right";
}
} // namespace cppssss

#endif // INCLUDE_MOVE_HPP
