#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>
#include <algorithm>
using namespace std;
int updownsampling()
{

	int threshold = 100;
	double plus = 1.015,minus=0.985;
	BITMAPFILEHEADER bmpFile, bmpFile_down;
	BITMAPINFOHEADER bmpInfo, bmpInfo_down;
	FILE* inputFile = NULL, * outputFile_up, * outputFile_down;
	inputFile = fopen("ORG/ticket.bmp", "rb");
	outputFile_up = fopen("upsample/ticket.bmp", "wb");
	outputFile_down = fopen("downsample/ticket.bmp", "wb");
	fread(&bmpFile, sizeof(BITMAPFILEHEADER), 1, inputFile);
	fread(&bmpInfo, sizeof(BITMAPINFOHEADER), 1, inputFile);

	int width = bmpInfo.biWidth;
	int height = bmpInfo.biHeight;
	int size = bmpInfo.biSizeImage;
	int bitCnt = bmpInfo.biBitCount;
	int stride = (((bitCnt / 8) * width) + 3) / 4 * 4;

	printf("W : %d(%d)\nH : %d\nS : %d\nD : %d\n", width, stride, height, size, bitCnt);

	unsigned char* inputImg = NULL, * outputImg_up = NULL, * outputImg_down;
	double* Y_org, * Y_down, * Y_up, * Y1, * Y2, * Y3, * Y4, mse = 0.0, psnr = 0.0, * upsampling_kernel, sort_kernel[9] = { 0 }, sum, A, B, C, gx, gy, g;
	int ratio = 2;
	int width2 = bmpInfo.biWidth / ratio;
	int height2 = bmpInfo.biHeight / ratio;
	int stride2 = (((bitCnt / 8) * width2) + 3) / 4 * 4;
	int size2 = stride2 * height2;
	inputImg = (unsigned char*)malloc(sizeof(unsigned char) * size);
	outputImg_up = (unsigned char*)malloc(sizeof(unsigned char) * size);
	outputImg_down = (unsigned char*)malloc(sizeof(unsigned char) * size2);
	upsampling_kernel = (double*)malloc(sizeof(double) * ratio * ratio);
	Y_org = (double*)malloc(sizeof(double) * width * height);
	Y_down = (double*)malloc(sizeof(double) * (width2 + 1) * (height2 + 1));
	Y_up = (double*)malloc(sizeof(double) * width * height);
	Y1 = (double*)malloc(sizeof(double) * width * height);
	Y2 = (double*)malloc(sizeof(double) * width * height);
	Y3 = (double*)malloc(sizeof(double) * width * height);

	fread(inputImg, sizeof(unsigned char), size, inputFile);
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			Y1[j * width + i] = Y_org[j * width + i] = 0.299 * inputImg[j * stride + 3 * i + 2] + 0.587 * inputImg[j * stride + 3 * i + 1] + 0.114 * inputImg[j * stride + 3 * i + 0];
			Y2[j * width + i] = 0;
		}
	}

	//modified sobel edge filer
	for (int j = 1; j < height - 1; j++)
	{
		for (int i = 1; i < width - 1; i++)
		{
			gx = Y1[(j - 1) * width + i + 1] + 2 * Y1[(j)*width + i + 1] + Y1[(j + 1) * width + i + 1] - Y1[(j - 1) * width + i - 1] - 2 * Y1[(j)*width + i - 1] - Y1[(j + 1) * width + i - 1];
			gy = Y1[(j + 1) * width + i - 1] + 2 * Y1[(j + 1) * width + i] + Y1[(j + 1) * width + i - 1] - Y1[(j - 1) * width + i - 1] - 2 * Y1[(j - 1) * width + i] - Y1[(j - 1) * width + i + 1];

			if (gy <= (-1) * threshold) Y2[j * width + i] = 0; //À§ÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			if (gy >= threshold)Y2[j * width + i] = 1; //¾Æ·¡ÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			if (gx <= (-1) * threshold) Y2[j * width + i] = 2; //¿ÞÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			if (gx >= threshold)Y2[j * width + i] = 3; //¿À¸¥ÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì

			if (gy <= (-1) * threshold && gx <= (-1) * threshold) Y2[j * width + i] = 5;//À§ÂÊ, ¿ÞÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			if (gy <= (-1) * threshold && gx >= threshold) Y2[j * width + i] = 6; // À§ÂÊ, ¿À¸¥ÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			if (gy >= threshold && gx <= (-1) * threshold) Y2[j * width + i] = 7; //¾Æ·¡ÂÊ, ¿ÞÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			if (gy >= threshold && gx >= threshold) Y2[j * width + i] = 8; //¾Æ·¡ÂÊ, ¿À¸¥ÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
		}
	}




	//Downsampling
	for (int j = 0; j < height2; j++)
	{
		for (int i = 0; i < width2; i++)
		{
			sum = 0.0;
			for (int y = 0; y < ratio; y++)
			{
				for (int x = 0; x < ratio; x++)
				{
					sum += Y_org[(j * ratio + y) * height + i * ratio + x];
				}
			}
			Y_down[j * width2 + i] = (unsigned char)(sum / (ratio * ratio));
			outputImg_down[j * stride2 + i * 3 + 2] = outputImg_down[j * stride2 + i * 3 + 1] = outputImg_down[j * stride2 + i * 3 + 0] = Y_down[j * width2 + i];
		}
	}

	//ÆÐµù
	for (int j = 0; j < height2 + 1; j++)
	{
		for (int i = 0; i < width2 + 1; i++)
		{
			if (i >= width2) Y_down[j * width2 + i] = Y_down[j * width2 + i - 1];
			if (j >= height2) Y_down[j * width2 + i] = Y_down[(j - 1) * width2 + i];
		}
	}


	//Upsampling
	for (int j = 0; j < height2; j++)
	{
		for (int i = 0; i < width2; i++)
		{
			A = Y_down[j * width2 + i];
			B = Y_down[j * width2 + i + 1];
			C = Y_down[(j + 1) * width2 + i];
			for (int y = 0; y < ratio; y++)
			{
				for (int x = 0; x < ratio; x++)
				{
					upsampling_kernel[y * ratio + x] = (2 * (ratio - x) * (ratio - y) * A + x * (2 * ratio - y) * B + y * (2 * ratio - x) * C) / (2 * ratio * ratio);
				}
			}
			for (int y = 0; y < ratio; y++)
			{
				for (int x = 0; x < ratio; x++)
				{
					Y3[(j * ratio + y) * height + i * ratio + x] = Y_up[(j * ratio + y) * height + i * ratio + x] = upsampling_kernel[y * ratio + x];
				}
			}
		}
	}

	for (int j = 1; j < height - 1; j++)
	{
		for (int i = 1; i < width - 1; i++)
		{
			if (Y2[j * width + i] == 0)//À§ÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			{
				Y3[(j - 1) * width + i - 1] *= plus;
				Y3[(j - 1) * width + i] *= plus;
				Y3[(j - 1) * width + i + 1] *= plus;

				Y3[(j + 1) * width + i - 1] *= minus;
				Y3[(j + 1) * width + i] *= minus;
				Y3[(j + 1) * width + i + 1] *= minus;
			}
			else if (Y2[j * width + i] == 1)//¾Æ·¡ÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			{
				Y3[(j + 1) * width + i - 1] *= plus;
				Y3[(j + 1) * width + i] *= plus;
				Y3[(j + 1) * width + i + 1] *= plus;

				Y3[(j - 1) * width + i - 1] *= minus;
				Y3[(j - 1) * width + i] *= minus;
				Y3[(j - 1) * width + i + 1] *= minus;
			}
			else if (Y2[j * width + i] == 2)//¿ÞÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			{
				Y3[(j - 1) * width + i - 1] *= plus;
				Y3[(j)*width + i - 1] *= plus;
				Y3[(j + 1) * width + i - 1] *= plus;

				Y3[(j - 1) * width + i + 1] *= minus;
				Y3[(j)*width + i + 1] *= minus;
				Y3[(j + 1) * width + i + 1] *= minus;
			}
			else if (Y2[j * width + i] == 3)//¿À¸¥ÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			{
				Y3[(j - 1) * width + i - 1] *= plus;
				Y3[(j)*width + i - 1] *= plus;
				Y3[(j + 1) * width + i - 1] *= plus;

				Y3[(j - 1) * width + i + 1] *= minus;
				Y3[(j)*width + i + 1] *= minus;
				Y3[(j + 1) * width + i + 1] *= minus;
			}
			else if (Y2[j * width + i] == 4)//À§ÂÊ, ¿ÞÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			{
				Y3[(j - 1) * width + i - 1] *= plus;
				Y3[(j - 1) * width + i] *= plus;
				Y3[(j - 1) * width + i + 1] *= plus;

				Y3[(j + 1) * width + i - 1] *= minus;
				Y3[(j + 1) * width + i] *= minus;
				Y3[(j + 1) * width + i + 1] *= minus;

				Y3[(j - 1) * width + i - 1] *= plus;
				Y3[(j)*width + i - 1] *= plus;
				Y3[(j + 1) * width + i - 1] *= plus;

				Y3[(j - 1) * width + i + 1] *= minus;
				Y3[(j)*width + i + 1] *= minus;
				Y3[(j + 1) * width + i + 1] *= minus;
			}
			if (Y2[j * width + i] == 5)//À§ÂÊ, ¿À¸¥ÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			{
				Y3[(j - 1) * width + i - 1] *= plus;
				Y3[(j - 1) * width + i] *= plus;
				Y3[(j - 1) * width + i + 1] *= plus;

				Y3[(j + 1) * width + i - 1] *= minus;
				Y3[(j + 1) * width + i] *= minus;
				Y3[(j + 1) * width + i + 1] *= minus;

				Y3[(j - 1) * width + i - 1] *= plus;
				Y3[(j)*width + i - 1] *= plus;
				Y3[(j + 1) * width + i - 1] *= plus;

				Y3[(j - 1) * width + i + 1] *= minus;
				Y3[(j)*width + i + 1] *= minus;
				Y3[(j + 1) * width + i + 1] *= minus;
			}
			else if (Y2[j * width + i] == 6)//¾Æ·¡ÂÊ, ¿ÞÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			{
				Y3[(j + 1) * width + i - 1] *= plus;
				Y3[(j + 1) * width + i] *= plus;
				Y3[(j + 1) * width + i + 1] *= plus;

				Y3[(j - 1) * width + i - 1] *= minus;
				Y3[(j - 1) * width + i] *= minus;
				Y3[(j - 1) * width + i + 1] *= minus;

				Y3[(j - 1) * width + i - 1] *= plus;
				Y3[(j)*width + i - 1] *= plus;
				Y3[(j + 1) * width + i - 1] *= plus;

				Y3[(j - 1) * width + i + 1] *= minus;
				Y3[(j)*width + i + 1] *= minus;
				Y3[(j + 1) * width + i + 1] *= minus;
			}
			else if (Y2[j * width + i] == 7)//¾Æ·¡ÂÊ, ¿À¸¥ÂÊ ÇÈ¼¿ÀÌ ¹àÀº°æ¿ì
			{
				Y3[(j + 1) * width + i - 1] *= plus;
				Y3[(j + 1) * width + i] *= plus;
				Y3[(j + 1) * width + i + 1] *= plus;

				Y3[(j - 1) * width + i - 1] *= minus;
				Y3[(j - 1) * width + i] *= minus;
				Y3[(j - 1) * width + i + 1] *= minus;

				Y3[(j - 1) * width + i - 1] *= plus;
				Y3[(j)*width + i - 1] *= plus;
				Y3[(j + 1) * width + i - 1] *= plus;

				Y3[(j - 1) * width + i + 1] *= minus;
				Y3[(j)*width + i + 1] *= minus;
				Y3[(j + 1) * width + i + 1] *= minus;
			}
		}
	}


	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			Y3[j * width + i] = (unsigned char)(Y3[j * width + i] > 255 ? 255 : (Y3[j * width + i] < 0 ? 0 : Y3[j * width + i]));
			outputImg_up[j * stride + i * 3 + 2] = outputImg_up[j * stride + i * 3 + 1] = outputImg_up[j * stride + i * 3 + 0] = Y3[j * width + i];
		}
	}


	//¼º´ÉÆò°¡
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			mse += (double)((Y_org[j * width + i] - Y_up[j * width + i]) * (Y_org[j * width + i] - Y_up[j * width + i]));
		}
	}

	mse /= (width * height);
	psnr = mse != 0.0 ? 10.0 * log10(255 * 255 / mse) : 99.99;
	printf("\n[Upsampling only] \nMSE = %.2lf\nPSNR = %.2lf\n\n", mse, psnr);
	mse = 0.0;
	psnr = 0.0;

	//¼º´ÉÆò°¡
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			mse += (double)((Y_org[j * width + i] - Y3[j * width + i]) * (Y_org[j * width + i] - Y3[j * width + i]));
		}
	}

	mse /= (width * height);
	psnr = mse != 0.0 ? 10.0 * log10(255 * 255 / mse) : 99.99;
	printf("\n[edge detection] \nMSE = %.2lf\nPSNR = %.2lf\n\n", mse, psnr);
	mse = 0.0;
	psnr = 0.0;




	bmpInfo_down = bmpInfo;
	bmpFile_down = bmpFile;
	bmpInfo_down.biWidth = width2;
	bmpInfo_down.biHeight = height2;
	bmpInfo_down.biSizeImage = size2;
	bmpFile_down.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + size2;

	fwrite(&bmpFile, sizeof(BITMAPFILEHEADER), 1, outputFile_up);
	fwrite(&bmpInfo, sizeof(BITMAPINFOHEADER), 1, outputFile_up);
	fwrite(outputImg_up, sizeof(unsigned char), size, outputFile_up);

	fwrite(&bmpFile_down, sizeof(BITMAPFILEHEADER), 1, outputFile_down);
	fwrite(&bmpInfo_down, sizeof(BITMAPINFOHEADER), 1, outputFile_down);
	fwrite(outputImg_down, sizeof(unsigned char), size2, outputFile_down);


	free(outputImg_up);
	free(outputImg_down);
	free(inputImg);
	fclose(inputFile);
	fclose(outputFile_up);
	fclose(outputFile_down);



	return 0;
}