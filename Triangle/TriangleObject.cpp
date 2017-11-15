#include <stdexcept>
#include "stdafx.h"
#include "TriangleObject.h"
#include <math.h>
#include <stdlib.h>


#define PI (3.1415926535)

void TriangleObject::sortPoints() {
	Vertex * tmp;

	topPt = p1;
	midVPt = p2;
	bottomPt = p3;

	// Make sure topPt is above midVPt
	if (topPt->y > midVPt->y) {
		tmp = topPt;
		topPt = midVPt;
		midVPt = tmp;
	}

	// Now, make sure topPt is the above bottomPt as well
	if (topPt->y > bottomPt->y) {
		tmp = topPt;
		topPt = bottomPt;
		bottomPt = tmp;
	}

	// Now, make sure midVPt is above bottomPt
	if (midVPt->y > bottomPt->y) {
		tmp = midVPt;
		midVPt = bottomPt;
		bottomPt = tmp;
	}

	leftPt = p1;
	midHPt = p2;
	rightPt = p3;

	// Make sure leftPt is above midHPt
	if (leftPt->x > midHPt->x) {
		tmp = leftPt;
		leftPt = midHPt;
		midHPt = tmp;
	}

	// Now, make sure leftPt is the above rightPt as well
	if (leftPt->x > rightPt->x) {
		tmp = leftPt;
		leftPt = rightPt;
		rightPt = tmp;
	}

	// Now, make sure midHPt is above rightPt
	if (midHPt->x > rightPt->x) {
		tmp = midHPt;
		midHPt = rightPt;
		rightPt = tmp;
	}

}

TriangleObject::TriangleObject(bool in_anti_aliasing, Vertex * vertex1, Vertex * vertex2, Vertex * vertex3) {
	anti_aliasing = in_anti_aliasing;
	p1 = new Vertex(vertex1);
	p2 = new Vertex(vertex2);
	p3 = new Vertex(vertex3);

	sortPoints();

	triangleData.computed = false;
	triangleData.topHeight = 0;
	triangleData.topY = 0;
	triangleData.topLX = NULL;
	triangleData.topRX = NULL;
	triangleData.bottomHeight = 0;
	triangleData.bottomY = 0;
	triangleData.bottomLX = NULL;
	triangleData.bottomRX = NULL;

	perimeterPen = CreatePen( PS_SOLID, 1, RGB(0, 192, 0) );
	gridPen = CreatePen( PS_SOLID, 1, RGB(0, 0, 192) );
	fillBrush = new HBRUSH[256];
	for (int i = 0; i < 256; i++) {
	    fillBrush[i] = CreateSolidBrush( RGB(255, 255-i , 255-i) );
	}
}

TriangleObject::~TriangleObject() {
	if( perimeterPen != 0 ) {
	    DeleteObject(perimeterPen);
	}
	for (int i = 0; i < 256; i++) {
		if (fillBrush[i] != 0) {
			DeleteObject(fillBrush[i]);
		}
	}
	delete fillBrush;

	delete p1;
	delete p2;
	delete p3;
	if (triangleData.topLX != NULL) {
		delete triangleData.topLX;
		triangleData.topLX = NULL;
	}
	if (triangleData.topRX != NULL) {
		delete triangleData.topRX;
		triangleData.topRX = NULL;
	}
	if (triangleData.bottomLX != NULL) {
		delete triangleData.bottomLX;
		triangleData.bottomLX = NULL;
	}
	if (triangleData.bottomRX != NULL) {
		delete triangleData.bottomRX;
		triangleData.bottomRX = NULL;
	}
}

bool clockWise(Vertex * p1, Vertex * p2, Vertex * p3) {
	// We know these are not co-linear, because calling function ensures this

	// First, find the centroid point
	float cX = (p1->x + p2->x + p3->x) / 3.0f;
	float cY = (p1->y + p2->y + p3->y) / 3.0f;

	// Now, look at the angle from point 1 to point 2, if negative, then the points are in counter-clockwise order
	double x1 = p1->x - cX;
	double y1 = p1->y - cY;
	double x2 = p2->x - cX;
	double y2 = p2->y - cY;

	double a1 = atan2(y1, x1) * 180.0 / PI;
	double a2 = atan2(y2, x2) * 180.0 / PI;

	// NOTE:  Since positive y is down, increasing angles means clockwise rotation, which
	//        is opposite of traditional cartesian coordinates.

	// clockwise means a2 - a1 should be less than 180.0 (since angles get more positive going clockwise in screen coordinates)
	// Example:  If piont 1 is at 30 degrees and point 2 is at 45 degrees, this is clockwise, and a2 - a1 would be less than 180.
	double deltaAngle = a2 - a1;
	if (deltaAngle < 0.0) {
		deltaAngle += 360.0;
	}

	if (deltaAngle < 180.0) {
		return true;
	}
	else {
		return false;
	}
}

