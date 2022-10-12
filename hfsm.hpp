#include <algorithm>
#include <tuple>
#include <vector>

template <class T>
struct Singleton {
  static T* getInstance() {
    static T instance;
    return &instance;
  }
};

enum class Event : unsigned;

template <class ContextType>
struct State {
  State() : parent{nullptr} {}
  virtual void onEntry(ContextType& context, const Event& event) = 0;
  virtual void perform(ContextType& context, const Event& event) = 0;
  virtual void onExit(ContextType& context, const Event& event) = 0;
  virtual bool isComposite() { return false; }

  State<ContextType>* parent;
};

template <class ContextType>
using Transition =
    std::vector<std::tuple<State<ContextType>*, Event, State<ContextType>*>>;

template <class ContextType>
struct Resolver {
  Transition<ContextType> transition;

  void set(State<ContextType>* parent, Transition<ContextType> transition) {
    this->transition = transition;
    for (auto&& tran : this->transition) {
      State<ContextType>* state = std::get<0>(tran);
      state->parent = parent;
    }
  }

  State<ContextType>* resolveFirst() {
    if (transition.size() == 0) return nullptr;
    return std::get<0>(transition[0]);
  }

  State<ContextType>* resolve(State<ContextType>* currState,
                              const Event& receivedEvent) {
    for (auto&& tran : transition) {
      auto&& state = std::get<0>(tran);
      auto&& event = std::get<1>(tran);
      if (state == currState && event == receivedEvent) {
        return std::get<2>(tran);
      }
    }
    return nullptr;
  }
};

template <class ContextType>
struct CompositeState : State<ContextType> {
  Resolver<ContextType> resolver;

  void set(Transition<ContextType> transition) {
    this->resolver.set(this, transition);
  }

  State<ContextType>* resolveFirst() { return resolver.resolveFirst(); }

  State<ContextType>* resolve(State<ContextType>* currState,
                              const Event& event) {
    return resolver.resolve(currState, event);
  }

  virtual bool isComposite() { return true; }
};

template <class ContextType, class InitialState>
struct StateMachine {
  struct Root : CompositeState<ContextType>, Singleton<Root> {
    virtual void onEntry(ContextType& context, const Event& event) {}
    virtual void perform(ContextType& context, const Event& event) {}
    virtual void onExit(ContextType& context, const Event& event) {}
  };
  Root root;
  State<ContextType>* prevState;
  State<ContextType>* currState;
  ContextType context;

  StateMachine(Transition<ContextType> transition) {
    root.set(transition);
    prevState = nullptr;
    currState = InitialState::getInstance();
  }

  void setState(State<ContextType>* nextState) {
    if (nextState && nextState != currState) {
      prevState = currState;
      currState = nextState;
    }
  }

  State<ContextType>* resolve(const Event& event) {
    if (!currState->parent) {
      return nullptr;
    }
    CompositeState<ContextType>* prevParent = nullptr;
    CompositeState<ContextType>* parent =
        static_cast<CompositeState<ContextType>*>(currState->parent);
    State<ContextType>* nextState = parent->resolve(currState, event);
    if (nextState && !nextState->isComposite()) return nextState;

    while (!nextState && parent->parent) {
      prevParent = parent;
      parent = static_cast<CompositeState<ContextType>*>(parent->parent);
      nextState = parent->resolve(prevParent, event);
    }

    while (nextState && nextState->isComposite()) {
      CompositeState<ContextType>* state =
          static_cast<CompositeState<ContextType>*>(nextState);
      nextState = state->resolveFirst();
    }

    return nextState;
  }

  void doAction(const Event& event) {
    if (currState == prevState) return;

    State<ContextType>* prevParent = prevState ? prevState->parent : nullptr;
    State<ContextType>* currParent = currState->parent;

    std::vector<State<ContextType>*> prevParents;
    std::vector<State<ContextType>*> currParents;
    while (prevParent) {
      prevParents.push_back(prevParent);
      prevParent = prevParent->parent;
    }
    while (currParent) {
      currParents.push_back(currParent);
      currParent = currParent->parent;
    }

    std::reverse(prevParents.begin(), prevParents.end());
    std::reverse(currParents.begin(), currParents.end());

    std::vector<State<ContextType>*> exitList;
    std::vector<State<ContextType>*> entryList;
    int len = std::min(prevParents.size(), currParents.size());
    for (int i = 0; i < len; ++i) {
      if (prevParents[i] != currParents[i]) {
        exitList.push_back(prevParents[i]);
        entryList.push_back(currParents[i]);
      }
    }

    if (prevParents.size() < currParents.size()) {
      for (int i = prevParents.size(); i < currParents.size(); ++i) {
        entryList.push_back(currParents[i]);
      }
    }

    if (prevParents.size() > currParents.size()) {
      for (int i = currParents.size(); i < prevParents.size(); ++i) {
        exitList.push_back(prevParents[i]);
      }
    }

    std::reverse(exitList.begin(), exitList.end());

    // onExit
    if (prevState) prevState->onExit(context, event);
    for (auto&& elm : exitList) {
      elm->onExit(context, event);
    }

    // onEntry
    for (auto&& elm : entryList) {
      elm->onEntry(context, event);
    }
    currState->onEntry(context, event);

    // Perform
    currState->perform(context, event);
  }

  bool dispatch(const Event& event) {
    State<ContextType>* nextState = resolve(event);
    if (!nextState)
      return false;
    setState(nextState);
    doAction(event);
    return true;
  }
};
