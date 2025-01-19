//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul X - 10_01_modele3D.cpp |
// =============================================
// 
// Program care deseneaza un model 3D importat  

// Biblioteci
#include <windows.h>  // biblioteci care urmeaza sa fie incluse
#include <stdio.h>
#include <stdlib.h> // necesare pentru citirea shader-elor
#include <cstdlib> 
#include <vector>
#include <math.h>
#include <iostream>
#include <GL/glew.h> // glew apare inainte de freeglut
#include <GL/freeglut.h> // nu trebuie uitat freeglut.h

#include "loadShaders.h"

#include "glm/glm.hpp"  
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "objloader.hpp"  
#include <chrono>



//  Identificatorii obiectelor de tip OpenGL;
GLuint
VaoId,
VboId,
ProgramId,
nrVertLocation,
myMatrixLocation,
viewLocation,
lightPosLocation,
projLocation;

GLuint RainProgramId;

// Valoarea lui pi
const float PI = 3.141592;

const float DAY_DURATION = 15.0f; // Duration of a full day in second
const float SUN_RADIUS = 10000.0f; // Radius of the sun's arc
const glm::vec3 SUN_CENTER = glm::vec3(0.0f, 0.0f, 0.0f); // Center of the sun's path

// Variabila pentru numarul de varfuri
int nrVertices;

// Vectori pentru varfuri, coordonate de texturare, normale
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

// Matrice utilizate
glm::mat4 myMatrix;
glm::mat4 view;
glm::mat4 projection;
glm::mat4 matrScale;
glm::mat4 matrScale2;


std::vector<glm::vec3> cloudVertices;
std::vector<glm::vec2> cloudUVs;
std::vector<glm::vec3> cloudNormals;
GLuint cloudVaoId, cloudVboId;


glm::vec3 rainCenter(0.0f, -40.0f, 500.0f);

//	Elemente pentru matricea de vizualizare;
float refX = 0.0f, refY = 0.0f, refZ = 0.0f,
obsX, obsY, obsZ,
vX = 0.0f, vY = 0.0f, vZ = 1.0f;
//	Elemente pentru deplasarea pe sfera;
float alpha = 0.41f, beta = -1.92f, dist = 100.0f,
incrAlpha1 = 0.01, incrAlpha2 = 0.01;
//	Elemente pentru matricea de proiectie;
float width = 800, height = 600, dNear = 4.f, fov = 60.f * PI / 180;





struct RainParticle {
	glm::vec3 position;
	glm::vec3 velocity;
	float lifetime;
};

struct Cloud {
	glm::vec3 position; // Position of the cloud
	float timer;        // Timer to track when to reset the cloud
	float speed;
	glm::vec3 scale;
};


std::vector<RainParticle> rainParticles;
const int NUM_PARTICLES = 100000;
const float rainRadius = 100.0f;
bool isSnow = false;
bool useWeather = false;
int weatherCounter = 1;

float currentCloudTimer=0.0f;
float currentCloudY = 0.0f;
float currentCloudZ = 0.0f;
std::vector<Cloud> clouds;
const int NUM_CLOUDS = 10; // Number of clouds to render


double getElapsedTime() {
	static auto startTime = std::chrono::steady_clock::now(); // Initialize start time
	auto currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsedTime = currentTime - startTime;
	return elapsedTime.count();
}


void InitializeRain() {
	rainParticles.resize(NUM_PARTICLES);
	for (auto& particle : rainParticles) {
		particle.position = glm::vec3(
			rainCenter.x + (rand() % int(rainRadius * 20) - rainRadius * 10) / 10.0f,
			rainCenter.y + (rand() % int(rainRadius * 20) - rainRadius * 10) / 10.0f,
			rainCenter.z + (rand() % int(rainRadius * 20) - rainRadius * 10) / 10.0f
		);
		particle.velocity = glm::vec3(0.0f, 0.0, -(rand() % 5 + 5)); // Downward velocity
		particle.lifetime = float(rand() % 1000) / 10.0f; // Random lifetime
	}
}

