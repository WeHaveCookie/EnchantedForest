#include "stdafx.h"
#include "CommandThrowStone.h"
#include "Manager/Level/LevelMgr.h"

void CommandThrowStone::init(Entity* ent, void* data)
{
	Command::init(ent);
	auto orientation = static_cast<LevelOrientation::Enum*>(data);
	if (orientation != nullptr)
	{
		m_orientation = *orientation;
	}
	
}

void CommandThrowStone::execute()
{
	Entity* entity = getEntity();
	LevelMgr::getSingleton()->throwStone(entity, m_orientation);
}

void CommandThrowStone::undo()
{

}

void* CommandThrowStone::makeCopy()
{
	return (void*)new CommandThrowStone();
}