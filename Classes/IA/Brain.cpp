#include "stdafx.h"
#include "Brain.h"
#include "Manager/Action/CommandMgr.h"
#include "Actions/Command.h"
#include "Entity/Entity.h"
#include "Manager/Level/LevelMgr.h"
#include "Graph/GraphAlgorithms.h"
#include "Graph/AStarHeuristicPolicies.h"
#include "Manager/Physic/PhysicMgr.h"
#include "Manager/Game/GameMgr.h"

std::vector<const char*> IntentoToCommandName =
{
	"CommandMoveUp",
	"CommandMoveRight",
	"CommandMoveDown",
	"CommandMoveLeft",
	"CommandClean",
	"CommandPickUp",
	"CommandExplore"
};

Brain::Brain(Entity* ent)
	:m_entity(ent),
	m_exploSparseGraph(false),
	m_debugPause(false),
	m_displayInfo(true)
{
	m_debugIntentionScore = 0;
	initGraph();
}

Brain::~Brain()
{
}

const bool Brain::executeAction()
{
	return false;
}


void Brain::process(const float dt)
{
	if (m_debugPause)
	{
		return;
	}
	executeAction();
}

void Brain::initGraph()
{
	auto cacheGrid = LevelMgr::getSingleton()->getGrid();
	
	for (int i = 0; i < (int)cacheGrid.size(); i++)
	{
		for (int j = 0; j < (int)cacheGrid[i].size(); j++)
		{
			Node node;
			node.caseHandler = cacheGrid[i][j];
			m_exploSparseGraph.AddNode(node);
			GraphEdge edge;
			edge.SetFrom(node.Index());

			if (i == 0 && j == 0) continue;
			if (i == 0)
			{
				node.caseHandler = cacheGrid[i][j - 1];
				edge.SetTo(node.Index());
				m_exploSparseGraph.AddEdge(edge);
			}
			else if (j == 0)
			{
				node.caseHandler = cacheGrid[i - 1][j];
				edge.SetTo(node.Index());
				m_exploSparseGraph.AddEdge(edge);
			}
			else
			{
				node.caseHandler = cacheGrid[i][j - 1];
				edge.SetTo(node.Index());
				m_exploSparseGraph.AddEdge(edge);
				node.caseHandler = cacheGrid[i - 1][j];
				edge.SetTo(node.Index());
				m_exploSparseGraph.AddEdge(edge);
			}
		}
	}
}

void Brain::showImGuiWindow()
{
	if (m_displayInfo)
	{
		std::string name = "Brain : " + std::to_string(m_entity->getUID());
		if (ImGui::Begin(name.c_str(), &m_displayInfo))
		{
			ImGui::Checkbox("Pause", &m_debugPause);
		}
		ImGui::End();
	}
}
