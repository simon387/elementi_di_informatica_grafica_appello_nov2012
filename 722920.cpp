#include <GL/glut.h>
#include <cstdlib>
#include "tga_io.h"
#include "3dmath.h"
#include <stdio.h> //per le printf
#include <string>
#include <iostream>
#define VERSION "2.1"
#define maxH 100 //dimensione massima h della piantina
#define maxW 100 //dimensione massima w della piantina

using namespace std;
using namespace CGU;

static GLuint texName;

//variabili varie
static char cMatrix[maxH][maxW][2];
static int nLuci = 0;
static bool nCamere = false;
static float locLuci[8][3];
static int dimensioni[2] = {0, 0};
static float cameraPos[4][3];
static float textLoc[3] = {0, 0, 0};
static int cC = 0; //indice di camera corrente (currentCamera)
static bool bFree = false;
static bool xFree = true;

//angoli di rotazione modificati dal movimento del mouse
static int xrot, yrot;
static float pitch;
static float speed = 1.5;//velocità di avanzamento 1.5 default

//booleani associati ai tasti freccia. true=premuto.
static bool left_key, right_key, down_key, up_key, top_key, under_key;
static bool freelook_flag;//modalità freelook con il mouse
static bool mouse_moved = false;//true se il mouse è stato mosso
static int width, height;//altezza e larghezza della finestra
static Camera camera;

//Callback initScene
void initScene(void);

//Callbacks GLUT
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void display(void);
void processMouse(int button, int state, int x, int y);
void processMousePassiveMotion(int x, int y);
void reshape(int w, int h);
void idle(void);

// Imposto le proprietà del materiale della parete
void imposta_materiale_parete() {
	GLfloat front_materiale_specular[] = {0.5, 0.5, 0.5, 1.0};
	GLfloat front_materiale_diffuse[] = {1.0, 0.0, 0.0, 1.0};
	GLfloat front_materiale_shininess[] = {30.0};
	glMaterialfv(GL_FRONT, GL_DIFFUSE, front_materiale_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, front_materiale_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, front_materiale_shininess);
}

// Imposto le proprietà del materiale del soffitto
void imposta_materiale_soffitto() {
	GLfloat front_materiale_specular[] = {0.0, 0.0, 0.0, 1.0};//1.0 finale
	GLfloat front_materiale_diffuse[] = {0.0, 0.0, 0.0, 1.0};// 1.0 finale
	GLfloat front_materiale_shininess[] = {120.0};//120
	glMaterialfv(GL_FRONT, GL_DIFFUSE, front_materiale_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, front_materiale_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, front_materiale_shininess);
}

// Imposto le proprietà del materiale del pavimento
void imposta_materiale_pavimento() {
	GLfloat front_materiale_specular[] = {0.3, 0.3, 0.3, 1.0};
	GLfloat front_materiale_diffuse[] = {0.2, 0.2, 0.2, 1.0};
	GLfloat front_materiale_shininess[] = {50.0};
	glMaterialfv(GL_FRONT, GL_DIFFUSE, front_materiale_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, front_materiale_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, front_materiale_shininess);
}

// Imposto le proprietà del materiale della lampada
void imposta_materiale_lampada() {
	GLfloat front_materiale_specular[] = {1, 1, 1, 1};
	GLfloat front_materiale_diffuse[] = {1, 1, 1, 1};
	GLfloat front_materiale_shininess[] = {50.0};//GLfloat a[] = {0,0,0,0};GLfloat b[] = {0};
	glMaterialfv(GL_FRONT, GL_DIFFUSE, front_materiale_diffuse);//glMaterialfv(GL_BACK, GL_DIFFUSE, a);
	glMaterialfv(GL_FRONT, GL_SPECULAR, front_materiale_specular);//glMaterialfv(GL_BACK, GL_SPECULAR, a);
	glMaterialfv(GL_FRONT, GL_SHININESS, front_materiale_shininess);//glMaterialfv(GL_BACK, GL_SHININESS, b);
}

