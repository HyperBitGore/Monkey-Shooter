#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

namespace Joe {
	struct AABB {
		//std::vector<glm::vec3> points;
		glm::vec3 min;
		glm::vec3 max;
	};
	struct Model {
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		GLuint vertexbuffer;
		GLuint normalbuffer;
		GLuint uvbuffer;
		GLuint texture;
	};
	struct Entity : Model {
		AABB bounding;
		int health;
	};
	struct Ray {
		glm::vec3 point;
		glm::vec3 inc;
		glm::vec3 distance;
	};

	class Files {
	public:
		static GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

			// Create the shaders
			GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
			GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

			// Read the Vertex Shader code from the file
			std::string VertexShaderCode;
			std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
			if (VertexShaderStream.is_open()) {
				std::stringstream sstr;
				sstr << VertexShaderStream.rdbuf();
				VertexShaderCode = sstr.str();
				VertexShaderStream.close();
			}
			else {
				printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
				getchar();
				return 0;
			}

			// Read the Fragment Shader code from the file
			std::string FragmentShaderCode;
			std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
			if (FragmentShaderStream.is_open()) {
				std::stringstream sstr;
				sstr << FragmentShaderStream.rdbuf();
				FragmentShaderCode = sstr.str();
				FragmentShaderStream.close();
			}

			GLint Result = GL_FALSE;
			int InfoLogLength;

			// Compile Vertex Shader
			printf("Compiling shader : %s\n", vertex_file_path);
			char const* VertexSourcePointer = VertexShaderCode.c_str();
			glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
			glCompileShader(VertexShaderID);

			// Check Vertex Shader
			glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (InfoLogLength > 0) {
				std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
				glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
				printf("%s\n", &VertexShaderErrorMessage[0]);
			}

			// Compile Fragment Shader
			printf("Compiling shader : %s\n", fragment_file_path);
			char const* FragmentSourcePointer = FragmentShaderCode.c_str();
			glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
			glCompileShader(FragmentShaderID);

			// Check Fragment Shader
			glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (InfoLogLength > 0) {
				std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
				glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
				printf("%s\n", &FragmentShaderErrorMessage[0]);
			}

			// Link the program
			printf("Linking program\n");
			GLuint ProgramID = glCreateProgram();
			glAttachShader(ProgramID, VertexShaderID);
			glAttachShader(ProgramID, FragmentShaderID);
			glLinkProgram(ProgramID);

			// Check the program
			glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
			glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
			if (InfoLogLength > 0) {
				std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
				glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
				printf("%s\n", &ProgramErrorMessage[0]);
			}

			glDetachShader(ProgramID, VertexShaderID);
			glDetachShader(ProgramID, FragmentShaderID);

			glDeleteShader(VertexShaderID);
			glDeleteShader(FragmentShaderID);

