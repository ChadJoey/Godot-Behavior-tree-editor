extends Node2D


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	var ai_scene = preload("res://brian_real.tscn")
	var ai = ai_scene.instantiate()
	add_child(ai)
# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
