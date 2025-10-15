#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "imageSegment.h"
using namespace std;

SegImage::SegImage() {
  width = 0;
  height = 0;
  segments = 0;
  gray = false;
  invert = false;
  flipflop = false;
  image = NULL;
}

SegImage::~SegImage() {
  if (image != NULL)
    cvReleaseImage(&image);
}

MatrixConfusion* SegImage::compareSegImage(SegImage* segImage) {
  int** fusionMatrix = new int*[segments];
  for (int i = 0; i < segments; i++) {
    fusionMatrix[i] = new int[segImage->segments];
    for (int j = 0; j < segImage->segments; j++) {
      fusionMatrix[i][j] = 0;
    }
  }
  uchar* p = (uchar*) image->imageData;
  uchar* q = (uchar*) segImage->image->imageData;
  int i = 0;
  while (i++<image->imageSize) {
    if (*p < segments && *q < segImage->segments)
      fusionMatrix[*p][*q]++;
    q++;
    p++;
  }
  MatrixConfusion* mx = new MatrixConfusion();
  mx->height = segments;
  mx->width = segImage->segments;
  mx->matrix = fusionMatrix;
  mx->imageSeg = filename;
  mx->imageRef = segImage->filename;
  return mx;
}

void SegImage::colorerSegImage(IplImage* imageSource, IplImage* imageDest) {
  CvScalar colors[segments];
  long totals[segments];

  //  IplImage* a = cvCloneImage(imageDest);
  uchar* p = (uchar*) image->imageData;
  uchar* q = (uchar*) imageSource->imageData;
  //  uchar* h = (uchar*) a->imageData;
  int i, j = 0;
  for (i = 0; i < segments; i++) {
    totals[i] = 0;
    colors[i] = cvScalar(0, 0, 0);
  }
  for (i = 0; i < height; i++)
    for (j = 0; j < width; j++) {
      int idx = CV_IMAGE_ELEM(image, uchar, i, j);
      uchar* src = &CV_IMAGE_ELEM(imageSource, uchar, i, j * 3);
      if (idx >= 0 && idx < segments) {
        //uchar* ptr = color_tab->data.ptr + (idx - 1)*3;
        totals[idx]++;
        colors[idx].val[0] = src[0];
        colors[idx].val[1] += src[1];
        colors[idx].val[2] += src[2];
      }
    }


  for (i = 0; i < segments; i++) {

    colors[i].val[0] /= totals[i];
    colors[i].val[1] /= totals[i];
    colors[i].val[2] /= totals[i];

    colors[i].val[0] = 255 - colors[i].val[0];
    colors[i].val[1] = colors[i].val[1];
    colors[i].val[2] = 255 - colors[i].val[2];
  }


  p = (uchar*) image->imageData;
  q = (uchar*) imageDest->imageData;

  i = 0;



  for (i = 0; i < height; i++)
    for (j = 0; j < width; j++) {
      int idx = CV_IMAGE_ELEM(image, uchar, i, j);
      uchar* dst = &CV_IMAGE_ELEM(imageDest, uchar, i, j * 3);
      if (idx == -1)
        dst[0] = dst[1] = dst[2] = (uchar) 255;
      else if (idx < 0 || idx > segments)
        dst[0] = dst[1] = dst[2] = (uchar) 0; // should not get here
      else {

        CvScalar color = colors[idx];
        dst[0] = (int) color.val[0];
        dst[1] = (int) color.val[1];
        dst[2] = (int) color.val[2];

      }
    }

}

void SegImage::graySegImage(IplImage* imageDest) {
  cvCopyImage(image, imageDest);
  cvScale(imageDest, imageDest, 255 / segments);
}

void displayFusionMatrix(int** fusionMatrix, int width, int height) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      cout << fusionMatrix[i][j] << ",";
    }
    cout << endl;
  }
}

string trim(string str) {

  while (str[0] == ' ') {

    str.erase(str.begin());
  }

  while (str[str.size() - 1] == ' ') {

    str.erase(str.end());
  }
  return str;
}

/*
 * Read seg file
 *
format ascii cr
date Fri Mar 9 16:43:30 2001
image 33039
user 1102
width 321
height 481
segments 12
gray 0
invert 0
flipflop 0
data
 */
