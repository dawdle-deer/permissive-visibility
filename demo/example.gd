extends Node


func _ready() -> void:
	#var example := ExampleClass.new()
	#example.print_type(example)
	var visibility_interface := PermissiveVisibilityInterface.new()
	var los_blocker_data := PackedByteArray([0, 0, 0, 0, 0, 0, 0, 0, 0])
	print(los_blocker_data.size())
	visibility_interface.prepare_to_calculate_sightlines(
		los_blocker_data,
		Vector2(3, 3)
	)
	#visibility_interface.set_visible(2, 2)
	#visibility_interface.set_visible(0, 0)
	##visibility_interface.set_visible(1, 0)
	#print(visibility_interface.calculate_sightlines_from_tile(0, 0))
	#print(visibility_interface.blocks_light(1, 1))
	#print(visibility_interface.can_tile_see(Vector2.ZERO, Vector2(2, 2)))
