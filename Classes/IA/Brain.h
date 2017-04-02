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

public:
	Brain(Entity* ent);
	~Brain();

	const bool executeAction();
	void process(const float dt);
	void showImGuiWindow();

	void showInfo() { m_displayInfo = !m_displayInfo; }

private:
	void initGraph();
	

	bool						m_displayInfo;
	Entity*						m_entity;
	Graph						m_exploSparseGraph;
	bool						m_debugPause;
	int							m_debugIntentionScore;

	std::vector<std::vector<CaseHandler>>	m_knwoledge;
};
