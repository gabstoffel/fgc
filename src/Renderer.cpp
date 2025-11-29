#include "Renderer.h"
#include "Player.h"
#include "Enemy.h"
#include "Game.h"
#include "matrices.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <stack>
#include <set>
#include <string>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
void TextRendering_Init();
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale);

static std::stack<glm::mat4> g_MatrixStack;

static void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

static void PopMatrix(glm::mat4& M)
{
    if (g_MatrixStack.empty())
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

static float DiferencaAngulo(glm::vec4 v, glm::vec4 u)
{
    if (norm(v) == 0 || norm(u) == 0)
    {
        return 0.0;
    }
    float angulo = acos(dotproduct(u, v) / (norm(v) * norm(u)));
    if (u.x * v.z - u.z * v.x < 0)
    {
        angulo = -angulo;
    }
    return angulo;
}

Renderer::Renderer()
    : m_vertexArrayObjectID(0)
    , m_gpuProgramID(0)
    , m_modelUniform(0)
    , m_viewUniform(0)
    , m_projectionUniform(0)
    , m_renderAsBlackUniform(0)
    , m_currentView(Matrix_Identity())
    , m_currentProjection(Matrix_Identity())
    , m_screenRatio(1.0f)
    , m_window(nullptr)
{
}

Renderer::~Renderer()
{
    if (m_gpuProgramID != 0)
    {
        glDeleteProgram(m_gpuProgramID);
    }
}

bool Renderer::init(GLFWwindow* window)
{
    m_window = window;

    loadShadersFromFiles();

    m_modelUniform = glGetUniformLocation(m_gpuProgramID, "model");
    m_viewUniform = glGetUniformLocation(m_gpuProgramID, "view");
    m_projectionUniform = glGetUniformLocation(m_gpuProgramID, "projection");
    m_renderAsBlackUniform = glGetUniformLocation(m_gpuProgramID, "render_as_black");
    m_objectIdUniform = glGetUniformLocation(m_gpuProgramID, "object_id");
    m_bbox_min_uniform   = glGetUniformLocation(m_gpuProgramID, "bbox_min");
    m_bbox_max_uniform   = glGetUniformLocation(m_gpuProgramID, "bbox_max");

    glUseProgram(m_gpuProgramID);
    //Inimigo
    glUniform1i(glGetUniformLocation(m_gpuProgramID, "TextureImage0"), 0);
    //Parede
    glUniform1i(glGetUniformLocation(m_gpuProgramID, "TextureImage1"), 1);
    //Ch�o
    glUniform1i(glGetUniformLocation(m_gpuProgramID, "TextureImage2"), 2);
    //Telhado
    glUniform1i(glGetUniformLocation(m_gpuProgramID, "TextureImage3"), 3);
    //Player
    glUniform1i(glGetUniformLocation(m_gpuProgramID, "TextureImage4"), 4);
    //Varinha
    glUniform1i(glGetUniformLocation(m_gpuProgramID, "TextureImage5"), 5);
    glUseProgram(0);

    m_vertexArrayObjectID = buildGeometry();

    try {
        LoadTextureImage("texturas/monstro.jpg"); //Monstro
        LoadTextureImage("texturas/parede.jpg");
        LoadTextureImage("texturas/Chao.png");
        LoadTextureImage("texturas/telhado.jpg");
        LoadTextureImage("texturas/Arqueira.png");
        LoadTextureImage("texturas/Varinha.png");

        ObjModel monstermodel("modelos/monstro.obj");
        computeNormals(&monstermodel);
        buildTrianglesFromObj(&monstermodel);

        ObjModel cubemodel("modelos/cube.obj");
        computeNormals(&cubemodel);
        buildTrianglesFromObj(&cubemodel);

        ObjModel planemodel("modelos/plane.obj");
        computeNormals(&planemodel);
        buildTrianglesFromObj(&planemodel);

        ObjModel arqueiramodel("modelos/arqueira.obj");
        computeNormals(&arqueiramodel);
        buildTrianglesFromObj(&arqueiramodel);

        ObjModel dragonmodel("modelos/dragon.obj");
        computeNormals(&dragonmodel);
        buildTrianglesFromObj(&dragonmodel);

        ObjModel varinhamodel("modelos/varinha.obj");
        computeNormals(&varinhamodel);
        buildTrianglesFromObj(&varinhamodel);

        printf("All OBJ models loaded successfully!\n");
    } catch (const std::exception& e) {
        fprintf(stderr, "ERROR loading OBJ models: %s\n", e.what());
    }

    glEnable(GL_DEPTH_TEST);

    TextRendering_Init();

    return true;
}

void Renderer::setProjection(const glm::mat4& projection)
{
    m_currentProjection = projection;
    glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, glm::value_ptr(projection));
}