void InitializeClouds() {
	clouds.resize(NUM_CLOUDS);
	for (auto& cloud : clouds) {
		cloud.position = glm::vec3(float(rand() % 200 + 200), float(rand() % 1000 - 500), float(rand() % 1000 - 500));
		cloud.timer = getElapsedTime(); // Initialize timers
		cloud.speed = float(rand() % 10 + 5) / 10.0f; // Speed between 0.5 and 1.5
		cloud.scale=glm::vec3(float(rand() % 15 + 10)); 
	}
}




void UpdateRain(float deltaTime) {
	for (auto& particle : rainParticles) {
		particle.position += particle.velocity * deltaTime;
		particle.lifetime -= deltaTime;

		// Reset particles that hit the ground or expire
		if (particle.position.y <= rainCenter.y || particle.lifetime <= 0.0f) {
			particle.position = glm::vec3(
				rainCenter.x + (rand() % int(rainRadius * 20) - rainRadius * 10) / 10.0f,
				rainCenter.y + (rand() % int(rainRadius * 20) - rainRadius * 10) / 10.0f,
				rainCenter.z + (rand() % int(rainRadius * 20) - rainRadius * 10) / 10.0f
			);

			particle.velocity = glm::vec3(0.0f,0.0f, -(rand() % 5 + 5));
			particle.lifetime = float(rand() % 1000) / 10.0f;
		}
	}
}


void RenderRain() {
	glUseProgram(RainProgramId);
	glUniformMatrix4fv(glGetUniformLocation(RainProgramId, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(RainProgramId, "projection"), 1, GL_FALSE, &projection[0][0]);

	glUniform1i(glGetUniformLocation(RainProgramId, "isSnow"), isSnow);


	glBegin(GL_POINTS);
	for (const auto& particle : rainParticles) {
		glVertex3f(particle.position.x, particle.position.y, particle.position.z);
	}
	glEnd();
}









glm::vec3 calculateSunPosition(float timeElapsed) {
	// Normalize time to a fraction of the day
	float normalizedTime = fmod(timeElapsed, DAY_DURATION) / DAY_DURATION;

	// Calculate the angle of the sun in the sky (0 to 2 * PI)
	float angle = normalizedTime * 2.0f * PI;

	// Sun's position along an arc
	float x = SUN_RADIUS * cos(angle) + SUN_CENTER.x;
	float y = SUN_RADIUS * sin(angle) + SUN_CENTER.y; // Height changes
	float z = SUN_CENTER.z; // Sun stays fixed on Z-axis (or can add variation)

	//std::cout << x << " " << y << " " << z << std::endl;

	return glm::vec3(x, y, z);
}


glm::vec4 calculateSkyColor(glm::vec3 sunPosition) {
	// Normalize the sun's height (Y) between -SUN_RADIUS and +SUN_RADIUS
	float normalizedY = glm::clamp((sunPosition.y + SUN_RADIUS) / (2.0f * SUN_RADIUS), 0.0f, 1.0f);
	//std::cout << normalizedY << std::endl;

	// Define key colors for the sky
	glm::vec4 nightColor = glm::vec4(0.0f, 0.0f, 0.05f, 1.0f);  // Pitch black with a slight blue tint
	glm::vec4 dawnColor = glm::vec4(70.0f/255, 76.0/255, 116.0/255, 1.0f);    
	glm::vec4 noonColor = glm::vec4(0.53f, 0.81f, 0.92f, 1.0f); // Light blue for daytime (e.g., glClearColor)

	// Interpolate based on the sun's normalized position
	if (normalizedY < 0.3f) {
		return glm::mix(nightColor, dawnColor, normalizedY / 0.25f);
	}
	else 
		return glm::mix(dawnColor, noonColor, (normalizedY - 0.25f) / 0.5f);
	
}




void processNormalKeys(unsigned char key, int x, int y)
{
	switch (key) {
	case '-':
		dist -= 0.25;	//	apasarea tastelor `+` si `-` schimba pozitia observatorului (se departeaza / aproprie);
		break;
	case '+':
		dist += 0.25;
		break;
	}
	if (key == 27)
		exit(0);
}

void processSpecialKeys(int key, int xx, int yy)
{
	switch (key)				//	Procesarea tastelor 'LEFT', 'RIGHT', 'UP', 'DOWN';
	{							//	duce la deplasarea observatorului pe suprafata sferica in jurul cubului;
	case GLUT_KEY_LEFT:
		beta -= 0.01;
		break;
	case GLUT_KEY_RIGHT:
		beta += 0.01;
		break;
	case GLUT_KEY_UP:
		alpha += incrAlpha1;
		if (abs(alpha - PI / 2) < 0.05)
		{
			incrAlpha1 = 0.f;
		}
		else
		{
			incrAlpha1 = 0.01f;
		}
		break;
	case GLUT_KEY_DOWN:
		alpha -= incrAlpha2;
		if (abs(alpha + PI / 2) < 0.05)
		{
			incrAlpha2 = 0.f;
		}
		else
		{
			incrAlpha2 = 0.01f;
		}
		break;
	}
}

// Se initializeaza un vertex Buffer Object(VBO) pentru transferul datelor spre memoria placii grafice(spre shadere);
// In acesta se stocheaza date despre varfuri;
void CreateVBO(void)
{

	// Generare VAO;
	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);

	// Generare VBO - varfurile si normalele sunt memorate in sub-buffere;
	glGenBuffers(1, &VboId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3) + normals.size() * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), &vertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), normals.size() * sizeof(glm::vec3), &normals[0]);

	// Atributele; 
	glEnableVertexAttribArray(0); // atributul 0 = pozitie
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1); // atributul 1 = normale
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(vertices.size() * sizeof(glm::vec3)));


	//Delete this if it stops working
	glBindVertexArray(0);
}


