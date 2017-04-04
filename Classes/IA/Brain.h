#pragma once
#include "Level/Level.h"
#include "Graph/SparseGraph.h"
#include "Graph/GraphEdgeTypes.h"
#include "Entity/Entity.h"

namespace Intention
{
	enum Enum
	{
		MoveUp,
		MoveRight,
		MoveDown,
		MoveLeft,
		Clean,
		PickUp,
		Explore
	};
}

class Entity;

struct Node
{
	CaseHandler*	caseHandler;
	CaseHandler		cacheCase;

	void update()
	{
		cacheCase = *caseHandler;
	}

	int Index()
	{
		return caseHandler->index;
	}

	const Vector2 Pos() const
	{
		return cacheCase.background->getPosition();
	}

};

typedef SparseGraph<Node, GraphEdge> Graph;

class Brain
{
	struct CaseTree
	{
		float proba;
		CaseTree*	left;
		CaseTree*	right;
	};

public:
	Brain(Entity* ent);
	~Brain();

	void explore();
	void updateBorder();
	void process(const float dt);
	void showImGuiWindow();

	void showInfo() { m_displayInfo = !m_displayInfo; }
	void initKnowledge(int size);
	void addInBorder(CaseHandler* cHandler);
	CaseHandler* getLowestRiskCase();
	const bool isRift(CaseHandler* cHandler) const;
	const bool isMonster(CaseHandler* cHandler);
	const bool isPossibleMonster(CaseHandler* cHandler) const;
	const bool isPossibleRift(CaseHandler* cHandler) const;
	void buildCaseTree(CaseTree* root);
	void reset();
private:

	void initGraph();
	void clearKnowledge();
	

	bool						m_displayInfo;
	Entity*						m_entity;
	Graph						m_exploSparseGraph;
	bool						m_debugPause;
	int							m_debugIntentionScore;

	std::vector<CaseHandler*>				m_border;
	std::vector<std::vector<CaseHandler*>>	m_knowledge;
};
