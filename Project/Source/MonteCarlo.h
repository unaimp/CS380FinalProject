#include <vector>


namespace MonteCarlo {
	struct Simulator; //fwd declaration

	class MonteCarloTree {
	public:
		struct Node {
			Node();
			Node(Node* parent, Moves m = Moves::M_NONE);
			~Node();

			void CreateChildren(QuoridorPlayer* q);

			Node* mParent;
			std::vector<Node*> mChildren;

			State* mState;
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
	};



	struct Simulator {
		double Simulate(MonteCarloTree::Node* startingPoint, QuoridorPlayer* AI, QuoridorPlayer* player);
		State& RollOut(const State currentState, QuoridorPlayer* q, bool AITurn, bool fwdNOT, bool rightNOT, bool leftNOT);

	private:
		int mPlayerRow, mPlayerColumn;
		int mAIRow, mAIColumn;
	};
}