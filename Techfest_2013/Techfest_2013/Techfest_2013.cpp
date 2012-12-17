// Techfest_2013.cpp : Defines the entry point for the console application.
//
//This time we'll be using triangle markers for bot identification. Will consume less space becoz less points to store.

#include "stdafx.h"
#include <highgui.h>

#include <cv.h>
CvPoint circle[2]; // global variable to store centre of circular tags on bot
CvPoint nr_sq;     // global variable to store centre of nearest square

#include<windows.h>

#include<string.h>

#include<conio.h>

#include<iostream>
 
using namespace std;

// Initialize the serial port variable and parameters
HANDLE hPort = CreateFile(TEXT("COM1"), GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
DCB dcb={0}; // Create a DCB struct variable
/*********************Function for Sending Data***********************/

bool writebyte(char* data)
{

DWORD byteswritten;
a1:
//Sleep (100);
 dcb.DCBlength = sizeof(dcb);
if (!GetCommState(hPort,&dcb))

{

cout<<"\nSerial port cant b opened\n";
goto a1;

//return false;

}

dcb.BaudRate = CBR_9600;  //9600 Baud 
dcb.ByteSize = 8;	//8 data bits
dcb.Parity = NOPARITY;    //no parity 
dcb.StopBits = ONESTOPBIT; //1 stop

int retVal=0;

if (SetCommState(hPort,&dcb)) //If Com port cannot be configured accordingly return false;
	retVal = WriteFile(hPort,(int *)data,1,&byteswritten,NULL); //Write the data to be sent to Serial port 
//printf("\n BYTES Written  %d  %d  \n",byteswritten,*data);
if (retVal==0)
	return false;
else
	return true;

// return true if the data is written
}

void movent_decide(IplImage *img)     // finds angle b/w the square center and the vector of bot orientation
{
	double side1=0.0,side2=0.0,side3=0.0,sum1=0.0;
	char data='n';

	// draw lines to join the centres of circles(vector) to see bot orientation
	cvLine(img,circle[0],circle[1],CV_RGB(255,0,0),3,4,0);
        // draw line b/w center of red circle to sq center
	cvLine(img,circle[0],nr_sq,CV_RGB(255,0,0),3,4,0);
	cvLine(img,circle[1],nr_sq,CV_RGB(255,0,0),3,4,0);
	side1= sqrt(pow((double)circle[1].y-circle[0].y,2)+pow((double)circle[1].x-circle[0].x,2));
	side2= sqrt(pow((double)circle[1].y-nr_sq.y,2)+pow((double)circle[1].x-nr_sq.x,2));
	side3= sqrt(pow((double)circle[0].y-nr_sq.y,2)+pow((double)circle[1].x-nr_sq.x,2));
	printf("%f   %f     %f\n",side1,side2,side3); 
	sum1=side1+side2;
	data='r';
			if(writebyte(&data))
				printf("\n%c ",data);
	if((sum1-side3)>20 || side2>side3)
	{
		if(nr_sq.y<=circle[1].y)
		{
			if(circle[1].x < nr_sq.x)
			{
				//turn right
				data='r';
			if(writebyte(&data))
				printf("\n%c ",data);
			}
			if(circle[1].x > nr_sq.x)
			{
				//turn left
				data='l';
			if(writebyte(&data))
				printf("\n%c ",data);
			}
		}
		if(nr_sq.y>circle[1].y)
		{
			if(circle[1].x < nr_sq.x)
			{
				//turn left
				data='l';
			if(writebyte(&data))
				printf("\n%c ",data);
			}
			if(circle[1].x > nr_sq.x)
			{
				//turn right
				data='r';
			if(writebyte(&data))
				printf("\n%c ",data);
			}
		}
	}
	else
	{
		if(side2>200)
		{
			// go forward
			data='f';
			if(writebyte(&data))
				printf("\n%c ",data);
		}
		else
		{
			data='n';
			if(writebyte(&data))
				printf("\n%c ",data);
		}
		
	}



	
	// calculates and prints the angle between the two lines
	//angle=fabs(atan((double)(nr_sq.y-circle[0].y)/(nr_sq.x-circle[0].x))-atan((double)(circle[1].y-circle[0].y)/(circle[1].x-circle[0].x)));
	//printf("angle - %f\n",angle*(180/3.146));
}
	


IplImage* thatonly(IplImage* m1,IplImage* m2,IplImage* m3,int contrast) // returns the image containing only one particular channel(m3)
{
	cvAdd(m1, m2, m2);
        cvSub(m3, m2, m3);
	cvScale(m3, m3,contrast ); // increasing contrast
	cvDilate(m3, m3, 0,1);
	cvErode(m3,m3, 0, 1);
	//cvReleaseImage(&m1);
	//cvReleaseImage(&m2);
	return m3;
}

CvPoint centre(CvPoint* at ,int n) // calculates and returns centroid of the figure
{
	int x=0,y=0,i;
	for(i=0;i<n;i++)
	{
		x+=at[i].x;
		y+=at[i].y;
	}
	CvPoint p=cvPoint(x/n,y/n);
	return p;
}


IplImage* threshImg(IplImage* img,int param1) // performs thresholding depending upon the value of the 'param1' parameter
{
	IplImage* imghsv = cvCreateImage(cvGetSize(img),8,1);
	cvThreshold(img,imghsv,param1,255,CV_THRESH_BINARY);
	return imghsv;
}



int main()
{
	CvCapture* capture = 0;
	IplImage* img=0,* red=0,* green=0,* blue=0,*ronly=0,*gonly=0,*bonly=0 ;
	//IplImage* imgthresh=0 ;
	//IplImage* red=0;
	//IplImage* green=0;
	//IplImage* blue=0;
	IplImage *imgthresh[3];
	imgthresh[0]=0;
	imgthresh[1]=0;
	imgthresh[2]=0;
	CvPoint *pt;
	int x=0,i=0,c=0,thresh=10,rthresh=10,gthresh=10,bthresh=10,min=100,max=10000,contrast=4,rcont=4,gcont=4,bcont=4,ch=0,prevch=0,n=0;
	//double sum=0;
	CvSeq *contours,*result;
        //  CvSeq* result;
        CvMemStorage *storage = cvCreateMemStorage(0);
	
	capture = cvCaptureFromCAM(1);
	if(!capture)
	{
		printf("Could not initialize capturing....\n");
		return -1;
	}

	cvNamedWindow("Original");
	cvNamedWindow("Channel");
	//cvNamedWindow("Green");
	//cvNamedWindow("Blue");
	cvNamedWindow("Threshold");
	cvCreateTrackbar("Ch_Select","Channel",&ch,2,NULL);
	cvCreateTrackbar("Thresh","Threshold",&thresh,255,NULL);
	//cvCreateTrackbar("Threshold","Green",&pg,60,NULL);
	//cvCreateTrackbar("Threshold","Blue",&pb,60,NULL);
	cvCreateTrackbar("Contrast","Threshold",&contrast,40,NULL);
	//cvCreateTrackbar("Contrast","Green",&gcont,20,NULL);
	//cvCreateTrackbar("Contrast","Blue",&bcont,20,NULL);
        cvCreateTrackbar("MIN AREA","Original",&min,1000000,NULL);
	cvCreateTrackbar("MAX AREA","Original",&max,9000000,NULL);
	printf("0->RED\n1->GREEN\n2->BLUE");
		
	while(true)
	{
		
		img=cvQueryFrame(capture);
		if(!img)
			break;
		//sum = cvGetCaptureProperty(capture,CV_CAP_PROP_FPS);
		//printf("\n FPS - %f  ",sum);
		cvShowImage("Original",img);
		red=cvCreateImage(cvGetSize(img), 8, 1);
	        green=cvCreateImage(cvGetSize(img), 8, 1);
	        blue=cvCreateImage(cvGetSize(img), 8, 1);
		//ronly=cvCreateImage(cvGetSize(img), 8, 1);
		//gonly=cvCreateImage(cvGetSize(img), 8, 1);
		//bonly=cvCreateImage(cvGetSize(img), 8, 1);
		if(prevch==ch) // to know whether slider for channel select has moved or not
		{
		switch(ch) // if not then update the specific channel related variables
		{
		case 0:
			rcont=contrast;
			rthresh=thresh;
			break;
		case 1:
			gcont=contrast;
			gthresh=thresh;
			break;
		case 2:
			bcont=contrast;
			bthresh=thresh;
			break;
		}
		}
		else  // if moved then show the new channel's variable values 
		{
			prevch=ch;
			switch(ch)
		{
		case 0:
			contrast=rcont;
			thresh=rthresh;
			cvSetTrackbarPos("Thresh","Threshold",thresh);
			cvSetTrackbarPos("Contrast","Threshold",contrast);
			break;
		case 1:
			contrast=gcont;
			thresh=gthresh;
			cvSetTrackbarPos("Thresh","Threshold",thresh);
			cvSetTrackbarPos("Contrast","Threshold",contrast);
			break;
		case 2:
			contrast=bcont;
			thresh=bthresh;
			cvSetTrackbarPos("Thresh","Threshold",thresh);
			cvSetTrackbarPos("Contrast","Threshold",contrast);
			break;
		}

		}



	
		cvSplit(img,blue, green, red, NULL);
		red=thatonly(blue,green,red,rcont);
		ronly=cvCloneImage(red);             // cvClone() func is used and ronly=red is not done because then we are just refering to   same image and when next cvSplit() executes it creates a problem...... becoz red and ronly are pointers..
		//cvShowImage("Red",ronly);


		cvSplit(img,blue, green, red, NULL);
		blue=thatonly(green,red,blue,bcont);
		bonly=cvCloneImage(blue);
		//cvShowImage("Blue",bonly);

		cvSplit(img,blue, green, red, NULL);
		green=thatonly(red,blue,green,gcont);
		gonly=cvCloneImage(green);
		//cvShowImage("Green",gonly);
		
		cvReleaseImage(&red); // releasing unused images
	        cvReleaseImage(&green);
	        cvReleaseImage(&blue);

		imgthresh[0] = threshImg(ronly,rthresh);  // thresholding each channel depending upon the value set by the slider
		imgthresh[1] = threshImg(gonly,gthresh);
		imgthresh[2] = threshImg(bonly,bthresh);
		
		
		switch(ch) // this helps to display correct images on the windows depending upon the channel select slider value
		{
		case 0:
			cvShowImage("Threshold",imgthresh[0]);
			cvShowImage("Channel",ronly);
			break;
		case 1:
			cvShowImage("Threshold",imgthresh[1]);
			cvShowImage("Channel",gonly);
			break;
		case 2:
			cvShowImage("Threshold",imgthresh[2]);
			cvShowImage("Channel",bonly);
			break;
		}
		//imgthresh=red;
		for(n=0;n<=2;n++) // loop to find contours in red green blue channels
		{
			
			cvFindContours(imgthresh[n], storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
	
	while(contours)
    {
         // approximating contours boundaries to get rid of torned edges
        result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP,cvContourPerimeter(contours)*0.03, 0);
		x=result->total ; // calculating the total no of edges in a approximated contour(blob)
                // if x==4 then its a square and fabs()-> gives abolute value and with cvContourArea we are setting the min and max blob size to be detected in image 
		if(x==4 && fabs(cvContourArea(result,CV_WHOLE_SEQ))>min && fabs(cvContourArea(result,CV_WHOLE_SEQ))<max && cvCheckContourConvexity(result))
		{ 
			
			CvPoint* at= (CvPoint*)malloc(x*sizeof(CvPoint));//http://stackoverflow.com/questions/260915/how-can-i-create-a-dynamically-sized-array-of-structs
			// printing total no of edge detected in the blob
			printf("Result %d",result->total);
                        // loop to store the coordinates of the edges of the detected contour and storing then in CvPoint structure
			for( i=0 ; i<x;i++)
			{
				pt=(CvPoint*)cvGetSeqElem(result,i);
				//printf("Point  x = %d   y= % d \n",pt->x,pt->y);
				at[i]=cvPoint(pt->x,pt->y);
			
			}
			
                        // loop to draw boundary around the contour
			for(i=0;i<x-1;i++)
				cvLine(img,at[i],at[i+1],cvScalar(255),1,4,0);
			cvLine(img,at[i],at[0],cvScalar(255),1,4,0);
			cvCircle(img,centre(at,x),3,CV_RGB(0,255,0),-1,8,0);
			if(n==2)// to store the centroid of square contour in red channel
			{
				nr_sq=centre(at,x);
			}
			free(at);
		}

		if(n!=1 )// entering only when red and blue channel is thresholded( basically i do not want to detect green colour circle, so i wont enter into the loop)
		{
			
                // edges greater than 6 means its a circle like figure and definetly not a square
		if(x>6 && fabs(cvContourArea(result,CV_WHOLE_SEQ))>min && fabs(cvContourArea(result,CV_WHOLE_SEQ))<max && cvCheckContourConvexity(result))
		{ 
			
			CvPoint* at= (CvPoint*)malloc(x*sizeof(CvPoint));//http://stackoverflow.com/questions/260915/how-can-i-create-a-dynamically-sized-array-of-structs
			
			printf("Result %d",result->total);
			for( i=0 ; i<x;i++)
			{
				pt=(CvPoint*)cvGetSeqElem(result,i);
			//	printf("Point  x = %d   y= % d \n",pt->x,pt->y);
				at[i]=cvPoint(pt->x,pt->y);
			}
			for(i=0;i<x-1;i++)
				cvLine(img,at[i],at[i+1],cvScalar(255),3,4,0);
			cvLine(img,at[i],at[0],cvScalar(255),3,4,0);
			cvCircle(img,centre(at,x),3,CV_RGB(0,255,0),-1,8,0);
			circle[n]=centre(at,x);
			if(n==2)
				circle[1]=centre(at,x);
			free(at);
		}
		}
		contours = contours->h_next;
	}
		}
		//movent_decide(img);
	cvShowImage("Original",img);
	c = cvWaitKey(20); // waiting for 20 milli seconds
		if(c==27)  // detecting esc key press
			break;
                 // doing some cleaning up
		cvReleaseImage(&ronly);
		cvReleaseImage(&gonly);
		cvReleaseImage(&bonly);
		cvReleaseImage(&imgthresh[0]);
		cvReleaseImage(&imgthresh[1]);
		cvReleaseImage(&imgthresh[2]);
		

		//cvClearSeq(result);
		//cvClearSeq(contours);
		//cvReleaseMemStorage(&contours->storage);
		
	}

	
	//cvReleaseImage(&img);
	cvReleaseMemStorage(&storage);
	//cvClearSeq(contours);
	CloseHandle(hPort);
	
	cvReleaseCapture(&capture);
	//cvClearSeq(contours);
	return 0;
}
