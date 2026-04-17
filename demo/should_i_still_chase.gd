@tool
extends BT_ConditionNode

var time = 0
var chaseDuration = 2
#mandatory if you want the condition to be registered in gdscript. else will #always return false unless a c++ implementation is found
func _ready():
	initialize_condition()

# This function must return true or false
# Should not modify the world
func condition_to_check(delta) -> bool:
	time += delta
	if time <= chaseDuration:
		return STATUS_SUCCESS
	time = 0
	return STATUS_SUCCESS
	
