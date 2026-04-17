#pragma once

#include <functional>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/graph_Node.hpp>
#include <godot_cpp/classes/graph_edit.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/menu_button.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/popup_panel.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <queue>

class BT_Graph_ActionNode;
class BT_Graph_ConditionNode;

using namespace godot;

/// <summary>
/// base Node
/// meant to be inhereted by nodes with actual functionality
/// </summary>
class BT_Graph_Node : public GraphNode {
	GDCLASS(BT_Graph_Node, GraphNode);

public:
	enum Status {
		STATUS_RUNNING,
		STATUS_SUCCESS,
		STATUS_FAILURE
	};
	Status status = STATUS_FAILURE;

	Script *script = nullptr;
	BT_Graph_Node *parent = nullptr;
	Vector<BT_Graph_Node *> children;
	godot::String name;
	godot::Vector2 editor_position;

	int max_children = 1; //-1 = unlimited
	int max_parents = 1;

	void set_status(Status s);
	Status get_status() const;

	// Add one visual slot row and enable a right (output) port on it
	void add_output_slot(GraphNode *gn, int idx, float row_height = 24.0f);

	void add_default_slot(GraphNode *gn, int idx, float row_height = 24.0f);

	void add_input_slot(GraphNode *gn, int idx, float row_height = 24.0f);

	bool _can_add_child() const {
		return max_children < 0 || (int)children.size() < max_children;
	}
	const int _get_max_children() {
		return max_children;
	}

	Vector<BT_Graph_Node *> _get_children() {
		return children;
	}
	BT_Graph_Node *_get_parent_node() {
		return parent;
	}

	virtual void rebuild_slots(float row_height = 24.0f);
	BT_Graph_Node() {
		add_input_slot(this, 0);
	}

	String get_node_title() const { return name; }

	void set_node_title(const String &t) {
		name = t;
		set_title(name);
	}

protected:
	float timer = 0;
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("get_node_title"), &BT_Graph_Node::get_node_title);
		ClassDB::bind_method(D_METHOD("set_node_title", "title"), &BT_Graph_Node::set_node_title);

		ADD_PROPERTY(PropertyInfo(Variant::STRING, "node_title"), "set_node_title", "get_node_title");
	}
};
VARIANT_ENUM_CAST(BT_Graph_Node::Status);

class BT_node_data : public Resource {
	GDCLASS(BT_node_data, Resource);

private:
	String node_type;
	String name;
	String title;
	String end_state;
	Vector2 editor_position;
	int id = -1;
	int max_children = 1;
	Array children_ids;
	Ref<Script> script;
	float duration = 0;

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_id", "id"), &BT_node_data::set_id);
		ClassDB::bind_method(D_METHOD("get_id"), &BT_node_data::get_id);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");

		ClassDB::bind_method(D_METHOD("set_node_type", "type"), &BT_node_data::set_node_type);
		ClassDB::bind_method(D_METHOD("get_node_type"), &BT_node_data::get_node_type);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "node_type"), "set_node_type", "get_node_type");

		ClassDB::bind_method(D_METHOD("set_name", "name"), &BT_node_data::set_name);
		ClassDB::bind_method(D_METHOD("get_name"), &BT_node_data::get_name);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");

		ClassDB::bind_method(D_METHOD("set_end_state", "end_state"), &BT_node_data::set_end_state);
		ClassDB::bind_method(D_METHOD("get_end_state"), &BT_node_data::get_end_state);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "state"), "set_end_state", "bet_end_state");


		ClassDB::bind_method(D_METHOD("set_title", "title"), &BT_node_data::set_title);
		ClassDB::bind_method(D_METHOD("get_title"), &BT_node_data::get_title);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");

		ClassDB::bind_method(D_METHOD("set_editor_position", "pos"), &BT_node_data::set_editor_position);
		ClassDB::bind_method(D_METHOD("get_editor_position"), &BT_node_data::get_editor_position);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "editor_position"), "set_editor_position", "get_editor_position");

		ClassDB::bind_method(D_METHOD("set_max_children", "count"), &BT_node_data::set_max_children);
		ClassDB::bind_method(D_METHOD("get_max_children"), &BT_node_data::get_max_children);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "max_children"), "set_max_children", "get_max_children");

				ClassDB::bind_method(D_METHOD("set_duration", "d"), &BT_node_data::set_duration);
		ClassDB::bind_method(D_METHOD("get_duration"), &BT_node_data::get_duration);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "d"), "set_duration", "get_duration");

		ClassDB::bind_method(D_METHOD("set_children_ids", "ids"), &BT_node_data::set_children_ids);
		ClassDB::bind_method(D_METHOD("get_children_ids"), &BT_node_data::get_children_ids);
		ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "children_ids"), "set_children_ids", "get_children_ids");

		ClassDB::bind_method(D_METHOD("set_script", "script"), &BT_node_data::set_script);
		ClassDB::bind_method(D_METHOD("get_script"), &BT_node_data::get_script);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "script", PROPERTY_HINT_RESOURCE_TYPE, "Script"), "set_script", "get_script");
	}

