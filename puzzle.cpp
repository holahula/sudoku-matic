#include<opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include<iostream>
#include <opencv/ml.h>
#include <opencv/highgui.h>
#include "digitrecognizer.h"
#include <sstream>

using namespace std;
using namespace cv;
using namespace cv::ml;

Mat thresh(Mat img)
{
	cvtColor(img,img,CV_BGR2GRAY);

	Mat img_t=Mat::zeros(img.size(),CV_8UC3);
	adaptiveThreshold(img,img_t,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY_INV,5,10);
	return img_t;
}

Mat grid_extract(Mat img)
{
	int index;
	double max;
	Mat grid;
	grid=Mat::zeros(img.size(),CV_8UC1);
	vector<vector<Point> > contour;
	vector<Vec4i> h;
	vector<Point> req;

	findContours(img,contour,h,CV_RETR_TREE,CV_CHAIN_APPROX_SIMPLE,Point(0,0));

	max=contourArea(contour[0]);
	for(int i=0;i<contour.size();i++)
	{
		double temp;
		temp=contourArea(contour[i]);
		if(max<temp)
		{
			max=temp;
			index=i;
			req=contour[i];
		}
	}

	drawContours(grid,contour,index,Scalar(255,255,255),CV_FILLED,8,h);
	//namedWindow("Grid",0);
	//imshow("Grid",grid);
	//waitKey(0);
	return grid(boundingRect(req));
}
//Function to remove the lines from the grid(To seperate out digits from the grid)
Mat hough(Mat img)
{
	vector<Vec4i> lines;
	HoughLinesP(img,lines,1,CV_PI/180,100,30,10);
	for(int i=0; i<lines.size();i++)
	{
		Vec4i l=lines[i];
		line(img,Point(l[0],l[1]),Point(l[2],l[3]),Scalar(0,0,0),10,CV_AA);
	}
	//imshow("Digits",img);
	//waitKey(0);
	return img;
}

void digit_extract(Mat img)
{
	char key = 0;
    Mat digit;
    vector<vector<Point> > contour;
    vector<Vec4i> h;

    findContours(img,contour,h,CV_RETR_TREE,CV_CHAIN_APPROX_SIMPLE,Point(0,0));
    for(int i=0;i<81;i++)
    {
        vector<Point> temp;
        temp=contour[i];
        img(boundingRect(temp)).copyTo(digit);
        namedWindow("Digit",0);

        std::ostringstream name;
        name << "photo" << i << ".png";
        cv::imwrite(name.str(), digit);

        /*
        imshow("Digit",digit);
        waitKey(500);
        */
    }
}
Mat DigitRecognizer::preprocessImage(Mat img)
{
	int rowTop=-1, rowBottom=-1, colLeft=-1, colRight=-1;

	Mat temp;
	int thresholdBottom = 50;
	int thresholdTop = 50;
	int thresholdLeft = 50;
	int thresholdRight = 50;
	int center = img.rows/2;
	for(int i=center;i<img.rows;i++)
	{
		if(rowBottom==-1)
		{
			temp = img.row(i);
			IplImage stub = temp;
			if(cvSum(&stub).val[0] < thresholdBottom || i==img.rows-1)
				rowBottom = i;

		}

		if(rowTop==-1)
		{
			temp = img.row(img.rows-i);
			IplImage stub = temp;
			if(cvSum(&stub).val[0] < thresholdTop || i==img.rows-1)
				rowTop = img.rows-i;

		}

		if(colRight==-1)
		{
			temp = img.col(i);
			IplImage stub = temp;
			if(cvSum(&stub).val[0] < thresholdRight|| i==img.cols-1)
				colRight = i;

		}

		if(colLeft==-1)
		{
			temp = img.col(img.cols-i);
			IplImage stub = temp;
			if(cvSum(&stub).val[0] < thresholdLeft|| i==img.cols-1)
				colLeft = img.cols-i;
		}
	}

	Mat newImg;

	newImg = newImg.zeros(img.rows, img.cols, CV_8UC1);

	int startAtX = (newImg.cols/2)-(colRight-colLeft)/2;

	int startAtY = (newImg.rows/2)-(rowBottom-rowTop)/2;

	for(int y=startAtY;y<(newImg.rows/2)+(rowBottom-rowTop)/2;y++)
	{
		uchar *ptr = newImg.ptr<uchar>(y);
		for(int x=startAtX;x<(newImg.cols/2)+(colRight-colLeft)/2;x++)
		{
			ptr[x] = img.at<uchar>(rowTop+(y-startAtY),colLeft+(x-startAtX));
		}
	}

	Mat cloneImg = Mat(numRows, numCols, CV_8UC1);

	resize(newImg, cloneImg, Size(numCols, numRows));

	// Now fill along the borders
	for(int i=0;i<cloneImg.rows;i++)
	{
		floodFill(cloneImg, cvPoint(0, i), cvScalar(0,0,0));

		floodFill(cloneImg, cvPoint(cloneImg.cols-1, i), cvScalar(0,0,0));

		floodFill(cloneImg, cvPoint(i, 0), cvScalar(0));
		floodFill(cloneImg, cvPoint(i, cloneImg.rows-1), cvScalar(0));
	}
	cloneImg = cloneImg.reshape(1, 1);

	return cloneImg;
}

