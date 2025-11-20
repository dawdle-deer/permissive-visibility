@tool
extends Node

@export_tool_button("Test Me!") var test_me_action = test_me

func test_me() -> void:
	#var example := ExampleClass.new()
	#example.print_type(example)
	var visibility_interface := PermissiveVisibilityInterfaceGDExt.new()
	var size := 5
	var arr : Array[bool] = []
	arr.resize(size * size)
	arr.fill(false)
	for i in range(size * size):
		arr[i] = randf() > 0.5
	var los_blocker_data := PackedByteArray(arr)
	print("Preparing to calculate sightlines")
	visibility_interface.prepare_to_calculate_sightlines(
		los_blocker_data,
		Vector2(size, size)
	)
	print("Calculating sightlines from all tiles")
	for j in range(size):
		print("-")
		for i in range(size):
			var sightlines := visibility_interface.calculate_sightlines_from_tile(i, j)
			print(sightlines)
			print_bool_arr(sightlines, size, true, Vector2i(i, j))
	print("Tiles block light:")
	print_bool_arr(visibility_interface.blocks_light, size, false)
	print("Checking if each tile can see another tile:")
	for y in range(size):
		for x in range(size):
			print("-")
			print_bool_arr(visibility_interface.can_tile_see.bind(Vector2(x, y)), size, true, Vector2(x, y))

func print_bool_arr(data, size : int, pass_vector := true, color_target := Vector2i(-1, -1)):
	for j in range(size):
		var row_str := ""
		for i in range(size):
			if i == color_target.x and j == color_target.y:
				row_str += "[color=red]"
			if data is Array:
				row_str += " " + ("1" if data[(j * size) + i] else "0")
			elif data is Callable:
				if pass_vector:
					row_str += " " + ("1" if data.call(Vector2(i, j)) else "0")
				else:
					row_str += " " + ("1" if data.call(i, j) else "0")
			if i == color_target.x and j == color_target.y:
				row_str += "[/color]"
		print_rich(row_str)
