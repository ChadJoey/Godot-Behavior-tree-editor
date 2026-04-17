#pragma once

#include <godot_cpp/classes/graph_Node.hpp>
#include <godot_cpp/classes/graph_edit.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "behavior_tree_graph.h"


using namespace godot;

/// <summary>
/// base Node
/// meant to be inhereted by nodes with actual functionality
/// </summary>
class BT_Node : public Node {
	GDCLASS(BT_Node, Node);

public:
	enum Status {
		STATUS_RUNNING,
		STATUS_SUCCESS,
		STATUS_FAILURE
	};

	Status status = STATUS_FAILURE;
	TypedArray<BT_Node> child_nodes; // Array of BT_Node*
	virtual int _update(float dt) = 0;
	void set_status(Status s);
	Status get_status() const;

	void add_bt_child(BT_Node *node) {
		child_nodes.push_back(node);
	}


protected:
	float timer = 0;
	static void _bind_methods() {
		BIND_ENUM_CONSTANT(STATUS_RUNNING);
		BIND_ENUM_CONSTANT(STATUS_SUCCESS);
		BIND_ENUM_CONSTANT(STATUS_FAILURE);
		// Bind setter/getter so the property shows in the inspector
		ClassDB::bind_method(D_METHOD("set_status", "status"), &BT_Node::set_status);
		ClassDB::bind_method(D_METHOD("get_status"), &BT_Node::get_status);

		// Register property with inspector
		ADD_PROPERTY(PropertyInfo(Variant::INT, "status", PROPERTY_HINT_ENUM, "STATUS_RUNNING,STATUS_SUCCESS,STATUS_FAILURE"),
				"set_status", "get_status");

		ClassDB::bind_method(D_METHOD("update", "dt"), &BT_Node::_update);
	}
};
VARIANT_ENUM_CAST(BT_Node::Status);





/// <summary>
/// returns true or false
/// checks facts should not change the world
/// example use:
/// Is_player_visible()? -> returns true if player is visible, returns false if not
/// </summary>
class BT_ConditionNode : public BT_Node {
	GDCLASS(BT_ConditionNode, BT_Node);

public:
	Callable condition_call;
	int _update(float dt) override;

	void _initialize_condition();
	void set_condition();

	//can be defined in gdscript or overridden by c++ logic
	virtual bool condition_to_check() {
		return false;
	};

	//check both gdscript and c++ to allow for both options. gdscript takes prio
	int evaluate(float dt);

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_condition"), &BT_ConditionNode::set_condition);
		ClassDB::bind_method(D_METHOD("evaluate"), &BT_ConditionNode::evaluate);
		ClassDB::bind_method(D_METHOD("initialize_condition"), &BT_ConditionNode::_initialize_condition);
	}
};
/// <summary>
/// has no children
/// preforms an action in the game world
/// example use:
/// BT_ActionNode MoveTo(target) -> returns Running until the NPC reaches the target, then SUCCES
/// </summary>
class BT_ActionNode : public BT_Node {
	GDCLASS(BT_ActionNode, BT_Node);

public:
	virtual int _update(float dt) {
		if (!has_method("_update")) {
			set_status(STATUS_FAILURE);
			return STATUS_FAILURE;
		}
		Variant result = Object::call("_update", dt);
		int int_status = static_cast<int>(result); // assuming the GDScript always returns int Status
		set_status(static_cast<Status>(int_status));
		return int_status;
	}

protected:
	static void _bind_methods() {
	}
};

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
class BT_SequenceNode : public BT_Node {
	GDCLASS(BT_SequenceNode, BT_Node);

public:
	int current_child = 0;
	virtual int _update(float dt) override;

protected:
	static void _bind_methods() {
	}
};

/// <summary>
/// fails only if all children fail
/// often used as an OR
/// example use: you want an agent to patrol OR chase the player
/// </summary>
class BT_SelectorNode : public BT_Node {
	GDCLASS(BT_SelectorNode, BT_Node);

public:

	int current_child = 0;
	virtual int _update(float dt) override;

protected:
	static void _bind_methods() {
	}
};

class BT_ReactiveSelectorNode : public BT_Node {
	GDCLASS(BT_ReactiveSelectorNode, BT_Node);

public:
	int current_child = 0;
	virtual int _update(float dt) override;

protected:
	static void _bind_methods() {
	}
};

class BT_ReactiveSequenceNode : public BT_Node {
	GDCLASS(BT_ReactiveSequenceNode, BT_Node);

public:
	int current_child = 0;
	virtual int _update(float dt) override;

protected:
	static void _bind_methods() {
	}
};



class BT_ParralelNode : public BT_Node {
	GDCLASS(BT_ParralelNode, BT_Node);

public:
	int current_child = 0;
	virtual int _update(float dt) override;

protected:
	static void _bind_methods() {
	}

private:
	Vector<BT_Node::Status> child_states;
};


