#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <bitset>
#include <map>

using namespace std;

struct Node {
    int count;
    int left;
    int right;
    int removed;
};

int findMinNode(Node* nodes, int length) {
    int index = -1;
    for (int i = 0; i < length; i++) {
        if ((index == -1 || nodes[i].count < nodes[index].count) && !nodes[i].removed && nodes[i].count > 0) {
            index = i;
        }
    }
    if (index != -1) {
        nodes[index].removed = 1;
    }
    return index;
}

int buildTree(Node* nodes, int* counts) {
    for (int i = 0; i < 256; i++) {
        nodes[i].left = -1;
        nodes[i].right = -1;
        nodes[i].count = counts[i];
        nodes[i].removed = 0;
    }
    int length = 256;

    while (1) {
        int l = findMinNode(nodes, length);
        if (l == -1) {
            break;
        }
        int r = findMinNode(nodes, length);
        if (r == -1) {
            break;
        }
        nodes[length].left = l;
        nodes[length].right = r;
        nodes[length].count = nodes[l].count + nodes[r].count;
        nodes[length].removed = 0;
        length++;
    }
    return length;
}

void buildTable(Node* nodes, int pos, string bits, string * table) {
    int l = nodes[pos].left;
    int r = nodes[pos].right;
    if (nodes[pos].left == -1 && nodes[pos].right == -1) {
        table[pos] = bits;
        return;
    }
    buildTable(nodes, r, bits + "1", table);
    buildTable(nodes, l, bits + "0", table);
}

void HuffmanEncode(unsigned char ** data, int height, int width, const char *writepath) {
    FILE* fp;
    int counts[256];
    memset(counts, 0, sizeof(int) * 256);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            counts[data[i][j]]++;
        }
    }

    Node nodes[256 * 2];
    int length = buildTree(nodes, counts);

    string table[256];
    buildTable(nodes, length - 1, "", table);
    string table_path = "";
    table_path = table_path + writepath + "_table";
    fp = fopen(table_path.c_str(), "w");
    for (int i = 0; i < 256; i++) {
        if (table[i].size() == 0) {
            fprintf(fp, "2\n");
        } else  {
            fprintf(fp, "%s\n", table[i].c_str());
        }
    }
    fclose(fp);

    int total_bit_length = 0;
    for (int i = 0; i < 256; i++) {
        total_bit_length += counts[i] * table[i].size();
    }
    char * str = new char[total_bit_length];
    int cur = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for (int k = 0; k < table[data[i][j]].size(); k++) {
                str[cur] = table[data[i][j]][k];
                cur++;
            }
            
        }
    }

    fp = fopen(writepath, "wb");
    int times = total_bit_length / 32 + 1;
    string total = "";
    total = total + str;
    for (int i = 0; i < 32 * times - total_bit_length; i++) {
        total = total + "0";
    }
    fwrite(&total_bit_length, sizeof(int), 1, fp);
    for (int i = 0; i < times; i++) {
        bitset<32> byte(total.substr(32 * i, 32));
        
        unsigned long tmp = byte.to_ulong();
        fwrite(&tmp, sizeof(int), 1, fp);
    }
    fclose(fp);
}

void decodeString(string & total, map<string, int> & table_map, int total_bit_length, unsigned char * data) {
    int index = 0;
    int cur = 1;
    int head = 0;
    while (head < total_bit_length) {
        if (total[head] != '1' && total[head] != '0') {
            head++;
            cur = head + 1;
            continue;
        }
        if (table_map.count(total.substr(head, cur - head))) {
            data[index++] = table_map[total.substr(head, cur - head)];
            head = cur;
            cur = head + 1;
        } else {
            cur++;
        }
    }
}

