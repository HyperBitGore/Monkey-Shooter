#include "JoeEngine3D.h"
#include <glm/gtx/vector_angle.hpp>
#include <random>
#include <time.h>

Joe::Entity spawnMonkey(glm::vec3 pos, Joe::Model *model, std::vector<Joe::Entity*>& monkes, std::vector<Joe::Entity*>& entities) {
	Joe::Entity* e = new Joe::Entity;
	*e = Joe::Engine::createEntity(model);
	glm::vec3 middle = e->bounding.min + (e->bounding.max - glm::abs(e->bounding.min));
	glm::vec3 dif(0,0,0);
	if (middle.x < pos.x) {
		dif.x = pos.x - middle.x;
	}
	else if (middle.x > pos.x) {
		dif.x =  middle.x - pos.x;
	}
	if (middle.z < pos.z) {
		dif.z = pos.z - middle.z;
	}
	else if (middle.z > pos.z) {
		dif.z = middle.z - pos.z;
	}
	Joe::Engine::moveEntityVertices(e, dif);
	Joe::Engine::moveAABB(&e->bounding, dif);
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
//easier if they're global
GLuint monkeyvertexbuffer, monkeyuvbuffer, monkeynormalbuffer;
std::vector<glm::vec3> monkeyvertices;
std::vector<glm::vec2> monkeyuvs;
std::vector<glm::vec3> monkeynormals;

void resizeMonkeyBuffers(std::vector<Joe::Entity*>& monkes ) {
	monkeyvertices.clear();
	monkeyuvs.clear();
	monkeynormals.clear();
	for (auto& j : monkes) {
		for (auto& i : j->vertices) {
			monkeyvertices.push_back(i);
		}
		for (auto& i : j->uvs) {
			monkeyuvs.push_back(i);
		}
		for (auto& i : j->normals) {
			monkeynormals.push_back(i);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, monkeyvertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, monkeyvertices.size() * sizeof(glm::vec3), &monkeyvertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, monkeyuvbuffer);
	glBufferData(GL_ARRAY_BUFFER, monkeyuvs.size() * sizeof(glm::vec2), &monkeyuvs[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, monkeynormalbuffer);
	glBufferData(GL_ARRAY_BUFFER, monkeynormals.size() * sizeof(glm::vec3), &monkeynormals[0], GL_STATIC_DRAW);
}

//fix monkes getting stuck(from collision)
//shooting sound effect
//rotate monkeys to look at player
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
	spawnMonkey(glm::vec3(0.5, 0.5, 0.5), &models[0], monkes, entities);

	Joe::Entity e2 = Joe::Engine::createEntity(&models[1]);
	entities.push_back(&e2);
	std::vector<Joe::Entity*> walls;
	walls.push_back(&e2);

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
	//variables needed for main rendering loop
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
	//spawning
	double monkeyspawn = 0;
	double monkeycool = 5.0;
	//generating buffers for monkeys
	for (auto& i : monkes[0]->vertices) {
		monkeyvertices.push_back(i);
	}
	for (auto& i : monkes[0]->uvs) {
		monkeyuvs.push_back(i);
	}
	for (auto& i : monkes[0]->normals) {
		monkeynormals.push_back(i);
	}
	glGenBuffers(1, &monkeyvertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, monkeyvertexbuffer);
	glBufferData(GL_ARRAY_BUFFER,  monkeyvertices.size() * sizeof(glm::vec3), &monkeyvertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &monkeyuvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, monkeyuvbuffer);
	glBufferData(GL_ARRAY_BUFFER, monkeyuvs.size() * sizeof(glm::vec2), &monkeyuvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &monkeynormalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, monkeynormalbuffer);
	glBufferData(GL_ARRAY_BUFFER, monkeynormals.size() * sizeof(glm::vec3), &monkeynormals[0], GL_STATIC_DRAW);
	//monkey collision
	std::vector<Joe::Entity*> acold;

	glfwSetInputMode(wind, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	while (glfwGetKey(wind, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(wind) == 0) {
		double currentTime = glfwGetTime();
		float delta = float(currentTime - lastTime);
		mouserest += delta;
		monkeyspawn += delta;
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
		if (monkeyspawn > monkeycool) {
			int rx = rand() % 100;
			int rz = rand() % 2 + 1;
			glm::vec3 sp(0);
			if (rx > 50) {
				sp.x = -8;
			}
			else {
				sp.x = 8;
			}
			sp.z = float(rand() % 9);
			if (rz > 1) {
				sp.z = -sp.z;
			}
			spawnMonkey(sp, &models[0], monkes, entities);
			monkeyspawn = 0;
		}
		resizeMonkeyBuffers(monkes);
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
		Joe::Engine::drawWindow(wind, monkeyvertices, { texture1, texture1 }, { monkeyvertexbuffer, e2.vertexbuffer }, { monkeyuvbuffer, e2.uvbuffer }, 
		{ monkeynormalbuffer, e2.normalbuffer }, &linedraw, &shoot, colorID);
		downray.point = control.getPosition();
		downray.point.y -= 0.5;
		if (mouserest > 0.1) {
			glfwSetCursorPos(wind, 1024 / 2, 768 / 2);
			mouserest = 0;
		}
		acold.clear();
		for (int i = 0; i < monkes.size();) {
			glm::vec3 move(0, 0, 0);
			glm::vec3 middle = player.min + (player.max - glm::abs(player.min));
			glm::vec3 monkemiddle = monkes[i]->bounding.min + (monkes[i]->bounding.max - glm::abs(monkes[i]->bounding.min));
			glm::vec3 da = glm::normalize(monkemiddle);
			glm::vec3 db = glm::normalize(middle);
			float angle = glm::acos(glm::dot(da, db));
			float rx = angle, rz = angle;
			if (angle < 0.1) {
				angle = 0.1;
			}
			if (monkemiddle.x < middle.x) {
				move.x = angle * delta * 1.5f;
				rx = angle * delta;
			}
			else if (monkemiddle.x > middle.x) {
				move.x = -angle * delta * 1.5f;
				rx = -angle * delta;
			}
			if (monkemiddle.z < middle.z) {
				move.z = angle * delta * 1.5f;
				rz = angle * delta;
			}
			else if (monkemiddle.z > middle.z) {
				move.z = -angle * delta * 1.5f;
				rz = -angle * delta;
			}
			Joe::Engine::moveEntityVertices(monkes[i], move);
			Joe::Engine::moveAABB(&monkes[i]->bounding, move);
			Joe::Ray ra = { monkes[i]->bounding.min, glm::vec3(rx, 0, rz), glm::vec3(0) };
			if (Joe::Engine::castRay(&ra, monkes, glm::vec3(1.5, 0, 1.5), monkes[i]).first) {
				glm::vec3 moveneg(0);
				moveneg.x = -move.x;
				moveneg.z = -move.z;
				Joe::Engine::moveEntityVertices(monkes[i], moveneg);
				Joe::Engine::moveAABB(&monkes[i]->bounding, moveneg);
			}
			for (auto& j : monkes) {
				while (Joe::Engine::AABBcollision(monkes[i]->bounding, j->bounding) && monkes[i] != j) {
					glm::vec3 moveneg(0);
					moveneg.x = -move.x;
					moveneg.z = -move.z;
					Joe::Engine::moveEntityVertices(monkes[i], moveneg);
					Joe::Engine::moveAABB(&monkes[i]->bounding, moveneg);
				}
			}
			if (Joe::Engine::AABBcollision(player, monkes[i]->bounding)) {
				phealth -= 10;
				std::cout << "colliding\n";
			}
			if (monkes[i]->health <= 0) {
				deleteMonkey(monkes[i], entities);
				monkes.erase(monkes.begin() + i);
			}
			else {
				i++;
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
			monkeyspawn = 0;
			monkeycool = 5.0f;
		}
		lastTime = currentTime;
	}

	return 0;
}