#include "StdAfx.h"
#include "ComputerPlayer.h"

#include <algorithm>

//--------------------------------------------------------------------------------------
bool CComputerPlayer::Move( CCheckersBoard& board ) const
{
	std::vector<CMove> moves;
	if( !board.GetMoves( m_player, moves ) )
		return false;

	if( moves.empty() )
		return false;

	std::random_shuffle( moves.begin(), moves.end() );
	return board.MakeMoveIfValid( m_player, moves[0] );
}
