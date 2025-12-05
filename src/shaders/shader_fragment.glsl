#version 330 core

// ============================================================================
// FRAGMENT SHADER - Iluminação Phong/Blinn-Phong e Mapeamento de Texturas
// ============================================================================
//
// Este shader processa cada fragmento (pixel) e calcula sua cor final usando:
// 1. Mapeamento de texturas (coordenadas UV)
// 2. Modelo de iluminação de Blinn-Phong (difusa + especular)
// 3. Múltiplas fontes de luz pontuais (tochas)
//
// REQUISITOS IMPLEMENTADOS:
// - REQUISITO 6: Modelos de iluminação difusa (Lambert) e Blinn-Phong
// - REQUISITO 7: Modelo de interpolação Phong (iluminação por fragmento)
// - REQUISITO 8: Mapeamento de texturas em todos os objetos
//
// MODELOS DE ILUMINAÇÃO:
//
// 1. LAMBERT (Difusa):
//     I_difusa = Kd * I_luz * max(n · l, 0)
//     - Luz espalha igualmente em todas as direções
//     - Intensidade proporcional ao cosseno do ângulo
//
// 2. BLINN-PHONG (Especular):
//     I_especular = Ks * I_luz * max(n · h, 0)^q
//     - h = normalize(l + v) é o vetor "halfway" entre luz e visão
//     - Simula reflexão especular (brilhos)
//     - q controla a "dureza" do brilho (maior q = brilho mais concentrado)
//
// A diferença de BLINN-PHONG para PHONG clássico:
//     - Phong: usa vetor de reflexão r = 2(n·l)n - l, calcula r·v
//     - Blinn-Phong: usa vetor halfway h, calcula n·h (mais eficiente)
//
// MODELO DE INTERPOLAÇÃO PHONG vs GOURAUD:
//     - GOURAUD: iluminação calculada por vértice, cor interpolada
//     - PHONG: normal interpolada, iluminação calculada por fragmento
//     - Phong é mais preciso para highlights, Gouraud é mais rápido
//
// ============================================================================

// Atributos interpolados recebidos do vertex shader via rasterizador
// O rasterizador interpola esses valores entre os vértices do triângulo
in vec4 position_world;   // Posição do fragmento em coordenadas do mundo
in vec4 normal;           // Normal interpolada (para Phong shading)
in vec4 position_model;   // Posição em coordenadas do modelo
in vec2 texcoords;        // Coordenadas UV interpoladas
in vec3 vertex_color;     // Cor calculada no vertex shader (Gouraud)
// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
const float M_PI = 3.14159265358979323846;
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
#define PROJECTILE 11
#define PROJECTILE_TRAIL 12
#define ENEMY_PROJECTILE 13
#define ENEMY_PROJECTILE_TRAIL 14
#define PILLAR 15
#define HEALTH_PICKUP 16
#define TORCH 17
#define DYING_ENEMY 18
uniform int object_id;
uniform vec4 bbox_min;
uniform vec4 bbox_max;
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3;
uniform sampler2D TextureImage4;
uniform sampler2D TextureImage5;
uniform sampler2D TextureImage6;
uniform sampler2D TextureImage7;
uniform sampler2D TextureImage8;
uniform sampler2D TextureImage9;
uniform sampler2D TextureImage10;
#define MAX_TORCHES 8
uniform vec3 torch_positions[MAX_TORCHES];
uniform vec3 torch_colors[MAX_TORCHES];
uniform float torch_intensities[MAX_TORCHES];
uniform int num_torches;

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

