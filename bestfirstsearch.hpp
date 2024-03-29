/*
    search.h: Best-first search algorithm, taken from Russell & Norvig's AIMA.
    Copyright (C) 2012  Jeremy W. Murphy <jeremy.william.murphy@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file bestfirstsearch.hpp
 * @brief Domain-independent best-first search functions (and hidden helper functions).
 */

#ifndef SEARCH_H
#define SEARCH_H

#include "evaluation.hpp"
#include "problem.hpp"
#include "utils/to_string.hpp"
#include "utils/queue_set.hpp"

#include <algorithm>
#include <stdexcept>
#include <limits>

#ifndef NDEBUG
#include <iostream>
#endif

namespace jsearch
{
#ifdef STATISTICS
	struct statistics
	{
		statistics() : popped(0), pushed(0), decreased(0), discarded(0) {}
		size_t popped;
		size_t pushed;
		size_t decreased;
		size_t discarded;
	};
	
	statistics stats;
#endif


	namespace detail
	{
		/**
        * @brief Encapsulate the top()+pop() calls into a single pop().
        */
        template <typename PriorityQueue>
        inline typename PriorityQueue::value_type pop(PriorityQueue &pq)
        {
            auto const E(pq.top());
            pq.pop();
            return E;
        }


        /**
		* @brief Handle the fate of a child being added to the frontier.
		*
		* @return: A Frontier element that equals
		* 				i) nullptr if CHILD was not added to the frontier
		* 				ii) CHILD if CHILD was added to the frontier, or
		* 				iii) another element if CHILD replaced it on the frontier.
		* */
		template <class Frontier>
        inline typename Frontier::value_type handle_child(Frontier &frontier, typename Frontier::const_reference const &CHILD)
		{
            typename Frontier::value_type result(nullptr); // Initialize to nullptr since it might be a bald pointer.

            auto const IT(frontier.find(CHILD->state()));

			if(IT != std::end(frontier))
			{
                auto const &DUPLICATE((IT->second)); // The duplicate on the frontier.
                if(CHILD->path_cost() < (*DUPLICATE)->path_cost())
				{
	#ifndef NDEBUG
                    std::cout << jwm::to_string(CHILD->state()) << ": replace " << (*DUPLICATE)->path_cost() << " with " << CHILD->path_cost() << ".\n";
	#endif
	#ifdef STATISTICS
					++stats.decreased;
	#endif
                    result = (*DUPLICATE); // Store a copy of the node that we are about to replace.
                    frontier.increase(DUPLICATE, CHILD); // The DECREASE-KEY operation is an increase because it is a max-heap.
                }
				else
				{
	#ifndef NDEBUG
                    std::cout << jwm::to_string(CHILD->state()) << ": keep " << (*DUPLICATE)->path_cost() << " and throw away " << CHILD->path_cost() << ".\n";
	#endif
	#ifdef STATISTICS
					++stats.discarded;
	#endif
				}
			}
			else
			{
                frontier.push(CHILD);
                result = CHILD;
	#ifndef NDEBUG
                std::cout << "frontier <= " << jwm::to_string(CHILD->state()) << "\n";
	#endif
	#ifdef STATISTICS
				++stats.pushed;
	#endif
			}

            return result;
		}
	}


