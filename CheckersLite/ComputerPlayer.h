#pragma once

#include "CheckersBoard.h"

class CComputerPlayer
{
public:
	CComputerPlayer( EPlayer player ) : m_player( player ) { }
	~CComputerPlayer(void) {}

	// Returns the player this computer represents.
	EPlayer GetPlayer() const { return m_player; }

	// Asks that the computer make a random valid move.
	bool Move( CCheckersBoard& board ) const;

private:
	EPlayer m_player;
};

