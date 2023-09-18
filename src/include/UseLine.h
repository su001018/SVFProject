#include "Graphs/SVFG.h"
#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/SVFIRBuilder.h"

using namespace std;
using namespace SVF;
using namespace llvm;
class UseLine
{
  public:
    const VFGNode *vfgNode;
    SVFG *vfg;
    Map<VFGNode *, UseLine *> children;
    string value;
    UseLine(const VFGNode *vfgNode, SVFG *vfg)
    {
        cout << "UseLine: " << vfgNode->getId() << endl;
        this->vfgNode = vfgNode;
        this->vfg = vfg;
        this->value = vfgNode->toString();
    }
    void print(int deep)
    {
        string depth;
        stringstream rawstr(depth);
        rawstr << "|";
        for (int i = 0; i < deep; i++)
        {
            rawstr << "-";
        }
        rawstr << value << endl;
        cout << endl << rawstr.str() << endl;
        for (Map<VFGNode *, UseLine *>::iterator it = children.begin(), eit = children.end(); it != eit; it++)
        {
            it->second->print(deep + 1);
        }
    }
    void build()
    {
        cout << "build: " << vfgNode->getId() << ": " << endl;
        for (VFGNode::iterator it = vfgNode->OutEdgeBegin(), eit = vfgNode->OutEdgeEnd(); it != eit; it++)
        {
            VFGEdge *edge = *it;
            VFGNode *node = edge->getDstNode();
            if (children.find(node) == children.end())
            {
                UseLine *child = new UseLine(node, vfg);
                children[node] = child;
                child->build();
            }
        }
        if (vfgNode->getNodeKind() == VFGNode::AParm)
        {
            if (const ActualParmSVFGNode *actualParamNode = SVFUtil::dyn_cast<ActualParmSVFGNode>(vfgNode))
            {
                const CallICFGNode *callICFGNode = actualParamNode->getCallSite();
                const SVFInstruction *instruction = callICFGNode->getCallSite();
                value += "\n";
                value += instruction->toString();
            }
        }
    }
};