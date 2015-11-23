#include "freerdpclient.h"
#include "config.h"
#include "freerdpeventloop.h"
#include "freerdphelpers.h"
#include "bitmaprectanglesink.h"
#include "pointerchangesink.h"
#include "rdpqtsoundplugin.h"

#include <freerdp/freerdp.h>
#include <freerdp/input.h>
#include <freerdp/cache/pointer.h>
#include <freerdp/client/channels.h>
#include <freerdp/client/cmdline.h>
#include <freerdp/codec/bitmap.h>
#ifdef Q_OS_UNIX
#include <freerdp/locale/keyboard.h>
#endif

#include <QDebug>
#include <QPainter>
#include <QKeyEvent>
#include <QByteArray>

int FreeRdpClient::instanceCount = 0;

namespace {

UINT16 qtMouseButtonToRdpButton(Qt::MouseButton button) {
    if (button == Qt::LeftButton) {
        return PTR_FLAGS_BUTTON1;
    } else if (button == Qt::RightButton) {
        return PTR_FLAGS_BUTTON2;
    }
    return 0;
}

void* channelAddinLoadHook(LPCSTR pszName, LPSTR pszSubsystem, LPSTR pszType, DWORD dwFlags) {
    QString name = pszName;
    QString subSystem = pszSubsystem;

    if (name == "rdpsnd" && subSystem == "qt") {
        return (void*)RdpQtSoundPlugin::create;
    }
    return freerdp_channels_load_static_addin_entry(pszName, pszSubsystem, pszType, dwFlags);
}

}

BOOL FreeRdpClient::PreConnectCallback(freerdp* instance) {
    freerdp_channels_pre_connect(instance->context->channels, instance);

    rdpSettings* settings = instance->settings;
     BOOL bitmap_cache = settings->BitmapCacheEnabled;
     settings->OrderSupport[NEG_DSTBLT_INDEX] = TRUE;
     settings->OrderSupport[NEG_PATBLT_INDEX] = TRUE;
     settings->OrderSupport[NEG_SCRBLT_INDEX] = TRUE;
     settings->OrderSupport[NEG_OPAQUE_RECT_INDEX] = TRUE;
     settings->OrderSupport[NEG_DRAWNINEGRID_INDEX] = FALSE;
     settings->OrderSupport[NEG_MULTIDSTBLT_INDEX] = FALSE;
     settings->OrderSupport[NEG_MULTIPATBLT_INDEX] = FALSE;
     settings->OrderSupport[NEG_MULTISCRBLT_INDEX] = FALSE;
     settings->OrderSupport[NEG_MULTIOPAQUERECT_INDEX] = TRUE;
     settings->OrderSupport[NEG_MULTI_DRAWNINEGRID_INDEX] = FALSE;
     settings->OrderSupport[NEG_LINETO_INDEX] = TRUE;
     settings->OrderSupport[NEG_POLYLINE_INDEX] = TRUE;
     settings->OrderSupport[NEG_MEMBLT_INDEX] = bitmap_cache;
     settings->OrderSupport[NEG_MEM3BLT_INDEX] = TRUE;
     settings->OrderSupport[NEG_MEMBLT_V2_INDEX] = bitmap_cache;
     settings->OrderSupport[NEG_MEM3BLT_V2_INDEX] = FALSE;
     settings->OrderSupport[NEG_SAVEBITMAP_INDEX] = FALSE;
     settings->OrderSupport[NEG_GLYPH_INDEX_INDEX] = TRUE;
     settings->OrderSupport[NEG_FAST_INDEX_INDEX] = TRUE;
     settings->OrderSupport[NEG_FAST_GLYPH_INDEX] = TRUE;
     settings->OrderSupport[NEG_POLYGON_SC_INDEX] = FALSE;
     settings->OrderSupport[NEG_POLYGON_CB_INDEX] = FALSE;
     settings->OrderSupport[NEG_ELLIPSE_SC_INDEX] = FALSE;
     settings->OrderSupport[NEG_ELLIPSE_CB_INDEX] = FALSE;

     settings->FrameAcknowledge = 10;

     //freerdp_register_addin_provider(freerdp_channels_load_static_addin_entry, 0);
     //freerdp_client_load_addins(instance->context->channels, instance->settings);

     freerdp_channels_pre_connect(instance->context->channels, instance);

     return TRUE;


    emit getMyContext(instance)->self->aboutToConnect();
    return TRUE;
}

