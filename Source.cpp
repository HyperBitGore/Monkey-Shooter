#include "JoeEngine3D.h"


//add shooting(draw long cube to monkey, get starting point centered)
//add score system
//add death and reset system
//monke spawning
int main() {
	GLFWwindow* wind = Joe::Engine::initGL(1024, 768);
	GLuint lightID = Joe::Files::LoadShaders("vertexshader.glsl", "fragmentshader.glsl");
	GLuint colorID = Joe::Files::LoadShaders("vertshader.glsl", "fragshader.glsl");

	GLuint texture1 = Joe::Files::loadBMP_Texture("xoK5F.bmp");
	GLuint texture2 = Joe::Files::loadBMP_Texture("redtexture.bmp");
	std::vector<Joe::Model> models;
	Joe::Engine::addModel("monkey.obj", models, texture1);
	Joe::Engine::addModel("floor.obj", models, texture1);
	Joe::Engine::moveModelVertices(&models[1], glm::vec3(0.0, -1.5, 0.0));

	std::vector<Joe::Entity*> entities;
	Joe::Entity e1 = Joe::Engine::createEntity(&models[0]);
	entities.push_back(&e1);
	Joe::Entity e2 = Joe::Engine::createEntity(&models[1]);
	entities.push_back(&e2);
	std::vector<Joe::Entity*> walls;
	walls.push_back(&e2);

	std::vector<Joe::Entity*> monkes;
	monkes.push_back(&e1);

	Joe::Model shoot;
	//make a cone or something here
	//right side triangle
	shoot.vertices.push_back(glm::vec3(0, 0, 5));
	shoot.vertices.push_back(glm::vec3(0, 0, 7));
	shoot.vertices.push_back(glm::vec3(2, 0, 5));
	//left side triangle
	shoot.vertices.push_back(glm::vec3(-2, 0, 5));
	shoot.vertices.push_back(glm::vec3(-2, 0, 7));
	shoot.vertices.push_back(glm::vec3(0, 0, 5));
	shoot.texture = texture2;
	glGenBuffers(1, &shoot.vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, shoot.vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, shoot.vertices.size() * sizeof(glm::vec3), &shoot.vertices[0], GL_STATIC_DRAW);

	Joe::AABB player = Joe::Engine::constructAABB(&models[0]);
	//maybe start from middle of AABB
	glm::vec3 difp = glm::vec3(0, 0, 5) - (player.min);
	Joe::Engine::moveAABB(&player, difp);
	//ray for downward collision of player
	Joe::Ray downray = {glm::vec3(0, 0, 5), glm::vec3(0, 0.01, 0), glm::vec3(0,0,0)};
	//variables needed for main loop
	Joe::Controls control;
	GLuint lightmat = glGetUniformLocation(lightID, "LIGHT");
	GLuint viewmatuniform = glGetUniformLocation(lightID, "V");
	GLuint modlematuniform = glGetUniformLocation(lightID, "M");
	GLuint matrixuniform = glGetUniformLocation(lightID, "MVP");
	GLuint matrix2uniform = glGetUniformLocation(colorID, "MVP");
	glm::vec3 lightpoint(0.5, 0.5, 1.5);
	double lastTime = 0;
	double mouserest = 0;
	//gravity and jumping
	bool falling = true;
	bool jumping = false;
	double jumptime = 0;
	double jumpmax = 0;
	double fallingcool = 0;
	//line drawing
	bool linedraw = false;
	double linecool = 0;

	glfwSetInputMode(wind, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	while (glfwGetKey(wind, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(wind) == 0) {
		double currentTime = glfwGetTime();
		float delta = float(currentTime - lastTime);
		mouserest += delta;
		if (linedraw) {
			linecool += delta;
			if (linecool > 3.0) {
				linedraw = false;
				linecool = 0;
			}
		}
		falling = true;
		control.computeMatricesFromInputs(wind, delta, &player, monkes, &shoot, &linedraw);
		glm::mat4 proj = control.getProjectionMatrix();
		glm::mat4 viewm = control.getViewMatrix();
		//model matrix
		glm::mat4 modm = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, 0.0));
		glm::mat4 MVP = proj * viewm * modm;
		glUseProgram(lightID);
		glUniformMatrix4fv(matrixuniform, 1, GL_FALSE, &MVP[0][0]);
		glUniform3f(lightmat, lightpoint.x, lightpoint.y, lightpoint.z);
		glUniformMatrix4fv(viewmatuniform, 1, GL_FALSE, &viewm[0][0]);
		glUniformMatrix4fv(modlematuniform, 1, GL_FALSE, &modm[0][0]);
		Joe::Engine::drawWindow(wind, entities, &linedraw, &shoot);
		downray.point = control.getPosition();
		downray.point.y -= 0.5;
		if (mouserest > 0.1) {
			glfwSetCursorPos(wind, 1024 / 2, 768 / 2);
			mouserest = 0;
		}
		if (Joe::Engine::AABBcollision(player, e1.bounding)) {
			std::cout << "Collision" << "\n";
		}
		Joe::Ray temp = downray;
		if (Joe::Engine::castRay(&downray, walls, glm::vec3(5, 5, 5)).first) {
			if (downray.distance.y < 0.5) {
				falling = false;
			}
		}
		if (glfwGetKey(wind, GLFW_KEY_SPACE) == GLFW_PRESS && !falling) {
			jumping = true;
		}
		downray = temp;
		if (falling && !jumping) {
			fallingcool += delta;
			if (fallingcool > 0.01) {
				control.editPosition(glm::vec3(0, -0.03, 0));
				fallingcool = 0;
			}
		}
		else if (jumping) {
			jumptime += delta;
			jumpmax += delta;
			if (jumptime > 0.01) {
				control.editPosition(glm::vec3(0, 0.03, 0));
				jumptime = 0;
			}
			if (jumpmax > 0.5) {
				jumping = false;
				jumptime = 0;
				jumpmax = 0;
			}
		}
		lastTime = currentTime;
	}

	return 0;
}