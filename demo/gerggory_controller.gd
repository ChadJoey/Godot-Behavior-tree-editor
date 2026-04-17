extends CharacterBody2D


const SPEED = 300.0
func _ready() -> void:
	add_to_group("player")
	motion_mode = 1

func _physics_process(delta: float) -> void:
	var direction = Input.get_vector("ui_left", "ui_right", "ui_up", "ui_down")
	velocity = direction * SPEED

	move_and_slide()