BOOL FreeRdpClient::PostConnectCallback(freerdp* instance) {
    auto context = getMyContext(instance);
    auto settings = instance->context->settings;
    auto self = context->self;
    UINT32 gdi_flags;
    pointer_cache_register_callbacks(instance->update);

    if (instance->settings->ColorDepth > 16)
      gdi_flags = CLRBUF_32BPP; //| CLRCONV_ALPHA | CLRCONV_INVERT;
    else
      gdi_flags = CLRBUF_16BPP;

    gdi_init(instance, gdi_flags, NULL);



    rdpPointer pointer;
    memset(&pointer, 0, sizeof(rdpPointer));
    pointer.size = self->pointerChangeSink->getPointerStructSize();
    pointer.New = PointerNewCallback;
    pointer.Free = PointerFreeCallback;
    pointer.Set = PointerSetCallback;
    pointer.SetNull = PointerSetNullCallback;
    pointer.SetDefault = PointerSetDefaultCallback;
    graphics_register_pointer(context->freeRdpContext.graphics, &pointer);

#ifdef Q_OS_UNIX
    // needed for freerdp_keyboard_get_rdp_scancode_from_x11_keycode() to work
    freerdp_keyboard_init(settings->KeyboardLayout);
#endif

    freerdp_channels_post_connect(instance->context->channels, instance);

    emit self->connected();

    return TRUE;
}

void FreeRdpClient::PostDisconnectCallback(freerdp* instance) {
    emit getMyContext(instance)->self->disconnected();
}

int FreeRdpClient::ReceiveChannelDataCallback(freerdp *instance, int channelId,
    BYTE *data, int size, int flags, int total_size) {
    return freerdp_channels_data(instance, channelId, data, size, flags, total_size);
}

BOOL FreeRdpClient::PointerNewCallback(rdpContext *context, rdpPointer *pointer) {
    return getMyContext(context)->self->pointerChangeSink->addPointer(pointer);
}

void FreeRdpClient::PointerFreeCallback(rdpContext *context, rdpPointer *pointer) {
    getMyContext(context)->self->pointerChangeSink->removePointer(pointer);
}

BOOL FreeRdpClient::PointerSetCallback(rdpContext *context, rdpPointer *pointer) {
   return getMyContext(context)->self->pointerChangeSink->changePointer(pointer);
}

BOOL FreeRdpClient::PointerSetNullCallback(rdpContext* context) {
    return TRUE;
}

BOOL FreeRdpClient::PointerSetDefaultCallback(rdpContext* context) {
    return TRUE;
}

BOOL FreeRdpClient::BeginPaintCallback(rdpContext *context) {
#if 0
    rdpGdi* gdi = context->gdi;
    gdi->primary->hdc->hwnd->invalid->null = 1;
    gdi->primary->hdc->hwnd->ninvalid = 0;
#endif


    return TRUE;
}

BOOL FreeRdpClient::EndPaintCallback(rdpContext *context) {
    int i;
    int ninvalid;
    HGDI_RGN cinvalid;
    int x1, y1, x2, y2;
    rdpContext *ctx = (rdpContext*)context;
    rdpSettings* settings = context->instance->settings;
    int length;
    int scanline;
    //UINT8 *dstp;
    //UINT8 *srcp = gdi->primary_buffer;
    auto self = getMyContext(context)->self;
    auto sink = self->bitmapRectangleSink;
    if (!sink)
        return TRUE;

#if 0
    ninvalid = ctx->rdpCtx.gdi->primary->hdc->hwnd->ninvalid;
    if (ninvalid == 0)
    {
        return TRUE;
    }

    cinvalid = ctx->rdpCtx.gdi->primary->hdc->hwnd->cinvalid;

    x1 = cinvalid[0].x;
    y1 = cinvalid[0].y;
    x2 = cinvalid[0].x + cinvalid[0].w;
    y2 = cinvalid[0].y + cinvalid[0].h;


    for (i = 0; i < ninvalid; i++)
    {
        x1 = MIN(x1, cinvalid[i].x);
        y1 = MIN(y1, cinvalid[i].y);
        x2 = MAX(x2, cinvalid[i].x + cinvalid[i].w);
        y2 = MAX(y2, cinvalid[i].y + cinvalid[i].h);
    }
      length = width * bpp;
      scanline = wBuf * bpp;

      srcp = (UINT8*) &srcBuf[(scanline * y) + (x * bpp)];
      dstp = (UINT8*) &dstBuf[(scanline * y) + (x * bpp)];

      for (i = 0; i < height; i++)
      {
        memcpy(dstp, srcp, length);
        srcp += scanline;
        dstp += scanline;
      }
     // copy_pixel_buffer(pixels, gdi->primary_buffer, x, y, width, height, gdi->width, gdi->height, gdi->bytesPerPixel);

#endif
      rdpGdi *gdi = context->gdi;
     QByteArray data = QByteArray((const char *)gdi->primary_buffer, gdi->width*gdi->height*gdi->bytesPerPixel);

    QRect rect(0, 0, gdi->width, gdi->height);
    sink->addRectangle(rect, data);
    emit self->desktopUpdated();

      return TRUE;
}

