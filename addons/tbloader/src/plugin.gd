@tool
extends EditorPlugin
class_name TBPlugin

var map_control: Control = null
var editing_loader: WeakRef = weakref(null)

func _enter_tree():
	set_icons(true)

	map_control = create_map_control()
	map_control.set_visible(false)
	add_control_to_container(EditorPlugin.CONTAINER_SPATIAL_EDITOR_MENU, map_control)

func _exit_tree():
	set_icons(false)

	remove_control_from_container(EditorPlugin.CONTAINER_SPATIAL_EDITOR_MENU, map_control)
	map_control.queue_free()
	map_control = null

func _handles(object):
	return object is TBLoader

func _make_visible(visible: bool):
	map_control.set_visible(visible)

func _edit(object):
	editing_loader = weakref(object)

func create_map_control() -> Control:
	var button_build_meshes = Button.new()
	button_build_meshes.flat = true
	button_build_meshes.text = "Build Meshes"
	button_build_meshes.connect("pressed", Callable(self, "build_meshes"))

	var button_build_csg = Button.new()
	button_build_csg.flat = true
	button_build_csg.text = "Build Combined CSG"
	button_build_csg.connect("pressed", Callable(self, "build_combined_csg"))

	var ret = HBoxContainer.new()
	ret.add_child(button_build_meshes)
	ret.add_child(button_build_csg)
	return ret

func build_meshes():
	var loader = editing_loader.get_ref()
	loader.build_meshes()

func build_combined_csg():
	var loader = editing_loader.get_ref()
	loader.build_combined_csg()

func set_icons(on):
	var editor_interface = get_editor_interface()
	var base_control = editor_interface.get_base_control()
	var theme = base_control.theme

	if on:
		var texture = ResourceLoader.load("res://addons/tbloader/icons/tbloader.svg")
		theme.set_icon("TBLoader", "EditorIcons", texture)
	else:
		theme.clear_icon("TBLoader", "EditorIcons")
