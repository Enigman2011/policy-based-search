**Note:** This project really should be called "best-first search".  The policy referred to is the design pattern policy (aka strategy), not the automated planning policy.

Although there are existing best-first search/A`*` implementations in C++, I couldn't find one that exactly fit my requirements.
[Boost](http://www.boost.org/doc/libs/release/libs/graph/doc/astar_search.html) looks powerful but very complicated given the conceptual overhead of the BGL.  Others were generally old-fashioned or unmaintained.

This implementation is:
  * **modern**: it utilizes C++11 and policy-based design.
  * **domain-independent**: it implements the Template Method pattern (with policy classes rather than subclasses) to abstract the problem from the search algorithm.
  * **canonical**: it models search according to the definition in [AIMA](http://aima.cs.berkeley.edu/).
  * **efficient**: it is designed to use an amortized O(1) hash lookup for find operations on the frontier.

There are two examples included:
  * **Romania**: Recognizable to anyone that has read AIMA, this trivial example demonstrates path planning in an explicit graph.
  * **TSP**: This Travelling Salesman Problem example is about as complicated as it gets.  Incremental edge-wise search through implicit combinatorial space.  An explanatory paper is coming...

There is also an implementation of **Recursive Best-First Search** (RBFS).

Requirements:
  * C++11-conformant compiler, such as gcc 4.7 or clang 3.1`*`.
  * Boost >= 1.49.0, for the iterable and mutable priority queues in Boost.Heap.

Any assistance or comments welcome.

`*`Some of the examples may require clang 3.2.