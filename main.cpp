// HTTP credits: http://lcs.ios.ac.cn/~maxt/SPelton/reports/report-9aa0d3.html
// JSON credits: https://github.com/nlohmann/json
#include "./http_stuff.h"
#include "./json.hpp"
#include "./move.hpp"
#include <iostream>
using namespace std;
using namespace nlohmann;

void add_cors_headers(httplib::Response &res) {
  res.set_header("Allow", "GET, POST");
  res.set_header("Access-Control-Allow-Origin", "*");
  res.set_header("Access-Control-Allow-Methods", "GET, POST");
  res.set_header("Access-Control-ALlow-Headers", "Content-Type");
}

int main(void) {
  httplib::Server svr;

  GameState *game = nullptr;

  svr.Options(".*", [](const auto &, auto &res) {
    res.set_content("ok", "text/plain");

    add_cors_headers(res);
  });
  svr.Get("/", [](const auto &, auto &res) {
    string head = "all-seeing";    // TODO: Change head
    string tail = "curled";        // TODO: Change tail
    string author = "killer-team"; // TODO: Change your battlesnake username
    string color = "#FF0000";      // TODO: Change a hex color
    res.set_content("{\"apiversion\":\"1\", \"head\":\"" + head +
                        "\", \"tail\":\"" + tail + "\", \"color\":\"" + color +
                        "\", " + "\"author\":\"" + author + "\"}",
                    "application/json");

    add_cors_headers(res);
  });
  svr.Post("/end", [](const auto &, auto &res) {
    res.set_content("ok", "text/plain");

    add_cors_headers(res);
  });
  svr.Post("/start", [game](const auto &req, auto &res) mutable {
    // We should utilize the startup call to allocate memory and
    // perform pre-computations: Every memory allocation in
    // `move` is costly! Also, I don't think we have a time limit
    // for this call either hehehe
    //
    // For example, instead of generating a min-max tree every move,
    // we generate a tree at the start, filling in the data every move.
    //
    // If we have to allocate memory in `move`, we should use our
    // own memory allocator, which is filled up at the start. That way,
    // memory is already allocated and we don't have to use the costly
    // `malloc` call (requires switching to Kernal mode which takes a lot
    // of cycles compared to staying in user mode). This is what a lot of
    // game engines do actually; learning gamedev has its perks :D
    //
    // Overall, we will be limited by the CPU: memory-intensive
    // algorithms are more viable now.
    const json data = json::parse(req.body);
    game = new GameState(data);

    res.set_content("ok", "text/plain");

    add_cors_headers(res);
  });
  svr.Post("/move", [game](auto &req, auto &res) mutable {
    const json data = json::parse(req.body);
    cout << data;
    cout << "\n\n";
    // You can get the "you" property like this:
    // data["you"];
    // Almost alike python dictionary parsing, but with a semicolon at the end
    // of each line. You might need to make some structs to store some data in a
    // variable Example: you_struct you = data["you"];
    // string moves[4] = {"up", "down", "left", "right"};
    // int index = rand() % 4;

    // The BattleSnake Tester site doesn't follow the game rules:
    // it doesn't first call `/start`!
    if (game == nullptr) {
      game = new GameState(data);
    } else {
      game->update(data);
    }

    std::string move = cppssss::move(game);

    res.set_content("{\"move\": \"" + move + "\"}", "text/plain");

    add_cors_headers(res);
  });
  svr.listen("0.0.0.0", 8080);
  cout << "Server started";
}
