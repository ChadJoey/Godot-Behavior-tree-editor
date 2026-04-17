#include "../src/behavior_tree.h"
#include <cassert>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/error_macros.hpp>

void BT_Tree::set_tickrate(int i) {
	TickRate = i;
}

int BT_Tree::get_tickrate() {
	return TickRate;
}

void BT_Node::set_status(Status s) {
	status = s;
}

BT_Node::Status BT_Node::get_status() const {
	return status;
}

int BT_SequenceNode::_update(float dt) {
	if (child_nodes.is_empty()) {
		set_status(STATUS_FAILURE);
		return STATUS_FAILURE;
	}

	while (current_child < child_nodes.size()) {
		BT_Node *child = Object::cast_to<BT_Node>(child_nodes[current_child].operator Object *());
		if (!child) {
			set_status(STATUS_FAILURE);
			return STATUS_FAILURE;
		}

		int status = child->_update(dt);

		if (status == STATUS_RUNNING) {
			set_status(STATUS_RUNNING);
			return STATUS_RUNNING;
		} else if (status == STATUS_FAILURE) {
			set_status(STATUS_FAILURE);
			current_child = 0;
			return STATUS_FAILURE;
		} else if (status == STATUS_SUCCESS) {
			current_child++;
			continue;
		}
	}

	// All children succeeded
	set_status(STATUS_SUCCESS);
	current_child = 0;
	return STATUS_SUCCESS;
}

//selector fails only if all chilren fail
//succeeds if 1 childs succeeds
int BT_SelectorNode::_update(float dt) {
	if (child_nodes.is_empty()) {
		godot::print_error("Selector has no children");
		return STATUS_FAILURE;
	}
	//le funny variant cast for godot
	Variant v = child_nodes[current_child];
	BT_Node *child = Object::cast_to<BT_Node>(v.operator Object *());
	if (!child) {
		godot::print_error("Null or invalid child node");
		return STATUS_FAILURE;
	}

	int status = child->_update(dt);

	switch (status) {
		case STATUS_FAILURE:
			current_child++;
			if (current_child >= child_nodes.size()) {
				current_child = 0;
				return STATUS_FAILURE;
			}
			return STATUS_RUNNING;

		case STATUS_RUNNING:
			return STATUS_RUNNING;

		case STATUS_SUCCESS:
			current_child = 0;
			return STATUS_SUCCESS;
	}

	current_child = 0;
	return STATUS_FAILURE;
}

void BT_Tree::_process(double delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return; // Skip when editing
	}

	//tickrate too low
	if (TickRate <= 0) {
		set_status(STATUS_FAILURE);
		return;
	}
	float ticktime = 1 / TickRate;
	timer += delta;
	if (timer >= ticktime) {
		timer -= ticktime; //check for a valid argument from the gdscript else return failure status
		godot::print_line("ticking tree");
		int int_status = root->_update(delta);
		set_status(static_cast<Status>(int_status));
	}
}

void BT_Tree::_ready() {

	CreateTree();
}

void BT_Tree::set_tree_data(const Ref<BT_Tree_Resource> &data) {
	tree_data = data;
}

Ref<BT_Tree_Resource> BT_Tree::get_tree_data() const {
	return tree_data;
}

void BT_Tree::CreateTree() {
	if (!tree_data.is_valid()) {
		godot::print_error("Invalid tree resource.");
		return;
	}

	HashMap<int, BT_Node *> id_to_node;

	Array nodes = tree_data->get_all_nodes();

	// --- 1. Instantiate all nodes ---
	for (int i = 0; i < nodes.size(); i++) {
		Ref<BT_node_data> data = nodes[i];
		if (!data.is_valid())
			continue;

		String type = data->get_node_type();
		BT_Node *node = nullptr;

		if (type == "BT_Graph_SelectorNode")
			node = memnew(BT_SelectorNode);
		else if (type == "BT_Graph_ReactiveSelectorNode")
			node = memnew(BT_ReactiveSelectorNode);
		else if (type == "BT_Graph_SequenceNode")
			node = memnew(BT_SequenceNode);
		else if (type == "BT_Graph_ReactiveSequenceNode")
			node = memnew(BT_ReactiveSequenceNode);
		else if (type == "BT_Graph_ActionNode")
			node = memnew(BT_ActionNode);
		else if (type == "BT_Graph_ConditionNode")
			node = memnew(BT_ConditionNode);
		else if (type == "BT_Graph_InvertDecorator")
			node = memnew(BT_InvertDecorator);
		else if (type == "BT_Graph_RepeaterDecorator")
			node = memnew(BT_RepeaterDecorator);
		else if (type == "BT_Graph_WaitNode")
			node = memnew(BT_WaitNode);
		else if (type == "BT_Graph_ParralelNode")
			node = memnew(BT_ParralelNode);
		else
			continue;

		node->set_name(data->get_name());
		node->set_meta("bt_id", data->get_id());
		node->set_script(data->get_script());

		id_to_node[data->get_id()] = node;
	}

	// --- 2. Link children recursively ---
	for (int i = 0; i < nodes.size(); i++) {
		Ref<BT_node_data> data = nodes[i];
		if (!data.is_valid())
			continue;

		BT_Node *parent_node = id_to_node[data->get_id()];
		if (!parent_node)
			continue;

		Array children_ids = data->get_children_ids();
		for (int j = 0; j < children_ids.size(); j++) {
			int child_id = children_ids[j];
			if (!id_to_node.has(child_id))
				continue;

			BT_Node *child_node = id_to_node[child_id];

			parent_node->add_child(child_node); // for scene tree
			parent_node->add_bt_child(child_node); // for update
		}
	}

	// --- 3. Add root node to the scene ---
	if (id_to_node.has(1)) {
		BT_Node *root = id_to_node[1];
		this->add_child(root);
		set_root(root);
	}
}

