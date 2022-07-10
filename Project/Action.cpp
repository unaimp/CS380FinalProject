#include "Action.h"

using namespace MonteCarlo;


State::State(Moves a, const int p_r, const int p_c, bool AI) : mMove(a), mAI(AI) {
	switch (mMove) {
	case Moves::M_MOVE_UP:
		if(AI)
			mRow = p_r - 1;
		else
			mRow = p_r + 1;
		mColumn = p_c;
		break;
	case Moves::M_MOVE_RIGHT:
		mRow = p_r;
		mColumn = p_c + 1;
		break;
	case Moves::M_MOVE_LEFT:
		mRow = p_r;
		mColumn = p_c - 1;
		break;
	case Moves::M_MOVE_DOWN:
		if (AI)
			mRow = p_r + 1;
		else
			mRow = p_r - 1;
		mColumn = p_c;
		break;
	case Moves::M_PLACE_WALL:
		mRow = p_r;
		mColumn = p_c;
		//...
		break;
	default:
		mRow = p_r;
		mColumn = p_c;
	}
}

bool State::IsTerminal() {
	//need to differentiate between win or lose to award points to the play
	if (mAI) {
		if (/*winning condition*/0)
			return true;


		return false;
	}
	else {
		if (/*winning condition*/0)
			return true;


		return false;
	}

}