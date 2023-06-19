#include <gdextension_interface.h>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <tb_loader.h>

using namespace godot;

void register_tbloader_types(ModuleInitializationLevel p_level)
{
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<TBLoader>();
}

void unregister_tbloader_types(ModuleInitializationLevel p_level)
{
}

extern "C"
{
	GDExtensionBool GDE_EXPORT tbloader_init(
			GDExtensionInterfaceGetProcAddress p_get_proc_address,
			GDExtensionClassLibraryPtr p_library,
			GDExtensionInitialization *r_initialization)
	{
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

		init_obj.register_initializer(register_tbloader_types);
		init_obj.register_terminator(unregister_tbloader_types);

		return init_obj.init();
	}
}
