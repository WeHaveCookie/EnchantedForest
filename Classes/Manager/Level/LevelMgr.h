#pragma once
#include "Manager/Manager.h"
#include "Entity/Entity.h"

class Quadtree;
class Entity;
class Level;

class LevelMgr : public Manager
{
    public:

		static LevelMgr* getSingleton() { return s_singleton; }

        LevelMgr();
        virtual ~LevelMgr();

		void init();
		void process(const float dt);
		void end();
		void paint();
		void showImGuiWindow(bool* window);

		void registerEntity(Entity* ent);
		void unregisterEntity(uint32_t id);
		std::vector<Entity*> getEntityAround(Entity* ent, sf::FloatRect bound, EntityType::Enum type = EntityType::All);
		Quadtree* getQuadtree() { return m_quadtree; }

		sf::Time getProcessTime() { return m_processTime; }
		int getRegisterCount();
		int getUnregisterCount();
		int getMasterRegisterCount();
		int getMasterQueryCount() { int tmp = m_queryCount; m_queryCount = 0; return tmp; }
		int getQueryCount();

		bool loadLevel(char* path);
		void unloadLevel();
		uint32_t loadLevelAsync();

		
		const sf::Vector2f getSizeCase() const { return m_level->getCaseSize(); }
		CaseHandler* getCase(uint32_t id);

		const sf::Vector2f getPositionLevel() const { return m_level->getPosition(); }
		const sf::Vector2f getSizeLevel() const { return m_level->getSizeLevel(); }

		const std::vector<std::vector<CaseHandler*>> getGrid() const { return m_level->getGrid(); };
		const std::vector<std::vector<CaseHandler>> getCacheGrid() const { return m_level->getCacheGrid(); }

		void generateGrid(int size);
		void clearGrid();
		void throwStone(Entity* entity, LevelOrientation::Enum orientation);
		void modifyScore(int value) { m_playerScore += value; }
		const int getGridSize() const { return m_level->getGridSize(); }

    protected:
    private:
        // Function

		static LevelMgr*	s_singleton;
		Quadtree*			m_quadtree;
		sf::Time			m_processTime;
		int					m_queryCount;
		Level*				m_level;
		std::list<uint32_t>	m_loadingEnvironment;
		int					m_playerScore;
};