void TriangleObject::fillTriangle( float startY, float endY, float leftX, float leftSlope, float rightX, float rightSlope,
	                               int * pH, int *pY, int ** pLX, int ** pRX , BYTE ** pLXColor, BYTE ** pRXColor ) {
	int topIntY = (int)(startY + 0.5);
	int bottomIntY = (int)(endY - 0.5);
	
	if (bottomIntY < topIntY) {
		// Nothing to do
		return;
	}

	int H = bottomIntY - topIntY + 1;
	*pH = H;
	*pY = topIntY;
	*pLX = new int[H];
	*pRX = new int[H];
	*pLXColor = new BYTE[H];
	*pRXColor = new BYTE[H];

	//int * tmpArray = (int *)malloc(H * sizeof(int));
	//*pLX = tmpArray;
	//*pRX = (int *)malloc(H * sizeof(int));

	float currentY;
	float deltaY;
	int yIndex = 0;
	for (int y = topIntY; y <= bottomIntY; y++) {
		currentY = (float)(y)+0.5;
		deltaY = currentY - startY;

		float tmpXL = leftX + deltaY * leftSlope;
		float tmpXR = rightX + deltaY * rightSlope;
		if (anti_aliasing) {
			(*pLX)[yIndex] = (int)(tmpXL);
			(*pRX)[yIndex] = (int)(tmpXR);
			if ((int)tmpXL == (int)tmpXR) {
				BYTE color = (BYTE)((tmpXR - tmpXL)*255.0);
				(*pLXColor)[yIndex] = color;
				(*pRXColor)[yIndex] = color;
			}
			else {
				(*pLXColor)[yIndex] = (BYTE)((1.0f - (tmpXL - (float)((int)tmpXL)))*255.0);
				(*pRXColor)[yIndex] = (BYTE)((tmpXR - (float)((int)tmpXR))*255.0);
			}
		} else {
			(*pLX)[yIndex] = (int)(tmpXL + 0.5);
			(*pRX)[yIndex] = (int)(tmpXR - 0.5);
		}
		yIndex++;
	}
}

void TriangleObject::computeTriangle(void) {
	bool exceptionRaised = false;

	if (((p1->y == p2->y) && (p1->y == p3->y)) ||
		((p1->x == p2->x) && (p1->x == p3->x))) {
        // All 3 points are co-linear, nothing to do
	    triangleData.computed = true;
		return;
	}

	if( !clockWise(p1, p2, p3) ) {
		throw "Points are not in clockwise rotation";
		exceptionRaised = true;
	}

	if (p1->y == p2->y) {
		throw "Not Yet Implemented";
		exceptionRaised = true;
	}
	else if (p1->y == p3->y) {
		throw "Not Yet Implemented";
		exceptionRaised = true;
	}
	else if (p2->y == p3->y) {
		throw "Not Yet Implemented";
		exceptionRaised = true;
	}
	else {
        // All points have different y values
		float leftSlope, rightSlope;
		if (midVPt->x < topPt->x) {
			leftSlope = (midVPt->x - topPt->x) / (midVPt->y - topPt->y);
			rightSlope = (bottomPt->x - topPt->x) / (bottomPt->y - topPt->y);
		}
		else {
			leftSlope = (bottomPt->x - topPt->x) / (bottomPt->y - topPt->y);
			rightSlope = (midVPt->x - topPt->x) / (midVPt->y - topPt->y);
		}

		fillTriangle(topPt->y, midVPt->y, topPt->x, leftSlope, topPt->x, rightSlope,
			         &(triangleData.topHeight), &(triangleData.topY), 
			         &(triangleData.topLX), &(triangleData.topRX),
			         &(triangleData.topLXColor), &(triangleData.topRXColor) );

		float leftX, rightX;
		if (midVPt->x < topPt->x) {
			leftSlope = (bottomPt->x - midVPt->x) / (bottomPt->y - midVPt->y);
			leftX = midVPt->x;
			rightX = topPt->x + rightSlope*(midVPt->y - topPt->y);
		}
		else {
			rightSlope = (bottomPt->x - midVPt->x) / (bottomPt->y - midVPt->y);
			leftX = topPt->x + leftSlope*(midVPt->y - topPt->y);
			rightX = midVPt->x;
		}

		fillTriangle(midVPt->y, bottomPt->y, leftX, leftSlope, rightX, rightSlope,
			         &(triangleData.bottomHeight), &(triangleData.bottomY),
			         &(triangleData.bottomLX), &(triangleData.bottomRX),
			         &(triangleData.bottomLXColor), &(triangleData.bottomRXColor) );
	}

	triangleData.computed = true;
}

