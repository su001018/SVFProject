#include "UAF/UseProgSlice.h"
#include "SVF-LLVM/LLVMUtil.h"

using namespace std;
using namespace SVF;
using namespace SVFUtil;
using namespace llvm;

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
                if (isBefore(freeNode, useNode, getSource()))
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
        for (auto eit = curNode->InEdgeBegin(); eit != curNode->InEdgeEnd(); eit++)
        {
            const SVFGEdge *edge = (*eit);
            const SVFGNode *succ = edge->getSrcNode();
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

    cout << "malloc: " << m->toString() << endl;
    cout << "free: " << f->toString() << endl;
    cout << "use: " << u->toString() << endl;
    // if (const SVFInstruction *mInst = SVFUtil::dyn_cast<SVFInstruction>(m->getValue()))
    // {
    //     const SVFBasicBlock *mBB = mInst->getParent();
    //     cout << "malloc bb: " << mBB->toString() << endl;
    // }
    // if (const SVFInstruction *fInst = SVFUtil::dyn_cast<SVFInstruction>(f->getValue()))
    // {
    //     const SVFBasicBlock *fBB = fInst->getParent();
    //     cout << "free bb: " << fBB->toString() << endl;
    // }
    // if (const SVFInstruction *uInst = SVFUtil::dyn_cast<SVFInstruction>(u->getValue()))
    // {
    //     const SVFBasicBlock *uBB = uInst->getParent();
    //     cout << "use bb: " << uBB->toString() << endl;
    // }
}
bool UseProgSlice::isBefore(const SVFGNode *src, const SVFGNode *dst, const SVFGNode *def)
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
                if (isNotRedefine(srcIcfg, dstIcfg, def))
                    return true;
                else
                    return false;
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
bool UseProgSlice::isNotRedefine(const ICFGNode *src, const ICFGNode *dst, const SVFGNode *def)
{

    SVF::FIFOWorkList<const ICFGNode *> worklist;
    std::ofstream fs_out;
    NodeBS visited;
    std::map<int, int> father;
    std::<int>callStack;

    fs_out.open("icfg_path.txt", std::ios::app);
    worklist.push(src);
    father[src->getId()] = -1;

    while (!worklist.empty())
    {
        const ICFGNode *cur = worklist.pop();
        if (cur->getId() == dst->getId())
        {
            break;
        }
        for (auto it = cur->getOutEdges().begin(); it != cur->getOutEdges().end(); it++)
        {
            const ICFGEdge *edge = (*it);
            const ICFGNode *succ = edge->getDstNode();
            if (father.count(succ->getId()) == 0)
            {
                father[succ->getId()] = cur->getId();
                worklist.push(succ);
            }
        }
    }
    int curId = dst->getId();
    std::stack<int> path;
    while (curId != -1)
    {
        ICFGNode *node = PAG::getPAG()->getICFG()->getICFGNode(curId);
        for (const SVFGNode *svfgNode : node->getVFGNodes())
        {
            if (svfgNode->getId() == def->getId())
            {
                fs_out << "The path from [free node]: " << src->toString() << endl
                       << "to [use node]: " << dst->toString() << endl
                       << "has reached [malloc node]: " << node->toString() << endl;
                fs_out.close();
                return false;
            }
        }
        if (RetICFGNode *retNode = SVFUtil::dyn_cast<RetICFGNode>(node))
        {
            const CallICFGNode *callNode = retNode->getCallICFGNode();
            if (!callNode)
            {
                fs_out << "The [return node]: " << retNode->toString() << endl << "dont have a [call node]!" << endl;
                fs_out.close();
                return false;
            }
            callStack.push(callNode->getId());
        }
        if (CallICFGNode *callNode = SVFUtil::dyn_cast<CallICFGNode>(node))
        {
            if ()
        }
        path.push(curId);
        curId = father[curId];
    }
    fs_out << "path start!" << endl;
    while (!path.empty())
    {
        int id = path.top();
        path.pop();
        ICFGNode *node = PAG::getPAG()->getICFG()->getICFGNode(id);
        fs_out << "node: " << node->toString() << endl;
    }
    fs_out << "path end!" << endl;
    fs_out.close();
    return true;
}
List<const SVFGNode *> UseProgSlice::getCompareNodeList(const SVFGNode *dst)
{

    SVFIR *pag = PAG::getPAG();
    ICFG *icfg = pag->getICFG();

    const SVFInstruction *dstInst = SVFUtil::dyn_cast<SVFInstruction>(dst->getValue());
    const ICFGNode *dstIcfg = icfg->getICFGNode(dstInst);

    SVF::FIFOWorkList<const ICFGNode *> worklist;
    NodeBS visited;

    SVFGNodeList list;

    if (dstIcfg)
    {
        worklist.push(dstIcfg);
        visited.set(dstIcfg->getId());
        while (!worklist.empty())
        {
            const ICFGNode *cur = worklist.pop();

            if (const IntraICFGNode *intraIcfg = SVFUtil::dyn_cast<IntraICFGNode>(cur))
            {
                const SVFInstruction *curInst = intraIcfg->getInst();
                auto llvmVal = LLVMModuleSet::getLLVMModuleSet()->getLLVMValue(curInst);
                if (llvmVal)
                {
                    if (const llvm::CmpInst *I = llvm::dyn_cast<llvm::CmpInst>(llvmVal))
                    {
                        for (const SVFGNode *node : cur->getVFGNodes())
                        {
                            list.push(node);
                            cout << "node: " << node->toString() << endl;
                        }
                    }
                }
            }

            for (auto it = cur->getInEdges().begin(); it != cur->getInEdges().end(); it++)
            {
                const ICFGEdge *edge = (*it);
                const ICFGNode *succ = edge->getSrcNode();
                if (!visited.test(succ->getId()))
                {
                    visited.set(succ->getId());
                    worklist.push(succ);
                }
            }
        }
    }
    return list;
}