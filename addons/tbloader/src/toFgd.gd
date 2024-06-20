extends SceneTree

var supported_types = {
  1: "bool",
  2: "integer",
  3: "float",
  4: "string",
  20: "color",
  27: "flags",
  28: "choices"
}

func get_files(entities_dir):
  # Define the root directory to search
  var root_dir = "res://%s" % entities_dir
  var dir = DirAccess.open(root_dir)
  var files = []
  var folders = []

  # Get all files in the directory
  dir.list_dir_begin()
  while true:
    var file = dir.get_next()
    if file == "":
      break
    if dir.current_is_dir():
      folders.append(file)
    else:
      files.append("%s/%s" % [entities_dir, file])
  dir.list_dir_end()

  for folder in folders:
    # Recursively get all files in the subdirectories
    files += get_files("%s/%s" % [entities_dir, folder])

  return files

func remove_attached_scripts(files):
  var filtered_files = []
  var attached_scripts = []

  # Find all attached scripts
  for file in files:
    if file.ends_with(".tscn"):
      var scene = load(file)
      var instance = scene.instantiate()
      var script = instance.get_script()
      if script: attached_scripts.append(script.get_path().replace("res://", ""))
      instance.queue_free()
  
  # Remove attached scripts from the list
  for file in files:
    if file not in attached_scripts:
      filtered_files.append(file)

  return filtered_files

func get_entity_properties(files = []):
  var properties = []

  for file in files:
    if file.ends_with(".gd"):
      properties.append(create_entity(file, script_loader(file)))
    if file.ends_with(".tscn"):
      properties.append(create_entity(file, scene_loader(file)))
    else:
      continue
  
  return properties

func scene_loader(path):
  var properties = []
  var scene = load(path)
  var instance = scene.instantiate()
  var script = instance.get_script()
  if script:
    properties = script_loader(script.get_path())
    var parsed_path = path_parser(path)
    properties = set_property_by_name("name", parsed_path[0], properties)
  instance.queue_free()
  return properties

func script_loader(path):
  var script = load(path)
  var properties = filter_properties(script.get_script_property_list())
  var instance = script.new()
  for property in properties:
    property.default_value = instance.get(property.name)
  instance.queue_free()
  return properties

func filter_properties(properties = []):
  # Filter out properties that are not exposed to the editor
  var filtered_properties = []
  for property in properties:
    if property.type > 0:
      filtered_properties.append(property)
  return filtered_properties

func type_converter(type: int):
  # Convert the type to a string
  return supported_types[type] if type in supported_types else "string"

func value_converter(type: int, input_value: Variant):
  var defaults = {
    1: false,
    2: 0,
    3: 0.0,
    4: "undefined",
    20: Color(1, 1, 1, 1),
    27: {},
    28: []
  }

  var value = input_value if input_value != null else defaults[type]

  # Convert the value to a string
  if type == 4:
    return "\"%s\"" % value
  elif type == 1:
    return "1" if value else "0"
  elif type == 20:
    return "\"%s %s %s\"" % [value.r, value.g, value.b]
  elif type == 27:
    var fgd_values = []
    var count = 1
    for key in value:
      var v = 1 if value[key] else 0
      fgd_values.append("    %s : \"%s\" : %s\n" % [count, key, v])
      if count > 1:
        count *= count
      else:
        count += 1
    
    return "[\n%s  ]" % "".join(PackedStringArray(fgd_values))
  elif type == 28: # Array (but only string array is supported)
    var fgd_values = []
    for i in value.size():
      var v = value[i]
      fgd_values.append("    %s : \"%s\"\n" % [i, v])
    
    return "0 = [\n%s  ]" % "".join(PackedStringArray(fgd_values))
  else:
    return value

func get_property_value_by_name(name, properties = [], default_value = null):
  # Get the properties by name
  for property in properties:
    if property.name == name:
      return property.default_value
  return default_value

