#include <emscripten/bind.h>
#include "scheduler.h"

using namespace emscripten;

/**
 * Wrapper to return JSON as string for JavaScript consumption
 */
std::string getStateJSONString(Scheduler& self) {
    return self.getStateJSON().dump();
}

EMSCRIPTEN_BINDINGS(scheduler_module) {
    class_<Scheduler>("Scheduler")
        .constructor<>()
        .function("addProcess", &Scheduler::addProcess)
        .function("setAlgorithm", &Scheduler::setAlgorithm)
        .function("setTimeQuantum", &Scheduler::setTimeQuantum)
        .function("setAging", &Scheduler::setAging)
        .function("setAgingThreshold", &Scheduler::setAgingThreshold)
        .function("tick", &Scheduler::tick)
        .function("isFinished", &Scheduler::isFinished)
        .function("getStateJSON", &getStateJSONString);
}