void drawLine(Vec2f line, Mat &img, Scalar rgb = CV_RGB(0,0,255)) //BASED ON HOUGH TRANSFORM DRAWS LINE FOR EVERY POINT
{
	if(line[1]!=0)
	{
		float m = -1/tan(line[1]);

		float c = line[0]/sin(line[1]);

		cv::line(img, Point(0, c), Point(img.size().width, m*img.size().width+c), rgb);
	}
	else
	{
		cv::line(img, Point(line[0], 0), Point(line[0], img.size().height), rgb);
	}

}

void mergeRelatedLines(vector<Vec2f> *lines, Mat &img)// averages nearby lines, lines within a certain distance will merge together
{
	vector<Vec2f>::iterator current; // helps traverse the array list, every element in array has 2 things: rho & theta (normal form of line)

	for(current=lines->begin();current!=lines->end();current++){ //
		if ((*current)[0]==0 && (*current)[1]==-100) continue; // mark lines that have been fused (set rho = 0 and theta = -100) so will skip merged lines

		float p1 = (*current)[0]; // store rho
		float theta1 = (*current)[1]; // store theta

		Point pt1current, pt2current; // finds 2 points on the line FOR LINE CURRENT
		if (theta1>CV_PI*45/180 && theta1<CV_PI*135/180){ // if the line is horizontal, finds a point at extreme left and extreme right
			pt1current.x=0;

			pt1current.y = p1/sin(theta1);

			pt2current.x=img.size().width;
			pt2current.y=-pt2current.x/tan(theta1) + p1/sin(theta1);
		} else { // if line is vertical, finds a point at top and bottom
			pt1current.y=0;
			pt1current.x=p1/cos(theta1);
			pt2current.y=img.size().height;
			pt2current.x=-pt2current.y/tan(theta1) + p1/cos(theta1);
		}

		vector<Vec2f>::iterator    pos;

		for (pos=lines->begin();pos!=lines->end();pos++){ // loops to compare every line with every other line;
			if (*current==*pos) continue; // if current = pos, the line is the same and there is no point fusing the same line

			if (fabs((*pos)[0]-(*current)[0])<20 && fabs((*pos)[1]-(*current)[1])<CV_PI*10/180) { // check if lines are w/in certain distance
				float p = (*pos)[0]; // store rho for line pos
				float theta = (*pos)[1]; // store theta for line pos
				Point pt1, pt2;

				if ((*pos)[1]>CV_PI*45/180 && (*pos)[1]<CV_PI*135/180) { // find 2 points on the line pos
					pt1.x=0; // pos is a horizontal line
					pt1.y = p/sin(theta);
					pt2.x=img.size().width;
					pt2.y=-pt2.x/tan(theta) + p/sin(theta);
				} else { // pos is a vertical line
					pt1.y=0;
					pt1.x=p/cos(theta);
					pt2.y=img.size().height;
					pt2.x=-pt2.y/tan(theta) + p/cos(theta);
				}

				if(((double)(pt1.x-pt1current.x)*(pt1.x-pt1current.x) + (pt1.y-pt1current.y)*(pt1.y-pt1current.y)<64*64) &&
						((double)(pt2.x-pt2current.x)*(pt2.x-pt2current.x) + (pt2.y-pt2current.y)*(pt2.y-pt2current.y)<64*64)) {
					// if the endpoints of pos and current are close, FUSION HA

					(*current)[0] = ((*current)[0]+(*pos)[0])/2; // MERGE THE 2 LINES (X COORD)

					(*current)[1] = ((*current)[1]+(*pos)[1])/2; // MERGE THE 2 LINES (Y COORD)

					(*pos)[0]=0;
					(*pos)[1]=-100;
				}
			}
		}
	}
}


