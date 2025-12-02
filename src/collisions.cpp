// CONCEITOS PRINCIPAIS (das notas de aula):
//
// 1) DETECÇÃO DE COLISÃO é normalmente separada em duas partes:
//    - "Broad Phase": rapidamente detecta quais objetos POTENCIALMENTE
//      podem ter intersecção (custo O(n log n) com estruturas espaciais)
//    - "Narrow Phase": de fato verifica quais colisões ocorreram
//      (testes exatos ou aproximados através de "hit boxes")
//
// 2) BOUNDING VOLUMES:
//    - AABB (Axis-Aligned Bounding Box): caixa alinhada aos eixos
//    - Bounding Sphere: esfera envolvente
//    - Utilizados para simplificar testes de colisão
//
// 3) CURVAS DE BÉZIER:
//    - Curvas paramétricas c(t) onde t ∈ [0,1]
//    - Definidas por pontos de controle P0, P1, P2, P3
//    - Avaliadas usando polinômios de Bernstein
//
// ============================================================================

#include "collisions.h"
#include "matrices.h"
#include <glm/vec4.hpp>
#include <cmath>
#include <algorithm>


// ============================================================================
// INTERSECÇÃO AABB-PLANO
// ============================================================================
//
// n·p + d = 0, onde d = -n·c
//
// Para testar AABB vs Plano, projetamos a "extensão efetiva" da caixa
// na direção da normal do plano.
// ============================================================================
bool testAABBPlane(glm::vec3 boxMin, glm::vec3 boxMax, glm::vec4 plane) {
    // Extrai a normal do plano (componentes x, y, z) e a distância d (componente w)
    // Equação do plano: normal.x * x + normal.y * y + normal.z * z + d = 0
    glm::vec3 normal = glm::vec3(plane.x, plane.y, plane.z);
    float d = plane.w;

    // Calcula o centro e as semi-extensões (half-extents) da AABB
    // Centro: ponto médio entre boxMin e boxMax
    // Extensões: distância do centro até as faces da caixa
    glm::vec3 center = (boxMax + boxMin) * 0.5f;
    glm::vec3 extents = boxMax - center;

    // Calcula o "raio de projeção" da AABB sobre a normal do plano
    // Este é o raio efetivo da caixa na direção perpendicular ao plano
    // Usamos valores absolutos pois queremos a extensão máxima em cada eixo
    float r = extents.x * std::abs(normal.x) +
              extents.y * std::abs(normal.y) +
              extents.z * std::abs(normal.z);

    // Calcula a distância com sinal do centro da caixa até o plano
    // s > 0: centro está do lado positivo da normal
    // s < 0: centro está do lado negativo da normal
    float s = dotproduct(normal, center) + d;

    // Há intersecção se a distância |s| for menor ou igual ao raio r
    // Isso significa que a caixa "atravessa" o plano
    return std::abs(s) <= r;
}

// ============================================================================
// RESOLUÇÃO DE COLISÃO AABB-PLANO
// ============================================================================
// Além de detectar a colisão, esta função RESOLVE a penetração,
// empurrando o objeto para fora do plano ao longo da normal.
//
// Profundidade de penetração = r - |s|
// Direção de resolução = normal do plano (ou oposta, dependendo do lado)
// ============================================================================
bool resolveAABBPlane(glm::vec3& position, glm::vec3 extents, glm::vec4 plane) {
    // Calcula os limites da AABB a partir da posição central e extensões
    glm::vec3 boxMin = position - extents;
    glm::vec3 boxMax = position + extents;

    // Primeiro verifica se há colisão
    if (!testAABBPlane(boxMin, boxMax, plane)) {
        return false; // Sem colisão, não precisa resolver
    }

    // Extrai componentes do plano
    glm::vec3 normal = glm::vec3(plane.x, plane.y, plane.z);
    float d = plane.w;

    // Calcula o raio de projeção (mesmo cálculo da função de teste)
    float r = extents.x * std::abs(normal.x) +
              extents.y * std::abs(normal.y) +
              extents.z * std::abs(normal.z);

    // Calcula distância com sinal do centro ao plano
    float s = dotproduct(normal, position) + d;

    // Profundidade de penetração: quanto a caixa está "dentro" do plano
    float penetrationDepth = r - std::abs(s);

    // Empurra a AABB para fora do plano ao longo da normal
    // A direção depende de qual lado do plano a caixa está
    if (s < 0) {
        // Caixa está do lado negativo do plano, empurra na direção da normal
        position += normal * penetrationDepth;
    } else {
        // Caixa está do lado positivo do plano, empurra na direção oposta
        position -= normal * penetrationDepth;
    }

    return true; // Colisão foi resolvida
}

// ============================================================================
// INTERSECÇÃO AABB-AABB
// ============================================================================
// Duas caixas alinhadas aos eixos se intersectam SE E SOMENTE SE
// elas se sobrepõem em TODOS os três eixos (x, y, z).
//
// Para cada eixo, verificamos se os intervalos [minA, maxA] e [minB, maxB]
// se sobrepõem: minA <= maxB AND maxA >= minB
// ============================================================================
bool testAABBAABB(const glm::vec3& minA, const glm::vec3& maxA,
                  const glm::vec3& minB, const glm::vec3& maxB)
{
    // Verifica sobreposição em cada eixo
    // Se houver separação em QUALQUER eixo, não há colisão
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&  // Eixo X
           (minA.y <= maxB.y && maxA.y >= minB.y) &&  // Eixo Y
           (minA.z <= maxB.z && maxA.z >= minB.z);    // Eixo Z
}