// ============================================================================
// FUNÇÃO DE ILUMINAÇÃO BLINN-PHONG
// ============================================================================
// REQUISITO 6: Modelo de iluminação difusa (Lambert) e Blinn-Phong
// REQUISITO 7: Modelo de interpolação Phong (iluminação por fragmento)
//
// Esta função calcula a contribuição de luz de todas as tochas usando:
//
// LAMBERT (Difusa):
//     diffuse = Kd * cor_luz * max(n · l, 0) * intensidade * atenuação
//
// BLINN-PHONG (Especular):
//     h = normalize(l + v)  // vetor halfway
//     specular = Ks * cor_luz * pow(max(n · h, 0), shininess) * intensidade * atenuação
//
// Parâmetros:
//     fragPos   - posição do fragmento em coordenadas do mundo
//     normal    - vetor normal da superfície (normalizado)
//     viewDir   - direção do fragmento para a câmera (normalizado)
//     Kd        - coeficiente de reflexão difusa (cor do material)
//     Ks        - coeficiente de reflexão especular
//     shininess - expoente especular (dureza do brilho)
// ============================================================================
vec3 calculateTorchLighting(vec4 fragPos, vec4 normal, vec4 viewDir,
                            vec3 Kd, vec3 Ks, float shininess)
{
    vec3 totalLight = vec3(0.0);

    // Itera sobre todas as tochas (fontes de luz pontuais)
    for (int i = 0; i < num_torches; i++)
    {
        vec3 lightPos = torch_positions[i];
        vec3 lightColor = torch_colors[i];
        float intensity = torch_intensities[i];

        // Vetor do fragmento para a luz
        vec4 lightDir = vec4(lightPos, 1.0) - fragPos;
        float distance = length(lightDir.xyz);
        lightDir = normalize(lightDir);

        // Atenuação com distância (luz enfraquece com d²)
        float attenuation = 1.0 / (1.0 + 0.7 * distance + 1.8 * distance * distance);

        // ─────────────────────────────────────────────────────────────────────
        // COMPONENTE DIFUSA (Lambert)
        // ─────────────────────────────────────────────────────────────────────
        // A intensidade é proporcional ao cosseno do ângulo entre n e l
        // max() garante que superfícies viradas para longe da luz = 0
        float NdotL = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = Kd * lightColor * NdotL * intensity * attenuation;

        // ─────────────────────────────────────────────────────────────────────
        // COMPONENTE ESPECULAR (Blinn-Phong)
        // ─────────────────────────────────────────────────────────────────────
        // Vetor halfway h = normalize(l + v)
        // Blinn-Phong é mais eficiente que Phong clássico pois evita
        // calcular o vetor de reflexão
        vec4 halfwayDir = normalize(lightDir + viewDir);
        float NdotH = max(dot(normal, halfwayDir), 0.0);
        // pow() com shininess controla a "concentração" do brilho
        vec3 specular = Ks * lightColor * pow(NdotH, shininess) * intensity * attenuation;

        totalLight += diffuse + specular;
    }

    return totalLight;
}

vec3 calculateTorchLightingDiffuseOnly(vec4 fragPos, vec4 normal, vec3 Kd)
{
    vec3 totalLight = vec3(0.0);

    for (int i = 0; i < num_torches; i++)
    {
        vec3 lightPos = torch_positions[i];
        vec3 lightColor = torch_colors[i];
        float intensity = torch_intensities[i];

        vec4 lightDir = vec4(lightPos, 1.0) - fragPos;
        float distance = length(lightDir.xyz);
        lightDir = normalize(lightDir);

        float attenuation = 1.0 / (1.0 + 0.7 * distance + 1.8 * distance * distance);

        // Lambert
        float NdotL = max(dot(normal, lightDir), 0.0);
        totalLight += Kd * lightColor * NdotL * intensity * attenuation;
    }

    return totalLight;
}

