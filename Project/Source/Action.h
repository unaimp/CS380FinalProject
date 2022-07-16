

namespace MonteCarlo {

	enum Moves {
		M_NONE,
		M_MOVE_FWD,
		M_PLACE_WALL,
		M_MOVE_RIGHT,
		M_MOVE_LEFT,
		M_MOVE_BACKWD,
		M_WALL_FWD_A,
		M_WALL_FWD_B,
		M_WALL_RIGHT_A,
		M_WALL_RIGHT_B,
		M_WALL_LEFT_A,
		M_WALL_LEFT_B
	};


	struct State {
		State();
		State(TileQ tile, bool AI, bool wallplacement);
		//State of the player from where take following actions
		TileQ mTile;
		bool mWallPlacement;

		bool IsTerminal();
		bool mAI;
	};
}