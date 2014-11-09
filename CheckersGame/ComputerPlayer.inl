#pragma once

#include "StdAfx.h"
#include "ComputerPlayer.h"

#include <algorithm>

template <typename TGameBoard>
CPerfTimer CComputerPlayer<TGameBoard>::s_Move( "CComputerPlayer::Move" );

template <typename TGameBoard>
CPerfTimer CComputerPlayer<TGameBoard>::s_AlphaBeta( "CComputerPlayer::AlphaBeta" );

//--------------------------------------------------------------------------------------
template <typename TGameBoard>
CComputerPlayer<TGameBoard>::CComputerPlayer( EPlayer player, unsigned int depth ) 
	: m_player( player )
	, m_depth( depth )
	, m_table( CacheSize )
{ 
}

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

	// Re-sort moves to find the best move (highest to lowest).
	std::sort( scoredMoves.begin(), scoredMoves.end(), []( const TScoredMove& lhs, const TScoredMove& rhs)->bool{return lhs.second > rhs.second;} );

	// Pick the first one which will be the best move.
	return board.MakeMoveIfValid( m_player, scoredMoves[0].first );
}

//--------------------------------------------------------------------------------------
template <typename TGameBoard>
void CComputerPlayer<TGameBoard>::SortByGuess( std::vector<TScoredMove>& scoredMoves, const std::vector<CMove>& moves, const TGameBoard& current, EPlayer nextPlayer )
{
	int scoreOffset = TGameBoard::MaxScore;

	// Build the set of expected scores.
	for( size_t i = 0; i < moves.size(); ++i )
	{
		STranspositionEntry* pEntry = m_table.Get( TGameBoard( current, nextPlayer, moves[i] ) );
		if( pEntry && pEntry->m_scoreType == ScoreType_Exact )
		{
			scoredMoves.push_back( TScoredMove( moves[i], pEntry ? (pEntry->m_score + m_depth * scoreOffset) : 0 ) );
		}
		else
		{
			scoredMoves.push_back( TScoredMove( moves[i], pEntry ? (pEntry->m_score + pEntry->m_draft * scoreOffset) : 0 ) );
		}
	}

	// Sort by expected score, but the order is based on who the next player is.
	auto compareMoves = [this, nextPlayer]( const TScoredMove& lhs, const TScoredMove& rhs )->bool{
		return ( m_player == nextPlayer ) ? ( lhs.second > rhs.second ) : ( lhs.second < rhs.second ); 
	};

	// Perform the sort.
	std::sort( scoredMoves.begin(), scoredMoves.end(), compareMoves );	
}

//--------------------------------------------------------------------------------------
template <typename TGameBoard>
int CComputerPlayer<TGameBoard>::AlphaBeta( const TGameBoard& board, const CMove& move, EPlayer movingPlayer, unsigned int depth, int alpha, int beta )
{
	CPerfTimerCall __call( s_AlphaBeta );

	// Stop testing if at max depth.
	if( !depth )
		return board.CalculatePlayerScore( m_player );

	// Stop test if the move is some how invalid.
	TGameBoard cpy( board );
	if( !cpy.MakeMoveIfValid( movingPlayer, move ) )
		return board.CalculatePlayerScore( m_player );

	STranspositionEntry* pEntry = m_table.Get( cpy );
	if( pEntry && pEntry->m_draft <= ( m_depth - depth ) )
		return pEntry->m_score;

	// Stop test if the next player cannot move after the moving player moves.
	EPlayer nextPlayer = TGameBoard::GetOpponent( movingPlayer );
	std::vector<CMove> moves;
	if( !cpy.GetMoves( nextPlayer, moves ) || moves.empty() )
	{
		int result = cpy.CalculatePlayerScore( m_player );
		m_table.UpdateCache( cpy, STranspositionEntry( m_depth - depth, result, ScoreType_Exact ) );
		return result;
	}

	// Try to have an early out.
	std::vector<TScoredMove> scoredMoves;
	SortByGuess( scoredMoves, moves, cpy, nextPlayer );

	const unsigned int newDepth = depth - 1;
	int result = 0;
	if( m_player == nextPlayer )
	{
		// Maximizing this player
		for( size_t i = 0; i < scoredMoves.size(); ++i )
		{
			alpha = max( alpha, AlphaBeta( cpy, scoredMoves[i].first, nextPlayer, newDepth, alpha, beta ) );
			// prune because we are not going to find any better.
			if( beta <= alpha )
				break;
		}
		m_table.UpdateCache( cpy, STranspositionEntry( m_depth - depth, alpha, ScoreType_UpperBound ) );
		result = alpha;
	}
	else
	{
		// Minimizing this player.
		for( size_t i = 0; i < scoredMoves.size(); ++i )
		{
			beta = min( beta, AlphaBeta( cpy, scoredMoves[i].first, nextPlayer, newDepth, alpha, beta ) );
			// prune because we are not going to find any worse.
			if( beta <= alpha )
				break;
		}
		m_table.UpdateCache( cpy, STranspositionEntry( m_depth - depth, beta, ScoreType_LowerBound ) );
		result = beta;
	}
	return result;
}
