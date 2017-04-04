#pragma once

class Entity;

struct LevelOrientation {
	enum Enum {
		Left,
		Right,
		Up,
		Down
	};
};

struct Background {
	sf::Texture	m_texture;
	sf::Sprite	m_sprite;
	int			m_backgroundLevel;

	void paint();
};

struct CaseHandler
{
	std::vector<Entity*> entities;
	std::vector<uint32_t> entitiesID;
	Vector2		currentPos;
	Vector2		currentScale;
	Entity*		background;
	Entity*		vanity;
	int			index; 
	int			line;
	int			column;

	CaseHandler()
	{
		currentPos = Vector2();
		currentScale = Vector2();
		background = nullptr;
	}

	void release();
	void releaseAll();
	void releaseEffect();
	const bool addObstacle(const char* path);
	void addEffects(const char* path);
	const bool removeMonster();
	const bool deadly() const;
	const bool isExit() const;
};

class Level
{

public:
	Level();
	~Level();

	void paint();
	bool load(const char* path);
	bool reload();
	void unload();

	const char* getName() const { return m_name.c_str(); }
	const sf::Vector2f getSize() const { return m_size; }
	void createGrid(const char* path);
	const std::vector<std::vector<CaseHandler*>> getGrid();
	const std::vector<std::vector<CaseHandler>> getCacheGrid();

	CaseHandler* getCase(uint32_t id);
	const sf::Vector2f getCaseSize() const { return m_caseSize; }
	const sf::Vector2f getPosition() const { return m_position; }
	const sf::Vector2f getSizeLevel() const { return sf::Vector2f(m_size.x * m_caseSize.x, m_size.y * m_caseSize.y); }
	
	void generateGrid(int size);
	void clearGrid();

	void throwStone(Entity* entity, LevelOrientation::Enum orientation);
	const int getGridSize() const { return m_Gridsize; }

private:
	Entity* createGround(int line, int column);
	void generateObstacle();
	
	std::string					m_path;
	std::string					m_name;
	sf::Vector2f				m_size;
	sf::Vector2f				m_caseSize;
	sf::Vector2f				m_position;
	std::vector<Entity*>		m_entitys;
	std::vector<Background*>	m_backgrounds;
	std::vector<Background*>	m_foregrounds;
	std::vector<std::vector<CaseHandler*>> m_grid;
	int							m_Gridsize;
};

