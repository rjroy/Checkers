#include "StdAfx.h"
#include "ComputerPlayer.h"

#include <algorithm>

typedef std::pair<CMove, int> TScoredMove;

//--------------------------------------------------------------------------------------
bool CComputerPlayer::Move( CCheckersBoard& board )
{
	std::vector<CMove> moves;
	if( !board.GetMoves( m_player, moves ) )
		return false;

	if( moves.empty() )
		return false;

	// add some randomness.
	std::random_shuffle( moves.begin(), moves.end() );

	// Score all moves.
	std::vector<TScoredMove> scoredMoves( moves.size() );
	for( size_t i = 0; i < moves.size(); ++i )
	{
		scoredMoves[i] = TScoredMove( moves[i], AlphaBeta( board, moves[i], m_player, m_depth, CCheckersBoard::MinScore, CCheckersBoard::MaxScore ) );
	}

	// Re-sort moves 
	std::sort( scoredMoves.begin(), scoredMoves.end(), []( const TScoredMove& lhs, const TScoredMove& rhs)->bool{return lhs.second > rhs.second;} );

	// Pick the first one which will be random.
	bool result = board.MakeMoveIfValid( m_player, scoredMoves[0].first );
	// Cache the expected score for this board state.
	m_expectedScore[ board ] = scoredMoves[0].second;
	return result;
}

//--------------------------------------------------------------------------------------
int CComputerPlayer::AlphaBeta( const CCheckersBoard& board, const CMove& move, EPlayer movingPlayer, unsigned int depth, int alpha, int beta )
{
	// Check for an already cached more state.
	TExpectedScore::iterator scoreItr = m_expectedScore.find( board );
	if( scoreItr != m_expectedScore.end() )
		return scoreItr->second;

	// Stop testing if at max depth.
	if( !depth )
		return board.CalculatePlayerScore( m_player );

	// Stop test if the move is some how invalid.
	CCheckersBoard cpy( board );
	if( !cpy.MakeMoveIfValid( movingPlayer, move ) )
	{
		return board.CalculatePlayerScore( m_player );
	}

	// Stop test if the next player cannot move after the moving player moves.
	EPlayer nextPlayer = CCheckersBoard::GetOpponent( movingPlayer );
	std::vector<CMove> moves;
	if( !cpy.GetMoves( nextPlayer, moves ) || moves.empty() )
	{
		return cpy.CalculatePlayerScore( m_player );
	}

	// Collect the next board score for this player based on each move.
	std::vector<TScoredMove> scoredMoves( moves.size() );
	for( size_t i = 0; i < scoredMoves.size(); ++i )
	{
		scoredMoves[i] = std::make_pair( moves[i], CCheckersBoard( cpy, nextPlayer, moves[i] ).CalculatePlayerScore( m_player ) );
	}

	// sort based on simple expected score to try and get better early pruning.
	auto compareScore = [this, nextPlayer]( const TScoredMove& lhs, const TScoredMove& rhs )->bool{ 
		return ( m_player == nextPlayer ) ? ( lhs.second > rhs.second ) : ( lhs.second < rhs.second ); 
	};
	std::sort( scoredMoves.begin(), scoredMoves.end(), compareScore );	

	const unsigned int newDepth = depth - 1;

	if( m_player == nextPlayer )
	{
		// Maximizing this player
		for( size_t i = 0; i < scoredMoves.size(); ++i )
		{
			int val = AlphaBeta( cpy, scoredMoves[i].first, nextPlayer, newDepth, alpha, beta );
			alpha = std::max( alpha, val );
			// prune because we are not going to find any better.
			if( beta <= alpha )
				break;
		}
		return alpha;
	}
	else
	{
		// Minimizing this player.
		for( size_t i = 0; i < scoredMoves.size(); ++i )
		{
			int val = AlphaBeta( cpy, scoredMoves[i].first, nextPlayer, newDepth, alpha, beta );
			beta = std::min( beta, val );
			// prune because we are not going to find any worse.
			if( beta <= alpha )
				break;
		}
		return beta;
	}
}
