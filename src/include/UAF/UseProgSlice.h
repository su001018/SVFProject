#ifndef USEPROGSLICE_H_
#define USEPROGSLICE_H_

#include "SABER/ProgSlice.h"
#include <mutex>

using namespace SVF;
using namespace std;
class UseProgSlice : public ProgSlice
{
  public:
    typedef Set<const SVFGNode *> SVFGNodeSet;
    typedef SVFGNodeSet::const_iterator SVFGNodeSetIter;
    typedef Map<int, SVFGNodeSet> SVFGIdToSet;
    typedef List<const ICFGNode *> ICFGNodeList;
    typedef List<const SVFGNode *> SVFGNodeList;
    UseProgSlice(const SVFGNode *src, SaberCondAllocator *pa, const SVFG *graph) : ProgSlice(src, pa, graph)
    {
    }
    virtual ~UseProgSlice()
    {
    }
    inline void addToUses(const SVFGNode *node)
    {
        uses.insert(node);
    }
    inline const SVFGNodeSet &getUses() const
    {
        return uses;
    }
    inline SVFGNodeSetIter usesBegin() const
    {
        return uses.begin();
    }
    inline SVFGNodeSetIter usesEnd() const
    {
        return uses.end();
    }
    inline set<string> getSequence() const
    {
        return sequence;
    }
    void increaseCount()
    {
        std::lock_guard<std::mutex> lock(mutex);
        count++;
    }
    int getCount()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return count;
    }
    string getSourceValueFromSVFGNode(const SVFGNode *node)
    {
        const SVFInstruction *inst = SVFUtil::dyn_cast<SVFInstruction>(node->getValue());
        if (inst)
        {
            return inst->getSourceLoc();
        }
        return "";
    }
    string joinValue(string mval, string fval, string uval)
    {
        string val = "malloc: " + mval + "\n" + +"free: " + fval + "\n" + "use: " + uval + "\n";
        return val;
    }
    bool canBeAddedToSequence(const SVFGNode *m, const SVFGNode *f, const SVFGNode *u)
    {
        string mval = getSourceValueFromSVFGNode(m);
        string fval = getSourceValueFromSVFGNode(f);
        string uval = getSourceValueFromSVFGNode(u);
        if (mval != "" && fval != "" && uval != "")
        {
            string val = joinValue(mval, fval, uval);
            if (sequence.find(val) == sequence.end())
                return true;
        }
        return false;
    }
    void addToSequence(const SVFGNode *m, const SVFGNode *f, const SVFGNode *u)
    {
        string mval = getSourceValueFromSVFGNode(m);
        string fval = getSourceValueFromSVFGNode(f);
        string uval = getSourceValueFromSVFGNode(u);
        sequence.insert(joinValue(mval, fval, uval));
    }
    void reportUAF();
    void printAnalyzeResult(string file);
    void getPath(const SVFGNode *node);
    bool inPath(const SVFGNode *src, const SVFGNode *dst);
    void putUAFResult(const SVFGNode *m, const SVFGNode *f, const SVFGNode *u);
    bool isBefore(const SVFGNode *src, const SVFGNode *dst, const SVFGNode *def);
    bool isNotRedefine(const ICFGNode *src, const ICFGNode *dst, const SVFGNode *def);
    // get all compare nodes which arrive dst
    SVFGNodeList getCompareNodeList(const SVFGNode *dst);

  private:
    SVFGNodeSet uses;
    SVFGIdToSet path;
    set<string> sequence;
    int count;
    std::mutex mutex;
};

class CallStack
{
  public:
    CallStack()
    {
    }
    CallStack(const ICFGNode *node)
    {
        curNode = node;
    }
    CallStack(const CallStack &cs)
    {
        for (const auto &item : cs.getStack())
        {
            st.push_back(item);
        }
        for (const auto &pair : cs.getVisited())
        {
            visited.insert(pair);
        }
        for (const auto &item : cs.getPath())
        {
            path.push_back(item);
        }
        curNode = cs.getCurNode();
        stackValue = cs.getStackValue();
    }
    string getStackValue() const
    {
        return stackValue;
    }
    void setStackValue(string val)
    {
        stackValue = val;
    }
    const ICFGNode *getCurNode() const
    {
        return curNode;
    }
    list<const ICFGNode *> getPath() const
    {
        return path;
    }
    void setCurNode(const ICFGNode *node)
    {
        curNode = node;
    }
    list<int> getStack() const
    {
        return st;
    }
    map<string, NodeBS> getVisited() const
    {
        return visited;
    }
    bool isStackEmpty()
    {
        return st.empty();
    }
    int getStackTop()
    {
        return st.back();
    }
    string stackToString()
    {
        std::ostringstream oss;
        auto it = st.begin();
        while (it != st.end())
        {
            oss << *it;
            if (++it != st.end())
            {
                oss << ",";
            }
        }

        std::string result = oss.str();
        return result;
    }
    void pushToStack(int id)
    {
        st.push_back(id);
        setStackValue(stackToString());
    }
    int popFromStack()
    {
        if (st.empty())
        {
            return -1;
        }
        int id = st.back();
        st.pop_back();
        setStackValue(stackToString());
        return id;
    }
    bool isVisited(int id)
    {
        if (visited.count(getStackValue()) == 0)
            return false;
        return visited[getStackValue()].test(id);
    }
    void addToVisited(int id)
    {
        visited[getStackValue()].set(id);
    }
    void addToPath(const ICFGNode *node)
    {
        path.push_back(node);
    }
    bool isReached(int id)
    {
        for (auto &pair : visited)
        {
            if (pair.second.test(id))
                return true;
        }
        return false;
    }
    bool operator<(const CallStack &other) const
    {
        if (this->stackValue < other.getStackValue())
            return true;
        if (this->stackValue > other.getStackValue())
            return false;
        return this->curNode->getId() < other.getCurNode()->getId();
    }
    bool testAddICFGNode(const ICFGNode *node);
    list<const ICFGNode *> findSucc();
    bool addICFGNode(const ICFGNode *node);
    void printPath(string file);
    void printCallPath(string file);
    static CallStack *prosessReachable(CallStack *cs, int dstId);

  private:
    list<int> st;
    list<const ICFGNode *> path;
    map<string, NodeBS> visited;
    string stackValue;
    const ICFGNode *curNode;
};

struct ComparePtr
{
    bool operator()(const CallStack *ptr1, const CallStack *ptr2) const
    {
        if (ptr1->getStackValue() != ptr2->getStackValue())
        {
            return ptr1->getStackValue() < ptr2->getStackValue();
        }
        return ptr1->getCurNode()->getId() < ptr2->getCurNode()->getId();
    }
};
class CallStackVisited
{
  public:
    bool isVisited(CallStack *callStack)
    {
        return visited.count(callStack->getStackValue()) != 0 &&
               visited[callStack->getStackValue()].test(callStack->getCurNode()->getId());
    }
    void addNode(CallStack *callStack)
    {
        visited[callStack->getStackValue()].set(callStack->getCurNode()->getId());
    }

  private:
    map<string, NodeBS> visited;
};

#endif
