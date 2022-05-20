#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>
#include <algorithm>
using namespace std;
int main()
{

	BITMAPFILEHEADER bmpFile,orgFile;
	BITMAPINFOHEADER bmpInfo,orgInfo;
	FILE* inputFile = NULL,*inputOrg=NULL;
	FILE* outputFile,*ORG_Y_File;
	inputFile = fopen("LR/novel.bmp", "rb");
	inputOrg = fopen("ORG/novel.bmp", "rb");
	outputFile = fopen("output/novel.bmp", "wb");
	ORG_Y_File = fopen("ORG_Y/novel.bmp", "wb");
	fread(&bmpFile, sizeof(BITMAPFILEHEADER), 1, inputFile);
	fread(&bmpInfo, sizeof(BITMAPINFOHEADER), 1, inputFile);
	fread(&orgFile, sizeof(BITMAPFILEHEADER), 1, inputOrg);
	fread(&orgInfo, sizeof(BITMAPINFOHEADER), 1, inputOrg);

	int width = bmpInfo.biWidth;
	int height = bmpInfo.biHeight;
	int size = bmpInfo.biSizeImage;
	int bitCnt = bmpInfo.biBitCount;
	int stride = (((bitCnt / 8) * width) + 3) / 4 * 4;

	printf("W : %d(%d)\nH : %d\nS : %d\nD : %d\n", width, stride, height, size, bitCnt);

	unsigned char* inputImg = NULL, * outputImg = NULL,*orgImg=NULL,*outorgImg=NULL;
	double* Y1, * Y2,*Y3,*Y4,*Y_recon,*Y_org, gx, gy, g, kernel[9] = { 0 },psnr=0.0,mse=0.0;
	

	inputImg = (unsigned char*)malloc(sizeof(unsigned char) * size);
	orgImg = (unsigned char*)malloc(sizeof(unsigned char) * size);
	outputImg = (unsigned char*)malloc(sizeof(unsigned char) * size);
	outorgImg= (unsigned char*)malloc(sizeof(unsigned char) * size);
	Y1 = (double*)malloc(sizeof(double) * width * height);
	Y2 = (double*)malloc(sizeof(double) * width * height);
	Y3 = (double*)malloc(sizeof(double) * width * height);
	Y4 = (double*)malloc(sizeof(double) * width * height);
	Y_recon= (double*)malloc(sizeof(double) * width * height);
	Y_org = (double*)malloc(sizeof(double) * width * height);
	fread(inputImg, sizeof(unsigned char), size, inputFile);
	fread(orgImg, sizeof(unsigned char), size, inputOrg);

	//입력 이미지 데이터 변수에 저장
	for (int j = 0; j < height; j++) 
	{
		for (int i = 0; i < width; i++) 
		{
			Y_recon[j * width + i] = Y1[j * width + i] = 0.299 * inputImg[j * stride + 3 * i + 2] + 0.587 * inputImg[j * stride + 3 * i + 1] + 0.114 * inputImg[j * stride + 3 * i + 0];
			Y_org[j * width + i] = 0.299 * orgImg[j * stride + 3 * i + 2] + 0.587 * orgImg[j * stride + 3 * i + 1] + 0.114 * orgImg[j * stride + 3 * i + 0];
			Y2[j * width + i] = 0;
			Y3[j * width + i] = 0;
			Y4[j * width + i] = 0;
			outorgImg[j * stride + 3 * i + 2] = outorgImg[j * stride + 3 * i + 1] = outorgImg[j * stride + 3 * i + 0] = Y_org[j * width + i];
		}
	}

	//오리지널 이미지의 Y만
	fwrite(&bmpFile, sizeof(BITMAPFILEHEADER), 1, ORG_Y_File);
	fwrite(&bmpInfo, sizeof(BITMAPINFOHEADER), 1, ORG_Y_File);
	fwrite(outorgImg, sizeof(unsigned char), size, ORG_Y_File);
	

	//sobel edge filer
	for (int j = 1; j < height - 1; j++) 
	{
		for (int i = 1; i < width - 1; i++) 
		{
			gx = Y1[(j - 1) * width + i + 1] + 2 * Y1[(j)*width + i + 1] + Y1[(j + 1) * width + i + 1] - Y1[(j - 1) * width + i - 1] - 2 * Y1[(j)*width + i - 1] - Y1[(j + 1) * width + i - 1];
			gy = Y1[(j + 1) * width + i - 1] + 2 * Y1[(j + 1) * width + i] + Y1[(j + 1) * width + i - 1] - Y1[(j - 1) * width + i - 1] - 2 * Y1[(j - 1) * width + i] - Y1[(j - 1) * width + i + 1];
			g = sqrt(gy*gy + gx*gx);
			Y2[j * width + i] = (unsigned char)(g > 255 ? 255 : (g < 0 ? 0 : g));
			Y2[j * width + i] = Y2[j * width + i] > 100 ? 255 : 0;
		}
	}


	//erosion
	for (int j = 1; j < height - 1; j++) 
	{
		for (int i = 1; i < width - 1; i++) 
		{
			for (int y = 0; y < 3; y++)
			{
				for (int x = 0; x < 3; x++) 
				{
					kernel[y * 3 + x] = Y2[(j - 1 + y) * width + i - 1 + x];
					sort(kernel, kernel + 9);
					Y3[j * width + i] = kernel[0];
				}
			}
		}
	}

	//dilation
	for (int j = 1; j < height - 1; j++)
	{
		for (int i = 1; i < width - 1; i++)
		{
			for (int y = 0; y < 3; y++)
			{
				for (int x = 0; x < 3; x++)
				{
					kernel[y * 3 + x] = Y3[(j - 1 + y) * width + i - 1 + x];
					sort(kernel, kernel + 9);
					Y4[j * width + i] = kernel[8];
				}
			}
		}
	}

	//gaussian filter적용, 엣지영역에 있을 경우 해당 픽셀의 비율 8/16, 나머지 경우 4/16

	for (int j = 1; j < height - 1; j++)
	{
		for (int i = 1; i < width - 1; i++)
		{
			Y_recon[j * width + i] = (Y1[(j - 1) * width + i - 1] + 2 * Y1[(j - 1) * width + i] + Y1[(j - 1) * width + i + 1] + 2 * Y1[(j)*width + i - 1] + 4 * Y1[(j)*width + i] +
				2 * Y1[(j)*width + i + 1] + Y1[(j + 1) * width + i - 1] + 2 * Y1[(j + 1) * width + i] + Y1[(j + 1) * width + i + 1]) / 16;
			if (Y4[(j-1) * width + i-1] > 128|| Y4[(j-1) * width + i] > 128 || Y4[(j-1) * width + i+1] > 128 || Y4[j * width + i-1] > 128 || 
				Y4[j * width + i] > 128 || Y4[j * width + i+1] > 128 || Y4[(j+1) * width + i-1] > 128 || Y4[(j+1) * width + i] > 128 || Y4[(j+1) * width + i+1] > 128)
			{
				Y_recon[j * width + i] = (Y1[(j - 1) * width + i - 1] + Y1[(j - 1) * width + i] + Y1[(j - 1) * width + i + 1] +  Y1[(j)*width + i - 1] + 16 * Y1[(j)*width + i] +
					Y1[(j)*width + i + 1] + Y1[(j + 1) * width + i - 1] +  Y1[(j + 1) * width + i] + Y1[(j + 1) * width + i + 1]) / 24;
				Y_recon[j * width + i] = Y1[(j)*width + i];
			}
		}
	}


	//성능 평가
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			mse += (double)((Y_org[j * width + i] - Y_recon[j * width + i]) * (Y_org[j * width + i] - Y_recon[j * width + i]));
		}
	}

	mse /= (width * height);
	psnr = mse != 0.0 ? 10.0 * log10(255 * 255 / mse) : 99.99;
	printf("MSE = %.2lf\nPSNR = %.2lf\n", mse, psnr);



	//출력 이미지 데이터 생성
	for (int j = 1; j < height - 1; j++) 
	{
		for (int i = 1; i < width - 1; i++) 
		{
			outputImg[j * stride + i * 3 + 2] = outputImg[j * stride + i * 3 + 1] = outputImg[j * stride + i * 3 + 0] = Y_recon[j * width + i];
		}
	}


	

	fwrite(&bmpFile, sizeof(BITMAPFILEHEADER), 1, outputFile);
	fwrite(&bmpInfo, sizeof(BITMAPINFOHEADER), 1, outputFile);
	fwrite(outputImg, sizeof(unsigned char), size, outputFile);


	free(outputImg);
	free(inputImg);
	free(Y1);
	free(Y2);
	fclose(inputFile);
	fclose(outputFile);




	return 0;
}