public:
	void set_id(int p_id) { id = p_id; }
	int get_id() const { return id; }

	void set_node_type(const String &t) { node_type = t; }
	String get_node_type() const { return node_type; }

	void set_name(const String &n) { name = n; }
	String get_name() const { return name; }

	void set_end_state(const String &e) { end_state = e; }
	String get_end_state() const { return end_state; }

	void set_title(const String &t) { title = t; }
	String get_title() const { return title; }

	void set_editor_position(const Vector2 &p) { editor_position = p; }
	Vector2 get_editor_position() const { return editor_position; }

	void set_max_children(int c) { max_children = c; }
	int get_max_children() const { return max_children; }

	void set_duration(float d) { duration = d; }
	float get_duration() const { return duration; }

	void set_children_ids(const TypedArray<int> &ids) { children_ids = ids; }
	TypedArray<int> get_children_ids() const { return children_ids; }

	void set_script(const Ref<Script> s) { script = s; }
	Ref<Script> get_script() const { return script; }


	void add_child_id(int child_id) {
		if (!children_ids.has(child_id)) {
			children_ids.append(child_id);
		}
	}
};

class BT_Tree_Resource : public Resource {
	GDCLASS(BT_Tree_Resource, Resource);

private:
	Ref<BT_node_data> root_node;
	TypedArray<BT_node_data> all_nodes;
	int index = 0;

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_root_node", "node"), &BT_Tree_Resource::set_root_node);
		ClassDB::bind_method(D_METHOD("get_root_node"), &BT_Tree_Resource::get_root_node);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root_node", PROPERTY_HINT_RESOURCE_TYPE, "BT_node_data"),
				"set_root_node", "get_root_node");


		ClassDB::bind_method(D_METHOD("get_all_nodes"), &BT_Tree_Resource::get_all_nodes);
		ClassDB::bind_method(D_METHOD("set_all_nodes", "nodes"), &BT_Tree_Resource::set_all_nodes);
		ClassDB::bind_method(D_METHOD("clear_nodes"), &BT_Tree_Resource::clear_nodes);

		ClassDB::bind_method(D_METHOD("set_node_id_count"), &BT_Tree_Resource::set_node_id_count);
		ClassDB::bind_method(D_METHOD("get_node_id_count"), &BT_Tree_Resource::get_node_id_count);

		ADD_PROPERTY(PropertyInfo(Variant::INT, "node_id_count"), "set_node_id_count", "get_node_id_count");

		ClassDB::bind_method(D_METHOD("get_node_by_id", "id"), &BT_Tree_Resource::get_node_by_id);
	}

public:

	void set_node_id_count(const int id) { index = id; }
	int get_node_id_count() const { return index; }
	void set_root_node(const Ref<BT_node_data> &node) { root_node = node;}
	Ref<BT_node_data> get_root_node() const { return root_node; }


	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;



	TypedArray<BT_node_data> get_all_nodes() const { return all_nodes; }
	void set_all_nodes(const TypedArray<BT_node_data> &nodes) { all_nodes = nodes; }

	void clear_nodes() {
		all_nodes.clear();
		root_node.unref();
	}

	Ref<BT_node_data> get_node_by_id(int id) const {
		for (int i = 0; i < all_nodes.size(); i++) {
			Ref<BT_node_data> node = all_nodes[i];
			if (node.is_valid() && node->get_id() == id)
				return node;
		}
		return nullptr;
	}
};

