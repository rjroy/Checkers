#include "StdAfx.h"
#include "CheckersBoard.h"

//--------------------------------------------------------------------------------------
static int BitCount( unsigned int i )
{
	i = i - ((i >> 1) & 0x55555555);
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static int BitCount( unsigned __int64 l )
{
	return BitCount( (unsigned int)l ) + BitCount( (unsigned int)(l >> 32) );
}

//--------------------------------------------------------------------------------------
int SPosition::Compare( const SPosition& rhs ) const
{
	int result = m_x - rhs.m_x;
	if( result )
		return result;
	return m_y - rhs.m_y;
}

//--------------------------------------------------------------------------------------
int CMove::Compare( const CMove& rhs ) const
{
	int result = m_start.Compare( rhs.m_start );
	if( result )
		return result;
	
	result = m_sequence.size() - rhs.m_sequence.size();
	if( result )
		return result;

	// NOTE: m_sequence.size() == rhs.m_sequence.size()
	for( size_t i = 0; i < m_sequence.size(); ++i )
	{
		result = m_sequence[i].Compare( rhs.m_sequence[i] );
		if( result )
			return result;
	}

	return 0;
}

//--------------------------------------------------------------------------------------
void CCheckersBoard::Initialize()
{
	m_blackPieces = 0l;
	m_redPieces = 0l;
	for( int i = 0; i < 4; ++i )
	{
		SetSquareState( SPosition(1 + i * 2, 0), SquareState_Red );
		SetSquareState( SPosition(0 + i * 2, 1), SquareState_Red );
		SetSquareState( SPosition(1 + i * 2, 2), SquareState_Red );

		SetSquareState( SPosition(0 + i * 2, 5), SquareState_Black );
		SetSquareState( SPosition(1 + i * 2, 6), SquareState_Black );
		SetSquareState( SPosition(0 + i * 2, 7), SquareState_Black );
	}
}

//--------------------------------------------------------------------------------------
bool CCheckersBoard::IsValidMove( EPlayer player, const CMove& move, std::vector<SPosition>* pRemovedPieces, SPosition* pFinalPosition ) const
{
	if( pRemovedPieces )
		pRemovedPieces->clear();

	if( !move.m_start.IsValid() )
		return false;

	if( GetSquareState( move.m_start ) != GetPlayerSquare( player ) )
		return false;

	if( !move.m_sequence.size() )
		return false;

	ESquareState opponentSquare = GetPlayerSquare( GetOpponent( player ) );

	SPosition curr = move.m_start;
	bool hasJumped = false;
	for( size_t i = 0; i < move.m_sequence.size(); ++i )
	{
		if( i > 0 && !hasJumped )
			return false;

		SPosition next = move.m_sequence[i];
		if( !next.IsValid() )
			return false;

		if( GetSquareState( next ) != SquareState_Blank )
			return false;

		// NOTE: The range of m_x and m_y is [0,8] therefore an int cast won't overflow.
		int disX = abs( (int)curr.m_x - (int)next.m_x );
		int disY = abs( (int)curr.m_y - (int)next.m_y );
		if( disX != disY )
			return false;

		if( disX == 0 || disX > 2 )
			return false;

		if( i > 0 && disY == 1 )
			return false;

		// CheckersLite: Can only ever move in one direction vertically.
		switch( player )
		{
		case Player_Red:
			if( curr.m_y > next.m_y )
				return false;
			break;
		case Player_Black:
			if( curr.m_y < next.m_y )
				return false;
			break;
		}

		// Jump test.
		if( disX == 2 )
		{
			SPosition middle;
			if( !GetMiddlePosition( curr, next, middle ) )
				return false;
			if( GetSquareState( middle ) != opponentSquare )
				return false;

			if( pRemovedPieces )
				pRemovedPieces->push_back( middle );
			hasJumped = true;
		}

		curr = next;
	}
	if( pFinalPosition )
		*pFinalPosition = curr;

	return true;
}

//--------------------------------------------------------------------------------------
bool CCheckersBoard::MakeMoveIfValid( EPlayer player, const CMove& move )
{
	std::vector<SPosition> removed;
	SPosition final;

	if( !IsValidMove( player, move, &removed, &final ) ) 
		return false;

	SetSquareState( move.m_start, SquareState_Blank );
	for( size_t i = 0; i < removed.size(); ++i )
		SetSquareState( removed[i], SquareState_Blank );
	SetSquareState( final, GetPlayerSquare( player) );

	return true;
}

//--------------------------------------------------------------------------------------
int CCheckersBoard::CalculatePlayerScore( EPlayer player ) const
{
	int result = 0;

	result += ( player == Player_Red )   ? BitCount( m_redPieces )   : -BitCount( m_redPieces );
	result += ( player == Player_Black ) ? BitCount( m_blackPieces ) : -BitCount( m_blackPieces );

	return result;
}

//--------------------------------------------------------------------------------------
bool CCheckersBoard::AddSimpleMoves( EPlayer player, const SPosition& start, std::vector<CMove>& moves ) const
{
	bool added = false;

	CMove test;
	test.m_start = start;
	test.m_sequence.resize( 1 );

	for( unsigned int move = 0; move < kMoveIndexLimit; ++move )
	{
		if( !GetNextSpace( test.m_start, move, test.m_sequence[ 0 ] ) )
			continue;
		if( !IsValidMove( player, test ) )
			continue;

		moves.push_back( test );
		added = true;
	}

	return added;
}

//--------------------------------------------------------------------------------------
bool CCheckersBoard::AddJumpMoves( EPlayer player, const SPosition& start, bool hasJumps, std::vector<CMove>& moves ) const
{
	bool added = false;

	CMove test;
	test.m_start = start;
	test.m_sequence.resize( 1 );

	for( unsigned int move = 0; move < kMoveIndexLimit; ++move )
	{
		if( !GetNextSpace( test.m_start, move, test.m_sequence[ 0 ] ) )
			continue;

		if( !GetNextSpace( test.m_sequence[ 0 ], move, test.m_sequence[ 0 ] ) )
			continue;

		if( !IsValidMove( player, test ) )
			continue;

		if( !added && !hasJumps )
			moves.clear();

		moves.push_back( test );
		AddNextJumpMoves( player, test, moves );
		added = true;
	}

	return added;
}

//--------------------------------------------------------------------------------------
bool CCheckersBoard::AddNextJumpMoves( EPlayer player, const CMove& start, std::vector<CMove>& moves ) const
{
	bool added = false;

	CMove test( start );

	test.m_sequence.resize( start.m_sequence.size() + 1 );

	if( test.m_sequence.size() < 2 )
		return added;

	const SPosition& prev = test.m_sequence[ test.m_sequence.size() - 2 ];
	SPosition& curr = test.m_sequence[ test.m_sequence.size() - 1 ];

	for( unsigned int move = 0; move < kMoveIndexLimit; ++move )
	{
		if( !GetNextSpace( prev, move, curr ) )
			continue;

		if( !GetNextSpace( curr, move, curr ) )
			continue;

		if( !IsValidMove( player, test ) )
			continue;

		moves.push_back( test );
		AddNextJumpMoves( player, test, moves );
		added = true;
	}

	return added;
}

//--------------------------------------------------------------------------------------
bool CCheckersBoard::GetMoves( EPlayer player, std::vector<CMove>& moves ) const
{
	ESquareState playerState = GetPlayerSquare( player );

	bool hasJumps = false;

	for( unsigned int x = 0; x < kBoardSize; ++x )
	{
		for( unsigned int y = 0; y < kBoardSize; ++y )
		{
			SPosition start( x, y );
			if( GetSquareState( start ) == playerState )
			{
				if( !hasJumps )
					AddSimpleMoves( player, start, moves );
				hasJumps |= AddJumpMoves( player, start, hasJumps, moves );
			}
		}
	}

	return !moves.empty();
}

//--------------------------------------------------------------------------------------
ESquareState CCheckersBoard::GetPlayerSquare( EPlayer player )
{
	switch( player )
	{
	case Player_Red:
		return SquareState_Red;
	case Player_Black:
		return SquareState_Black;
	default:
		return SquareState_Blank;
	};
}

//--------------------------------------------------------------------------------------
bool CCheckersBoard::GetNextSpace( const SPosition& start, int moveIndex, SPosition& next )
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
bool CCheckersBoard::GetMiddlePosition( const SPosition& start, const SPosition& next, SPosition& middle )
{
	middle.m_x = ( start.m_x + next.m_x ) / 2;
	middle.m_y = ( start.m_y + next.m_y ) / 2;
	
	return( middle.IsValid() && middle != start && middle != next );
}
