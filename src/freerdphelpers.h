#ifndef MYCONTEXT_H
#define MYCONTEXT_H

class FreeRdpClient;
#include <freerdp/client/cliprdr.h>

#include <QImage>
#include <freerdp/freerdp.h>

struct QtContext
{
    QtContext();
    rdpContext freeRdpContext;
    FreeRdpClient* self;
    BOOL clipboardSync;
    UINT32 numServerFormats;
    UINT32 requestedFormatId;
    CLIPRDR_FORMAT* serverFormats;
    CliprdrClientContext* cliprdrContext;
    UINT32 clipboardCapabilities;
    QString* clipboardText;
};

QtContext* getQtContextFromRDPContext(rdpContext* context);
QtContext* getQtContextFromFreeRDPInstance(freerdp* instance);
QtContext* getQtContextFromClipRdrContext(CliprdrClientContext* cliprdrContext);
void initQtContext(QtContext* qtc);

QImage::Format bppToImageFormat(int bpp);

#endif // MYCONTEXT_H
