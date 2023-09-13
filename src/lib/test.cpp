#include "AndersenPTA.h"
#include "Graphs/SVFG.h"
#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "UseLine.h"
#include "Util/Options.h"
#include "WPA/Andersen.h"

using namespace llvm;
using namespace std;
using namespace SVF;

static llvm::cl::opt<std::string> InputFilename(cl::Positional, llvm::cl::desc("<input bitcode>"), llvm::cl::init("-"));

/*!
 * An example to query alias results of two LLVM values
 */
SVF::AliasResult aliasQuery(PointerAnalysis *pta, Value *v1, Value *v2)
{
    SVFValue *val1 = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(v1);
    SVFValue *val2 = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(v2);

    return pta->alias(val1, val2);
}

/*!
 * An example to print points-to set of an LLVM value
 */
std::string printPts(PointerAnalysis *pta, Value *val)
{

    std::string str;
    raw_string_ostream rawstr(str);
    SVFValue *svfval = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(val);

    NodeID pNodeId = pta->getPAG()->getValueNode(svfval);
    const PointsTo &pts = pta->getPts(pNodeId);
    for (PointsTo::iterator ii = pts.begin(), ie = pts.end(); ii != ie; ii++)
    {
        rawstr << " " << *ii << " ";
        PAGNode *targetObj = pta->getPAG()->getGNode(*ii);
        if (targetObj->hasValue())
        {
            rawstr << "(" << targetObj->getValue()->toString() << ")\t ";
        }
    }

    return rawstr.str();
}

/*!
 * An example to query/collect all successor nodes from a ICFGNode (iNode) along control-flow graph (ICFG)
 */
void traverseOnICFG(ICFG *icfg, const Instruction *inst)
{
    SVFInstruction *svfinst = LLVMModuleSet::getLLVMModuleSet()->getSVFInstruction(inst);

    ICFGNode *iNode = icfg->getICFGNode(svfinst);
    FIFOWorkList<const ICFGNode *> worklist;
    Set<const ICFGNode *> visited;
    worklist.push(iNode);

    /// Traverse along VFG
    while (!worklist.empty())
    {
        const ICFGNode *iNode = worklist.pop();
        for (ICFGNode::const_iterator it = iNode->OutEdgeBegin(), eit = iNode->OutEdgeEnd(); it != eit; ++it)
        {
            ICFGEdge *edge = *it;
            ICFGNode *succNode = edge->getDstNode();
            if (visited.find(succNode) == visited.end())
            {
                visited.insert(succNode);
                worklist.push(succNode);
            }
        }
    }
}

/*!
 * An example to query/collect all the uses of a definition of a value along value-flow graph (VFG)
 */
void traverseOnVFG(const SVFG *vfg, PAGNode *val)
{
    // SVFIR *pag = SVFIR::getPAG();
    // SVFValue *svfval = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(val);
    // PAGNode *pNode = pag->getGNode(pag->getValueNode(svfval));
    PAGNode *pNode = val;
    string valStr = pNode->toString();
    printf("----------------------------------------------\n");
    printf("traverseOnVFG: the valStr is %s.\n", valStr.c_str());
    const VFGNode *vNode = vfg->getDefSVFGNode(pNode);
    FIFOWorkList<const VFGNode *> worklist;
    Set<const VFGNode *> visited;
    worklist.push(vNode);

    /// Traverse along VFG
    while (!worklist.empty())
    {
        const VFGNode *vNode = worklist.pop();
        for (VFGNode::const_iterator it = vNode->OutEdgeBegin(), eit = vNode->OutEdgeEnd(); it != eit; ++it)
        {
            VFGEdge *edge = *it;
            VFGNode *succNode = edge->getDstNode();
            if (visited.find(succNode) == visited.end())
            {
                visited.insert(succNode);
                worklist.push(succNode);
            }
        }
    }

    /// Collect all LLVM Values
    for (Set<const VFGNode *>::const_iterator it = visited.begin(), eit = visited.end(); it != eit; ++it)
    {
        const VFGNode *node = *it;
        /// can only query VFGNode involving top-level pointers (starting with % or @ in LLVM IR)
        // const PAGNode *pNode = vfg->getLHSTopLevPtr(node);
        // const SVFValue *val = pNode->getValue();
        string str = node->toString();
        printf("traverseOnVFG: the SVFValue is %s\n", str.c_str());
    }
}