//Funzione di inizializzazione. Inizializza stati opengl, flags e variabili di programma.
void initGL(void) {
	//Resetto variabili per le rotazioni gestite dal mouse.
	xrot = yrot = 0;
	pitch = 0;//beccheggio, asse che corre da destra a sinistra
	//reset dei booleani associati ai tasti di movimento
	left_key = right_key = down_key = up_key = top_key = under_key = false;
	//cerco la posizione iniziale, primo punto g(Green) trovato
	int x = 0, y = 0, z = 0;
	for (int piano = 0; piano < 2; piano++)
		for (int h = 0; h < dimensioni[0]; h++)
			for (int w = 0; w < dimensioni[1]; w++)
				if (cMatrix[h][w][piano] == 'g') {
					x = ((1 + h) * 20) - 10;
					y = ((1 + w) * 20) - 10;
					z = piano;
				}
	float altezzaCamera = 30;
	if (z == 1) altezzaCamera += 60.0;
	printf("%s %i %f %i\n", "setting the camera's position:", x, altezzaCamera, y);
	camera.position.set(x, altezzaCamera, y);//posizione iniziale della camera
	freelook_flag = false;

	for (int i = 0; i < 4; i++) {
		cameraPos[i][0] = x;
		cameraPos[i][1] = altezzaCamera;
		cameraPos[i][2] = y;
	}
	
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);

	// abilito l'illuminazione
	glEnable(GL_LIGHTING);

	// imposto la luce
	GLfloat light0_ambient[] = {0.0, 0.0, 0.0, 0.0};//222	0.1, 0.1, 0.1, 1.0
	GLfloat light0_diffuse[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat light0_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat spot_direction[] = {0, 0, 0};
	float linearAttenuation = 0.001;
	float spotCutoff = 180.0; //0:90-180
	float spotExponent = 2.0;
	switch (nLuci) {
		case 8: {
			glEnable(GL_LIGHT7);
			printf("%s\n", "enabling light7");
		}
		case 7: {
			glEnable(GL_LIGHT6);
			printf("%s\n", "enabling light6");
		}	
		case 6: {
			glEnable(GL_LIGHT5);
			printf("%s\n", "enabling light5");
		}	
		case 5: {
			glEnable(GL_LIGHT4);
			printf("%s\n", "enabling light4");
		}	
		case 4: {
			glEnable(GL_LIGHT3);
			printf("%s\n", "enabling light3");
		}	
		case 3: {
			glEnable(GL_LIGHT2);
			printf("%s\n", "enabling light2");
		}	
		case 2: {
			glEnable(GL_LIGHT1);
			printf("%s\n", "enabling light1");
		}	
		case 1: {
			glEnable(GL_LIGHT0);
			printf("%s\n", "enabling light0");
		}	
	}
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	glLightf (GL_LIGHT0, GL_QUADRATIC_ATTENUATION, linearAttenuation);
	glLightf (GL_LIGHT0, GL_SPOT_CUTOFF, spotCutoff);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);
	glLightf (GL_LIGHT0, GL_SPOT_EXPONENT, spotExponent);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light0_specular);
	glLightf (GL_LIGHT1, GL_QUADRATIC_ATTENUATION, linearAttenuation);
	glLightf (GL_LIGHT1, GL_SPOT_CUTOFF, spotCutoff);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
	glLightf (GL_LIGHT1, GL_SPOT_EXPONENT, spotExponent);
	glLightfv(GL_LIGHT2, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, light0_specular);
	glLightf (GL_LIGHT2, GL_QUADRATIC_ATTENUATION, linearAttenuation);
	glLightf (GL_LIGHT2, GL_SPOT_CUTOFF, spotCutoff);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_direction);
	glLightf (GL_LIGHT2, GL_SPOT_EXPONENT, spotExponent);
	glLightfv(GL_LIGHT3, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT3, GL_SPECULAR, light0_specular);
	glLightf (GL_LIGHT3, GL_QUADRATIC_ATTENUATION, linearAttenuation);
	glLightf (GL_LIGHT3, GL_SPOT_CUTOFF, spotCutoff);
	glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, spot_direction);
	glLightf (GL_LIGHT3, GL_SPOT_EXPONENT, spotExponent);
	glLightfv(GL_LIGHT4, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT4, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT4, GL_SPECULAR, light0_specular);
	glLightf (GL_LIGHT4, GL_QUADRATIC_ATTENUATION, linearAttenuation);
	glLightf (GL_LIGHT4, GL_SPOT_CUTOFF, spotCutoff);
	glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, spot_direction);
	glLightf (GL_LIGHT4, GL_SPOT_EXPONENT, spotExponent);
	glLightfv(GL_LIGHT5, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT5, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT5, GL_SPECULAR, light0_specular);
	glLightf (GL_LIGHT5, GL_QUADRATIC_ATTENUATION, linearAttenuation);
	glLightf (GL_LIGHT5, GL_SPOT_CUTOFF, spotCutoff);
	glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, spot_direction);
	glLightf (GL_LIGHT5, GL_SPOT_EXPONENT, spotExponent);
	glLightfv(GL_LIGHT6, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT6, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT6, GL_SPECULAR, light0_specular);
	glLightf (GL_LIGHT6, GL_QUADRATIC_ATTENUATION, linearAttenuation);
	glLightf (GL_LIGHT6, GL_SPOT_CUTOFF, spotCutoff);
	glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, spot_direction);
	glLightf (GL_LIGHT6, GL_SPOT_EXPONENT, spotExponent);
	glLightfv(GL_LIGHT7, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT7, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT7, GL_SPECULAR, light0_specular);
	glLightf (GL_LIGHT7, GL_QUADRATIC_ATTENUATION, linearAttenuation);
	glLightf (GL_LIGHT7, GL_SPOT_CUTOFF, spotCutoff);
	glLightfv(GL_LIGHT7, GL_SPOT_DIRECTION, spot_direction);
	glLightf (GL_LIGHT7, GL_SPOT_EXPONENT, spotExponent);
	
	glClearColor (0.0, 0.0, 0.0, 0.0);
	//glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);// descrive come i dati bitmap sono memorizzati nel computer
	glGenTextures(1, &texName);// da il nome al texture object
	glBindTexture(GL_TEXTURE_2D, texName);// prima chiamata:crea il texture object
	
	// parametri vari per la texture
	CGU::TGAImg img;
  	img.load("logo_unimib.tga"); 
  	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
}