/// <summary>
/// inverts the result of its child
/// example use: you want the agent to check if someone is NOT visible you can now just run a visible check and reverse the result
/// </summary>
class BT_InvertDecorator : public BT_Node {
	GDCLASS(BT_InvertDecorator, BT_Node);

public:
	virtual int _update(float dt) override;

protected:
	static void _bind_methods() {
	}
};

/// <summary>
/// repeats the child for a certain amount of times or until it succeeds
/// </summary>
class BT_RepeaterDecorator : public BT_Node {
	GDCLASS(BT_RepeaterDecorator, BT_Node);

public:
	virtual int _update(float dt) override;

	enum repeat_mode { until_succes = 0,
		repeat_X_times = 1 } mode;

	void set_mode(repeat_mode p_mode);
	repeat_mode get_mode();


	void set_repeat_count(int count);
	int get_repeat_count();

	int repeat_amount = 1;

protected:
	int repeat_count = 0;
	static void _bind_methods() {
		// Expose enum as int
		ClassDB::bind_method(D_METHOD("set_mode", "mode"), &BT_RepeaterDecorator::set_mode);
		ClassDB::bind_method(D_METHOD("get_mode"), &BT_RepeaterDecorator::get_mode);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_ENUM, "Until Success,Repeat X Times"),
				"set_mode", "get_mode");

		// Bind constants so they show up in scripts
		BIND_ENUM_CONSTANT(until_succes);
		BIND_ENUM_CONSTANT(repeat_X_times);

		// Repeat count
		ClassDB::bind_method(D_METHOD("set_repeat_count", "count"), &BT_RepeaterDecorator::set_repeat_count);
		ClassDB::bind_method(D_METHOD("get_repeat_count"), &BT_RepeaterDecorator::get_repeat_count);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "repeat_count"),
				"set_repeat_count", "get_repeat_count");
	}
};
VARIANT_ENUM_CAST(BT_RepeaterDecorator::repeat_mode);


/// <summary>
/// waits a given period of time before executing child
/// </summary>
class BT_WaitNode : public BT_Node {
	GDCLASS(BT_WaitNode, BT_Node);

	public:

	enum end_state { return_succes = 0,
			return_failed = 1 } state;

	virtual int _update(float dt) override;

	float get_duration() { return duration; };
	void set_duration(float time) {duration = time; };


	void set_end_state(end_state p_mode);
	end_state get_end_state();

	private:
	float duration = 0;
	float time = 0;

	protected:

	static void _bind_methods()
	{

		ClassDB::bind_method(D_METHOD("set_mode", "mode"), &BT_WaitNode::set_end_state);
		ClassDB::bind_method(D_METHOD("get_mode"), &BT_WaitNode::get_end_state);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_ENUM, "choose wich state it returns on completion"),
				"set_mode", "get_mode");

		ClassDB::bind_method(D_METHOD("set_duration", "duration"), &BT_WaitNode::set_duration);
		ClassDB::bind_method(D_METHOD("get_duration"), &BT_WaitNode::get_duration);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"),"set_duration", "get_duration");
	}
};
VARIANT_ENUM_CAST(BT_WaitNode::end_state);


class BT_Tree : public BT_Node {
	GDCLASS(BT_Tree, BT_Node);

public:
	float TickRate = 60;

	//_update does not get used here and has to be defined for bt_node
	int _update(float dt) override { return 0; };
	BT_Node* root = nullptr;
	void set_tickrate(int i);
	int get_tickrate();

	void set_root(BT_Node *p_root) { root = p_root; }
	BT_Node *get_root() const { return root; }

	Ref<BT_Tree_Resource> tree_data;
	TypedArray<BT_Node> tree;

	void _process(double delta) override;
	void _ready() override;

	void set_tree_data(const Ref<BT_Tree_Resource> &data);
	Ref<BT_Tree_Resource> get_tree_data() const;

	void CreateTree();

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_tree_data", "data"), &BT_Tree::set_tree_data);
		ClassDB::bind_method(D_METHOD("get_tree_data"), &BT_Tree::get_tree_data);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tree_data", PROPERTY_HINT_RESOURCE_TYPE, "BT_Tree_Resource"), "set_tree_data", "get_tree_data");

		ClassDB::bind_method(D_METHOD("set_root", "root"), &BT_Tree::set_root);
		ClassDB::bind_method(D_METHOD("get_root"), &BT_Tree::get_root);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root", PROPERTY_HINT_RESOURCE_TYPE, "BT_Node"), "set_root", "get_root");

		ClassDB::bind_method(D_METHOD("set_tickrate", "tickrate"), &BT_Tree::set_tickrate);
		ClassDB::bind_method(D_METHOD("get_tickrate"), &BT_Tree::get_tickrate);
		ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tickrate"), "set_tickrate", "get_tickrate");
	}
};
