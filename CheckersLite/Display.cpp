#include "StdAfx.h"
#include "Display.h"


#include <string>

//--------------------------------------------------------------------------------------
void CDisplay::ClearScreen( std::ostream& os ) const
{
	os << std::string( 100, '\n' );
}

//--------------------------------------------------------------------------------------
void CDisplay::Show( std::ostream& os, const CCheckersBoard& board ) const
{
	os << "  ";
	for( unsigned int x = 0; x < kBoardSize; ++x )
	{
		os << (x + 1);
	}
	os << std::endl;

	for( unsigned int y = 0; y < kBoardSize; ++y )
	{
		os << (char)(y + 'A') << " ";
		for( unsigned int x = 0; x < kBoardSize; ++x )
		{
			SPosition pos( x, y );

			switch( board.GetSquareState( pos ) )
			{
			case SquareState_Black:
				os << "O";
				break;
			case SquareState_Red:
				os << "X";
				break;
			case SquareState_BlackKing:
				os << "P";
				break;
			case SquareState_RedKing:
				os << "Y";
				break;
			default:
				os << ".";
				break;
			}
		}
		
		os << std::endl;
	}
}

//--------------------------------------------------------------------------------------
void CDisplay::ShowMoves( std::ostream& os, const std::vector<CMove>& moves ) const
{
	if( moves.empty() )
	{
		os << "No more moves!!" << std::endl;
	}
	else
	{
		for( size_t i = 0; i < moves.size(); ++i )
		{
			if( !moves[i].m_sequence.size() )
				continue;
			os << i << ": ";

			ShowPos( os, moves[i].m_start );
			os << "->";
			ShowPos( os, moves[i].m_sequence[0] );

			for( size_t j = 1; j < moves[i].m_sequence.size(); ++j )
			{
				os << "->";
				ShowPos( os, moves[i].m_sequence[j] );
			}
			os << std::endl;
		}
	}
}

//--------------------------------------------------------------------------------------
void CDisplay::ShowPos( std::ostream& os, const SPosition& pos ) const
{
	os << (char)(pos.m_y + 'A') << (pos.m_x + 1);
}
