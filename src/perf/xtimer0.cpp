#include "perf/xtimer0.h"

#include <cstdio>
#include <ctime>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include "log/xlogger.h"
#include "sys/xplatform.h"

namespace au {
namespace perf {
namespace {

// ==========================================================================
// Output serialisation
// ==========================================================================

using OutputMutex = std::recursive_mutex;
OutputMutex gOutputMutex;

// ==========================================================================
// Debug-mode tree (thread_local)
// ==========================================================================

struct TreeNode
{
    const char*            name;
    float                  elapsed = 0.f;
    std::vector<TreeNode>  children;
    XTimer0                 timer;   // starts on construction
};

class TimerTree
{
public:
    void addNode(const char* name)
    {
        TreeNode& parent = findActive(mLevel);
        ++mLevel;
        parent.children.emplace_back();
        TreeNode& node   = parent.children.back();
        node.name        = name;
        // timer already started by TreeNode constructor
    }

    void resetActive()
    {
        TreeNode& node = findActive(mLevel);
        --mLevel;
        node.elapsed = node.timer.elapsed();
    }

    void joinLevelTo(int level)
    {
        while (mLevel > level)
            resetActive();
    }

    int  getLevel() const { return mLevel; }

    void print(const XTimer0Context& ctx) const
    {
        printNode(ctx, mRoot, 0);
    }

    void clear()
    {
        mRoot.children.clear();
        mLevel = 0;
    }

private:
    TreeNode& findActive(int level)
    {
        TreeNode* p = &mRoot;
        for (int i = 0; i < level; ++i) {
            // level is always ≤ actual depth; safe to call back()
            p = &p->children.back();
        }
        return *p;
    }

    static void printNode(const XTimer0Context& ctx, const TreeNode& node, int depth)
    {
        if (depth > 0) {
            // Only apply level filter to non-root nodes
            if (!ctx.shouldPrint(depth))
                return;
            printIndent(ctx, depth);
            std::string line;
            line.reserve(128);
            line += node.name;
            line += ": ";
            line += formatMs(node.elapsed);
            line += "\n";
            ctx.write(line.c_str(), static_cast<int>(line.size()));
        }
        for (const auto& child : node.children)
            printNode(ctx, child, depth + 1);
    }

    static void printIndent(const XTimer0Context& ctx, int depth)
    {
        // ASCII only, no Unicode:  "|  " for each ancestor, then "|--" for node
        char  buf[128];
        char* p   = buf;
        char* end = buf + sizeof(buf) - 1;
        for (int i = 1; i < depth && p < end; ++i) {
            *p++ = '|';
            if (p + 1 < end) { *p++ = ' '; *p++ = ' '; }
        }
        if (p + 2 < end) { *p++ = '|'; *p++ = '-'; *p++ = '-'; }
        *p = '\0';
        ctx.write(buf, static_cast<int>(p - buf));
    }

    static std::string formatMs(float ms)
    {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.2f ms", ms);
        return buf;
    }

