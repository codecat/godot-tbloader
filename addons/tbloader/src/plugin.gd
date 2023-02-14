@tool
extends EditorPlugin
class_name TBPlugin

var map_control: Control = HBoxContainer.new()
var editing_loader: WeakRef = weakref(null)

func _enter_tree():
	set_icons(true)

	add_button_to_control("Build Meshes", "build_meshes")
	add_button_to_control("Build FGD", "build_fgd")
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

func add_button_to_control(text, callback):
	var button = Button.new()
	button.flat = true
	button.text = text
	button.connect("pressed", Callable(self, callback))
	map_control.add_child(button)

func build_meshes():
	var loader = editing_loader.get_ref()
	loader.build_meshes()

func build_fgd():
	var loader = editing_loader.get_ref()
	loader.build_fgd()

func set_icons(on):
	var editor_interface = get_editor_interface()
	var base_control = editor_interface.get_base_control()
	var theme = base_control.theme

	if on:
		var texture = ResourceLoader.load("res://addons/tbloader/icons/tbloader.svg")
		theme.set_icon("TBLoader", "EditorIcons", texture)
	else:
		theme.clear_icon("TBLoader", "EditorIcons")