	/**
	 * @brief goal_not_found is thrown... when... < drum roll > THE GOAL IS NOT FOUND!
	 */
	class goal_not_found : public std::exception
	{
	public:
		goal_not_found() {}
	};
	
    
    /**************************
	 * 	 	 Graph search	  *
	 **************************/
	template <template <typename T, typename Comparator> class PriorityQueue,
			template <typename Traits> class Comparator,
			template <typename T> class Set,
			template <typename Key, typename Value> class Map,
			typename Traits,
			template <typename Traits_> class StepCostPolicy,
			template <typename Traits_> class ActionsPolicy,
			template <typename Traits_> class ResultPolicy,
			template <typename Traits_> class GoalTestPolicy,
			template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
			template <typename Traits_,
				template <typename Traits__> class StepCostPolicy,
				template <typename Traits__> class ResultPolicy,
				template <typename Traits__> class CreatePolicy>
				class ChildPolicy = DefaultChildPolicy,
            typename Output>
	typename Traits::pathcost best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, CreatePolicy, ChildPolicy> const &PROBLEM, Output path)
	{
        typedef typename Traits::node Node;
		typedef typename Traits::state State;
		typedef typename Traits::action Action;
		// typedef typename Traits::pathcost PathCost;

        jsearch::queue_set<PriorityQueue<Node, Comparator<Traits>>, Map> frontier;
        Set<State> closed;

        frontier.push(PROBLEM.create(PROBLEM.initial, Node(), Action(), 0));

		while(!frontier.empty())
		{
            auto const S(detail::pop(frontier));
#ifndef NDEBUG
            std::cout << S->state() << " <= frontier\n";
#endif
#ifdef STATISTICS
			++stats.popped;
#endif
            if(PROBLEM.goal_test(S->state()))
			{
#ifndef NDEBUG
				std::cout << "frontier: " << frontier.size() << "\n";
				std::cout << "closed: " << closed.size() << "\n";
#endif
                std::function<Output(Output &, Node const &)> unravel =  [&](Output &path, Node const &node)
                                {
                                    *path++ = node->state();
                                    if(node->parent())
                                        return unravel(path, node->parent());
                                    else
                                        return path;
                                };
                unravel(path, S);
                return S->path_cost();
			}
			else
			{
                closed.insert(S->state());
                auto const &ACTIONS(PROBLEM.actions(S->state()));
                // TODO: Change to auto parameter declaration once C++14 is implemented.
                std::for_each(std::begin(ACTIONS), std::end(ACTIONS), [&](Action const &ACTION)
                {
                    auto const &SUCCESSOR(PROBLEM.result(S->state(), ACTION));
                    if(closed.find(SUCCESSOR) == std::end(closed))
                        detail::handle_child(frontier, PROBLEM.child(S, ACTION, SUCCESSOR));
                });
			}
		}

		throw goal_not_found();
	}


	/**************************
	 *		Tree search		  *
	 **************************/
	template <template <typename T, typename Comparator> class PriorityQueue,
			template <typename Traits> class Comparator,
			typename Traits,
			template <typename Traits_> class StepCostPolicy,
			template <typename Traits_> class ActionsPolicy,
			template <typename Traits_> class ResultPolicy,
			template <typename Traits_> class GoalTestPolicy,
			template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
			template <typename Traits_,
				template <typename Traits__> class StepCostPolicy,
				template <typename Traits__> class ResultPolicy,
				template <typename Traits__> class CreatePolicy>
				class ChildPolicy = DefaultChildPolicy>
	typename Traits::node best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, CreatePolicy, ChildPolicy> const &PROBLEM)
	{
		typedef typename Traits::node Node;
		// typedef typename Traits::state State;
		typedef typename Traits::action Action;
		// typedef typename Traits::pathcost PathCost;

		typedef PriorityQueue<Node, Comparator<Traits>> Frontier;

		Frontier frontier;
		frontier.emplace(PROBLEM.create(PROBLEM.initial, Node(), Action(), 0));

		while(!frontier.empty())
		{
            auto const S(detail::pop(frontier));

			if(PROBLEM.goal_test(S->state()))
			{
#ifndef NDEBUG
				std::cout << "frontier: " << frontier.size() << "\n";
#endif
				return S;
			}
			else
			{
				auto const &ACTIONS(PROBLEM.actions(S->state()));
                
				std::for_each(std::begin(ACTIONS), std::end(ACTIONS), [&](Action const &action)
				{
                    frontier.emplace(PROBLEM.child(S, action));
				});
			}
		}

		throw goal_not_found();
	}


	namespace recursive
	{
		template <typename Traits, template <typename Traits_> class TiePolicy, template <typename T> class PriorityQueue>
		class NodeCost : protected TiePolicy<Traits>
		{
			using TiePolicy<Traits>::split;

		public:
			typedef typename Traits::node Node;
			typedef typename Traits::cost Cost;
			typedef typename PriorityQueue<NodeCost<Traits, TiePolicy, PriorityQueue>>::handle_type handle_type;

			NodeCost(Node const &NODE, Cost const &COST) : node_(NODE), cost_(COST) {}

			const Node &node() const { return node_; }
			const Cost &cost() const { return cost_; }

			/**
			 * First compare on the stored cost, if they are equal use the TiePolicy.
			 */
			bool operator<(NodeCost<Traits, TiePolicy, PriorityQueue> const &OTHER) const
			{
				// Greater-than for max-heap.
				auto const RESULT(cost_ == OTHER.cost_ ? split(node_, OTHER.node_) : (cost_ > OTHER.cost_ ? true : false));
				return RESULT;
			}

			void update_cost(Cost const &COST) { cost_ = COST; }

			handle_type handle; // TODO: Encapsulate.

		private:
			Node node_;
			Cost cost_;
		};


#ifndef NDEBUG
		template <typename Traits, template <typename Traits_> class TiePolicy, template <typename T> class PriorityQueue>
		std::ostream& operator<<(std::ostream& stream, NodeCost<Traits, TiePolicy, PriorityQueue> const &O)
		{
			stream << "{" << *O.node() << ", " << O.cost() << "}";
			return stream;
		}
#endif


		/**
		 * If SearchResult::first == nullptr then SearchResult::pathcost contains a valid value.
		 * If SearchResult::first != nullptr then it is the goal node and SearchResult::second is undefined.
		 */
		template <typename Traits>
		using SearchResult = std::pair<typename Traits::node, typename Traits::pathcost>;

		
		/*******************************
		* Recursive best-first search *
		*******************************/
		/**
		 * This is the recursive implementation of the search, not to be called by clients.
		 */
		template <template <typename Traits> class CostFunction,
			template <typename Traits> class TiePolicy,
			template <typename T> class PriorityQueue,
			typename Traits,
			template <typename Traits_> class StepCostPolicy,
			template <typename Traits_> class ActionsPolicy,
			template <typename Traits_> class ResultPolicy,
			template <typename Traits_> class GoalTestPolicy,
			template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
			template <typename Traits_,
				template <typename Traits__> class StepCostPolicy,
				template <typename Traits__> class ResultPolicy,
				template <typename Traits__> class CreatePolicy>
				class ChildPolicy = DefaultChildPolicy>
		SearchResult<Traits> recursive_best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, CreatePolicy, ChildPolicy> const &PROBLEM, CostFunction<Traits> const &COST, typename Traits::node const &NODE, typename Traits::pathcost const &F_N, typename Traits::pathcost const &B)
		{
			// typedef typename Traits::node Node;
			// typedef typename Traits::state State;
			// typedef typename Traits::action Action;
			typedef typename Traits::pathcost PathCost;
			// typedef typename Traits::cost Cost;
			typedef NodeCost<Traits, TiePolicy, PriorityQueue> RBFSNodeCost; // uhh...
			// typedef PriorityQueue<RBFSNodeCost> ChildrenPQ;
			typedef SearchResult<Traits> RBFSResult;

			/*	A single-line comment (//) is a direct quotes from the algorithm, to show how it has been interpreted.
			*	Mainly so that if there is a bug, it will be easier to track down.  :)
			*
			*	It is assumed that the algorithm used 1-offset arrays.
			*/
#ifndef NDEBUG
			std::cerr << ">>> " << __FUNCTION__ << "(PROBLEM, COST, " << *NODE << ", " << F_N << ", " << B << ")\n";
#endif

// What I hope is a legitimate use of a macro.
#ifndef RBFS_INF
#define RBFS_INF	std::numeric_limits<PathCost>::max()
#endif
			auto const f_N(COST.f(NODE));

			// IF f(N)>B, return f(N)
			if(f_N > B)
				return RBFSResult(nullptr, f_N);

			// IF N is a goal, EXIT algorithm
			if(PROBLEM.goal_test(NODE->state()))
				return RBFSResult(NODE, 0);
			auto const ACTIONS(PROBLEM.actions(NODE->state()));

			// IF N has no children, RETURN infinity
			if(ACTIONS.empty())
				return RBFSResult(nullptr, RBFS_INF);

			PriorityQueue<RBFSNodeCost> children;

			// FOR each child Ni of N,
			for(auto const ACTION : ACTIONS)
			{
				auto const CHILD(PROBLEM.child(NODE, ACTION));
				auto const f_CHILD(COST.f(CHILD));
				// IF f(N)<F(N) THEN F[i] := MAX(F(N),f(Ni))
				// ELSE F[i] := f(Ni)
				auto const f_RESULT(f_N < F_N ? std::max(F_N, f_CHILD) : f_CHILD);
				auto const HANDLE(children.push(RBFSNodeCost(CHILD, f_RESULT)));
				(*HANDLE).handle = HANDLE; // Looks weird, makes sense.
			}

			// sort Ni and F[i] in increasing order of F[i]
			/*	They sort automatically.	*/

			// IF only one child, F[2] := infinity
			/*	Handle this sentinel value later.	*/

			// WHILE (F[1] <= B and F[1] < infinity)
			/*	TODO: I almost fail to see the point of testing for less than infinity???	*/
			while(children.top().cost() <= B && children.top().cost() < RBFS_INF)
			{
				auto it(children.ordered_begin());
				auto const &BEST(*it++);
				auto const SECOND_BEST_COST(it == children.ordered_end() ? RBFS_INF : it->cost());
				// F[1] := RBFS(N1, F[1], MIN(B, F[2]))
				auto const RESULT(recursive_best_first_search<CostFunction, TiePolicy, PriorityQueue>(PROBLEM, COST, BEST.node(), BEST.cost(), std::min(B, SECOND_BEST_COST)));
				if(!RESULT.first)
					(*BEST.handle).update_cost(RESULT.second);
				else
					return RESULT;
				// insert N1 and F[1] in sorted order
				/*	N1 is updated in-place.	*/
				children.update(BEST.handle);
			}

			// return F[1]
			return RBFSResult(nullptr, children.top().cost());
		}
	}

