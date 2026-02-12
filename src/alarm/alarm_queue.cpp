#include "alarm_queue.h"
#include <queue>

static std::queue<AlarmEvent> q;

void alarm_queuePush(const AlarmEvent& ev) {
    q.push(ev);
}

bool alarm_queuePop(AlarmEvent& ev) {
    if (q.empty()) return false;
    ev = q.front();
    q.pop();
    return true;
}

size_t alarm_queueSize() {
    return q.size();
}
