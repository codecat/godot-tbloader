#pragma once

#include <gdextension_interface.h>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/defs.hpp>

#include <godot_cpp/classes/editor_import_plugin.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

using namespace godot;

class TBMapImportPlugin : public EditorImportPlugin
{
	GDCLASS(TBMapImportPlugin, EditorImportPlugin);

protected:
	static void _bind_methods() {}

public:
	
	TBMapImportPlugin();
	~TBMapImportPlugin();

	virtual String _get_save_extension() const override;
	virtual String _get_importer_name() const override;
	virtual String _get_resource_type() const override;
	virtual String _get_visible_name() const override;

	virtual bool _get_option_visibility(const String &path, const StringName &option_name, const Dictionary &options) const override;

	virtual double _get_priority() const override;
	virtual int64_t _get_import_order() const override;

	virtual TypedArray<Dictionary> _get_import_options(const String &path, int64_t preset_index) const override;
	virtual int64_t _get_preset_count() const override;
	virtual String _get_preset_name(int64_t preset_index) const override;

	virtual PackedStringArray _get_recognized_extensions() const override;

	virtual int64_t _import(const String &source_file, const String &save_path, const Dictionary &options, const TypedArray<String> &platform_variants, const TypedArray<String> &gen_files) const override;
};

