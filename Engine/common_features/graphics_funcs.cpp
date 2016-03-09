/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2016 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QPixmap>
#include <QImage>
#include <QRgb>
#include <QFileInfo>
#include <QMessageBox>
#include <QSysInfo>

#include "graphics_funcs.h"
#include <EasyBMP/EasyBMP.h>
extern "C"{
#include <giflib/gif_lib.h>
}

#ifdef DEBUG_BUILD
#include <common_features/logger.h>
#include <QElapsedTimer>
#endif

#include <common_features/file_mapper.h>

#ifdef _WIN32
#define FREEIMAGE_LIB 1
#define DWORD unsigned int //Avoid definition as "unsigned long" while some functions are built as "unsigned int"
#endif
#include <FreeImageLite.h>

#include <QtDebug>

bool GraphicsHelps::initSDLImage()
{
    FreeImage_Initialise();
//    int imgFlags = IMG_INIT_PNG;
//    if( !( IMG_Init( imgFlags ) & imgFlags ) ) {
//        return false;
//    }
    return true;
}

void GraphicsHelps::closeSDLImage()
{
    FreeImage_DeInitialise();
    //IMG_Quit();
}


FIBITMAP* GraphicsHelps::loadImage(QString file, bool convertTo32bit)
{
    #ifdef DEBUG_BUILD
    QElapsedTimer loadingTime;
    QElapsedTimer fReadTime;
    QElapsedTimer imgConvTime;
    loadingTime.start();
    fReadTime.start();
    #endif
    #if  defined(__unix__) || defined(__APPLE__) || defined(_WIN32)
    PGE_FileMapper fileMap;
    if( !fileMap.open_file(file.toUtf8().data()) )
    {
        return NULL;
    }

    FIMEMORY *imgMEM = FreeImage_OpenMemory((unsigned char*)fileMap.data, (unsigned int)fileMap.size);
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileTypeFromMemory(imgMEM);
    if(formato  == FIF_UNKNOWN) { return NULL; }
    FIBITMAP* img = FreeImage_LoadFromMemory(formato, imgMEM, 0);
    FreeImage_CloseMemory(imgMEM);
    fileMap.close_file();
    if(!img) {
        return NULL;
    }
    #else
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(file.toUtf8().data(), 0);
    if(formato  == FIF_UNKNOWN) { return NULL; }
    FIBITMAP* img = FreeImage_Load(formato, file.toUtf8().data());
    if(!img) { return NULL; }
    #endif
    #ifdef DEBUG_BUILD
    int fReadTimeElapsed=fReadTime.elapsed();
    int imgConvertElapsed=0;
    #endif
    if(convertTo32bit)
    {
        #ifdef DEBUG_BUILD
        imgConvTime.start();
        #endif
        FIBITMAP* temp;
        temp = FreeImage_ConvertTo32Bits(img);
        if(!temp) { return NULL; }
        FreeImage_Unload(img);
        img = temp;
        #ifdef DEBUG_BUILD
        imgConvertElapsed=imgConvTime.elapsed();
        #endif
    }

    #ifdef DEBUG_BUILD
    WriteToLog(QtDebugMsg, QString("File read of texture %1 passed in %2 milliseconds").arg(file).arg(fReadTimeElapsed));
    WriteToLog(QtDebugMsg, QString("Conv to 32-bit of %1 passed in %2 milliseconds").arg(file).arg(imgConvertElapsed));
    WriteToLog(QtDebugMsg, QString("Total Loading of image %1 passed in %2 milliseconds").arg(file).arg(loadingTime.elapsed()));
    #endif
//    SDL_Surface* img=IMG_Load( file.toUtf8().data() );
//    if(img)
//    {
//        if(img->format->format!=SDL_PIXELFORMAT_RGBA8888)//!=
//        {
//            SDL_Surface* formattedSurf = SDL_ConvertSurfaceFormat(img,
//                                                                  SDL_PIXELFORMAT_RGBA8888,
//                                                                  0);
//            SDL_FreeSurface(img);
//            return formattedSurf;
//        }
//    } else {
//        qWarning()<<"Failed to load image:" << file << " Error string: "<< IMG_GetError();
//    }
    return img;
}