class BT_Graph_Tree : public BT_Graph_Node {
	GDCLASS(BT_Graph_Tree, BT_Graph_Node);

public:
	BT_Graph_Tree() {
		add_output_slot(this, 0);
	}
	float TickRate = 60;
	void _ready() override {
		set_title(godot::String("root_tree"));
		set_size(godot::Vector2(10, 10));
	}
	void rebuild_slots(float row_height = 24.0f) override;
	Vector<BT_Graph_Node *> unconnected_nodes;

protected:
	static void _bind_methods() {
	}
};
/// <summary>
/// returns true or false
/// checks facts should not change the world
/// example use:
/// Is_player_visible()? -> returns true if player is visible, returns false if not
/// </summary>
class BT_Graph_ConditionNode : public BT_Graph_Node {
	GDCLASS(BT_Graph_ConditionNode, BT_Graph_Node);

public:
protected:
	static void _bind_methods() {
	}
};
/// <summary>
/// has no children
/// preforms an action in the game world
/// example use:
/// BT_ActionNode MoveTo(target) -> returns Running until the NPC reaches the target, then SUCCES
/// </summary>
class BT_Graph_ActionNode : public BT_Graph_Node {
	GDCLASS(BT_Graph_ActionNode, BT_Graph_Node);

public:

protected:
	static void _bind_methods() {
	}
};



class BT_Graph_WaitNode : public BT_Graph_Node {
	GDCLASS(BT_Graph_WaitNode, BT_Graph_Node);

public:
	BT_Graph_WaitNode() { add_default_slot(this, 0); };
	enum end_state { return_succes = 0,
	return_failed = 1 } state;



	float get_duration() { return duration; };
	void set_duration(float time) { duration = time; };

	void set_end_state(end_state p_mode) { state = p_mode; };
	end_state get_end_state() { return state; };

	private:
	float duration;



protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_end_state", "state"),
				&BT_Graph_WaitNode::set_end_state);
		ClassDB::bind_method(D_METHOD("get_end_state"),
				&BT_Graph_WaitNode::get_end_state);

		ADD_PROPERTY(PropertyInfo(Variant::INT, "end_state",
							 PROPERTY_HINT_ENUM, "Success,Failed"),
				"set_end_state", "get_end_state");

		ClassDB::bind_method(D_METHOD("set_duration", "duration"),
				&BT_Graph_WaitNode::set_duration);
		ClassDB::bind_method(D_METHOD("get_duration"),
				&BT_Graph_WaitNode::get_duration);

		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"),
				"set_duration", "get_duration");
	}

};
VARIANT_ENUM_CAST(BT_Graph_WaitNode::end_state);

/// <summary>
/// fails if 1 child fails
/// only succeeds if all children suceed
/// used for multiple steps in sequence
/// example use:
///
/// BT_ConditionNode: Is enemy visible?
/// BT_ActionNode: Move to enemy.
/// BT_ActionNode: Attack enemy.
///
/// If BT_ConditionNode fails → stop immediately (FAILURE).
/// If moving is still happening → return RUNNING.
/// If all succeed → return SUCCESS.
/// </summary>

class BT_Graph_SequenceNode : public BT_Graph_Node {
	GDCLASS(BT_Graph_SequenceNode, BT_Graph_Node);

public:
	BT_Graph_SequenceNode() {
		add_default_slot(this, 0);
		max_children = -1;
		Button *add_button = memnew(Button);
		add_button->set_text("+");
		add_button->connect("pressed", callable_mp(this, &BT_Graph_SequenceNode::add_slot));
		this->add_child(add_button);

		Button *remove_button = memnew(Button);
		remove_button->set_text("-");
		remove_button->connect("pressed", callable_mp(this, &BT_Graph_SequenceNode::remove_slot));
		this->add_child(remove_button);
	}

	void rebuild_slots(float row_height = 24.0f) override {
		this->clear_all_slots();

		add_default_slot(this, 0);
		for (size_t i = 1; i < this->children.size(); i++) {
			add_output_slot(this, i);
		}
		this->reset_size();
	}

