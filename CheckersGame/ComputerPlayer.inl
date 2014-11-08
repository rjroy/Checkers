#pragma once

#include "StdAfx.h"
#include "ComputerPlayer.h"

#include <algorithm>

typedef std::pair<CMove, int> TScoredMove;

template <typename TGameBoard>
CPerfTimer CComputerPlayer<TGameBoard>::s_Move( "CComputerPlayer::Move" );

template <typename TGameBoard>
CPerfTimer CComputerPlayer<TGameBoard>::s_AlphaBeta( "CComputerPlayer::AlphaBeta" );

//--------------------------------------------------------------------------------------
template <typename TGameBoard>
bool CComputerPlayer<TGameBoard>::Move( TGameBoard& board )
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
		scoredMoves[i] = TScoredMove( moves[i], AlphaBeta( board, moves[i], m_player, m_depth, TGameBoard::MinScore, TGameBoard::MaxScore ) );
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
template <typename TGameBoard>
int CComputerPlayer<TGameBoard>::AlphaBeta( const TGameBoard& board, const CMove& move, EPlayer movingPlayer, unsigned int depth, int alpha, int beta )
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
	TGameBoard cpy( board );
	if( !cpy.MakeMoveIfValid( movingPlayer, move ) )
	{
		return board.CalculatePlayerScore( m_player );
	}

	// Stop test if the next player cannot move after the moving player moves.
	EPlayer nextPlayer = TGameBoard::GetOpponent( movingPlayer );
	std::vector<CMove> moves;
	if( !cpy.GetMoves( nextPlayer, moves ) || moves.empty() )
	{
		return cpy.CalculatePlayerScore( m_player );
	}

	// sort based on simple expected score to try and get better early pruning.
	CMove* pExpectedMove = m_expectedMove.Get( cpy );
	auto compareScore = [this, nextPlayer, pExpectedMove]( const CMove& lhs, const CMove& rhs )->bool{
		// Assure the expected move is at the front.		
		if( pExpectedMove )
		{
			if( lhs == rhs )
			{
				return false;
			}
			if( lhs == *pExpectedMove ) 
			{
				return true;
			}
			if( rhs == *pExpectedMove )
			{
				return false;
			}
		}
		return ( m_player == nextPlayer ) ? ( lhs < rhs ) : ( lhs > rhs ); 
	};
	std::sort( moves.begin(), moves.end(), compareScore );	

	const unsigned int newDepth = depth - 1;

	if( m_player == nextPlayer )
	{
		// Maximizing this player
		size_t bestMove = moves.size();
		for( size_t i = 0; i < moves.size(); ++i )
		{
			int val = AlphaBeta( cpy, moves[i], nextPlayer, newDepth, alpha, beta );
			if( val > alpha )
			{
				alpha = val;
				bestMove = i;
				// prune because we are not going to find any better.
				if( beta <= alpha )
					break;
			}
		}
		if( bestMove < moves.size() )
			m_expectedMove.UpdateCache( cpy, moves[bestMove] );
		return alpha;
	}
	else
	{
		// Minimizing this player.
		size_t bestMove = moves.size();
		for( size_t i = 0; i < moves.size(); ++i )
		{
			int val = AlphaBeta( cpy, moves[i], nextPlayer, newDepth, alpha, beta );
			if( val < beta )
			{
				beta = val;
				bestMove = i;
				// prune because we are not going to find any worse.
				if( beta <= alpha )
					break;
			}
		}
		if( bestMove < moves.size() )
			m_expectedMove.UpdateCache( cpy, moves[bestMove] );
		return beta;
	}
}
