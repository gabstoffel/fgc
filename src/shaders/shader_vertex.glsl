#version 330 core

// Atributos de vértice recebidos como entrada ("in") pelo Vertex Shader.
// Veja a função BuildTrianglesAndAddToVirtualScene() em "main.cpp".
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 normal_coefficients;
layout (location = 2) in vec2 texture_coefficients;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int object_id;

#define MAX_TORCHES 8
uniform vec3 torch_positions[MAX_TORCHES];
uniform vec3 torch_colors[MAX_TORCHES];
uniform float torch_intensities[MAX_TORCHES];
uniform int num_torches;
// Atributos de vértice que serão gerados como saída ("out") pelo Vertex Shader.
// ** Estes serão interpolados pelo rasterizador! ** gerando, assim, valores
// para cada fragmento, os quais serão recebidos como entrada pelo Fragment
// Shader. Veja o arquivo "shader_fragment.glsl".
out vec4 position_world;
out vec4 position_model;
out vec4 normal;
out vec2 texcoords;
out vec3 vertex_color;
#define PLANE  2
#define WALL_NORTH 3
#define WALL_SOUTH 4
#define WALL_EAST 5
#define WALL_WEST 6
#define CEILING 7

void main()
{
    // A variável gl_Position define a posição final de cada vértice
    // OBRIGATORIAMENTE em "normalized device coordinates" (NDC), onde cada
    // coeficiente estará entre -1 e 1 após divisão por w.
    // Veja {+NDC2+}.
    //
    // O código em "main.cpp" define os vértices dos modelos em coordenadas
    // locais de cada modelo (array model_coefficients). Abaixo, utilizamos
    // operações de modelagem, definição da câmera, e projeção, para computar
    // as coordenadas finais em NDC (variável gl_Position). Após a execução
    // deste Vertex Shader, a placa de vídeo (GPU) fará a divisão por W.

    gl_Position = projection * view * model * model_coefficients;

    // Como as variáveis acima  (tipo vec4) são vetores com 4 coeficientes,
    // também é possível acessar e modificar cada coeficiente de maneira
    // independente. Esses são indexados pelos nomes x, y, z, e w (nessa
    // ordem, isto é, 'x' é o primeiro coeficiente, 'y' é o segundo, ...):
    //
    //     gl_Position.x = model_coefficients.x;
    //     gl_Position.y = model_coefficients.y;
    //     gl_Position.z = model_coefficients.z;
    //     gl_Position.w = model_coefficients.w;
    //

    // Agora definimos outros atributos dos vértices que serão interpolados pelo
    // rasterizador para gerar atributos únicos para cada fragmento gerado.

    // Posição do vértice atual no sistema de coordenadas global (World).
    position_world = model * model_coefficients;
    position_model = model_coefficients;
    // Normal do vértice atual no sistema de coordenadas global (World).
    // Veja slides 123-151 do documento Aula_07_Transformacoes_Geometricas_3D.pdf.
    normal = inverse(transpose(model)) * normal_coefficients;
    normal.w = 0.0;
    texcoords = texture_coefficients;
    vec3 Kd;
    vec3 Ka;
    if(object_id >= 2 && object_id<= 7){
        vec4 n = normalize(normal);
        vec4 world_pos = model * model_coefficients;

        vec3 Ia = vec3(0.10, 0.08, 0.05);

        if(object_id == PLANE){
            Kd = vec3(0.2,0.2,0.2);
            Ka = vec3(0.1,0.1,0.1);
        }
        else{
            Kd = vec3(0.3,0.3,0.3);
            Ka = vec3(0.15,0.15,0.15);
        }

        vec3 diffuse = vec3(0.0);
        for(int i = 0; i < num_torches; i++)
        {
            vec3 to_light = torch_positions[i] - world_pos.xyz;
            float dist = length(to_light);
            vec3 l = to_light / dist;

            float attenuation = 1.0 / (1.0 + 0.7 * dist + 1.8 * dist * dist);

            // Lambert diffuse (Gouraud - computed per vertex, then interpolated)
            float NdotL = max(dot(n.xyz, l), 0.0);
            diffuse += torch_colors[i] * NdotL * torch_intensities[i] * attenuation;
        }

        vertex_color = Ka * Ia + Kd * diffuse;
    }
    else
    {
        vertex_color = vec3(1.0,1.0,1.0);
    }

}

