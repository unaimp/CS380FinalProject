#include <vector>

class TileQ;

namespace MonteCarlo {
	struct Simulator; //fwd declaration

	class MonteCarloTree {
	public:
		struct Node {
			Node();
			Node(Node* parent, TileQ tile, bool wallPlacement, bool AI);
			~Node();

			void CreateChildren(QuoridorPlayer* q);

			Node* mParent;
			std::vector<Node*> mChildren;

			State* mState;
			bool mAI;
			//data
			unsigned int mVisitedTimes;
			float mTotalSimulationReward;
		};
		MonteCarloTree(void);
		~MonteCarloTree(void);

		void Start(void);
		void Execute(void);
		void End(void);

		std::string GetStats() { return mStats; }

	private:
		void Eraser(Node* n);
		//Principle of operation
		Node* Selection(Node* currentNode);
		Node* Expansion(Node* leafNode);
		bool Simulation(Node* node);
		void BackPropagation(Node*, bool aiWon);
		void ActualMove();

		QuoridorPlayer* mAIPlayer;
		QuoridorPlayer* mHumanPlayer;

		Simulator* mSimulator;
		//Member variables
		Node* mRoot;

		float mUCTvar;

		int mMaximumIterations;
		int mCurrentIterations;
		int mMinimumVisitedTimes;

		int mAIWinTimes;
		int mPlayerWinTimes;

		std::string mStats;
	};



	struct Simulator {
		Simulator();
		bool Simulate(const TileQ& AITile, const TileQ& playerTile, QuoridorPlayer* AI, QuoridorPlayer* player, bool AIStarts);
		TileQ PlaceWall(const TileQ& origin, bool propagation_up, float decay = 0.05f, float growing = 0.15f);

		//void InfluenceUp(TileQ tile, float influence, int separation_value, bool last_was_separation);
		//void InfluenceDown(TileQ tile, float influence, int separation_value, bool last_was_separation);
		//void InfluenceLeft(TileQ tile, float influence, int separation_value, bool last_was_separation, bool go_up);
		//void InfluenceRight(TileQ tile, float influence, int separation_value, bool last_was_separation, bool go_up);

	private:
		int mPlayerRow, mPlayerColumn;
		int mAIRow, mAIColumn;

		State RollOut(const State currentState, QuoridorPlayer* movingQuoridor, QuoridorPlayer* otherPlayer, bool AITurn, std::vector<Moves>& posibleMoves = std::vector<Moves>());
	};
}