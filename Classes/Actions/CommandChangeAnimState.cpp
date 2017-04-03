#include "stdafx.h"
#include "CommandChangeAnimState.h"
#include "Entity/Entity.h"

void CommandChangeAnimState::init(Entity* ent, void* data)
{
	
	m_state = static_cast<char*>(data);
	Command::init(ent);
}

void CommandChangeAnimState::execute()
{
	Entity* entity = getEntity();
	entity->setState(m_state);
}

void CommandChangeAnimState::undo()
{

}

void* CommandChangeAnimState::makeCopy()
{
	return (void*)new CommandChangeAnimState();
}