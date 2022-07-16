#include <Stdafx.h>

using namespace MonteCarlo;

State::State() : mAI(false), mWallPlacement(false) {

}

State::State(TileQ tile, bool AI, bool wallplacement, TileQ wall)
	: mAI(AI), mTile(tile), mWallPlacement(wallplacement), mWall(wall) {
	
}

bool State::IsTerminal() {
	//need to differentiate between win or lose to award points to the play
	if (mAI) {
		if (mTile.row == 0)
			return true;


		return false;
	}
	else {
		if (mTile.row == 8)
			return true;


		return false;
	}

}