// build slice to collect malloc, free and use
// then calculate the condition between malloc, free and use by SaberCondAllocator
// need to think alias

#ifndef USEAFTERFREECHECKER_H_
#define USEAFTERFREECHECKER_H_

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
    void initUses();
    inline void addUseToCurSlice(const SVFGNode *node)
    {
        _curSlice->addToUses(node);
        addToCurForwardSlice(node);
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

  private:
    UseProgSlice *_curSlice;
    SVFGNodeSet uses;
};

#endif
