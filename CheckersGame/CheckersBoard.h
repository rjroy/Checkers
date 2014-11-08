#pragma once

#include "stdafx.h"

#include "GameBoardBasics.h"

#include <memory.h>
#include <vector>
#include <xhash>

static const int kMoveIndexLimit = 4;

//--------------------------------------------------------------------------------------
// Used to mark which piece is in a square.
enum ESquareState
{
	SquareState_Blank,
	SquareState_Red,
	SquareState_Black,
	SquareState_RedKing,
	SquareState_BlackKing,

	SquareStateCount
};

//--------------------------------------------------------------------------------------
// Represents the board and performs most game operations.
class CCheckersBoard
{
public:
	enum { MaxScore = kBoardSize * kBoardSize, MinScore = - kBoardSize * kBoardSize };

	CCheckersBoard(const CCheckersBoard& cpy, EPlayer movingPlayer, const CMove& move);
	CCheckersBoard(void) { Initialize(); }
	~CCheckersBoard(void) {}

	// Sets the game board to the initial state.
	void Initialize();

	// Returns the piece on a particulare square.
	ESquareState GetSquareState( const SPosition& pos ) const;

	// Calculates the list of valid moves for a provided player.
	bool GetMoves( EPlayer player, std::vector<CMove>& moves ) const;

	// Determines if a particular move is valid and also calculates the changes that would occur.
	bool IsValidMove( EPlayer player, const CMove& move, std::vector<SPosition>* pRemovedPieces = NULL, SPosition* pFinalPosition = NULL, ESquareState* pNewState = NULL ) const;

	// Tests if a move is valid and finalizes it.
	bool MakeMoveIfValid( EPlayer player, const CMove& move );

	// Evaluate score.
	int CalculatePlayerScore( EPlayer player ) const;

	// Returns the opponent player to the given player.
	static EPlayer GetOpponent( EPlayer player ) { return( player == Player_Red ? Player_Black : Player_Red ); }

	// Returns the owner of the square.
	static EPlayer GetPlayerOwner( ESquareState square );
	// Returns true if the space is a king.
	static bool IsKing( ESquareState square );

	// Returns 0 if equal else +1 if this > rhs else -1 (implying this < rhs)
	int Compare( const CCheckersBoard& rhs ) const;

	bool operator==( const CCheckersBoard& rhs ) const { return Compare( rhs ) == 0; }
	bool operator!=( const CCheckersBoard& rhs ) const { return Compare( rhs ) != 0; }
	bool operator< ( const CCheckersBoard& rhs ) const { return Compare( rhs ) <  0; }
	bool operator<=( const CCheckersBoard& rhs ) const { return Compare( rhs ) <= 0; }
	bool operator> ( const CCheckersBoard& rhs ) const { return Compare( rhs ) >  0; }
	bool operator>=( const CCheckersBoard& rhs ) const { return Compare( rhs ) >= 0; }

	operator size_t() const
	{
		unsigned int* begin = (unsigned int*)(this);
		unsigned int* end = begin + sizeof(CCheckersBoard) / sizeof(unsigned int);
		return stdext::_Hash_value( begin, end );
	}

	static CPerfTimer s_GetMoves;
	static CPerfTimer s_IsValidMove;
	static CPerfTimer s_MakeMoveIfValid;

private:
	// The memory holding the game state.
	// NOTE: assumes a kBoardSize of 8
	unsigned __int64 m_blackPieces;
	unsigned __int64 m_redPieces;
	unsigned __int64 m_blackKings;
	unsigned __int64 m_redKings;

	// Sets the game state of a space.
	ESquareState SetSquareState( const SPosition& pos, ESquareState state );

	// Determines the next position from the start position given a number from 0 to 3 which represents one of the 4 diagonals.
	// Returns false of the next position requested isn't on the board.
	static bool GetNextSpace( const SPosition& start, int moveIndex, SPosition& next );
	// Determines the position that is inbetween the given two positions.
	// Returns false if there is no position between the given two positions.
	static bool GetMiddlePosition( const SPosition& start, const SPosition& next, SPosition& middle );
	// Determins if the sequence has a loop at the end.
	static bool EndsInLoop( const std::vector<SPosition>& sequence );

	// Adds non-jump moves from the start position for the given player.
	bool AddSimpleMoves( EPlayer player, const SPosition& start, std::vector<CMove>& moves ) const;
	// Adds all jump moves from the start position for the given player.
	bool AddJumpMoves( EPlayer player, const SPosition& start, bool hasJumps, std::vector<CMove>& moves ) const;
	// Adds multi-jump moves from the start move for the given player.
	bool AddNextJumpMoves( EPlayer player, const CMove& start, std::vector<CMove>& moves ) const;

