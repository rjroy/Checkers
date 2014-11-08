#include "StdAfx.h"
#include "CheckersBoard.h"

CPerfTimer CCheckersBoard::s_GetMoves( "CCheckersBoard::GetMoves" );
CPerfTimer CCheckersBoard::s_IsValidMove( "CCheckersBoard::IsValidMove" );
CPerfTimer CCheckersBoard::s_MakeMoveIfValid( "CCheckersBoard::MakeMoveIfValid" );

//--------------------------------------------------------------------------------------
static int BitCount( unsigned int i )
{
	i = i - ((i >> 1) & 0x55555555);
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

//--------------------------------------------------------------------------------------
static int BitCount( unsigned __int64 l )
{
	return BitCount( (unsigned int)l ) + BitCount( (unsigned int)(l >> 32) );
}

//--------------------------------------------------------------------------------------
CCheckersBoard::CCheckersBoard(const CCheckersBoard& cpy, EPlayer movingPlayer, const CMove& move)
{
	*this = cpy;
	MakeMoveIfValid( movingPlayer, move );
}

//--------------------------------------------------------------------------------------
void CCheckersBoard::Initialize()
{
	m_blackPieces = 0ll;
	m_redPieces = 0ll;
	m_blackKings = 0ll;
	m_redKings = 0ll;
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
bool CCheckersBoard::IsValidMove( EPlayer player, const CMove& move, std::vector<SPosition>* pRemovedPieces, SPosition* pFinalPosition, ESquareState* pNewState ) const
{
	CPerfTimerCall __call( s_IsValidMove );

	if( pRemovedPieces )
		pRemovedPieces->clear();

	assert( move.m_start.IsValid() );
	assert( GetPlayerOwner( GetSquareState( move.m_start ) ) == player );
	assert( move.m_sequence.size() );

	EPlayer opponentPlayer = GetOpponent( player );
	bool isKing = IsKing( GetSquareState( move.m_start ) );
	
	// Only test for loops with kings since only they can jump backwards.
	if( isKing && EndsInLoop( move.m_sequence ) )
		return false;

	SPosition curr = move.m_start;
	SPosition prev( 0, 0 );
	bool hasJumped = false;
	for( size_t i = 0; i < move.m_sequence.size(); ++i )
	{
		// Since we know how the moves are being build, we just need to do asserts.
		assert( i == 0 || hasJumped );

		SPosition next = move.m_sequence[i];
		assert( next.IsValid() );

		assert( GetSquareState( next ) == SquareState_Blank );
		if( next == prev )
			return false;

		// NOTE: The range of m_x and m_y is [0,7] therefore an int cast won't overflow.
		int disX = abs( (int)curr.m_x - (int)next.m_x );
		int disY = abs( (int)curr.m_y - (int)next.m_y );

		// Since we know how the moves are being build, we just need to do asserts.
		assert( disX == disY );
		assert( disX || disY );
		assert( disX <= 2 );
		assert( i == 0 || disY == 2 );

		// Only kings can reverse direction.
		if( !isKing )
		{
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
		}

		// Jump test.
		if( disX == 2 )
		{
			SPosition middle;
			if( !GetMiddlePosition( curr, next, middle ) )
				return false;
			assert( GetPlayerOwner( GetSquareState( middle ) ) == opponentPlayer );
			if( pRemovedPieces )
				pRemovedPieces->push_back( middle );
			hasJumped = true;
		}

		prev = curr;
		curr = next;
	}
	if( pFinalPosition )
		*pFinalPosition = curr;
	if( pNewState )
	{
		// If the piece reaches the back row then it becomes a king if it isn't already.
		switch( player )
		{
		case Player_Red:
			*pNewState = (curr.m_y == 7 || isKing) ? SquareState_RedKing : SquareState_Red;
			break;
		case Player_Black:
			*pNewState = (curr.m_y == 0 || isKing) ? SquareState_BlackKing : SquareState_Black;
			break;
		default:
			// error state.
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------
bool CCheckersBoard::MakeMoveIfValid( EPlayer player, const CMove& move )
{
	CPerfTimerCall __call( s_MakeMoveIfValid );

	std::vector<SPosition> removed;
	SPosition final;
	ESquareState newState;

	if( !IsValidMove( player, move, &removed, &final, &newState ) ) 
		return false;

	SetSquareState( move.m_start, SquareState_Blank );
	for( size_t i = 0; i < removed.size(); ++i )
		SetSquareState( removed[i], SquareState_Blank );
	SetSquareState( final, newState );

	return true;
}

//--------------------------------------------------------------------------------------
int CCheckersBoard::CalculatePlayerScore( EPlayer player ) const
{
	// Score kings as 2 points.

	int redScore   = BitCount( m_redPieces   ) + 2 * BitCount( m_redKings );
	int blackScore = BitCount( m_blackPieces ) + 2 * BitCount( m_blackKings );

	// Test for win state.
	if( !redScore )
		blackScore = CCheckersBoard::MaxScore;
	if( !blackScore )
		redScore = CCheckersBoard::MaxScore;

	return ( player == Player_Red ) ? ( redScore - blackScore ) : ( blackScore - redScore );
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

		if( GetSquareState( test.m_sequence[ 0 ] ) != SquareState_Blank )
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

		if( GetSquareState( test.m_sequence[ 0 ] ) == SquareState_Blank )
			continue;

		if( !GetNextSpace( test.m_sequence[ 0 ], move, test.m_sequence[ 0 ] ) )
			continue;

		if( GetSquareState( test.m_sequence[ 0 ] ) != SquareState_Blank )
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

	EPlayer opponentPlayer = GetOpponent( player );

	const SPosition& prev = test.m_sequence[ test.m_sequence.size() - 2 ];
	SPosition& curr = test.m_sequence[ test.m_sequence.size() - 1 ];

	for( unsigned int move = 0; move < kMoveIndexLimit; ++move )
	{
		if( !GetNextSpace( prev, move, curr ) )
			continue;

		if( GetPlayerOwner( GetSquareState( curr ) ) != opponentPlayer )
			continue;

		if( !GetNextSpace( curr, move, curr ) )
			continue;

		if( GetSquareState( curr ) != SquareState_Blank )
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
	CPerfTimerCall __call( s_GetMoves );

	bool hasJumps = false;

	for( unsigned int x = 0; x < kBoardSize; ++x )
	{
		for( unsigned int y = 0; y < kBoardSize; ++y )
		{
			SPosition start( x, y );
			if( GetPlayerOwner( GetSquareState( start ) ) == player )
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
bool CCheckersBoard::EndsInLoop( const std::vector<SPosition>& sequence )
{
	size_t maxLoopSize = sequence.size() >> 1;
	for( size_t testSize = 2; testSize <= maxLoopSize; ++testSize )
	{
		bool found = true;
		for( int i1 = sequence.size() - 1, i2 = sequence.size() - 1 - testSize; i1 >= 0 && i2 >= 0; i1--, i2-- )
		{
			if( sequence[i1] != sequence[i2] )
			{
				found = false;
				break;
			}
		}
		if( found )
			return true;
	}
	return false;
}
