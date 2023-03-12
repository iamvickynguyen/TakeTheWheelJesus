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
  std::deque<Point> body; // head move, tail cuts off
  Point head;
  Point tail;
  bool is_alive;

  Snake() {}
  Snake(const nlohmann::json snake_data) : is_alive(true) {
    id = snake_data["id"];
    health = snake_data["health"];

    for (auto &point : snake_data["body"]) {
      body.push_back(Point(point["x"], point["y"], 0));
    }

    head = body[0];
    tail = body[body.size() - 1];
  }
};

class GameState {
public:
	using vv = std::vector<std::vector<Objects>>;
	using vvv = std::vector<vv>;
	
  int height, width;
  std::vector<Snake *> snakes; // 0 index is my snake
  std::vector<Point> foods;
  std::vector<Point> hazards;
  std::vector<std::vector<Objects>> grid;
	vvv grids;	

  GameState(const nlohmann::json all_data) {
    nlohmann::json board = all_data["board"];
    nlohmann::json game = all_data["game"];

    height = board["height"];
    width = board["width"];
    grid.assign(height, std::vector<Objects>(width, EMPTY));

    // Allocate max possible to avoid allocation
    foods.reserve(height * width);
    hazards.reserve(height * width);

    size_t max_snakes = board["snakes"].size();
    snakes.reserve(max_snakes);

		// init grids
		std::cout << "GRID INIT: " << max_snakes << ", height=" << height << ", width=" << width << '\n';
		grids.reserve(max_snakes);

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
      grid[part.y][part.x] = MYBODY;
    }
    grid[mysnake->head.y][mysnake->head.x] = MYHEAD;
    grid[mysnake->tail.y][mysnake->tail.x] = MYTAIL;

    for (auto snake_data : board["snakes"]) {
      Snake *snake = new Snake(snake_data);
      if (snake->id != mysnake->id) {
        snakes.push_back(snake);

        // update other snake
        for (auto &part : snake->body) {
          grid[part.y][part.x] = OTHERBODY;
        }
        grid[snake->head.y][snake->head.x] = OTHERHEAD;
        grid[snake->tail.y][snake->tail.x] = OTHERTAIL;
      }
    }

    foods.clear(); // Hopefully doesn't reallocate
    for (auto &point : board["food"]) {
      foods.push_back(Point(point["x"], point["y"], 0));
      grid[point["y"]][point["x"]] = FOOD; // update grid
    }

    hazards.clear(); // Hopefully doesn't reallocate
    for (auto &point : board["hazards"]) {
      hazards.push_back(Point(point["x"], point["y"], 0));
      grid[point["y"]][point["x"]] = HAZARD; // upate grid
    }

		grids.clear();
		std::cout << "GRID UPDATE HERE: \n";
		for (int i = 0; i < snakes.size(); ++i) {
			std::cout << "ASSIGN GRID: " << i << '\n';
			grids.push_back(grid);
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

	
	// Update new position of the snake, also update the grid
	// If the snake eats food then, tail stays
	// also health = 100
	// return if tail position change
	bool snake_move(const int snake_index, const int newy, const int newx, const int is_mysnake) {
		Snake* snake = snakes[snake_index];
		snake->head.y = newy;
		snake->head.x = newx;

		bool tail_move = false;

		vv snake_grid = std::move(grids[snake_index]);

		Point oldtail = snake->body.back();
		Objects obj = grid[newy][newx];

		snake->body.push_front(snake->head);
	
		if (snake_grid[newy][newx] != FOOD) { // update tail
			snake->body.pop_back();
			Point newtail = snake->body.back();
			snake_grid[oldtail.y][oldtail.x] = EMPTY;
			snake_grid[newtail.y][newtail.x] = is_mysnake ? MYTAIL : OTHERTAIL;
			tail_move = true;
		} // else FOOD -> do nothing

		// Update head	
		snake_grid[newy][newx] = is_mysnake ? MYHEAD : OTHERHEAD;
		
		return tail_move;
	}

	// snake moves backwards
	void restore_snake_move(const int snake_index, Point& oldtail, vv& oldgrid, bool did_tail_move) {
		Snake* snake = snakes[snake_index];
		
		// restore tail
		if (did_tail_move) {
			Point curtail = snake->body.back();
			snake->body.push_back(oldtail);
		}
		
		// restore head
		snake->body.pop_front();
		snake->head = snake->body.front();
		
		grids[snake_index] = oldgrid;
	}

	void combine_and_copy_grids() {
		for (int i = 0; i < height; ++i) {
			for (int j = 0; j < width; ++j) {
				bool empty = true;
				std::vector<int> snake_heads(snakes.size(), 0);

				for (int k = 0; k < grids.size(); ++k) {
					empty &= (grids[k][i][j] == EMPTY);
					
					if (grids[k][i][j] == OTHERHEAD || grids[k][i][j] == MYHEAD) {
						snake_heads[k] = snakes[k]->body.size();
					}
				}
				
				if (empty) grid[i][j] = EMPTY;
				else {
					auto max_it = max_element(snake_heads.begin(), snake_heads.end());
					if (*max_it == 0) grid[i][j] = EMPTY;
					else {
						int p = max_it - snake_heads.begin();
						for (int k = 0; k < snakes.size(); ++k) {
							if (k != p && (grids[k][i][j] == MYHEAD || grids[k][i][j] == OTHERHEAD)) {
								for (auto& point: snakes[k]->body) {
									grids[k][point.y][point.x] = EMPTY;
								}
							}
						}

						Objects val = (p == 0) ? MYBODY : OTHERBODY;
						for (auto& point: snakes[p]->body) {
							grid[point.y][point.x] = val;
						}
						grid[i][j] = grids[p][i][j];
					}
				}
			}
		}

		// copy
		for (auto &g: grids) {
			g = grid;
		}
	}

	void restore_grids(vv& orig_grid) {
		for (auto& g: grids) {
			g = orig_grid;
		}

		grid = orig_grid;
	}
};

#endif // INCLUDE_OBJECTS_HPP
