@tool
extends EditorPlugin
class_name TBPlugin

var map_control: Control = null
var editing_loader: WeakRef = weakref(null)

func _enter_tree():
	map_control = create_map_control()
	map_control.set_visible(false)
	add_control_to_container(EditorPlugin.CONTAINER_SPATIAL_EDITOR_MENU, map_control)

func _exit_tree():
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

	var ret = HBoxContainer.new()
	ret.add_child(button_build_meshes)
	return ret

func build_meshes():
	var loader = editing_loader.get_ref()
	loader.build_meshes()
