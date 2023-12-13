#ifndef USEPROGSLICE_H_
#define USEPROGSLICE_H_

#include "SABER/ProgSlice.h"

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
    void reportUAF();
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
};

class callStack
{
  public:
    string getStackValue()
    {
        return stackValue;
    }
    string stackToString()
    {
        string val = "";
        for (int id : st)
        {
            val += id + ",";
        }
        if (val == "")
            return val;
        return val.substr(0, val.length() - 1);
    }
    void pushToStack(int id)
    {
        st.push_back(id);
        stackValue = stackToString();
    }
    int popFromStack()
    {
        if (st.empty())
        {
            cout << "callStack is empty!" << endl;
            return -1;
        }
        int id = st.back();
        st.pop_back();
        stackValue = stackToString();
        return id;
    }
    bool isVisited(int id)
    {
        if (visited.count(stackValue) == 0)
            return false;
        return visited[stackValue].test(id);
    }
    bool addICFGNode(const ICFGNode *node)
    {
        int id = node->getId();
        if (isVisited(id))
            return false;
        }

  private:
    list<int> st;
    map<string, NodeBS> visited;
    string stackValue;
};

#endif
