

namespace MonteCarlo {

	enum Moves {
		//enum ActualAction {
			M_NONE,
			M_MOVE_UP,
			M_MOVE_RIGHT,
			M_MOVE_LEFT,
			M_MOVE_DOWN,
			M_PLACE_WALL
			//...
		//};


		//ActualAction mAction;
	};


	struct State {
		State(Moves a, const int p_r, const int p_c, bool AI);
		//State of the player from where take following actions
		int mRow;
		int mColumn;

		bool IsTerminal();
		bool mAI;
		Moves mMove;
	};
}