	void remove_slot() {
		//remove last inx slot
		int slots = this->get_output_port_count();
		if (slots <= 0) {
			return;
		}

		for (int i = get_child_count() - 1; i >= 0; --i) {
			if (Control *c = Object::cast_to<Control>(get_child(i))) {
				if (c->is_in_group("slot")) { // only remove the ones representing slots
					remove_child(c);
					c->queue_free();
					break;
				}
			}
		}
		this->reset_size();
	}

	void add_slot() {
		Control *row = memnew(Control); // create a child Control (visual row)
		row->set_custom_minimum_size(Vector2(0, 24.f)); // give it vertical space so slots are spread out
		row->add_to_group("slot");
		this->add_child(row); // add the Control to the GraphNode (creates the GUI row)

		this->set_slot(this->get_output_port_count(),
				/*left*/ false, /*type_left*/ 0, Color(0, 0, 0, 0),
				/*right*/ true, /*type_right*/ 0, Color(0.2, 0.8, 0.2));
	}

	int current_child = 0;

protected:
	static void _bind_methods() {
	}

private:
};

class BT_Graph_ReactiveSequenceNode : public BT_Graph_Node {
	GDCLASS(BT_Graph_ReactiveSequenceNode, BT_Graph_Node);

public:
	BT_Graph_ReactiveSequenceNode() {
		add_default_slot(this, 0);
		max_children = -1;
		Button *add_button = memnew(Button);
		add_button->set_text("+");
		add_button->connect("pressed", callable_mp(this, &BT_Graph_ReactiveSequenceNode::add_slot));
		this->add_child(add_button);

		Button *remove_button = memnew(Button);
		remove_button->set_text("-");
		remove_button->connect("pressed", callable_mp(this, &BT_Graph_ReactiveSequenceNode::remove_slot));
		this->add_child(remove_button);
	}

	void rebuild_slots(float row_height = 24.0f) override {
		this->clear_all_slots();

		add_default_slot(this, 0);
		for (size_t i = 1; i < this->children.size(); i++) {
			add_output_slot(this, i);
		}
		this->reset_size();
	}

	void remove_slot() {
		//remove last inx slot
		int slots = this->get_output_port_count();
		if (slots <= 0) {
			return;
		}

		for (int i = get_child_count() - 1; i >= 0; --i) {
			if (Control *c = Object::cast_to<Control>(get_child(i))) {
				if (c->is_in_group("slot")) { // only remove the ones representing slots
					remove_child(c);
					c->queue_free();
					break;
				}
			}
		}
		this->reset_size();
	}

	void add_slot() {
		Control *row = memnew(Control); // create a child Control (visual row)
		row->set_custom_minimum_size(Vector2(0, 24.f)); // give it vertical space so slots are spread out
		row->add_to_group("slot");
		this->add_child(row); // add the Control to the GraphNode (creates the GUI row)

		this->set_slot(this->get_output_port_count(),
				/*left*/ false, /*type_left*/ 0, Color(0, 0, 0, 0),
				/*right*/ true, /*type_right*/ 0, Color(0.2, 0.8, 0.2));
	}

	int current_child = 0;

protected:
	static void _bind_methods() {
	}

private:
};


/// <summary>
/// fails only if all children fail
/// often used as an OR
/// example use: you want an agent to patrol OR chase the player
/// </summary>
class BT_Graph_SelectorNode : public BT_Graph_Node {
	GDCLASS(BT_Graph_SelectorNode, BT_Graph_Node);

public:
	BT_Graph_SelectorNode() {
		add_default_slot(this, 0);
		max_children = -1;
		Button *add_button = memnew(Button);
		add_button->set_text("+");
		add_button->connect("pressed", callable_mp(this, &BT_Graph_SelectorNode::add_slot));
		this->add_child(add_button);

		Button *remove_button = memnew(Button);
		remove_button->set_text("-");
		remove_button->connect("pressed", callable_mp(this, &BT_Graph_SelectorNode::remove_slot));
		this->add_child(remove_button);
	}
	Array child_nodes; // Array of BT_Node*
	int current_child = 0;

