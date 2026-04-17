#include "behavior_tree_plugin.h"

void BT_GraphEdit::_enter_tree() {
	godot::print_line("initializing plugin");

	save_dialog = memnew(EditorFileDialog);
	save_dialog->set_file_mode(EditorFileDialog::FILE_MODE_SAVE_FILE);
	save_dialog->set_current_path("res://behavior_trees/");
	save_dialog->add_filter("*.tres; Behavior Tree Resource");
	save_dialog->connect("file_selected", callable_mp(this, &BT_GraphEdit::_on_save_path_selected));
	add_child(save_dialog);

	load_dialog = memnew(EditorFileDialog);
	load_dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_FILE);
	load_dialog->add_filter("*.tres; Behavior Tree Resource");
	load_dialog->connect("file_selected", callable_mp(this, &BT_GraphEdit::_on_load_path_selected));
	add_child(load_dialog);

	graph_edit = memnew(GraphEdit);
	graph_edit->set_name("behavior tree");
	add_control_to_bottom_panel(graph_edit, "BT graph dock");

	HBoxContainer *menu = graph_edit->get_menu_hbox();

	MenuButton *node_menu = memnew(MenuButton);
	node_menu->set_text("add node");

	PopupMenu *popup = node_menu->get_popup();
	popup->add_item("Condition_node", 0);
	popup->add_item("Action_node", 1);
	popup->add_item("selector_node", 2);
	popup->add_item("sequence_node", 3);
	popup->add_item("invert_decorator", 4);
	popup->add_item("repeat_decorator", 5);
	popup->add_item("Wait Node", 6);
	popup->add_item("Parralel node", 7);
	popup->add_item("ReactiveSequenceNode", 8);
	popup->add_item("ReactiveSelectorNode ", 9);

	popup->connect("id_pressed", callable_mp(this, &BT_GraphEdit::on_node_menu_selected));
	menu->add_child(node_menu);

	MenuButton *file_menu = memnew(MenuButton);
	file_menu->set_text("File");
	PopupMenu *file_popup = file_menu->get_popup();
	file_popup->add_item("New Tree", 0);
	file_popup->add_item("Save Tree", 1);
	file_popup->add_item("Load Tree", 2);
	file_popup->connect("id_pressed", callable_mp(this, &BT_GraphEdit::_on_file_menu_selected));
	graph_edit->get_menu_hbox()->add_child(file_menu);

	graph_edit->set_right_disconnects(true);
	graph_edit->connect("connection_request",
			callable_mp(this, &BT_GraphEdit::_on_connection_request));
	graph_edit->connect("disconnection_request",
			callable_mp(this, &BT_GraphEdit::_on_disconnection_request));
	graph_edit->connect("node_selected", callable_mp(this, &BT_GraphEdit::_on_graph_node_selected));
}

void BT_GraphEdit::_on_disconnection_request(const String &from, int from_slot, const String &to, int to_slot) {
	graph_edit->disconnect_node(from, from_slot, to, to_slot);

	BT_Graph_Node *from_node = Object::cast_to<BT_Graph_Node>(graph_edit->get_node_or_null(from));
	BT_Graph_Node *to_node = Object::cast_to<BT_Graph_Node>(graph_edit->get_node_or_null(to));

	if (!from_node || !to_node)
		return;

	auto &children = from_node->children;
	children.erase(to_node);

	from_node->children;
	to_node->parent = nullptr;
	selectedTree->unconnected_nodes.push_back(to_node);
	godot::print_line("disconnected: " + from + " -> " + to);
}

void BT_GraphEdit::_rebuild_visual_connection(const godot::String &from, int from_slot, const godot::String &to, int to_slot) {
	BT_Graph_Node *from_node = Object::cast_to<BT_Graph_Node>(graph_edit->get_node_or_null(from));
	BT_Graph_Node *to_node = Object::cast_to<BT_Graph_Node>(graph_edit->get_node_or_null(to));

	graph_edit->connect_node(from, from_slot, to, to_slot);
}

void BT_GraphEdit::_on_connection_request(const godot::String &from, int from_slot, const godot::String &to, int to_slot) {
	BT_Graph_Node *from_node = Object::cast_to<BT_Graph_Node>(graph_edit->get_node_or_null(from));
	BT_Graph_Node *to_node = Object::cast_to<BT_Graph_Node>(graph_edit->get_node_or_null(to));

	//already has a parent
	if (to_node->parent) {
		godot::print_error("node already has a parent");
		return;
	}

	// loop through children of from_node to see if any already occupies from_slot
	for (int i = 0; i < from_node->children.size(); i++) {
		BT_Graph_Node *child = from_node->children[i];

		//use GraphEdit to get the port number for this child
		//loop through all connections to find the slot used by this child
		Array connections = graph_edit->get_connection_list();
		for (int j = 0; j < connections.size(); j++) {
			Dictionary conn = connections[j];
			if (conn["from_node"] == from_node->get_name() && conn["to_node"] == child->get_name()) {
				int occupied_slot = conn["from_port"];
				if (occupied_slot == from_slot) {
					godot::print_error("Slot already in use by another child");
					return;
				}
			}
		}
	}

	if (!from_node || !to_node) {
		godot::print_error("Invalid node(s) for connection: " + from + " -> " + to);
		return;
	}

	if (!from_node->_can_add_child()) {
		godot::print_error("Connection denied: parent cannot accept more children.");
		return;
	}

	//remove connected node from the unconnected list
	selectedTree->unconnected_nodes.erase(to_node);
	graph_edit->connect_node(from, from_slot, to, to_slot);
	godot::print_line("connected: " + from + " -> " + to);
	from_node->children.push_back(to_node);
	to_node->parent = from_node;
}

