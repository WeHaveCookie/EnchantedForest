#include "stdafx.h"
#include "LevelMgr.h"
#include "Level/Quadtree.h"
#include "Manager/Game/GameMgr.h"
#include "Manager/Render/RenderMgr.h"
#include "Level/Level.h"
#include "Manager/File/FileMgr.h"
#include "Utils/wcharUtils.h"
#include "Utils/Random.h"
#include "Manager/Entity/EntityMgr.h"
#include "Manager/Physic/PhysicMgr.h"
#include "Entity/Entity.h"
#include "IA/Brain.h"
#include "Manager/Sound/SoundMgr.h"

LevelMgr* LevelMgr::s_singleton = NULL;

LevelMgr::LevelMgr()
:Manager(ManagerType::Enum::Level)
{
	s_singleton = this;
	m_queryCount = 0.0f;
}

LevelMgr::~LevelMgr()
{
	delete m_quadtree;
}

void LevelMgr::init()
{
	m_processTime = sf::Time::Zero;
	m_quadtree = new Quadtree();
	m_quadtree->init(0.0f, 0.0f, 1920.0f, 1080.0f);
	m_quadtree->setNodeCapacity(50);
	m_level = new Level();
	loadLevel("Data/Level/level1.json");
	GameMgr::getSingleton()->setNumberPlayer(1);
	generateGrid(m_level->getGridSize());
	m_playerScore = 0;
}

void LevelMgr::process(const float dt)
{
	sf::Clock clock;
	m_quadtree->update();
	auto player = GameMgr::getSingleton()->getEntityPlayer();
	if (!player->hasTarget())
	{
		auto cHandler = PhysicMgr::getSingleton()->getCase(player);
		if (cHandler != nullptr)
		{
			if (cHandler->deadly())
			{
				auto levelSize = m_level->getGridSize();
				m_playerScore -= 10 * levelSize * levelSize;
				SoundMgr::getSingleton()->addSound("Data/Sound/FX/die.ogg");
				generateGrid(levelSize);
			}
			if (cHandler->isExit())
			{
				auto levelSize = m_level->getGridSize();
				m_playerScore += 10 * levelSize * levelSize;
				SoundMgr::getSingleton()->addSound("Data/Sound/FX/portal.ogg");
				generateGrid(levelSize + 1);
			}
		}
	}
	m_processTime = clock.getElapsedTime();
}

void LevelMgr::end()
{
}

void LevelMgr::paint()
{
	m_level->paint();
	//m_quadtree->paint();
}

void LevelMgr::showImGuiWindow(bool* window)
{
	if (ImGui::Begin("LevelMgr", window))
	{
		if(ImGui::CollapsingHeader("Quadtree"))
		{
			auto elements = m_quadtree->getAllElements();
			for (auto& element : elements)
			{
				ImGui::Text("%i - %s", element->getUID(), element->getName());
				if (ImGui::IsItemClicked())
				{
					element->showInfo();
				}
			}
		}

		ImGui::Text("Current Level : %s", m_level->getName());
		ImGui::Text("x = %f; y = %f", m_level->getSize().x, m_level->getSize().y);
		
		std::vector<std::wstring> files;
		FileMgr::GetFilesInDirectory(files, L"Data/Level", L".json");

		char** filesLabel = (char**)malloc(sizeof(char*) * files.size());

		for (unsigned int i = 0; i < files.size(); i++)
		{
			filesLabel[i] = (char*)malloc(sizeof(char) * files[i].size() + 1); // +1 for null terminated
			strcpy(filesLabel[i], WideChartoAscii(files[i]).c_str());
		}

		static int itemID = 0;

		ImGui::Combo("Level", &itemID, (const char**)filesLabel, files.size());
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			unloadLevel();
			loadLevel(filesLabel[itemID]);
		}
		ImGui::SameLine();
		if (ImGui::Button("Unload"))
		{
			unloadLevel();
		}

		ImGui::Text("Throw stone ");
		ImGui::SameLine();
		if (ImGui::Button("Left"))
		{
			throwStone(GameMgr::getSingleton()->getEntityPlayer(), LevelOrientation::Left);
		}
		ImGui::SameLine();
		if (ImGui::Button("Right"))
		{
			throwStone(GameMgr::getSingleton()->getEntityPlayer(), LevelOrientation::Right);
		}
		ImGui::SameLine();
		if (ImGui::Button("Up"))
		{
			throwStone(GameMgr::getSingleton()->getEntityPlayer(), LevelOrientation::Up);
		}
		ImGui::SameLine();
		if (ImGui::Button("Down"))
		{
			throwStone(GameMgr::getSingleton()->getEntityPlayer(), LevelOrientation::Down);
		}

		ImGui::Text("Player score : %i", m_playerScore);

		for (unsigned int i = 0; i < files.size(); i++)
		{
			free(filesLabel[i]);
		}
		free(filesLabel);

	}
	ImGui::End();
}

void LevelMgr::registerEntity(Entity* ent)
{
	m_quadtree->registerEntity(ent);
}

void LevelMgr::unregisterEntity(uint32_t id)
{
	m_quadtree->unregisterEntity(id);
}

std::vector<Entity*> LevelMgr::getEntityAround(Entity* ent, sf::FloatRect bound, EntityType::Enum type)
{
	m_queryCount++;
	return m_quadtree->queryRange(ent, bound, type);
}

int LevelMgr::getRegisterCount() 
{ 
	return m_quadtree->getRegisterCount(); 
}

int LevelMgr::getUnregisterCount() 
{ 
	return m_quadtree->getUnregisterCount(); 
}

int LevelMgr::getMasterRegisterCount()
{
	return m_quadtree->getMasterRegisterCount();
}

int LevelMgr::getQueryCount()
{
	return m_quadtree->getQueryCount();
}

bool LevelMgr::loadLevel(char* path)
{
	return m_level->load(path);
}

uint32_t LevelMgr::loadLevelAsync()
{
	return 1;
}

void LevelMgr::unloadLevel()
{
	m_level->unload();
}

CaseHandler* LevelMgr::getCase(uint32_t id)
{
	return m_level->getCase(id);
}

void LevelMgr::generateGrid(int size)
{
	m_level->generateGrid(size);
	GameMgr::getSingleton()->getEntityPlayer()->getBrain()->initKnowledge(size);
}

void LevelMgr::clearGrid()
{
	m_level->clearGrid();
}

void LevelMgr::throwStone(Entity* entity, LevelOrientation::Enum orientation)
{
	m_level->throwStone(entity, orientation);
	modifyScore(-10);
}