void BT_ConditionNode::_initialize_condition() {
	//set the condition on game start
	godot::print_error("setting condition");
	set_condition();
}

int BT_ConditionNode::_update(float dt) {
	//made the abstraction to _update here bcs thats what the tree expects. and i wanted naming convention to be clear in evaluate here
	return evaluate(dt);
}

void BT_ConditionNode::set_condition() {
	if (has_method("condition_to_check")) {
		condition_call = Callable(this, "condition_to_check");
		godot::print_line("found condition function");

	} else {
		godot::print_error("function condition_to_check() not found");
	}
}

int BT_ConditionNode::evaluate(float dt) {
	if (condition_call.is_valid()) {
		Variant ret = condition_call.callv({ dt });
		godot::print_line("using gdscript function");
		return static_cast<bool>(ret) ? STATUS_SUCCESS : STATUS_FAILURE;
	} else {
		// fallback to C++ virtual
		godot::print_line("c++ fallback");
		bool result = condition_to_check();
		return result ? STATUS_SUCCESS : STATUS_FAILURE;
	}
}

int BT_InvertDecorator::_update(float dt) {
	if (child_nodes.is_empty()) {
		godot::print_error("Selector has no children");
		return STATUS_FAILURE;
	}
	//le funny variant cast for godot
	Variant v = child_nodes[0];
	BT_Node *child = Object::cast_to<BT_Node>(v.operator Object *());
	if (!child) {
		godot::print_error("Null or invalid child node");
		return STATUS_FAILURE;
	}


	int status = child->_update(dt);

	switch (status) {
		case STATUS_SUCCESS:
			godot::print_line("Inverted SUCCES -> FAILURE");
			return STATUS_FAILURE;
			break;
		case STATUS_RUNNING:
			return STATUS_RUNNING;
			break;
		case STATUS_FAILURE:
			godot::print_line("Inverted FAILURE -> SUCCES");
			return STATUS_SUCCESS;
			break;
		default:
			godot::print_error("Inverter no valid status found");
			//fallback
			return STATUS_FAILURE;
			break;
	}
}

int BT_RepeaterDecorator::_update(float dt) {
	if (child_nodes.is_empty()) {
		godot::print_error("Selector has no children");
		return STATUS_FAILURE;
	}
	//le funny variant cast for godot
	Variant v = child_nodes[0];
	BT_Node *child = Object::cast_to<BT_Node>(v.operator Object *());
	if (!child) {
		godot::print_error("Null or invalid child node");
		return STATUS_FAILURE;
	}


	int status = child->_update(dt);

	switch (mode) {
		case BT_RepeaterDecorator::until_succes:
			if (status != STATUS_SUCCESS) {
				//set to 0 here for safety
				godot::print_line("repeating until succes");
				repeat_count = 0;
				return STATUS_RUNNING;
			} else {
				godot::print_line("succesfuly succeeded");
				return STATUS_SUCCESS;
			}

			godot::print_error("repeater safery catch");
			return STATUS_RUNNING;
			break;
		case BT_RepeaterDecorator::repeat_X_times:
			if (repeat_count <= repeat_amount) {
				if (status == STATUS_SUCCESS) {
					repeat_count = 0;
					godot::print_line("succesfuly succeeded");
					return STATUS_SUCCESS;
				}
				godot::print_line("AGAIN");
				repeat_count++;
				return STATUS_RUNNING;
			} else {
				godot::print_line("set repeats reached. no succes");
				repeat_count = 0;
				return STATUS_FAILURE;
			}
			break;
		default:
			break;
	}

	godot::print_error("repeater safery catch");
	return STATUS_FAILURE;
}