void Renderer::setView(const glm::mat4& view)
{
    m_currentView = view;
    glUniformMatrix4fv(m_viewUniform, 1, GL_FALSE, glm::value_ptr(view));
}

void Renderer::renderScene(const Player& player, const EnemyManager& enemyManager, const Enemy& dragonBoss, bool dragonBossAlive, const ProjectileManager* projectileManager)
{
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_gpuProgramID);

    glUniformMatrix4fv(m_viewUniform, 1, GL_FALSE, glm::value_ptr(m_currentView));
    glUniformMatrix4fv(m_projectionUniform, 1, GL_FALSE, glm::value_ptr(m_currentProjection));

    glBindVertexArray(m_vertexArrayObjectID);

    renderArena();
    renderPlayer(player);
    renderEnemies(enemyManager, player.getPosition());
    renderDragonBoss(dragonBoss, dragonBossAlive, player.getPosition());

    if (projectileManager != nullptr)
    {
        renderProjectiles(*projectileManager);
    }
}

void Renderer::renderArena()
{
    glm::mat4 model = Matrix_Identity();
    glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(m_vertexArrayObjectID);

    glUniform1i(m_objectIdUniform, 2);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(36*sizeof(GLuint)));

    glUniform1i(m_objectIdUniform, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(50*sizeof(GLuint)));

    glUniform1i(m_objectIdUniform, 4);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(56*sizeof(GLuint)));

    glUniform1i(m_objectIdUniform, 5);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(62*sizeof(GLuint)));

    glUniform1i(m_objectIdUniform, 6);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(68*sizeof(GLuint)));

    glUniform1i(m_objectIdUniform, 7);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(44*sizeof(GLuint)));
}

void Renderer::renderPlayer(const Player& player)
{
    glm::mat4 model = Matrix_Identity();
    glm::vec4 position = player.getPosition();

    PushMatrix(model);
    float dist_chao = m_virtualScene["Arqueira"].bbox_min.y;
    dist_chao=0-dist_chao;
    dist_chao=dist_chao*0.001f;
    model = model * Matrix_Translate(position.x, dist_chao, position.z)
                  * Matrix_Rotate_Y(player.getMovementAngle());
    PushMatrix(model);
        //Varinha
        model = model*Matrix_Translate(0.057f,0.06f,0.02f)*Matrix_Rotate_X(M_PI/4)* Matrix_Scale(0.09f, 0.09f, 0.09f);
        glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(m_objectIdUniform, 10);
        drawVirtualObject("Varinha");
    PopMatrix(model);
    model=model* Matrix_Scale(0.001f, 0.001f, 0.001f);
    glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(m_objectIdUniform, 1);
    // Renderriza o player
    if (m_virtualScene.find("Arqueira") != m_virtualScene.end())
        drawVirtualObject("Arqueira");
    else if (m_virtualScene.find("cube_faces") != m_virtualScene.end())
    {
        glBindVertexArray(m_vertexArrayObjectID);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
    }

    PopMatrix(model);
}

void Renderer::renderEnemies(const EnemyManager& enemyManager, const glm::vec4& playerPosition)
{
    const std::vector<Enemy>& enemies = enemyManager.getEnemies();
    glm::mat4 model = Matrix_Identity();
    float dist_chao = m_virtualScene["turle"].bbox_min.y;
    dist_chao=0-dist_chao;
    dist_chao=dist_chao*0.15f;
    for (size_t i = 0; i < enemies.size(); i++)
    {
        PushMatrix(model);
        model = model * Matrix_Translate(enemies[i].getX(), dist_chao, enemies[i].getZ());
        float rotation_angle = enemies[i].lookAt(playerPosition);
        model = model * Matrix_Rotate_Y(rotation_angle);
        model = model * Matrix_Scale(0.15f, 0.15f, 0.15f);

        glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(m_objectIdUniform, 0);

        if (m_virtualScene.find("turle") != m_virtualScene.end())
            drawVirtualObject("turle");
        else if (m_virtualScene.find("cube_faces") != m_virtualScene.end())
        {
            glBindVertexArray(m_vertexArrayObjectID);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
        }

        PopMatrix(model);
    }
}

