#include <gdextension_interface.h>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <tb_loader.h>
#include <map_import.h>

using namespace godot;

void register_tbloader_types(ModuleInitializationLevel p_level)
{
	switch (p_level)
	{
	case MODULE_INITIALIZATION_LEVEL_EDITOR:
		ClassDB::register_class<TBMapImportPlugin>();
		break;
	case MODULE_INITIALIZATION_LEVEL_SCENE:
		ClassDB::register_class<TBLoader>();
		break;
	default:
		break;
	}
}

void unregister_tbloader_types(ModuleInitializationLevel p_level)
{
}

extern "C"
{
	GDExtensionBool GDE_EXPORT tbloader_init(
		const GDExtensionInterface *p_interface,
		GDExtensionClassLibraryPtr p_library,
		GDExtensionInitialization *r_initialization
	) {
		GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

		init_obj.register_initializer(register_tbloader_types);
		init_obj.register_terminator(unregister_tbloader_types);

		return init_obj.init();
	}
}
