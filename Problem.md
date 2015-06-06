The first and hardest thing to do is define the problem.  Luckily, here's one I prepared earlier: **Romania**.

# Traits #
Create a traits class that defines the types of objects in a problem.  There is no right or wrong here, except in the case of `node`, which must be a pointer type and you would be foolish to use anything other than a shared\_ptr.
```
struct Romania
{
	typedef std::string state;
	typedef std::string action;
	typedef unsigned int cost; // Return type of f().
	typedef cost pathcost; // Return type of g().
	typedef std::shared_ptr<jsearch::DefaultNode<Romania>> node;
	typedef cost heuristic_cost; // Return type of h().
};
```

Before or at the same time as defining the traits, you may need to create classes to represent state and action if they are too complex to represent in a string or such.  That part is entirely up to you.

The return types of f(), g() and h() are typically but not necessarily the same.  I haven't tested an example where h() is radically different.

# Policies #
The fun part.  Create policy classes that define how your problem is processed.  Useful defaults are provided where possible.

The following code returns the `PathCost` for performing `ACTION` in `STATE`.  `COST` is a global variable that holds the cost of all the roads.  The algorithm doesn't know how the problem data is structured because it just calls `step_cost()`.
```
template <typename Traits>
class Distance
{
public: // Public interface, not sure if these typedefs are required.
	typedef typename Traits::state State;
	typedef typename Traits::action Action;
	typedef typename Traits::pathcost PathCost;
	
protected:
	PathCost step_cost(State const &STATE, Action const &ACTION) const // Required function signature.
	{
		return COST.at(STATE).at(ACTION); // Definition is entirely dependent on the problem.
	}
};
```

# Initial state #
An object of type State that has the value of the initial state.
E.g.:
```
State const INITIAL("Arad");
```

# Putting it all together #
Construct a class for your problem from the Problem class template by passing the traits class and all the policy classes that you defined as template parameters.
Like so:
```
Problem<Romania, Distance, Neighbours, Visit, GoalTest> const BUCHAREST(INITIAL); // The problem is to get to Bucharest.
```
Note that it uses the default policies for the last two template arguments.