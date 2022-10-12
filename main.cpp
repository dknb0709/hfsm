#include <iostream>

#include "hfsm.hpp"

enum class Event : unsigned { SW_OFF, SW_ON, START, STOP, NEXT };

struct Context {};

struct Sleeping : State<Context>, Singleton<Sleeping> {
  virtual void onEntry(Context& context, const Event& event) {
    std::cout << "  " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void perform(Context& context, const Event& event) {
    std::cout << "  " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void onExit(Context& context, const Event& event) {
    std::cout << "  " << __PRETTY_FUNCTION__ << std::endl;
  }
};

struct Active : CompositeState<Context>, Singleton<Active> {
  virtual void onEntry(Context& context, const Event& event) {
    std::cout << "  " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void perform(Context& context, const Event& event) {
    std::cout << "  " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void onExit(Context& context, const Event& event) {
    std::cout << "  " << __PRETTY_FUNCTION__ << std::endl;
  }
};

struct Playing : CompositeState<Context>, Singleton<Playing> {
  virtual void onEntry(Context& context, const Event& event) {
    std::cout << "    " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void perform(Context& context, const Event& event) {
    std::cout << "    " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void onExit(Context& context, const Event& event) {
    std::cout << "    " << __PRETTY_FUNCTION__ << std::endl;
  }
};

struct Playing1 : State<Context>, Singleton<Playing1> {
  virtual void onEntry(Context& context, const Event& event) {
    std::cout << "      " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void perform(Context& context, const Event& event) {
    std::cout << "      " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void onExit(Context& context, const Event& event) {
    std::cout << "      " << __PRETTY_FUNCTION__ << std::endl;
  }
};

struct Playing2 : State<Context>, Singleton<Playing2> {
  virtual void onEntry(Context& context, const Event& event) {
    std::cout << "      " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void perform(Context& context, const Event& event) {
    std::cout << "      " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void onExit(Context& context, const Event& event) {
    std::cout << "      " << __PRETTY_FUNCTION__ << std::endl;
  }
};

struct Paused : State<Context>, Singleton<Paused> {
  virtual void onEntry(Context& context, const Event& event) {
    std::cout << "    " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void perform(Context& context, const Event& event) {
    std::cout << "    " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void onExit(Context& context, const Event& event) {
    std::cout << "    " << __PRETTY_FUNCTION__ << std::endl;
  }
};

int main() {
  StateMachine<Context, Sleeping> sm(
      {{Sleeping::getInstance(), Event::SW_ON, Active::getInstance()},
       {Active::getInstance(), Event::SW_OFF, Sleeping::getInstance()}});

  CompositeState<Context>* active = Active::getInstance();
  active->set({{Paused::getInstance(), Event::START, Playing::getInstance()},
               {Playing::getInstance(), Event::STOP, Paused::getInstance()}});

  CompositeState<Context>* playing = Playing::getInstance();
  playing->set(
      {{Playing1::getInstance(), Event::NEXT, Playing2::getInstance()},
       {Playing2::getInstance(), Event::NEXT, Playing1::getInstance()}});

  std::cout << "SW_ON" << std::endl;
  sm.dispatch(Event::SW_ON);
  std::cout << "START" << std::endl;
  sm.dispatch(Event::START);
  std::cout << "NEXT" << std::endl;
  sm.dispatch(Event::NEXT);
  std::cout << "NEXT" << std::endl;
  sm.dispatch(Event::NEXT);
  std::cout << "STOP" << std::endl;
  sm.dispatch(Event::STOP);
  std::cout << "SW_OFF" << std::endl;
  sm.dispatch(Event::SW_OFF);
}
