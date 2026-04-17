extends BT_ActionNode

var agent_node : Node2D
var player: CharacterBody2D
func _ready() -> void:
	player = get_tree().get_first_node_in_group("player")
	agent_node = get_tree().get_first_node_in_group("agent")


func _update(delta: float):
	if not agent_node:
		agent_node = get_tree().get_first_node_in_group("agent")
		return STATUS_RUNNING
	if not player:
		player = get_tree().get_first_node_in_group("player")
		return STATUS_RUNNING
		
	var direction: Vector2 = player.global_position - agent_node.global_position
	if direction.length_squared() > 0.0001:
		# Rotate agent to face player
		agent_node.rotation = direction.angle()
		
	return STATUS_RUNNING	
		