	// Helpers for the compare function.
	int CompareBlack( const CCheckersBoard& rhs ) const { return ( m_blackPieces == rhs.m_blackPieces ) ? 0 : ( m_blackPieces > rhs.m_blackPieces ) ? 1 : -1; }
	int CompareRed( const CCheckersBoard& rhs ) const { return ( m_redPieces == rhs.m_redPieces ) ? 0 : ( m_redPieces > rhs.m_redPieces ) ? 1 : -1; }
	int CompareBlackKing( const CCheckersBoard& rhs ) const { return ( m_blackKings == rhs.m_blackKings ) ? 0 : ( m_blackKings > rhs.m_blackKings ) ? 1 : -1; }
	int CompareRedKing( const CCheckersBoard& rhs ) const { return ( m_redKings == rhs.m_redKings ) ? 0 : ( m_redKings > rhs.m_redKings ) ? 1 : -1; }
};


//--------------------------------------------------------------------------------------
inline ESquareState CCheckersBoard::SetSquareState( const SPosition& pos, ESquareState state ) 
{ 
	static const unsigned __int64 one = 1;

	if( pos.IsValid() ) 
	{
		int index = pos.ToIndex();
		switch( state )
		{
		case SquareState_Red:
			m_blackPieces &= ~( one << index );
			m_blackKings  &= ~( one << index );
			m_redKings    &= ~( one << index );
			m_redPieces   |=  ( one << index );
			break;
		case SquareState_Black:
			m_redPieces   &= ~( one << index );
			m_blackKings  &= ~( one << index );
			m_redKings    &= ~( one << index );
			m_blackPieces |=  ( one << index );
			break;
		case SquareState_RedKing:
			m_blackPieces &= ~( one << index );
			m_blackKings  &= ~( one << index );
			m_redPieces   &= ~( one << index );
			m_redKings    |=  ( one << index );
			break;
		case SquareState_BlackKing:
			m_redPieces   &= ~( one << index );
			m_redKings    &= ~( one << index );
			m_blackPieces &= ~( one << index );
			m_blackKings  |=  ( one << index );
			break;
		case SquareState_Blank:
		default:
			m_blackPieces &= ~( one << index );
			m_blackKings  &= ~( one << index );
			m_redKings    &= ~( one << index );
			m_redPieces   &= ~( one << index );
			break;
		}
		return state; 
	}
	return SquareState_Blank;
}

//--------------------------------------------------------------------------------------
inline ESquareState CCheckersBoard::GetSquareState( const SPosition& pos ) const 
{ 
	static const unsigned __int64 one = 1;

	if( !pos.IsValid() ) 
		return SquareState_Blank;

	int index = pos.ToIndex();
	if( m_blackPieces & ( one << index ) )
		return SquareState_Black;
	if ( m_redPieces  & ( one << index ) )
		return SquareState_Red;
	if( m_blackKings  & ( one << index ) )
		return SquareState_BlackKing;
	if ( m_redKings   & ( one << index ) )
		return SquareState_RedKing;

	return SquareState_Blank;
}

//--------------------------------------------------------------------------------------
inline int CCheckersBoard::Compare( const CCheckersBoard& rhs ) const 
{ 
	int result = CompareBlack( rhs ); if( result ) return result; 
	result = CompareRed( rhs ); if( result ) return result; 
	result = CompareBlackKing( rhs ); if( result ) return result; 
	return CompareRedKing( rhs ); 
}

//--------------------------------------------------------------------------------------
inline bool CCheckersBoard::GetNextSpace( const SPosition& start, int moveIndex, SPosition& next )
{
	switch( moveIndex )
	{
	case 0:
		next.m_x = start.m_x + 1;
		next.m_y = start.m_y + 1;
		return next.IsValid();
	case 1:
		next.m_x = start.m_x - 1;
		next.m_y = start.m_y + 1;
		return next.IsValid();
	case 2:
		next.m_x = start.m_x + 1;
		next.m_y = start.m_y - 1;
		return next.IsValid();
	case 3:
		next.m_x = start.m_x - 1;
		next.m_y = start.m_y - 1;
		return next.IsValid();
	}

	return false;
}

//--------------------------------------------------------------------------------------
inline EPlayer CCheckersBoard::GetPlayerOwner( ESquareState square )
{
	switch( square )
	{
	case SquareState_Red:
	case SquareState_RedKing:
		return Player_Red;
	case SquareState_Black:
	case SquareState_BlackKing:
		return Player_Black;
	default:
		return Player_None;
	};
}

//--------------------------------------------------------------------------------------
inline bool CCheckersBoard::IsKing( ESquareState square )
{
	switch( square )
	{
	case SquareState_RedKing:
	case SquareState_BlackKing:
		return true;
	case SquareState_Black:
	case SquareState_Red:
	default:
		return false;
	};
}

//--------------------------------------------------------------------------------------
inline bool CCheckersBoard::GetMiddlePosition( const SPosition& start, const SPosition& next, SPosition& middle )
{
	middle.m_x = ( start.m_x + next.m_x ) / 2;
	middle.m_y = ( start.m_y + next.m_y ) / 2;
	
	return( middle.IsValid() && middle != start && middle != next );
}
