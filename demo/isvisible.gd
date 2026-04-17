@tool
extends BT_ConditionNode

var player: CharacterBody2D
var agent_node : Node2D


var angle = 30
var range = 5

func _ready():
	initialize_condition()
	player = get_tree().get_first_node_in_group("player")
	agent_node = get_tree().get_first_node_in_group("agent")

# This function must return true or false
# Should not modify the world
func condition_to_check(delta):
	if not player:
		player = get_tree().get_first_node_in_group("player")
	if not agent_node:
		agent_node = get_tree().get_first_node_in_group("agent")
		
		
	if not agent_node or not player:
		printerr("attempt to assign player or agent failed")
		return false
		
	var from = agent_node.global_position
	var to = player.global_position
	var space_state = agent_node.get_world_2d().direct_space_state

	# Create raycast parameters
	var query = PhysicsRayQueryParameters2D.create(from, to)
	query.exclude = [agent_node]
	query.collision_mask = 0xFFFFFFFF

	# Perform the raycast
	var result = space_state.intersect_ray(query)
	if result and result.collider == player:
		print("i can see");
	else:
		print("i cant see")
	# Return true if the ray hit the player
	return result and result.collider == player