BOOL FreeRdpClient::BitmapUpdateCallback(rdpContext *context, BITMAP_UPDATE *updates) {
    auto self = getMyContext(context)->self;
    auto sink = self->bitmapRectangleSink;

    if (sink) {
        for (quint32 i = 0; i < updates->number; i++) {
            auto u = &updates->rectangles[i];
            QRect rect(u->destLeft, u->destTop, u->width, u->height);
            QByteArray data;
            Q_ASSERT(u->bitsPerPixel == 16);

            if (u->compressed) {
                data.resize(u->width * u->height * (u->bitsPerPixel / 8));
#if 0 // FIXME
                if (!bitmap_decompress(u->bitmapDataStream, (BYTE*)data.data(), u->width, u->height, u->bitmapLength, u->bitsPerPixel, u->bitsPerPixel)) {
                    qWarning() << "Bitmap update decompression failed";
                }
#endif

            } else {
                data = QByteArray((char*)u->bitmapDataStream, u->bitmapLength);
            }

            sink->addRectangle(rect, data);
        }
        emit self->desktopUpdated();
    }
    return TRUE;
}

FreeRdpClient::FreeRdpClient(PointerChangeSink *pointerSink)
    : freeRdpInstance(nullptr), bitmapRectangleSink(nullptr),
      pointerChangeSink(pointerSink) {

    if (instanceCount == 0) {
        //freerdp_channels_global_init();
        freerdp_register_addin_provider(channelAddinLoadHook, 0);
        //freerdp_wsa_startup();
    }
    instanceCount++;

    loop = new FreeRdpEventLoop(this);
}

FreeRdpClient::~FreeRdpClient() {
    if (freeRdpInstance) {
        freerdp_channels_free(freeRdpInstance->context->channels);
        freerdp_context_free(freeRdpInstance);
        freerdp_free(freeRdpInstance);
        freeRdpInstance = nullptr;
    }

    instanceCount--;
    if (instanceCount == 0) {
        //freerdp_channels_global_uninit();
        //freerdp_wsa_cleanup();
    }
}

void FreeRdpClient::requestStop() {
    loop->quit();
}

void FreeRdpClient::sendMouseMoveEvent(const QPoint &pos) {
    sendMouseEvent(PTR_FLAGS_MOVE, pos);
}

void FreeRdpClient::sendMousePressEvent(Qt::MouseButton button, const QPoint &pos) {
    auto rdpButton = qtMouseButtonToRdpButton(button);
    if (!rdpButton) {
        return;
    }
    sendMouseEvent(rdpButton | PTR_FLAGS_DOWN, pos);
}

void FreeRdpClient::sendMouseReleaseEvent(Qt::MouseButton button, const QPoint &pos) {
    auto rdpButton = qtMouseButtonToRdpButton(button);
    if (!rdpButton) {
        return;
    }
    sendMouseEvent(rdpButton, pos);
}

void FreeRdpClient::sendKeyEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) {
        return;
    }

    bool down = event->type() == QEvent::KeyPress;
    auto code = event->nativeScanCode();
    auto input = freeRdpInstance->input;

#ifdef Q_OS_UNIX
    code = freerdp_keyboard_get_rdp_scancode_from_x11_keycode(code);
#endif

    if (input) {
        freerdp_input_send_keyboard_event_ex(input, down, code);
    }
}

void FreeRdpClient::setBitmapRectangleSink(BitmapRectangleSink *sink) {
    bitmapRectangleSink = sink;
}