void TriangleObject::drawHalfTriangle(HDC hdc, float scale, float xOff, float yOff, int H, int Y,
	                  int * LX, int * RX, BYTE * LXColor, BYTE * RXColor) {
	RECT r;
	for (int i = 0; i < H; i++) {
		if (anti_aliasing) {
			r.left = (LX[i])*scale + xOff;
			r.right = (LX[i] + 1)*scale + xOff;
			r.top = (Y + i)*scale + yOff;
			r.bottom = (Y + i + 1)*scale + yOff;
			FillRect(hdc, &r, fillBrush[LXColor[i]]);

			r.left = (RX[i])*scale + xOff;
			r.right = (RX[i] + 1)*scale + xOff;
			FillRect(hdc, &r, fillBrush[RXColor[i]]);

			r.left = (LX[i] + 1)*scale + xOff;
			r.right = (RX[i])*scale + xOff;
			if (r.left < r.right) {
				FillRect(hdc, &r, fillBrush[255]);
			}
		}
		else {
			r.left = LX[i] * scale + xOff;
			r.right = (RX[i] + 1)*scale + xOff;
			if (r.left < r.right) {
				r.top = (Y + i)*scale + yOff;
				r.bottom = (Y + i + 1)*scale + yOff;

				FillRect(hdc, &r, fillBrush[255]);
			}
		}
	}
}

void TriangleObject::draw(HDC hdc, float scale) {


	if (!triangleData.computed) {
		computeTriangle();
	}

	HGDIOBJ oldObj = SelectObject(hdc, perimeterPen);

	// We want to anchor the triangle at the same top-left location as the unscaled version.
	// So, if the scale is 10, and the left anchor was 3, without adjustment, the left anchor would be 3*scale = 30.
	// So, we need to subtract  (scale - 1)*xOff from each scaled X location to get it anchored correct.
	//    So, 30  +   (-(10 - 1)*3 ) = 30 - 27 = 3
	float xOff = -(float)(scale - 1)*leftPt->x;
	float yOff = -(float)(scale - 1)*topPt->y;

	drawHalfTriangle(hdc, scale, xOff, yOff, triangleData.topHeight, triangleData.topY,
		triangleData.topLX, triangleData.topRX, triangleData.topLXColor, triangleData.topRXColor);

	drawHalfTriangle(hdc, scale, xOff, yOff, triangleData.bottomHeight, triangleData.bottomY,
		triangleData.bottomLX, triangleData.bottomRX, triangleData.bottomLXColor, triangleData.bottomRXColor);

	if (scale <= 1) {
	    //MoveToEx(hdc, p1->x, p1->y, NULL);
	    //LineTo(hdc, p2->x, p2->y);
	    //LineTo(hdc, p3->x, p3->y);
	    //LineTo(hdc, p1->x, p1->y);
	} else {
	    SelectObject(hdc, gridPen);

		int leftXInt = leftPt->x;
		int rightXInt = (int)(rightPt->x + 0.9999);
		int	topYInt = topPt->y;
		int bottomYInt = (int)(bottomPt->y + 0.9999);
		for (int y = topYInt; y <= bottomYInt; y++) {
	        MoveToEx(hdc, leftXInt*scale + xOff, y*scale + yOff, NULL);
	        LineTo(hdc, rightXInt*scale + xOff, y*scale + yOff);
		}
		for (int x = leftXInt; x <= rightXInt; x++) {
	        MoveToEx(hdc, x*scale + xOff, topYInt * scale + yOff, NULL);
	        LineTo(hdc, x*scale + xOff, bottomYInt * scale + yOff);
		}

	    SelectObject(hdc, perimeterPen);

		MoveToEx(hdc, p1->x * scale + xOff, p1->y*scale + yOff, NULL);
	    LineTo(hdc, p2->x * scale + xOff, p2->y * scale + yOff);
	    LineTo(hdc, p3->x * scale + xOff, p3->y * scale + yOff);
	    LineTo(hdc, p1->x * scale + xOff, p1->y * scale + yOff);
	}

	SelectObject(hdc, oldObj);
}