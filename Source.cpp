#include "JoeEngine3D.h"
#include <glm/gtx/vector_angle.hpp>

Joe::Entity spawnMonkey(glm::vec3 pos, Joe::Model *model, std::vector<Joe::Entity*>& monkes, std::vector<Joe::Entity*>& entities) {
	Joe::Entity* e = new Joe::Entity;
	*e = Joe::Engine::createEntity(model);
	e->bounding;
	e->vertices;
	e->health = 100;
	monkes.push_back(e);
	entities.push_back(e);
	return *e;
}
void deleteMonkey(Joe::Entity* e, std::vector<Joe::Entity*>& entities) {
	e->vertices.clear();
	e->uvs.clear();
	e->normals.clear();
	for (int i = 0; i < entities.size();) {
		if (entities[i] == e) {
			entities.erase(entities.begin() + i);
			break;
		}
		else {
			i++;
		}
	}
	delete e;
}

//monke spawning(spawn randomly)
//shooting sound effect
int main() {
	GLFWwindow* wind = Joe::Engine::initGL(1024, 768);
	GLuint lightID = Joe::Files::LoadShaders("vertexshader.glsl", "fragmentshader.glsl");
	GLuint colorID = Joe::Files::LoadShaders("vertshader.glsl", "fragshader.glsl");

	GLuint texture1 = Joe::Files::loadBMP_Texture("xoK5F.bmp");
	GLuint texture2 = Joe::Files::loadBMP_Texture("shoot2.bmp");
	std::vector<Joe::Model> models;
	Joe::Engine::addModel("monkey.obj", models, texture1);
	Joe::Engine::addModel("floor.obj", models, texture1);
	Joe::Engine::moveModelVertices(&models[1], glm::vec3(0.0, -1.5, 0.0));

	std::vector<Joe::Entity*> monkes;
	std::vector<Joe::Entity*> entities;
	//Joe::Entity e1 = Joe::Engine::createEntity(&models[0]);
	//e1.health = 100;
	//entities.push_back(&e1);
	spawnMonkey(glm::vec3(0.5, 0.5, 0.5), &models[0], monkes, entities);

	
	Joe::Entity e2 = Joe::Engine::createEntity(&models[1]);
	entities.push_back(&e2);
	std::vector<Joe::Entity*> walls;
	walls.push_back(&e2);

	
	//monkes.push_back(&e1);

	Joe::Model shoot;
	//left triangle
	shoot.vertices.push_back(glm::vec3(0, 0, 50));
	shoot.vertices.push_back(glm::vec3(0, 50, 50));
	shoot.vertices.push_back(glm::vec3(50, 0, 50));
	//right triangle
	shoot.vertices.push_back(glm::vec3(0, 50, 50));
	shoot.vertices.push_back(glm::vec3(50, 50, 50));
	shoot.vertices.push_back(glm::vec3(50, 0, 50));
	//uvs
	shoot.uvs.push_back(glm::vec2(0, 0));
	shoot.uvs.push_back(glm::vec2(0, 1.0));
	shoot.uvs.push_back(glm::vec2(1.0, 0));
	shoot.uvs.push_back(glm::vec2(0, 1));
	shoot.uvs.push_back(glm::vec2(1, 1));
	shoot.uvs.push_back(glm::vec2(1, 0));

	shoot.texture = texture2;
	glGenBuffers(1, &shoot.vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, shoot.vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, shoot.vertices.size() * sizeof(glm::vec3), &shoot.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &shoot.uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, shoot.uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, shoot.uvs.size() * sizeof(glm::vec3), &shoot.uvs[0], GL_STATIC_DRAW);

	Joe::AABB player = {glm::vec3(0, 0, 0.05), glm::vec3(0.5, 1.0, 0.1)};
	Joe::AABB resetp;
	int phealth = 100;
	//maybe start from middle of AABB
	glm::vec3 difp = glm::vec3(0, 0, 5) - (glm::vec3(0.25, 0.5, 0.075));
	Joe::Engine::moveAABB(&player, difp);
	resetp = player;
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
	double shootcool = 0;



	glfwSetInputMode(wind, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	while (glfwGetKey(wind, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(wind) == 0) {
		double currentTime = glfwGetTime();
		float delta = float(currentTime - lastTime);
		mouserest += delta;
		if (linedraw) {
			linecool += delta;
			if (linecool > 0.5) {
				linedraw = false;
				linecool = 0;
			}
		}
		else {
			shootcool += delta;
		}
		falling = true;
		control.computeMatricesFromInputs(wind, delta, &player, monkes, &shoot, &linedraw, &shootcool);
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
		Joe::Engine::drawWindow(wind, entities, &linedraw, &shoot, colorID);
		downray.point = control.getPosition();
		downray.point.y -= 0.5;
		if (mouserest > 0.1) {
			glfwSetCursorPos(wind, 1024 / 2, 768 / 2);
			mouserest = 0;
		}
		for (auto& i : monkes) {
			glm::vec3 move(0, 0, 0);
			glm::vec3 middle;
			middle = player.min + (player.max - glm::abs(player.min));
			glm::vec3 monkemiddle = i->bounding.min + (i->bounding.max - glm::abs(i->bounding.min));
			float angle;
			glm::vec3 da = glm::normalize(monkemiddle);
			glm::vec3 db = glm::normalize(middle);
			angle = glm::acos(glm::dot(da, db));
			if (angle < 0.1) {
				angle = 0.1;
			}
			//move.x = angle * delta;
			if (monkemiddle.x < middle.x) {
				move.x = angle * delta;
			}
			else if (monkemiddle.x > middle.x) {
				move.x = -angle * delta;
			}
			if (monkemiddle.z < middle.z) {
				move.z = angle * delta * 3.0f;
			}
			else if (monkemiddle.z > middle.z) {
				move.z = -angle * delta * 3.0f;
			}
			Joe::Engine::moveEntityVertices(i, move);
			Joe::Engine::moveAABB(&i->bounding, move);
			if (Joe::Engine::AABBcollision(player, i->bounding)) {
				phealth -= 10;
				std::cout << "colliding\n";
			}
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
		if (phealth <= 0) {
			phealth = 100;
			control.reset();
			player = resetp;
			for (int i = 0; i < monkes.size();) {
				deleteMonkey(monkes[i], entities);
				monkes.erase(monkes.begin() + i);
			}
			spawnMonkey(glm::vec3(0.5, 0.5, 0.5), &models[0], monkes, entities);
		}
		for (int i = 0; i < monkes.size();) {
			if (monkes[i]->health <= 0) {
				deleteMonkey(monkes[i], entities);
				monkes.erase(monkes.begin() + i);
			}
			else {
				i++;
			}
		}
		lastTime = currentTime;
	}

	return 0;
}