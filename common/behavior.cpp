//
// Created by baifeng on 2021/10/27.
//

#include "behavior.h"

SequenceBehavior::SequenceBehavior() {}

SequenceBehavior::SequenceBehavior(Behaviors const& sequence):_sequence(sequence) {

}

Behavior::Status SequenceBehavior::tick(float delta) {
    auto status = success;
    for (auto& c : _sequence) {
        if ((status = c->tick(delta)) == fail) {
            break;
        }
    }
    return status;
}

void SequenceBehavior::add(Behavior::Ptr const& behavior) {
    _sequence.push_back(behavior);
}