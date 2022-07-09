#include <vector>

namespace MonteCarlo {


	class MonteCarloTree {
		struct Node {
			Node(Node* parent = nullptr);

			Node* GetNonVisitedChild();

			Node* mParent;
			std::vector<Node*> mChildren;

			//data
			unsigned int mVisitedTimes;
			int mTotalSimulationReward;
		};

	public:
		MonteCarloTree(void);
		~MonteCarloTree(void);

		void Start(void);
		void Update(void);
		void End(void);

	private:
		//Principle of operation
		Node* Selection(Node* root);
		void Expansion(Node* node);
		void Simulation(Node* node);
		void BackPropagation(Node* node);

		//Principle of operation helpers
		//...

		//Member variables
		Node* mRoot;

		int mMaximumIterations;
		int mCurrentIterations;
		int mMinimumVisitedTimes;
	};
}