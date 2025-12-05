#version 330 core

// ============================================================================
// VERTEX SHADER - Transformações e Iluminação Gouraud
// ============================================================================
//
// Este shader processa cada vértice da geometria e realiza:
// 1. Transformação de coordenadas (modelo → mundo → câmera → NDC)
// 2. Cálculo de iluminação Gouraud para superfícies específicas
//
// REQUISITOS IMPLEMENTADOS:
// - REQUISITO 7: Modelo de interpolação Gouraud
//   (cálculo de iluminação POR VÉRTICE, interpolado pelo rasterizador)
//
// GOURAUD SHADING (Modelo de Interpolação):
//     - A cor é calculada no vertex shader para cada vértice
//     - O rasterizador INTERPOLA essas cores entre os vértices
//     - Resultado: iluminação mais suave mas menos precisa em highlights
//     - Usado aqui para: chão, paredes, teto (superfícies difusas)
//
// A diferença para Phong shading é que em Phong a iluminação é calculada
// no FRAGMENT SHADER (por fragmento), resultando em highlights mais nítidos.
//
// ============================================================================

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
    // ─────────────────────────────────────────────────────────────────────────
    // ILUMINAÇÃO GOURAUD (Por Vértice)
    // ─────────────────────────────────────────────────────────────────────────
    // REQUISITO 7: Modelo de interpolação Gouraud
    //
    // Para objetos da arena (chão, paredes, teto), calculamos a iluminação
    // aqui no vertex shader. A cor resultante (vertex_color) será INTERPOLADA
    // pelo rasterizador e recebida pelo fragment shader.
    //
    // Usamos apenas iluminação DIFUSA (Lambert) para estas superfícies:
    //     I_difusa = Kd * I_luz * max(n · l, 0) * atenuação
    //
    // onde:
    //     Kd = coeficiente de reflexão difusa do material
    //     I_luz = intensidade/cor da fonte de luz
    //     n = vetor normal da superfície (normalizado)
    //     l = vetor direção para a luz (normalizado)
    //     atenuação = 1 / (1 + 0.7*d + 1.8*d²) para simular queda com distância
    // ─────────────────────────────────────────────────────────────────────────
    vec3 Kd;
    vec3 Ka;
    if(object_id >= 2 && object_id<= 7){
        // Normal em coordenadas do mundo (para cálculo de iluminação)
        vec4 n = normalize(normal);
        vec4 world_pos = model * model_coefficients;

        // Luz ambiente (iluminação base mesmo sem luz direta)
        vec3 Ia = vec3(0.10, 0.08, 0.05);

        // Define coeficientes de material baseado no tipo de superfície
        if(object_id == PLANE){
            Kd = vec3(0.2,0.2,0.2);  // Chão mais escuro
            Ka = vec3(0.1,0.1,0.1);
        }
        else{
            Kd = vec3(0.3,0.3,0.3);  // Paredes mais claras
            Ka = vec3(0.15,0.15,0.15);
        }

        // Acumula contribuição de todas as tochas (fontes de luz pontuais)
        vec3 diffuse = vec3(0.0);
        for(int i = 0; i < num_torches; i++)
        {
            // Vetor do vértice para a luz
            vec3 to_light = torch_positions[i] - world_pos.xyz;
            float dist = length(to_light);
            vec3 l = to_light / dist;  // Direção normalizada

            // Atenuação quadrática (luz enfraquece com distância)
            float attenuation = 1.0 / (1.0 + 0.7 * dist + 1.8 * dist * dist);

            // GOURAUD: Lambert diffuse calculado POR VÉRTICE
            // O resultado será interpolado pelo rasterizador
            float NdotL = max(dot(n.xyz, l), 0.0);
            diffuse += torch_colors[i] * NdotL * torch_intensities[i] * attenuation;
        }

        // Cor final do vértice = ambiente + difusa (será interpolada)
        vertex_color = Ka * Ia + Kd * diffuse;
    }
    else
    {
        vertex_color = vec3(1.0,1.0,1.0);
    }

}