//funzione di supporto per convertire coordinate mondo in interi indici della matrice di caratteri
int converter(float x) {
	return (int)(((x + 10) / 20) - 1) + 1;
}

//funzione Level Detector, date le coordinate dell'asse z della camera restituisce un intero 0-1 che indica il piano
int LD(float x) {
	if (x < 60.1) return 0;
	return 1;
}

//funzione per il supporto della gestione delle collisioni con le pareti e le finestre
bool collisionControl(float x, float y, float z) {
	if (bFree) return true;
	const char c = cMatrix[converter(x)][converter(y)][LD(z)];
	if ((c != 'r') && (c != 'a')) return true;
	return false;
}

//funzione di supporto per la gestione delle collisioni tra soffitto e pavimento
bool collisionControlUpDown(float x, float y, float z, bool d) {
	if (bFree) return true;
	const char c = cMatrix[converter(x)][converter(y)][LD(z)];
	bool t = false;
	if (LD(z) == 1) t = true;
	if (c == 'm') {
		if (d) {
			if (z < 115)
				return true;
		}
		else {
			if (z > 5)
				return true;
		}
	}
	if (d) {//se la direzione è verso l'alto
		if (t) {//se si è al piano di sopra
			if (z < 115)
				return true;
		}
		else {
			if (z < 55)
				return true;
		}
	}
	else {
		if (t) {
			if (z > 65)
				return true;
		}
		else {
			if (z > 5)
				return true;
		}
	}
	return false;
}

