#include <vector>

namespace MonteCarlo {


	class MonteCarloTree {
		struct Node {
			Node* mParent;
			std::vector<Node*> mChildren;

			//data
		};

		MonteCarloTree(void);
		~MonteCarloTree(void);

		void Start(void);
		void Update(void);
		void End(void);

		//Principle of operation
		void Selection(Node* node);
		void Expansion(Node* node);
		void Simulation(Node* node);
		void BackPropagation(Node* node);

		//Principle of operation helpers
		//...
	};
}