	void rebuild_slots(float row_height = 24.0f) override {
		this->clear_all_slots();

		add_default_slot(this, 0);
		for (size_t i = 1; i < this->children.size(); i++) {
			add_output_slot(this, i);
		}
		this->reset_size();
	}

	void remove_slot() {
		//remove last inx slot
		int slots = this->get_output_port_count();
		if (slots <= 0) {
			return;
		}

		for (int i = get_child_count() - 1; i >= 0; --i) {
			if (Control *c = Object::cast_to<Control>(get_child(i))) {
				if (c->is_in_group("slot")) { // only remove the ones representing slots
					remove_child(c);
					c->queue_free();
					break;
				}
			}
		}
		this->reset_size();
	}

	void add_slot() {
		Control *row = memnew(Control); // create a child Control (visual row)
		row->set_custom_minimum_size(Vector2(0, 24.f)); // give it vertical space so slots are spread out
		row->add_to_group("slot");
		this->add_child(row); // add the Control to the GraphNode (creates the GUI row)

		this->set_slot(this->get_output_port_count(),
				/*left*/ false, /*type_left*/ 0, Color(0, 0, 0, 0),
				/*right*/ true, /*type_right*/ 0, Color(0.2, 0.8, 0.2));
	}

protected:
	static void _bind_methods() {
	}
};

class BT_Graph_ReactiveSelectorNode : public BT_Graph_Node {
	GDCLASS(BT_Graph_ReactiveSelectorNode, BT_Graph_Node);

public:
	BT_Graph_ReactiveSelectorNode() {
		add_default_slot(this, 0);
		max_children = -1;
		Button *add_button = memnew(Button);
		add_button->set_text("+");
		add_button->connect("pressed", callable_mp(this, &BT_Graph_ReactiveSelectorNode ::add_slot));
		this->add_child(add_button);

		Button *remove_button = memnew(Button);
		remove_button->set_text("-");
		remove_button->connect("pressed", callable_mp(this, &BT_Graph_ReactiveSelectorNode ::remove_slot));
		this->add_child(remove_button);
	}
	Array child_nodes; // Array of BT_Node*
	int current_child = 0;

	void rebuild_slots(float row_height = 24.0f) override {
		this->clear_all_slots();

		add_default_slot(this, 0);
		for (size_t i = 1; i < this->children.size(); i++) {
			add_output_slot(this, i);
		}
		this->reset_size();
	}

	void remove_slot() {
		//remove last inx slot
		int slots = this->get_output_port_count();
		if (slots <= 0) {
			return;
		}

		for (int i = get_child_count() - 1; i >= 0; --i) {
			if (Control *c = Object::cast_to<Control>(get_child(i))) {
				if (c->is_in_group("slot")) { // only remove the ones representing slots
					remove_child(c);
					c->queue_free();
					break;
				}
			}
		}
		this->reset_size();
	}

	void add_slot() {
		Control *row = memnew(Control); // create a child Control (visual row)
		row->set_custom_minimum_size(Vector2(0, 24.f)); // give it vertical space so slots are spread out
		row->add_to_group("slot");
		this->add_child(row); // add the Control to the GraphNode (creates the GUI row)

		this->set_slot(this->get_output_port_count(),
				/*left*/ false, /*type_left*/ 0, Color(0, 0, 0, 0),
				/*right*/ true, /*type_right*/ 0, Color(0.2, 0.8, 0.2));
	}

protected:
	static void _bind_methods() {
	}
};

class BT_Graph_ParralelNode : public BT_Graph_Node {
	GDCLASS(BT_Graph_ParralelNode, BT_Graph_Node);

public:
	BT_Graph_ParralelNode() {
		add_default_slot(this, 0);
		max_children = -1;
		Button *add_button = memnew(Button);
		add_button->set_text("+");
		add_button->connect("pressed", callable_mp(this, &BT_Graph_ParralelNode::add_slot));
		this->add_child(add_button);

		Button *remove_button = memnew(Button);
		remove_button->set_text("-");
		remove_button->connect("pressed", callable_mp(this, &BT_Graph_ParralelNode::remove_slot));
		this->add_child(remove_button);
	}
	Array child_nodes; // Array of BT_Node*
	int current_child = 0;