FIBITMAP* GraphicsHelps::loadImageRC(QString file)
{
    QFile _file(file);
    _file.open(QIODevice::ReadOnly);
    QByteArray data=_file.readAll();
    FIMEMORY x; x.data=data.data();
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileTypeFromMemory( &x, data.size() );
    if(formato  == FIF_UNKNOWN) { return NULL; }
    FIBITMAP* img = FreeImage_LoadFromMemory(formato, &x, 0);
    if(!img) { return NULL; }

    FIBITMAP* temp;
    temp = FreeImage_ConvertTo32Bits(img);
    if(!temp) { return NULL; }
    FreeImage_Unload(img);
    img = temp;

    return img;
}

/*
void GraphicsHelps::putPixel(SDL_Surface * surface, int x, int y, Uint32 color)
{
    if( SDL_MUSTLOCK(surface) )
            SDL_LockSurface(surface);

    Uint8 * pixel = (Uint8*)surface->pixels;
    pixel += (y * surface->pitch) + (x * sizeof(Uint32));
    *((Uint32*)pixel) = color;

    if( SDL_MUSTLOCK(surface) )
        SDL_UnlockSurface(surface);
}


Uint32 GraphicsHelps::getPixel(SDL_Surface *surface, int x, int y)
{
    Uint8 * pixel = (Uint8*)surface->pixels;
    pixel += (y * surface->pitch) + (x * sizeof(Uint32));
    return *((Uint32*)pixel);
}
*/


void GraphicsHelps::mergeWithMask(FIBITMAP *image, QString pathToMask)
{
    if(!image) return;
    if(!QFileInfo(pathToMask).exists()) return; //Nothing to do
    FIBITMAP* mask = loadImage( pathToMask, true );
    if(!mask) return;//Nothing to do

    unsigned int img_w = FreeImage_GetWidth(image);
    unsigned int img_h = FreeImage_GetHeight(image);
    unsigned int mask_w = FreeImage_GetWidth(mask);
    unsigned int mask_h = FreeImage_GetHeight(mask);

    BYTE *img_bits  = FreeImage_GetBits(image);
    BYTE *mask_bits = FreeImage_GetBits(mask);

    BYTE *FPixP=img_bits;
    BYTE *SPixP=mask_bits;

    RGBQUAD Npix = {0x0, 0x0, 0x0, 0xff};   //Destination pixel color
    short newAlpha=255;//Calculated destination alpha-value

    for(unsigned int y=0; (y<img_h) && (y<mask_h); y++ )
    {
        for(unsigned int x=0; (x<img_w) && (x<mask_w); x++ )
        {
            Npix.rgbBlue =  ((SPixP[FI_RGBA_BLUE] & 0x7F) | FPixP[FI_RGBA_BLUE]);
            Npix.rgbGreen = ((SPixP[FI_RGBA_GREEN] & 0x7F) | FPixP[FI_RGBA_GREEN]);
            Npix.rgbRed =   ((SPixP[FI_RGBA_RED] & 0x7F) | FPixP[FI_RGBA_RED]);

            newAlpha= 255-( ( short(SPixP[FI_RGBA_RED])+
                          short(SPixP[FI_RGBA_GREEN])+
                          short(SPixP[FI_RGBA_BLUE]) ) / 3);

            if(  (SPixP[FI_RGBA_RED]>240u) //is almost White
               &&(SPixP[FI_RGBA_GREEN]>240u)
               &&(SPixP[FI_RGBA_BLUE]>240u))
            {
                newAlpha = 0;
            }

            newAlpha += ( ( short(FPixP[FI_RGBA_RED])+
                                   short(FPixP[FI_RGBA_GREEN])+
                                   short(FPixP[FI_RGBA_BLUE])) / 3);

            if(newAlpha > 255) newAlpha=255;
            FPixP[FI_RGBA_BLUE]  = Npix.rgbBlue;
            FPixP[FI_RGBA_GREEN] = Npix.rgbGreen;
            FPixP[FI_RGBA_RED]   = Npix.rgbRed;
            FPixP[FI_RGBA_ALPHA] = (BYTE)newAlpha;
            FPixP+=4; SPixP+=4;
        }
    }
    FreeImage_Unload(mask);
}



QImage GraphicsHelps::setAlphaMask(QImage image, QImage mask)
{
    if(mask.isNull())
        return image;

    if(image.isNull())
        return image;

    QImage target = image;
    QImage newmask = mask;

    if(target.size()!= newmask.size())
    {
        newmask = newmask.copy(0,0, target.width(), target.height());
    }

    newmask.invertPixels();

    target.setAlphaChannel(newmask);

    return target;
}

