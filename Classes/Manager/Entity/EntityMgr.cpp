#include "stdafx.h"
#include "EntityMgr.h"

#include "../../External/rapidjson/document.h"
#include "Entity/EntityPool.h"
#include "Manager/Loading/LoadingMgr.h"
#include "Manager/File/FileMgr.h"
#include "Manager/Physic/PhysicMgr.h"
#include "Utils/wcharUtils.h"
#include "Manager/Input/InputMgr.h"
#include "Manager/Level/LevelMgr.h"
#include "IA/Brain.h"

EntityMgr* EntityMgr::s_singleton = NULL;
uint32_t Entity::newUID = 0;

EntityMgr::EntityMgr()
:Manager(ManagerType::Enum::Entity)
{
	s_singleton = this;
	m_entitys = new EntityPool(1000);
}

EntityMgr::~EntityMgr()
{
	delete m_entitys;
}

void EntityMgr::init()
{
	m_processTime = sf::Time::Zero;
	m_mainCharacterID = 1;
	m_onEdition = false;
}

void EntityMgr::process(const float dt)
{
	sf::Clock clock;
	//auto entitys = m_entitys->getUsedEntitysSortedHTL();
// 	for (auto& entity : entitys)
// 	{
// 		auto mouseCurrentPosition = InputMgr::getSingleton()->getMousePosition();
// 		if (entity->getLastGlobalBounds().contains(mouseCurrentPosition.sf()) && InputMgr::getSingleton()->keyIsJustPressed(KeyType::mouseLeft))
// 		{
// 			entity->showInfo();
// 		}
// 		if (entity->getLastGlobalBounds().contains(mouseCurrentPosition.sf()) && InputMgr::getSingleton()->keyIsJustPressed(KeyType::mouseWheelButton))
// 		{
// 			if(m_onEdition && !entity->isEdition())
// 			{
// 				continue;
// 			}
// 			entity->edition();
// 			m_onEdition = entity->isEdition();
// 			break;
// 		}
// 	}
	m_entitys->process(dt);
	m_processTime = clock.getElapsedTime();
}

void EntityMgr::end()
{
	free(m_entitys);
}

void EntityMgr::paint()
{
	m_entitys->paint();
}