// ============================================================================
// FUNÇÃO PRINCIPAL DO FRAGMENT SHADER
// ============================================================================
// Calcula a cor final de cada fragmento usando:
// - Amostragem de texturas
// - Modelo de iluminação apropriado para cada tipo de objeto
// - Correção gamma para display sRGB
// ============================================================================
void main()
{
    // Obtém a posição da câmera invertendo a matriz view
    // Usado para calcular o vetor de visão v = normalize(camera - fragmento)
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // Espectro da fonte de iluminação direcional (não usada atualmente)
    vec3 I = vec3(1.2,1.2,1.2);

    // Luz ambiente global (iluminação base da cena)
    vec3 Ia = vec3(0.10, 0.08, 0.05);
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
    vec4 r = -l+2*n*(dotproduct(n, l));
    vec4 h = normalize(l + v);
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
        U = texcoords.x;
        V = texcoords.y;
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

    // ─────────────────────────────────────────────────────────────────────────
    // MONSTRO - Iluminação Blinn-Phong com Texturas
    // ─────────────────────────────────────────────────────────────────────────
    // REQUISITO 6: Modelo de iluminação difusa (Lambert) + Blinn-Phong
    // REQUISITO 7: Modelo de interpolação PHONG (iluminação por fragmento)
    // REQUISITO 8: Mapeamento de texturas
    //
    // A normal 'n' foi INTERPOLADA pelo rasterizador entre os vértices.
    // Calculamos a iluminação aqui no fragment shader (por fragmento),
    // o que caracteriza o modelo de interpolação de PHONG.
    //
    // Comparando com o chão/paredes que usam GOURAUD (vertex shader),
    // os monstros têm highlights especulares mais nítidos e precisos.
    // ─────────────────────────────────────────────────────────────────────────
    if (object_id == MONSTRO)
    {
        // Amostra a textura da pele do monstro nas coordenadas UV
        vec3 pele = texture(TextureImage0, vec2(U,V)).rgb;

        // Define coeficientes de material:
        // Kd = reflexão difusa (usa cor da textura)
        // Ks = reflexão especular (brilhos)
        // Ka = reflexão ambiente
        vec3 Kd_monster = pele;
        vec3 Ks_monster = vec3(0.2);  // Levemente brilhante
        vec3 Ka_monster = pele;

        // Calcula iluminação Blinn-Phong (difusa + especular) para cada tocha
        // shininess = 64 = brilho médio-alto
        vec3 pointLighting = calculateTorchLighting(p, n, v, Kd_monster, Ks_monster, 64.0);

        // Cor final = ambiente + iluminação das tochas
        color.rgb = Ka_monster * Ia + pointLighting;
    }
    else if (object_id == DYING_ENEMY)
    {
        vec3 pele = texture(TextureImage0, vec2(U,V)).rgb;
        vec3 flashColor = vec3(1.0, 0.9, 0.5);
        color.rgb = mix(pele, flashColor, 0.7) * 1.5;
    }
    // ─────────────────────────────────────────────────────────────────────────
    // ARENA (Chão, Paredes, Teto) - Iluminação GOURAUD
    // ─────────────────────────────────────────────────────────────────────────
    // REQUISITO 7: Modelo de interpolação GOURAUD
    // REQUISITO 8: Mapeamento de texturas
    //
    // Para a arena, usamos iluminação GOURAUD:
    // - A iluminação foi calculada no VERTEX SHADER (por vértice)
    // - O valor 'vertex_color' já vem INTERPOLADO pelo rasterizador
    // - Aqui apenas multiplicamos pela textura
    //
    // Resultado: iluminação mais suave, sem highlights especulares nítidos
    // Isso é adequado para superfícies grandes e difusas como paredes.
    //
    // Comparando GOURAUD vs PHONG:
    // - Gouraud: iluminação em 3 vértices → interpolada → multiplica textura
    // - Phong: normal interpolada → iluminação por pixel → mais preciso
    // ─────────────────────────────────────────────────────────────────────────
    else if(object_id >= 2 && object_id<= 7)
    {
        // Seleciona a textura apropriada para cada superfície
        vec3 tex;
        if(object_id == PLANE)
        {
            tex = texture(TextureImage2, vec2(U,V)).rgb;  // Textura do chão
        }
        else if(object_id == CEILING)
        {
            tex = texture(TextureImage3, vec2(U,V)).rgb;  // Textura do teto
        }
        else
        {
            tex = texture(TextureImage1, vec2(U,V)).rgb;  // Textura das paredes
        }
        // Combina cor Gouraud (do vertex shader) com textura
        color.rgb=vertex_color*tex;
    }
    else if(object_id == CUBE){
        vec3 corpo = texture(TextureImage4, vec2(U,V)).rgb;
        vec3 Kd_player = corpo;
        vec3 Ks_player = vec3(0.3);
        vec3 Ka_player = corpo;

        vec3 pointLighting = calculateTorchLighting(p, n, v, Kd_player, Ks_player, 128.0);
        color.rgb = Ka_player * Ia + pointLighting;
    }
    else if(object_id == VARINHA){
        vec3 varinha = texture(TextureImage5, vec2(U,V)).rgb;
        vec3 Kd_wand = varinha;
        vec3 Ks_wand = vec3(0.4);
        vec3 Ka_wand = varinha;

        vec3 pointLighting = calculateTorchLighting(p, n, v, Kd_wand, Ks_wand, 32.0);
        color.rgb = Ka_wand * Ia + pointLighting;
    }
    else if(object_id == DRAGON_BOSS){
        vec3 dragon_tex = texture(TextureImage9, vec2(U,V)).rgb;
        vec3 Kd_dragon = dragon_tex;
        vec3 Ks_dragon = vec3(0.4);  // Shiny scales
        vec3 Ka_dragon = dragon_tex;

        vec3 pointLighting = calculateTorchLighting(p, n, v, Kd_dragon, Ks_dragon, 64.0);
        color.rgb = Ka_dragon * Ia + pointLighting;
    }
    // ─────────────────────────────────────────────────────────────────────────
    // PROJÉTIL - Mapeamento de Textura ESFÉRICO
    // ─────────────────────────────────────────────────────────────────────────
    // REQUISITO 8: Mapeamento de texturas (projeção esférica)
    //
    // Para objetos esféricos como projéteis, usamos coordenadas UV baseadas
    // em coordenadas esféricas (theta, phi) em vez de coordenadas planares:
    //
    //     theta = atan2(x, z)      // ângulo horizontal [-π, π]
    //     phi = asin(y / raio)     // ângulo vertical [-π/2, π/2]
    //
    //     U = (theta + π) / (2π)   // normaliza para [0, 1]
    //     V = (phi + π/2) / π      // normaliza para [0, 1]
    //
    // Isso "envolve" a textura ao redor da esfera de forma natural,
    // similar ao mapeamento de textura de um globo terrestre.
    // ─────────────────────────────────────────────────────────────────────────
    else if(object_id == PROJECTILE)
    {
        // Centro da bounding box (centro da esfera)
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        // Vetor do centro para o ponto atual na superfície
        vec4 vetor_centro = position_model-bbox_center;
        float raio = length(vetor_centro);

        // Converte para coordenadas esféricas
        float arcotangente = atan(vetor_centro.x,vetor_centro.z);  // theta
        float arcsen = asin(vetor_centro.y/raio);                   // phi

        // Normaliza para coordenadas UV [0, 1]
        U = (arcotangente+M_PI)/(2*M_PI);
        V = (arcsen+M_PI/2)/M_PI;

        vec3 magic = texture(TextureImage7, vec2(U,V)).rgb;
        vec3 Kd_proj = magic;
        vec3 Ks_proj = vec3(0.3);
        vec3 Ka_proj = magic;

        vec3 pointLighting = calculateTorchLighting(p, n, v, Kd_proj, Ks_proj, 64.0);
        color.rgb = Ka_proj * Ia + pointLighting;
    }
    else if(object_id == PROJECTILE_TRAIL)
    {
        color.rgb = vec3(0.7, 0.1, 0.7) * 2.5;
    }
    else if(object_id == ENEMY_PROJECTILE)
    {
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        vec4 vetor_centro = position_model-bbox_center;
        float raio = length(vetor_centro);
        float arcotangente = atan(vetor_centro.x,vetor_centro.z);
        float arcsen = asin(vetor_centro.y/raio);
        U = (arcotangente+M_PI)/(2*M_PI);
        V = (arcsen+M_PI/2)/M_PI;

        vec3 fire = texture(TextureImage8, vec2(U,V)).rgb;
        vec3 Kd_enemy_proj = fire;
        vec3 Ks_enemy_proj = vec3(0.3);
        vec3 Ka_enemy_proj = fire;

        vec3 pointLighting = calculateTorchLighting(p, n, v, Kd_enemy_proj, Ks_enemy_proj, 64.0);
        color.rgb = Ka_enemy_proj * Ia + pointLighting;
    }
    else if(object_id == ENEMY_PROJECTILE_TRAIL)
    {
        color.rgb = vec3(1.0, 0.2, 0.0) * 1.8;
    }
    else if(object_id == PILLAR)
    {
        vec3 absPos = abs(position_model.xyz);
        float maxCoord = max(absPos.x, max(absPos.y, absPos.z));
        float pillarU, pillarV;

        if (absPos.x >= maxCoord - 0.01) {
            pillarU = position_world.z - floor(position_world.z);
            pillarV = position_world.y - floor(position_world.y);
        } else if (absPos.z >= maxCoord - 0.01) {
            pillarU = position_world.x - floor(position_world.x);
            pillarV = position_world.y - floor(position_world.y);
        } else {
            pillarU = position_world.x - floor(position_world.x);
            pillarV = position_world.z - floor(position_world.z);
        }

        vec3 tex = texture(TextureImage1, vec2(pillarU, pillarV)).rgb;
        vec3 Kd_pillar = tex;
        vec3 Ka_pillar = tex;

        vec3 pointLighting = calculateTorchLightingDiffuseOnly(p, n, Kd_pillar);
        color.rgb = Ka_pillar * Ia + pointLighting;
    }
    else if(object_id == HEALTH_PICKUP)
    {
        U = texcoords.x;
        V = texcoords.y;
        vec3 health_tex = texture(TextureImage6, vec2(U,V)).rgb;
        vec3 Kd_health = health_tex;
        vec3 Ks_health = vec3(0.2);
        vec3 Ka_health = health_tex;

        vec3 pointLighting = calculateTorchLighting(p, n, v, Kd_health, Ks_health, 4.0);
        color.rgb = Ka_health * Ia + pointLighting;
    }
    else if(object_id == TORCH)
    {
        vec4 p = (position_model - bbox_min) / (bbox_max - bbox_min);
        vec4 absP = abs(p);

        if(absP.x >= absP.y && absP.x >= absP.z) {
            U = p.z;
            V = p.y;
        }
        else if(absP.y >= absP.x && absP.y >= absP.z) {
            U = p.x;
            V = p.z;
        }
        else {
            U = p.x;
            V = p.y;
        }
        U = U * 0.5 + 0.5;
        V = V * 0.5 + 0.5;
        vec3 fireCore = vec3(1.0, 0.5, 0.1);
        vec3 fireGlow = vec3(1.0, 0.8, 0.2);
        float fresnel = 1.0 - max(dot(n, v), 0.0);
        fresnel = pow(fresnel, 2.0);
        vec3 luz_fogo = mix(fireCore, fireGlow, fresnel) * 2.5;
        vec3 fogo_tex = texture(TextureImage10, vec2(U,V)).rgb;
        color.rgb=luz_fogo*fogo_tex;
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
    // Canal alpha = 1 (totalmente opaco)
    color.a=1;

    // ─────────────────────────────────────────────────────────────────────────
    // CORREÇÃO GAMMA
    // ─────────────────────────────────────────────────────────────────────────
    // Monitores sRGB têm uma curva de resposta não-linear (gamma ~2.2)
    // Para que as cores calculadas linearmente apareçam corretas no monitor,
    // aplicamos a correção gamma inversa:
    //
    //     cor_final = cor_linear ^ (1/2.2)
    //
    // Isso "levanta" os valores escuros, compensando a curva do monitor.
    // ─────────────────────────────────────────────────────────────────────────
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
}