// ============================================================================
// INTERSECÇÃO PONTO-ESFERA (Bounding Sphere)
// ============================================================================
// Um ponto p está dentro de uma esfera com centro s e raio r se:
//    ||p - s|| <= r
//
// Equivalentemente (evitando a raiz quadrada por eficiência):
//    ||p - s||² <= r²
//
// Este teste é usado como "hit box" esférica para detecção de colisão.
// ============================================================================
bool testPointSphere(const glm::vec3& point, const glm::vec3& sphereCenter, float sphereRadius) {
    // Calcula o quadrado da distância do ponto ao centro da esfera
    float distSq = norm(point - sphereCenter) * norm(point - sphereCenter);

    // Compara com o quadrado do raio (evita calcular raiz quadrada)
    return distSq <= sphereRadius * sphereRadius;
}

// ============================================================================
// CLAMPING DE POSIÇÃO (Restrição à Arena)
// ============================================================================
// Restringe uma posição para ficar dentro de uma caixa delimitadora.
// Útil para manter objetos dentro da arena de jogo.
//
// Para cada componente: position = max(min, min(position, max))
// ============================================================================
void clampPositionToBox(glm::vec3& position, glm::vec3 minBounds, glm::vec3 maxBounds) {
    // Clamp em cada eixo independentemente
    position.x = std::max(minBounds.x, std::min(position.x, maxBounds.x));
    position.y = std::max(minBounds.y, std::min(position.y, maxBounds.y));
    position.z = std::max(minBounds.z, std::min(position.z, maxBounds.z));
}

// ============================================================================
// INTERSECÇÃO PONTO-AABB EXPANDIDA
// ============================================================================
// Testa se um ponto com raio (esfera) intersecta uma AABB.
//
// Técnica: expandir a AABB pelo raio em todas as direções,
// depois testar se o ponto (centro da esfera) está dentro da AABB expandida.
//
// Isso é equivalente a testar esfera vs AABB, mas mais simples de implementar.
// ============================================================================
bool testPointExpandedAABB(const glm::vec3& point, float radius,
                           const glm::vec3& boxMin, const glm::vec3& boxMax)
{
    // Expande a AABB pelo raio em todas as direções
    // boxMin diminui, boxMax aumenta
    glm::vec3 expandedMin = boxMin - glm::vec3(radius);
    glm::vec3 expandedMax = boxMax + glm::vec3(radius);

    // Testa se o ponto está dentro da AABB expandida
    return (point.x >= expandedMin.x && point.x <= expandedMax.x) &&
           (point.y >= expandedMin.y && point.y <= expandedMax.y) &&
           (point.z >= expandedMin.z && point.z <= expandedMax.z);
}

// ============================================================================
// INTERSECÇÃO CURVA DE BÉZIER CÚBICA - AABB
// ============================================================================
bool testBezierAABB(const glm::vec4& p0, const glm::vec4& p1,
                    const glm::vec4& p2, const glm::vec4& p3,
                    float objectRadius,
                    const glm::vec3& boxMin, const glm::vec3& boxMax,
                    int numSamples, float& outCollisionT)
{
    // Inicializa o parâmetro t de colisão como inválido
    outCollisionT = -1.0f;

    // Amostra numSamples+1 pontos ao longo da curva (t = 0, 1/n, 2/n, ..., 1)
    for (int i = 0; i <= numSamples; i++)
    {
        // Calcula o parâmetro t ∈ [0, 1]
        // t = 0: início da curva (ponto P0)
        // t = 1: fim da curva (ponto P3)
        float t = static_cast<float>(i) / static_cast<float>(numSamples);

        // ================================================================
        // AVALIAÇÃO DA CURVA DE BÉZIER CÚBICA
        // ================================================================
        // Usando a fórmula com polinômios de Bernstein:
        //
        // c(t) = (1-t)³·P0 + 3(1-t)²t·P1 + 3(1-t)t²·P2 + t³·P3
        //

        float u = 1.0f - t;      // u = (1-t), complemento de t
        float u2 = u * u;        // u ^2 = (1-t)^2
        float u3 = u2 * u;       // u ^3 = (1-t)^3
        float t2 = t * t;        // t ^2
        float t3 = t2 * t;       // t ^3

        // Avalia o ponto na curva usando a fórmula de Bernstein
        glm::vec4 point4 = u3 * p0 +              // (1-t)^3 · P0
                           3.0f * u2 * t * p1 +   // 3(1-t)^2t · P1
                           3.0f * u * t2 * p2 +   // 3(1-t)t^2 · P2
                           t3 * p3;               // t^3 · P3

        // Converte para vec3 
        glm::vec3 point(point4.x, point4.y, point4.z);

        // Testa se este ponto da curva (com o raio do objeto) colide com a AABB
        if (testPointExpandedAABB(point, objectRadius, boxMin, boxMax))
        {
            // Encontrou colisão! Retorna o parâmetro t onde ocorreu
            outCollisionT = t;
            return true;
        }
    }

    // Nenhum ponto amostrado colidiu com a AABB
    return false;
}
