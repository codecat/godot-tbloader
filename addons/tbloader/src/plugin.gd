@tool
extends EditorPlugin
class_name TBPlugin

var map_control: Control = null
var editing_loader: WeakRef = weakref(null)

var import_plugin: TBMapImportPlugin = TBMapImportPlugin.new()

func _enter_tree():
	set_icons(true)

	map_control = create_map_control()
	map_control.set_visible(false)
	add_control_to_container(EditorPlugin.CONTAINER_SPATIAL_EDITOR_MENU, map_control)

	add_import_plugin(import_plugin)

func _exit_tree():
	set_icons(false)

	remove_control_from_container(EditorPlugin.CONTAINER_SPATIAL_EDITOR_MENU, map_control)
	map_control.queue_free()
	map_control = null

	remove_import_plugin(import_plugin)

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
	if loader.get_scene_file_path() != null: # from imported
		var interface = get_editor_interface()
		var fs = interface.get_resource_filesystem()
		fs.reimport_files([loader.get_scene_file_path()])
		interface.reload_scene_from_path(loader.get_tree().edited_scene_root.get_scene_file_path())
	else:
		loader.build_meshes()

func set_icons(on):
	var editor_interface = get_editor_interface()
	var base_control = editor_interface.get_base_control()
	var theme = base_control.theme

	if on:
		var texture = ResourceLoader.load("res://addons/tbloader/icons/tbloader.svg")
		theme.set_icon("TBLoader", "EditorIcons", texture)
	else:
		theme.clear_icon("TBLoader", "EditorIcons")