	void rebuild_slots(float row_height = 24.0f) override {
		this->clear_all_slots();

		add_default_slot(this, 0);
		for (size_t i = 1; i < this->children.size(); i++) {
			add_output_slot(this, i);
		}
		this->reset_size();
	}

	void remove_slot() {
		//remove last inx slot
		int slots = this->get_output_port_count();
		if (slots <= 0) {
			return;
		}

		for (int i = get_child_count() - 1; i >= 0; --i) {
			if (Control *c = Object::cast_to<Control>(get_child(i))) {
				if (c->is_in_group("slot")) { // only remove the ones representing slots
					remove_child(c);
					c->queue_free();
					break;
				}
			}
		}
		this->reset_size();
	}

	void add_slot() {
		Control *row = memnew(Control); // create a child Control (visual row)
		row->set_custom_minimum_size(Vector2(0, 24.f)); // give it vertical space so slots are spread out
		row->add_to_group("slot");
		this->add_child(row); // add the Control to the GraphNode (creates the GUI row)

		this->set_slot(this->get_output_port_count(),
				/*left*/ false, /*type_left*/ 0, Color(0, 0, 0, 0),
				/*right*/ true, /*type_right*/ 0, Color(0.2, 0.8, 0.2));
	}

protected:
	static void _bind_methods() {
	}
};

/// <summary>
/// inverts the result of its child
/// example use: you want the agent to check if someone is NOT visible you can now just run a visible check and reverse the result
/// </summary>
class BT_Graph_InvertDecorator : public BT_Graph_Node {
	GDCLASS(BT_Graph_InvertDecorator, BT_Graph_Node);

public:
	BT_Graph_InvertDecorator() {
		add_default_slot(this, 0);
	}

	NodePath child_node; // Array of BT_Node*
	NodePath get_child_node();
	void set_child_node(const NodePath &child);

	void rebuild_slots(float row_height = 24.0f) override {
		this->clear_all_slots();

		add_default_slot(this, 0);
		for (size_t i = 1; i < this->children.size(); i++) {
			add_output_slot(this, i);
		}
		this->reset_size();
	}


protected:
	static void _bind_methods() {
	}
};

/// <summary>
/// repeats the child for a certain amount of times or until it succeeds
/// </summary>
class BT_Graph_RepeaterDecorator : public BT_Graph_Node {
	GDCLASS(BT_Graph_RepeaterDecorator, BT_Graph_Node);

public:
	BT_Graph_RepeaterDecorator() {
		add_default_slot(this, 0);
	}

	enum repeat_mode { until_succes = 0,
		repeat_X_times = 1 } mode;

	void set_mode(repeat_mode p_mode);
	repeat_mode get_mode();

	void set_repeat_count(int count);
	int get_repeat_count();

	int repeat_amount = 1;


	void rebuild_slots(float row_height = 24.0f) override {
		this->clear_all_slots();

		add_default_slot(this, 0);
		for (size_t i = 1; i < this->children.size(); i++) {
			add_output_slot(this, i);
		}
		this->reset_size();
	}


protected:
	int repeat_count = 0;
	static void _bind_methods() {

		// Expose enum as int
		ClassDB::bind_method(D_METHOD("set_mode", "mode"), &BT_Graph_RepeaterDecorator::set_mode);
		ClassDB::bind_method(D_METHOD("get_mode"), &BT_Graph_RepeaterDecorator::get_mode);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_ENUM, "Until Success,Repeat X Times"),
				"set_mode", "get_mode");

		// Bind constants so they show up in scripts
		BIND_ENUM_CONSTANT(until_succes);
		BIND_ENUM_CONSTANT(repeat_X_times);

		// Repeat count
		ClassDB::bind_method(D_METHOD("set_repeat_count", "count"), &BT_Graph_RepeaterDecorator::set_repeat_count);
		ClassDB::bind_method(D_METHOD("get_repeat_count"), &BT_Graph_RepeaterDecorator::get_repeat_count);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "repeat_count"),
				"set_repeat_count", "get_repeat_count");
	}
};
VARIANT_ENUM_CAST(BT_Graph_RepeaterDecorator::repeat_mode);
