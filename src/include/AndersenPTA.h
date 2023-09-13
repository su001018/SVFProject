#include "SVF-LLVM/LLVMUtil.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "WPA/Andersen.h"

using namespace std;
using namespace llvm;
using namespace SVF;

class AndersenPTA : public SVF::AndersenBase
{
  public:
    // constructor
    AndersenPTA(SVF::PAG *_pag) : AndersenBase(_pag){};

    // dump constraint graph
    void dump_constraintGraph(string name)
    {
        consCG->dump(name);
    }

    // get all address statement
    void getAllAddr();

  private:
    // init from pag, process the Addr edge
    void processAllAddr();

    // solve rules
    virtual void solveWorklist();

    // Add copy edge on constraint graph
    virtual bool addCopyEdge(SVF::NodeID src, SVF::NodeID dst)
    {
        if (consCG->addCopyCGEdge(src, dst))
            return true;
        else
            return false;
    }
};

void AndersenPTA::solveWorklist()
{
    processAllAddr();

    // Andersen's worklist-based transitive closure solving starts from  here
    // Keep solving until workList is empty.
    while (!isWorklistEmpty())
    {
        NodeID nodeId = popFromWorklist();
        ConstraintNode *node = consCG->getConstraintNode(nodeId);

        /// foreach o \in pts(p)
        for (NodeID o : getPts(nodeId))
        {
            /// *p = q  pts(q) \subseteq pts(o)
            for (ConstraintEdge *edge : node->getStoreInEdges())
                if (addCopyEdge(edge->getSrcID(), o))
                    pushIntoWorklist(edge->getSrcID());

            // r = *p   pts(o) \subseteq pts(r)
            for (ConstraintEdge *edge : node->getLoadOutEdges())
                if (addCopyEdge(o, edge->getDstID()))
                    pushIntoWorklist(o);
        }
        /// q = p or q = &p->f  pts(p) \subseteq pts(q)
        for (ConstraintEdge *edge : node->getDirectOutEdges())
            if (unionPts(edge->getDstID(), edge->getSrcID()))
                pushIntoWorklist(edge->getDstID());
    }
}

// TODO: Initialize each pointer at the address constraint
void AndersenPTA::processAllAddr()
{
    for (ConstraintGraph::const_iterator nodeIt = consCG->begin(), nodeEit = consCG->end(); nodeIt != nodeEit; nodeIt++)
    {
        ConstraintNode *cgNode = nodeIt->second;
        for (ConstraintNode::const_iterator it = cgNode->incomingAddrsBegin(), eit = cgNode->incomingAddrsEnd();
             it != eit; ++it)
        {
            numOfProcessedAddr++;

            const AddrCGEdge *addr = SVFUtil::cast<AddrCGEdge>(*it);
            NodeID dst = addr->getDstID();
            NodeID src = addr->getSrcID();
            if (addPts(dst, src))
                pushIntoWorklist(dst);
        }
    }
}

void AndersenPTA::getAllAddr()
{
    for (ConstraintGraph::const_iterator nodeIt = consCG->begin(), nodeEit = consCG->end(); nodeIt != nodeEit; nodeIt++)
    {
        ConstraintNode *cgNode = nodeIt->second;
        for (ConstraintNode::const_iterator it = cgNode->incomingAddrsBegin(), eit = cgNode->incomingAddrsEnd();
             it != eit; it++)
        {
            const AddrCGEdge *addr = SVFUtil::cast<AddrCGEdge>(*it);
            NodeID dst = addr->getDstID();
            NodeID src = addr->getSrcID();
            PAGNode *dstNode = pag->getGNode(dst);
            PAGNode *srcNode = pag->getGNode(src);
            dstNode->dump();
            srcNode->dump();
        }
    }
}
