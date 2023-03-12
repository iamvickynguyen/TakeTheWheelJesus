#ifndef INCLUDE_MOVE_HPP
#define INCLUDE_MOVE_HPP

#include "./objects.hpp"

constexpr int MIN = std::numeric_limits<int>::min();
constexpr int MAX = std::numeric_limits<int>::max();

namespace cppssss {

/* Things to consider to calculate the score:
 * 	1. Length: Are we at least the second longest?
 * 	2. Food: How many foods we can reach before the other snakes?
 * 	3. Snakes: How many snakes are still alive?
 * 	4. Space: How much space can we move? (i.e., How many squares can we
 * reach before other snakes?) Some properties are more important than others.
 * Let's give them some weights. So our formula can be like this:
 * score = 20 * length
 * 				+ 30 * food * (health > 50) + 50 * food * (health <= 50)
 * 				- 10 * snakes
 * 				+ 40 * space
 * 				+ 50 * (10 - distance to tail) * (health > 50) + 30 * (10 - distance to tail) * (health <= 50)
 * `Snakes` is 0.1 because hardly any
 * snakes will die with small number of steps
 */

/* =================================================
 * ================== HEURISTICS ===================
 * ================================================= */

int length_score(GameState* state) {
	std::vector<Snake*> snakes = state->snakes;
  const int mysize = snakes[0]->body.size();
  int count = 0;
  for (int i = 1; i < snakes.size(); ++i) {
    count += (mysize >= snakes[i]->body.size());
  }
  return (count * 100) / (snakes.size() - 1);
}

int food_score(GameState* state) {
	int count = 0;
	Snake* mysnake = state->snakes[0];
	int myhead_y = mysnake->head.y;
	int myhead_x = mysnake->head.x;
	for (auto &food: state->foods) {
		int mydist = abs(food.y - myhead_y) + abs(food.x - myhead_x);
		for (int i = 1; i < state->snakes.size(); ++i) {
			Snake* snake = state->snakes[i];
			int y = snake->head.y, x = snake->head.x;
			int dist = abs(food.y - y) + abs(food.x - x);
			count += (mydist > dist);
		}
	}
	return (count * 100) / (state->foods.size());
}

int snake_score(GameState* state) {
	std::vector<std::vector<Objects>> grid = state->grid;
	int survivors = 0;
	for (int i = 0; i < grid.size(); ++i) {
		for (int j = 0; j < grid[0].size(); ++j) {
			if (grid[i][j] == OTHERHEAD) ++survivors;
		}
	}
	return (100 * survivors) / state->snakes.size();
}

int space_score(GameState* state) {
	int count = 0;
	std::vector<std::vector<Objects>> grid = state->grid;
	Snake* mysnake = state->snakes[0];
	const int myhead_y = mysnake->head.y;
	const int myhead_x = mysnake->head.x;
	for (int i = 0; i < state->height; ++i) {
		for (int j = 0; j < state->width; ++j) {
			if (grid[i][j] == EMPTY) {
				int mydist = abs(i - myhead_y) + abs(j - myhead_x);
				int mindist = MAX;
				for (int k = 1; k < state->snakes.size(); ++k) {
					Snake* snake = state->snakes[k];
					int y = snake->head.y, x = snake->head.x;
					int dist = abs(i - y) + abs(j - x);
					mindist = std::min(mindist, dist);
				}

				count += (mydist > mindist);
			}
		}
	}

	return (100 * count) / (state->height * state->width);
}

int dist_to_tail_score(GameState* state) {
	Snake* snake = state->snakes[0];
	int dist = abs(snake->tail.y - snake->head.y) + abs(snake->tail.x - snake->head.x);
	return (100 * dist) / (state->height + state->width);
}
	

int calculate_score(GameState* state, const int snake_index) {
  Snake *snake = state->snakes[snake_index];

	// if it's our turn
	if (snake_index == 0 && state->grid[snake->head.y][snake->head.x] != MYHEAD)
			return MIN;

	// if we're the last one
	bool other_survive = false;
	if (snake_index == 0) {
		for (auto &row: state->grid) {
			for (auto &cell: row) {
				other_survive |= (cell == OTHERHEAD);
			}
		}

		if (!other_survive) return MAX;
	}

	int score_length = length_score(state);
	
	int score_food = food_score(state);
	
	int score_snake = snake_score(state);
	
	int score_space = space_score(state);
	
	int dist = dist_to_tail_score(state);

	

	return 20 * score_length
		+ 30 * score_food * (snake->health > 50) + 50 * score_food * (snake->health <= 50)
		- 10 * score_snake
		+ 40 * score_space
		+ 50 * (10 - dist) * (snake->health > 50) + 30 * (10 - dist) * (snake->health <= 50);
}

/* =================================================
 * ================== MINIMAX ======================
 * ================================================= */

/* The code below is horrendous
 * TODO: rewrite
 */

// We always maximize the score when it's our turn (i.e, snake_index=0)
std::pair<int, char> minimax(GameState *state, int snake_index,
                                char direction, int alpha, int beta,
                                int depth, int max_depth) {
	


  if (depth == max_depth)
    return {calculate_score(state, snake_index), direction};

  if (snake_index == 0) {
		// combine and copy to local grids
		GameState::vv orig_grid = state->grid;
		state->combine_and_copy_grids();

    int curmax = MIN;
    char curmove = direction;

    Snake *snake = state->snakes[snake_index];
    std::tuple<int, int, char> moves[4] = {
        {-1, 0, 'd'}, {1, 0, 'u'}, {0, -1, 'l'}, {0, 1, 'r'}};
    for (auto &[y_offset, x_offset, dir] : moves) {
      int y = snake->head.y + y_offset;
      int x = snake->head.x + x_offset;

			if (state->in_bound(y, x) && state->is_safe(y, x)) {
				// store current state
				Point tail = snake->tail;
				GameState::vv cur_grid = state->grids[snake_index];
				
				bool did_tail_move = state->snake_move(snake_index, y, x, true);
				
				int new_index = (snake_index + 1) % (state->snakes.size());
				auto [value, new_dir] =
					minimax(state, new_index, dir, alpha, beta, depth + 1, max_depth);

				if (value > curmax) {
					curmax = value;
					curmove = new_dir;
				}

				// restore
				state->restore_snake_move(snake_index, tail, cur_grid, did_tail_move);
        
				alpha = std::max(alpha, curmax);
        if (beta <= alpha) break;
    	}
		}
		
		// restore original grid and all grids
		state->restore_grids(orig_grid);
    return {curmax, curmove};
  }

  // TODO: rewrite this, too much duplicate code
  else {
    int curmin = MAX;
    char curmove = direction;

    Snake *snake = state->snakes[snake_index];
    std::tuple<int, int, char> moves[4] = {
        {-1, 0, 'd'}, {1, 0, 'u'}, {0, -1, 'l'}, {0, 1, 'r'}};
    for (auto &[y_offset, x_offset, dir] : moves) {
      int y = snake->head.y + y_offset;
      int x = snake->head.x + x_offset;

			if (state->in_bound(y, x) && state->is_safe(y, x)) {
				// store current state
				Point tail = snake->tail;
				GameState::vv cur_grid = state->grids[snake_index];
				
				bool did_tail_move = state->snake_move(snake_index, y, x, true);
				
				int new_index = (snake_index + 1) % (state->snakes.size());
				auto [value, new_dir] =
					minimax(state, new_index, dir, alpha, beta, depth + 1, max_depth);

        if (value < curmin) {
          curmin = value;
          curmove = new_dir;
        }

				// restore
				state->restore_snake_move(snake_index, tail, cur_grid, did_tail_move);
      
        beta = std::min(beta, curmin);
        if (beta <= alpha)
          break;
      }
    }
    return {curmin, curmove};
  }

	
  return {MIN, 'u'}; // dummy
}

std::string move(GameState *state) {
  // last survivor
  // TODO: bug
  if (state->snakes.size() == 0)
    return "up";

  int curmax = MIN;
  char curmove;
  int alpha = MIN, beta = MAX;

  Snake *snake = state->snakes[0];
  std::tuple<int, int, char> moves[4] = {
      {-1, 0, 'd'}, {1, 0, 'u'}, {0, -1, 'l'}, {0, 1, 'r'}};
  for (auto &[y_offset, x_offset, dir] : moves) {
		

    int y = snake->head.y + y_offset;
    int x = snake->head.x + x_offset;

		if (state->in_bound(y, x) && state->is_safe(y, x)) {
				// store current state
				Point tail = snake->tail;
				GameState::vv cur_grid = state->grids[0];
				
				bool did_tail_move = state->snake_move(0, y, x, true);
				
      	auto [value, new_dir] = minimax(state, 1, dir, alpha, beta, 1, 3);

				if (value > curmax) {
					curmax = value;
					curmove = new_dir;
				}

				// restore
				state->restore_snake_move(0, tail, cur_grid, did_tail_move);
        
				alpha = std::max(alpha, curmax);
    		if (beta <= alpha) break;		
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
