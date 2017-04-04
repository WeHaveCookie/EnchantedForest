#pragma once

#include "Command.h"
#include "Level/Level.h"


class CommandThrowStone : public Command
{
public:
	CommandThrowStone() : Command("CommandThrowStone", CommandExeType::JustPressed) {};
	~CommandThrowStone() {};

	virtual void init(Entity* ent, void* data = NULL);
	virtual void execute();
	virtual void undo();
	virtual void* makeCopy();

private:
	LevelOrientation::Enum m_orientation;
};