void EntityMgr::showImGuiWindow(bool* window)
{
	static bool showAll = false;
	if (ImGui::Begin("EntityMgr", window))
	{
		ImGui::Checkbox("Show all entity", &showAll);
		ImGui::SameLine();
		if (ImGui::Button("Close All"))
		{
			for (auto& entity : m_entitys->getEntitys())
			{
				entity->closeInfo();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Open All"))
		{
			for (auto& entity : m_entitys->getUsedEntitys())
			{
				entity->closeInfo();
				entity->showInfo();
			}
		}
		ImGui::Text("Used entitys : %i / %i", getNumberUsedEntity(), m_entitys->getPoolSize());

		std::vector<std::wstring> files;
		FileMgr::GetFilesInDirectory(files, L"Data/Character", L".json");

		char** filesLabel = (char**) malloc(sizeof(char*) * files.size());
		
		for (unsigned int i = 0; i < files.size(); i++)
		{
			filesLabel[i] = (char*)malloc(sizeof(char) * files[i].size() + 1); // +1 for null terminated
			strcpy(filesLabel[i], WideChartoAscii(files[i]).c_str());
		}

		static int itemID = 0;

		ImGui::Combo("Entity", &itemID, (const char**)filesLabel, files.size());
		ImGui::SameLine();
		if (ImGui::Button("Create"))
		{
			createAsyncEntity(filesLabel[itemID]);
		}


		for (unsigned int i = 0; i < files.size(); i++)
		{
			free(filesLabel[i]);
		}
		free(filesLabel);

		ImGui::Text("Main character : %i", getMainCharacter()->getUID());
		if (ImGui::IsItemClicked())
		{
			getMainCharacter()->showInfo();
		}

		if(ImGui::CollapsingHeader("Entitys"))
		{
			int mainID = getMainCharacter()->getUID();
			int previousMainID = mainID;
			ImGui::InputInt("Main Character", &mainID);
			if (mainID != previousMainID)
			{
				if (mainID >= 0 && mainID < m_entitys->getPoolSize())
				{
					setMainCharacter(mainID);
				}
			}
			ImGui::Text("Main character : %i | %s", getMainCharacter()->getUID(), getMainCharacter()->getName());
			for (auto& entity : m_entitys->getEntitys())
			{
				if (!entity->isAlive() && showAll)
				{
					ImGui::Text("%i : unused", entity->getUID());
				}
				else if (entity->isAlive())
				{
					ImGui::Text("%i : %s", entity->getUID(), entity->getName());
					if (ImGui::IsItemClicked())
					{
						entity->showInfo();
					}
					std::string label = "Delete###" + std::to_string(entity->getUID());
					ImGui::SameLine();
					if (ImGui::Button(label.c_str()))
					{
						deleteEntity(entity->getUID());
					}
				}
			}
		}
	}
	ImGui::End();
}

Entity* EntityMgr::createEntity(const char* path)  const
{
	Entity* newEntity = m_entitys->getNextEntity();
	LoadingMgr::getSingleton()->load(newEntity, path);
	return newEntity;
}

uint32_t EntityMgr::createAsyncEntity(const char* path)  const
{
	return LoadingMgr::getSingleton()->loadAsync(m_entitys->getNextEntity(), path);
}

void EntityMgr::deleteEntity(uint32_t id)
{
	m_entitys->release(id);
}

Entity* EntityMgr::getEntity(uint32_t id)  const
{
	return m_entitys->getEntity(id);
}

Entity* EntityMgr::getEntity(char* name) const
{
	return m_entitys->getEntity(name);
}

Entity* EntityMgr::getAsyncEntity(uint32_t id) const
{
	Entity* res = nullptr;
	auto valid = LoadingMgr::getSingleton()->getResult(id, (void**)&res);
	return res;
}

int EntityMgr::getNumberUsedEntity()
{
	return m_entitys->getNumberUsedEntity();
}

const bool EntityMgr::entityIsLoaded(uint32_t id) const
{
	return LoadingMgr::getSingleton()->isLoaded(id);
}

void EntityMgr::displayEntitysInfos()
{
	auto entitys = m_entitys->getUsedEntitys();
	for (auto& entity : entitys)
	{
		entity->showImGuiWindow();
	}
}

void EntityMgr::moveEntity(MoveDirection::Enum dir, Entity* ent)
{
	auto levelMgr = LevelMgr::getSingleton();
	if (ent->hasTarget())
	{
		return;
	}
	switch (dir)
	{
	case MoveDirection::Left:
	{
		auto x = ent->getPosition().x - (levelMgr->getSizeCase().x * ent->getScale().x);
		if (x >= levelMgr->getPositionLevel().x)
		{
			ent->setTargetPos(Vector2(x, ent->getPosition().y));
			ent->setState(EntityAnimationState::Left);
			LevelMgr::getSingleton()->modifyScore(-1);
		}
		break;
	}
	case MoveDirection::Right:
	{
		auto x = ent->getPosition().x + (levelMgr->getSizeCase().x * ent->getScale().x);
		if (x < levelMgr->getPositionLevel().x + levelMgr->getSizeLevel().x * ent->getScale().x)
		{
			ent->setTargetPos(Vector2(x, ent->getPosition().y));
			ent->setState(EntityAnimationState::Right);
			LevelMgr::getSingleton()->modifyScore(-1);
		}
		break;
	}
	case MoveDirection::Up:
	{
		auto y = ent->getPosition().y - (levelMgr->getSizeCase().y * ent->getScale().y);
		if (y >= levelMgr->getPositionLevel().y - (ent->getGlobalBounds().height / ent->getScale().y))
		{
			ent->setTargetPos(Vector2(ent->getPosition().x, y));
			ent->setState(EntityAnimationState::Up);
			LevelMgr::getSingleton()->modifyScore(-1);
		}
		break;
	}
	case MoveDirection::Down:
	{
		auto y = ent->getPosition().y + (levelMgr->getSizeCase().y * ent->getScale().y);
		auto sizeLevel = levelMgr->getSizeLevel().y;
		auto scaley = ent->getScale().y;
		auto mumu = sizeLevel * scaley;
		auto posLvl = levelMgr->getPositionLevel().y;
		auto finalPos = posLvl + mumu;
		if (y < levelMgr->getPositionLevel().y - (ent->getGlobalBounds().height / ent->getScale().y) + levelMgr->getSizeLevel().y * ent->getScale().y)
		{
			ent->setTargetPos(Vector2(ent->getPosition().x, y));
			ent->setState(EntityAnimationState::Down);
			LevelMgr::getSingleton()->modifyScore(-1);
		}
		break;
	}
	default:
		break;
	}
}

void EntityMgr::deleteCase()
{
	for (auto& entity : m_entitys->getUsedEntitys())
	{
		if (entity->getCaseHandler() != nullptr)
		{
			entity->getCaseHandler()->releaseAll();
			deleteEntity(entity->getUID());
		}
	}
}