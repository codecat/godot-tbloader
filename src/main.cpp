#include <godot/gdnative_interface.h>

#include <godot_cpp/godot.hpp>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/binder_common.hpp>

#include <godot_cpp/classes/node3d.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

class TBLoader : public Node3D
{
	GDCLASS(TBLoader, Node3D);

protected:
	static void _bind_methods()
	{
		ClassDB::bind_method(D_METHOD("do_something"), &TBLoader::do_something);
	}

public:
	TBLoader()
	{
		UtilityFunctions::print("TBLoader constructor");
	}

	~TBLoader()
	{
		UtilityFunctions::print("TBLoader denstructor");
	}

	void do_something()
	{
		UtilityFunctions::print("TBLoader do_something was called!");
	}
};

void register_tbloader_types()
{
	ClassDB::register_class<TBLoader>();
}

void unregister_tbloader_types()
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

		init_obj.register_scene_initializer(register_tbloader_types);
		init_obj.register_scene_terminator(unregister_tbloader_types);

		return init_obj.init();
	}
}
