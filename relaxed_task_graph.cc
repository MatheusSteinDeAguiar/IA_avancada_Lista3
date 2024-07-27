
#include "relaxed_task_graph.h"

#include <iostream>
#include <vector>
#include <queue>

using namespace std;

namespace planopt_heuristics {
RelaxedTaskGraph::RelaxedTaskGraph(const TaskProxy &task_proxy)
    : relaxed_task(task_proxy),
      variable_node_ids(relaxed_task.propositions.size()) {
    /*
      TODO: add your code for exercise 2 (b) here. Afterwards
        - variable_node_ids[i] should contain the node id of the variable node for variable i
        - initial_node_id should contain the node id of the initial node
        - goal_node_id should contain the node id of the goal node
        - the graph should contain precondition and effect nodes for all operators
        - the graph should contain all necessary edges.
    */

    //Create nodes for variables
    for (size_t i = 0; i < relaxed_task.propositions.size(); i++) {
        variable_node_ids[i] = relaxed_task.propositions[i].id;
        graph.add_node(NodeType::OR);
    }

    //Create initial state node
    initial_node_id = graph.add_node(NodeType::AND);
    // Create edges from initial state to initial facts
    for (PropositionID id : relaxed_task.initial_state) {
        graph.add_edge(id, initial_node_id);
    }

    //Create goal state node
    goal_node_id = graph.add_node(NodeType::AND);
    // Create edges from goal facts to goal state
    for (PropositionID id : relaxed_task.goal) {
        graph.add_edge(goal_node_id,id);
    }

    // Create nodes for precondition and effect nodes for all operators
    for (RelaxedOperator op : relaxed_task.operators) {
        NodeID operator_node = graph.add_node(NodeType::AND);
        NodeID effect_node = graph.add_node(NodeType::OR, op.cost);

        // Add edges from precondition node to variable nodes
        for (PropositionID id : op.preconditions) {
            graph.add_edge(operator_node, id);
        }

        // Add edges from effect node to operator node
        graph.add_edge(effect_node,operator_node);

        //For every conditional effect in the operator
        for (PropositionID id : op.effects){
            graph.add_edge(id, effect_node);
        }
    }
}

void RelaxedTaskGraph::change_initial_state(const GlobalState &global_state) {
    // Remove all initial edges that where introduced for relaxed_task.initial_state.
    for (PropositionID id : relaxed_task.initial_state) {
        graph.remove_edge(variable_node_ids[id], initial_node_id);
    }

    // Switch initial state of relaxed_task
    relaxed_task.initial_state = relaxed_task.translate_state(global_state);

    // Add all initial edges for relaxed_task.initial_state.
    for (PropositionID id : relaxed_task.initial_state) {
        graph.add_edge(variable_node_ids[id], initial_node_id);
    }
}

bool RelaxedTaskGraph::is_goal_relaxed_reachable() {
    // Compute the most conservative valuation of the graph and use it to
    // return true iff the goal is reachable in the relaxed task.

    graph.most_conservative_valuation();
    return graph.get_node(goal_node_id).forced_true;
}

int RelaxedTaskGraph::additive_cost_of_goal() {
    // Compute the weighted most conservative valuation of the graph and use it
    // to return the h^add value of the goal node.

    // TODO: add your code for exercise 2 (c) here.
    graph.weighted_most_conservative_valuation();
    return graph.get_node(goal_node_id).additive_cost;
}

int RelaxedTaskGraph::ff_cost_of_goal() {
    // TODO: add your code for exercise 2 (e) here.
    graph.weighted_most_conservative_valuation();

    queue<NodeID> queue;
    unordered_set<NodeID> expanded;

    int sum_cost = 0;

    queue.push(goal_node_id);

    while(!queue.empty()){
        AndOrGraphNode current = graph.get_node(queue.front());
        queue.pop();

        if(expanded.find(current.id) != expanded.end()){
            continue;
        }

        expanded.insert(current.id);

        if(current.type == NodeType::AND){
            for(NodeID id : current.successor_ids){
                AndOrGraphNode successor = graph.get_node(id);

                if(expanded.find(id) == expanded.end()){
                    queue.push(id);
                }
            }
        }
        else if (current.type == NodeType::OR){
            if(expanded.find(current.achiever) == expanded.end()){
                queue.push(current.achiever);
            }
        }
    }

    for(NodeID id : expanded){
        sum_cost += graph.get_node(id).direct_cost;
    }

    return sum_cost;
}

}
