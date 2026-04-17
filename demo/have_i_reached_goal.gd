@tool
extends BT_ConditionNode
var agent_node : Node2D
var nav_agent : NavigationAgent2D
#mandatory if you want the condition to be registered in gdscript. else will always return false unless a c++ implementation is found
func _ready():
	initialize_condition()


# This function must return true or false
# Should not modify the world
func condition_to_check(delta):
	if not agent_node:
		agent_node = get_tree().get_first_node_in_group("agent")
	if not nav_agent:
		nav_agent = agent_node.get_node("NavigationAgent2D")
	
	
	if nav_agent.is_navigation_finished():
		return STATUS_FAILURE
	return STATUS_SUCCESS