void HuffmanDecode(unsigned char * decoded_data, int height, int width, const char *readpath) {
    FILE* fp;
    string table[256];
    string path = "";
    path = path + readpath + "_table";
    fp = fopen(path.c_str(), "rb");
    for (int i = 0; i < 256; i++) {
        char tmp[30];
        fscanf(fp, "%s", tmp);
        table[i] = table[i] + tmp;
    }
    fclose(fp);

    map<string, int> table_map;
    for (int i = 0; i < 256; i++) {
        table_map[table[i]] = i;
    }

    fp = fopen(readpath, "rb");
    int total_bit_length;
    fread(&total_bit_length, sizeof(int), 1, fp);
    int times = total_bit_length / 32 + 1;
    string total = "";
    char * str = new char[total_bit_length];
    int *words = new int[times];
    fread(words, sizeof(int), times, fp);
    int cur = 0;
    for (int i = 0; i < times; i++) {
        bitset<32> bits(words[i]);
        string tmp = bits.to_string();
        for (int j = 0; j < 32; j++) {
            str[cur] = tmp[j];
            cur++;
        }
    }
    fclose(fp);
    
    total = total + str;
    decodeString(total, table_map, total_bit_length, decoded_data);
}

int main() {
	char readpath[50];
	printf("BMP format image name:");
	scanf("%s", readpath);
	string path = "";
	path = path + readpath + ".bmp";
	FILE * fp;
	fp = fopen(path.c_str(), "rb");
	if (fp == NULL) {
		printf("Image open error!\n");
		return 0;
	} 
	
	BITMAPFILEHEADER bf;	
	BITMAPINFOHEADER bi;
	fread(&bf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&bi, sizeof(BITMAPINFOHEADER), 1, fp);
	int * buffer = new int[256];
	fread(buffer, sizeof(int), 256, fp);
	
	if (bi.biBitCount != 8) {
		printf("Gray image only!\n");
		return 0;
	}
	
	unsigned char ** data = new unsigned char*[bi.biHeight];
	int row_width = bi.biWidth + (4 - bi.biWidth % 4);
	for (int i = 0; i < bi.biHeight; i++) {
		data[i] = new unsigned char[bi.biWidth];
	}
	for (int i = bi.biHeight - 1; i >= 0; i--) {
		for (int j = 0; j < bi.biWidth; j++) {
			fread(&data[i][j], 1, 1, fp);
		}
		if (bi.biWidth % 4 > 0) {
			fseek(fp, 4 - bi.biWidth % 4, SEEK_CUR);
		}
	}
	fclose(fp);
	printf("Image %s read successful.\n", readpath);
	
	path = "";
	path = path + readpath + "_head";
	fp = fopen(path.c_str(), "wb");
	fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, fp);
	fwrite(buffer, sizeof(int), 256, fp);
	fclose(fp);
	printf("Header file saved.\n");
	
	path = "";
	path = path + readpath;
	HuffmanEncode(data, bi.biHeight, bi.biWidth, path.c_str());
	printf("Image encoded.\n");
	
	path = "";
	path = path + readpath + "_head";
	fp = fopen(path.c_str(), "rb");
	BITMAPFILEHEADER bf2;	
	BITMAPINFOHEADER bi2;
	fread(&bf2, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&bi2, sizeof(BITMAPINFOHEADER), 1, fp);
	int * buffer2 = new int[256];
	fread(buffer2, sizeof(int), 256, fp);
	fclose(fp);
	
	path = "";
	path = path + readpath;
	unsigned char * decoded_data = new unsigned char[bi2.biHeight * bi2.biWidth];
	HuffmanDecode(decoded_data, bi2.biHeight, bi2.biWidth, path.c_str());
	printf("Image decoded.\n");
	
	path = "";
	path = path + readpath + "_decode.bmp";
	fp = fopen(path.c_str(), "wb");
	fwrite(&bf2, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite(&bi2, sizeof(BITMAPINFOHEADER), 1, fp);
	fwrite(buffer2, sizeof(int), 256, fp);
	for (int i = bi2.biHeight - 1; i >= 0; i--) {
		fwrite(&decoded_data[i * bi2.biWidth], bi2.biWidth, 1, fp);
		char tmp = 0;
		if (bi2.biWidth % 4 > 0) {
			fwrite(&tmp, 1, 4 - bi2.biWidth % 4, fp);
		}
	}
	fclose(fp);
	printf("Decoded image saved.\n");
}