int main(int, char**) {
	// JASMINE's CODE TO TAKE A PHOTO
	VideoCapture cap(0);
	if(!cap.isOpened())  // check if we succeeded
		return -1;

	Mat sudokuB;
	cap >> sudokuB; //
	char key = 0;

	while (key != 27 && cap.isOpened()) {
		bool Frame = cap.read(sudokuB);
		if (!Frame || sudokuB.empty()) {
			cout << "error: frame not read from webcam\n";
			break;
		}
		namedWindow("sudokuB", CV_WINDOW_NORMAL);
		imshow("imgOriginal", sudokuB);
		key = waitKey(1);
	}

	imwrite("sudoku.jpg", sudokuB);// saves the photo as a jpeg.

	// END OF JASMINE's CODE

	int size = 16 * 16;

	Mat sudoku = imread("sudoku.jpg", 0); // loads a static image for detecting a photo

	Mat original = sudoku.clone(); // clones the sudoku (original photo)


	Mat outerBox = Mat(sudoku.size(), CV_8UC1); // blank image of same size, will hold the actual outer box of the photo

	GaussianBlur(sudoku, sudoku, Size(11,11), 0); // blurs the image to smooth out the noise and makes extracting grid lines easier

	adaptiveThreshold(sudoku, outerBox, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 5, 2); // thresholds image; adaptive is good b/c image can have varying levels of light

	// calculates mean over 5 x 5 box, then -2 from mean the level for every pixel

	bitwise_not(outerBox, outerBox); // inverts the image (borders are white + other noise)

	Mat kernel = (Mat_<uchar>(3,3) << 0,1,0,1,1,1,0,1,0);

	dilate(outerBox, outerBox, kernel);
	// dilate the image again to fill in "cracks" between lines (here a + shaped kernel matrix is used)

	// imwrite("sudokuA.jpg", outerBox);


	int count = 0;
	int max=-1;

	Point maxPt;

	for(int y=0;y<outerBox.size().height;y++)
	{
		uchar *row = outerBox.ptr(y);
		for(int x=0;x<outerBox.size().width;x++){
			if(row[x]>=128){ // ensures only white parts are filtered{
				int area = floodFill(outerBox, Point(x,y), CV_RGB(0,0,64)); // returns a bounding rectangle of pixels it filled

				if(area>max)
				{
					maxPt = Point(x,y); //since biggest blob is puzzle, will have biggest bounding box (saves location where fill done)
					max = area;
				}
			}
		}
	}

	floodFill(outerBox, maxPt, CV_RGB(255,255,255));//fill the blob with the max area with white

	for(int y=0;y<outerBox.size().height;y++) // turn the other blobs BLACK
	{
		uchar *row = outerBox.ptr(y);
		for(int x=0;x<outerBox.size().width;x++)
		{
			if(row[x]==64 && x!=maxPt.x && y!=maxPt.y)
			{
				int area = floodFill(outerBox, Point(x,y), CV_RGB(0,0,0)); // if a dark gray point is encountered, makes it BLACK
			}
		}
	}
	//imwrite("thresholded.jpg", outerBox);

	erode(outerBox, outerBox, kernel); // b/c image was dilated a bit it is restored a bit by eroding it
	//imwrite("eroded.jpg", outerBox);


	vector<Vec2f> lines;
	HoughLines(outerBox, lines, 1, CV_PI/180, 200); // hough transform
	mergeRelatedLines(&lines, sudoku); // MERGE RELATED LINES

	for(int i=0;i<lines.size();i++)
	{
		drawLine(lines[i], outerBox, CV_RGB(0,0,128)); // draws line based on the hough transform to see if the lines are good enough
	}

	// mergeRelatedLines(&lines, sudoku); // Add this line
	imwrite("sudokuA.jpg", outerBox);

	// FIND EXTREME LINES

	// set lines as ridiculous values to ensure the edges of the grid can be found

	Vec2f topEdge = Vec2f(1000,1000);
	double topYIntercept=100000, topXIntercept=0;

	Vec2f bottomEdge = Vec2f(-1000,-1000);
	double bottomYIntercept=0, bottomXIntercept=0;

	Vec2f leftEdge = Vec2f(1000,1000);
	double leftXIntercept=100000, leftYIntercept=0;

	Vec2f rightEdge = Vec2f(-1000,-1000);
	double rightXIntercept=0, rightYIntercept=0;

	for(int i=0;i<lines.size();i++){
		Vec2f current = lines[i];

		float p=current[0]; // store rho
		float theta=current[1]; // store theta

		if(p==0 && theta== -100) continue; // if current = a merged line, skip

		double xIntercept, yIntercept; // calculate x and y intercepts (where lines intersect x and y axis)
		xIntercept = p/cos(theta);
		yIntercept = p/(cos(theta)*sin(theta));

		if (theta>CV_PI*80/180 && theta<CV_PI*100/180){ // if line is vertical
			if(p<topEdge[0]) topEdge = current;
			if(p>bottomEdge[0]) bottomEdge = current;
		} else if (theta<CV_PI*10/180 || theta>CV_PI*170/180) {
			if (xIntercept>rightXIntercept) {
				rightEdge = current;
				rightXIntercept = xIntercept;
			} else if (xIntercept<=leftXIntercept) {
				leftEdge = current;
				leftXIntercept = xIntercept;
			}
		}
	}// all lines other than vertical and horizontal are IGNORED

	// just to visualize the extreme lines
	drawLine(topEdge, sudoku, CV_RGB(0,0,0));
	drawLine(bottomEdge, sudoku, CV_RGB(0,0,0));
	drawLine(leftEdge, sudoku, CV_RGB(0,0,0));
	drawLine(rightEdge, sudoku, CV_RGB(0,0,0));

	// calculate intersection of these 4 lines
	Point left1, left2, right1, right2, bottom1, bottom2, top1, top2;

	// finds 2 points on each line, then calculates where any 2 lines intersect
	// right and left edges need 'if' statements to distinguish (vertical lines) can have infinite slope (computer rip)
	// if slope is incalculable, aka vertical line, calculates 'safely'
	//
	int height=outerBox.size().height;
	int width=outerBox.size().width;

	if(leftEdge[1]!=0) {
		left1.x=0;
		left1.y=leftEdge[0]/sin(leftEdge[1]);

		left2.x=width;
		left2.y=-left2.x/tan(leftEdge[1]) + left1.y;
	} else {
		left1.y=0;
		left1.x=leftEdge[0]/cos(leftEdge[1]);

		left2.y=height;
		left2.x=left1.x - height*tan(leftEdge[1]);
	}

	if (rightEdge[1]!=0) {
		right1.x=0;
		right1.y=rightEdge[0]/sin(rightEdge[1]);

		right2.x=width;
		right2.y=-right2.x/tan(rightEdge[1]) + right1.y;
	} else {
		right1.y=0;
		right1.x=rightEdge[0]/cos(rightEdge[1]);

		right2.y=height;
		right2.x=right1.x - height*tan(rightEdge[1]);
	}

	bottom1.x=0;
	bottom1.y=bottomEdge[0]/sin(bottomEdge[1]);

	bottom2.x=width;
	bottom2.y=-bottom2.x/tan(bottomEdge[1]) + bottom1.y;

	top1.x=0;
	top1.y=topEdge[0]/sin(topEdge[1]);

	top2.x=width;
	top2.y=-top2.x/tan(topEdge[1]) + top1.y;

	// Next, we find the intersection of  these four lines
	double leftA = left2.y-left1.y;
	double leftB = left1.x-left2.x;

	double leftC = leftA*left1.x + leftB*left1.y;

	double rightA = right2.y-right1.y;
	double rightB = right1.x-right2.x;

	double rightC = rightA*right1.x + rightB*right1.y;

	double topA = top2.y-top1.y;
	double topB = top1.x-top2.x;

	double topC = topA*top1.x + topB*top1.y;

	double bottomA = bottom2.y-bottom1.y;
	double bottomB = bottom1.x-bottom2.x;

	double bottomC = bottomA*bottom1.x + bottomB*bottom1.y;

	// Intersection of left and top
	double detTopLeft = leftA*topB - leftB*topA;

	CvPoint ptTopLeft = cvPoint((topB*leftC - leftB*topC)/detTopLeft, (leftA*topC - topA*leftC)/detTopLeft);

	// Intersection of top and right
	double detTopRight = rightA*topB - rightB*topA;

	CvPoint ptTopRight = cvPoint((topB*rightC-rightB*topC)/detTopRight, (rightA*topC-topA*rightC)/detTopRight);

	// Intersection of right and bottom
	double detBottomRight = rightA*bottomB - rightB*bottomA;
	CvPoint ptBottomRight = cvPoint((bottomB*rightC-rightB*bottomC)/detBottomRight, (rightA*bottomC-bottomA*rightC)/detBottomRight);// Intersection of bottom and left
	double detBottomLeft = leftA*bottomB-leftB*bottomA;
	CvPoint ptBottomLeft = cvPoint((bottomB*leftC-leftB*bottomC)/detBottomLeft, (leftA*bottomC-bottomA*leftC)/detBottomLeft);

	// corrects the skewed photo, orients it STRAIGHT; finds the longest edge, then creates a square w/ this side length
	// calculate length of each side, whenever find a longer edge store its length squared. Once you have longest edge sqrt to find exact length
	int maxLength = (ptBottomLeft.x-ptBottomRight.x)*(ptBottomLeft.x-ptBottomRight.x) + (ptBottomLeft.y-ptBottomRight.y)*(ptBottomLeft.y-ptBottomRight.y);
	int temp = (ptTopRight.x-ptBottomRight.x)*(ptTopRight.x-ptBottomRight.x) + (ptTopRight.y-ptBottomRight.y)*(ptTopRight.y-ptBottomRight.y);

	if(temp>maxLength) maxLength = temp;

	temp = (ptTopRight.x-ptTopLeft.x)*(ptTopRight.x-ptTopLeft.x) + (ptTopRight.y-ptTopLeft.y)*(ptTopRight.y-ptTopLeft.y);

	if(temp>maxLength) maxLength = temp;

	temp = (ptBottomLeft.x-ptTopLeft.x)*(ptBottomLeft.x-ptTopLeft.x) + (ptBottomLeft.y-ptTopLeft.y)*(ptBottomLeft.y-ptTopLeft.y);

	if(temp>maxLength) maxLength = temp;

	maxLength = sqrt((double)maxLength);

	// create source and destination points
	Point2f src[4], dst[4];
	src[0] = ptTopLeft;
	src[1] = ptTopRight;
	src[2] = ptBottomRight;
	src[3] = ptBottomLeft;

	dst[0] = Point2f(0,0);
	dst[1] = Point2f(maxLength-1, 0);
	dst[2] = Point2f(maxLength-1, maxLength-1);
	dst[3] = Point2f(0, maxLength-1);

	// create a new image and do the undistortion; now the image undistored has the corrected image
	Mat undistorted = Mat(Size(maxLength, maxLength), CV_8UC1);
	cv::warpPerspective(original, undistorted, cv::getPerspectiveTransform(src, dst), Size(maxLength, maxLength));
	//adaptiveThreshold(undistorted, undistortedThreshed, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 101, 1);

	imwrite("original.jpg", original);
	imwrite("sudoku2.jpg", undistorted);// saves the photo as a jpeg.

	erode(undistorted, undistorted, 0); // b/c image was dilated a bit it is restored a bit by eroding it

	Mat undistortedThreshed = undistorted.clone();
	adaptiveThreshold(undistorted, undistortedThreshed, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 101, 1);

	//DigitRecognizer(undistorted);
	//Mat DigitRecognizer::preprocessImage(Mat undistorted);

	imwrite("sudoku3.jpg", undistortedThreshed);

	Mat sudokuC = undistortedThreshed.clone();

	//sudokuC = thresh(sudokuC);

	//GaussianBlur(sudokuB, sudokuB, Size(11,11), 0); // blurs the image to smooth out the noise and makes extracting grid lines easier

	//imwrite("B.png", sudokuC);

	//sudokuC = grid_extract(sudokuC);

	//imwrite("C.png", sudokuC);

	//erode(sudokuB, sudokuB, 0); // b/c image was dilated a bit it is restored a bit by eroding it
	//dilate(sudokuB, sudokuB, kernel);

	//imwrite("D.png", sudokuB);

	sudokuC = hough(sudokuC);

	imwrite("E.png", sudokuC);

	erode(sudokuC, sudokuC, 5); // b/c image was dilated a bit it is restored a bit by eroding it

	imwrite("F.png", sudokuC);

	fopen("tesseractAPI.exe", "r+");

	fopen("sudokuSolverComplete.exe", "r+");

	//sudokuB = grid_extract(sudokuB);

	//imwrite("G.png", sudokuB);

	//digit_extract(sudokuC);

	//system("tesseract /Users/austinjiang/programming/eclipse-workspace/puzzle/F.png /Users/austinjiang/programming/eclipse-workspace/puzzle/output -psm 6");
	//system("tesseract F.png output.txt");






	return 0;

}