void CreateCloudVBO() {
	glGenVertexArrays(1, &cloudVaoId);
	glBindVertexArray(cloudVaoId);

	glGenBuffers(1, &cloudVboId);
	glBindBuffer(GL_ARRAY_BUFFER, cloudVboId);
	glBufferData(GL_ARRAY_BUFFER, cloudVertices.size() * sizeof(glm::vec3) + cloudNormals.size() * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, cloudVertices.size() * sizeof(glm::vec3), &cloudVertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, cloudVertices.size() * sizeof(glm::vec3), cloudNormals.size() * sizeof(glm::vec3), &cloudNormals[0]);

	glEnableVertexAttribArray(0); // Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(1); // Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(cloudVertices.size() * sizeof(glm::vec3)));

	glBindVertexArray(0);
}

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void)
{
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

//  Crearea si compilarea obiectelor de tip shader;
void CreateShaders(void)
{
	ProgramId = LoadShaders("10_01_Shader.vert", "10_01_Shader.frag");
	glUseProgram(ProgramId);

	// Load and create the rain shader program
	RainProgramId = LoadShaders("RainShader.vert", "RainShader.frag");
}

// Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
};

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void)
{
	glClearColor(135.0/255, 206.0/255, 235.0/255, 1.0f); // culoarea de fond a ecranului

	// Incarcarea modelului 3D in format OBJ, trebuie sa fie in acelasi director cu proiectul actual;
	bool model = loadOBJ("Plane.obj", vertices, uvs, normals);
	nrVertices = vertices.size();

	bool cloudLoaded = loadOBJ("cloud2.obj", cloudVertices, cloudUVs, cloudNormals);
	if (!cloudLoaded) {
		std::cerr << "Failed to load cloud.obj" << std::endl;
		return;
	}

	// Crearea VBO / shadere-lor
	CreateCloudVBO();
	CreateVBO();
	CreateShaders();

	InitializeRain();
	InitializeClouds();

	// Locatii ptr shader
	nrVertLocation = glGetUniformLocation(ProgramId, "nrVertices");
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
	viewLocation = glGetUniformLocation(ProgramId, "view");
	projLocation = glGetUniformLocation(ProgramId, "projection");
	lightPosLocation = glGetUniformLocation(ProgramId, "lightPos");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Variabile ce pot fi transmise catre shader
	glUniform1i(ProgramId, nrVertices);
}

int x = 0,y=0,z=0;

