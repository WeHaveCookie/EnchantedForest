#include "stdafx.h"
#include "Level.h"
#include "Entity/Entity.h"
#include "Manager/Render/RenderMgr.h"
#include "Manager/File/FileMgr.h"
#include "../../External/rapidjson/document.h"
#include "Utils/jsonUtils.h"
#include "Manager/Entity/EntityMgr.h"
#include "Utils/containerUtils.h"
#include "Manager/Game/GameMgr.h"
#include "Manager/Sound/SoundMgr.h"
#include "Manager/Physic/PhysicMgr.h"
#include "Utils/Random.h"

void Background::paint()
{
	auto rdrWin = RenderMgr::getSingleton()->getMainRenderWindow();
	rdrWin->draw(m_sprite); 
}

void CaseHandler::release()
{
	if (vanity != nullptr)
	{
		EntityMgr::getSingleton()->deleteEntity(vanity->getUID());
		vanity = nullptr;
	}
}

void CaseHandler::releaseAll()
{
	release();
	releaseEffect();
}

void CaseHandler::releaseEffect()
{
	if (entities.size() > 0)
	{
		for (auto& entity : entities)
		{
			EntityMgr::getSingleton()->deleteEntity(entity->getUID());
		}
	}
	entities.clear();
}

const bool CaseHandler::addObstacle(const char* path)
{
	if (vanity != nullptr && vanity->getElement() == EntityElement::Portal)
	{
		return false;
	}
	release();
	releaseEffect();
	auto ent = EntityMgr::getSingleton()->createEntity(path);
	ent->setPosition(currentPos);
	ent->setScale(currentScale);
	vanity = ent;
	return true;
}

void CaseHandler::addEffects(const char* path)
{
	if (vanity != nullptr && (vanity->getElement() == EntityElement::Monster || vanity->getElement() == EntityElement::Rift || vanity->getElement() == EntityElement::Portal))
	{
		return;
	}
	release();
	Entity* ent = EntityMgr::getSingleton()->createEntity(path);
	
	if (entities.size() < 1)
	{
		ent->setPosition(currentPos);
		ent->setScale(currentScale);
	}
	else
	{
		if (ent->getUID() == entities[0]->getUID())
		{
			return;
		}
		Vector2 newScale = Vector2(currentScale.x / 2 , currentScale.y / 2);
		entities[0]->setScale(newScale);
		ent->setScale(newScale);
		
		Vector2 newPos;
		if (entities.size() == 1)
		{
			newPos = Vector2(currentPos.x + entities[0]->getGlobalBounds().width, currentPos.y);
		}
		else if (entities.size() == 2)
		{
			newPos = Vector2(currentPos.x, currentPos.y + entities[0]->getGlobalBounds().height);
		}
		else
		{
			newPos = Vector2(currentPos.x + entities[0]->getGlobalBounds().width, currentPos.y + entities[0]->getGlobalBounds().height);
		}
		
		ent->setPosition(newPos);
		
	}
	entities.push_back(ent);
}

const bool CaseHandler::removeMonster()
{
	if (vanity != nullptr && vanity->getElement() == EntityElement::Monster)
	{
		release();
		return true;
	}
	return false;
}

const bool CaseHandler::deadly() const
{
	if (vanity != nullptr)
	{
		auto elem = vanity->getElement();
		return (elem == EntityElement::Monster || elem == EntityElement::Rift);
	}
	return false;
}

const bool CaseHandler::isExit() const
{
	if (vanity != nullptr)
	{
		auto elem = vanity->getElement();
		return (elem == EntityElement::Portal);
	}
	return false;
}

Level::Level()
{
}


Level::~Level()
{
}

void Level::paint()
{
	for (auto &background : m_backgrounds)
	{
		background->paint();
	}
}

bool cmpDisplayLevelLTH(Background* a, Background* b)
{
	return a->m_backgroundLevel >= b->m_backgroundLevel;

}

