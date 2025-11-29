#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;
in vec4 position_model;
in vec2 texcoords;
in vec3 vertex_color;
// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define MONSTRO 0
#define CUBE  1
#define PLANE  2
#define WALL_NORTH 3
#define WALL_SOUTH 4
#define WALL_EAST 5
#define WALL_WEST 6
#define CEILING 7
#define DRAGON_BOSS 9
#define VARINHA 10
uniform int object_id;
uniform vec4 bbox_min;
uniform vec4 bbox_max;
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3;
uniform sampler2D TextureImage4;
uniform sampler2D TextureImage5;
// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

//Função dotproduct
float dotproduct(vec4 u, vec4 v)
{
    float u1 = u.x;
    float u2 = u.y;
    float u3 = u.z;
    float u4 = u.w;
    float v1 = v.x;
    float v2 = v.y;
    float v3 = v.z;
    float v4 = v.w;

    return u1*v1 + u2*v2 + u3*v3;
}

//Função norm
float norm(vec4 v)
{
    float vx = v.x;
    float vy = v.y;
    float vz = v.z;

    return sqrt( vx*vx + vy*vy + vz*vz );
}

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;
    // Espectro da fonte de iluminação
    vec3 I = vec3(1.0,1.0,1.0);

    // Espectro da luz ambiente
    vec3 Ia = vec3(0.2,0.2,0.2);
    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.5,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);
    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = -l+2*n*(dotproduct(n, l));
    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong
    float U = 0.0;
    float V = 0.0;

    if ( object_id == MONSTRO )
    {
        U = texcoords.x;
        V = texcoords.y;
    }
    else if ( object_id ==  CUBE )
    {
        U = texcoords.x;
        V = texcoords.y;
    }
    else if ( object_id == DRAGON_BOSS )
    {
        Kd = vec3(0.5,0.2,0.2);
        Ks = vec3(0.5,0.5,0.5);
        Ka = vec3(0.2,0.1,0.1);
        q = 32.0;
    }
    else if ( object_id == VARINHA )
    {

        U = texcoords.x;
        V = texcoords.y;
    }
    else if(object_id >= 2 && object_id<= 7)
    {
        if(object_id == PLANE || object_id == CEILING){
            U = position_model.x - floor(position_model.x);
            V = position_model.z - floor(position_model.z);
        }
        else if(object_id == WALL_NORTH || object_id == WALL_SOUTH){
            U = position_model.x - floor(position_model.x);
            V = position_model.y - floor(position_model.y);
        }
        else if(object_id == WALL_EAST || object_id == WALL_WEST){
            U = position_model.z - floor(position_model.z);
            V = position_model.y - floor(position_model.y);
        }
    }
    else
    {
        Kd = vec3(0.0,0.0,0.0);
        Ks = vec3(0.0,0.0,0.0);
        Ka = vec3(0.0,0.0,0.0);
        q = 1.0;
    }

    if (object_id == MONSTRO)
    {
        q=64;
        vec3 pele = texture(TextureImage0, vec2(U,V)).rgb;
        float lambert = max(0,dot(n,l));
        float phong = pow(max(0,dot(r,v)),q);
        color.rgb = pele*lambert*I+pele*Ia+phong*I;
    }
    else if(object_id >= 2 && object_id<= 7)
    {
        vec3 tex;
        if(object_id == PLANE)
        {
            tex = texture(TextureImage2, vec2(U,V)).rgb;
        }
        else if(object_id == CEILING)
        {
            tex = texture(TextureImage3, vec2(U,V)).rgb;
        }
        else
        {
            tex = texture(TextureImage1, vec2(U,V)).rgb;
        }
        color.rgb=vertex_color*tex;
    }
    else if(object_id == CUBE){
        q=32;
        vec3 corpo = texture(TextureImage4, vec2(U,V)).rgb;
        float lambert = max(0,dot(n,l));
        float phong = pow(max(0,dot(r,v)),q);
        color.rgb = corpo*lambert*I+corpo*Ia+phong*I;
    }
    else if(object_id == VARINHA){
        q=8;
        vec3 varinha = texture(TextureImage5, vec2(U,V)).rgb;
        float lambert = max(0,dot(n,l));
        float phong = pow(max(0,dot(r,v)),q);
        color.rgb = varinha*lambert*I+varinha*Ia+phong*I;
    }
    else
    {

        // Termo difuso utilizando a lei dos cossenos de Lambert
        vec3 lambert_diffuse_term = Kd*I*max(dotproduct(n, l), 0.0);

        // Termo ambiente
        vec3 ambient_term = Ka*Ia; // PREENCHA AQUI o termo ambiente

        // Termo especular utilizando o modelo de iluminação de Phong
        vec3 phong_specular_term  = Ks*I*pow(max(dotproduct(r,v),0.0),q);

        // Cor final do fragmento calculada com uma combinação dos termos difuso, especular, e ambiente.
        color.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;
    }
    color.a=1;
    // Cor final com correção gamma, considerando monitor sRGB.
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
}

