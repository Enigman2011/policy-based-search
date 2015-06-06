This is where you make important decisions that affect the behaviour of the algorithm.

# Heuristic #
If you are doing a heuristic search, you need to create a policy class that contains an h() function, like so:
```
template <typename Traits>
class EuclideanDistance
{
public:
	typedef typename Traits::state State;
	typedef typename Traits::pathcost PathCost;
	
protected:
	PathCost h(State const &STATE) const
	{
		auto const RESULT = SLD.at(STATE);
		return RESULT;
	}
};
```
At the moment the algorithm passes a State object to h(), but there is probably a good argument that this should be a Node, I'm just waiting to find the example that makes it.

# Implementation #
You must choose a cost function that determines how nodes are evaluated.  Defaults provided are A`*`, Greedy and Dijkstra, but you could create your own that implements f().

Do this by defining a type alias that takes a traits class as a template parameter, and fill in the other required parameters to the cost function with your policy classes.
```
template <typename Traits>
using CostFunction = AStar<Traits, EuclideanDistance>;
```

# Tie-breaking #
If you are using a cost function that that summarizes two or more costs (such as A`*`), then you must specify how to break ties.  The usual policy of breaking on low h() is provided.  Again, create a type alias and in the case of A`*`, pass in the heuristic policy:
```
template <typename Traits>
using TieBreaker = LowH<Traits, EuclideanDistance>;
```

If you must specify a tie-breaking policy even if you don't require one, such as in RBFS, use FalseTiePolicy to always return `false`, which is the equivalent of no tie-breaking.