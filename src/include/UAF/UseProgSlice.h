#ifndef USEPROGSLICE_H_
#define USEPROGSLICE_H_

#include "SABER/ProgSlice.h"

using namespace SVF;
class UseProgSlice : public ProgSlice
{
  public:
    typedef Set<const SVFGNode *> SVFGNodeSet;
    typedef SVFGNodeSet::const_iterator SVFGNodeSetIter;
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

  private:
    SVFGNodeSet uses;
};

#endif