extends BT_ActionNode

var agent_node : Node2D
var nav_agent : NavigationAgent2D
var player : CharacterBody2D
var speed = 350


func _ready():
	player = get_tree().get_first_node_in_group("player")



func _update(delta: float):
	if not agent_node:
		agent_node = get_tree().get_first_node_in_group("agent")
		nav_agent = agent_node.get_node("NavigationAgent2D")
		
	if not agent_node or not nav_agent or not player:
		printerr("no valid player or agent or nav_agent node found")
		return STATUS_RUNNING
	#set target position to the player position
	nav_agent.target_position = player.global_position
	var next_path_position = nav_agent.get_next_path_position()
	var direction = (next_path_position - agent_node.global_position).normalized()
		
	agent_node.global_position += direction * speed * delta	
	return STATUS_RUNNING
