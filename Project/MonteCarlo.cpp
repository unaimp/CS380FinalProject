#include "MonteCarlo.h"


namespace MonteCarlo {
	MonteCarloTree::MonteCarloTree(void) : 
		mRoot(nullptr),
		mCurrentIterations(0),
		mMinimumVisitedTimes(3),
		mMaximumIterations(100)
	{}

	MonteCarloTree::~MonteCarloTree(void) {

	}


	void MonteCarloTree::Start(void) {

	}

	void MonteCarloTree::End(void) {

	}

	void MonteCarloTree::Update(void) {
		mCurrentIterations = 0;

		while (mCurrentIterations < mMaximumIterations) {
			mCurrentIterations++;


			//Principle of operations
			Node* selectedNode = Selection(mRoot);

			//Unless selectedNode ends the game decisively (e.g. win/loss) for either player, 
			//...

			Expansion(selectedNode);
		}
	}


	//PRINCIPLE OF OPERATIONS
	/*Start from root R and select successive child nodes until a leaf node L is reached. 
	The root is the current game state and a leaf is any node that has a potential child from which
	no simulation (playout) has yet been initiated.
	Upper Confidence Bound applied to trees or shortly UCT*/
	MonteCarloTree::Node* MonteCarloTree::Selection(Node* root) {
		//if the node has not been visited more than x times, pick a random child
		if(root->mVisitedTimes < mMinimumVisitedTimes)
			return root->GetNonVisitedChild();

		//Use UCT formula
		float bestUCT = -10000.f;
		Node* betterOption;

		//for (auto& it : root->mChildren) {
		//	if(//formula)
		//}

		return betterOption;
	}

	/*create one (or more) child nodes and choose node C from one of them. Child nodes are any valid moves 
	from the game position defined by L.*/
	void MonteCarloTree::Expansion(Node* node) {

	}


	/* Complete one random playout from node C
	A playout may be as simple as choosing uniform random moves until the game is decided */
	void MonteCarloTree::Simulation(Node* node) {
		//During simulation the moves are chosen with respect to a function called rollout policy function

	}

	/*Use the result of the playout to update information in the nodes on the path from C to R.*/
	void MonteCarloTree::BackPropagation(Node* node) {
		if (node == nullptr)		return; //root node



		BackPropagation(node->mParent);
	}






	MonteCarloTree::Node::Node(Node* parent) : mVisitedTimes(0u), mTotalSimulationReward(0){
		//add parent as parent pointer
		mParent = parent;

		//add this node to the children vector of parent
		if (parent)
			parent->mChildren.push_back(this);
	}

	MonteCarloTree::Node* MonteCarloTree::Node::GetNonVisitedChild() {
		for (unsigned int i = 0; i < mChildren.size(); i++) {
			if (mChildren[i]->mVisitedTimes == 0) //get a not visited child
				return mChildren[i];
		}
	}
}