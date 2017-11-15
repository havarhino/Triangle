#pragma once
#include "stdafx.h"


class Vertex {
public:
	Vertex(float inX, float inY) {
		x = inX;
		y = inY;
	}
	Vertex(Vertex * inVertex) {
		x = inVertex->x;
		y = inVertex->y;
	}

	float x;
	float y;
private:
};

struct TriangleData {
	bool computed;
	int firstLineY;
	int lastLineY;

	int topHeight;
	int topY;
	int * topLX;
	int * topRX;
	BYTE * topLXColor;
	BYTE * topRXColor;

	int bottomHeight;
	int bottomY;
	int * bottomLX;
	int * bottomRX;
	BYTE * bottomLXColor;
	BYTE * bottomRXColor;
};
class TriangleObject {

public:
	TriangleObject(bool in_anti_aliasing, Vertex * vertex1, Vertex * vertex2, Vertex * vertex3);
	~TriangleObject();

	void draw(HDC hdc, float scale);

private:
	bool anti_aliasing;
	Vertex *p1, *p2, *p3;
	Vertex *topPt, *bottomPt, *midVPt;
	Vertex *leftPt, *rightPt, *midHPt;
	TriangleData triangleData;
	HPEN perimeterPen;
	HPEN gridPen;
	HBRUSH * fillBrush;


	void fillTriangle(float startY, float endY, float leftX, float leftSlope, float rightX, float rightSlope, int * pH, int * pY,
		        int ** pLX, int ** pRX, BYTE ** pLXColor, BYTE ** pRXColor );
	void drawHalfTriangle(HDC hdc, float scale, float xOff, float yOff, int H, int Y,
		        int * LX, int * RX, BYTE * LXColor, BYTE * RXColor);
	void computeTriangle(void);
	void sortPoints(void);
};