void RenderClouds() {
	glUniform1i(glGetUniformLocation(ProgramId, "isCloud"), 1); // Set as cloud
	glBindVertexArray(cloudVaoId);

	for (auto& cloud : clouds) {
		// Move the cloud along the X-axis
		cloud.position.x -= 10.0f * (getElapsedTime() - cloud.timer)*cloud.speed; // Move based on elapsed time
		cloud.timer = getElapsedTime(); // Update the timer for consistent movement

		// Reset the cloud if it goes off-screen
		if (cloud.position.x < -300.0f) {
			cloud.position = glm::vec3(300.0f, float(rand() % 800 - 400), float(rand() % 800 - 400)); // Reset position
		}

		// Create the transformation matrix for the cloud
		glm::mat4 cloudMatrix = glm::translate(glm::mat4(1.0f), cloud.position) *
			glm::scale(glm::mat4(1.0f), cloud.scale) *
			glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(0.0, 1.0, 0.0));

		// Pass the transformation matrix to the shader
		glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &cloudMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, cloudVertices.size());
	}

	glBindVertexArray(0);
}



//	Functia de desenare a graficii pe ecran;
void RenderFunction(void)
{

	// Initializare ecran + test de adancime;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(ProgramId);

	matrScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.05, 0.05, 0.05));

	float timeElapsed = getElapsedTime(); // Or use any timer function
	glm::vec3 sunPosition = calculateSunPosition(timeElapsed);
	glUniform3f(lightPosLocation, sunPosition.x, sunPosition.y, sunPosition.z);
	//std::cout << sunPosition.x << sunPosition.y << sunPosition.z<<std::endl;
	//glClearColor(1 / 255, 206.0 / 255, 235.0 / 255, 1.0f);
	glm::vec4 skyColor = calculateSkyColor(sunPosition);
	glClearColor(skyColor.r, skyColor.g, skyColor.b, skyColor.a);
	//glClearColor(1.0, 1.0, 1.0, 1.0);

	
	// Matricea de modelare 
	myMatrix = glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(0.0, 1.0, 0.0))
		* glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(0.0, 0.0, 1.0))*matrScale;
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glPointSize(2.0);

	


	

	//	Vizualizare;
	//	Pozitia observatorului - se deplaseaza pe sfera;
	obsX = refX + dist * cos(alpha) * cos(beta);
	obsY = refY + dist * cos(alpha) * sin(beta);
	obsZ = refZ + dist * sin(alpha);

	//std::cout << obsX << " " << obsY << " " << obsZ << std::endl;
	//	Vectori pentru matricea de vizualizare;
	glm::vec3 obs = glm::vec3(obsX, obsY, obsZ);		//	Pozitia observatorului;	
	glm::vec3 pctRef = glm::vec3(refX, refY, refZ); 	//	Pozitia punctului de referinta;
	glm::vec3 vert = glm::vec3(vX, vY, vZ);			//	Verticala din planul de vizualizare; 
	// Matricea de vizualizare, transmitere catre shader
	view = glm::lookAt(obs, pctRef, vert);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	//	Proiectie;
	projection = glm::infinitePerspective(GLfloat(fov), GLfloat(width) / GLfloat(height), dNear);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

	// "Legarea"VAO, desenare;
	glUniform1i(glGetUniformLocation(ProgramId, "isCloud"), 0); // Set as non-cloud
	glBindVertexArray(VaoId);
	glEnableVertexAttribArray(0); // atributul 0 = pozitie
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	RenderClouds();
	//std::cout << timeElapsed << std::endl;
	if (timeElapsed > 5 *weatherCounter ) {
		std::cout << "test";
		weatherCounter++;
		useWeather = !useWeather;
		isSnow = rand() % 2;
		//isSnow = !isSnow;
	}
	
	if (useWeather) {
		UpdateRain(timeElapsed);
		glUseProgram(RainProgramId);
		RenderRain();
	}
	
	glutSwapBuffers();
	glFlush();
}

//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1200, 900);
	glutCreateWindow("Utilizarea unui model predefinit in format OBJ");
	glewInit();
	Initialize();
	glutIdleFunc(RenderFunction);
	glutDisplayFunc(RenderFunction);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);
	glutCloseFunc(Cleanup);
	glutMainLoop();
}

