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
	m_debugPause(true),
	m_displayInfo(true)
{
	m_currentIntent = 0;
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

const bool Brain::isMonster(CaseHandler* cHandler)
{
	if (!isPossibleMonster(cHandler))
	{
		return false;
	}
	else
	{
		// Proba isMonster with Border and knowledge
		return true;
	}
}

const bool Brain::isRift(CaseHandler* cHandler) const
{
	if (!isPossibleRift(cHandler))
	{
		return false;
	}
	else
	{
		// Proba isRift with Border and knowledge
		return true;
	}
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
	m_currentIntent = 0;
	m_debugIntents.clear();
	m_intents = std::queue<Intention::Enum>();
}


void Brain::explore()
{
	auto cHandler = getLowestRiskCase();
	if (cHandler != nullptr)
	{
		m_intents = std::queue<Intention::Enum>();
		m_debugIntents.clear();
		typedef Graph_SearchAStar<Graph, Heuristic_Enchanted> AStar;
		auto search = AStar(m_exploSparseGraph, PhysicMgr::getSingleton()->getCase(m_entity)->index, cHandler->index);
		createIntention(search.GetPathToTarget());
		executeAction();
	}
	else
	{
		// Search to throw a stone or take risk ! 
	}
}

void Brain::createIntention(std::list<int> path)
{
	auto currentPos = PhysicMgr::getSingleton()->getCase(m_entity)->currentPos;
	for (auto& nodeID : path)
	{
		auto node = m_exploSparseGraph.GetNode(nodeID);
		auto casePos = node.Pos();

		if (casePos.x < currentPos.x)
		{
			m_intents.push(Intention::MoveLeft);
			m_debugIntents.push_back(Intention::MoveLeft);
		}
		else if (casePos.x > currentPos.x)
		{
			m_intents.push(Intention::MoveRight);
			m_debugIntents.push_back(Intention::MoveRight);
		}
		else if (casePos.y < currentPos.y)
		{
			m_intents.push(Intention::MoveUp);
			m_debugIntents.push_back(Intention::MoveUp);
		}
		else if (casePos.y > currentPos.y)
		{
			m_intents.push(Intention::MoveDown);
			m_debugIntents.push_back(Intention::MoveDown);
		}
		currentPos = casePos;
	}
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

const bool Brain::executeAction()
{

	if (m_intents.empty()) {
		m_currentIntent++;
		return false;
	}
	else {
		if (m_entity->hasTarget())
		{
			return true;
		}
		int id;
		Command* cmd = CommandMgr::getSingleton()->getCommand(IntentoToCommandName[m_intents.front()], &id);
		cmd->init(m_entity);
		cmd->setExeType(CommandExeType::AtOnce);
		CommandMgr::getSingleton()->addCommand(cmd);
		m_intents.pop();
		m_currentIntent++;
		return true;
	}
}

void Brain::process(const float dt)
{
	if (m_debugPause || m_entity->hasTarget())
	{
		executeAction();
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
			ImGui::SameLine();
			ImGui::Text("Intention - nbr : %i", m_intents.size());
			ImGui::Separator();
			int counter = 0;
			for (auto& intention : m_debugIntents)
			{
				if (counter < m_currentIntent)
				{
					ImGui::TextColored(ImColor(0, 0, 0), IntentoToCommandName[intention]);
				}
				else if (counter == m_currentIntent)
				{
					ImGui::TextColored(ImColor(128, 255, 0), IntentoToCommandName[intention]);
				}
				else
				{
					ImGui::TextColored(ImColor(0, 153, 0), IntentoToCommandName[intention]);
				}
				counter++;
			}
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
		for (auto& handler : m_border)
		{
			if (handler->index == cHandler->index)
			{
				return;
			}
		}
		m_border.push_back(cHandler);
	}
}