QImage GraphicsHelps::fromBMP(QString &file)
{
    QImage errImg;

    BMP tarBMP;
    if(!tarBMP.ReadFromFile( file.toLocal8Bit().data() )){
        return errImg; //Check if empty with errImg.isNull();
    }

    QImage bmpImg(tarBMP.TellWidth(),tarBMP.TellHeight(),QImage::Format_RGB666);

    for(int x = 0; x < tarBMP.TellWidth(); x++){
        for(int y = 0; y < tarBMP.TellHeight(); y++){
            RGBApixel pixAt = tarBMP.GetPixel(x,y);
            bmpImg.setPixel(x,y,qRgb(pixAt.Red, pixAt.Green, pixAt.Blue));
        }
    }

    return bmpImg;
}

//QPixmap GraphicsHelps::loadPixmap(QString file)
//{
//    return QPixmap::fromImage(loadQImage(file));
//}


QImage GraphicsHelps::loadQImage(QString file)
{
    QFile img(file);
    if(!img.exists())
        return QImage();
    if(!img.open(QIODevice::ReadOnly))
        return QImage();

    QByteArray mg=img.read(5); //Get magic number!
    if(mg.size()<2) // too short!
        return QImage();
    char *magic = mg.data();
    img.close();

    //Detect PNG 89 50 4e 47
    if( (magic[0]=='\x89')&&(magic[1]=='\x50')&&(magic[2]=='\x4e')&&(magic[3]=='\x47') )
    {
        QImage image = QImage( file );
        return image;
    } else
    //Detect GIF 47 49 46 38
    if( (magic[0]=='\x47')&&(magic[1]=='\x49')&&(magic[2]=='\x46')&&(magic[3]=='\x38') )
    {
        QImage image = QImage( file );
        //QImage image = fromGIF( file );
        return image;
    }
    else
    //Detect BMP 42 4d
    if( (magic[0]=='\x42')&&(magic[1]=='\x4d') )
    {
        QImage image = fromBMP( file );
        return image;
    }

    //Another formats, supported by Qt :P
    QImage image = QImage( file );
    return image;
}

QPixmap GraphicsHelps::squareImage(QPixmap image, QSize targetSize=QSize(0,0) )
{
    QPixmap target = QPixmap(targetSize);
    target.fill(Qt::transparent);
    QPixmap source;

    if( ( targetSize.width() < image.width() ) || ( targetSize.height() < image.height() ))
        source = image.scaled(targetSize, Qt::KeepAspectRatio);
    else
        source = image;

    QPainter p(&target);

    int targetX = qRound( ( ( qreal(target.width()) - qreal(source.width()) ) / 2 ) );
    int targetY = qRound( ( ( qreal(target.height()) - qreal(source.height()) ) / 2 ) );

    p.drawPixmap( targetX, targetY,source.width(),source.height(), source );

    p.end();
    return target;
}

SDL_Surface* GraphicsHelps::QImage_toSDLSurface(const QImage &sourceImage)
{
    // Ensure that the source image is in the correct pixel format
    QImage image = sourceImage;
    if (image.format() != QImage::Format_ARGB32)
        image = image.convertToFormat(QImage::Format_ARGB32);

    // QImage stores each pixel in ARGB format
    // Mask appropriately for the endianness
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 amask = 0x000000ff;
    Uint32 rmask = 0x0000ff00;
    Uint32 gmask = 0x00ff0000;
    Uint32 bmask = 0xff000000;
#else
    Uint32 amask = 0xff000000;
    Uint32 rmask = 0x00ff0000;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x000000ff;
#endif

    return SDL_CreateRGBSurfaceFrom((void*)image.constBits(),
        image.width(), image.height(), image.depth(), image.bytesPerLine(),
        rmask, gmask, bmask, amask);
}

void GraphicsHelps::loadMaskedImage(QString rootDir, QString in_imgName, QString &out_maskName, QString &out_errStr, PGE_Size* imgSize)
{
    if( in_imgName.isEmpty() )
    {
        out_errStr = "Image filename isn't defined";
        return;
    }

    if(!QFile(rootDir+in_imgName).exists())
    {
        out_errStr="image file is not exist: "+rootDir+in_imgName;
        return;
    }

    out_maskName=in_imgName;
    int i = out_maskName.size()-1;
    for( ;i>0; i--)
    {
        if(out_maskName[i]=='.')
        {
            out_maskName.insert(i, 'm');
            break;
        }
    }

    if(i==0)
    {
        out_maskName = "";
    }
    out_errStr = "";

    if(imgSize)
    {
        QImage img = loadQImage(rootDir+in_imgName);
        if(img.isNull())
            out_errStr="Invalid image file!";
        else
            imgSize->setSize(img.width(), img.height());
    }
}


