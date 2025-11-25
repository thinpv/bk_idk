#pragma once

using namespace std;

class IrProtocol
{
private:
	IrProtocol();
	virtual ~IrProtocol();

public:
	static IrProtocol *GetInstance();

	void init();
};
