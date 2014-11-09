#pragma once

#include "stdafx.h"

#include "GameBoardBasics.h"
#include "LearningCache.h"

//--------------------------------------------------------------------------------------
template <typename TGameBoard>
class CComputerPlayer
{
	enum { CacheSize = 10240 };
public:
	CComputerPlayer( EPlayer player, unsigned int depth );
	~CComputerPlayer(void) {}

	// Returns the player this computer represents.
	EPlayer GetPlayer() const { return m_player; }

	// Asks that the computer make a random valid move.
	bool Move( TGameBoard& board );

	static CPerfTimer s_Move;
	static CPerfTimer s_AlphaBeta;

private:
	const EPlayer m_player;
	const unsigned int m_depth;

	enum EScoreType
	{
		ScoreType_Exact,
		ScoreType_UpperBound,
		ScoreType_LowerBound,

		ScoreTypeCount
	};

	struct STranspositionEntry
	{
		unsigned int m_draft;
		unsigned int m_score;
		EScoreType m_scoreType;

		STranspositionEntry( unsigned int draft, unsigned int score, EScoreType scoreType ) : m_draft(draft), m_score(score), m_scoreType(scoreType) { }
	};


	// Memory of expected AlphaBeta results.
	typedef CLearningCache<TGameBoard, STranspositionEntry> TTranspositionTable;
	TTranspositionTable m_table;

	// Determine the best score for the given move using alpha-beta prunning.
	int AlphaBeta( const TGameBoard& board, const CMove& move, EPlayer movingPlayer, unsigned int draft, int alpha, int beta );
	// Sort by expected score.
	void SortByGuess( std::vector<TScoredMove>& scoredMoves, const std::vector<CMove>& moves, const TGameBoard& current, EPlayer nextPlayer );
};