Set<const SVFGNode *> getAddr(SVFG *vfg)
{
    Set<const SVFGNode *> res;
    for (SVFG::const_iterator it = vfg->begin(), eit = vfg->end(); it != eit; it++)
    {
        const SVFGNode *node = it->second;
        if (node->getNodeKind() == VFGNode::Addr)
        {
            res.insert(node);
        }
    }
    return res;
}

Set<UseLine *> getUseLines(Set<const SVFGNode *> svfgNodes, SVFG *vfg)
{
    Set<UseLine *> res;
    for (const SVFGNode *node : svfgNodes)
    {
        UseLine *useLine = new UseLine(node, vfg);
        res.insert(useLine);
    }
    return res;
}

int main(int argc, char **argv)
{

    int arg_num = 0;
    char **arg_value = new char *[argc];
    std::vector<std::string> moduleNameVec;
    LLVMUtil::processArguments(argc, argv, arg_num, arg_value, moduleNameVec);
    cl::ParseCommandLineOptions(arg_num, arg_value, "Whole Program Points-to Analysis\n");
    if (Options::WriteAnder() == "ir_annotator")
    {
        LLVMModuleSet::getLLVMModuleSet()->preProcessBCs(moduleNameVec);
    }

    SVFModule *svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);

    /// Build Program Assignment Graph (SVFIR)
    SVFIRBuilder builder(svfModule);
    SVFIR *pag = builder.build();
    pag->dump("pag");

    /// Create Andersen's pointer analysis
    // Andersen* ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
    // printf("Andersen build ssuccess!");
    /// Query aliases
    /// aliasQuery(ander,value1,value2);

    /// Print points-to information
    /// printPts(ander, value1);

    /// Call Graph
    // PTACallGraph* callgraph = ander->getPTACallGraph();
    // printf("PTACallGraph build ssuccess!");
    /// ICFG
    // ICFG* icfg = pag->getICFG();
    // printf("ICFG build ssuccess!");
    /// Value-Flow Graph (VFG)
    // VFG* vfg = new VFG(callgraph);
    // printf("VFG build ssuccess!");
    /// Sparse value-flow graph (SVFG)

    /// Collect uses of an LLVM Value
    /// traverseOnVFG(svfg, value);

    /// Collect all successor nodes on ICFG
    /// traverseOnICFG(icfg, value);

    // clean up memory
    // delete vfg;

    AndersenPTA *andersenPTA = new AndersenPTA(pag);
    andersenPTA->analyze();
    andersenPTA->dump_constraintGraph("consG");

    // get all address
    // andersenPTA->getAllAddr();

    SVFGBuilder svfBuilder;
    SVFG *svfg = svfBuilder.buildFullSVFG(andersenPTA);
    svfg->dump("svfg");

    Set<const SVFGNode *> addrNodes = getAddr(svfg);
    Set<UseLine *> useLines = getUseLines(addrNodes, svfg);
    for (UseLine *useline : useLines)
    {
        useline->build();
        useline->print(1);
    }

    // OrderedNodeSet nodeIds = andersenPTA->getPAG()->getAllValidPtrs();
    // for (NodeID nodeId : nodeIds)
    // {
    //     PAGNode *pagNode = andersenPTA->getPAG()->getGNode(nodeId);
    //     if (pagNode->getIncomingEdges(PAGEdge::Addr).size())
    //     {
    //         traverseOnVFG(svfg, pagNode);
    //     }
    // }

    AndersenWaveDiff::releaseAndersenWaveDiff();

    SVFIR::releaseSVFIR();

    LLVMModuleSet::getLLVMModuleSet()->dumpModulesToFile(".svf.bc");

    SVF::LLVMModuleSet::releaseLLVMModuleSet();

    llvm::llvm_shutdown();

    return 0;
}
