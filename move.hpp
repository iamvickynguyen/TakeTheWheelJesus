#ifndef INCLUDE_MOVE_HPP
#define INCLUDE_MOVE_HPP

#include "./objects.hpp"

constexpr double MIN = std::numeric_limits<double>::min();
constexpr double MAX = std::numeric_limits<double>::max();

namespace cppssss {

/* Things to consider to calculate the score:
 * 	1. Length: Are we at least the second longest?
 * 	2. Food: How many foods we can reach before the other snakes?
 * 	3. Snakes: How many snakes are still alive?
 * 	4. Space: How much space can we move? (i.e., How many squares can we
 * 																					reach
 * before other snakes?) Some properties are more important than others. Let's
 * give them some weights Sum(weights) = 1 Need some ways to normalize the
 * values above So our formula can be like this:
 * 		score = 20 * length
 * 				+ 30 * food * (health > 50) + 50 * food * (health <= 50)
 * 				- 10 * snakes
 * 				+ 40 * space
 * 				+ 50 * (10 - distance to tail) * (health > 50) + 30 * (10 - distance to tail) * (health <= 50)
 * `Snakes` is 0.1 because hardly any snakes
 * will die with small number of steps
 */

/* =================================================
 * ================== HEURISTICS ===================
 * ================================================= */

// Return percentage

double length_score(const std::deque<Snake *> &snakes) {
  const int mysize = snakes[0]->body.size();
  int count = 0;
  for (int i = 1; i < snakes.size(); ++i) {
    count += (mysize >= snakes[i]->body.size());
  }
  return (count + 0.0) / (snakes.size() - 1);
}

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

/* =================================================
 * ================== MINIMAX ======================
 * ================================================= */

/* The code below is horrendous
 * TODO: rewrite
 */

// We always maximize the score when it's our turn (i.e, snake_index=0)
std::pair<double, char> minimax(GameState &state, int snake_index,
                                char direction, double alpha, double beta,
                                int depth, int max_depth) {
  if (depth == max_depth)
    return {calculate_score(state, snake_index), direction};

  if (snake_index == 0) {
    double curmax = MIN;
    char curmove = direction;

    Snake *snake = state.snakes[snake_index];
    std::tuple<int, int, char> moves[4] = {
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

        int new_index = (snake_index + 1) % (state.snakes.size());
        auto [value, new_dir] =
            minimax(state, new_index, dir, alpha, beta, depth + 1, max_depth);

        if (value > curmax) {
          curmax = value;
          curmove = new_dir;
        }

        // restore
        snake->body.pop_front();
        snake->body.push_back(p);
        snake->head->y -= y_offset;
        snake->head->x += x_offset;

        alpha = std::max(alpha, curmax);
        if (beta <= alpha)
          break;
      }

      return {curmax, curmove};
    }
  }

  // TODO: rewrite this, too much duplicate code
  else {
    double curmin = MAX;
    char curmove = direction;

    Snake *snake = state.snakes[snake_index];
    std::tuple<int, int, char> moves[4] = {
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

        int new_index = (snake_index + 1) % (state.snakes.size());
        auto [value, new_dir] =
            minimax(state, new_index, dir, alpha, beta, depth + 1, max_depth);

        if (value < curmin) {
          curmin = value;
          curmove = new_dir;
        }

        // restore
        snake->body.pop_front();
        snake->body.push_back(p);
        snake->head->y -= y_offset;
        snake->head->x += x_offset;

        beta = std::min(beta, curmin);
        if (beta <= alpha)
          break;
      }

      return {curmin, curmove};
    }
  }

  return {MIN, 'u'}; // dummy
}

std::string move(const nlohmann::json data) {
  GameState state(data);

	// last survivor
	// TODO: bug
	if (state.snakes.size() == 0)
		return "up";

  double curmax = MIN;
  char curmove;
  double alpha = MIN, beta = MAX;

  Snake *snake = state.snakes[0];
  std::tuple<int, int, char> moves[4] = {
      {-1, 0, 'd'}, {1, 0, 'u'}, {0, -1, 'l'}, {0, 1, 'r'}};
  for (auto &[y_offset, x_offset, dir] : moves) {
    int y = state.snakes[0]->head->y + y_offset;
    int x = state.snakes[0]->head->x + x_offset;
    if (y >= 0 && y < state.height && x >= 0 && x < state.width) {
      snake->head->y = y;
      snake->head->x = x;

      Point *p = snake->body.back();
      snake->body.pop_back();
      snake->body.push_front(snake->head);

      auto [value, new_dir] = minimax(state, 1, dir, alpha, beta, 1, 20);

      if (value > curmax) {
        curmax = value;
        curmove = new_dir;
      }

      // restore
      snake->body.pop_front();
      snake->body.push_back(p);
      snake->head->y -= y_offset;
      snake->head->x += x_offset;

      alpha = std::max(alpha, curmax);
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
