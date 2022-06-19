#include <godot/gdnative_interface.h>

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
	GDNativeBool GDN_EXPORT tbloader_init(
		const GDNativeInterface *p_interface,
		const GDNativeExtensionClassLibraryPtr p_library,
		GDNativeInitialization *r_initialization
	) {
		GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

		init_obj.register_initializer(register_tbloader_types);
		init_obj.register_terminator(unregister_tbloader_types);

		return init_obj.init();
	}
}