func set_property_by_name(name, value, properties = []):
  # Set the properties by name
  for i in properties.size():
    if properties[i].name == name:
      properties[i][name] = value
  return properties

func path_parser(raw_path):
  var regex = RegEx.new()

  # Parse the path to get the entity and group name
  var entity_name = raw_path.get_file().split(".")[0]
  var entity_location = raw_path.get_base_dir()

  # Remove non-alphanumeric characters
  regex.compile("^entities")
  entity_location = regex.sub(entity_location, "")
  entity_location = entity_location.replace("/", "_")
  regex.compile("[\\-\\s]")
  entity_location = regex.sub(entity_location, "_", true)
  regex.compile("[\\W]")
  entity_location = regex.sub(entity_location, "", true)
  regex.compile("^_")
  entity_location = regex.sub(entity_location, "")
  if entity_location:
    entity_location += "_"
  
  var entity_label = string_to_title_case(entity_location)

  return ["%s%s" % [entity_location, entity_name], entity_label]
  
func string_to_title_case(string: String) -> String:
  var words = string.split("_")
  var result = ""
  for word in words:
    result += word.capitalize() + " "
  return result.strip_edges(true, true)

func create_entity(path, properties = []):
  if properties == null:
    return null
  # Header
  # Example: @PointClass size(-4 -4 -4, 4 4 4) color(255 255 0) model({ "path": ":progs/player.mdl" }) = light : "Light" [
  var fgd_solid = get_property_value_by_name("fgd_solid", properties, false)
  var fgd_size = get_property_value_by_name("fgd_size", properties, 4)
  var fgd_color = get_property_value_by_name("fgd_color", properties, "(0 255 0)")
  var fgd_model = get_property_value_by_name("fgd_model", properties, "")
  var fgd_block = get_property_value_by_name("fgd_block", properties, [])

  var class_type = "SolidClass" if fgd_solid else "PointClass"
  var entity_name_properties = path_parser(path)
  var entity_name = entity_name_properties[0]
  var entity_label = entity_name_properties[1]
  fgd_model = " model(%s)" % JSON.stringify(fgd_model) if fgd_model else ""

  var entity = "@%s size(-%s -%s -%s, %s %s %s) color%s%s = %s : \"%s\" [\n" % [
    class_type,
    fgd_size, fgd_size, fgd_size,
    fgd_size, fgd_size, fgd_size,
    fgd_color, fgd_model,
    entity_name, entity_label
  ]

  # Body
  # Example: energy(float) : "Energy" : 1 : "The light's strengh multiplier"
  for property in properties:
    if property.name.begins_with("fgd_") or !supported_types.has(property.type) or property.name in fgd_block:
      continue
    
    if property.type in [27]: # Flags
      entity += "  %s(%s) = %s" % [
        property.name if property.name else "unnamed",
        type_converter(property.type) if property.type else 4,
        value_converter(property.type, property.default_value),
      ]
    else:
      entity += "  %s(%s) : \"%s\" : %s" % [
        property.name if property.name else "unnamed",
        type_converter(property.type) if property.type else 4,
        string_to_title_case(property.name) if property.name else "",
        value_converter(property.type, property.default_value),
      ]

      if property.type not in [28]: # Array
        entity += " : \"%s\"" % property.hint_string if property.hint_string else ""
    
    entity += "\n"

  # Footer
  entity += "]\n\n"

  return entity

func build(entities_dir = "entities"):
  var all_files = get_files(entities_dir) # Get all .gd and .tscn files in the entity folder
  var files = remove_attached_scripts(all_files) # Remove attached scripts
  var properties = get_entity_properties(files) # Get the properties of each entity

  var file = FileAccess.open("res://Game.fgd", FileAccess.WRITE)
  if !file:
    push_error("Failed to open/create Game.fgd!")
    return

  for property in properties:
    if property:
      file.store_string(property)

  print("Game.fgd created successfully!")

func _init():
  build()
