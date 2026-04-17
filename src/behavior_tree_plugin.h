#pragma once

#include "behavior_tree_graph.h"
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/editor_file_dialog.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/file_system_dock.hpp>
#include <godot_cpp/classes/graph_Node.hpp>
#include <godot_cpp/classes/graph_edit.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/menu_button.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/popup_panel.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/v_box_container.hpp>

class BT_GraphEdit : public EditorPlugin {
	GDCLASS(BT_GraphEdit, EditorPlugin);

public:
	BT_GraphEdit() {};
	~BT_GraphEdit() {};

	void _enter_tree() override;

	void _on_disconnection_request(const String &from, int from_slot, const String &to, int to_slot);

	void _rebuild_visual_connection(const godot::String &from, int from_slot, const godot::String &to, int to_slot);

	void _on_connection_request(const godot::String &from, int from_slot, const godot::String &to, int to_slot);

	void on_node_menu_selected(int id);

	template <class T>
	T *add_new_node() {
		static_assert(std::is_base_of<BT_Graph_Node, T>::value, "T must inherit from BT_Graph_Node");

		T *bt_node = memnew(T);

		// Unique name
		godot::String space = "_";
		bt_node->set_title(T::get_class_static() + space + itos(node_id_counter));
		bt_node->set_name(T::get_class_static() + space + itos(node_id_counter));

		// Assign a unique ID
		bt_node->set_meta("bt_id", node_id_counter++);

		selectedTree->unconnected_nodes.push_back(bt_node);
		bt_node->set_owner(selectedTree->get_owner());
		bt_node->set_size(Vector2(100, 100));
		graph_edit->add_child(bt_node);

		return bt_node;
	}

	Ref<BT_Tree_Resource> BT_GraphEdit::build_tree_from_graph() {
		Ref<BT_Tree_Resource> tree;
		tree.instantiate();

		Array all_data_nodes;
		Array nodes = traverse(selectedTree);

		for (int i = 0; i < nodes.size(); i++) {
			BT_Graph_Node *node = Object::cast_to<BT_Graph_Node>(nodes[i]);
			if (!node)
				continue;
			//store tree nodes
			Ref<BT_node_data> data;
			data.instantiate();
			data->set_id(node->get_meta("bt_id"));
			data->set_name(node->get_name());
			data->set_node_type(node->get_class());
			data->set_editor_position(node->get_position_offset());
			data->set_max_children(node->_get_max_children());
			data->set_title(node->get_title());
			data->set_script(node->get_script());

			BT_Graph_WaitNode *wait_node = Object::cast_to<BT_Graph_WaitNode>(nodes[i]);
			if (wait_node)
			{
				data->set_duration(wait_node->get_duration());
			}


			//holds ids to childed nodes
			Array children_ids;
			for (int j = 0; j < node->children.size(); j++) {
				BT_Graph_Node *child = node->children[j];
				children_ids.append(child->get_meta("bt_id"));
			}
			data->set_children_ids(children_ids);
			
			all_data_nodes.append(data);
		}
		tree->set_all_nodes(all_data_nodes);
		tree->set_node_id_count(node_id_counter);
		return tree;
	}


	void BT_GraphEdit::_on_graph_node_selected(const Variant &node_var) {
		Node *node = Object::cast_to<Node>(node_var);
		BT_Graph_Node *gn = Object::cast_to<BT_Graph_Node>(node);
		if (!gn)
			return;

		EditorSelection *selection = get_editor_interface()->get_selection();
		selection->clear();
		selection->add_node(gn);
	}

	void BT_GraphEdit::_on_file_menu_selected(int id) {
		switch (id) {
			case 0: { // New Tree
				node_id_counter = 0;
				_clear_graph();

				BT_Graph_Tree *root_node = memnew(BT_Graph_Tree);
				selectedTree = root_node;
				root_node->set_meta("bt_id", node_id_counter++);
				root_node->set_title("root");
				root_node->set_position_offset(Vector2(200, 200));
				root_node->set_name("tree");
				root_node->set_size(Vector2(100, 100));
				graph_edit->add_child(root_node);

				godot::print_line("Created new behavior tree.");
			} break;

			case 1: { // Save Tree
				save_dialog->popup_centered_ratio(0.6);
			} break;

			case 2: { // Load Tree
				load_dialog->popup_centered_ratio(0.6);
			} break;
		}
	}

	void BT_GraphEdit::_on_save_path_selected(const String &path) {
		// Rebuild resource from current graph
		selectedTreeResource = build_tree_from_graph();

		if (!selectedTreeResource.is_valid())
			return;

		// Make sure directory exists
		String dir = "res://behavior_trees/";
		String file_name = path.get_file();
		String full_path = String(dir + file_name); // safer than string concatenation

		if (DirAccess::make_dir_recursive_absolute(dir) != OK) {
			godot::print_error("Failed to create directory: " + dir);
			return;
		}

		Ref<DirAccess> da = DirAccess::open(dir);
		if (!da.is_valid()) {
			godot::print_error("Failed to open directory: " + dir);
			return;
		}

		// SAVE using full_path
		Error err = ResourceSaver::get_singleton()->save(selectedTreeResource, full_path, ResourceSaver::FLAG_NONE);
		if (err == OK)
			godot::print_line("Tree saved to: " + full_path);
		else {
			godot::print_error("Failed to save tree to: " + full_path);
			godot::print_error(err);
		}
	}

	void BT_GraphEdit::_on_load_path_selected(const String &path) {
		Ref<BT_Tree_Resource> loaded = ResourceLoader::get_singleton()->load(path);
		if (!loaded.is_valid()) {
			godot::print_error("Failed to load tree: " + path);
			return;
		}

		selectedTreeResource = loaded;
		_clear_graph();
		_populate_from_tree(loaded);

		godot::print_line("Loaded Behavior Tree resource: " + path);
	}

	void _exit_tree() override;

	Array traverse(BT_Graph_Node *root);
	void BT_GraphEdit::_populate_from_tree(Ref<BT_Tree_Resource> tree);

	void _clear_graph();

protected:
	static void _bind_methods() {
	}

private:
	Control *dock = nullptr;
	GraphEdit *graph_edit = nullptr;
	BT_Graph_Tree *selectedTree = nullptr;
	Ref<BT_Tree_Resource> selectedTreeResource;
	EditorFileDialog *save_dialog = nullptr;
	EditorFileDialog *load_dialog = nullptr;

	int node_id_counter = 0;
};
