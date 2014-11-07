#include "StdAfx.h"
#include "ComputerPlayer.h"

#include <algorithm>

typedef std::pair<CMove, int> TScoredMove;

CPerfTimer CComputerPlayer::s_Move( "CComputerPlayer::Move" );
CPerfTimer CComputerPlayer::s_AlphaBeta( "CComputerPlayer::AlphaBeta" );

//--------------------------------------------------------------------------------------
bool CComputerPlayer::Move( CCheckersBoard& board )
{
	CPerfTimerCall __call( s_Move );

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
	m_expectedScore.UpdateCache( board, scoredMoves[0].second );
	return result;
}

//--------------------------------------------------------------------------------------
int CComputerPlayer::AlphaBeta( const CCheckersBoard& board, const CMove& move, EPlayer movingPlayer, unsigned int depth, int alpha, int beta )
{
	CPerfTimerCall __call( s_AlphaBeta );

	// Check for an already cached more state.
	int* pScore = m_expectedScore.Get( board );
	if( pScore )
		return *pScore;

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

	CMove* pExpectedMove = m_expectedMove.Get( cpy );

	// sort based on simple expected score to try and get better early pruning.
	auto compareScore = [this, nextPlayer, pExpectedMove]( const TScoredMove& lhs, const TScoredMove& rhs )->bool{
		// Assure the expected move is at the front.		
		if( pExpectedMove )
		{
			if( lhs.first == rhs.first )
			{
				return false;
			}
			if( lhs.first == *pExpectedMove ) 
			{
				return true;
			}
			if( rhs.first == *pExpectedMove )
			{
				return false;
			}
		}
		return ( m_player == nextPlayer ) ? ( lhs.second < rhs.second ) : ( lhs.second > rhs.second ); 
	};
	std::sort( scoredMoves.begin(), scoredMoves.end(), compareScore );	

	const unsigned int newDepth = depth - 1;

	if( m_player == nextPlayer )
	{
		// Maximizing this player
		size_t bestMove = scoredMoves.size();
		for( size_t i = 0; i < scoredMoves.size(); ++i )
		{
			int val = AlphaBeta( cpy, scoredMoves[i].first, nextPlayer, newDepth, alpha, beta );
			if( val > alpha )
			{
				alpha = val;
				bestMove = i;
				// prune because we are not going to find any better.
				if( beta <= alpha )
					break;
			}
		}
		if( bestMove < scoredMoves.size() )
			m_expectedMove.UpdateCache( cpy, scoredMoves[bestMove].first );
		return alpha;
	}
	else
	{
		// Minimizing this player.
		size_t bestMove = scoredMoves.size();
		for( size_t i = 0; i < scoredMoves.size(); ++i )
		{
			int val = AlphaBeta( cpy, scoredMoves[i].first, nextPlayer, newDepth, alpha, beta );
			if( val < beta )
			{
				beta = val;
				bestMove = i;
				// prune because we are not going to find any worse.
				if( beta <= alpha )
					break;
			}
		}
		if( bestMove < scoredMoves.size() )
			m_expectedMove.UpdateCache( cpy, scoredMoves[bestMove].first );
		return beta;
	}
}
