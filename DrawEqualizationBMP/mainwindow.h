#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "bmpstruct.h"
#include <iostream>

using namespace std;

namespace Ui {
class MainWindow;
}

class FileBMP
{
private:
    unsigned short read_u16(FILE *fp)
    {
        unsigned char b0, b1;

        b0 = getc(fp);
        b1 = getc(fp);

        return b1 << 8 | b0;
    }


    unsigned int read_u32(FILE *fp)
    {
        unsigned char b0, b1, b2, b3;

        b0 = getc(fp);
        b1 = getc(fp);
        b2 = getc(fp);
        b3 = getc(fp);

        return ((b3 << 8 | b2) << 8 | b1) << 8 | b0;
    }


    int read_s32(FILE *fp)
    {
        unsigned char b0, b1, b2, b3;

        b0 = getc(fp);
        b1 = getc(fp);
        b2 = getc(fp);
        b3 = getc(fp);

        int value = (int)(((b3 << 8 | b2) << 8 | b1) << 8 | b0);
        if (value < 0)
        {
            value = ~value;
            value++;
        }

        return value;
    }

public:

    BMPFILEHEADER header;
    BMPINFOHEADER bmiHeader;

    RGBQ *Palette = new RGBQ[256];

    unsigned char** arrIndexes;

    void init(FILE *pFile)
    {
        // считываем заголовок файла
        header.bfType = read_u16(pFile);
        header.bfSize = read_u32(pFile);
        header.bfReserved1 = read_u16(pFile);
        header.bfReserved2 = read_u16(pFile);
        header.bfOffBits = read_u32(pFile);

        // считываем заголовок изображения
        bmiHeader.biSize = read_u32(pFile);
        bmiHeader.biWidth = read_s32(pFile);
        bmiHeader.biHeight = read_s32(pFile);
        bmiHeader.biPlanes = read_u16(pFile);
        bmiHeader.biBitCount = read_u16(pFile);
        bmiHeader.biCompression = read_u32(pFile);
        bmiHeader.biSizeImage = read_u32(pFile);
        bmiHeader.biXPelsPerMeter = read_s32(pFile);
        bmiHeader.biYPelsPerMeter = read_s32(pFile);
        bmiHeader.biClrUsed = read_u32(pFile);
        bmiHeader.biClrImportant = read_u32(pFile);

        for (int i = 0; i < 256; i++)
        {
            Palette[i].rgbBlue = getc(pFile);
            Palette[i].rgbGreen = getc(pFile);
            Palette[i].rgbRed = getc(pFile);
            Palette[i].rgbReserved = getc(pFile);
        }

        arrIndexes = new unsigned char*[bmiHeader.biHeight];
        for (int i = 0; i < bmiHeader.biHeight; i++)
            arrIndexes[i] = new unsigned char[bmiHeader.biWidth];

        for (int i = 0; i < bmiHeader.biHeight; i++) {
            for (int j = 0; j < bmiHeader.biWidth; j++) {
                arrIndexes[i][j] = static_cast<unsigned char>(getc(pFile));
            }
            // пропускаем последний байт в строке
            // getc(pFile);
            // getc(pFile);
        }

        if (bmiHeader.biSizeImage == 0)
            bmiHeader.biSizeImage = bmiHeader.biWidth*bmiHeader.biHeight;
    }


    int get_Width() { return bmiHeader.biWidth; }
    int get_Height() { return bmiHeader.biHeight; }
    uint get_SizeImage() { return bmiHeader.biSizeImage; }

};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void Message();
    FileBMP* fBMP;
    QImage paint_image(unsigned char** data, int width, int height);
    QImage paint_hist(unsigned char** data, int width, int height);
    QImage paint_equliz_hist(unsigned char** data, int width, int height);
    QImage paint_equliz_image(unsigned char** data, int width, int height);

protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
