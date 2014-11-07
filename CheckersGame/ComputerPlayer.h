#pragma once

#include "CheckersBoard.h"
#include "LearningCache.h"

#include <map>


//--------------------------------------------------------------------------------------
class CComputerPlayer
{
	enum { ScoreCacheSize = 1000, MoveCacheSize = 1024 * 1024 };
public:
	CComputerPlayer( EPlayer player, unsigned int depth = 3 ) : m_player( player ), m_depth( depth ), m_expectedScore(ScoreCacheSize), m_expectedMove(MoveCacheSize) { }
	~CComputerPlayer(void) {}

	// Returns the player this computer represents.
	EPlayer GetPlayer() const { return m_player; }

	// Asks that the computer make a random valid move.
	bool Move( CCheckersBoard& board );

private:
	const EPlayer m_player;
	const unsigned int m_depth;

	// Memory of expected AlphaBeta results.
	typedef CLearningCache<CCheckersBoard, int> TExpectedScore;
	typedef CLearningCache<CCheckersBoard, CMove> TExpectedMove;
	TExpectedScore m_expectedScore;
	TExpectedMove  m_expectedMove;

	// Determine the best score for the given move using alpha-beta prunning.
	int AlphaBeta( const CCheckersBoard& board, const CMove& move, EPlayer movingPlayer, unsigned int depth, int alpha, int beta );
};

