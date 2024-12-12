#include <iomanip>
#include <sstream>
#include "timer.h"

#define NODE_PREFIX(node) nodePrefix(node).c_str()

#define CHECK_TIMER_THREAD() if ( !(inThisThread()) ) { return; }


namespace perf {

    size_t LevelVisible = -1;

    inline std::string symbolicLine(size_t n, std::string symbol)
    {
        std::string ret;
        for (int i = 0; i < n; ++i) {
            ret += symbol;
        }
        return ret;
    }


    /*********************************************************
     * class Timer
     * 
     */

    void Timer::sleepfor(long long ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    std::string Timer::getTimeFormatted(std::string fmt)
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        auto tFmt = std::put_time(std::localtime(&t), fmt.c_str());
        std::stringstream ss;
        ss << tFmt;
        return ss.str();
    }

    Timer::Timer()
    {
        restart();
    }

    void Timer::restart()
    {
        mBegin = std::chrono::steady_clock::now();
    }

    float Timer::count()
    {
        mEnd = std::chrono::steady_clock::now();
        mDuration = mEnd - mBegin;
        return mDuration.count();
    }


    /*********************************************************
     * class TimerNode
     * 
     */

    class TimerNode {
    public:
        TimerNode(size_t level = 0, std::string name = "unnamed") {
            mName = name;
            mLevel = level;
            mTimer.restart();
        }

        void reset() {
            mDuration = mTimer.count();
        }
    
    public:
        std::string             mName;
        Timer                   mTimer;
        float                   mDuration;

        size_t                  mLevel;

        std::vector<TimerNode>  mNodes;
    };

    inline std::string nodePrefix(const TimerNode& node)
    {
        std::string line;

        if (node.mLevel == 0) return line;

        for (int i = 0; i < node.mLevel - 1; ++i) {
            line += "|" + symbolicLine(SIZE_INDENT, " "); // │
        }

        return line + "|" + symbolicLine(SIZE_INDENT, "-"); // ├ ─
    }


    /*********************************************************
     * class TimerTree
     * 
     */

    class TimerTree {
    public:
        static TimerTree& get() {
            static TimerTree instance;
            return instance;
        }

        ~TimerTree() {
            joinLevelTo(0); // shut up all active node
            mRoot.reset(); // reset the root node
            showByDFS(mRoot);
            mRoot.mNodes.clear();
        }

        void setName(std::string name) {
            mRoot.mName = name;
        }

        void addNode(std::string name = "unnamed") {
            CHECK_TIMER_THREAD();
            // increasing current level after adding
            auto& nodeActived = findActiveNodeInCurrentLevel(mLevelNow++);
            // increasing node level
            nodeActived.mNodes.emplace_back(TimerNode(nodeActived.mLevel + 1, name));
        }

        void resetActiveNode() {
            CHECK_TIMER_THREAD();
            // decreasing current level after reset
            auto& nodeActived = findActiveNodeInCurrentLevel(mLevelNow--);
            nodeActived.reset();
        }

        size_t getCurrentLevel() const {
            return mLevelNow;
        }

        void joinLevelTo(size_t level) {
            size_t levelNow = mLevelNow; // backup level current, mLevelNow will be changed by resetActiveNode()
            for (int i = level; i < levelNow; ++i) {
                resetActiveNode();
            }
        }

    private:
        TimerTree() : mLevelNow(0) {
            mThreadId = std::this_thread::get_id();
            mRoot = TimerNode();
        }

        bool inThisThread() {
            return mThreadId == std::this_thread::get_id();
        }

        // depth first search
        void showByDFS(const TimerNode& node) {
            printf("%s%s: %.2f ms\n", NODE_PREFIX(node), node.mName.c_str(), node.mDuration);
            for (auto& n : node.mNodes) {
                showByDFS(n);
            }
        }

        TimerNode& findActiveNodeInCurrentLevel(size_t level) {
            TimerNode* temp = &mRoot;
            for (int i = 0; i < level; ++i) {
                temp = &(temp->mNodes.back()); // the active node always locates in the end
            }
            return *temp;
        }
    
    private:
        std::thread::id mThreadId;
        TimerNode       mRoot;
        size_t          mLevelNow;
    };


    void setTimerRootName(std::string name)
    {
        TimerTree::get().setName(name);
    }

    void setPerfVisibleLevel(size_t level)
    {
        LevelVisible = level;
    }


    /*********************************************************
     * class TimerScoped
     * 
     */

    TimerScoped::TimerScoped(std::string name)
    {
        mLevelBegin = TimerTree::get().getCurrentLevel();

        if (mLevelBegin < LevelVisible) { // mLevelBegin + 1 <= LevelVisible
            TimerTree::get().addNode(name);
        }
    }

    TimerScoped::~TimerScoped()
    {
        TimerTree::get().joinLevelTo(mLevelBegin);
    }

    void TimerScoped::sub(std::string name)
    {
        if (mLevelBegin + 1 < LevelVisible) {
            sub(); // finish the last one first
            TimerTree::get().addNode(name);
        }
    }

    void TimerScoped::sub()
    {
        // level of scoped timer itself is (mLevelBegin + 1)
        TimerTree::get().joinLevelTo(mLevelBegin + 1);
    }

    size_t TimerScoped::getLevel() const
    {
        return mLevelBegin + 1;
    }

} // namespace perf