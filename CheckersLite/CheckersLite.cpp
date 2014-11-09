// CheckersLite.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Display.h"
#include "ComputerPlayer.h"
#include "ComputerPlayer.inl"

#include <time.h>
#include <iostream>
using namespace std;

bool UserMove( CCheckersBoard& board, const CDisplay& display, EPlayer userPlayer );


int _tmain(int argc, _TCHAR* argv[])
{
	srand( (unsigned int)time(NULL) );

	
	CComputerPlayer<CCheckersBoard> p1( Player_Red, 3 );
	CComputerPlayer<CCheckersBoard> p2( Player_Black, 10 );
	//EPlayer p2 = CCheckersBoard::GetOpponent( p1.GetPlayer() );

	unsigned int p1Wins = 0;
	unsigned int p2Wins = 0;

	for( ;; ) // for repeated testing.
	{
	CDisplay display;
	CCheckersBoard board;

	display.Show( cout, board );
	cout << endl;
	for( ;; ) 
	{
		if( !p1.Move( board ) )
		{
			display.Show( cout, board );
			break;
		}

		display.Show( cout, board );
		cout << "P1: " << board.CalculatePlayerScore( p1.GetPlayer() ) << endl << endl;

		// if( !UserMove( board, display, p2 ) ) break;

		if( !p2.Move( board ) )
		{
			display.Show( cout, board );
			break;
		}

		display.Show( cout, board );
		cout << "P2: " << board.CalculatePlayerScore( p2.GetPlayer() ) << endl << endl;
	}

	int p1Score = board.CalculatePlayerScore( p1.GetPlayer() );
	int p2Score = board.CalculatePlayerScore( p2.GetPlayer() );
	if( p1Score > p2Score )
	{
		p1Wins++;
		cout << "Player 1 Wins!" << endl;
	}
	else if( p2Score > p1Score )
	{
		p2Wins++;
		cout << "Player 2 Wins!" << endl;
	}
	else
		cout << "Tie." << endl;
	cout << "[ p1: " << p1Wins << " ; p2: " << p2Wins << "]" << endl;

	cout << "DONE" << endl;
	cout << CCheckersBoard::s_GetMoves;
	cout << CCheckersBoard::s_AddSimpleMoves;
	cout << CCheckersBoard::s_AddJumpMoves;
	cout << CCheckersBoard::s_AddNextJumpMoves;
	cout << CCheckersBoard::s_IsValidMove;
	cout << CCheckersBoard::s_MakeMoveIfValid;
	cout << CComputerPlayer<CCheckersBoard>::s_AlphaBeta;
	cout << CComputerPlayer<CCheckersBoard>::s_Move;

	cin.get();
	}

	return 0;
}

bool UserMove( CCheckersBoard& board, const CDisplay& display, EPlayer userPlayer)
{
	std::vector<CMove> moves;
	if( !board.GetMoves( userPlayer, moves ) )
		return false;
	if( moves.empty() )
		return false;

	unsigned int selection;
	do {
		display.ShowMoves( cout, moves );
		cout << "Please select a move: ";
		cin >> selection;
	} while( selection >= moves.size() );
	if( !board.MakeMoveIfValid( userPlayer, moves[selection] ) )
	{
		cout << "failed to make the valid move: " << selection << endl;
		return false;
	}

	return true;
}