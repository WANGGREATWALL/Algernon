#include "perf/xtimer4.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "log/xlogger.h"
#include "sys/xplatform.h"

#if AU_OS_WINDOWS
#include <windows.h>
#else
#include <time.h>
#include <unistd.h>
#endif


namespace au {
namespace perf {


// ===========================================================================
//  XPerfContext4Impl  (Pimpl)
// ===========================================================================

class XPerfContext4Impl
{
public:
    std::atomic<bool>     mEnabled{true};
    std::atomic<Mode4>    mMode{Mode4::Release};
    std::atomic<int32_t>  mTimerLevel{3};
    std::atomic<int32_t>  mTracerLevel{kPerfLevelAll4};
    std::atomic<uint32_t> mMaxDepth{64};
    std::atomic<bool>     mAggregate{false};

    // Root header label. Mutated only via setRootName; readers use a length
    // store with release/acquire to avoid torn reads under contention.
    std::atomic<uint32_t> mRootNameLen{4};
    char                  mRootName[64]{'p', 'e', 'r', 'f', '\0'};

    // Pluggable writer (raw pointer; lifetime owned by the caller). Stored
    // as atomic<void*> + reinterpret_cast because std::atomic<T*> with a
    // non-trivially-destructible T is awkward to forward-declare.
    std::atomic<IPerfWriter4*> mWriter{nullptr};
};


// ===========================================================================
//  Default writer (xlogger-backed) and writer dispatch
// ===========================================================================

namespace {


/// Forwards to XLOG_I. Used when no custom writer is installed.
class DefaultWriter4 : public IPerfWriter4
{
public:
    void write(const char* data, std::size_t size) noexcept override
    {
        // xlogger expects a NUL-terminated format-style argument; copy into
        // a stack buffer if small, otherwise fall back to a heap copy. We
        // strip a trailing newline if present because XLOG_I will append
        // its own implicit framing and the tests pattern-match raw text.
        char        stack[512];
        std::size_t cp     = std::min(size, sizeof(stack) - 1);
        bool        useHeap = (size > sizeof(stack) - 1);

        if (!useHeap) {
            std::memcpy(stack, data, cp);
            stack[cp] = '\0';
            XLOG_I("%s", stack);
            return;
        }

        std::string heap;
        try {
            heap.assign(data, size);
        } catch (...) {
            // Last-resort: truncate to the stack budget.
            std::memcpy(stack, data, sizeof(stack) - 1);
            stack[sizeof(stack) - 1] = '\0';
            XLOG_I("%s", stack);
            return;
        }
        XLOG_I("%s", heap.c_str());
    }
};


DefaultWriter4& defaultWriter4() noexcept
{
    static DefaultWriter4 instance;
    return instance;
}


/// Resolve the active writer for a context: custom if installed, else default.
inline IPerfWriter4& resolveWriter4(const XPerfContext4Impl& impl) noexcept
{
    IPerfWriter4* w = impl.mWriter.load(std::memory_order_acquire);
    return w ? *w : static_cast<IPerfWriter4&>(defaultWriter4());
}


/// Format-then-emit helper. Truncates to 512 bytes; emits via @p writer.
void emitFormatted4(IPerfWriter4& writer, const char* fmt, ...) noexcept
{
    char    buf[512];
    va_list ap;
    va_start(ap, fmt);
    int n = std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (n <= 0) {
        return;
    }
    const std::size_t len = (static_cast<std::size_t>(n) >= sizeof(buf)) ? sizeof(buf) - 1 : static_cast<std::size_t>(n);
    writer.write(buf, len);
}


}  // anonymous namespace


// ===========================================================================
//  XPerfContext4 鈥?public surface
// ===========================================================================

XPerfContext4::XPerfContext4() noexcept : mImpl(new (std::nothrow) XPerfContext4Impl)
{
    // If allocation failed mImpl is null; every accessor below tolerates that
    // by treating it as "permanently disabled". This avoids crashing during
    // OOM at program startup.
}


XPerfContext4::~XPerfContext4() noexcept
{
    delete mImpl;
    mImpl = nullptr;
}


XPerfContext4& XPerfContext4::defaultContext() noexcept
{
    static XPerfContext4 instance;
    return instance;
}


void XPerfContext4::setEnabled(bool on) noexcept
{
    if (mImpl != nullptr) {
        mImpl->mEnabled.store(on, std::memory_order_relaxed);
    }
}


bool XPerfContext4::isEnabled() const noexcept
{
    return mImpl != nullptr && mImpl->mEnabled.load(std::memory_order_relaxed);
}


void XPerfContext4::setMode(Mode4 mode) noexcept
{
    if (mImpl != nullptr) {
        mImpl->mMode.store(mode, std::memory_order_relaxed);
    }
}


Mode4 XPerfContext4::getMode() const noexcept
{
    return mImpl != nullptr ? mImpl->mMode.load(std::memory_order_relaxed) : Mode4::Release;
}


void XPerfContext4::setTimerLevel(int32_t threshold) noexcept
{
    if (mImpl != nullptr) {
        mImpl->mTimerLevel.store(threshold, std::memory_order_relaxed);
    }
}


int32_t XPerfContext4::getTimerLevel() const noexcept
{
    return mImpl != nullptr ? mImpl->mTimerLevel.load(std::memory_order_relaxed) : kPerfLevelOff4;
}


void XPerfContext4::setTracerLevel(int32_t threshold) noexcept
{
    if (mImpl != nullptr) {
        mImpl->mTracerLevel.store(threshold, std::memory_order_relaxed);
    }
}


int32_t XPerfContext4::getTracerLevel() const noexcept
{
    return mImpl != nullptr ? mImpl->mTracerLevel.load(std::memory_order_relaxed) : kPerfLevelOff4;
}


void XPerfContext4::setMaxTreeDepth(uint32_t depth) noexcept
{
    if (mImpl != nullptr) {
        mImpl->mMaxDepth.store(depth, std::memory_order_relaxed);
    }
}


uint32_t XPerfContext4::getMaxTreeDepth() const noexcept
{
    return mImpl != nullptr ? mImpl->mMaxDepth.load(std::memory_order_relaxed) : 0u;
}


void XPerfContext4::setRootName(const char* name) noexcept
{
    setRootName(name, name != nullptr ? std::strlen(name) : 0u);
}


void XPerfContext4::setRootName(const char* name, std::size_t len) noexcept
{
    if (mImpl == nullptr) {
        return;
    }
    const std::size_t cap = sizeof(mImpl->mRootName) - 1;
    const std::size_t cp  = (name != nullptr) ? std::min(len, cap) : 0u;
    if (cp > 0u) {
        std::memcpy(mImpl->mRootName, name, cp);
    }
    mImpl->mRootName[cp] = '\0';
    mImpl->mRootNameLen.store(static_cast<uint32_t>(cp), std::memory_order_release);
}


void XPerfContext4::getRootName(char* outBuf, std::size_t bufSize) const noexcept
{
    if (outBuf == nullptr || bufSize == 0) {
        return;
    }
    if (mImpl == nullptr) {
        outBuf[0] = '\0';
        return;
    }
    const uint32_t    len = mImpl->mRootNameLen.load(std::memory_order_acquire);
    const std::size_t cp  = std::min(static_cast<std::size_t>(len), bufSize - 1);
    std::memcpy(outBuf, mImpl->mRootName, cp);
    outBuf[cp] = '\0';
}


void XPerfContext4::setAggregateMode(bool on) noexcept
{
    if (mImpl != nullptr) {
        mImpl->mAggregate.store(on, std::memory_order_relaxed);
    }
}


bool XPerfContext4::isAggregateMode() const noexcept
{
    return mImpl != nullptr && mImpl->mAggregate.load(std::memory_order_relaxed);
}


void XPerfContext4::setWriter(IPerfWriter4* writer) noexcept
{
    if (mImpl != nullptr) {
        mImpl->mWriter.store(writer, std::memory_order_release);
    }
}


IPerfWriter4* XPerfContext4::getWriter() const noexcept
{
    return mImpl != nullptr ? mImpl->mWriter.load(std::memory_order_acquire) : nullptr;
}


void XPerfContext4::loadFromSystemProperty(const char* propEnabled, const char* propMode, const char* propTimerLevel,
                                           const char* propTracerLevel) noexcept
{
    if (mImpl == nullptr) {
        return;
    }

    if (propEnabled != nullptr) {
        const int v = au::sys::getSystemPropertyValue(propEnabled, isEnabled() ? 1 : 0);
        setEnabled(v != 0);
    }
    if (propMode != nullptr) {
        const int v = au::sys::getSystemPropertyValue(propMode, static_cast<int>(getMode()));
        setMode(v != 0 ? Mode4::Debug : Mode4::Release);
    }
    if (propTimerLevel != nullptr) {
        const int v = au::sys::getSystemPropertyValue(propTimerLevel, getTimerLevel());
        setTimerLevel(v);
    }
    if (propTracerLevel != nullptr) {
        const int v = au::sys::getSystemPropertyValue(propTracerLevel, getTracerLevel());
        setTracerLevel(v);
    }
}

// ===========================================================================
//  Internal tree data structures (file-local)
// ===========================================================================

namespace {


/// One node in the per-thread tree. ~64 bytes (one cache line).
struct PerfNode4
{
    uint32_t                              nameOffset;   // offset into nameArena
    uint32_t                              nameLen;
    int32_t                               parent;       // -1 for thread-level roots
    int32_t                               firstChild;
    int32_t                               lastChild;
    int32_t                               nextSibling;
    uint32_t                              depth;
    uint32_t                              flags;        // bit0 closed, bit1 truncated
    std::chrono::steady_clock::time_point begin;
    float                                 durationMs;
};


constexpr uint32_t kFlagClosed4    = 1u << 0;
constexpr uint32_t kFlagTruncated4 = 1u << 1;


/// Per-thread context. Stored as thread_local, never shared.
struct PerfThreadCtx4
{
    std::vector<PerfNode4> pool;
    std::vector<char>      nameArena;
    std::vector<int32_t>   openStack;
    std::thread::id        tid;
    bool                   inited{false};
    bool                   inFlush{false};
};


constexpr std::size_t kPoolReserve4  = 256;
constexpr std::size_t kArenaReserve4 = 8192;
constexpr std::size_t kMaxNameLen4   = 1023;

/// Largest name slice that prints in a single line. The line buffer in
/// emitFormatted4() is 512 bytes; we have to leave room for the prefix,
/// branch ASCII, alignment padding, the duration field and the trailing
/// markers ("(open)", "(truncated)"). 256 keeps the worst case below the
/// emit cap with comfortable margin and forces a visible "(truncated)"
/// suffix whenever the original arena content was clipped.
constexpr uint32_t    kPrintNameClip4 = 256;


PerfThreadCtx4& tlsCtx4() noexcept
{
    thread_local PerfThreadCtx4 ctx;
    if (!ctx.inited) {
        try {
            ctx.pool.reserve(kPoolReserve4);
            ctx.nameArena.reserve(kArenaReserve4);
            ctx.openStack.reserve(64);
        } catch (...) {
            // Tolerate OOM at TLS init 鈥?subsequent push_back may still throw,
            // and that path is caught locally in begin()/sub().
        }
        ctx.tid    = std::this_thread::get_id();
        ctx.inited = true;
    }
    return ctx;
}


/// Append @p name into the TLS arena. Returns (offset, len, truncated).
struct ArenaPut4Result
{
    uint32_t offset;
    uint32_t len;
    bool     truncated;
};


ArenaPut4Result arenaPut4(PerfThreadCtx4& ctx, const char* name, std::size_t inLen) noexcept
{
    const bool        truncated = inLen > kMaxNameLen4;
    const uint32_t    len       = static_cast<uint32_t>(truncated ? kMaxNameLen4 : inLen);
    const uint32_t    offset    = static_cast<uint32_t>(ctx.nameArena.size());
    try {
        ctx.nameArena.insert(ctx.nameArena.end(), name, name + len);
    } catch (...) {
        return {0u, 0u, false};
    }
    return {offset, len, truncated};
}


uint64_t tidHash4(std::thread::id id) noexcept
{
    return static_cast<uint64_t>(std::hash<std::thread::id>{}(id));
}


// ---------------------------------------------------------------------------
//  Tree printer (ASCII only)
// ---------------------------------------------------------------------------


std::vector<int32_t> collectRoots4(const std::vector<PerfNode4>& pool)
{
    std::vector<int32_t> roots;
    roots.reserve(8);
    for (int32_t i = 0; i < static_cast<int32_t>(pool.size()); ++i) {
        if (pool[static_cast<std::size_t>(i)].parent == -1) {
            roots.push_back(i);
        }
    }
    return roots;
}


/// Compute the maximum prefix+name column width across the whole forest so
/// duration values can right-align. Two-pass design (a separate measurement
/// walk before the print walk) keeps alignment correct regardless of DFS
/// ordering 鈥?the same correctness fix v1 introduced over v3.
uint32_t computeNameColumnWidth4(const std::vector<PerfNode4>& pool, const std::vector<int32_t>& roots) noexcept
{
    uint32_t maxWidth = 0;

    for (std::size_t ri = 0; ri < roots.size(); ++ri) {
        const bool rootIsLast = (ri + 1 == roots.size());

        struct Frame
        {
            int32_t     idx;
            std::string prefix;
            bool        isLast;
        };
        std::vector<Frame> dfs;
        dfs.push_back({roots[ri], std::string(), rootIsLast});

        while (!dfs.empty()) {
            Frame cur = std::move(dfs.back());
            dfs.pop_back();

            const PerfNode4& n         = pool[static_cast<std::size_t>(cur.idx)];
            const uint32_t   prefixLen = static_cast<uint32_t>(cur.prefix.size()) + (cur.prefix.empty() ? 0u : 4u);
            // Use the SAME clip used by the printer, otherwise an outsized
            // node name expands the column to a value the printer cannot
            // fill, blowing past emitFormatted4()'s 512-byte buffer and
            // dropping the trailing markers.
            const uint32_t   nameW     = (n.nameLen > kPrintNameClip4) ? kPrintNameClip4 : n.nameLen;
            const uint32_t   w         = prefixLen + nameW;
            if (w > maxWidth) {
                maxWidth = w;
            }

            std::vector<int32_t> children;
            for (int32_t c = n.firstChild; c != -1; c = pool[static_cast<std::size_t>(c)].nextSibling) {
                children.push_back(c);
            }
            const std::string childPrefix = cur.prefix + (cur.isLast ? "    " : "|   ");
            for (std::size_t k = children.size(); k-- > 0;) {
                const bool last = (k == children.size() - 1);
                dfs.push_back({children[k], childPrefix, last});
            }
        }
    }
    return maxWidth;
}


void printNodeLine4(IPerfWriter4& writer, const std::vector<PerfNode4>& pool, const std::vector<char>& arena,
                    int32_t idx, const std::string& prefix, bool isLast, uint32_t nameCol) noexcept
{
    const PerfNode4& n    = pool[static_cast<std::size_t>(idx)];
    const char*      name = (n.nameLen == 0) ? "" : (arena.data() + n.nameOffset);

    const char*    branch    = prefix.empty() ? "" : (isLast ? "`-- " : "|-- ");
    const uint32_t prefixLen = static_cast<uint32_t>(prefix.size()) + (prefix.empty() ? 0u : 4u);

    // Clamp the printed name slice so emitFormatted4()'s 512-byte cap
    // never silently swallows trailing markers.
    const uint32_t printLen   = (n.nameLen > kPrintNameClip4) ? kPrintNameClip4 : n.nameLen;
    const bool     printClip  = (n.nameLen > kPrintNameClip4);
    const bool     truncFlag  = (n.flags & kFlagTruncated4) != 0u;

    int padding = static_cast<int>(nameCol) - static_cast<int>(prefixLen) - static_cast<int>(printLen);
    if (padding < 0) {
        padding = 0;
    }

    const float ms        = (n.flags & kFlagClosed4) ? n.durationMs : 0.0f;
    const char* openMark  = (n.flags & kFlagClosed4) ? "" : " (open)";
    // A name truncated by the arena (>1023 bytes) is also clipped at print
    // time, but a name <=1023 bytes can still be print-clipped at 256 鈥?both
    // cases deserve the visible marker.
    const char* truncMark = (truncFlag || printClip) ? " (truncated)" : "";

    emitFormatted4(writer, "%s%s%.*s%*s : %8.3f ms%s%s\n", prefix.c_str(), branch, static_cast<int>(printLen), name,
                   padding, "", ms, openMark, truncMark);
}


void printTree4(IPerfWriter4& writer, const std::vector<PerfNode4>& pool, const std::vector<char>& arena,
                const std::vector<int32_t>& roots, uint64_t tid, const char* rootName) noexcept
{
    if (roots.empty()) {
        return;
    }

    emitFormatted4(writer, "[perf4][tid=0x%llx] %s\n", static_cast<unsigned long long>(tid),
                   (rootName != nullptr && rootName[0] != '\0') ? rootName : "perf");

    const uint32_t nameCol = computeNameColumnWidth4(pool, roots);

    for (std::size_t i = 0; i < roots.size(); ++i) {
        const bool rootIsLast = (i + 1 == roots.size());

        struct Frame
        {
            int32_t     idx;
            std::string prefix;
            bool        isLast;
        };
        std::vector<Frame> dfs;
        dfs.push_back({roots[i], std::string(), rootIsLast});

        while (!dfs.empty()) {
            Frame cur = std::move(dfs.back());
            dfs.pop_back();

            printNodeLine4(writer, pool, arena, cur.idx, cur.prefix, cur.isLast, nameCol);

            const PerfNode4&     n = pool[static_cast<std::size_t>(cur.idx)];
            std::vector<int32_t> children;
            for (int32_t c = n.firstChild; c != -1; c = pool[static_cast<std::size_t>(c)].nextSibling) {
                children.push_back(c);
            }
            const std::string childPrefix = cur.prefix + (cur.isLast ? "    " : "|   ");
            for (std::size_t k = children.size(); k-- > 0;) {
                const bool last = (k == children.size() - 1);
                dfs.push_back({children[k], childPrefix, last});
            }
        }
    }
}


// ---------------------------------------------------------------------------
//  Aggregate collector (process-global)
// ---------------------------------------------------------------------------


struct FlushedTree4
{
    std::vector<PerfNode4> pool;
    std::vector<char>      arena;
    std::vector<int32_t>   roots;
    uint64_t               tid;
    char                   rootName[64];
    IPerfWriter4*          writer;  // resolved at flush-enqueue time
};


class AggregateCollector4
{
public:
    static AggregateCollector4& get() noexcept
    {
        static AggregateCollector4 instance;
        return instance;
    }

