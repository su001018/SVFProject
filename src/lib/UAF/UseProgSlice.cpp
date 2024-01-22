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
                if (canBeAddedToSequence(getSource(), freeNode, useNode))
                {
                    if (isBefore(freeNode, useNode, getSource()))
                    {
                        addToSequence(getSource(), freeNode, useNode);
                    }
                }
            }
        }
        // Double free
    }
    printAnalyzeResult("uaf_result.txt");
}
void UseProgSlice::printAnalyzeResult(string file)
{
    if (getSequence().size() <= 0)
    {
        return;
    }
    std::ofstream fs_out;
    fs_out.open(file, std::ios::app);
    fs_out << "total use after free sequence number is: " << getSequence().size() << endl;
    for (string val : getSequence())
    {
        fs_out << val;
    }
    fs_out.close();
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
    if (const SVFInstruction *mInst = SVFUtil::dyn_cast<SVFInstruction>(m->getValue()))
    {
        // const SVFBasicBlock *mBB = mInst->getParent();
        // cout << "malloc bb: " << mBB->getSourceLoc() << endl;
        cout << "malloc inst: " << mInst->getSourceLoc() << endl;
    }
    if (const SVFInstruction *fInst = SVFUtil::dyn_cast<SVFInstruction>(f->getValue()))
    {
        // const SVFBasicBlock *fBB = fInst->getParent();
        // cout << "free bb: " << fBB->getSourceLoc() << endl;
        cout << "free inst: " << fInst->getSourceLoc() << endl;
    }
    if (const SVFInstruction *uInst = SVFUtil::dyn_cast<SVFInstruction>(u->getValue()))
    {
        // const SVFBasicBlock *uBB = uInst->getParent();
        // cout << "use bb: " << uBB->getSourceLoc() << endl;
        cout << "use inst: " << uInst->getSourceLoc() << endl;
    }
}
bool UseProgSlice::isBefore(const SVFGNode *src, const SVFGNode *dst, const SVFGNode *def)
{
    SVFIR *pag = PAG::getPAG();
    ICFG *icfg = pag->getICFG();

    const SVFInstruction *srcInst = SVFUtil::dyn_cast<SVFInstruction>(src->getValue());
    const ICFGNode *srcIcfg = icfg->getICFGNode(srcInst);

    const SVFInstruction *dstInst = SVFUtil::dyn_cast<SVFInstruction>(dst->getValue());
    const ICFGNode *dstIcfg = icfg->getICFGNode(dstInst);

    const SVFInstruction *defInst = SVFUtil::dyn_cast<SVFInstruction>(def->getValue());
    const ICFGNode *defIcfg = icfg->getICFGNode(defInst);

    // const ICFGNode *srcIcfg = src->getICFGNode();
    // const ICFGNode *dstIcfg = dst->getICFGNode();
    // const ICFGNode *defIcfg = def->getICFGNode();

    // SVF::FIFOWorkList<const ICFGNode *> worklist;
    // NodeBS visited;

    if (srcIcfg && dstIcfg && defIcfg)
    {
        if (srcIcfg->getId() == dstIcfg->getId())
            return false;
        CallStack *callStack = new CallStack(srcIcfg);
        if (CallStack *resultStack = CallStack::prosessReachable(callStack, dstIcfg->getId()))
        {
            if (!resultStack->isReached(defIcfg->getId()))
            {
                // resultStack->printPath("icfg_path.txt");
                return true;
            }
        }
        // worklist.push(srcIcfg);
        // visited.set(srcIcfg->getId());
        // while (!worklist.empty())
        // {
        //     const ICFGNode *cur = worklist.pop();
        //     if (cur->getId() == dstIcfg->getId())
        //     {
        //         if (isNotRedefine(srcIcfg, dstIcfg, def))
        //             return true;
        //         else
        //             return false;
        //     }
        //     for (auto it = cur->getOutEdges().begin(); it != cur->getOutEdges().end(); it++)
        //     {
        //         const ICFGEdge *edge = (*it);
        //         const ICFGNode *succ = edge->getDstNode();
        //         if (!visited.test(succ->getId()))
        //         {
        //             visited.set(succ->getId());
        //             worklist.push(succ);
        //         }
        //     }
        // }
    }
    return false;
}
bool UseProgSlice::isNotRedefine(const ICFGNode *src, const ICFGNode *dst, const SVFGNode *def)
{

    SVF::FIFOWorkList<const ICFGNode *> worklist;
    std::ofstream fs_out;
    NodeBS visited;
    std::map<int, int> father;
    std::stack<int> callStack;

    fs_out.open("icfg_path.txt", std::ios::app);
    worklist.push(src);
    father[src->getId()] = -1;

    fs_out << "src: " << src->toString() << endl;
    fs_out << "dst: " << dst->toString() << endl;

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
            fs_out << "+++++++++add node++++++++++" << endl << retNode->toString() << endl;
            callStack.push(callNode->getId());
        }
        if (CallICFGNode *callNode = SVFUtil::dyn_cast<CallICFGNode>(node))
        {
            if (callStack.empty() || callStack.top() != callNode->getId())
            {
                string message = "";
                if (callStack.empty())
                    message = "call stack is empty!";
                else
                {
                    message = "don't match [return node]: " +
                              PAG::getPAG()->getICFG()->getICFGNode(callStack.top())->toString();
                }
                fs_out << "The [call node]: " << callNode->toString() << endl
                       << "dont match because " << message << endl
                       << "it's [return node]: " << callNode->getRetICFGNode()->toString() << endl;
                fs_out.close();
                return false;
            }
            fs_out << "--------------pop node--------------" << endl << node->toString() << endl;
            callStack.pop();
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

bool CallStack::testAddICFGNode(const ICFGNode *node)
{
    int id = node->getId();
    if (isVisited(id))
        return false;
    if (const RetICFGNode *retNode = SVFUtil::dyn_cast<RetICFGNode>(node))
    {
        if (isStackEmpty())
        {
            return true;
        }
        const CallICFGNode *callNode = retNode->getCallICFGNode();
        return getStackTop() == callNode->getId();
    }
    return true;
}
bool CallStack::addICFGNode(const ICFGNode *node)
{
    if (!testAddICFGNode(node))
        return false;
    addToVisited(node->getId());
    setCurNode(node);
    addToPath(node);
    if (const CallICFGNode *callNode = SVFUtil::dyn_cast<CallICFGNode>(node))
    {
        pushToStack(callNode->getId());
        addToVisited(callNode->getId());
    }
    if (const RetICFGNode *retNode = SVFUtil::dyn_cast<RetICFGNode>(node))
    {
        popFromStack();
        addToVisited(retNode->getId());
    }
    return true;
}
list<const ICFGNode *> CallStack::findSucc()
{
    list<const ICFGNode *> res;
    const ICFGNode *node = getCurNode();
    for (auto it = node->getOutEdges().begin(); it != node->getOutEdges().end(); it++)
    {
        const ICFGEdge *edge = (*it);
        const ICFGNode *succ = edge->getDstNode();
        if (testAddICFGNode(succ))
            res.push_back(succ);
    }
    return res;
}
void CallStack::printPath(string file)
{
    std::ofstream fs_out;
    fs_out.open(file, std::ios::app);
    fs_out << "-----print path begin!-----" << endl;
    for (const ICFGNode *node : getPath())
    {
        fs_out << node->toString() << endl;
    }
    fs_out << "-----print path end!-----" << endl;
    fs_out.close();
}
void CallStack::printCallPath(string file)
{
    std::ofstream fs_out;
    fs_out.open(file, std::ios::app);
    fs_out << "-----print call path begin!-----" << endl;
    string functionName = "";
    for (const ICFGNode *node : getPath())
    {
        if (const CallICFGNode *callNode = SVFUtil::dyn_cast<CallICFGNode>(node))
        {

            string curName = callNode->getFun()->getName();
            if (functionName != curName)
            {
                functionName = curName;
                fs_out << "call: " << curName << endl;
            }
        }
        if (const RetICFGNode *retNode = SVFUtil::dyn_cast<RetICFGNode>(node))
        {
            string curName = retNode->getFun()->getName();
            if (functionName != curName)
            {
                functionName = curName;
                fs_out << "ret: " << curName << endl;
            }
        }
    }
    fs_out << "-----print call path end!-----" << endl;
    fs_out.close();
}
CallStack *CallStack::prosessReachable(CallStack *cs, int dstId)
{

    SVF::FIFOWorkList<CallStack *> worklist;
    worklist.push(cs);
    CallStackVisited visited;
    visited.addNode(cs);

    while (!worklist.empty())
    {
        CallStack *callStack = worklist.pop();

        while (true)
        {
            if (callStack->getCurNode()->getId() == dstId)
            {
                return callStack;
            }
            list<const ICFGNode *> succs = callStack->findSucc();
            if (succs.size() == 0)
            {
                delete (callStack);
                break;
            }
            for (auto it = succs.begin(); it != succs.end(); it++)
            {
                if (it == succs.begin())
                    continue;
                const ICFGNode *nextNode = (*it);
                CallStack *nextStack = new CallStack(*callStack);
                nextStack->addICFGNode(nextNode);
                if (!visited.isVisited(nextStack))
                {
                    visited.addNode(nextStack);
                    worklist.push(nextStack);
                }
                else
                    delete (nextStack);
            }
            callStack->addICFGNode(succs.front());
        }
    }
    return nullptr;
}