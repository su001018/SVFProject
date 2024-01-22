// build slice to collect malloc, free and use
// then calculate the condition between malloc, free and use by SaberCondAllocator
// need to think alias

#ifndef USEAFTERFREECHECKER_H_
#define USEAFTERFREECHECKER_H_

#include "MTA/FSMPTA.h"
#include "MTA/LockAnalysis.h"
#include "MTA/MHP.h"
#include "MTA/TCT.h"
#include "SABER/LeakChecker.h"
#include "UAF/UseProgSlice.h"

using namespace SVF;
/*!
 * Use after free checker to check
 */

class UseAfterFreeChecker : public LeakChecker
{

  public:
    /// Constructor
    UseAfterFreeChecker() : LeakChecker()
    {
        _curSlice = nullptr;
    }

    /// Destructor
    virtual ~UseAfterFreeChecker(){

    };

    void analyze(SVFModule *module) override;
    /// Set current slice
    void setCurSlice(const SVFGNode *src) override;
    void initialize(SVFModule *module) override;
    void initSrcs() override;
    void initUses();
    inline void addUseToCurSlice(const SVFGNode *node)
    {
        _curSlice->addToUses(node);
        addToCurForwardSlice(node);
    }
    inline void addSinkToCurSlice(const SVFGNode *node)
    {
        _curSlice->addToSinks(node);
        addToCurForwardSlice(node);
    }
    inline bool isInCurForwardSlice(const SVFGNode *node)
    {
        return _curSlice->inForwardSlice(node);
    }
    inline bool isInCurBackwardSlice(const SVFGNode *node)
    {
        return _curSlice->inBackwardSlice(node);
    }
    inline void addToCurForwardSlice(const SVFGNode *node)
    {
        _curSlice->addToForwardSlice(node);
    }
    inline void addToCurBackwardSlice(const SVFGNode *node)
    {
        _curSlice->addToBackwardSlice(node);
    }
    inline bool isUse(const SVFGNode *node)
    {
        return getUses().find(node) != getUses().end();
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
    inline void addToUses(const SVFGNode *node)
    {
        uses.insert(node);
    }
    inline UseProgSlice *getCurSlice()
    {
        return _curSlice;
    }

  protected:
    void reportBug(UseProgSlice *slice);
    /// Forward traverse
    inline void FWProcessCurNode(const DPIm &item) override
    {
        const SVFGNode *node = getNode(item.getCurNodeID());
        if (isSink(node))
        {
            addSinkToCurSlice(node);
            _curSlice->setPartialReachable();
        }
        if (isUse(node))
        {
            addUseToCurSlice(node);
        }
        else
            addToCurForwardSlice(node);
    }
    /// Backward traverse
    inline void BWProcessCurNode(const DPIm &item) override
    {
        const SVFGNode *node = getNode(item.getCurNodeID());
        if (isInCurForwardSlice(node))
        {
            addToCurBackwardSlice(node);
        }
    }

    /// Propagate information forward by matching context
    void FWProcessOutgoingEdge(const DPIm &item, SVFGEdge *edge) override;
    /// Propagate information backward without matching context, as forward analysis already did it
    void BWProcessIncomingEdge(const DPIm &item, SVFGEdge *edge) override;

  private:
    UseProgSlice *_curSlice;
    SVFGNodeSet uses;
};

#endif
