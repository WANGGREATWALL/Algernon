#include "perf/xtimer2.h"

#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

#include "log/xlogger.h"

namespace algernon {
namespace perf {

// ── XPerfContext ──

XPerfContext::XPerfContext(const std::string& name, const std::string& timerEnProp, const std::string& timerLevelProp,
                           const std::string& tracerEnProp, const std::string& tracerLevelProp,
                           const std::string& treeModeProp)
    : mName(name)
{
    mProps[0] = timerEnProp;
    mProps[1] = timerLevelProp;
    mProps[2] = tracerEnProp;
    mProps[3] = tracerLevelProp;
    mProps[4] = treeModeProp;
    reload();
}

void XPerfContext::reload()
{
    mConfig.timerEnabled  = mProps[0].empty() ? true : sys::getSystemPropertyValue(mProps[0].c_str(), 1) != 0;
    mConfig.timerLevel    = mProps[1].empty() ? 255 : sys::getSystemPropertyValue(mProps[1].c_str(), 255);
    mConfig.tracerEnabled = mProps[2].empty() ? true : sys::getSystemPropertyValue(mProps[2].c_str(), 1) != 0;
    mConfig.tracerLevel   = mProps[3].empty() ? 255 : sys::getSystemPropertyValue(mProps[3].c_str(), 255);
    mConfig.treeMode      = mProps[4].empty() ? false : sys::getSystemPropertyValue(mProps[4].c_str(), 0) != 0;
}

XPerfContext& getGlobalPerfContext()
{
    static XPerfContext instance("Global");
    return instance;
}

// ── XTimer2 ──

void XTimer2::sleepFor(long long ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

std::string XTimer2::getTimeFormatted(const std::string& fmt)
{
    auto              now = std::chrono::system_clock::now();
    std::time_t       t   = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), fmt.c_str());
    return ss.str();
}

// ── Thread Local Tree Structure ──

namespace {

struct TraceNode
{
    const char* name;
    float       duration_ms;
    int         depth;
};

struct TreeContext
{
    std::vector<TraceNode> nodes;
    int                    currentDepth = 0;

    TreeContext()
    {
        nodes.reserve(1024);  // Reserve memory to prevent runtime allocation
    }

    void reset()
    {
        nodes.clear();
        currentDepth = 0;
    }
};

static thread_local TreeContext t_treeCtx;

inline void pushNode(const char* name)
{
    t_treeCtx.nodes.push_back({name, -1.0f, t_treeCtx.currentDepth});
    t_treeCtx.currentDepth++;
}

inline void popNode(float duration)
{
    if (t_treeCtx.currentDepth <= 0)
        return;
    t_treeCtx.currentDepth--;
    // Find the most recent unclosed node at the target depth
    for (int i = static_cast<int>(t_treeCtx.nodes.size()) - 1; i >= 0; --i) {
        if (t_treeCtx.nodes[i].depth == t_treeCtx.currentDepth && t_treeCtx.nodes[i].duration_ms < 0.0f) {
            t_treeCtx.nodes[i].duration_ms = duration;
            break;
        }
    }
}

inline bool isTreeRoot() { return t_treeCtx.currentDepth == 0; }

inline void printAndClearTree(const char* ctxName)
{
    if (t_treeCtx.nodes.empty())
        return;

    XLOG_I("=== Performance Tree [%s] ===\n", ctxName ? ctxName : "Global");

    // Pre-calculate "is_last" for each node to properly draw lines.
    std::vector<bool> isLast(t_treeCtx.nodes.size(), true);
    for (size_t i = 0; i < t_treeCtx.nodes.size(); ++i) {
        int depth = t_treeCtx.nodes[i].depth;
        for (size_t j = i + 1; j < t_treeCtx.nodes.size(); ++j) {
            if (t_treeCtx.nodes[j].depth < depth)
                break;
            if (t_treeCtx.nodes[j].depth == depth) {
                isLast[i] = false;
                break;
            }
        }
    }

    std::vector<bool> ancestorIsLast(64, true);

    for (size_t i = 0; i < t_treeCtx.nodes.size(); ++i) {
        const auto& node = t_treeCtx.nodes[i];

        if (node.depth >= 0 && node.depth < 64) {
            ancestorIsLast[node.depth] = isLast[i];
        }

        std::string prefix;
        if (node.depth > 0) {
            for (int d = 1; d < node.depth; ++d) {
                if (d < 64 && ancestorIsLast[d]) {
                    prefix += "    ";
                } else {
                    prefix += "│   ";
                }
            }
            if (isLast[i]) {
                prefix += "└── ";
            } else {
                prefix += "├── ";
            }
        }

        float dur = node.duration_ms < 0.0f ? 0.0f : node.duration_ms;
        XLOG_I("%s%s: %.2f ms\n", prefix.c_str(), node.name, dur);
    }
    XLOG_I("=======================================\n");

    t_treeCtx.reset();
}

}  // anonymous namespace

// ── XTimer2Scoped ──

XTimer2Scoped::XTimer2Scoped(XPerfContext* ctx, const char* name, int level)
    : mCtx(ctx ? ctx : &getGlobalPerfContext()), mName(name), mLastSubName(nullptr), mLevel(level), mHasSub(false)
{
    mActive   = mCtx->isTimerEnabled(mLevel);
    mTreeMode = mCtx->isTreeMode();

    if (mActive) {
        if (mTreeMode) {
            pushNode(name);
        }
    }
}

void XTimer2Scoped::sub(const char* name)
{
    if (!mActive)
        return;

    float subElapsed = mSubTimer.elapsed();
    if (mTreeMode) {
        if (mHasSub) {
            popNode(subElapsed);  // close previous sub
        }
        pushNode(name);  // open new sub
    } else {
        // immediate log mode
        if (mHasSub) {
            XLOG_I("[Timer:%s] %s -> %s: %.2f ms\n", mCtx->getName(), mName, mLastSubName, subElapsed);
        } else {
            XLOG_I("[Timer:%s] %s (pre-sub): %.2f ms\n", mCtx->getName(), mName, subElapsed);
        }
    }
    mHasSub      = true;
    mLastSubName = name;
    mSubTimer.restart();
}

void XTimer2Scoped::sub()
{
    if (!mActive)
        return;

    if (mHasSub) {
        float subElapsed = mSubTimer.elapsed();
        if (mTreeMode) {
            popNode(subElapsed);
        } else {
            XLOG_I("[Timer:%s] %s -> %s: %.2f ms\n", mCtx->getName(), mName, mLastSubName, subElapsed);
        }
        mHasSub      = false;
        mLastSubName = nullptr;
    }
}

XTimer2Scoped::~XTimer2Scoped()
{
    if (!mActive)
        return;

    float totalElapsed = mTimer.elapsed();

    if (mTreeMode) {
        if (mHasSub) {
            float subElapsed = mSubTimer.elapsed();
            popNode(subElapsed);  // close last sub
        }
        popNode(totalElapsed);  // close main

        if (isTreeRoot()) {
            printAndClearTree(mCtx->getName());
        }
    } else {
        if (mHasSub) {
            float subElapsed = mSubTimer.elapsed();
            XLOG_I("[Timer:%s] %s -> %s: %.2f ms\n", mCtx->getName(), mName, mLastSubName, subElapsed);
        }
        XLOG_I("[Timer:%s] %s (Total): %.2f ms\n", mCtx->getName(), mName, totalElapsed);
    }
}

}  // namespace perf
}  // namespace algernon
