#include "my_game_object.h"
#include <iostream>

// -----------------------------------------------------------------------
// addChild: attach a child node to this node in the scene graph
// -----------------------------------------------------------------------
void MySceneGraphNode::addChild(std::shared_ptr<MySceneGraphNode>& obj)
{
	m_vMyChildren.push_back(obj);
}

// -----------------------------------------------------------------------
// printSceneGraph: public entry point - starts recursive print from root
// -----------------------------------------------------------------------
void MySceneGraphNode::printSceneGraph()
{
	int indent = 0;
	_printSceneGraph(indent);
}

// -----------------------------------------------------------------------
// _collectNodes: DFS pre-order traversal - collects raw pointers into list
// -----------------------------------------------------------------------
void MySceneGraphNode::_collectNodes(std::vector<MySceneGraphNode*>& nodes)
{
	nodes.push_back(this);
	for (auto& child : m_vMyChildren)
	{
		child->_collectNodes(nodes);
	}
}

// -----------------------------------------------------------------------
// traverseNext: advance the scene graph iterator to the next node in DFS
// pre-order. Wraps around from last node back to root. Returns pointer to
// the newly selected node (also sets its m_bIsCurrent flag).
// -----------------------------------------------------------------------
MySceneGraphNode* MySceneGraphNode::traverseNext(MySceneGraphNode* pCurrentNode)
{
	// Build a flat DFS-ordered list of all nodes
	std::vector<MySceneGraphNode*> nodes;
	_collectNodes(nodes);

	// Clear the current flag on every node
	for (auto* n : nodes)
	{
		n->m_bIsCurrent = false;
	}

	// Determine which node becomes "current"
	MySceneGraphNode* nextNode = nodes[0]; // default: wrap around to root

	if (pCurrentNode == nullptr)
	{
		// No current selection yet - start at root
		nextNode = nodes[0];
	}
	else
	{
		for (size_t i = 0; i < nodes.size(); i++)
		{
			if (nodes[i] == pCurrentNode)
			{
				// Advance to next, wrapping from last back to first (root)
				nextNode = nodes[(i + 1) % nodes.size()];
				break;
			}
		}
	}

	nextNode->m_bIsCurrent = true;
	return nextNode;
}

// -----------------------------------------------------------------------
// _printSceneGraph: recursive pretty-printer with indentation.
// Prints '*' before the name of the currently selected node.
// -----------------------------------------------------------------------
void MySceneGraphNode::_printSceneGraph(int indent)
{
	int i = 0;
	indent++;

	for (i = 0; i < indent; i++)
	{
		std::cout << "\t";
	}

	// Phase 3: mark the current iterator node with an asterisk
	if (m_bIsCurrent)
		std::cout << "* ";

	std::cout << m_sName << std::endl;

	for (i = 0; i < (int)m_vMyChildren.size(); i++)
	{
		m_vMyChildren[i]->_printSceneGraph(indent);
	}
}