quint8 FreeRdpClient::getDesktopBpp() const {
    if (freeRdpInstance && freeRdpInstance->settings) {
        return freeRdpInstance->settings->ColorDepth;
    }
    return 0;
}

void FreeRdpClient::run() {

    initFreeRDP();

    auto context = freeRdpInstance->context;

    if (!freerdp_connect(freeRdpInstance)) {
        qDebug() << "Failed to connect";
        emit disconnected();
        return;
    }

    loop->exec(freeRdpInstance);

    freerdp_channels_close(context->channels, freeRdpInstance);
    freerdp_disconnect(freeRdpInstance);

    if (context->cache) {
        cache_free(context->cache);
    }
    emit disconnected();
		return;
}

void FreeRdpClient::initFreeRDP() {
    if (freeRdpInstance) {
        return;
    }

    freeRdpInstance = freerdp_new();

    freeRdpInstance->ContextSize = sizeof(MyContext);
    freeRdpInstance->ContextNew = nullptr;
    freeRdpInstance->ContextFree = nullptr;
    freeRdpInstance->Authenticate = nullptr;
    freeRdpInstance->VerifyCertificate = nullptr;
    freeRdpInstance->VerifyChangedCertificate = nullptr;
    freeRdpInstance->LogonErrorInfo = nullptr;
    //freeRdpInstance->ReceiveChannelData = ReceiveChannelDataCallback; // FIXME
    freeRdpInstance->PreConnect = PreConnectCallback;
    freeRdpInstance->PostConnect = PostConnectCallback;
    freeRdpInstance->PostDisconnect = PostDisconnectCallback;

    freerdp_context_new(freeRdpInstance);
    getMyContext(freeRdpInstance)->self = this;


    auto update = freeRdpInstance->update;
    update->BitmapUpdate = BitmapUpdateCallback;
    update->EndPaint = EndPaintCallback;
    update->BeginPaint = BeginPaintCallback;

    auto settings = freeRdpInstance->context->settings;
    settings->Username = "bunny";
    settings->Password = "secret";

    settings->SoftwareGdi = TRUE;
    settings->BitmapCacheV3Enabled = TRUE;

    freeRdpInstance->context->channels = freerdp_channels_new();
    // add shared clipboard support
    addStaticChannel(QStringList() << "cliprdr");

/*
    // add sound support
#ifdef WITH_QTSOUND
    // use Qt Multimedia based audio output
    addStaticChannel(QStringList() << "rdpsnd" << "sys:qt");
#else
    // use what FreeRDP provides
    addStaticChannel(QStringList() << "rdpsnd");
#endif
*/
    freerdp_client_load_addins(freeRdpInstance->context->channels, settings);
}

void FreeRdpClient::sendMouseEvent(UINT16 flags, const QPoint &pos) {
    // note that this method is called from another thread, so lots of checking
    // is needed, perhaps we would need a mutex as well?
    if (freeRdpInstance) {
        auto input = freeRdpInstance->input;
        if (input && input->MouseEvent) {
            input->MouseEvent(input, flags, pos.x(), pos.y());
        }
    }
}

void FreeRdpClient::addStaticChannel(const QStringList &args) {
    auto argsArray = new char*[args.size()];
    QList<char*> delList;

    for (int i = 0; i < args.size(); i++) {
        auto d = args[i].toLocal8Bit();
        delList << (argsArray[i] = _strdup(d.data()));
    }

    freerdp_client_add_static_channel(freeRdpInstance->settings, args.size(), argsArray);

    qDeleteAll(delList);
    delete[] argsArray;
}

void FreeRdpClient::setSettingServerHostName(const QString &host) {
    initFreeRDP();
    auto hostData = host.toLocal8Bit();
    auto settings = freeRdpInstance->context->settings;
    free(settings->ServerHostname);
    settings->ServerHostname = _strdup(hostData.data());
}

void FreeRdpClient::setSettingServerPort(quint16 port) {
    initFreeRDP();
    auto settings = freeRdpInstance->context->settings;
    settings->ServerPort = port;
}

void FreeRdpClient::setSettingDesktopSize(quint16 width, quint16 height) {
    initFreeRDP();
    auto settings = freeRdpInstance->settings;
    settings->DesktopWidth = width;
    settings->DesktopHeight = height;
}
