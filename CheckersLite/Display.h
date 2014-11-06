#pragma once

#include <iostream>

#include "CheckersBoard.h"

class CDisplay
{
public:
	CDisplay(void) {} 
	~CDisplay(void) {}

	// Prints 100 newlines.
	void ClearScreen( std::ostream& os ) const;

	// Print the current state of the board.
	void Show( std::ostream& os, const CCheckersBoard& board ) const;

	// Prints the set of moves with numbers so that a user can select one.
	void ShowMoves( std::ostream& os, const std::vector<CMove>& moves ) const;
	
	// Prints the two character identifier for the given position.
	void ShowPos( std::ostream& os, const SPosition& pos ) const;
};

