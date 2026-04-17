#include "register_types.h"

#include "behavior_tree.h"
#include "behavior_tree_graph.h"
#include "behavior_tree_plugin.h"
#include "example_class.h"
#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_INTERNAL_CLASS(BT_GraphEdit);

		EditorPlugins::add_by_type<BT_GraphEdit>();
	}

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	GDREGISTER_CLASS(BT_Tree_Resource);
	GDREGISTER_CLASS(BT_node_data);

	GDREGISTER_ABSTRACT_CLASS(BT_Node);
	GDREGISTER_CLASS(BT_Tree);
	GDREGISTER_CLASS(BT_ActionNode);
	GDREGISTER_CLASS(BT_WaitNode);
	GDREGISTER_CLASS(BT_SequenceNode);
	GDREGISTER_CLASS(BT_ReactiveSequenceNode);
	GDREGISTER_CLASS(BT_ParralelNode);
	GDREGISTER_CLASS(BT_ConditionNode);
	GDREGISTER_CLASS(BT_SelectorNode);
	GDREGISTER_CLASS(BT_ReactiveSelectorNode);
	GDREGISTER_CLASS(BT_InvertDecorator);
	GDREGISTER_CLASS(BT_RepeaterDecorator);
	GDREGISTER_ABSTRACT_CLASS(BT_Graph_Node);
	GDREGISTER_CLASS(BT_Graph_Tree);
	GDREGISTER_CLASS(BT_Graph_ActionNode);
	GDREGISTER_CLASS(BT_Graph_WaitNode);
	GDREGISTER_CLASS(BT_Graph_SequenceNode);
	GDREGISTER_CLASS(BT_Graph_ReactiveSequenceNode);
	GDREGISTER_CLASS(BT_Graph_ConditionNode);
	GDREGISTER_CLASS(BT_Graph_SelectorNode);
	GDREGISTER_CLASS(BT_Graph_ReactiveSelectorNode);
	GDREGISTER_CLASS(BT_Graph_ParralelNode);
	GDREGISTER_CLASS(BT_Graph_InvertDecorator);
	GDREGISTER_CLASS(BT_Graph_RepeaterDecorator);
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
// Initialization
GDExtensionBool GDE_EXPORT behaviortree_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
	init_obj.register_initializer(initialize_gdextension_types);
	init_obj.register_terminator(uninitialize_gdextension_types);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
