#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "HuffmanEncode.cpp"
#include "HuffmanDecode.cpp"

using namespace std;

struct ImageHeader {
    BITMAPFILEHEADER bf;    
    BITMAPINFOHEADER bi;
    int rgb[256];
};

int ReadImage(string path, ImageHeader & ih, unsigned char ** & data) {
    FILE * fp;
    fp = fopen(path.c_str(), "rb");
    if (fp == NULL) {
        return 0;
    } 
    
    fread(&ih.bf, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&ih.bi, sizeof(BITMAPINFOHEADER), 1, fp);
    fread(&ih.rgb, sizeof(int), 256, fp);
    
    if (ih.bi.biBitCount != 8) {
        printf("Gray image only!\n");
        return 0;
    }
    
    data = new unsigned char*[ih.bi.biHeight];
    int row_width = ih.bi.biWidth + (4 - ih.bi.biWidth % 4);
    for (int i = 0; i < ih.bi.biHeight; i++) {
        data[i] = new unsigned char[ih.bi.biWidth];
    }
    for (int i = ih.bi.biHeight - 1; i >= 0; i--) {
        for (int j = 0; j < ih.bi.biWidth; j++) {
            fread(&data[i][j], 1, 1, fp);
        }
        if (ih.bi.biWidth % 4 > 0) {
            fseek(fp, 4 - ih.bi.biWidth % 4, SEEK_CUR);
        }
    }
    fclose(fp);
    return 1;
}

int CopyHeader(string path, ImageHeader & ih) {
    FILE * fp;
    fp = fopen(path.c_str(), "wb");
    if (fp == NULL) {
        return 0;
    }
    fwrite(&ih.bf, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&ih.bi, sizeof(BITMAPINFOHEADER), 1, fp);
    fwrite(&ih.rgb, sizeof(int), 256, fp);
    fclose(fp);
    return 1;
}

int ReaderHeader(string path, ImageHeader & ih2) {
    FILE * fp;
    fp = fopen(path.c_str(), "rb");
    if (fp == NULL) {
        return 0;
    }
    fread(&ih2.bf, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&ih2.bi, sizeof(BITMAPINFOHEADER), 1, fp);
    fread(&ih2.rgb, sizeof(int), 256, fp);
    fclose(fp);
    return 1;
}

int WriteImage(string path, ImageHeader & ih2, unsigned char * & decoded_data) {
    FILE * fp;
    fp = fopen(path.c_str(), "wb");
    if (fp == NULL) {
        return 0;
    }
    fwrite(&ih2.bf, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&ih2.bi, sizeof(BITMAPINFOHEADER), 1, fp);
    fwrite(&ih2.rgb, sizeof(int), 256, fp);
    for (int i = ih2.bi.biHeight - 1; i >= 0; i--) {
        fwrite(&decoded_data[i * ih2.bi.biWidth], ih2.bi.biWidth, 1, fp);
        char tmp = 0;
        if (ih2.bi.biWidth % 4 > 0) {
            fwrite(&tmp, 1, 4 - ih2.bi.biWidth % 4, fp);
        }
    }
    fclose(fp);
    return 1;
}

int main() {
    char readpath[50];
    printf("BMP format image name:");
    scanf("%s", readpath);
    ImageHeader ih;
    unsigned char ** data;

    string path = "";
    path = path + readpath + ".bmp";
    if (ReadImage(path, ih, data)) {
        printf("Image %s read successful.\n", readpath);
    } else {
        printf("Image %s reading failed.\n", readpath);
        return 0;
    }

    path = "";
    path = path + readpath + "_head";
    if (CopyHeader(path, ih)) {
        printf("Header file copied.\n");
    } else {
        printf("Header file copying failed.\n");
        return 0;
    }
    
    path = "";
    path = path + readpath;
    HuffmanEncode(data, ih.bi.biHeight, ih.bi.biWidth, path.c_str());
    printf("Image encoded.\n");
    
    path = "";
    path = path + readpath + "_head";
    ImageHeader ih2;
    if (ReaderHeader(path, ih2)) {
        printf("Header file read successful.\n");
    } else {
        printf("Header file reading failed.\n");
        return 0;
    }
    
    path = "";
    path = path + readpath;
    unsigned char * decoded_data = new unsigned char[ih2.bi.biHeight * ih2.bi.biWidth];
    HuffmanDecode(decoded_data, ih2.bi.biHeight, ih2.bi.biWidth, path.c_str());
    printf("Image decoded.\n");
    
    path = "";
    path = path + readpath + "_decode.bmp";
    if (WriteImage(path, ih2, decoded_data)) {
        printf("Decoded image saved successful.\n");
    } else {
        printf("Decoded image saving failed.\n");
        return 0;
    }
}