bool Level::load(const char* path)
{
	m_path = path;

	char* json;
	int jsonSize;

	FileMgr::ReadFile(path, (void**)&json, &jsonSize);

	json[jsonSize] = 0;

	rapidjson::Document document;
	document.Parse((char*)json);
	auto error = document.HasParseError();
	auto object = document.IsObject();
	if (error || !object)
	{
		return false;
	}
	assert(!error);
	assert(object);

	auto namePtr = &m_name;
	checkAndAffect(&document, "Name", ValueType::String, (void**)&namePtr, "NoName", sizeof("NoName"));

	auto sizePtr = &m_size;
	checkAndAffect(&document, "Size", ValueType::Vector2, (void**)&sizePtr);

	auto caseSizePtr = &m_caseSize;
	checkAndAffect(&document, "CaseSize", ValueType::Vector2, (void**)&caseSizePtr);

	auto positionPtr = &m_position;
	checkAndAffect(&document, "Position", ValueType::Vector2, (void**)&positionPtr);

	if (document.HasMember("Music"))
	{
		auto music = SoundMgr::getSingleton()->addMusic(document["Music"].GetString(), true);
	}

	if (document.HasMember("Path"))
	{
		createGrid(document["Path"].GetString());
	}

	if (document.HasMember("Backgrounds"))
	{
		const rapidjson::Value& backgrounds = document["Backgrounds"];

		for (auto& background : backgrounds.GetArray())
		{
			
			assert(background.HasMember("Path") && background.HasMember("BackgroundLevel"));
			Background* back = new Background();
			auto path = background["Path"].GetString();
			auto load = back->m_texture.loadFromFile(path);
			back->m_sprite.setTexture(back->m_texture);
			back->m_backgroundLevel = background["BackgroundLevel"].GetUint();

			if (background.HasMember("Position") && background["Position"].GetArray().Size())
			{
				back->m_sprite.setPosition(sf::Vector2f(background["Position"][0].GetFloat(), background["Position"][1].GetFloat()));
			}

			pushSorted(&m_backgrounds, back, cmpDisplayLevelLTH);
		}
	}

	if (document.HasMember("Entitys"))
	{
		const rapidjson::Value& entitys = document["Entitys"];

		for (auto& entity : entitys.GetArray())
		{
			assert(entity.HasMember("Path"));
			
			
			if (entity.HasMember("Position"))
			{
				const rapidjson::Value& clones = entity["Position"];

				for (auto& clone : clones.GetArray())
				{
					Entity* ent = EntityMgr::getSingleton()->createEntity(entity["Path"].GetString());
					ent->setPosition(Vector2(clone[0].GetFloat(), clone[1].GetFloat()));
					m_entitys.push_back(ent);
				}
			}
			else
			{
				Entity* ent = EntityMgr::getSingleton()->createEntity(entity["Path"].GetString());
				m_entitys.push_back(ent);
			}
		}
	}
	m_Gridsize = m_size.x;
	FileMgr::CloseFile(json);
	return true;
}

bool Level::reload()
{
	unload();
	return load(m_path.c_str());
}

void Level::unload()
{
	m_backgrounds.clear();
	for (auto &entity : m_entitys)
	{
		EntityMgr::getSingleton()->deleteEntity(entity->getUID());
	}
	m_entitys.clear();
	m_name = "";
	m_size = sf::Vector2f();
}

void Level::createGrid(const char* path)
{
	m_grid.clear();
	int counter = 0;
	for (int i = 0; i < m_size.x; i++)
	{
		std::vector<CaseHandler*> line;
		for (int j = 0; j < m_size.y; j++)
		{
			auto back = EntityMgr::getSingleton()->createEntity(path);
			back->setPosition(Vector2(m_caseSize.x * i + m_position.x, m_caseSize.y * j + m_position.y));
			CaseHandler* cHandler = new CaseHandler();
			cHandler->currentPos = back->getPosition();
			cHandler->index = counter++;
			line.push_back(cHandler);
			back->setCaseHandler(cHandler);
		}
		m_grid.push_back(line);
	}
}

const std::vector<std::vector<CaseHandler*>> Level::getGrid()
{
	return m_grid;
}

const std::vector<std::vector<CaseHandler>> Level::getCacheGrid()
{
	std::vector<std::vector<CaseHandler>> ans;
	for (auto& line : m_grid)
	{
		std::vector<CaseHandler> currentLine;
		for (auto& caseH : line)
		{
			currentLine.push_back(*caseH);
		}
		ans.push_back(currentLine);
	}
	return ans;
}

CaseHandler* Level::getCase(uint32_t id)
{
	CaseHandler* ans = nullptr;
	for (auto& line : m_grid)
	{
		for (auto& handler : line)
		{
			if (handler->index == id)
			{
				ans = handler;
			}
		}
	}
	return ans;
}

Entity* Level::createGround(int line, int column)
{
	auto ent = EntityMgr::getSingleton()->createEntity("Data/Environment/Ground.json");
	sf::IntRect textRec;
	if (line == 0 && column == 0)
	{
		textRec = sf::IntRect(32*0,0, 32, 32);
	}
	else if (line == m_Gridsize - 1 && column == 0)
	{
		textRec = sf::IntRect(32*1, 0, 32, 32);
	}
	else if (line == 0 && column == m_Gridsize - 1)
	{
		textRec = sf::IntRect(32*2, 0, 32, 32);
	}
	else if (line == m_Gridsize - 1 && column == m_Gridsize - 1)
	{
		textRec = sf::IntRect(32*3, 0, 32, 32);
	}
	else if (column == 0)
	{
		textRec = sf::IntRect(32*4, 0, 32, 32);
	}
	else if (column == m_Gridsize - 1)
	{
		textRec = sf::IntRect(32*5, 0, 32, 32);
	}
	else if (line == 0)
	{
		textRec = sf::IntRect(32*7, 0, 32, 32);
	}
	else if (line == m_Gridsize - 1)
	{
		textRec = sf::IntRect(32*6, 0, 32, 32);
	}
	else
	{
		textRec = sf::IntRect(32 * 8, 0, 32, 32);
	}

	ent->setTextureRect(textRec);
	return ent;
}

