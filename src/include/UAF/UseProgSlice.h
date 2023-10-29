#ifndef USEPROGSLICE_H_
#define USEPROGSLICE_H_

#include "SABER/ProgSlice.h"

using namespace SVF;
class UseProgSlice : public ProgSlice
{
  public:
    typedef Set<const SVFGNode *> SVFGNodeSet;
    typedef SVFGNodeSet::const_iterator SVFGNodeSetIter;
    typedef Map<int, SVFGNodeSet> SVFGIdToSet;
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

  private:
    SVFGNodeSet uses;
    SVFGIdToSet path;
};

#endif