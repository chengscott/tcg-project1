#pragma once
#include "action.h"
#include "board.h"
#include <algorithm>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>

class agent {
public:
  agent(const std::string &args = "") {
    std::stringstream ss("name=unknown role=unknown " + args);
    for (std::string pair; ss >> pair;) {
      std::string key = pair.substr(0, pair.find('='));
      std::string value = pair.substr(pair.find('=') + 1);
      meta[key] = {value};
    }
  }
  virtual ~agent() {}
  virtual void open_episode(const std::string &flag = "") {}
  virtual void close_episode(const std::string &flag = "") {}
  virtual action take_action(const board &b, unsigned) { return action(); }
  virtual bool check_for_win(const board &b) { return false; }

public:
  virtual std::string property(const std::string &key) const {
    return meta.at(key);
  }
  virtual void notify(const std::string &msg) {
    meta[msg.substr(0, msg.find('='))] = {msg.substr(msg.find('=') + 1)};
  }
  virtual std::string name() const { return property("name"); }
  virtual std::string role() const { return property("role"); }

protected:
  typedef std::string key;
  struct value {
    std::string value;
    operator std::string() const { return value; }
    template <typename numeric,
              typename = typename std::enable_if<
                  std::is_arithmetic<numeric>::value, numeric>::type>
    operator numeric() const {
      return numeric(std::stod(value));
    }
  };
  std::map<key, value> meta;
};

class random_agent : public agent {
public:
  random_agent(const std::string &args = "") : agent(args) {
    if (meta.find("seed") != meta.end())
      engine.seed(int(meta["seed"]));
  }
  virtual ~random_agent() {}

protected:
  std::default_random_engine engine;
};

/**
 * random environment
 * add a new random tile to an empty cell
 */
class rndenv : public random_agent {
public:
  rndenv(const std::string &args = "")
      : random_agent("name=random role=environment " + args),
        popup(), space{{12, 13, 14, 15},
                       {0, 4, 8, 12},
                       {0, 1, 2, 3},
                       {3, 7, 11, 15}} {}

  action init_action(size_t step) {
    static std::array<unsigned, 16> init_space{0, 1, 2,  3,  4,  5,  6,  7,
                                               8, 9, 10, 11, 12, 13, 14, 15};
    if (step == 0) {
      popup.reset();
      std::shuffle(init_space.begin(), init_space.end(), engine);
    }
    board::tile_t tile = popup(engine);
    return action::place(init_space[step], tile);
  }

  virtual action take_action(const board &after, unsigned move_) {
    auto &cur = space[move_];
    std::shuffle(cur.begin(), cur.end(), engine);
    for (int pos : cur) {
      if (after(pos) != 0)
        continue;
      board::tile_t tile = popup(engine);
      return action::place(pos, tile);
    }
    return action();
  }

private:
  template <class _IntType, size_t _Size> class bag_int_distribution {
  public:
    bag_int_distribution() { std::iota(std::begin(bag_), std::end(bag_), 1); }
    void reset() { index_ = _Size; }
    _IntType operator()(std::default_random_engine engine) {
      if (index_ == _Size) {
        std::shuffle(std::begin(bag_), std::end(bag_), engine);
        index_ = 0;
      }
      return bag_[index_++];
    }

  private:
    std::array<_IntType, _Size> bag_;
    size_t index_ = _Size;
  };

private:
  std::array<int, 4> space[4];
  bag_int_distribution<board::tile_t, 3> popup;
};

/**
 * dummy player
 * select a legal action randomly
 */
class player : public random_agent {
public:
  player(const std::string &args = "")
      : random_agent("name=dummy role=player " + args), opcode({0, 1, 2, 3}) {}

  virtual action take_action(const board &before, unsigned) {
    std::shuffle(opcode.begin(), opcode.end(), engine);
    for (int op : opcode) {
      board::reward_t reward = board(before).slide(op);
      if (reward != -1)
        return action::slide(op);
    }
    return action();
  }

private:
  std::array<int, 4> opcode;
};
