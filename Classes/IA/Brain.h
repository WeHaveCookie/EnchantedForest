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
		ThrowRight,
		ThrowLeft,
		ThrowUp,
		ThrowDown
	};
}

class Entity;

struct Node
{
	CaseHandler*	caseHandler;


	int Index()
	{
		return caseHandler->index;
	}

	const Vector2 Pos() const
	{
		return caseHandler->currentPos;
	}

	const bool Deadly() const 
	{
		return caseHandler->deadly();
	}
};

typedef SparseGraph<Node, GraphEdge> Graph;

class Brain
{

public:
	Brain(Entity* ent);
	~Brain();

	void explore();
	void updateBorder();
	void process(const float dt);
	void showImGuiWindow();
	void start() { m_debugPause = false; }

	void showInfo() { m_displayInfo = !m_displayInfo; }
	void initKnowledge(int size);
	void addInBorder(CaseHandler* cHandler);
	CaseHandler* getLowestRiskCase();
	const bool isRift(CaseHandler* cHandler) const;
	const bool isMonster(CaseHandler* cHandler);
	const bool isPossibleMonster(CaseHandler* cHandler) const;
	const bool isPossibleRift(CaseHandler* cHandler) const;
	void reset();
	void createIntention(std::list<int> path);
	const bool executeAction();
	CaseHandler* getMonsterCaseInBorder();
	CaseHandler* getKnowCase(CaseHandler* cHandler);
	void throwStone(CaseHandler* cKnow, CaseHandler* cMonster);

private:

	void initGraph();
	void clearKnowledge();
	

	bool						m_displayInfo;
	Entity*						m_entity;
	Graph						m_exploSparseGraph;
	bool						m_debugPause;

	int							m_currentIntent;
	std::vector<Intention::Enum> m_debugIntents;
	std::queue<Intention::Enum>	m_intents;

	std::vector<CaseHandler*>				m_border;
	std::vector<std::vector<CaseHandler*>>	m_knowledge;
};
