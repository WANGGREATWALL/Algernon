#include "perf/xtimer.h"

#include <cstdio>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

#include "log/xlogger.h"

namespace au {
namespace perf {

namespace {

size_t        gLevelVisible = static_cast<size_t>(-1);
constexpr int kIndentSize   = 2;

std::string symbolicLine(size_t n, const std::string& symbol)
{
    std::string ret;
    for (size_t i = 0; i < n; ++i)
        ret += symbol;
    return ret;
}

// ── Timer Node ──

class TimerNode
{
public:
    TimerNode(size_t level = 0, const std::string& name = "unnamed") : mName(name), mLevel(level) { mTimer.restart(); }
    void reset() { mDuration = mTimer.elapsed(); }

    std::string            mName;
    XTimer                 mTimer;
    float                  mDuration = 0.0f;
    size_t                 mLevel    = 0;
    std::vector<TimerNode> mNodes;
};

std::string nodePrefix(const TimerNode& node)
{
    std::string line;
    if (node.mLevel == 0)
        return line;
    for (size_t i = 0; i < node.mLevel - 1; ++i) {
        line += "|" + symbolicLine(kIndentSize, " ");
    }
    return line + "|" + symbolicLine(kIndentSize, "-");
}

// ── Timer Tree ──

class TimerTree
{
public:
    static TimerTree& get()
    {
        static TimerTree instance;
        return instance;
    }

    ~TimerTree()
    {
        joinLevelTo(0);
        if (!mRoot.mNodes.empty()) {
            mRoot.reset();
            showByDFS(mRoot);
            mRoot.mNodes.clear();
        }
    }

    void setName(const std::string& name) { mRoot.mName = name; }

    void addNode(const std::string& name)
    {
        if (!inThisThread())
            return;
        auto& active = findActive(mLevel++);
        active.mNodes.emplace_back(TimerNode(active.mLevel + 1, name));
    }

    void resetActive()
    {
        if (!inThisThread())
            return;
        findActive(mLevel--).reset();
    }

    size_t getLevel() const { return mLevel; }

    void joinLevelTo(size_t level)
    {
        size_t cur = mLevel;
        for (size_t i = level; i < cur; ++i)
            resetActive();

        if (level == 0 && !mRoot.mNodes.empty()) {
            mRoot.reset();
            showByDFS(mRoot);
            mRoot.mNodes.clear();
            mRoot.mTimer.restart();
            mRoot.mDuration = 0.0f;
        }
    }

private:
    TimerTree() : mLevel(0), mThreadId(std::this_thread::get_id()) {}
    bool inThisThread() const { return mThreadId == std::this_thread::get_id(); }

    void showByDFS(const TimerNode& node)
    {
        XLOG_I("%s%s: %.2f ms\n", nodePrefix(node).c_str(), node.mName.c_str(), node.mDuration);
        for (auto& n : node.mNodes)
            showByDFS(n);
    }

    TimerNode& findActive(size_t level)
    {
        TimerNode* p = &mRoot;
        for (size_t i = 0; i < level; ++i)
            p = &p->mNodes.back();
        return *p;
    }

    std::thread::id mThreadId;
    TimerNode       mRoot;
    size_t          mLevel;
};

}  // anonymous namespace

// ── Public API ──

void   setTimerRootName(const std::string& name) { TimerTree::get().setName(name); }
void   setPerfVisibleLevel(size_t level) { gLevelVisible = level; }
size_t getPerfVisibleLevel() { return gLevelVisible; }

// ── XTimer ──

void XTimer::sleepFor(long long ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

std::string XTimer::getTimeFormatted(const std::string& fmt)
{
    auto              now = std::chrono::system_clock::now();
    std::time_t       t   = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), fmt.c_str());
    return ss.str();
}

XTimer::XTimer() { restart(); }
void  XTimer::restart() { mBegin = Clock::now(); }
float XTimer::elapsed()
{
    auto end = Clock::now();
    return Duration(end - mBegin).count();
}

// ── XTimerScoped ──

XTimerScoped::XTimerScoped(const std::string& name)
{
    mLevelBegin = TimerTree::get().getLevel();
    if (mLevelBegin < gLevelVisible) {
        TimerTree::get().addNode(name);
    }
}

XTimerScoped::~XTimerScoped() { TimerTree::get().joinLevelTo(mLevelBegin); }

void XTimerScoped::sub(const std::string& name)
{
    if (mLevelBegin + 1 < gLevelVisible) {
        sub();
        TimerTree::get().addNode(name);
    }
}

void XTimerScoped::sub() { TimerTree::get().joinLevelTo(mLevelBegin + 1); }

size_t XTimerScoped::getLevel() const { return mLevelBegin + 1; }

}  // namespace perf
}  // namespace au
