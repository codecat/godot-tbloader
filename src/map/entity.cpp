#include "entity.h"

#include <string.h>

bool LMEntity::has_property(const char* key)
{
	for (int i = 0; i < property_count; i++) {
		if (!strcmp(properties[i].key, key)) {
			return true;
		}
	}
	return false;
}

const char* LMEntity::get_property(const char* key, const char* def)
{
	for (int i = 0; i < property_count; i++) {
		auto& prop = properties[i];
		if (!strcmp(prop.key, key)) {
			return prop.value;
		}
	}
	return def;
}

int LMEntity::get_property_int(const char* key, int def)
{
	const char* str = get_property(key, nullptr);
	if (str == nullptr) {
		return def;
	}
	return atoi(str);
}

float LMEntity::get_property_float(const char* key, float def)
{
	const char* str = get_property(key, nullptr);
	if (str == nullptr) {
		return def;
	}
	//TODO: Locale?
	return (float)atof(str);
}

double LMEntity::get_property_double(const char* key, double def)
{
	const char* str = get_property(key, nullptr);
	if (str == nullptr) {
		return def;
	}
	//TODO: Locale?
	return atof(str);
}

vec2 LMEntity::get_property_vec2(const char* key, const vec2& def)
{
	const char* str = get_property(key, nullptr);
	if (str == nullptr) {
		return def;
	}
	return vec2_parse(str);
}

vec3 LMEntity::get_property_vec3(const char* key, const vec3& def)
{
	const char* str = get_property(key, nullptr);
	if (str == nullptr) {
		return def;
	}
	return vec3_parse(str);
}
