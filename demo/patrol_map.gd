extends BT_ActionNode

var path_follow : PathFollow2D
var agent_node : Node2D
var speed = 500
var arrive_threshold := 5.0


func _ready() -> void:
	call_deferred("_init_path_follow")

func _init_path_follow() -> void:
	path_follow = get_tree().get_first_node_in_group("brian")
	if path_follow:
		print("PathFollow found: ", path_follow.name)
	else:
		printerr("No PathFollow in group 'brian'")
	

func _update(delta: float):
	if not agent_node:
		agent_node = get_tree().get_first_node_in_group("agent")
		return STATUS_RUNNING
	if not path_follow:
		get_tree().get_first_node_in_group("brian")
		return STATUS_RUNNING
		
	# Compute direction toward the current path position
	var target_pos = path_follow.global_position
	var direction = target_pos - agent_node.global_position
	var distance_sq = direction.length_squared()

	if distance_sq > arrive_threshold * arrive_threshold:
		# Move toward the path position smoothly
		direction = direction.normalized()
		agent_node.global_position += direction * speed * delta
	else:
		# Agent is close enough to path, advance along it
		path_follow.progress += speed * delta

	return STATUS_RUNNING
		