			return ProgramID;
		}
		static bool loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uv, std::vector<glm::vec3>& outnormals) {
			std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
			std::vector<glm::vec3> temp_vertices;
			std::vector<glm::vec2> temp_uvs;
			std::vector<glm::vec3> temp_normals;
			FILE* file;
			fopen_s(&file, path, "r");
			if (file == NULL) {
				printf("Failed to open the file\n");
				return false;
			}
			char lineHeader[128];
			int res = fscanf_s(file, "%s", lineHeader, 128);
			while (res != EOF) {
				if (std::strcmp(lineHeader, "v") == 0) {
					//vertex
					glm::vec3 vertex;
					fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
					temp_vertices.push_back(vertex);
				}
				else if (std::strcmp(lineHeader, "vt") == 0) {
					//uv
					glm::vec2 uv;
					fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
					temp_uvs.push_back(uv);
				}
				else if (std::strcmp(lineHeader, "vn") == 0) {
					glm::vec3 normal;
					fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
					temp_normals.push_back(normal);
				}
				else if (strcmp(lineHeader, "f") == 0) {
					std::string vertex1, vertex2, vertex3;
					unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
					int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2], &vertexIndex[3], &uvIndex[3], &normalIndex[3]);
					if (matches == 9) {
						vertexIndices.push_back(vertexIndex[0]);
						vertexIndices.push_back(vertexIndex[1]);
						vertexIndices.push_back(vertexIndex[2]);
						uvIndices.push_back(uvIndex[0]);
						uvIndices.push_back(uvIndex[1]);
						uvIndices.push_back(uvIndex[2]);
						normalIndices.push_back(normalIndex[0]);
						normalIndices.push_back(normalIndex[1]);
						normalIndices.push_back(normalIndex[2]);
					}
					else if (matches == 12) {
						vertexIndices.push_back(vertexIndex[0]);
						vertexIndices.push_back(vertexIndex[1]);
						vertexIndices.push_back(vertexIndex[2]);
						vertexIndices.push_back(vertexIndex[3]);
						uvIndices.push_back(uvIndex[0]);
						uvIndices.push_back(uvIndex[1]);
						uvIndices.push_back(uvIndex[2]);
						uvIndices.push_back(uvIndex[3]);
						normalIndices.push_back(normalIndex[0]);
						normalIndices.push_back(normalIndex[1]);
						normalIndices.push_back(normalIndex[2]);
						normalIndices.push_back(normalIndex[3]);
					}
					else {
						printf("File can't be read by our simple parser : ( Try exporting with other options\n");
						return false;
					}
				}
				res = fscanf_s(file, "%s", lineHeader, 128);
			}
			for (size_t i = 0; i < vertexIndices.size(); i++) {
				glm::vec3 vertex = temp_vertices[vertexIndices[i] - 1];
				out_vertices.push_back(vertex);
			}
			for (size_t i = 0; i < uvIndices.size(); i++) {
				glm::vec2 uv = temp_uvs[uvIndices[i] - 1];
				out_uv.push_back(uv);
			}
			for (size_t i = 0; i < normalIndices.size(); i++) {
				glm::vec3 normal = temp_normals[normalIndices[i] - 1];
				outnormals.push_back(normal);
			}

			return true;
		}
		static GLuint loadBMP_Texture(const char* imgpath) {
			unsigned char header[54];
			unsigned int dataPos;
			unsigned int w, h;
			unsigned int imagesize; // = width*heigh*3
			//RGB data
			unsigned char* data = nullptr;
			FILE* file;
			fopen_s(&file, imgpath, "rb");
			if (!file) { printf("Image could not be opened\n"); return 0; }
			if (fread(header, 1, 54, file) != 54) { // If not 54 bytes read : problem
				printf("Not a correct BMP file\n");
				return -1;
			}
			if (header[0] != 'B' || header[1] != 'M') {
				printf("Not a correct BMP file\n");
				return -1;
			}
			dataPos = *(int*)&(header[0x0A]);
			imagesize = *(int*)&(header[0x22]);
			w = *(int*)&(header[0x12]);
			h = *(int*)&(header[0x16]);
			// Some BMP files are misformatted, guess missing information
			if (imagesize == 0)    imagesize = w * h * 3; // 3 : one byte for each Red, Green and Blue component
			if (dataPos == 0)      dataPos = 54; // The BMP header is done that way
			//allocate memory for data and then read file data into memory
			data = new unsigned char[imagesize];
			fread(data, 1, imagesize, file);
			fclose(file);
			//create texture
			GLuint textureID;
			glGenTextures(1, &textureID);
			//bind texture so future function calls will use this texture
			glBindTexture(GL_TEXTURE_2D, textureID);
			//give image data to opengl
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
			//magnifying image use linear filtering
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//minifying image use linear blend of two mipmaps
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glGenerateMipmap(GL_TEXTURE_2D);
			return textureID;
			//delete[] data;
		}
	};

	class Engine {
	public:
		//sets up basic GL
		static GLFWwindow* initGL(int w, int h) {
			srand(time(NULL));
			glewExperimental = true;
			if (glfwInit() == GLFW_FALSE) {
				std::cout << "Failed to init glfw" << std::endl;
			}
			glfwWindowHint(GLFW_SAMPLES, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			GLFWwindow* wind;
			wind = glfwCreateWindow(w, h, "Learn OpenGL Intermediate", NULL, NULL);
			glfwMakeContextCurrent(wind);
			if (glewInit() != GLEW_OK) {
				std::cout << "Failed to init glew" << std::endl;
			}
			glfwSetInputMode(wind, GLFW_STICKY_KEYS, GL_TRUE);
			//enable the depth testing
			glEnable(GL_DEPTH_TEST);
			//Accept fragment if close to camera than last one
			glDepthFunc(GL_LESS);
			glEnable(GL_CULL_FACE);

			GLuint VertexArrayID;
			glGenVertexArrays(1, &VertexArrayID);
			glBindVertexArray(VertexArrayID);
			return wind;
		}
		//inits unique buffers for a model
		static void initBuffers(Model *model) {
			glGenBuffers(1, &model->vertexbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, model->vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, model->vertices.size() * sizeof(glm::vec3), &model->vertices[0], GL_STATIC_DRAW);
			glGenBuffers(1, &model->uvbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, model->uvbuffer);
			glBufferData(GL_ARRAY_BUFFER, model->uvs.size() * sizeof(glm::vec2), &model->uvs[0], GL_STATIC_DRAW);
			glGenBuffers(1, &model->normalbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, model->normalbuffer);
			glBufferData(GL_ARRAY_BUFFER, model->normals.size() * sizeof(glm::vec3), &model->normals[0], GL_STATIC_DRAW);
		}
		//Just draws raw models with no collision objects attached
		static void drawWindow(GLFWwindow* wind, std::vector<Model>& models) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			std::vector<glm::vec3> vertices;
			std::vector<glm::vec2> uvs;
			std::vector<glm::vec3> normals;
			for (auto& i : models) {
				glBindTexture(GL_TEXTURE_2D, i.texture);
				//vertex attrib
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, i.vertexbuffer);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
				//uv attrib
				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, i.uvbuffer);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
				//normal attrib
				glEnableVertexAttribArray(2);
				glBindBuffer(GL_ARRAY_BUFFER, i.normalbuffer);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
				glDrawArrays(GL_TRIANGLES, 0, i.vertices.size() * 3);
				//std::copy(i.vertices.begin(), i.vertices.end(), std::back_inserter(vertices));
				//std::copy(i.uvs.begin(), i.uvs.end(), std::back_inserter(uvs));
				//std::copy(i.normals.begin(), i.normals.end(), std::back_inserter(normals));
			}
		

			glfwSwapBuffers(wind);
			glfwPollEvents();
		}
		//overload for use if you want to use Entities instead of models
		static void drawWindow(GLFWwindow* wind, std::vector<Entity*>& models, bool* drawline, Model* shoot, GLuint colorID, GLuint matrixuniform) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			std::vector<glm::vec3> vertices;
			std::vector<glm::vec2> uvs;
			std::vector<glm::vec3> normals;
			for (auto& i : models) {
				glBindTexture(GL_TEXTURE_2D, i->texture);
				//vertex attrib
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, i->vertexbuffer);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
				//uv attrib
				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, i->uvbuffer);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
				//normal attrib
				glEnableVertexAttribArray(2);
				glBindBuffer(GL_ARRAY_BUFFER, i->normalbuffer);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
				glDrawArrays(GL_TRIANGLES, 0, i->vertices.size() * 3);
				//std::copy(i.vertices.begin(), i.vertices.end(), std::back_inserter(vertices));
				//std::copy(i.uvs.begin(), i.uvs.end(), std::back_inserter(uvs));
				//std::copy(i.normals.begin(), i.normals.end(), std::back_inserter(normals));
			}
			glm::mat4 Ortho = glm::ortho(-(1024.0f / 2.0f), 1024.0f/2.0f, 768.0f/2.0f, -(768.0f/2.0f), -1000.0f, 1000.0f);
			glm::mat4 _guiMVP;
			glm::mat4 _guiview = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
			glm::mat4 _guimodel = glm::mat4();
			_guiMVP = Ortho * _guiview * _guimodel;
			if (*drawline) {
				glDisable(GL_DEPTH_TEST);
				//glUniformMatrix4fv(matrixuniform, 1, GL_FALSE, &_guiMVP[0][0]);
				glUseProgram(colorID);
				glBindTexture(GL_TEXTURE_2D, shoot->texture);
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, shoot->vertexbuffer);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
				glDrawArrays(GL_TRIANGLES, 0, 3);
				glEnable(GL_DEPTH_TEST);
			}

			glfwSwapBuffers(wind);
			glfwPollEvents();
		}
		//creates a model struct and loads the obj data from file
		static void addModel(const char* filepath, std::vector<Model>& models, GLuint texture) {
			Model m;
			Joe::Files::loadOBJ(filepath, m.vertices, m.uvs, m.normals);
			initBuffers(&m);
			m.texture = texture;
			models.push_back(m);
		}
		//this is slow as compared to changing model matrices, but i dont really care
		static void moveModelVertices(Model* model, glm::vec3 change) {
			for (auto& i : model->vertices) {
				i.x += change.x;
				i.y += change.y;
				i.z += change.z;
			}
			glBindBuffer(GL_ARRAY_BUFFER, model->vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, model->vertices.size() * sizeof(glm::vec3), &model->vertices[0], GL_STATIC_DRAW);
		}
		//this is slow as compared to changing model matrices, but i dont really care
		static void moveEntityVertices(Entity* model, glm::vec3 change) {
			for (auto& i : model->vertices) {
				i.x += change.x;
				i.y += change.y;
				i.z += change.z;
			}
			glBindBuffer(GL_ARRAY_BUFFER, model->vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, model->vertices.size() * sizeof(glm::vec3), &model->vertices[0], GL_STATIC_DRAW);
		}
		static AABB constructAABB(Model* model) {
			AABB box;
			//get min/max for x, then do y, then do z;
			//construct the box points from that
			glm::vec3 ptmin(model->vertices[0]);
			glm::vec3 ptmax(model->vertices[0]);
			for (auto& i : model->vertices) {
				ptmin.x = glm::min(ptmin.x, i.x);
				ptmax.x = glm::max(ptmax.x, i.x);
				ptmin.y = glm::min(ptmin.y, i.y);
				ptmax.y = glm::max(ptmax.y, i.y);
				ptmin.z = glm::min(ptmin.z, i.z);
				ptmax.z = glm::max(ptmax.z, i.z);
			}
			box.max = ptmax;
			box.min = ptmin;

			return box;
		}
		static void moveAABB(AABB* box, glm::vec3 change) {
			box->max += change;
			box->min += change;
		}
		static bool AABBcollision(AABB one, AABB two) {
			if (one.min.x <= two.max.x && one.max.x >= two.min.x &&
				one.min.y <= two.max.y && one.max.y >= two.min.y &&
				one.min.z <= two.max.z && one.max.z >= two.min.z) {
				return true;
			}

			return false;
		}
		static Entity createEntity(Model* model) {
			Entity e;
			e.vertexbuffer = model->vertexbuffer;
			e.uvbuffer = model->uvbuffer;
			e.normalbuffer = model->normalbuffer;
			e.vertices = model->vertices;
			e.uvs = model->uvs;
			e.normals = model->normals;
			e.texture = model->texture;
			e.bounding = constructAABB(&e);
			return e;
		}
		static bool rayCollision(AABB box, Ray r) {
			if (r.point.x >= box.min.x && r.point.x <= box.max.x &&
				r.point.y >= box.min.y && r.point.y <= box.max.y &&
				r.point.z >= box.min.z && r.point.z <= box.max.z) {
				return true;
			}
			return false;
		}
		static std::pair<bool, Entity*> castRay(Ray* r, std::vector<Entity*> entities, glm::vec3 maxwalk) {
			bool colliding = false;
			glm::vec3 start = r->point;
			Entity* ret;
			while (!colliding) {
				for (auto& i : entities) {
					if (rayCollision(i->bounding, *r)) {
						colliding = true;
						ret = i;
					}
				}
				r->point += r->inc;
				r->distance = glm::abs(r->point - start);
				if (r->distance.x > maxwalk.x || r->distance.y > maxwalk.y || r->distance.z > maxwalk.z) {
					return {false, nullptr};
				}
			}
			return {true, ret};
		}
	};


	class Controls {
	private:
		// position
		glm::vec3 position = glm::vec3(0, 0, 5);
		// horizontal angle : toward -Z
		float horizontalAngle = 3.14f;
		// vertical angle : 0, look at the horizon
		float verticalAngle = 0.0f;
		// Initial Field of View
		float initialFoV = 45.0f;
		float speed = 3.0f; // 3 units / second
		float mouseSpeed = 0.005f;
		glm::mat4 Project;
		glm::mat4 View;
	public:
		void computeMatricesFromInputs(GLFWwindow* wind, float delta, AABB *bounding, std::vector<Entity*>& enemies, Model* shoot, bool *drawline) {
			double xpos, ypos;
			glfwGetCursorPos(wind, &xpos, &ypos);
			horizontalAngle += mouseSpeed * delta * float(1024 / 2 - xpos);
			verticalAngle += mouseSpeed * delta * float(768 / 2 - ypos);
			glm::vec3 direction(
				cos(verticalAngle) * sin(horizontalAngle),
				sin(verticalAngle),
				cos(verticalAngle) * cos(horizontalAngle)
			);
			// Right vector
			glm::vec3 right = glm::vec3(
				sin(horizontalAngle - 3.14f / 2.0f),
				0,
				cos(horizontalAngle - 3.14f / 2.0f)
			);
			// Up vector : perpendicular to both direction and right
			glm::vec3 up = glm::cross(right, direction);
			// Move forward
			if (glfwGetKey(wind, GLFW_KEY_W) == GLFW_PRESS) {
				position.x += direction.x * delta * speed;
				position.z += direction.z * delta * speed;
				bounding->max += direction * delta * speed;
				bounding->min += direction * delta * speed;
			}
			// Move backward
			if (glfwGetKey(wind, GLFW_KEY_S) == GLFW_PRESS) {
				position.x -= direction.x * delta * speed;
				position.z -= direction.z * delta * speed;
				bounding->max -= direction * delta * speed;
				bounding->min -= direction * delta * speed;
			}
			// Strafe right
			if (glfwGetKey(wind, GLFW_KEY_D) == GLFW_PRESS) {
				position += right * delta * speed;
				bounding->max += right * delta * speed;
				bounding->min += right * delta * speed;
			}
			// Strafe left
			if (glfwGetKey(wind, GLFW_KEY_A) == GLFW_PRESS) {
				position -= right * delta * speed;
				bounding->max -= right * delta * speed;
				bounding->min -= right * delta * speed;
			}
			if (glfwGetMouseButton(wind, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				Ray ray1 = { position, direction * delta, glm::vec3(0) };
				std::pair<bool, Entity*> output = Engine::castRay(&ray1, enemies, glm::vec3(5.0));
				if (output.first) {
					*drawline = true;
					//damage enemy here
					output.second->health -= 10;
					std::cout << output.second->health << "\n";
				}
			}
			//Projection matrix
			Project = glm::perspective(glm::radians(initialFoV), 4.0f / 3.0f, 0.1f, 100.0f);
			// Camera matrix
			View = glm::lookAt(
				position,           // Camera is here
				position + direction, // and looks here : at the same position, plus "direction"
				up                  // Head is up (set to 0,-1,0 to look upside-down)
			);
		}
		glm::mat4 getProjectionMatrix() {
			return Project;
		}
		glm::mat4 getViewMatrix() {
			return View;
		}
		glm::vec3 getPosition() {
			return position;
		}
		void editPosition(glm::vec3 change) {
			position += change;
		}
	};
}
