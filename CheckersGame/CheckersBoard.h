#pragma once

#include <memory.h>
#include <vector>
#include <xhash>

static const int kBoardSize = 8;
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
// Used to identify the two players.
enum EPlayer
{
	Player_Black,
	Player_Red,
	Player_None,
	
	PlayerCount
};

//--------------------------------------------------------------------------------------
// Represents a position on the board.
struct SPosition
{
	unsigned char m_x : 4;
	unsigned char m_y : 4;

	SPosition() : m_x(0), m_y(0) {}
	SPosition( unsigned int x, unsigned int y ) : m_x(x), m_y(y) { }

	int ToIndex() const { return m_x * kBoardSize + m_y; }

	int Compare( const SPosition& rhs ) const;
	bool IsValid() const { return ( m_x < kBoardSize && m_y < kBoardSize ); }

	bool operator == ( const SPosition& rhs ) const { return Compare( rhs ) == 0; }
	bool operator != ( const SPosition& rhs ) const { return Compare( rhs ) != 0; }
};

//--------------------------------------------------------------------------------------
// Represents a potential move.
class CMove
{
public:
	SPosition m_start;
	std::vector<SPosition> m_sequence;

	int Compare( const CMove& rhs ) const;

	bool operator == ( const CMove& rhs ) const { return Compare( rhs ) == 0; }
	bool operator != ( const CMove& rhs ) const { return Compare( rhs ) != 0; }
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
