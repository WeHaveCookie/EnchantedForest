#pragma once

#include "Command.h"

class CommandChangeAnimState : public Command
{
public:
	CommandChangeAnimState() : Command("CommandChangeAnimState", CommandExeType::AtOnce) {};
	~CommandChangeAnimState() {};

	virtual void init(Entity* ent, void* data);
	virtual void execute();
	virtual void undo();
	virtual void* makeCopy();

private:
	std::string m_state;
};
