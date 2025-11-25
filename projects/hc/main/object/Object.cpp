#include "Object.h"
#include "Base64.h"

void Object::setData(const string &value)
{
	string dataStr;
	string decode = macaron::Base64::Decode(value, dataStr);
	if (decode == "")
	{
		dataValue.parse(dataStr);
	}
	else
	{
		dataValue.parse(value);
	}

	if (!dataValue.isObject())
	{
		dataValue = Json::objectValue;
	}
	parseDataValue(dataValue);
}