@tool
extends Node

@export_tool_button("Test Me!") var test_me_action = test_me

func test_me() -> void:
	#var example := ExampleClass.new()
	#example.print_type(example)
	var visibility_interface := PermissiveVisibilityInterface.new()
	var size := 5
	var arr := []
	arr.resize(size * size)
	arr.fill(0)
	var los_blocker_data := PackedByteArray(arr)
	print("Preparing to calculate sightlines")
	visibility_interface.prepare_to_calculate_sightlines(
		los_blocker_data,
		Vector2(size, size)
	)
	print("Calculating sightlines from all tiles")
	for j in range(size):
		var row_str := ""
		for i in range(size):
			row_str += " " + str(visibility_interface.calculate_sightlines_from_tile(i, j))
		print(row_str)
	print("Tiles block light:")
	for j in range(size):
		var row_str := ""
		for i in range(size):
			row_str += " " + str(visibility_interface.blocks_light(i, j))
		print(row_str)
	print("Checking if each tile can see another tile:")
	for y in range(size):
		for x in range(size):
			print("-")
			for j in range(size):
				var row_str := ""
				for i in range(size):
					row_str += " " + str(visibility_interface.can_tile_see(Vector2(x, y), Vector2(i, j)))
				print(row_str)