void Renderer::renderDragonBoss(const Enemy& dragon, bool isAlive, const glm::vec4& playerPosition)
{
    if (!isAlive)
        return;

    glm::mat4 model = Matrix_Identity();
    glm::vec4 dragonPos = dragon.getPosition();

    PushMatrix(model);

    float dx = playerPosition.x - dragonPos.x;
    float dz = playerPosition.z - dragonPos.z;
    float angleToPlayer = atan2(dx, dz) + 3.14159f;  

    model = model * Matrix_Translate(dragonPos.x, dragonPos.y + 0.15f, dragonPos.z)
                  * Matrix_Rotate_Y(angleToPlayer)
                  * Matrix_Scale(0.4f, 0.4f, 0.4f);

    glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(m_objectIdUniform, 9);

    if (m_virtualScene.find("Mesh1.001") != m_virtualScene.end())
    {
        drawVirtualObject("Mesh1.001");
    }

    PopMatrix(model);
}

void Renderer::renderPillars(const std::vector<Pillar>& pillars)
{
    glDisable(GL_CULL_FACE);

    for (size_t i = 0; i < pillars.size(); i++)
    {
        const Pillar& pillar = pillars[i];

        glm::mat4 model = Matrix_Identity();
        float cubeSize = 0.2f;
        float scaleX = (pillar.radius * 2.0f) / cubeSize; 
        float scaleY = pillar.height / cubeSize;
        float scaleZ = (pillar.radius * 2.0f) / cubeSize;

        model = model * Matrix_Translate(pillar.position.x, pillar.position.y + pillar.height * 0.5f, pillar.position.z)
                      * Matrix_Scale(scaleX, scaleY, scaleZ);

        glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(m_objectIdUniform, 15); // PILAR

        glBindVertexArray(m_vertexArrayObjectID);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
    }

    glEnable(GL_CULL_FACE);
}

void Renderer::renderHealthPickups(const std::vector<HealthPickup>& pickups)
{
    static float rotation = 0.0f;
    rotation += 0.02f;

    for (size_t i = 0; i < pickups.size(); i++)
    {
        if (!pickups[i].active)
            continue;

        const HealthPickup& pickup = pickups[i];

        glm::mat4 model = Matrix_Identity();
        float bobHeight = sin(rotation * 2.0f + i) * 0.02f;
        float pickupScale = 0.5f;
        model = model * Matrix_Translate(pickup.position.x, pickup.position.y + 0.1f + bobHeight, pickup.position.z)
                      * Matrix_Rotate_Y(rotation)
                      * Matrix_Scale(pickupScale, pickupScale, pickupScale);

        glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(m_objectIdUniform, 16); 

        glBindVertexArray(m_vertexArrayObjectID);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
    }
}

