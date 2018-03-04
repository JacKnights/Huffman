#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <bitset>
#include <map>

using namespace std;

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