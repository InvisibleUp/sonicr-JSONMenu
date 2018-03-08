#pragma once
#include "rapidjson\document.h"
#include "rapidjson\filereadstream.h"

// https://stackoverflow.com/a/4609795
template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

rapidjson::Document dump_json(const char *filepath);
unsigned int JSON_GetNum(rapidjson::Value &base, const char *prop, unsigned int def);