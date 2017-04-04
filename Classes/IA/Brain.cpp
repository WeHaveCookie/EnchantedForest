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

const bool Brain::isPossibleRift(CaseHandler* cHandler) const
{
	int gridSize = LevelMgr::getSingleton()->getGridSize();
	int line = cHandler->line;
	int column = cHandler->column;
	if (line - 1 >= 0)
	{
		if (m_knowledge[line - 1][column] != nullptr && m_knowledge[line - 1][column]->isWind())
		{
			return true;
		}
	}
	if (line + 1 < gridSize)
	{
		if (m_knowledge[line + 1][column] != nullptr && m_knowledge[line + 1][column]->isWind())
		{
			return true;
		}
	}
	if (column + 1 < gridSize)
	{
		if (m_knowledge[line][column + 1] != nullptr && m_knowledge[line][column + 1]->isWind())
		{
			return true;
		}
	}
	if (column - 1 >= 0)
	{
		if (m_knowledge[line][column - 1] != nullptr && m_knowledge[line][column - 1]->isWind())
		{
			return true;
		}
	}
	return false;
}

const bool Brain::isPossibleMonster(CaseHandler* cHandler) const
{
	int gridSize = LevelMgr::getSingleton()->getGridSize();
	int line = cHandler->line;
	int column = cHandler->column;
	if (line - 1 >= 0)
	{
		if (m_knowledge[line - 1][column] != nullptr && m_knowledge[line - 1][column]->isPoop())
		{
			return true;
		}
	}
	if (line + 1 < gridSize)
	{
		if (m_knowledge[line + 1][column] != nullptr && m_knowledge[line + 1][column]->isPoop())
		{
			return true;
		}
	}
	if (column + 1 < gridSize)
	{
		if (m_knowledge[line][column + 1] != nullptr && m_knowledge[line][column + 1]->isPoop())
		{
			return true;
		}
	}
	if (column - 1 >= 0)
	{
		if (m_knowledge[line][column - 1] != nullptr && m_knowledge[line][column - 1]->isPoop())
		{
			return true;
		}
	}
	return false;
}

void Brain::buildCaseTree(CaseTree* root)
{
	CaseTree* currentRoot = root;
	for (auto& cBorder : m_border)
	{
		CaseTree* leafL, *leafR;

		if (isPossibleMonster(cBorder))
		{
			leafL->proba = 0.2f;
			leafR->proba = 0.8f;
			currentRoot->left = leafL;
			currentRoot->right = leafR;
			buildCaseTree(currentRoot->left);
			buildCaseTree(currentRoot->right);

		}

	}
}

const bool Brain::isMonster(CaseHandler* cHandler)
{
	CaseTree* root;
	root->proba = 1.0f;
	buildCaseTree(root);
	
	return (cHandler->vanity != nullptr && cHandler->vanity->getElement() == EntityElement::Monster);
}

const bool Brain::isRift(CaseHandler* cHandler) const
{
	return (cHandler->vanity != nullptr && cHandler->vanity->getElement() == EntityElement::Rift);
}

CaseHandler* Brain::getLowestRiskCase()
{
	bool find = false;
	auto it = m_border.begin();
	for (auto& cHandler : m_border)
	{
		if (!isMonster(cHandler) && !isRift(cHandler))
		{
			m_border.erase(it);
			return cHandler;
		}
		it++;
	}
	return nullptr;
}

void Brain::reset()
{
	clearKnowledge();
	m_border.clear();
	m_exploSparseGraph.Clear();
}


void Brain::explore()
{
	auto cHandler = getLowestRiskCase();

}

void Brain::updateBorder()
{
	
	auto currentCHandle = PhysicMgr::getSingleton()->getCase(m_entity);
	
	m_knowledge[currentCHandle->line][currentCHandle->column] = currentCHandle;
	
	auto grid = LevelMgr::getSingleton()->getGrid();
	
	int line = currentCHandle->line;
	int column = currentCHandle->column;
	int gridSize = LevelMgr::getSingleton()->getGridSize();

	if (line - 1 >= 0)
	{
		addInBorder(grid[line - 1][column]);
	}
	if (line + 1 < gridSize)
	{
		addInBorder(grid[line + 1][column]);
	}
	if (column + 1 < gridSize)
	{
		addInBorder(grid[line][column + 1]);
	}
	if (column - 1 >= 0)
	{
		addInBorder(grid[line][column - 1]);
	}
}

void Brain::process(const float dt)
{
	if (m_debugPause || m_entity->hasTarget())
	{
		return;
	}

	updateBorder();
	explore(); 
}

void Brain::initGraph()
{
	auto grid = LevelMgr::getSingleton()->getGrid();
	for (int i = 0; i < (int)grid.size(); i++)
	{
		for (int j = 0; j < (int)grid[i].size(); j++)
		{
			Node node;
			node.caseHandler = grid[i][j];
			m_exploSparseGraph.AddNode(node);
			GraphEdge edge;
			edge.SetFrom(node.Index());

			if (i == 0 && j == 0) continue;
			if (i == 0)
			{
				node.caseHandler = grid[i][j - 1];
				edge.SetTo(node.Index());
				m_exploSparseGraph.AddEdge(edge);
			}
			else if (j == 0)
			{
				node.caseHandler = grid[i - 1][j];
				edge.SetTo(node.Index());
				m_exploSparseGraph.AddEdge(edge);
			}
			else
			{
				node.caseHandler = grid[i][j - 1];
				edge.SetTo(node.Index());
				m_exploSparseGraph.AddEdge(edge);
				node.caseHandler = grid[i - 1][j];
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

void Brain::initKnowledge(int size)
{
	reset();
	for (int line = 0; line < size; line++)
	{
		std::vector<CaseHandler*> lineHandler;
		for (int column = 0; column < size; column++)
		{
			lineHandler.push_back(nullptr);
		}
		m_knowledge.push_back(lineHandler);
	}
	initGraph();
}

void Brain::clearKnowledge()
{
	for (auto& line : m_knowledge)
	{
		line.clear();
	}
	m_knowledge.clear();
}

void Brain::addInBorder(CaseHandler* cHandler)
{
	if (m_knowledge[cHandler->line][cHandler->column] == nullptr)
	{
		m_border.push_back(cHandler);
	}
}