void Renderer::renderTorches(const std::vector<Torch>& torches)
{
    static float flicker = 0.0f;
    flicker += 0.15f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
    glDepthMask(GL_FALSE);

    for (size_t i = 0; i < torches.size(); i++)
    {
        if (!torches[i].active)
            continue;

        const Torch& torch = torches[i];

        float scale = 0.12f + 0.03f * sin(flicker + i * 1.5f);

        glm::mat4 model = Matrix_Identity();
        model = model * Matrix_Translate(torch.position.x, torch.position.y, torch.position.z)
                      * Matrix_Scale(scale, scale * 1.5f, scale);  

        glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(m_objectIdUniform, 17);  // TOCHA

        glBindVertexArray(m_vertexArrayObjectID);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Renderer::renderProjectiles(const ProjectileManager& projectileManager)
{
    const std::vector<Projectile>& projectiles = projectileManager.getProjectiles();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    for (size_t i = 0; i < projectiles.size(); i++)
    {
        if (!projectiles[i].active)
            continue;

        const Projectile& proj = projectiles[i];

        glm::mat4 model = Matrix_Identity();
        model = model * Matrix_Translate(proj.position.x, proj.position.y, proj.position.z)
                      * Matrix_Scale(0.05f, 0.05f, 0.05f);

        glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(m_objectIdUniform, proj.isEnemyProjectile ? 13 : 11);

        glBindVertexArray(m_vertexArrayObjectID);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);

        for (int t = 0; t < Projectile::TRAIL_LENGTH; t++)
        {
            int idx = (proj.trailIndex - 1 - t + Projectile::TRAIL_LENGTH) % Projectile::TRAIL_LENGTH;
            glm::vec3 trailPos = proj.trailPositions[idx];

            float dx = trailPos.x - proj.position.x;
            float dy = trailPos.y - proj.position.y;
            float dz = trailPos.z - proj.position.z;
            float distSq = dx*dx + dy*dy + dz*dz;
            if (distSq < 0.01f) continue;

            float scale = 0.04f * (1.0f - (float)t / Projectile::TRAIL_LENGTH);
            if (scale < 0.01f) continue;

            model = Matrix_Identity();
            model = model * Matrix_Translate(trailPos.x, trailPos.y, trailPos.z)
                          * Matrix_Scale(scale, scale, scale);

            glUniformMatrix4fv(m_modelUniform, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(m_objectIdUniform, proj.isEnemyProjectile ? 14 : 12);

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
        }
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Renderer::renderHitMarker()
{
    if (m_window == nullptr)
        return;

    TextRendering_PrintString(m_window, "X", -0.02f, -0.02f, 2.0f);
}

void Renderer::renderMuzzleFlash()
{
    // Removed - purple projectile provides sufficient visual feedback
}

void Renderer::renderDamageFlash(float intensity)
{
    if (m_window == nullptr || intensity <= 0.0f)
        return;

    if (intensity > 0.5f)
    {
        TextRendering_PrintString(m_window, "!", -0.95f, 0.0f, 3.0f);
        TextRendering_PrintString(m_window, "!", 0.90f, 0.0f, 3.0f);
    }
}

void Renderer::renderCrosshair(bool isFirstPerson)
{
    if (!isFirstPerson || m_window == nullptr)
        return;

    TextRendering_PrintString(m_window, "+", -0.02f, -0.02f, 2.0f);
}

void Renderer::renderHUD(const Player& player, const EnemyManager& enemies, const Enemy& boss, bool bossAlive)
{
    if (m_window == nullptr)
        return;

    char buffer[64];

    snprintf(buffer, 64, "HP: %d/%d", player.getVida(), player.getMaxVida());
    TextRendering_PrintString(m_window, buffer, -0.95f, 0.9f, 1.5f);

    snprintf(buffer, 64, "Inimigos: %zu", enemies.getEnemyCount());
    TextRendering_PrintString(m_window, buffer, -0.95f, 0.8f, 1.5f);

    if (bossAlive)
    {
        snprintf(buffer, 64, "BOSS: %d", boss.getVida());
        TextRendering_PrintString(m_window, buffer, -0.15f, 0.9f, 1.5f);
    }
}

void Renderer::renderMenu(int selectedDifficulty)
{
    if (m_window == nullptr)
        return;

    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextRendering_PrintString(m_window, "=== ARENA SURVIVAL ===", -0.45f, 0.5f, 2.0f);
    TextRendering_PrintString(m_window, "Selecione a Dificuldade:", -0.35f, 0.2f, 1.5f);
    TextRendering_PrintString(m_window, "[1] Facil", -0.15f, 0.0f, 1.5f);
    TextRendering_PrintString(m_window, "[2] Normal", -0.15f, -0.1f, 1.5f);
    TextRendering_PrintString(m_window, "[3] Dificil", -0.15f, -0.2f, 1.5f);
    TextRendering_PrintString(m_window, "Pressione 1, 2 ou 3 para comecar", -0.45f, -0.5f, 1.2f);
}

void Renderer::renderGameOver()
{
    if (m_window == nullptr)
        return;

    glClearColor(0.2f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextRendering_PrintString(m_window, "=== GAME OVER ===", -0.35f, 0.3f, 2.5f);
    TextRendering_PrintString(m_window, "Voce foi derrotado!", -0.30f, 0.0f, 1.5f);
    TextRendering_PrintString(m_window, "Pressione R para reiniciar", -0.40f, -0.2f, 1.3f);
    TextRendering_PrintString(m_window, "Pressione M para o menu", -0.40f, -0.35f, 1.3f);
}

void Renderer::renderWin()
{
    if (m_window == nullptr)
        return;

    glClearColor(0.0f, 0.15f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    TextRendering_PrintString(m_window, "=== VITORIA ===", -0.30f, 0.3f, 2.5f);
    TextRendering_PrintString(m_window, "Voce derrotou o Dragao!", -0.35f, 0.0f, 1.5f);
    TextRendering_PrintString(m_window, "Pressione R para jogar novamente", -0.45f, -0.2f, 1.3f);
    TextRendering_PrintString(m_window, "Pressione M para o menu", -0.40f, -0.35f, 1.3f);
}

void Renderer::renderCountdown(int countdownNumber)
{
    if (m_window == nullptr)
        return;

    char buffer[8];
    if (countdownNumber > 0)
    {
        snprintf(buffer, 8, "%d", countdownNumber);
        TextRendering_PrintString(m_window, buffer, -0.05f, 0.0f, 5.0f);
    }
    else
    {
        TextRendering_PrintString(m_window, "GO!", -0.12f, 0.0f, 4.0f);
    }
}

GLuint Renderer::buildGeometry()
{
    GLfloat model_coefficients[] = {
        -0.1f,  0.1f,  0.1f, 1.0f,
        -0.1f, -0.1f,  0.1f, 1.0f,
         0.1f, -0.1f,  0.1f, 1.0f,
         0.1f,  0.1f,  0.1f, 1.0f,
        -0.1f,  0.1f, -0.1f, 1.0f,
        -0.1f, -0.1f, -0.1f, 1.0f,
         0.1f, -0.1f, -0.1f, 1.0f,
         0.1f,  0.1f, -0.1f, 1.0f,
         5.0f, 0.0f,  2.0f, 1.0f,
        -5.0f, 0.0f,  2.0f, 1.0f,
         5.0f, 0.0f, -2.0f, 1.0f,
        -5.0f, 0.0f, -2.0f, 1.0f,
         0.0f,  0.0f,  0.0f, 1.0f,
         0.0f,  0.0f,  0.4f, 1.0f,
         5.0f, 3.0f,  2.0f, 1.0f,
        -5.0f, 3.0f,  2.0f, 1.0f,
         5.0f, 3.0f, -2.0f, 1.0f,
        -5.0f, 3.0f, -2.0f, 1.0f,
         4.5f, 0.0f,  1.5f, 1.0f,
        -4.5f, 0.0f,  1.5f, 1.0f,
        -4.5f, 3.0f,  1.5f, 1.0f,
         4.5f, 3.0f,  1.5f, 1.0f,
        -4.5f, 0.0f, -1.5f, 1.0f,
         4.5f, 0.0f, -1.5f, 1.0f,
         4.5f, 3.0f, -1.5f, 1.0f,
        -4.5f, 3.0f, -1.5f, 1.0f,
         4.5f, 0.0f, -1.5f, 1.0f,
         4.5f, 0.0f,  1.5f, 1.0f,
         4.5f, 3.0f,  1.5f, 1.0f,
         4.5f, 3.0f, -1.5f, 1.0f,
        -4.5f, 0.0f,  1.5f, 1.0f,
        -4.5f, 0.0f, -1.5f, 1.0f,
        -4.5f, 3.0f, -1.5f, 1.0f,
        -4.5f, 3.0f,  1.5f, 1.0f,
    };

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(model_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(model_coefficients), model_coefficients);
    GLuint location = 0;
    GLint  number_of_dimensions = 4;
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLfloat normal_coefficients[] = {
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f,
    };

    GLuint VBO_normal_coefficients_id;
    glGenBuffers(1, &VBO_normal_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normal_coefficients), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(normal_coefficients), normal_coefficients);
    location = 1;
    number_of_dimensions = 4;
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint indices[] = {
        0, 1, 2,
        7, 6, 5,
        3, 2, 6,
        4, 0, 3,
        4, 5, 1,
        1, 5, 6,
        0, 2, 3,
        7, 5, 4,
        3, 6, 7,
        4, 3, 7,
        4, 1, 0,
        1, 6, 2,
        9, 8, 10,
        11, 9, 10,
        12, 13,
        14, 15, 16,
        15, 17, 16,
        18, 19, 21,
        19, 20, 21,
        22, 23, 25,
        23, 24, 25,
        26, 27, 29,
        27, 28, 29,
        30, 31, 33,
        31, 32, 33
    };

    SceneObject cube_faces;
    cube_faces.name           = "Cubo (faces coloridas)";
    cube_faces.first_index    = 0;
    cube_faces.num_indices    = 36;
    cube_faces.rendering_mode = GL_TRIANGLES;
    m_virtualScene["cube_faces"] = cube_faces;

    SceneObject piso;
    piso.name = "Piso";
    piso.first_index = (36*sizeof(GLuint));
    piso.num_indices = 6;
    piso.rendering_mode = GL_TRIANGLES;
    m_virtualScene["piso"] = piso;

    SceneObject eixo_z;
    eixo_z.name = "Z";
    eixo_z.first_index = (42*sizeof(GLuint));
    eixo_z.num_indices = 2;
    eixo_z.rendering_mode = GL_LINES;
    m_virtualScene["eixo_z"] = eixo_z;

    SceneObject teto;
    teto.name = "Teto";
    teto.first_index = (44*sizeof(GLuint));
    teto.num_indices = 6;
    teto.rendering_mode = GL_TRIANGLES;
    m_virtualScene["teto"] = teto;

    SceneObject parede_norte;
    parede_norte.name = "Parede Norte";
    parede_norte.first_index = (50*sizeof(GLuint));
    parede_norte.num_indices = 6;
    parede_norte.rendering_mode = GL_TRIANGLES;
    m_virtualScene["parede_norte"] = parede_norte;

    SceneObject parede_sul;
    parede_sul.name = "Parede Sul";
    parede_sul.first_index = (56*sizeof(GLuint));
    parede_sul.num_indices = 6;
    parede_sul.rendering_mode = GL_TRIANGLES;
    m_virtualScene["parede_sul"] = parede_sul;

    SceneObject parede_leste;
    parede_leste.name = "Parede Leste";
    parede_leste.first_index = (62*sizeof(GLuint));
    parede_leste.num_indices = 6;
    parede_leste.rendering_mode = GL_TRIANGLES;
    m_virtualScene["parede_leste"] = parede_leste;

    SceneObject parede_oeste;
    parede_oeste.name = "Parede Oeste";
    parede_oeste.first_index = (68*sizeof(GLuint));
    parede_oeste.num_indices = 6;
    parede_oeste.rendering_mode = GL_TRIANGLES;
    m_virtualScene["parede_oeste"] = parede_oeste;

    GLuint indices_id;
    glGenBuffers(1, &indices_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);
    glBindVertexArray(0);
    return vertex_array_object_id;
}

void Renderer::LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Par�metros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = m_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    m_NumLoadedTextures += 1;
}

void Renderer::loadShadersFromFiles()
{
    GLuint vertex_shader_id = loadShader_Vertex("src/shaders/shader_vertex.glsl");
    GLuint fragment_shader_id = loadShader_Fragment("src/shaders/shader_fragment.glsl");
    if (m_gpuProgramID != 0)
        glDeleteProgram(m_gpuProgramID);
    m_gpuProgramID = createGpuProgram(vertex_shader_id, fragment_shader_id);
}

GLuint Renderer::loadShader_Vertex(const char* filename)
{
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    loadShader(filename, vertex_shader_id);
    return vertex_shader_id;
}

GLuint Renderer::loadShader_Fragment(const char* filename)
{
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    loadShader(filename, fragment_shader_id);
    return fragment_shader_id;
}

void Renderer::loadShader(const char* filename, GLuint shader_id)
{
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch (std::exception& e) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>(str.length());
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);
    glCompileShader(shader_id);
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);
    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);
    if (log_length != 0)
    {
        std::string output;

        if (!compiled_ok)
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }
    delete [] log;
}

GLuint Renderer::createGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);
    if (linked_ok == GL_FALSE)
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
        GLchar* log = new GLchar[log_length];
        glGetProgramInfoLog(program_id, log_length, &log_length, log);
        std::string output;
        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";
        delete [] log;
        fprintf(stderr, "%s", output.c_str());
    }
    return program_id;
}