void BT_GraphEdit::on_node_menu_selected(int id) {
	switch (id) {
		case 0:
			add_new_node<BT_Graph_ConditionNode>();
			break;
		case 1:
			add_new_node<BT_Graph_ActionNode>();
			break;
		case 2:
			add_new_node<BT_Graph_SelectorNode>();
			break;
		case 3:
			add_new_node<BT_Graph_SequenceNode>();
			break;
		case 4:
			add_new_node<BT_Graph_InvertDecorator>();
			break;
		case 5:
			add_new_node<BT_Graph_RepeaterDecorator>();
			break;
		case 6:
			add_new_node<BT_Graph_WaitNode>();
			break;
		case 7:
			add_new_node<BT_Graph_ParralelNode>();
			break;
		case 8:
			add_new_node<BT_Graph_ReactiveSequenceNode>();
			break;
		case 9:
			add_new_node<BT_Graph_ReactiveSelectorNode>();
			break;
		default:
			break;
	}
}

void BT_GraphEdit::_exit_tree() {
	remove_control_from_docks(graph_edit);
	graph_edit->queue_free();
	graph_edit = nullptr;
}

Array BT_GraphEdit::traverse(BT_Graph_Node *root) {
	godot::List<BT_Graph_Node *> q;
	Array all_nodes_in_tree;

	q.push_back(root);
	while (!q.is_empty()) {
		BT_Graph_Node *current = q.front()->get();
		q.pop_front();

		all_nodes_in_tree.push_back(current);

		for (auto &child : current->children) {
			q.push_back(child);
		}
	}

	auto &unconnected_nodes = selectedTree->unconnected_nodes;
	for (size_t i = 0; i < unconnected_nodes.size(); i++) {
		all_nodes_in_tree.push_back(unconnected_nodes[i]);
	}

	return all_nodes_in_tree;
}

void BT_GraphEdit::_populate_from_tree(Ref<BT_Tree_Resource> tree) {
	if (!tree.is_valid()) {
		godot::print_error("Invalid tree resource.");
		return;
	}

	_clear_graph();

	// Map node IDs → BT_Graph_Node pointers
	HashMap<int, BT_Graph_Node *> id_to_node;

	// 1. Create GraphNodes for each node_data
	Array nodes = tree->get_all_nodes();

	for (int i = 0; i < nodes.size(); i++) {
		Ref<BT_node_data> data = nodes[i];
		if (!data.is_valid())
			continue;

		String type = data->get_node_type();
		BT_Graph_Node *node = nullptr;

		// Spawn the right type dynamically
		if (type == "BT_Graph_SelectorNode")
			node = memnew(BT_Graph_SelectorNode);
		else if (type == "BT_Graph_ReactiveSelectorNode")
			node = memnew(BT_Graph_ReactiveSelectorNode);
		else if (type == "BT_Graph_SequenceNode")
			node = memnew(BT_Graph_SequenceNode);
		else if (type == "BT_Graph_ReactiveSequenceNode")
			node = memnew(BT_Graph_ReactiveSequenceNode);
		else if (type == "BT_Graph_ParralelNode")
			node = memnew(BT_Graph_ParralelNode);
		else if (type == "BT_Graph_WaitNode") {
			node = memnew(BT_Graph_WaitNode);
			BT_Graph_WaitNode *wait_node = Object::cast_to<BT_Graph_WaitNode>(node);
			if (wait_node)
			{
				wait_node->set_duration(data->get_duration());
			};
		}
		else if (type == "BT_Graph_ActionNode")
			node = memnew(BT_Graph_ActionNode);
		else if (type == "BT_Graph_ConditionNode")
			node = memnew(BT_Graph_ConditionNode);
		else if (type == "BT_Graph_InvertDecorator")
			node = memnew(BT_Graph_InvertDecorator);
		else if (type == "BT_Graph_RepeaterDecorator")
			node = memnew(BT_Graph_RepeaterDecorator);
		else if (type == "BT_Graph_Tree") {
			node = memnew(BT_Graph_Tree);
			selectedTree = Object::cast_to<BT_Graph_Tree>(node);
		}
		else {
			godot::print_error("Unknown node type: " + type);
			continue;
		}

		node->set_name(data->get_name());
		node->set_title(data->get_title());
		node->set_meta("bt_id", data->get_id());
		node->set_position_offset(data->get_editor_position());
		node->set_size(Vector2(100, 100));
		node->set_script(data->get_script());

		graph_edit->add_child(node);
		id_to_node[data->get_id()] = node;
	}

	// 2. Connect nodes based on children_ids
	for (int i = 0; i < nodes.size(); i++) {
		Ref<BT_node_data> data = nodes[i];
		if (!data.is_valid())
			continue;

		BT_Graph_Node *parent_node = id_to_node[data->get_id()];
		if (!parent_node)
			continue;

		Array children_ids = data->get_children_ids();
		for (int j = 0; j < children_ids.size(); j++) {
			int child_id = children_ids[j];
			if (!id_to_node.has(child_id))
				continue;

			BT_Graph_Node *child_node = id_to_node[child_id];
			graph_edit->connect_node(parent_node->get_name(), j, child_node->get_name(), 0);
			parent_node->children.push_back(child_node);
			child_node->parent = parent_node;
		}
		id_to_node[data->get_id()]->rebuild_slots();
		
	}

	node_id_counter = tree->get_node_id_count();

	godot::print_line("Graph repopulated from tree resource.");
}

void BT_GraphEdit::_clear_graph() {
	for (int i = graph_edit->get_child_count() - 1; i >= 0; i--) {
		GraphNode *gn = Object::cast_to<GraphNode>(graph_edit->get_child(i));
		if (gn) {
			graph_edit->remove_child(gn);
		}
	}
	graph_edit->clear_connections();
}
