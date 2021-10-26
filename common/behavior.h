//
// Created by baifeng on 2021/10/27.
//

#ifndef SDL2_UI_BEHAVIOR_H
#define SDL2_UI_BEHAVIOR_H

#include <memory>
#include <vector>

class Behavior {
public:
    enum Status {
        fail,
        success,
        running
    };
    typedef std::shared_ptr<Behavior> Ptr;
    template<typename T, typename... Args>
    static Ptr New(Args const&... args) {
        return Ptr(new T(args...));
    }
public:
    virtual ~Behavior() {}
    virtual Status tick(float delta) = 0;
};

class SequenceBehavior : public Behavior {
public:
    typedef std::vector<Ptr> Behaviors;
public:
    SequenceBehavior();
    SequenceBehavior(Behaviors const& sequence);
protected:
    Status tick(float delta) override;
    void add(Behavior::Ptr const& behavior);
protected:
    Behaviors _sequence;
};

#endif //SDL2_UI_BEHAVIOR_H
