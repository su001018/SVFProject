#include "UAF/UseAfterFreeChecker.h"
#include "WPA/Andersen.h"

using namespace SVF;
using namespace SVFUtil;
using namespace std;

void UseAfterFreeChecker::analyze(SVFModule *module)
{
    initialize(module);
    for (SVFGNodeSetIter iter = sourcesBegin(), eiter = sourcesEnd(); iter != eiter; ++iter)
    {
        setCurSlice(*iter);

        DBOUT(DGENERAL, outs() << "Analysing slice:" << (*iter)->getId() << ")\n");
        ContextCond cxt;
        DPIm item((*iter)->getId(), cxt);
        forwardTraverse(item);

        /// do not consider there is bug when reaching a global SVFGNode
        /// if we touch a global, then we assume the client uses this memory until the program exits.
        if (getCurSlice()->isReachGlobal())
        {
            DBOUT(DSaber, outs() << "Forward analysis reaches globals for slice:" << (*iter)->getId() << ")\n");
        }
        else
        {
            DBOUT(DSaber, outs() << "Forward process for slice:" << (*iter)->getId()
                                 << " (size = " << getCurSlice()->getForwardSliceSize() << ")\n");

            for (SVFGNodeSetIter sit = getCurSlice()->sinksBegin(), esit = getCurSlice()->sinksEnd(); sit != esit;
                 ++sit)
            {
                ContextCond cxt;
                DPIm item((*sit)->getId(), cxt);
                backwardTraverse(item);
            }

            for (SVFGNodeSetIter sit = getCurSlice()->usesBegin(), esit = getCurSlice()->usesEnd(); sit != esit; sit++)
            {
                ContextCond cxt;
                DPIm item((*sit)->getId(), cxt);
                backwardTraverse(item);
            }

            DBOUT(DSaber, outs() << "Backward process for slice:" << (*iter)->getId()
                                 << " (size = " << getCurSlice()->getBackwardSliceSize() << ")\n");

            if (Options::DumpSlice())
                annotateSlice(_curSlice);

            if (_curSlice->AllPathReachableSolve())
                _curSlice->setAllReachable();

            DBOUT(DSaber, outs() << "Guard computation for slice:" << (*iter)->getId() << ")\n");
        }

        reportBug(getCurSlice());
    }
    finalize();
}

/// Set current slice
void UseAfterFreeChecker::setCurSlice(const SVFGNode *src)
{
    if (_curSlice != nullptr)
    {
        delete _curSlice;
        _curSlice = nullptr;
        clearVisitedMap();
    }

    _curSlice = new UseProgSlice(src, getSaberCondAllocator(), getSVFG());
}

/// Initialize analysis
void UseAfterFreeChecker::initialize(SVFModule *module)
{
    SVFIR *pag = PAG::getPAG();

    AndersenWaveDiff *ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
    if (Options::SABERFULLSVFG())
        svfg = memSSA.buildFullSVFG(ander);
    else
        svfg = memSSA.buildPTROnlySVFG(ander);
    setGraph(memSSA.getSVFG());
    ptaCallGraph = ander->getPTACallGraph();
    // AndersenWaveDiff::releaseAndersenWaveDiff();
    /// allocate control-flow graph branch conditions
    getSaberCondAllocator()->allocate(getPAG()->getModule());

    initSrcs();
    initSnks();
    initUses();
}

void UseAfterFreeChecker::initUses()
{
    for (auto it = svfg->begin(), eit = svfg->end(); it != eit; it++)
    {
        const SVFGNode *node = it->second;

        if (const LoadSVFGNode *loadNode = SVFUtil::dyn_cast<LoadSVFGNode>(node))
        {
            // load nodes
            addToUses(loadNode);
        }
        else if (const StoreSVFGNode *storeNode = SVFUtil::dyn_cast<StoreSVFGNode>(node))
        {
            // store nodes
            addToUses(storeNode);
        }
        else if (const GepSVFGNode *gepNode = SVFUtil::dyn_cast<GepSVFGNode>(node))
        {
            // gep nodes
            addToUses(gepNode);
        }
    }
}

void UseAfterFreeChecker::FWProcessOutgoingEdge(const DPIm &item, SVFGEdge *edge)
{
    DBOUT(DSaber, outs() << "\n##processing source: " << getCurSlice()->getSource()->getId()
                         << " forward propagate from (" << edge->getSrcID());

    // for indirect SVFGEdge, the propagation should follow the def-use chains
    // points-to on the edge indicate whether the object of source node can be propagated

    const SVFGNode *dstNode = edge->getDstNode();
    DPIm newItem(dstNode->getId(), item.getContexts());

    /// handle globals here
    if (isGlobalSVFGNode(dstNode) || getCurSlice()->isReachGlobal())
    {
        getCurSlice()->setReachGlobal();
        return;
    }

    /// perform context sensitive reachability
    // push context for calling
    if (edge->isCallVFGEdge())
    {
        CallSiteID csId = 0;
        if (const CallDirSVFGEdge *callEdge = SVFUtil::dyn_cast<CallDirSVFGEdge>(edge))
            csId = callEdge->getCallSiteId();
        else
            csId = SVFUtil::cast<CallIndSVFGEdge>(edge)->getCallSiteId();

        newItem.pushContext(csId);
        DBOUT(DSaber, outs() << " push cxt [" << csId << "] ");
    }
    // match context for return
    else if (edge->isRetVFGEdge())
    {
        CallSiteID csId = 0;
        if (const RetDirSVFGEdge *callEdge = SVFUtil::dyn_cast<RetDirSVFGEdge>(edge))
            csId = callEdge->getCallSiteId();
        else
            csId = SVFUtil::cast<RetIndSVFGEdge>(edge)->getCallSiteId();

        if (newItem.matchContext(csId) == false)
        {
            DBOUT(DSaber, outs() << "-|-\n");
            return;
        }
        DBOUT(DSaber, outs() << " pop cxt [" << csId << "] ");
    }

    /// whether this dstNode has been visited or not
    if (forwardVisited(dstNode, newItem))
    {
        DBOUT(DSaber, outs() << " node " << dstNode->getId() << " has been visited\n");
        return;
    }
    else
        addForwardVisited(dstNode, newItem);

    if (pushIntoWorklist(newItem))
        DBOUT(DSaber,
              outs() << " --> " << edge->getDstID() << ", cxt size: " << newItem.getContexts().cxtSize() << ")\n");
}

/*!
 * Propagate information backward without matching context, as forward analysis already did it
 */
void UseAfterFreeChecker::BWProcessIncomingEdge(const DPIm &, SVFGEdge *edge)
{
    DBOUT(DSaber, outs() << "backward propagate from (" << edge->getDstID() << " --> " << edge->getSrcID() << ")\n");
    const SVFGNode *srcNode = edge->getSrcNode();
    if (backwardVisited(srcNode))
        return;
    else
        addBackwardVisited(srcNode);

    ContextCond cxt;
    DPIm newItem(srcNode->getId(), cxt);
    pushIntoWorklist(newItem);
}

void UseAfterFreeChecker::reportBug(UseProgSlice *slice)
{
    slice->reportUAF();
}
