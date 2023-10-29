#include "UAF/UseProgSlice.h"

using namespace std;

void UseProgSlice::reportUAF()
{
    for (SVFGNodeSetIter fit = sinksBegin(), feit = sinksEnd(); fit != feit; fit++)
    {
        const SVFGNode *freeNode = *fit;
        getPath(freeNode);
        // UAF
        for (SVFGNodeSetIter uit = usesBegin(), ueit = usesEnd(); uit != ueit; uit++)
        {
            Condition guard = condAnd(getVFCond(*fit), getVFCond(*uit));
            if (!isEquivalentBranchCond(guard, getFalseCond()))
            {
                const SVFGNode *useNode = *uit;
                getPath(useNode);
                if (inPath(freeNode, useNode) || !inPath(useNode, freeNode))
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
    SVF::NodeBS visited;
    worklist.push(node);
    visited.set(node->getId());
    while (!worklist.empty())
    {
        const SVFGNode *curNode = worklist.pop();
        for (auto eit = curNode->OutEdgeBegin(); eit != curNode->OutEdgeEnd(); eit++)
        {
            const SVFGEdge *edge = (*eit);
            const SVFGNode *succ = edge->getDstNode();
            if (!inBackwardSlice(succ))
                continue;
            if (!visited.test(succ->getId()))
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
    cout << "malloc: " << m->toString() << endl;
    cout << "free: " << f->toString() << endl;
    cout << "use: " << u->toString() << endl;
}