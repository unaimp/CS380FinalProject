#include <vector>

namespace MonteCarlo {

	class MonteCarloTree {
		struct Node {
			Node*					mParent;
			std::vector<Node*>		mChildren;

			//data
			//...
		};

		MonteCarloTree();
		~MonteCarloTree();

		void Start(void);
		void Update(void);
		void End(void);


		Node*	mRoot;

	};

}