    bool append(FlushedTree4&& tree) noexcept
    {
        try {
            std::lock_guard<std::mutex> lk(mMutex);
            mTrees.emplace_back(std::move(tree));
            registerAtexitOnce();
            return true;
        } catch (...) {
            return false;
        }
    }

    void flush() noexcept
    {
        std::vector<FlushedTree4> local;
        try {
            std::lock_guard<std::mutex> lk(mMutex);
            local.swap(mTrees);
        } catch (...) {
            return;
        }
        if (local.empty()) {
            return;
        }
        std::stable_sort(local.begin(), local.end(),
                         [](const FlushedTree4& a, const FlushedTree4& b) { return a.tid < b.tid; });

        // Use the first tree's writer for the headers; per-tree writers are
        // honoured below. This keeps the global header on whichever sink the
        // user installed for the first contributing scope.
        IPerfWriter4& headerWriter = local.front().writer ? *local.front().writer : defaultWriter4();

        emitFormatted4(headerWriter, "[perf4] ===== aggregate flush: %zu block(s) =====\n", local.size());
        for (const FlushedTree4& t : local) {
            IPerfWriter4& w = t.writer ? *t.writer : defaultWriter4();
            printTree4(w, t.pool, t.arena, t.roots, t.tid, t.rootName);
        }
        emitFormatted4(headerWriter, "[perf4] ===== end =====\n");
    }

private:
    AggregateCollector4() = default;

