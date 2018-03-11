#include "stdafx.h"

rapidjson::Document dump_json(const char *filepath) {
	FILE* fp;
	char *buf = (char *)malloc(65536);

	fopen_s(&fp, filepath, "rb");
	if (fp == 0) { return rapidjson::Document(); }
	rapidjson::FileReadStream is(fp, buf, sizeof(buf));

	rapidjson::Document d;
	d.ParseStream(is);
	fclose(fp);

	free(buf);
	return d;
}

unsigned int JSON_GetNum(rapidjson::Value &base, const char *prop, unsigned int def)
{
	if (
		base.HasMember(prop) &&
		base[prop].IsInt()
		) {
		return base[prop].GetInt();
	}
	else if (
		base.HasMember(prop) &&
		base[prop].IsString()
		) {
		return strtoul(base[prop].GetString(), NULL, 0);
	}
	else {
		return def;
	}
}