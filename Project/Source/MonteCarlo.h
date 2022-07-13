#include <vector>


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
			double mTotalSimulationReward;
		};
		MonteCarloTree(void);
		~MonteCarloTree(void);

		void Start(void);
		void Execute(void);
		void End(void);

	private:
		void Eraser(Node* n);
		//Principle of operation
		Node* Selection(Node* currentNode);
		Node* Expansion(Node* leafNode);
		double Simulation(Node* node);
		void BackPropagation(Node*, double simValue);
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
	};



	struct Simulator {
		double Simulate(TileQ& AITile, TileQ& playerTile, QuoridorPlayer* AI, QuoridorPlayer* player, bool AIStarts);
		State& RollOut(const State currentState, QuoridorPlayer* q, bool AITurn, std::vector<Moves>& posibleMoves = std::vector<Moves>());

	private:
		int mPlayerRow, mPlayerColumn;
		int mAIRow, mAIColumn;

		void PlaceWall() {};
	};
}