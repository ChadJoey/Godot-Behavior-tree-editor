# meta-name: Condition Node
# meta-description: Template for a custom BT_ConditionNode
# meta-default: false

@tool
extends BT_ConditionNode

#mandatory if you want the condition to be registered in gdscript. else will #always return false unless a c++ implementation is found
func _ready():
    initialize_condition()

# This function must return true or false
# Should not modify the world
func condition_to_check() -> bool:
    return true