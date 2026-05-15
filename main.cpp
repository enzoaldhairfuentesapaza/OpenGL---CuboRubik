#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cmath>

const int WIDTH  = 1000;
const int HEIGHT = 800;

void framebuffer_size_callback
(
    GLFWwindow* window,
    int width,
    int height
)
{
    glViewport(0,0,width,height);
}

class Matrix4
{
public:

    float m[16];

    Matrix4()
    {
        for(int i = 0; i < 16; i++)
        {
            m[i] = 0.0f;
        }
    }
};

Matrix4 identityMatrix()
{
    Matrix4 result;

    result.m[0]  = 1.0f;
    result.m[5]  = 1.0f;
    result.m[10] = 1.0f;
    result.m[15] = 1.0f;

    return result;
}

Matrix4 multiplyMatrix
(
    Matrix4 a,
    Matrix4 b
)
{
    Matrix4 result;

    for(int row = 0; row < 4; row++)
    {
        for(int col = 0; col < 4; col++)
        {
            result.m[col + row * 4] =
			a.m[0 + row * 4] * b.m[col + 0 * 4] +
			a.m[1 + row * 4] * b.m[col + 1 * 4] +
			a.m[2 + row * 4] * b.m[col + 2 * 4] +
			a.m[3 + row * 4] * b.m[col + 3 * 4];
        }
    }

    return result;
}

Matrix4 translationMatrix
(
    float x,
    float y,
    float z
)
{
    Matrix4 result = identityMatrix();

    result.m[12] = x;
    result.m[13] = y;
    result.m[14] = z;

    return result;
}

Matrix4 rotationXMatrix(float angle)
{
    Matrix4 result = identityMatrix();

    result.m[5]  = cos(angle);
    result.m[6]  = sin(angle);

    result.m[9]  = -sin(angle);
    result.m[10] = cos(angle);

    return result;
}

Matrix4 rotationYMatrix(float angle)
{
    Matrix4 result = identityMatrix();

    result.m[0]  = cos(angle);
    result.m[2]  = -sin(angle);

    result.m[8]  = sin(angle);
    result.m[10] = cos(angle);

    return result;
}

Matrix4 perspectiveMatrix
(
    float fov,
    float aspect,
    float nearPlane,
    float farPlane
)
{
    Matrix4 result;

    float tanHalfFov =
        tan(fov / 2.0f);

    result.m[0] =
        1.0f / (aspect * tanHalfFov);

    result.m[5] =
        1.0f / tanHalfFov;

    result.m[10] =
        -(farPlane + nearPlane) /
        (farPlane - nearPlane);

    result.m[11] = -1.0f;

    result.m[14] =
        -(2.0f * farPlane * nearPlane) /
        (farPlane - nearPlane);

    return result;
}

const char *vertexShaderSource =
"#version 330 core\n"

"layout (location = 0) in vec3 aPos;\n"

"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"

"void main()\n"
"{\n"

"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"

"}\0";

const char *fragmentShaderSource =
"#version 330 core\n"

"out vec4 FragColor;\n"

"uniform vec3 faceColor;\n"

"void main()\n"
"{\n"

"   FragColor = vec4(faceColor, 1.0f);\n"

"}\n\0";

