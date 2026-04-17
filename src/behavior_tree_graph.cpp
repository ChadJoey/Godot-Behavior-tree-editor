
#include "../src/behavior_tree_graph.h"
#include "behavior_tree_plugin.h"
#include <cassert>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/error_macros.hpp>

void BT_Graph_Tree::rebuild_slots(float row_height) {
	this->clear_all_slots();
	add_output_slot(this, 0);
}

void BT_Graph_Node::add_output_slot(GraphNode *gn, int idx, float row_height) {
	Control *row = memnew(Control); // create a child Control (visual row)
	row->set_custom_minimum_size(Vector2(0, row_height)); // give it vertical space so slots are spread out
	gn->add_child(row); // add the Control to the GraphNode (creates the GUI row)
	//add to slot group so we can use it for rebuilding slots
	row->add_to_group("slot");
	// set_slot(index, enable_left, type_left, color_left, enable_right, type_right, color_right)
	gn->set_slot(idx,
			/*left*/ false, /*type_left*/ 0, Color(0, 0, 0, 0),
			/*right*/ true, /*type_right*/ 0, Color(0.2, 0.8, 0.2));
}

void BT_Graph_Node::add_default_slot(GraphNode *gn, int idx, float row_height) {
	Control *row = memnew(Control); // create a child Control (visual row)
	row->set_custom_minimum_size(Vector2(0, row_height)); // give it vertical space so slots are spread out
	gn->add_child(row); // add the Control to the GraphNode (creates the GUI row)
	row->add_to_group("slot");
	// set_slot(index, enable_left, type_left, color_left, enable_right, type_right, color_right)
	gn->set_slot(idx,
			/*left*/ true, /*type_left*/ 0, Color(0.2, 0.8, 0.2),
			/*right*/ true, /*type_right*/ 0, Color(0.2, 0.8, 0.2));
}

void BT_Graph_Node::add_input_slot(GraphNode *gn, int idx, float row_height) {
	Control *row = memnew(Control); // create a child Control (visual row)
	row->set_custom_minimum_size(Vector2(0, row_height)); // give it vertical space so slots are spread out
	gn->add_child(row); // add the Control to the GraphNode (creates the GUI row)
	row->add_to_group("slot");
	// set_slot(index, enable_left, type_left, color_left, enable_right, type_right, color_right)
	gn->set_slot(idx,
			/*left*/ true, /*type_left*/ 0, Color(0.2, 0.8, 0.2),
			/*right*/ false, /*type_right*/ 0, Color(0, 0, 0));
}

void BT_Graph_Node::rebuild_slots(float row_height) {
	add_input_slot(this, 0);
	for (size_t i = 0; i < this->children.size(); i++) {
		add_output_slot(this, i);
	}
	this->reset_size();
}


void BT_Graph_RepeaterDecorator::set_mode(repeat_mode p_mode) {
	mode = p_mode;
}

BT_Graph_RepeaterDecorator::repeat_mode BT_Graph_RepeaterDecorator::get_mode() {
	return mode;
}

void BT_Graph_RepeaterDecorator::set_repeat_count(int count) {
	repeat_amount = count;
}

int BT_Graph_RepeaterDecorator::get_repeat_count() {
	return repeat_amount;
}




void BT_Tree_Resource::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::OBJECT,"root_node", PROPERTY_HINT_RESOURCE_TYPE, "BT_node_data"));
	p_list->push_back(PropertyInfo(Variant::ARRAY, "all_nodes", PROPERTY_HINT_ARRAY_TYPE, "Resource/BT_node_data"));
}

bool BT_Tree_Resource::_set(const StringName &p_name, const Variant &p_value) {
	if (p_name == StringName("all_nodes")) {
		all_nodes = p_value;
		return true;
	}
	if (p_name == StringName("root_node")) {
		root_node = p_value;
		return true;
	}
	return false;
}

bool BT_Tree_Resource::_get(const StringName &p_name, Variant &r_ret) const {
	if (p_name == StringName("all_nodes")) {
		r_ret = all_nodes;
		return true;
	}
	if (p_name == StringName("root_node")) {
		r_ret = root_node;
		return true;
	}
	return false;
}