    TreeNode mRoot;
    int      mLevel = 0;
};

thread_local TimerTree t_tree;

// ==========================================================================
// XTimer0Context registry
// ==========================================================================

constexpr int kMaxContexts = 32;

struct ContextSlot
{
    XTimer0Context* ctx   = nullptr;
    char           name[64] = {0};
};

ContextSlot gContexts[kMaxContexts];
std::mutex  gRegistryMutex;

XTimer0Context& getOrCreateContext(const char* name)
{
    if (!name || !*name)
        return XTimer0Context::global();

    // Fast path: linear scan under mutex is acceptable since
    // context creation happens once at init time.
    {
        std::lock_guard<std::mutex> lk(gRegistryMutex);
        for (int i = 0; i < kMaxContexts; ++i) {
            if (gContexts[i].ctx && std::strcmp(gContexts[i].name, name) == 0)
                return *gContexts[i].ctx;
        }
        // Allocate new slot
        for (int i = 0; i < kMaxContexts; ++i) {
            if (!gContexts[i].ctx) {
                auto* c = new XTimer0Context();
                gContexts[i].ctx = c;
                std::strncpy(gContexts[i].name, name, sizeof(gContexts[i].name) - 1);
                return *c;
            }
        }
    }
    // Fallback: registry full
    return XTimer0Context::global();
}

}  // anonymous namespace

// ==========================================================================
// XTimer0Context
// ==========================================================================

XTimer0Context& XTimer0Context::get(const char* name) { return getOrCreateContext(name); }

XTimer0Context& XTimer0Context::global()
{
    static XTimer0Context instance;
    return instance;
}

void XTimer0Context::configureFrom(const char* propertyPrefix) noexcept
{
    if (!propertyPrefix)
        return;

    std::string keyEnabled = std::string(propertyPrefix) + ".enabled";
    std::string keyLevel   = std::string(propertyPrefix) + ".level";
    std::string keyMode    = std::string(propertyPrefix) + ".mode";

    // Try Android system property first, then env var
    std::string valEnabled = sys::getSystemPropertyValue(keyEnabled.c_str(), "");
    if (valEnabled.empty())
        valEnabled = sys::getEnv(keyEnabled.c_str());
    if (!valEnabled.empty()) {
        int v = std::atoi(valEnabled.c_str());
        setEnabled(v != 0);
    }

    std::string valLevel = sys::getSystemPropertyValue(keyLevel.c_str(), "");
    if (valLevel.empty())
        valLevel = sys::getEnv(keyLevel.c_str());
    if (!valLevel.empty()) {
        setLevel(std::atoi(valLevel.c_str()));
    }

    std::string valMode = sys::getSystemPropertyValue(keyMode.c_str(), "");
    if (valMode.empty())
        valMode = sys::getEnv(keyMode.c_str());
    if (!valMode.empty()) {
        setMode(std::atoi(valMode.c_str()) != 0 ? Mode::Debug : Mode::Release);
    }
}

void XTimer0Context::write(const char* msg, int len) const noexcept
{
    if (mWriter) {
        mWriter(msg, len, mWriterData);
        return;
    }
    // Default: use xlogger + stdout synchronised
    std::lock_guard<OutputMutex> lk(gOutputMutex);
    std::fwrite(msg, 1, static_cast<size_t>(len), stdout);
    std::fflush(stdout);
}

// ==========================================================================
// XTimer0
// ==========================================================================

void XTimer0::sleepFor(long long ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

std::string XTimer0::getTimeFormatted(const std::string& fmt)
{
    auto        now = std::chrono::system_clock::now();
    std::time_t t   = std::chrono::system_clock::to_time_t(now);
    char        buf[128];

#ifdef AURA_OS_WINDOWS
    std::tm tmBuf;
    localtime_s(&tmBuf, &t);
    std::strftime(buf, sizeof(buf), fmt.c_str(), &tmBuf);
#else
    std::tm tmBuf;
    localtime_r(&t, &tmBuf);
    std::strftime(buf, sizeof(buf), fmt.c_str(), &tmBuf);
#endif

    return buf;
}

float XTimer0::elapsed() const noexcept
{
    auto end = Clock::now();
    return std::chrono::duration<float, std::milli>(end - mBegin).count();
}

// ==========================================================================
// XTimer0Scoped
// ==========================================================================

XTimer0Scoped::XTimer0Scoped(const char* name, XTimer0Context* ctx) noexcept
    : mCtx(ctx ? ctx : &XTimer0Context::global())
    , mName(name)
    , mSegName(name)
    , mLevel(0)
    , mLevelBegin(0)
    , mActive(false)
{
    if (!mCtx->isEnabled())
        return;

    mActive     = true;
    mLevelBegin = t_tree.getLevel();
    mLevel      = mLevelBegin + 1;

    if (mCtx->getMode() == XTimer0Context::Mode::Debug) {
        t_tree.addNode(name);
    }
}

XTimer0Scoped::~XTimer0Scoped() noexcept
{
    if (!mActive)
        return;

    if (mCtx->getMode() == XTimer0Context::Mode::Debug) {
        t_tree.joinLevelTo(mLevelBegin);
        if (mLevelBegin == 0) {
            // Outermost scope — print tree and reset
            std::lock_guard<OutputMutex> lk(gOutputMutex);
            t_tree.print(*mCtx);
            t_tree.clear();
        }
    } else {
        // Release mode: print elapsed for the final segment
        float ms = mTimer.elapsed();
        std::lock_guard<OutputMutex> lk(gOutputMutex);
        if (mCtx->shouldPrint(mLevel)) {
            char buf[128];
            int len = std::snprintf(buf, sizeof(buf), "%s: %.2f ms\n", mSegName, ms);
            if (len > 0)
                mCtx->write(buf, len);
        }
    }
}

void XTimer0Scoped::sub(const char* name) noexcept
{
    if (!mActive)
        return;

    if (mCtx->getMode() == XTimer0Context::Mode::Debug) {
        // End current tree node, start new one at same level
        t_tree.joinLevelTo(mLevelBegin + 1);
        t_tree.addNode(name);
    } else {
        // Release mode: print elapsed for the segment that just finished
        float ms = mTimer.elapsed();
        mTimer.restart();
        if (mCtx->shouldPrint(mLevel)) {
            std::lock_guard<OutputMutex> lk(gOutputMutex);
            char buf[128];
            int len = std::snprintf(buf, sizeof(buf), "%s: %.2f ms\n", mSegName, ms);
            if (len > 0)
                mCtx->write(buf, len);
        }
        mSegName = name;
    }
}

void XTimer0Scoped::sub() noexcept
{
    if (!mActive)
        return;

    if (mCtx->getMode() == XTimer0Context::Mode::Debug) {
        t_tree.joinLevelTo(mLevelBegin + 1);
    }
    // Release mode: no-op (bare sub just transitions to next named sub)
}

float XTimer0Scoped::elapsed() const noexcept
{
    return mTimer.elapsed();
}

}  // namespace perf
}  // namespace au