#ifdef RBFS_INF
#undef RBFS_INF
#endif


	/**
	 * \brief Recursive best-first search (RBFS) from Korf (1993).
	 *
	 * \return A goal Node from which the path can be reconstructed.
	 *
	 * \throws goal_not_found
	 */
	template <template <typename Traits> class CostFunction,
		template <typename Traits> class TiePolicy,
		template <typename T> class PriorityQueue,
		typename Traits,
		template <typename Traits_> class StepCostPolicy,
		template <typename Traits_> class ActionsPolicy,
		template <typename Traits_> class ResultPolicy,
		template <typename Traits_> class GoalTestPolicy,
		template <typename Traits_> class CreatePolicy = DefaultNodeCreator,
		template <typename Traits_,
			template <typename Traits__> class StepCostPolicy,
			template <typename Traits__> class ResultPolicy,
			template <typename Traits__> class CreatePolicy>
			class ChildPolicy = DefaultChildPolicy>
	typename Traits::node recursive_best_first_search(Problem<Traits, StepCostPolicy, ActionsPolicy, ResultPolicy, GoalTestPolicy, CreatePolicy, ChildPolicy> const &PROBLEM)
	{
		typedef typename Traits::node Node;
		// typedef typename Traits::state State;
		typedef typename Traits::action Action;
		typedef typename Traits::pathcost PathCost;


		constexpr auto const INF(std::numeric_limits<PathCost>::max());
		auto initial(PROBLEM.create(PROBLEM.initial, Node(), Action(), 0));
		CostFunction<Traits> const COST; // TODO: Design flaw?

		auto const RESULT(recursive::recursive_best_first_search<CostFunction, TiePolicy, PriorityQueue>(PROBLEM, COST, initial, COST.f(initial), INF));

		if(!RESULT.first)
			throw goal_not_found();
		
		return RESULT.first;
	}
}

#endif // SEARCH_H


