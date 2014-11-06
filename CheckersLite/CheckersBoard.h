#pragma once

#include <memory.h>
#include <vector>

static const int kBoardSize = 8;
static const int kMoveIndexLimit = 4;

// Used to mark which piece is in a square.
enum ESquareState
{
	SquareState_Blank,
	SquareState_Red,
	SquareState_Black,

	SquareStateCount
};

// Used to identify the two players.
enum EPlayer
{
	Player_Black,
	Player_Red,
	
	PlayerCount
};

// Represents a position on the board.
struct SPosition
{
	unsigned int m_x;
	unsigned int m_y;

	SPosition() : m_x(0), m_y(0) {}
	SPosition( unsigned int x, unsigned int y ) : m_x(x), m_y(y) { }

	int ToIndex() const { return m_x * kBoardSize + m_y; }

	int Compare( const SPosition& rhs ) const;
	bool IsValid() const { return ( m_x < kBoardSize && m_y < kBoardSize ); }

	bool operator == ( const SPosition& rhs ) const { return Compare( rhs ) == 0; }
	bool operator != ( const SPosition& rhs ) const { return Compare( rhs ) != 0; }
};

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

// Represents the board and performs most game operations.
class CCheckersBoard
{
public:
	enum { MaxScore = kBoardSize * kBoardSize, MinScore = - kBoardSize * kBoardSize };

	CCheckersBoard(void) { Initialize(); }
	~CCheckersBoard(void) {}

	// Sets the game board to the initial state.
	void Initialize();

	// Returns the piece on a particulare square.
	ESquareState GetSquareState( const SPosition& pos ) const 
	{ 
		static const unsigned __int64 one = 1;

		if( !pos.IsValid() ) 
			return SquareState_Blank;
		if( m_blackPieces & ( one << pos.ToIndex() ) )
			return SquareState_Black;
		if ( m_redPieces  & ( one << pos.ToIndex() ) )
			return SquareState_Red;

		return SquareState_Blank;
	}

	// Calculates the list of valid moves for a provided player.
	bool GetMoves( EPlayer player, std::vector<CMove>& moves ) const;

	// Determines if a particular move is valid and also calculates the changes that would occur.
	bool IsValidMove( EPlayer player, const CMove& move, std::vector<SPosition>* pRemovedPieces = NULL, SPosition* pFinalPosition = NULL ) const;

	// Tests if a move is valid and finalizes it.
	bool MakeMoveIfValid( EPlayer player, const CMove& move );

	// Evaluate score.
	int CalculatePlayerScore( EPlayer player ) const;

	// Returns the opponent player to the given player.
	static EPlayer GetOpponent( EPlayer player ) { return( player == Player_Red ? Player_Black : Player_Red ); }

	// Returns the piece state used by the given player.
	static ESquareState GetPlayerSquare( EPlayer player );

private:
	// The memory holding the game state.
	// NOTE: assumes a kBoardSize of 8
	unsigned __int64 m_blackPieces;
	unsigned __int64 m_redPieces;

	// Sets the game state of a space.
	ESquareState SetSquareState( const SPosition& pos, ESquareState state ) 
	{ 
		static const unsigned __int64 one = 1;

		if( pos.IsValid() ) 
		{
			switch( state )
			{
			case SquareState_Red:
				m_blackPieces &= ~( one << pos.ToIndex() );
				m_redPieces   |=  ( one << pos.ToIndex() );
				break;
			case SquareState_Black:
				m_redPieces   &= ~( one << pos.ToIndex() );
				m_blackPieces |=  ( one << pos.ToIndex() );
				break;
			case SquareState_Blank:
			default:
				m_blackPieces &= ~( one << pos.ToIndex() );
				m_redPieces   &= ~( one << pos.ToIndex() );
				break;
			}
			return state; 
		}
		return SquareState_Blank;
	}

	// Determines the next position from the start position given a number from 0 to 3 which represents one of the 4 diagonals.
	// Returns false of the next position requested isn't on the board.
	static bool GetNextSpace( const SPosition& start, int moveIndex, SPosition& next );
	// Determines the position that is inbetween the given two positions.
	// Returns false if there is no position between the given two positions.
	static bool GetMiddlePosition( const SPosition& start, const SPosition& next, SPosition& middle );

	// Adds non-jump moves from the start position for the given player.
	bool AddSimpleMoves( EPlayer player, const SPosition& start, std::vector<CMove>& moves ) const;
	// Adds all jump moves from the start position for the given player.
	bool AddJumpMoves( EPlayer player, const SPosition& start, bool hasJumps, std::vector<CMove>& moves ) const;
	// Adds multi-jump moves from the start move for the given player.
	bool AddNextJumpMoves( EPlayer player, const CMove& start, std::vector<CMove>& moves ) const;
};