//invert decorator can only have 1 childe



void BT_RepeaterDecorator::set_mode(repeat_mode p_mode) {
	mode = p_mode;
}

BT_RepeaterDecorator::repeat_mode BT_RepeaterDecorator::get_mode() {
	return mode;
}

void BT_RepeaterDecorator::set_repeat_count(int count) {
	repeat_amount = count;
}

int BT_RepeaterDecorator::get_repeat_count() {
	return repeat_amount;
}

int BT_WaitNode::_update(float dt) {
	if (child_nodes.is_empty()) {
		godot::print_error("wait node has no children");
		return STATUS_FAILURE;
	}
	//le funny variant cast for godot
	Variant v = child_nodes[0];
	BT_Node *child = Object::cast_to<BT_Node>(v.operator Object *());
	if (!child) {
		godot::print_error("Null or invalid child node");
		return STATUS_FAILURE;
	}

	time += dt;
	godot::print_error(time);

	if (time <= duration) {
		godot::print_error("Running");
		return STATUS_RUNNING;
	} else {
		time = 0;
		switch (state) {
			case BT_WaitNode::return_succes:
				godot::print_error("succes");
				return STATUS_SUCCESS;
				break;
			case BT_WaitNode::return_failed:
				return STATUS_FAILURE;
				break;
			default:
				break;
		}
	}

}

void BT_WaitNode::set_end_state(end_state end_state) {
	state = end_state;
}

BT_WaitNode::end_state BT_WaitNode::get_end_state() {
	return state;
}

int BT_ParralelNode::_update(float dt) {
	if (child_nodes.is_empty()) {
		godot::print_error("Selector has no children");
		return STATUS_FAILURE;
	}


	    if (child_states.size() != child_nodes.size()) {
		child_states.resize(child_nodes.size());
		for (int i = 0; i < child_states.size(); i++) {
			child_states.set(i,STATUS_RUNNING);
		}
	}

	int succes_count = 0;

	for (size_t i = 0; i < child_nodes.size(); i++) {
		//le funny variant cast for godot
		Variant v = child_nodes[i];
		BT_Node *child = Object::cast_to<BT_Node>(v.operator Object *());

		if (!child) {
			godot::print_error("Null or invalid child node");
			child_states.clear();
			return STATUS_FAILURE;
		}

		if (child_states[i] == STATUS_RUNNING)
		{
			child_states.set(i, static_cast<BT_Node::Status>(child->_update(dt)));
		}

		if (child_states[i] == STATUS_FAILURE) {
			child_states.clear();
			return STATUS_FAILURE; // ANY fail
		}

		if (child_states[i] == STATUS_SUCCESS)
		{
			succes_count++;
		}
	}

	if (succes_count == child_nodes.size()) {
		return STATUS_SUCCESS;
		child_states.clear();
	}
	return STATUS_RUNNING;
}

int BT_ReactiveSequenceNode::_update(float dt) {

	if (child_nodes.is_empty()) {
		return STATUS_FAILURE;
	}

	for (int i = 0; i < child_nodes.size(); i++) {
		BT_Node *child = Object::cast_to<BT_Node>(
				child_nodes[i].operator Object *());

		if (!child) {
			return STATUS_FAILURE;
		}

		int status = child->_update(dt);

		if (status != STATUS_SUCCESS) {
			return status; // FAILURE or RUNNING
		}
	}

	return STATUS_SUCCESS;

	// All children succeeded
	set_status(STATUS_SUCCESS);
	current_child = 0;
	return STATUS_SUCCESS;
}

int BT_ReactiveSelectorNode::_update(float dt) {
	if (child_nodes.is_empty()) {
		godot::print_error("ReactiveSelector has no children");
		return STATUS_FAILURE;
	}

	for (int i = 0; i < child_nodes.size(); i++) {
		Variant v = child_nodes[i];
		BT_Node *child = Object::cast_to<BT_Node>(v.operator Object *());

		if (!child) {
			godot::print_error("Null or invalid child node");
			return STATUS_FAILURE;
		}

		int status = child->_update(dt);

		if (status != STATUS_FAILURE) {
			return status; // SUCCESS or RUNNING
		}
	}

	return STATUS_FAILURE;
}
