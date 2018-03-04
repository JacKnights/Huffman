#include "mex.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <bitset>
#include <map>

using namespace std;

void decodeString(string & total, map<string, int> & table_map, int total_bit_length, double* res) {
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
            res[index++] = table_map[total.substr(head, cur - head)];
            head = cur;
            cur = head + 1;
        } else {
            cur++;
        }
    }
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    const char *readpath = mxArrayToString(prhs[0]);

    int height = mxGetScalar(prhs[1]);
    int width = mxGetScalar(prhs[2]);
    FILE* fp;
    string table[256];
    string table_path = "table_of_";
    table_path = table_path + readpath;
    fp = fopen(table_path.c_str(), "rb");
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
    int buffer;
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
    double *res;
    plhs[0] = mxCreateDoubleMatrix(height, width, mxREAL);
    res = mxGetPr(plhs[0]);
    decodeString(total, table_map, total_bit_length, res);
}  