Entity* createVanity()
{
	auto ent = EntityMgr::getSingleton()->createEntity("Data/Environment/Ground.json");
	sf::IntRect textRec;
	auto line = randIntBorned(1, 3);
	int column = randIntBorned(0, 7);
	ent->setTextureRect(sf::IntRect(32 * column, 32 * line, 32, 32));
	ent->setBackgroundLevel(30);
	ent->setElement(EntityElement::Vanity);
	return ent;
}

void Level::clearGrid()
{
	EntityMgr::getSingleton()->deleteCase();
	m_grid.clear();
}

void Level::generateObstacle()
{
	Vector2 scale;
	std::string pathObstacle, pathEffect;

	int portalLine, portalColumn;
	do 
	{
		portalLine = randIntBorned(0, m_Gridsize);
		portalColumn = randIntBorned(0, m_Gridsize);
	} while (portalLine == 0 && portalColumn == 0);

	m_grid[portalLine][portalColumn]->addObstacle("Data/Environment/Portal.json");

	for (int line = 0; line < m_Gridsize; line++)
	{
		for (int column = 0; column < m_Gridsize; column++)
		{
			if (line == 0 && column == 0)
			{
				continue;
			}
			if (randIntBorned(0, 100) < 20)
			{
				
				if (randIntBorned(0, 100) < 50)
				{
					pathObstacle = "Data/Environment/Monster.json";
					pathEffect = "Data/Environment/Poop.json";
				}
				else
				{
					pathObstacle = "Data/Environment/Rift.json";
					pathEffect = "Data/Environment/Wind.json";
				}
				
				if (!m_grid[line][column]->addObstacle(pathObstacle.c_str()))
				{
					continue;
				}
				
				if (line - 1 >= 0)
				{
					m_grid[line - 1][column]->addEffects(pathEffect.c_str());
				}
				if (line + 1 < m_Gridsize)
				{
					m_grid[line + 1][column]->addEffects(pathEffect.c_str());
				}
				if (column - 1 >= 0)
				{
					m_grid[line][column - 1]->addEffects(pathEffect.c_str());
				}
				if (column + 1 < m_Gridsize)
				{
					m_grid[line][column + 1]->addEffects(pathEffect.c_str());
				}
			}
		}
	}
}

void Level::generateGrid(int size)
{
	clearGrid();
	m_Gridsize = size;
	m_size = sf::Vector2f(m_Gridsize, m_Gridsize);
	int counter = 0;
	Vector2 scale;
	for (int lineID = 0; lineID < m_Gridsize; lineID++)
	{
		std::vector<CaseHandler*> line;
		for (int column = 0; column < m_Gridsize; column++)
		{
			auto back = createGround(lineID, column);
			auto vanity = createVanity();
			scale = back->getScale();
			if (size > 10)
			{
				float factor = size / 10.0f;
				scale /= factor;
				back->setScale(scale);
				vanity->setScale(scale);
			}
			back->setPosition(Vector2(m_caseSize.x * lineID * scale.x + m_position.x, m_caseSize.y * column * scale.y + m_position.y));
			vanity->setPosition(back->getPosition());
			CaseHandler* cHandler = new CaseHandler();
			cHandler->currentPos = back->getPosition();
			cHandler->currentScale = scale;
			cHandler->index = counter++;
			cHandler->vanity = vanity;
			cHandler->line = lineID;
			cHandler->column = column;
			line.push_back(cHandler);
			back->setCaseHandler(cHandler);
		}
		m_grid.push_back(line);
	}

	auto mainChar = GameMgr::getSingleton()->getEntityPlayer();
	mainChar->setScale(scale);
	auto newPos = Vector2(m_position.x, m_position.y - mainChar->getGlobalBounds().height / 3.0f);
	mainChar->setPosition(newPos);
	mainChar->setTargetPos(newPos);


	generateObstacle();
}

void Level::throwStone(Entity* entity, LevelOrientation::Enum orientation)
{
	auto cHandler = PhysicMgr::getSingleton()->getCase(entity);
	auto line = cHandler->line;
	auto column = cHandler->column;

	switch (orientation)
	{
	case LevelOrientation::Left:
		while (line - 1 >= 0)
		{
			if (m_grid[line - 1][column]->removeMonster())
			{
				break;
			}
			line--;
		}
		break;
	case LevelOrientation::Right:
		while (line + 1 < m_Gridsize)
		{
			if (m_grid[line + 1][column]->removeMonster())
			{
				break;
			}
			line++;
		}
		break;
	case LevelOrientation::Down:
		while (column + 1 < m_Gridsize)
		{
			if (m_grid[line][column + 1]->removeMonster())
			{
				break;
			}
			column++;
		}
		break;
	case LevelOrientation::Up:
		while (column - 1 >= 0)
		{
			if (m_grid[line][column - 1]->removeMonster())
			{
				break;
			}
			column--;
		}
		break;
	default:
		break;
	}
}