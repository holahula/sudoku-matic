
#ifndef SRC_DIGITRECOGNIZER_H_
#define SRC_DIGITRECOGNIZER_H_

#include<opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include<iostream>
#include <opencv/ml.h>
#include <opencv/highgui.h>

using namespace cv;

#define MAX_NUM_IMAGES    60000


class DigitRecognizer { // create a constructor and destructor in the class
public:
    DigitRecognizer();

    ~DigitRecognizer();

    bool train(char* trainPath, char* labelsPath); // takes a path to a data-set of images and a path to its corresponding labels (similar to k-Nearest Neighbors for OpenCV

    int classify (Mat img); //takes an image and returns what digit it is

private:
    Mat preprocessImage(Mat img); //does preprocessing LMAO

    int readFlippedInteger(FILE *fp); //similar to in k-Nearest Neighbours (endianness of process & file-format of dataset used)

private:
     cv::ml::KNearest *knn; // k-Nearest Neighbour data structure
     int numRows, numCols, numImages; // store the # of rows, columns in training dataset
    // numIages stores # of images in dataset
};

#endif /* SRC_DIGITRECOGNIZER_H_ */