unsigned int createShader()
{
    unsigned int vertexShader =
        glCreateShader(GL_VERTEX_SHADER);

    glShaderSource
    (
        vertexShader,
        1,
        &vertexShaderSource,
        NULL
    );

    glCompileShader(vertexShader);

    unsigned int fragmentShader =
        glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource
    (
        fragmentShader,
        1,
        &fragmentShaderSource,
        NULL
    );

    glCompileShader(fragmentShader);

    unsigned int shaderProgram =
        glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

enum Face
{
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

class FaceData
{
public:

    float r;
    float g;
    float b;

    std::vector<unsigned int> triangles;
    std::vector<unsigned int> lines;

    FaceData()
    {
        r = 0.0f;
        g = 0.0f;
        b = 0.0f;
    }

    void setColor
    (
        float red,
        float green,
        float blue
    )
    {
        r = red;
        g = green;
        b = blue;
    }
};

class Cube
{
private:

    unsigned int VAO;
    unsigned int VBO;

    float posX;
    float posY;
    float posZ;

    std::vector<float> vertices;

    FaceData faces[6];

public:

    Cube()
    {
        posX = 0;
        posY = 0;
        posZ = 0;

        setupVertices();
        setupFaces();
        setupMesh();
		
        for(int i = 0; i < 6; i++)
        {
            faces[i].setColor(0.5f,0.5f,0.5f);
        }
    }

    FaceData& operator[](Face face)
    {
        return faces[face];
    }

    void setPosition
    (
        float x,
        float y,
        float z
    )
    {
        posX = x;
        posY = y;
        posZ = z;
    }

    void setupVertices()
    {
        vertices =
        {
            -0.45f,-0.45f, 0.45f,
             0.45f,-0.45f, 0.45f,
             0.45f, 0.45f, 0.45f,
            -0.45f, 0.45f, 0.45f,

            -0.45f,-0.45f,-0.45f,
             0.45f,-0.45f,-0.45f,
             0.45f, 0.45f,-0.45f,
            -0.45f, 0.45f,-0.45f
        };
    }

    void setupFaces()
    {
        faces[FRONT].triangles =
        {
            0,1,2,
            2,3,0
        };

        faces[FRONT].lines =
        {
            0,1,
            1,2,
            2,3,
            3,0
        };

        faces[BACK].triangles =
        {
            4,5,6,
            6,7,4
        };

        faces[BACK].lines =
        {
            4,5,
            5,6,
            6,7,
            7,4
        };

        faces[LEFT].triangles =
        {
            4,0,3,
            3,7,4
        };

        faces[LEFT].lines =
        {
            4,0,
            0,3,
            3,7,
            7,4
        };

        faces[RIGHT].triangles =
        {
            1,5,6,
            6,2,1
        };

        faces[RIGHT].lines =
        {
            1,5,
            5,6,
            6,2,
            2,1
        };

        faces[TOP].triangles =
        {
            3,2,6,
            6,7,3
        };

        faces[TOP].lines =
        {
            3,2,
            2,6,
            6,7,
            7,3
        };

        faces[BOTTOM].triangles =
        {
            4,5,1,
            1,0,4
        };

        faces[BOTTOM].lines =
        {
            4,5,
            5,1,
            1,0,
            0,4
        };
    }

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData
        (
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(float),
            vertices.data(),
            GL_STATIC_DRAW
        );

        glVertexAttribPointer
        (
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            3 * sizeof(float),
            (void*)0
        );

        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }


    void drawFace
    (
        unsigned int shader,
        Face face
    )
    {
        Matrix4 model =
            translationMatrix
            (
                posX,
                posY,
                posZ
            );

        glUniformMatrix4fv
        (
            glGetUniformLocation(shader, "model"),
            1,
            GL_FALSE,
            model.m
        );

        glUniform3f
        (
            glGetUniformLocation(shader, "faceColor"),
            faces[face].r,
            faces[face].g,
            faces[face].b
        );

        glBindVertexArray(VAO);

        unsigned int EBO;

        glGenBuffers(1, &EBO);

        glBindBuffer
        (
            GL_ELEMENT_ARRAY_BUFFER,
            EBO
        );

        glBufferData
        (
            GL_ELEMENT_ARRAY_BUFFER,
            faces[face].triangles.size()
            * sizeof(unsigned int),

            faces[face].triangles.data(),

            GL_STATIC_DRAW
        );

        glDrawElements
        (
            GL_TRIANGLES,
            faces[face].triangles.size(),
            GL_UNSIGNED_INT,
            0
        );

        glLineWidth(2.0f);

        glUniform3f
        (
            glGetUniformLocation(shader, "faceColor"),
            0,
            0,
            0
        );

        glBufferData
        (
            GL_ELEMENT_ARRAY_BUFFER,
            faces[face].lines.size()
            * sizeof(unsigned int),

            faces[face].lines.data(),

            GL_STATIC_DRAW
        );

        glDrawElements
        (
            GL_LINES,
            faces[face].lines.size(),
            GL_UNSIGNED_INT,
            0
        );

        glDeleteBuffers(1, &EBO);
    }

    void draw(unsigned int shader)
    {
        drawFace(shader, FRONT);
        drawFace(shader, BACK);
        drawFace(shader, LEFT);
        drawFace(shader, RIGHT);
        drawFace(shader, TOP);
        drawFace(shader, BOTTOM);
    }
};


class RubikCube
{
private:

    std::vector<Cube*> cubes;

public:

    RubikCube()
    {
        generate();
    }

    ~RubikCube()
    {
        for(Cube* cube : cubes)
        {
            delete cube;
        }
    }

    void generate()
    {
        for(int x = -1; x <= 1; x++)
        {
            for(int y = -1; y <= 1; y++)
            {
                for(int z = -1; z <= 1; z++)
                {
					if (x == 0 && y == 0 && z == 0) 
					{
						continue;
					}
                    Cube* cube = new Cube();
                    cube->setPosition
                    (
                        x * 1.5f,
                        y * 1.5f,
                        z * 1.5f
                    );

                    if(z == 1)
                    {
                        (*cube)[FRONT].setColor(1,0,0);
                    }

                    if(z == -1)
                    {
                        (*cube)[BACK].setColor(1,0.5f,0);
                    }

                    if(x == -1)
                    {
                        (*cube)[LEFT].setColor(0,0,1);
                    }

                    if(x == 1)
                    {
                        (*cube)[RIGHT].setColor(0,1,0);
                    }

                    if(y == 1)
                    {
                        (*cube)[TOP].setColor(1,1,1);
                    }

                    if(y == -1)
                    {
                        (*cube)[BOTTOM].setColor(1,1,0);
                    }

                    cubes.push_back(cube);
                }
            }
        }
    }

    void draw(unsigned int shader)
    {
        for(Cube* cube : cubes)
        {
            cube->draw(shader);
        }
    }
};

int main()
{
    glfwInit();

    glfwWindowHint
    (
        GLFW_CONTEXT_VERSION_MAJOR,
        3
    );

    glfwWindowHint
    (
        GLFW_CONTEXT_VERSION_MINOR,
        3
    );

    glfwWindowHint
    (
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    GLFWwindow* window =
        glfwCreateWindow(
            WIDTH,
            HEIGHT,
            "Cubo Rubik",
            NULL,
            NULL
        );

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback
    (
        window,
        framebuffer_size_callback
    );

    gladLoadGL(glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    unsigned int shader =
        createShader();

    RubikCube rubik;

    float cameraZ = -10.0f;

    float rotX = 0.3f;
    float rotY = 0.7f;

    float moveSpeed = 0.05f;

    Matrix4 projection =
        perspectiveMatrix
        (
            45.0f * 3.14159f / 180.0f,
            (float)WIDTH / HEIGHT,
            0.1f,
            100.0f
        );
		
    while(!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_A))
        {
            rotY += moveSpeed;
        }

        if(glfwGetKey(window, GLFW_KEY_D))
        {
            rotY -= moveSpeed;
        }

        if(glfwGetKey(window, GLFW_KEY_W))
        {
            rotX += moveSpeed;
        }

        if(glfwGetKey(window, GLFW_KEY_S))
        {
            rotX -= moveSpeed;
        }

        if(glfwGetKey(window, GLFW_KEY_Q))
        {
            cameraZ += 0.05f;
        }

        if(glfwGetKey(window, GLFW_KEY_E))
        {
            cameraZ -= 0.05f;
        }

        glClearColor
        (
            0.1f,
            0.1f,
            0.1f,
            1.0f
        );

        glClear
        (
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        glUseProgram(shader);


        Matrix4 translation =
            translationMatrix
            (
                0,
                0,
                cameraZ
            );

        Matrix4 rotationX =
            rotationXMatrix(rotX);

        Matrix4 rotationY =
            rotationYMatrix(rotY);

        Matrix4 rotation =
            multiplyMatrix
            (
                rotationY,
                rotationX
            );

        Matrix4 view =
            multiplyMatrix
            (
                rotation,
                translation
            );

        glUniformMatrix4fv
        (
            glGetUniformLocation(shader, "view"),
            1,
            GL_FALSE,
            view.m
        );

        glUniformMatrix4fv
        (
            glGetUniformLocation(shader, "projection"),
            1,
            GL_FALSE,
            projection.m
        );

        rubik.draw(shader);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}