//Callback di rendering
void renderScene() {
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//cancello il depth buffer e il color buffer 
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);//scelgo la modelview come matrice corrente.
	glLoadIdentity();//carico la matrice identità sulla modelview (quindi la resetto)
	//Impostazione della camera attraverso manipolazione di un frame ortonormale
	if (freelook_flag) {
		Matrix4x4 m;
		//Cambio la visuale con il mouse
		buildRotationMatrix(camera.frame.Yaxis, deg2rad(-yrot), m);
		transform(m, camera.frame.Xaxis);
		transform(m, camera.frame.Yaxis);
		transform(m, camera.frame.Zaxis);
		float cpx, cpz;
		
		//Mi sposto con la tastiera.
		//if (up_key || down_key || left_key || right_key || top_key || under_key) {
		//	printf("%i %f %i\n", converter(camera.position.x), camera.position.y, converter(camera.position.z));
		//	printf("%c\n", cMatrix[converter(camera.position.x)][converter(camera.position.z)][0]);
		//}
		if (up_key) {
			cpx = camera.position.x - camera.frame.Zaxis.x * speed;
			cpz = camera.position.z - camera.frame.Zaxis.z * speed;
			if (collisionControl(cpx, cpz, camera.position.y)) {
				camera.position.x += -camera.frame.Zaxis.x * speed;
				camera.position.y += -camera.frame.Zaxis.y * speed;
				camera.position.z += -camera.frame.Zaxis.z * speed;
			}
		}
		if (down_key) {
			cpx = camera.position.x + camera.frame.Zaxis.x * speed;
			cpz = camera.position.z + camera.frame.Zaxis.z * speed;
			if (collisionControl(cpx, cpz, camera.position.y)) {
				camera.position.x += camera.frame.Zaxis.x * speed;
				camera.position.z += camera.frame.Zaxis.z * speed;
				camera.position.y += camera.frame.Zaxis.y * speed;
			}
		}
		if (left_key) {
			cpx = camera.position.x - camera.frame.Xaxis.x * speed;
			cpz = camera.position.z - camera.frame.Xaxis.z * speed;
			if (collisionControl(cpx, cpz, camera.position.y)) {
				camera.position.x += -camera.frame.Xaxis.x * speed;
				camera.position.y += -camera.frame.Xaxis.y * speed;
				camera.position.z += -camera.frame.Xaxis.z * speed;
			}
		}
		if (right_key) {
			cpx = camera.position.x + camera.frame.Xaxis.x * speed;
			cpz = camera.position.z + camera.frame.Xaxis.z * speed;
			if (collisionControl(cpx, cpz, camera.position.y)) {
				camera.position.x += camera.frame.Xaxis.x * speed;
				camera.position.y += camera.frame.Xaxis.y * speed;
				camera.position.z += camera.frame.Xaxis.z * speed;
			}
		}
		if (top_key) {
			cpx = camera.position.x + camera.frame.Yaxis.x * speed;
			cpz = camera.position.z + camera.frame.Yaxis.z * speed;
			if (collisionControlUpDown(cpx, cpz, camera.position.y, true)) {
				camera.position.x += camera.frame.Yaxis.x * speed;
				camera.position.y += camera.frame.Yaxis.y * speed;
				camera.position.z += camera.frame.Yaxis.z * speed;
			}
		}
		if (under_key) {
			cpx = camera.position.x - camera.frame.Yaxis.x * speed;
			cpz = camera.position.z - camera.frame.Yaxis.z * speed;
			if (collisionControlUpDown(cpx, cpz, camera.position.y, false)) {
				camera.position.x += -camera.frame.Yaxis.x * speed;
				camera.position.y += -camera.frame.Yaxis.y * speed;
				camera.position.z += -camera.frame.Yaxis.z * speed;
			}
		}
    }

	pitch += xrot;
	if (pitch > 90) pitch = 90;
	if (pitch < -90) pitch = -90;
	Camera temp(camera);
	Matrix4x4 m2;
	buildRotationMatrix(temp.frame.Xaxis, deg2rad(pitch), m2);
	transform(m2, temp.frame.Xaxis);
	if (xFree) transform(m2, temp.frame.Yaxis);
	if (xFree) transform(m2, temp.frame.Zaxis);
	Matrix4x4 c; 
	buildCameraMatrix(temp, c);
	glMultMatrixf(c.GL_array());

	if (freelook_flag)
		if(mouse_moved) {
			//Forzo il puntatore del mouse al centro della finestra. Questo artificio è chiaramente "brutto", necessario dato che glut non mette a disposizioni funzioni per leggere il movimento relativo del mouse.
			mouse_moved = false;
			glutWarpPointer(width/2, height/2);
		}
	xrot = yrot = 0;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLdouble ratio = static_cast<GLdouble>(width) / static_cast<GLdouble>(height);
	gluPerspective(45,		//angolo di visuale
					ratio,	//aspect ratio (rapporto tra larghezza e altezza)
					1,		//Z near cutting plane
					1000);	//Z far cutting plane
	glClearColor (0.0, 0.0, 0.0, 0.0); 
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//cancello i buffers
	
	glEnable(GL_TEXTURE_2D);   // attiva il texturing

	// setto il modo di disegno a GL_DECAL (adesivo), il poligono è disegnato
	// usando i colori della texture, senza tenere conto degli eventuali
	// colori che avrebbe se la texture non ci fosse
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	
	glBindTexture(GL_TEXTURE_2D, texName);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-32 + textLoc[0], -16 + textLoc[1], 0 + textLoc[2]);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(-32 + textLoc[0], 16  + textLoc[1], 0 + textLoc[2]);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(0  + textLoc[0], 16  + textLoc[1], 0 + textLoc[2]);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(0  + textLoc[0], -16 + textLoc[1], 0 + textLoc[2]);
	glEnd(); 
	
	glDisable(GL_TEXTURE_2D);

	
	float fGND = 60.1; // altezza da terra per disegnare i poligoni dei diversi piani
	for (int p = 0; p < 2; p++) {
		for (int x = 0; x < (dimensioni[0] * 20); x += 20) {
			for (int z = 0; z < (dimensioni[1] * 20); z += 20) {
				if ((cMatrix[x / 20][z / 20][p] == 'w') || (cMatrix[x / 20][z / 20][p] == 'g')) {
					glBegin(GL_QUADS);//pavimento
						imposta_materiale_pavimento();
						glNormal3f(0, 1, 0);
						glVertex3f( 0 + x, 0 + fGND * p,  0 + z);
						glVertex3f(20 + x, 0 + fGND * p,  0 + z);
						glVertex3f(20 + x, 0 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 0 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//soffitto
						imposta_materiale_soffitto();
						glNormal3f(0, -1, 0);
						glVertex3f( 0 + x, 60 + fGND * p,  0 + z);
						glVertex3f(20 + x, 60 + fGND * p,  0 + z);
						glVertex3f(20 + x, 60 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 60 + fGND * p, 20 + z);
					glEnd();
				}
				if ((p == 0) && (cMatrix[x / 20][z / 20][0] == 'm')) {
					glBegin(GL_QUADS);//pavimento
						imposta_materiale_pavimento();
						glNormal3f(0, 1, 0);
						glVertex3f( 0 + x, 0 + fGND * p,  0 + z);
						glVertex3f(20 + x, 0 + fGND * p,  0 + z);
						glVertex3f(20 + x, 0 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 0 + fGND * p, 20 + z);
					glEnd();
				}
				if ((p == 1) && (cMatrix[x / 20][z / 20][1] == 'm')) {	
					glBegin(GL_QUADS);//soffitto
						imposta_materiale_soffitto();
						glNormal3f(0, -1, 0);
						glVertex3f( 0 + x, 60 + fGND * p,  0 + z);
						glVertex3f(20 + x, 60 + fGND * p,  0 + z);
						glVertex3f(20 + x, 60 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 60 + fGND * p, 20 + z);
					glEnd();
				}
				if (cMatrix[x / 20][z / 20][p] == 'r') {
					glBegin(GL_QUADS);//parete 1/4
						imposta_materiale_parete();
						glNormal3f(-1, 0, 0);
						glVertex3f(0 + x,  0 + fGND * p,  0 + z);
						glVertex3f(0 + x, 60 + fGND * p,  0 + z);
						glVertex3f(0 + x, 60 + fGND * p, 20 + z);
						glVertex3f(0 + x,  0 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 2/4
						imposta_materiale_parete();
						glNormal3f(0, 0, -1);
						glVertex3f( 0 + x,  0 + fGND * p, 0 + z);
						glVertex3f( 0 + x, 60 + fGND * p, 0 + z);
						glVertex3f(20 + x, 60 + fGND * p, 0 + z);
						glVertex3f(20 + x,  0 + fGND * p, 0 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 3/4
						imposta_materiale_parete();
						glNormal3f(1, 0, 0);
						glVertex3f(20 + x,  0 + fGND * p,  0 + z);
						glVertex3f(20 + x, 60 + fGND * p,  0 + z);
						glVertex3f(20 + x, 60 + fGND * p, 20 + z);
						glVertex3f(20 + x,  0 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 4/4
						imposta_materiale_parete();
						glNormal3f(0, 0, 1);
						glVertex3f(20 + x,  0 + fGND * p, 20 + z);
						glVertex3f(20 + x, 60 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 60 + fGND * p, 20 + z);
						glVertex3f( 0 + x,  0 + fGND * p, 20 + z);
					glEnd();
				}
				if (cMatrix[x / 20][z / 20][p] == 'a') {
					glBegin(GL_QUADS);//sotto finestra
						imposta_materiale_parete();
						glNormal3f(0, 1, 0);
						glVertex3f( 0 + x, 20 + fGND * p,  0 + z);
						glVertex3f(20 + x, 20 + fGND * p,  0 + z);
						glVertex3f(20 + x, 20 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 20 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//sopra finestra
						imposta_materiale_parete();
						glNormal3f(0, -1, 0);
						glVertex3f( 0 + x, 40 + fGND * p,  0 + z);
						glVertex3f(20 + x, 40 + fGND * p,  0 + z);
						glVertex3f(20 + x, 40 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 40 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 1/4 sotto
						imposta_materiale_parete();
						glNormal3f(-1, 0, 0);
						glVertex3f(0 + x,  0 + fGND * p,  0 + z);
						glVertex3f(0 + x, 20 + fGND * p,  0 + z);
						glVertex3f(0 + x, 20 + fGND * p, 20 + z);
						glVertex3f(0 + x,  0 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 2/4 sotto
						imposta_materiale_parete();
						glNormal3f(0, 0, -1);
						glVertex3f( 0 + x,  0 + fGND * p, 0 + z);
						glVertex3f( 0 + x, 20 + fGND * p, 0 + z);
						glVertex3f(20 + x, 20 + fGND * p, 0 + z);
						glVertex3f(20 + x,  0 + fGND * p, 0 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 3/4 sotto
						imposta_materiale_parete();
						glNormal3f(1, 0, 0);
						glVertex3f(20 + x,  0 + fGND * p,  0 + z);
						glVertex3f(20 + x, 20 + fGND * p,  0 + z);
						glVertex3f(20 + x, 20 + fGND * p, 20 + z);
						glVertex3f(20 + x,  0 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 4/4 sotto
						imposta_materiale_parete();
						glNormal3f(0, 0, 1);
						glVertex3f(20 + x,  0 + fGND * p, 20 + z);
						glVertex3f(20 + x, 20 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 20 + fGND * p, 20 + z);
						glVertex3f( 0 + x,  0 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 1/4 sopra
						imposta_materiale_parete();
						glNormal3f(-1, 0, 0);
						glVertex3f(0 + x, 40 + fGND * p,  0 + z);
						glVertex3f(0 + x, 60 + fGND * p,  0 + z);
						glVertex3f(0 + x, 60 + fGND * p, 20 + z);
						glVertex3f(0 + x, 40 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 2/4 sopra
						imposta_materiale_parete();
						glNormal3f(0, 0, -1);
						glVertex3f( 0 + x, 40 + fGND * p, 0 + z);
						glVertex3f( 0 + x, 60 + fGND * p, 0 + z);
						glVertex3f(20 + x, 60 + fGND * p, 0 + z);
						glVertex3f(20 + x, 40 + fGND * p, 0 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 3/4 sopra
						imposta_materiale_parete();
						glNormal3f(1, 0, 0);
						glVertex3f(20 + x, 40 + fGND * p,  0 + z);
						glVertex3f(20 + x, 60 + fGND * p,  0 + z);
						glVertex3f(20 + x, 60 + fGND * p, 20 + z);
						glVertex3f(20 + x, 40 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//parete 4/4 sopra
						imposta_materiale_parete();
						glNormal3f(0, 0, 1);
						glVertex3f(20 + x, 40 + fGND * p, 20 + z);
						glVertex3f(20 + x, 60 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 60 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 40 + fGND * p, 20 + z);
					glEnd();
				}
				if (cMatrix[x / 20][z / 20][p] == 'y') {
					glBegin(GL_QUADS);//pavimento
						imposta_materiale_pavimento();
						glNormal3f(0, 1, 0);
						glVertex3f( 0 + x, 0 + fGND * p,  0 + z);
						glVertex3f(20 + x, 0 + fGND * p,  0 + z);
						glVertex3f(20 + x, 0 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 0 + fGND * p, 20 + z);
					glEnd();
					glBegin(GL_QUADS);//luce
						imposta_materiale_lampada();
						glNormal3f(0, -1, 0);
						glVertex3f( 0 + x, 60 + fGND * p,  0 + z);
						glVertex3f(20 + x, 60 + fGND * p,  0 + z);
						glVertex3f(20 + x, 60 + fGND * p, 20 + z);
						glVertex3f( 0 + x, 60 + fGND * p, 20 + z);
					glEnd();
				}
			}
		}
	}
	switch (nLuci) {
		case 8: {
			GLfloat light7_position[] = {locLuci[7][0] + 10, locLuci[7][2], locLuci[7][1] + 10, 1.0};
			glLightfv(GL_LIGHT7, GL_POSITION, light7_position);
		}
		case 7: {
			GLfloat light6_position[] = {locLuci[6][0] + 10, locLuci[6][2], locLuci[6][1] + 10, 1.0};
			glLightfv(GL_LIGHT6, GL_POSITION, light6_position);
		}
		case 6: {
			GLfloat light5_position[] = {locLuci[5][0] + 10, locLuci[5][2], locLuci[5][1] + 10, 1.0};
			glLightfv(GL_LIGHT5, GL_POSITION, light5_position);
		}
		case 5: {
			GLfloat light4_position[] = {locLuci[4][0] + 10, locLuci[4][2], locLuci[4][1] + 10, 1.0};
			glLightfv(GL_LIGHT4, GL_POSITION, light4_position);
		}
		case 4: {
			GLfloat light3_position[] = {locLuci[3][0] + 10, locLuci[3][2], locLuci[3][1] + 10, 1.0};
			glLightfv(GL_LIGHT3, GL_POSITION, light3_position);
		}
		case 3: {
			GLfloat light2_position[] = {locLuci[2][0] + 10, locLuci[2][2], locLuci[2][1] + 10, 1.0};
			glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
		}
		case 2: {
			GLfloat light1_position[] = {locLuci[1][0] + 10, locLuci[1][2], locLuci[1][1] + 10, 1.0};
			glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
		}
		case 1: {
			GLfloat light0_position[] = {locLuci[0][0] + 10, locLuci[0][2], locLuci[0][1] + 10, 1.0};
			glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
		}
	}
	glFlush();
	glutSwapBuffers();
}

//Funzione di idle: forzo il redrawing 
void idle (void) {
	glutPostRedisplay();
}

//Handler della tastiera
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'q':
			printf("%s\n", "quitting");
			exit(EXIT_SUCCESS);
			break;
		case 'u':
			freelook_flag = !freelook_flag;
			if (freelook_flag) {
				printf("%s\n", "locking the look\nRendering!");
				glutSetCursor(GLUT_CURSOR_NONE);
				glutWarpPointer(width/2,height/2);
			}else {
				printf("%s\n", "unlocking the look\nRendering!");
				glutSetCursor(GLUT_CURSOR_INHERIT);
			}
			break;
		case 'e':
			up_key = true;
			break;
		case 's':
			left_key = true;
			break;
		case 'f':
			right_key = true;
			break;
		case 'd':
			down_key = true;
			break;
		case 'a':
			top_key = true;
			break;
		case 'z':
			under_key = true;
			break;
		case '1':
			printf("%s\n", "switching to Camera 1");
			cameraPos[cC][0] = camera.position.x;
			cameraPos[cC][1] = camera.position.y;
			cameraPos[cC][2] = camera.position.z;
			cC = 0;
			camera.position.set(cameraPos[cC][0], cameraPos[cC][1], cameraPos[cC][2]);
			break;
		case '2':
			printf("%s\n", "switching to Camera 2");
			cameraPos[cC][0] = camera.position.x;
			cameraPos[cC][1] = camera.position.y;
			cameraPos[cC][2] = camera.position.z;
			cC = 1;
			camera.position.set(cameraPos[cC][0], cameraPos[cC][1], cameraPos[cC][2]);
			break;
		case '3':
			printf("%s\n", "switching to Camera 3");
			cameraPos[cC][0] = camera.position.x;
			cameraPos[cC][1] = camera.position.y;
			cameraPos[cC][2] = camera.position.z;
			cC = 2;
			camera.position.set(cameraPos[cC][0], cameraPos[cC][1], cameraPos[cC][2]);
			break;
		case '4':
			printf("%s\n", "switching to Camera 4");
			cameraPos[cC][0] = camera.position.x;
			cameraPos[cC][1] = camera.position.y;
			cameraPos[cC][2] = camera.position.z;
			cC = 3;
			camera.position.set(cameraPos[cC][0], cameraPos[cC][1], cameraPos[cC][2]);
			break;
		case 'p':
			printf("%s\n", "moving the texture to the current camerta position");
			textLoc[0] = camera.position.x;
			textLoc[1] = camera.position.y;
			textLoc[2] = camera.position.z;
			break;
		case 'o':
			if (bFree) {
				printf("%s\n", "collision control enabled");
				bFree = false;
			}
			else {
				printf("%s\n", "collision control disabled");
				bFree = true;
			}
			break;
		case 'x':
			if (xFree) {
				printf("%s\n", "y and z axis mouse control turned off");
				xFree = false;
			}
			else {
				printf("%s\n", "y and z axis mouse control turned on");
				xFree = true;
			}
	}
}

//Handler della tastiera per quando i tasti vengono rilasciati
void keyboardUp(unsigned char key, int x, int y) {
	switch (key) {
		case 'e':
			up_key = false;
			break;
		case 's':
			left_key = false;
			break;
		case 'f':
			right_key = false;
			break;
		case 'd':
			down_key = false;
			break;
		case 'a':
			top_key = false;
			break;
		case 'z':
			under_key =false;
			break;
	}
}

//Callback per il movimento passivo del mouse.
void processMousePassiveMotion(int x, int y) {
	if (freelook_flag) {//Nota: il movimento relativo lo ricavo come scostamento dal centro della finestra. Ad ogni chiamata di rendering, il mouse viene forzato al centro della finestra. Questo è un artificio "brutto ma necessario" poichè glut non fornisce funzioni che forniscono scostamenti relativi del mouse.
		xrot = y - height / 2;
		yrot = x - width / 2;
		mouse_moved = true;
	}
}

//Callback per la gestione dei clic del mouse
void processMouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		//lock del mouse in caso non lo sia
		if (!freelook_flag) {
			freelook_flag = true;
			glutSetCursor(GLUT_CURSOR_NONE);
			glutWarpPointer(width / 2,height / 2);
		}
	}
}

//Handler per gli eventi di resizing della finestra.
void reshape(int w, int h) {
	width = w;
	height = h;
	glViewport(0, 0, width, height);
	if (freelook_flag) glutWarpPointer(width / 2, height / 2);
}

//funzione che legge il contenuto dei file tga e riempie la matrice cMatrix
int readPiantina(int iPiano, char* sFileName) {
	printf("%s %s\n", sFileName, "selected as a .tga file");
	printf("%s %s\n", "loading", sFileName);
	CGU::TGAImg img;
	img.load(sFileName);
	printf("%s %s\n", sFileName, "loaded");
	printf("%s %s %i\n", sFileName, "height:", img.height());
	if (((img.height() != dimensioni[0]) && (dimensioni[0] != 0)) || ((img.width() != dimensioni[1]) && (dimensioni[1] != 0))) {
		printf("%s\n", "error on files [height]x[width]");
		return EXIT_FAILURE;
	}
	dimensioni[0] = img.height();
	printf("%s %s %i\n", sFileName, "width:", img.width());
	dimensioni[1] = img.width();
	printf("%s %s %i\n", sFileName, "bit per pixel:", img.BPP());
	printf("%s %s%s\n", "reading", sFileName, "'s pixels");
	const unsigned char* pMatrix = img.pixels();
	float altezzaLuce = 55.0;
	if (iPiano == 1) altezzaLuce += 60.0;
	for (int h = 0; h < img.height(); h++) {
		for (int w = 0; w < img.width(); w++) {
			if (((int)*pMatrix ==   0) && ((int)*(pMatrix + 1) ==   0) && ((int)*(pMatrix + 2) ==   0)) cMatrix[h][w][iPiano] = 'b';
			if (((int)*pMatrix == 255) && ((int)*(pMatrix + 1) == 255) && ((int)*(pMatrix + 2) == 255)) cMatrix[h][w][iPiano] = 'w';
			if (((int)*pMatrix ==   0) && ((int)*(pMatrix + 1) ==   0) && ((int)*(pMatrix + 2) == 255)) cMatrix[h][w][iPiano] = 'a';
			if (((int)*pMatrix == 255) && ((int)*(pMatrix + 1) ==   0) && ((int)*(pMatrix + 2) ==   0)) cMatrix[h][w][iPiano] = 'r';
			if (((int)*pMatrix == 255) && ((int)*(pMatrix + 1) ==   0) && ((int)*(pMatrix + 2) == 255)) cMatrix[h][w][iPiano] = 'm';
			if (((int)*pMatrix == 255) && ((int)*(pMatrix + 1) == 255) && ((int)*(pMatrix + 2) ==   0)) {
				cMatrix[h][w][iPiano] = 'y';
				if (nLuci > 8) {
					printf("%s\n", "found more than 8 lights!");
					return EXIT_FAILURE;
				}
				locLuci[nLuci][0] = h * 20;
				locLuci[nLuci][1] = w * 20;
				locLuci[nLuci][2] = altezzaLuce;
				if (nLuci < 8) nLuci++;
			}
			if (((int)*pMatrix ==   0) && ((int)*(pMatrix + 1) == 255) && ((int)*(pMatrix + 2) ==   0)) {
				if (nCamere) {
					printf("%s\n", "found more than one camera's positions!");
					return EXIT_FAILURE;
				}
				else {
					nCamere = true;
					printf("%s\n", "found the camera's position");
					cMatrix[h][w][iPiano] = 'g';
				}
			}
			pMatrix += (img.BPP() / 8);
		}
	}
	return 0;
}

//main
int main(int argc, char** argv) {
	int errors = 0;
	printf("%s %s\n", "starting esame.exe version", VERSION);
	if (argc != 3) {
		printf("%s\n", "correct syntax: esame <file.tga> <file.tga>");
		return EXIT_FAILURE;
	}
	printf("%s\n", " - - - reading first file - - -");
	errors += readPiantina(0, argv[1]);
	printf("%s\n", " - - - reading second file - - -");
	errors += readPiantina(1, argv[2]);
	if (errors > 0) return EXIT_FAILURE;
	if (!nCamere) {
		printf("%s\n", "camera's position not found!");
		return EXIT_FAILURE;
	}
	for (int i = 0; i < dimensioni[0]; i++)
		for (int c = 0; c < dimensioni[1]; c++)
			if ((cMatrix[i][c][0] == 'm') && (cMatrix[i][c][1] != 'm')) {
				printf("%s\n", "no compatible maps");
				return EXIT_FAILURE;
			}
	printf("%s\n", " - - - end of reading - - -");
	printf("%s %i\n", "number of lights found:", nLuci);
	printf("%s\n", "initializing glut");
	glutInit(&argc, argv);
	printf("%s\n", "setting display mode GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA");
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);//imposto modalità in double buffering e alloco il depth buffer
	printf("%s\n", "setting window's position 10 10");
	glutInitWindowPosition(10, 10);
	printf("%s\n", "setting window's size 800 600");
	glutInitWindowSize(800, 600);
	printf("%s\n", "creating window");
	glutCreateWindow("Esame di Elementi di Informatica Grafica");
	//effettuo le chiamate OpenGL solo dopo aver creato il contesto OpenGL, ovvero dopo la creazione della finestra
	printf("%s\n", "initializing scene");
	initGL();//inizializzo la scena (chiamate openGL)
	//Registro le callback GLUT
	printf("%s\n", "setting keyboard's callback");
	glutKeyboardFunc(keyboard);
	printf("%s\n", "setting keyboard's up callback");
	glutKeyboardUpFunc(keyboardUp);
	printf("%s\n", "setting the rendering scene's function");
	glutDisplayFunc(renderScene);//funzione di disegno della scena
	printf("%s\n", "setting the idle's function (forcing scene's redraw)");
	glutIdleFunc(idle);
	printf("%s\n", "setting the reshape function for the window");
	glutReshapeFunc(reshape);
	printf("%s\n", "setting mouse's click's handler");
	glutMouseFunc(processMouse);
	printf("%s\n", "setting mouse's moves handler");
	glutPassiveMotionFunc(processMousePassiveMotion);
	printf("%s\n", "setting the main glut loop\nRendering!");
	glutMainLoop();//(bloccante!)
	return 0;
}