    void registerAtexitOnce() noexcept
    {
        if (!mAtexitDone.exchange(true, std::memory_order_acq_rel)) {
            std::atexit(&atexitHook);
        }
    }

    static void atexitHook() noexcept { get().flush(); }

    std::mutex                mMutex;
    std::vector<FlushedTree4> mTrees;
    std::atomic<bool>         mAtexitDone{false};
};


}  // anonymous namespace


void XPerfContext4::flushAggregated() noexcept
{
    AggregateCollector4::get().flush();
}

// ===========================================================================
//  XTimer4 鈥?static helpers
// ===========================================================================

void XTimer4::sleepFor(int64_t ms) noexcept
{
    if (ms <= 0) {
        return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}


std::string XTimer4::getTimeFormatted(const char* fmt) noexcept
{
    using namespace std::chrono;
    const auto  now = system_clock::now();
    std::time_t t   = system_clock::to_time_t(now);

    std::tm tm{};
#if AU_OS_WINDOWS
    if (localtime_s(&tm, &t) != 0) {
        return {};
    }
#else
    if (localtime_r(&t, &tm) == nullptr) {
        return {};
    }
#endif

    std::array<char, 128> outBuf{};
    const std::size_t     written =
        std::strftime(outBuf.data(), outBuf.size(), (fmt != nullptr ? fmt : "%Y-%m-%d-%H-%M-%S"), &tm);
    if (written == 0) {
        return {};
    }

    const auto subMs =
        duration_cast<milliseconds>(now.time_since_epoch()).count() -
        duration_cast<seconds>(now.time_since_epoch()).count() * 1000;

    std::string out(outBuf.data(), written);
    out.push_back('_');
    out.append(std::to_string(subMs));
    return out;
}


float XTimer4::elapsedMs() const noexcept
{
    return std::chrono::duration<float, std::milli>(Clock::now() - mBegin).count();
}


// ===========================================================================
//  XTimer4Scoped
// ===========================================================================

XTimer4Scoped::XTimer4Scoped(const char* name, int32_t level) noexcept
{
    begin(XPerfContext4::defaultContext(), name, name != nullptr ? std::strlen(name) : 0u, level);
}


XTimer4Scoped::XTimer4Scoped(const char* name, std::size_t nameLen, int32_t level) noexcept
{
    begin(XPerfContext4::defaultContext(), name, nameLen, level);
}


XTimer4Scoped::XTimer4Scoped(XPerfContext4& ctx, const char* name, int32_t level) noexcept
{
    begin(ctx, name, name != nullptr ? std::strlen(name) : 0u, level);
}


XTimer4Scoped::XTimer4Scoped(XPerfContext4& ctx, const char* name, std::size_t nameLen, int32_t level) noexcept
{
    begin(ctx, name, nameLen, level);
}


void XTimer4Scoped::begin(XPerfContext4& ctx, const char* name, std::size_t nameLen, int32_t level) noexcept
{
    mCtx        = &ctx;
    mNodeIdx    = -1;
    mSubNodeIdx = -1;
    mDepth      = 0;
    mIsRoot     = false;
    mNameLen    = 0;
    mNameInline[0] = '\0';
    mBegin      = std::chrono::steady_clock::now();

    // Always copy the inline name first 鈥?even on the inactive path we
    // tolerate temporary name lifetimes. If the gate trips later we just
    // throw it away.
    if (name != nullptr && nameLen > 0) {
        const std::size_t cp = std::min(nameLen, kInlineNameCap - 1);
        std::memcpy(mNameInline, name, cp);
        mNameInline[cp] = '\0';
        mNameLen        = static_cast<uint32_t>(cp);
    }

    if (!ctx.isEnabled()) {
        return;
    }
    if (level > ctx.getTimerLevel()) {
        return;
    }

    if (ctx.getMode() == Mode4::Release) {
        mNodeIdx = -2;  // sentinel: active, no tree node 鈥?destructor prints one-liner
        return;
    }

    // 鈹€鈹€ Debug path: append to TLS pool 鈹€鈹€
    PerfThreadCtx4& tls = tlsCtx4();
    if (tls.inFlush) {
        return;  // log callback re-entered us during flush; stay silent
    }

    const uint32_t depth  = static_cast<uint32_t>(tls.openStack.size());
    const uint32_t maxDep = ctx.getMaxTreeDepth();
    if (depth >= maxDep) {
        mNodeIdx = -2;  // exceed depth: degrade to one-liner
        return;
    }

    try {
        const int32_t  idx       = static_cast<int32_t>(tls.pool.size());
        ArenaPut4Result arenaRet = arenaPut4(tls, name != nullptr ? name : "", nameLen);

        PerfNode4 node{};
        node.nameOffset       = arenaRet.offset;
        node.nameLen          = arenaRet.len;
        node.parent           = tls.openStack.empty() ? -1 : tls.openStack.back();
        node.firstChild       = -1;
        node.lastChild        = -1;
        node.nextSibling      = -1;
        node.depth            = depth;
        node.flags            = arenaRet.truncated ? kFlagTruncated4 : 0u;
        node.begin            = mBegin;
        node.durationMs       = 0.0f;
        tls.pool.push_back(node);

        if (node.parent == -1) {
            // Root-level: link sibling chain to the previous root.
            int32_t prevRoot = -1;
            for (int32_t i = idx - 1; i >= 0; --i) {
                if (tls.pool[static_cast<std::size_t>(i)].parent == -1) {
                    prevRoot = i;
                    break;
                }
            }
            if (prevRoot != -1) {
                tls.pool[static_cast<std::size_t>(prevRoot)].nextSibling = idx;
            }
        } else {
            PerfNode4& parent = tls.pool[static_cast<std::size_t>(node.parent)];
            if (parent.firstChild == -1) {
                parent.firstChild = idx;
            } else {
                tls.pool[static_cast<std::size_t>(parent.lastChild)].nextSibling = idx;
            }
            parent.lastChild = idx;
        }

        tls.openStack.push_back(idx);
        mNodeIdx = idx;
        mDepth   = depth;
        mIsRoot  = (depth == 0);
    } catch (...) {
        mNodeIdx = -2;  // OOM degrade
    }
}


XTimer4Scoped::~XTimer4Scoped() noexcept
{
    if (mCtx == nullptr || mNodeIdx == -1) {
        return;
    }

    const auto  now = std::chrono::steady_clock::now();
    const float ms  = std::chrono::duration<float, std::milli>(now - mBegin).count();

    XPerfContext4Impl* impl = mCtx->mImpl;
    if (impl == nullptr) {
        return;
    }
    IPerfWriter4& writer = resolveWriter4(*impl);

    // 鈹€鈹€ Release / degraded path: one-liner 鈹€鈹€
    if (mNodeIdx == -2) {
        emitFormatted4(writer, "[perf4] %.*s : %8.3f ms\n", static_cast<int>(mNameLen), mNameInline, ms);
        return;
    }

    // 鈹€鈹€ Debug path: close node, optionally flush thread tree 鈹€鈹€
    PerfThreadCtx4& tls = tlsCtx4();
    if (tls.inFlush) {
        return;
    }

    if (mNodeIdx >= 0 && mNodeIdx < static_cast<int32_t>(tls.pool.size())) {
        PerfNode4& node = tls.pool[static_cast<std::size_t>(mNodeIdx)];
        node.durationMs = ms;
        node.flags |= kFlagClosed4;
    }
    if (!tls.openStack.empty() && tls.openStack.back() == mNodeIdx) {
        tls.openStack.pop_back();
    }

    if (!mIsRoot || !tls.openStack.empty()) {
        return;
    }

    // 鈹€鈹€ Outermost scope: flush thread tree 鈹€鈹€
    tls.inFlush = true;

    const bool aggregate = mCtx->isAggregateMode();
    char       rootName[64];
    mCtx->getRootName(rootName, sizeof(rootName));

    try {
        if (aggregate) {
            FlushedTree4 snap;
            snap.pool   = tls.pool;
            snap.arena  = tls.nameArena;
            snap.roots  = collectRoots4(tls.pool);
            snap.tid    = tidHash4(tls.tid);
            snap.writer = &writer;
            std::memcpy(snap.rootName, rootName, sizeof(snap.rootName));
            AggregateCollector4::get().append(std::move(snap));
        } else {
            const auto roots = collectRoots4(tls.pool);
            printTree4(writer, tls.pool, tls.nameArena, roots, tidHash4(tls.tid), rootName);
        }
    } catch (...) {
        // Perf must never kill the host.
    }

    tls.pool.clear();
    tls.nameArena.clear();
    tls.openStack.clear();
    tls.inFlush = false;
}


void XTimer4Scoped::sub(const char* name) noexcept
{
    sub(name, name != nullptr ? std::strlen(name) : 0u);
}


void XTimer4Scoped::sub(const char* name, std::size_t nameLen) noexcept
{
    if (mCtx == nullptr || mNodeIdx < 0) {
        return;  // sub() is meaningful only when this scope owns a tree node
    }

    PerfThreadCtx4& tls = tlsCtx4();
    if (tls.inFlush) {
        return;
    }

    // Close the previous sub (if any) by popping it from the open stack.
    if (mSubNodeIdx >= 0 && mSubNodeIdx < static_cast<int32_t>(tls.pool.size())) {
        PerfNode4& prev = tls.pool[static_cast<std::size_t>(mSubNodeIdx)];
        if ((prev.flags & kFlagClosed4) == 0) {
            const auto now  = std::chrono::steady_clock::now();
            prev.durationMs = std::chrono::duration<float, std::milli>(now - prev.begin).count();
            prev.flags |= kFlagClosed4;
        }
        if (!tls.openStack.empty() && tls.openStack.back() == mSubNodeIdx) {
            tls.openStack.pop_back();
        }
        mSubNodeIdx = -1;
    }

    const uint32_t maxDep = mCtx->getMaxTreeDepth();
    const uint32_t depth  = mDepth + 1;
    if (depth >= maxDep) {
        return;  // depth cap: silently skip new sub
    }

    try {
        const int32_t  idx       = static_cast<int32_t>(tls.pool.size());
        ArenaPut4Result arenaRet = arenaPut4(tls, name != nullptr ? name : "", nameLen);

        PerfNode4 node{};
        node.nameOffset       = arenaRet.offset;
        node.nameLen          = arenaRet.len;
        node.parent           = mNodeIdx;
        node.firstChild       = -1;
        node.lastChild        = -1;
        node.nextSibling      = -1;
        node.depth            = depth;
        node.flags            = arenaRet.truncated ? kFlagTruncated4 : 0u;
        node.begin            = std::chrono::steady_clock::now();
        node.durationMs       = 0.0f;
        tls.pool.push_back(node);

        PerfNode4& parent = tls.pool[static_cast<std::size_t>(mNodeIdx)];
        if (parent.firstChild == -1) {
            parent.firstChild = idx;
        } else {
            tls.pool[static_cast<std::size_t>(parent.lastChild)].nextSibling = idx;
        }
        parent.lastChild = idx;

        tls.openStack.push_back(idx);
        mSubNodeIdx = idx;
    } catch (...) {
        // Drop sub silently on OOM.
    }
}


void XTimer4Scoped::sub() noexcept
{
    if (mCtx == nullptr || mNodeIdx < 0 || mSubNodeIdx < 0) {
        return;
    }

    PerfThreadCtx4& tls = tlsCtx4();
    if (tls.inFlush) {
        return;
    }

    if (mSubNodeIdx < static_cast<int32_t>(tls.pool.size())) {
        PerfNode4& prev = tls.pool[static_cast<std::size_t>(mSubNodeIdx)];
        if ((prev.flags & kFlagClosed4) == 0) {
            const auto now  = std::chrono::steady_clock::now();
            prev.durationMs = std::chrono::duration<float, std::milli>(now - prev.begin).count();
            prev.flags |= kFlagClosed4;
        }
    }
    if (!tls.openStack.empty() && tls.openStack.back() == mSubNodeIdx) {
        tls.openStack.pop_back();
    }
    mSubNodeIdx = -1;
}


float XTimer4Scoped::elapsedMs() const noexcept
{
    return std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - mBegin).count();
}


}  // namespace perf
}  // namespace au