SegImage* readSegFile(char* fileSeg) {

  char str[2000];
  fstream file_op(fileSeg);

  SegImage* segimage = new SegImage();
  segimage->filename = fileSeg;
  // read header
  while (!file_op.eof()) {
    file_op.getline(str, 2000);
    string a(str);
    if (a[0] == '#')
      continue;

    int pos = a.find(' ');
    string cmd = a.substr(0, pos);
    string strValue = a.substr(pos + 1);
    strValue = trim(strValue);
    if (cmd.compare("format") == 0) {
      pos = strValue.find(' ');
      string ascii = strValue.substr(0, pos);
      if (ascii.compare("ascii") == 0) {
        strValue = strValue.substr(pos + 1);
        strValue = trim(strValue);
        if (strValue[0] == 'c' && strValue[1] == 'r') {
          continue;
        } else {
          throw "Error: Not seg cr file.";
        }

      } else {
        throw "Error: Not seg ascii file.";
      }
    } else if (cmd.compare("date") == 0) {
      continue;
    } else if (cmd.compare("image") == 0) {
      continue;
    } else if (cmd.compare("user") == 0) {
      continue;
    } else if (cmd.compare("width") == 0) {
      pos = strValue.find(' ');
      string strwidth = strValue.substr(0, pos);
      segimage->width = atoi(strwidth.c_str());
    } else if (cmd.compare("height") == 0) {
      pos = strValue.find(' ');
      string strheight = strValue.substr(0, pos);
      segimage->height = atoi(strheight.c_str());
    } else if (cmd.compare("segments") == 0) {
      pos = strValue.find(' ');
      string strsegments = strValue.substr(0, pos);
      segimage->segments = atoi(strsegments.c_str());
    } else if (cmd.compare("gray") == 0) {
      pos = strValue.find(' ');
      string strgray = strValue.substr(0, pos);
      segimage->gray = (strgray[0] == '0') ? false : true;
    } else if (cmd.compare("invert") == 0) {
      pos = strValue.find(' ');
      string strinvert = strValue.substr(0, pos);
      segimage->invert = (strinvert[0] == '0') ? false : true;
    } else if (cmd.compare("flipflop") == 0) {
      pos = strValue.find(' ');
      string strflipflop = strValue.substr(0, pos);
      segimage->flipflop = (strflipflop[0] == '0') ? false : true;
    } else if (cmd.compare("data") == 0) {
      break;
    }

  }
  //
  segimage->image = cvCreateImage(cvSize(segimage->width, segimage->height), IPL_DEPTH_8U, 1);


  // read data
  while (!file_op.eof()) {
    file_op.getline(str, 2000);
    string a(str);
    int pos = a.find(' ');
    string strseg = trim(a.substr(0, pos));
    int seg = atoi(strseg.c_str());

    a = trim(a.substr(pos + 1));
    pos = a.find(' ');
    string strrow = trim(a.substr(0, pos));
    int row = atoi(strrow.c_str());

    a = trim(a.substr(pos + 1));
    pos = a.find(' ');
    string strcol1 = trim(a.substr(0, pos));
    int col1 = atoi(strcol1.c_str());

    a = trim(a.substr(pos + 1));
    pos = a.find(' ');
    string strcol2 = trim(a.substr(0, pos));
    int col2 = atoi(strcol2.c_str());

    int min = col1 < col2 ? col1 : col2;
    int max = col1 > col2 ? col1 : col2;
    for (int i = min; i <= max; i++) {
      cvSetAt(segimage->image, cvScalar(seg), row, i);
    }
  }
  file_op.close();
  return segimage;
}

void saveMatrixConfusion(char* file, MatrixConfusion* mc) {

  ofstream file_op(file);
  file_op << "column;" << mc->imageRef << endl;
  file_op << "row;" << mc->imageSeg << endl;
  for (int i = 0; i < mc->height; i++) {
    for (int j = 0; j < mc->width; j++) {
      file_op << mc->matrix[i][j] << ";";
    }
    file_op << endl;
  }
  file_op.close();

}
