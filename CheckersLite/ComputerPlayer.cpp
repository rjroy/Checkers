#include "StdAfx.h"
#include "ComputerPlayer.h"

#include <algorithm>

//--------------------------------------------------------------------------------------
bool CComputerPlayer::Move( CCheckersBoard& board )
{
	std::vector<CMove> moves;
	if( !board.GetMoves( m_player, moves ) )
		return false;

	if( moves.empty() )
		return false;

	auto compareMoves = [this, board]( const CMove& lhs, const CMove& rhs )->bool{
		int lhsVal = this->MiniMax( board, lhs, m_player, m_depth );
		int rhsVal = this->MiniMax( board, rhs, m_player, m_depth );
		return (lhsVal > rhsVal);
	};

	std::sort( moves.begin(), moves.end(), compareMoves );	
	return board.MakeMoveIfValid( m_player, moves[0] );
}

//--------------------------------------------------------------------------------------
int CComputerPlayer::MiniMax( const CCheckersBoard& board, const CMove& move, EPlayer movingPlayer, unsigned int depth )
{
	TExpectedScore::iterator scoreItr = m_expectedScore.find( board );
	if( scoreItr != m_expectedScore.end() )
		return scoreItr->second;

	if( !depth )
		return board.CalculatePlayerScore( m_player );

	CCheckersBoard cpy( board );
	if( !cpy.MakeMoveIfValid( movingPlayer, move ) )
	{
		int result = board.CalculatePlayerScore( m_player );
		m_expectedScore[ board ] = result;
		return result;
	}

	EPlayer nextPlayer = CCheckersBoard::GetOpponent( movingPlayer );
	std::vector<CMove> moves;
	if( !cpy.GetMoves( nextPlayer, moves ) || moves.empty() )
	{
		int result = cpy.CalculatePlayerScore( m_player );
		m_expectedScore[ cpy ] = result;
		return result;
	}

	unsigned int newDepth = depth - 1;

	if( m_player == movingPlayer )
	{
		int bestValue = CCheckersBoard::MinScore;
		for( size_t i = 0; i < moves.size(); ++i )
		{
			int val = MiniMax( cpy, moves[i], nextPlayer, newDepth );
			bestValue = std::max( bestValue, val );
		}
		if( depth == m_depth )
			m_expectedScore[ cpy ] = bestValue;
		return bestValue;
	}
	else
	{
		int bestValue = CCheckersBoard::MaxScore;
		for( size_t i = 0; i < moves.size(); ++i )
		{
			int val = MiniMax( cpy, moves[i], nextPlayer, newDepth );
			bestValue = std::min( bestValue, val );
		}
		if( depth == m_depth )
			m_expectedScore[ cpy ] = bestValue;
		return bestValue;
	}
}