/*********************Code from Qt*********************/

static inline QRgb qt_gl_convertToGLFormatHelper(QRgb src_pixel, GLenum texture_format)
{
    if (texture_format == GL_BGRA) {
        if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            return ((src_pixel << 24) & 0xff000000)
                   | ((src_pixel >> 24) & 0x000000ff)
                   | ((src_pixel << 8) & 0x00ff0000)
                   | ((src_pixel >> 8) & 0x0000ff00);
        } else {
            return src_pixel;
        }
    } else {  // GL_RGBA
        if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            return (src_pixel << 8) | ((src_pixel >> 24) & 0xff);
        } else {
            return ((src_pixel << 16) & 0xff0000)
                   | ((src_pixel >> 16) & 0xff)
                   | (src_pixel & 0xff00ff00);
        }
    }
}

static void convertToGLFormatHelper(QImage &dst, const QImage &img, GLenum texture_format)
{
    Q_ASSERT(dst.depth() == 32);
    Q_ASSERT(img.depth() == 32);

    if (dst.size() != img.size()) {
        int target_width = dst.width();
        int target_height = dst.height();
        qreal sx = target_width / qreal(img.width());
        qreal sy = target_height / qreal(img.height());

        quint32 *dest = (quint32 *) dst.scanLine(0); // NB! avoid detach here
        const uchar *srcPixels = img.constScanLine(img.height() - 1);
        int sbpl = img.bytesPerLine();
        int dbpl = dst.bytesPerLine();

        int ix = int(0x00010000 / sx);
        int iy = int(0x00010000 / sy);

        quint32 basex = int(0.5 * ix);
        quint32 srcy = int(0.5 * iy);

        // scale, swizzle and mirror in one loop
        while (target_height--) {
            const uint *src = (const quint32 *) (srcPixels - (srcy >> 16) * sbpl);
            int srcx = basex;
            for (int x=0; x<target_width; ++x) {
                dest[x] = qt_gl_convertToGLFormatHelper(src[srcx >> 16], texture_format);
                srcx += ix;
            }
            dest = (quint32 *)(((uchar *) dest) + dbpl);
            srcy += iy;
        }
    } else {
        const int width = img.width();
        const int height = img.height();
        const uint *p = (const uint*) img.scanLine(img.height() - 1);
        uint *q = (uint*) dst.scanLine(0);

        if (texture_format == GL_BGRA) {
            if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
                // mirror + swizzle
                for (int i=0; i < height; ++i) {
                    const uint *end = p + width;
                    while (p < end) {
                        *q = ((*p << 24) & 0xff000000)
                             | ((*p >> 24) & 0x000000ff)
                             | ((*p << 8) & 0x00ff0000)
                             | ((*p >> 8) & 0x0000ff00);
                        p++;
                        q++;
                    }
                    p -= 2 * width;
                }
            } else {
                const uint bytesPerLine = img.bytesPerLine();
                for (int i=0; i < height; ++i) {
                    memcpy(q, p, bytesPerLine);
                    q += width;
                    p -= width;
                }
            }
        } else {
            if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
                for (int i=0; i < height; ++i) {
                    const uint *end = p + width;
                    while (p < end) {
                        *q = (*p << 8) | ((*p >> 24) & 0xff);
                        p++;
                        q++;
                    }
                    p -= 2 * width;
                }
            } else {
                for (int i=0; i < height; ++i) {
                    const uint *end = p + width;
                    while (p < end) {
                        *q = ((*p << 16) & 0xff0000) | ((*p >> 16) & 0xff) | (*p & 0xff00ff00);
                        p++;
                        q++;
                    }
                    p -= 2 * width;
                }
            }
        }
    }
}

QImage GraphicsHelps::convertToGLFormat(const QImage& img)
{
    QImage res(img.size(), QImage::Format_ARGB32);
    convertToGLFormatHelper(res, img.convertToFormat(QImage::Format_ARGB32), GL_RGBA);
    return res;
}

/*********************Code from Qt**end****************/


