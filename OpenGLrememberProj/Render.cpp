#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;


	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(-8, 6, 10);
	}


	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);


		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL* ogl, int key)
{

}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL* ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);


	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE* texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH);

	// чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

struct Vector {
	float x;
	float y;
	float z;
};

Vector FindNormal(const double(&p1)[3], const double(&p2)[3], const double(&p3)[3])
{
	Vector v1, v2, normal;

	// Вычисляем векторы треугольника
	v1.x = p2[0] - p1[0];
	v1.y = p2[1] - p1[1];
	v1.z = p2[2] - p1[2];

	v2.x = p3[0] - p1[0];
	v2.y = p3[1] - p1[1];
	v2.z = p3[2] - p1[2];

	// Вычисляем векторное произведение
	normal.x = v1.y * v2.z - v1.z * v2.y;
	normal.y = -(v1.x * v2.z - v1.z * v2.x);
	normal.z = v1.x * v2.y - v1.y * v2.x;

	// Нормализуем вектор
	float length = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	normal.x /= length;
	normal.y /= length;
	normal.z /= length;
	glNormal3d(normal.x, normal.y, normal.z);
	return normal;
}

void OtrPrizm()
{
	double A[] = { 0, 3, 0 };
	double B[] = { 0, 11, 0 };
	double C[] = { -4, 9, 0 };
	double D[] = { -5, 16, 0 };
	double E[] = { -11, 14, 0 };
	double F[] = { -8, 9, 0 };
	double G[] = { -10, 4, 0 };
	double H[] = { -6, 6, 0 };
	double Z[] = { -2, 4,0 };

	double A1[] = { 0, 3, 6 };
	double B1[] = { 0, 11, 6 };
	double C1[] = { -4, 9, 6 };
	double D1[] = { -5, 16, 6 };
	double E1[] = { -11, 14, 6 };
	double F1[] = { -8, 9, 6 };
	double G1[] = { -10, 4, 6 };
	double H1[] = { -6, 6, 6 };
	double Z1[] = { -2, 4, 6 };


	//Днище
	glBegin(GL_TRIANGLE_FAN);
	FindNormal(A, B, H);
	glColor3d(0.3, 0.3, 0.3);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(H);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	FindNormal(B, C, H);
	glColor3d(0.3, 0.3, 0.3);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(H);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	FindNormal(C, F, H);
	glColor3d(0.3, 0.3, 0.3);
	glVertex3dv(C);
	glVertex3dv(F);
	glVertex3dv(H);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	FindNormal(F, G, H);
	glColor3d(0.3, 0.3, 0.3);
	glVertex3dv(F);
	glVertex3dv(G);
	glVertex3dv(H);
	glEnd();

	//Место для выреза
	glBegin(GL_TRIANGLE_FAN);
	FindNormal(C, D, F);
	glColor3d(0.3, 0.3, 0.3);
	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(F);
	glEnd();

	//Место для выреза
	glBegin(GL_TRIANGLE_FAN);
	FindNormal(D, E, F);
	glColor3d(0.3, 0.3, 0.3);
	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(F);
	glEnd();

	//Потолок
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_TRIANGLE_STRIP);
	FindNormal(A1, B1, C1);
	glTexCoord2d(0, 3); 
	glVertex3dv(A1);
	glTexCoord2d(0, 11); 
	glVertex3dv(B1);
	glTexCoord2d(-4, 9); 
	glVertex3dv(C1);

	glTexCoord2d(-4, 9); 
	glVertex3dv(C1);
	glTexCoord2d(0, 3); 
	glVertex3dv(A1);
	glTexCoord2d(-6, 6);
	glVertex3dv(H1);

	glTexCoord2d(-4, 9); 
	glVertex3dv(C1);
	glTexCoord2d(-6, 6);
	glVertex3dv(H1);
	glTexCoord2d(-8, 9);
	glVertex3dv(F1);

	glTexCoord2d(-4, 9); 
	glVertex3dv(C1);
	glTexCoord2d(-11, 14);
	glVertex3dv(E1);
	glTexCoord2d(-5, 16);
	glVertex3dv(D1);

	glTexCoord2d(-4, 9); 
	glVertex3dv(C1);
	glTexCoord2d(-11, 14);
	glVertex3dv(E1);
	glTexCoord2d(-8, 9);
	glVertex3dv(F1);

	glTexCoord2d(-6, 6);
	glVertex3dv(H1);
	glTexCoord2d(-10, 4);
	glVertex3dv(G1);
	glTexCoord2d(-8, 9);
	glVertex3dv(F1);
	
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//glBegin(GL_TRIANGLE_FAN);
	//FindNormal(A1, B1, H1);
	//glColor3d(0.3, 0.3, 0.3);
	//glVertex3dv(A1);
	//glVertex3dv(B1);
	//glVertex3dv(H1);
	//glEnd();

	//glBegin(GL_TRIANGLE_FAN);
	//FindNormal(B1, C1, H1);
	//glColor3d(0.3, 0.3, 0.3);
	//glVertex3dv(B1);
	//glVertex3dv(C1);
	//glVertex3dv(H1);
	//glEnd();

	//glBegin(GL_TRIANGLE_FAN);
	//FindNormal(C1, F1, H1);
	//glColor3d(0.3, 0.3, 0.3);
	//glVertex3dv(C1);
	//glVertex3dv(F1);
	//glVertex3dv(H1);
	//glEnd();

	//glBegin(GL_TRIANGLE_FAN);
	//FindNormal(F1, G1, H1);
	//glColor3d(0.3, 0.3, 0.3);
	//glVertex3dv(F1);
	//glVertex3dv(G1);
	//glVertex3dv(H1);
	//glEnd();

	////Место для выреза
	//glBegin(GL_TRIANGLE_FAN);
	//FindNormal(C1, D1, F1);
	//glColor3d(0.3, 0.3, 0.3);
	//glVertex3dv(C1);
	//glVertex3dv(D1);
	//glVertex3dv(F1);
	//glEnd();

	////Место для выреза
	//glBegin(GL_TRIANGLE_FAN);
	//FindNormal(D1, E1, F1);
	//glColor3d(0.3, 0.3, 0.3);
	//glVertex3dv(D1);
	//glVertex3dv(E1);
	//glVertex3dv(F1);
	//glEnd();

	//Стенки

	/*glBegin(GL_POLYGON);
	glColor3d(0.349, 0.341, 0.62);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(B1);
	glVertex3dv(A1);
	glEnd();*/

	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Радиус полукруглой выпуклости
	double radius = 4.0;

	// Центр полукруглой выпуклости (выберите координаты в соответствии с вашими требованиями)
	double center1[] = { 0, 7, 0 }; // Центр для A и B
	double center2[] = { 0, 7, 6 }; // Центр для A1 и B1

	// Количество сегментов для аппроксимации полукруглой выпуклости
	int segments = 30;

	// Вычисление и отрисовка вершин полукруглой выпуклости для A и B
	glBegin(GL_TRIANGLE_FAN);
	
	glColor3d(0.25, 0.5, 0);
	for (int i = 0; i <= segments; ++i) {
		double theta = M_PI * i / segments;
		double x = center1[0] + radius * sin(theta);
		double y = center1[1] + radius * cos(theta);
		double z = center1[2];
		glVertex3d(x, y, z);
	}
	glEnd();

	glBegin(GL_QUADS);
	glColor3d(0.25, 0.5, 0);

	for (int i = 0; i < segments; ++i) {
		double theta1 = M_PI * i / segments;
		double theta2 = M_PI * (i + 1) / segments;
		double x1 = center1[0] + radius * sin(theta1);
		double y1 = center1[1] + radius * cos(theta1);
		double z1 = center1[2];
		double x2 = center2[0] + radius * sin(theta1);
		double y2 = center2[1] + radius * cos(theta1);
		double z2 = center2[2];
		double x3 = center2[0] + radius * sin(theta2);
		double y3 = center2[1] + radius * cos(theta2);
		double z3 = center2[2];
		double x4 = center1[0] + radius * sin(theta2);
		double y4 = center1[1] + radius * cos(theta2);
		double z4 = center1[2];

		// Вершины для полуокружности A1 и B1
		glVertex3d(x1, y1, z1);
		glVertex3d(x2, y2, z2);
		glVertex3d(x3, y3, z3);
		glVertex3d(x4, y4, z4);
	}

	glEnd();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texId);
	// Вычисление и отрисовка вершин полукруглой выпуклости для A1 и B1
	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0.25, 0.5, 0);
	for (int i = 0; i <= segments; ++i) {
		double theta = M_PI * i / segments;
		double x = center2[0] + radius * sin(theta);
		double y = center2[1] + radius * cos(theta);
		double z = center2[2];
		glTexCoord2d(x, y);
		glVertex3d(x, y, z);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//////////////////////////////////////////////////////////////////////////////////////////////////

	glBegin(GL_POLYGON);
	glColor3d(0.349, 0.341, 0.62);
	glVertex3dv(D);
	glVertex3dv(C);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glEnd();

	////Грань перед впуклостью
	//glEnable(GL_BLEND);
	//glBegin(GL_POLYGON);
	//glColor4d(0.349, 0.341, 0.62, 0.1); // Цвет со значением альфа
	//glVertex3dv(E);
	//glVertex3dv(D);
	//glVertex3dv(D1);
	//glVertex3dv(E1);
	//glEnd();
	//glDisable(GL_BLEND);

	glBegin(GL_POLYGON);
	glColor3d(0.349, 0.341, 0.62);
	glVertex3dv(E);
	glVertex3dv(F);
	glVertex3dv(F1);
	glVertex3dv(E1);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3d(0.349, 0.341, 0.62);
	glVertex3dv(F);
	glVertex3dv(G);
	glVertex3dv(G1);
	glVertex3dv(F1);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3d(0.349, 0.341, 0.62);
	glVertex3dv(G);
	glVertex3dv(H);
	glVertex3dv(H1);
	glVertex3dv(G1);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3d(0.349, 0.341, 0.62);
	glVertex3dv(H);
	glVertex3dv(A);
	glVertex3dv(A1);
	glVertex3dv(H1);
	glEnd();

	glEnable(GL_BLEND);
	glBegin(GL_POLYGON);
	glColor4d(0.349, 0.341, 0.62, 0.6); // Цвет со значением альфа
	glVertex3dv(C);
	glVertex3dv(B);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glEnd();
	glDisable(GL_BLEND);

	///////////////////////////////////////////////////////////////////////////////////

	//// Радиус полукруглой выпуклости

	///////////////////////////////////////////////////////////////////////////////////

	double radius2 = 3.087;
	double center3[] = { -7.999, 15, 0 }; // Центр для E, C и D
	double center4[] = { -7.999, 15, 6 }; // Центр для E1, C1 и D1
	int segments2 = 30; // Количество сегментов для аппроксимации полукруглой выпуклости

	// Сохраняем текущую матрицу
	glPushMatrix();

	// Переносим центр фигуры в начало координат
	glTranslatef(center3[0], center3[1], center3[2]);

	// Добавляем поворот
	glRotatef(199.0f, 0.0f, 0.0f, 1.0f); // Поворот на 45 градусов вокруг Y-оси

	// Возвращаем центр фигуры обратно
	glTranslatef(-center3[0], -center3[1], -center3[2]);

	//glBegin(GL_TRIANGLE_FAN);
	//glColor3d(0.5, 0, 0.5); // Фиолетовый цвет
	//for (int i = 0; i <= segments2; ++i) {
	//	double theta = M_PI * i / segments2;
	//	double x = center3[0] + radius2 * cos(theta);
	//	double y = center3[1] + radius2 * sin(theta);
	//	double z = center3[2];
	//	glVertex3d(x, y, z);
	//}
	//glEnd();

	glBegin(GL_QUADS);
	glColor3d(0.5, 0, 0.5);

	for (int i = 0; i < segments2; ++i) {
		double theta1 = M_PI * i / segments2;
		double theta2 = M_PI * (i + 1) / segments2;
		double x1 = center3[0] + radius2 * cos(theta1);
		double y1 = center3[1] + radius2 * sin(theta1);
		double z1 = center3[2];
		double x2 = center4[0] + radius2 * cos(theta1);
		double y2 = center4[1] + radius2 * sin(theta1);
		double z2 = center4[2];
		double x3 = center4[0] + radius2 * cos(theta2);
		double y3 = center4[1] + radius2 * sin(theta2);
		double z3 = center4[2];
		double x4 = center3[0] + radius2 * cos(theta2);
		double y4 = center3[1] + radius2 * sin(theta2);
		double z4 = center3[2];

		// Вершины для полуокружности ECD и E1C1D1
		glVertex3d(x1, y1, z1);
		glVertex3d(x2, y2, z2);
		glVertex3d(x3, y3, z3);
		glVertex3d(x4, y4, z4);
	}


	glEnd();
}

void DrawAlphaObject() {
	// Включаем альфа-наложение
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Отрисовываем объект OtrPrizm
	OtrPrizm(); // Ваша функция OtrPrizm

	// Выключаем альфа-наложение после отрисовки
	glDisable(GL_BLEND);
}


void Render(OpenGL* ogl)
{
	//Добавление освещения
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	OtrPrizm();

	//DrawAlphaObject();


	/*double A[2] = { -4, -4 };
	double B[2] = { 4, -4 };
	double C[2] = { 4, 4 };
	double D[2] = { -4, 4 };

	glBindTexture(GL_TEXTURE_2D, texId);

	glColor3d(0.6, 0.6, 0.6);
	glBegin(GL_QUADS);

	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);

	glEnd();*/


	//Сообщение вверху экрана


	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
									//(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}