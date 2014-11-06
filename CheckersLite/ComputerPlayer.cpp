#include "StdAfx.h"
#include "ComputerPlayer.h"

#include <algorithm>

//--------------------------------------------------------------------------------------
bool CComputerPlayer::Move( CCheckersBoard& board, unsigned int depth ) const
{
	std::vector<CMove> moves;
	if( !board.GetMoves( m_player, moves ) )
		return false;

	if( moves.empty() )
		return false;

	auto compareMoves = [this, board, depth]( const CMove& lhs, const CMove& rhs )->bool{
		int lhsVal = this->MiniMax( board, lhs, m_player, depth );
		int rhsVal = this->MiniMax( board, rhs, m_player, depth );
		return (lhsVal > rhsVal);
	};

	std::sort( moves.begin(), moves.end(), compareMoves );	
	return board.MakeMoveIfValid( m_player, moves[0] );
}

//--------------------------------------------------------------------------------------
int CComputerPlayer::MiniMax( const CCheckersBoard& board, const CMove& move, EPlayer movingPlayer, unsigned int depth ) const
{
	if( !depth )
		return board.CalculatePlayerScore( m_player );

	CCheckersBoard cpy( board );
	if( !cpy.MakeMoveIfValid( movingPlayer, move ) )
		return board.CalculatePlayerScore( m_player );

	EPlayer nextPlayer = CCheckersBoard::GetOpponent( movingPlayer );
	std::vector<CMove> moves;
	if( !cpy.GetMoves( nextPlayer, moves ) || moves.empty() )
		return cpy.CalculatePlayerScore( m_player );

	if( m_player == movingPlayer )
	{
		int bestValue = CCheckersBoard::MinScore;
		for( size_t i = 0; i < moves.size(); ++i )
		{
			int val = MiniMax( cpy, moves[i], nextPlayer, depth - 1 );
			bestValue = std::max( bestValue, val );
		}
		return bestValue;
	}
	else
	{
		int bestValue = CCheckersBoard::MaxScore;
		for( size_t i = 0; i < moves.size(); ++i )
		{
			int val = MiniMax( cpy, moves[i], nextPlayer, depth - 1 );
			bestValue = std::min( bestValue, val );
		}
		return bestValue;
	}
}
