#include "IrProtocol.h"
#include "Log.h"

static IrProtocol *irProtocol = NULL;

IrProtocol *IrProtocol::GetInstance()
{
  if (!irProtocol)
  {
    irProtocol = new IrProtocol();
  }
  return irProtocol;
}

IrProtocol::IrProtocol()
{
}

IrProtocol::~IrProtocol()
{
}

void IrProtocol::init()
{
}
