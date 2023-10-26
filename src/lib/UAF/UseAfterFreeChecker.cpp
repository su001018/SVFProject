#include "UAF/UseAfterFreeChecker.h"
#include "WPA/Andersen.h"

using namespace SVF;
using namespace SVFUtil;
using namespace std;

void UseAfterFreeChecker::analyze(SVFModule *module)
{
    initialize(module);
    std::cout << "UseAfterFreeChecker::analyze" << std::endl;
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
    SVFIR *pag = getPAG();
    SVF::SVFStmt::SVFStmtSetTy loadStmtSet = pag->getSVFStmtSet(SVF::SVFStmt::PEDGEK::Load);
    SVF::SVFStmt::SVFStmtSetTy storeStmtSet = pag->getSVFStmtSet(SVF::SVFStmt::PEDGEK::Store);
    SVF::SVFStmt::SVFStmtSetTy gepStmtSet = pag->getSVFStmtSet(SVF::SVFStmt::PEDGEK::Gep);
    // add load stmt to uses
    for (SVF::SVFStmt *edge : loadStmtSet)
    {
        const PAGNode *dstPag = edge->getDstNode();
        if (svfg->hasDefSVFGNode(dstPag))
        {
            const SVFGNode *dst = svfg->getDefSVFGNode(dstPag);
            cout << "dst: " << dst->toString() << endl;
            addToUses(dst);
        }
        const StmtSVFGNode *stmt = svfg->getStmtVFGNode(edge);
        cout << "load: " << stmt->toString() << endl;
    }
    // add store stmt to uses
    for (SVF::SVFStmt *edge : storeStmtSet)
    {
    }
    // add gep stmt to uses
    for (SVF::SVFStmt *edge : gepStmtSet)
    {
    }
}
