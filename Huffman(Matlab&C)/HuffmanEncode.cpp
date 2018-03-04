#include "mex.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <bitset>

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

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    double* dataCursor = mxGetPr(prhs[0]);
    int height = mxGetM(prhs[0]);
    int width = mxGetN(prhs[0]);
    char *writepath = mxArrayToString(prhs[1]);
    FILE* fp;
    int * data = (int *)dataCursor;
    int counts[256];
    memset(counts, 0, sizeof(int) * 256);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            counts[data[i + height * j]]++;
        }
    }

    Node nodes[256 * 2];
    int length = buildTree(nodes, counts);

    string table[256];
    buildTable(nodes, length - 1, "", table);
    string table_path = "table_of_";
    table_path = table_path + writepath;
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
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            for (int k = 0; k < table[data[height * i + j]].size(); k++) {
                str[cur] = table[data[height * i + j]][k];
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