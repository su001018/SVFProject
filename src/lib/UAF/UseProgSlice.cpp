#include "UAF/UseProgSlice.h"

using namespace std;
using namespace SVF;
using namespace SVFUtil;

void UseProgSlice::reportUAF()
{
    for (SVFGNodeSetIter fit = sinksBegin(), feit = sinksEnd(); fit != feit; fit++)
    {
        const SVFGNode *freeNode = *fit;

        // UAF
        for (SVFGNodeSetIter uit = usesBegin(), ueit = usesEnd(); uit != ueit; uit++)
        {
            Condition guard = condAnd(getVFCond(*fit), getVFCond(*uit));
            if (!isEquivalentBranchCond(guard, getFalseCond()))
            {
                const SVFGNode *useNode = *uit;
                if (isBefore(freeNode, useNode))
                {
                    putUAFResult(getSource(), freeNode, useNode);
                }
            }
        }
        // Double free
    }
}

void UseProgSlice::getPath(const SVFGNode *node)
{
    if (path.count(node->getId()))
        return;
    SVF::FIFOWorkList<const SVFGNode *> worklist;
    worklist.push(node);
    while (!worklist.empty())
    {
        const SVFGNode *curNode = worklist.pop();
        for (auto eit = curNode->OutEdgeBegin(); eit != curNode->OutEdgeEnd(); eit++)
        {
            const SVFGEdge *edge = (*eit);
            const SVFGNode *succ = edge->getDstNode();
            if (!inBackwardSlice(succ))
                continue;
            if (!inPath(node, succ))
            {
                worklist.push(succ);
                path[node->getId()].insert(succ);
            }
        }
    }
}
bool UseProgSlice::inPath(const SVFGNode *src, const SVFGNode *dst)
{
    return path.count(src->getId()) && path[src->getId()].find(dst) != path[src->getId()].end();
}
void UseProgSlice::putUAFResult(const SVFGNode *m, const SVFGNode *f, const SVFGNode *u)
{
    cout << "---------------------------" << endl;
    if (const SVFInstruction *mInst = SVFUtil::dyn_cast<SVFInstruction>(m->getValue()))
    {
        const SVFBasicBlock *mBB = mInst->getParent();
        cout << "malloc bb: " << mBB->toString() << endl;
    }
    if (const SVFInstruction *fInst = SVFUtil::dyn_cast<SVFInstruction>(f->getValue()))
    {
        const SVFBasicBlock *fBB = fInst->getParent();
        cout << "free bb: " << fBB->toString() << endl;
    }
    if (const SVFInstruction *uInst = SVFUtil::dyn_cast<SVFInstruction>(u->getValue()))
    {
        const SVFBasicBlock *uBB = uInst->getParent();
        cout << "use bb: " << uBB->toString() << endl;
    }
}
bool UseProgSlice::isBefore(const SVFGNode *src, const SVFGNode *dst)
{
    SVFIR *pag = PAG::getPAG();
    ICFG *icfg = pag->getICFG();

    const SVFInstruction *srcInst = SVFUtil::dyn_cast<SVFInstruction>(src->getValue());
    const ICFGNode *srcIcfg = icfg->getICFGNode(srcInst);

    const SVFInstruction *dstInst = SVFUtil::dyn_cast<SVFInstruction>(dst->getValue());
    const ICFGNode *dstIcfg = icfg->getICFGNode(dstInst);

    SVF::FIFOWorkList<const ICFGNode *> worklist;
    NodeBS visited;

    if (srcIcfg && dstIcfg)
    {
        if (srcIcfg->getId() == dstIcfg->getId())
            return false;
        worklist.push(srcIcfg);
        visited.set(srcIcfg->getId());
        while (!worklist.empty())
        {
            const ICFGNode *cur = worklist.pop();
            if (cur->getId() == dstIcfg->getId())
            {
                return true;
            }
            for (auto it = cur->getOutEdges().begin(); it != cur->getOutEdges().end(); it++)
            {
                const ICFGEdge *edge = (*it);
                const ICFGNode *succ = edge->getDstNode();
                if (!visited.test(succ->getId()))
                {
                    visited.set(succ->getId());
                    worklist.push(succ);
                }
            }
        }
    }
    return false;
}