ObjModel::ObjModel(const char* filename, const char* basepath, bool triangulate)
{
    printf("Carregando objetos do arquivo \"%s\"...\n", filename);

    std::string fullpath(filename);
    std::string dirname;
    if (basepath == NULL)
    {
        auto i = fullpath.find_last_of("/");
        if (i != std::string::npos)
        {
            dirname = fullpath.substr(0, i+1);
            basepath = dirname.c_str();
        }
    }

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

    if (!err.empty())
        fprintf(stderr, "\n%s\n", err.c_str());

    if (!ret)
        throw std::runtime_error("Erro ao carregar modelo.");

    for (size_t shape = 0; shape < shapes.size(); ++shape)
    {
        if (shapes[shape].name.empty())
        {
            fprintf(stderr,
                    "*********************************************\n"
                    "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                    "*********************************************\n",
                filename);
            throw std::runtime_error("Objeto sem nome.");
        }
        printf("- Objeto '%s'\n", shapes[shape].name.c_str());
    }
    printf("OK.\n");
}

void Renderer::computeNormals(ObjModel* model)
{
    if (!model->attrib.normals.empty())
        return;

    std::set<unsigned int> sgroup_ids;
    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            unsigned int sgroup = model->shapes[shape].mesh.smoothing_group_ids[triangle];
            sgroup_ids.insert(sgroup);
        }
    }

    size_t num_vertices = model->attrib.vertices.size() / 3;
    model->attrib.normals.reserve(3*num_vertices);

    for (const unsigned int & sgroup : sgroup_ids)
    {
        std::vector<int> num_triangles_per_vertex(num_vertices, 0);
        std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();
            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];
                if (sgroup_tri != sgroup)
                    continue;

                glm::vec4 vertices[3];
                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                    const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                    const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                    vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
                }

                const glm::vec4 a = vertices[0];
                const glm::vec4 b = vertices[1];
                const glm::vec4 c = vertices[2];
                glm::vec4 u = glm::vec4(b - a);
                glm::vec4 v = glm::vec4(c - a);
                const glm::vec4 n = crossproduct(u,v);

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    num_triangles_per_vertex[idx.vertex_index] += 1;
                    vertex_normals[idx.vertex_index] += n;
                }
            }
        }

        std::vector<size_t> normal_indices(num_vertices, 0);
        for (size_t vertex_index = 0; vertex_index < vertex_normals.size(); ++vertex_index)
        {
            if (num_triangles_per_vertex[vertex_index] == 0)
                continue;

            glm::vec4 n = vertex_normals[vertex_index] / (float)num_triangles_per_vertex[vertex_index];
            n /= norm(n);
            model->attrib.normals.push_back(n.x);
            model->attrib.normals.push_back(n.y);
            model->attrib.normals.push_back(n.z);

            size_t normal_index = (model->attrib.normals.size() / 3) - 1;
            normal_indices[vertex_index] = normal_index;
        }

        for (size_t shape = 0; shape < model->shapes.size(); ++shape)
        {
            size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();
            for (size_t triangle = 0; triangle < num_triangles; ++triangle)
            {
                unsigned int sgroup_tri = model->shapes[shape].mesh.smoothing_group_ids[triangle];
                if (sgroup_tri != sgroup)
                    continue;

                for (size_t vertex = 0; vertex < 3; ++vertex)
                {
                    tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                    model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index =
                        normal_indices[idx.vertex_index];
                }
            }
        }
    }
}

void Renderer::buildTrianglesFromObj(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                model_coefficients.push_back( vx );
                model_coefficients.push_back( vy );
                model_coefficients.push_back( vz );
                model_coefficients.push_back( 1.0f );

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);


                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx );
                    normal_coefficients.push_back( ny );
                    normal_coefficients.push_back( nz );
                    normal_coefficients.push_back( 0.0f );
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index;
        theobject.num_indices    = last_index - first_index + 1;
        theobject.rendering_mode = GL_TRIANGLES;
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        m_virtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0;
    GLint  number_of_dimensions = 4;
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1;
        number_of_dimensions = 4;
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2;
        number_of_dimensions = 2;
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    GLuint indices_id;
    glGenBuffers(1, &indices_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    glBindVertexArray(0);
}

void Renderer::drawVirtualObject(const std::string& object_name)
{
    glBindVertexArray(m_virtualScene[object_name].vertex_array_object_id);
    glm::vec3 bbox_min = m_virtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = m_virtualScene[object_name].bbox_max;
    glUniform4f(m_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(m_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);
    glDrawElements(
        m_virtualScene[object_name].rendering_mode,
        m_virtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(m_virtualScene[object_name].first_index * sizeof(GLuint))
    );
    glBindVertexArray(0);
}
