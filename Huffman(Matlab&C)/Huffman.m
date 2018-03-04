img = imread('test.bmp');
%img = rgb2gray(img);
[height, width] = size(img);

mex HuffmanEncode.cpp
HuffmanEncode(int32(img), 'test.txt');

mex HuffmanDecode.cpp
mat = uint8(HuffmanDecode('test.txt', height, width));
imwrite(mat, 'test_decoded.bmp', 'bmp');
