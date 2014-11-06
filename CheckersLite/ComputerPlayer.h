#pragma once

#include "CheckersBoard.h"

#include <map>

class CComputerPlayer
{
public:
	CComputerPlayer( EPlayer player, unsigned int depth = 3 ) : m_player( player ), m_depth( depth ) { }
	~CComputerPlayer(void) {}

	// Returns the player this computer represents.
	EPlayer GetPlayer() const { return m_player; }

	// Asks that the computer make a random valid move.
	bool Move( CCheckersBoard& board );

private:
	const EPlayer m_player;
	const unsigned int m_depth;

	typedef std::map<CCheckersBoard, int> TExpectedScore;
	TExpectedScore m_expectedScore;

	int MiniMax( const CCheckersBoard& board, const CMove& move, EPlayer movingPlayer, unsigned int depth );
};

