#include "Feature.h"

Feature *Feature::GetInstance()
{
	static Feature *feature = NULL;
	if (!feature)
	{
		feature = new Feature();
	}
	return feature;
}

Feature::Feature()
{
}

void Feature::init()
{
}

bool Feature::CheckData(Json::Value &dataValue, bool &rs)
{
	return false;
}

int Feature::Do(Json::Value &dataValue)
{
	return 1;
}