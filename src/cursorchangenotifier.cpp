#include "cursorchangenotifier.h"
#include "freerdphelpers.h"
#include <freerdp/freerdp.h>
#include <freerdp/codec/color.h>
#include <QCursor>
#include <QImage>
#include <QPixmap>
#include <QBitmap>
#include <QMetaType>
#include <QMap>
#include <QMutex>

namespace {

struct CursorData {
    CursorData(const QImage &image, const QImage &mask, int hotX, int hotY)
        : image(image), mask(mask), hotX(hotX), hotY(hotY) {
    }

    QImage image;
    QImage mask;
    int hotX;
    int hotY;
};

struct MyPointer {
    rdpPointer pointer;
    int index;
};

MyPointer* getMyPointer(rdpPointer* pointer) {
    return reinterpret_cast<MyPointer*>(pointer);
}

}

class CursorChangeNotifierPrivate {
public:
    CursorChangeNotifierPrivate() : cursorDataIndex(0) {
    }

    QMap<int,CursorData*> cursorDataMap;
    int cursorDataIndex;
    QMutex mutex;
};

CursorChangeNotifier::CursorChangeNotifier(QObject *parent)
    : QObject(parent), d_ptr(new CursorChangeNotifierPrivate) {
}

CursorChangeNotifier::~CursorChangeNotifier() {
    delete d_ptr;
}

void CursorChangeNotifier::qt_freerdp_alpha_cursor_convert(BYTE* alphaData, BYTE* andMask, int width, int height, int bpp)
{
  // This is a modified copy of freerdp_alpha_cursor_convert() located in libfreerdp/codec/color.c
  UINT32 andPixel;
  UINT32 x, y, jj;

  for (y = 0; y < height; y++)
  {
    jj = (bpp == 1) ? y : (height - 1) - y;

    for (x = 0; x < width; x++)
    {
      andPixel = freerdp_get_pixel(andMask, x, jj, width, height, 1);

      if (andPixel)
        freerdp_set_pixel(alphaData, x, y, width, height, 32, 0xFFFFFF);
      else
        freerdp_set_pixel(alphaData, x, y, width, height, 32, 0);
    }
  }
}

BOOL CursorChangeNotifier::addPointer(rdpPointer* pointer) {
    Q_D(CursorChangeNotifier);

    if (!pointer)
        return FALSE;

    QImage ci(pointer->width, pointer->height, QImage::Format_RGB32); // qt uses ARGB internal
    QImage mask(pointer->width, pointer->height, QImage::Format_RGB32);

    if (freerdp_image_copy_from_pointer_data(
          (BYTE*) ci .constBits(), PIXEL_FORMAT_ARGB32,
          pointer->width * 4, 0, 0, pointer->width, pointer->height,
          pointer->xorMaskData, pointer->lengthXorMask,
          pointer->andMaskData, pointer->lengthAndMask,
          pointer->xorBpp, NULL) < 0)
    {
        return FALSE;
    }

    if ((pointer->andMaskData != 0) && (pointer->xorMaskData != 0))
    {
        qt_freerdp_alpha_cursor_convert((BYTE*)mask.constBits(), pointer->andMaskData,
                                        pointer->width, pointer->height, pointer->xorBpp);
    }

    QMutexLocker(&d->mutex);
    d->cursorDataMap[d->cursorDataIndex] = new CursorData(ci, mask, pointer->xPos, pointer->yPos);
    getMyPointer(pointer)->index = d->cursorDataIndex;
    d->cursorDataIndex++;
    return TRUE;
}

void CursorChangeNotifier::removePointer(rdpPointer* pointer) {
    Q_D(CursorChangeNotifier);
    QMutexLocker(&d->mutex);
    delete d->cursorDataMap.take(getMyPointer(pointer)->index);
}

BOOL CursorChangeNotifier::changePointer(rdpPointer* pointer) {
    Q_D(CursorChangeNotifier);
    // pass the changed pointer index from RDP thread to GUI thread because
    // instances of QCursor should not created outside of GUI thread
    int index = getMyPointer(pointer)->index;
    QMetaObject::invokeMethod(this, "onPointerChanged", Q_ARG(int, index));
    return TRUE;
}

void CursorChangeNotifier::onPointerChanged(int index) {
    Q_D(CursorChangeNotifier);
    QMutexLocker(&d->mutex);
    if (d->cursorDataMap.contains(index)) {
        auto data = d->cursorDataMap[index];
        auto imgPixmap = QPixmap::fromImage(data->image);
        auto maskBitmap = QBitmap::fromImage(data->mask);
        imgPixmap.setMask(maskBitmap);

        emit cursorChanged(QCursor(imgPixmap, data->hotX, data->hotY));
    }
}

int CursorChangeNotifier::getPointerStructSize() const {
    return sizeof(MyPointer);
}
