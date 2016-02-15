#include "freerdphelpers.h"
#include <QDebug>

QtContext::QtContext() :
  self(nullptr),
  serverFormats(nullptr),
  cliprdrContext(nullptr),
  clipboardSync(FALSE),
  clipboardText(nullptr)
{
  // This Constructor is never called!

  initQtContext(this);
}

void initQtContext(QtContext* qtc)
{
  qtc->self = nullptr;
  qtc->serverFormats = nullptr;
  qtc->cliprdrContext = nullptr;
  qtc->clipboardSync = FALSE;
  qtc->clipboardText = nullptr;
}

QtContext* getQtContextFromRDPContext(rdpContext* context)
{
  return reinterpret_cast<QtContext*>(context);
}

QtContext* getQtContextFromFreeRDPInstance(freerdp* instance)
{
  if (!instance)
    return nullptr;

  return getQtContextFromRDPContext(instance->context);
}

QtContext* getQtContextFromClipRdrContext(CliprdrClientContext* cliprdrContext)
{
  return reinterpret_cast<QtContext*>(cliprdrContext->custom);
}

QImage::Format bppToImageFormat(int bpp)
{
  switch (bpp) {
    case 16:
      return QImage::Format_RGB16;
    case 24:
      return QImage::Format_RGB888;
    case 32:
      return QImage::Format_RGB32;
  }
  qWarning() << "Cannot handle" << bpp << "bits per pixel!";
  return QImage::Format_Invalid;
}
