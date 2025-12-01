#include "sfx.h"
#include "miniaudio.h"
#include <stdio.h>
//Váriavel global
Sfx sfx;
bool Sfx::start() {
    if (initialized) {
        printf("[SFX] Engine já estava iniciado!\n");
        return true;
    }

    printf("[SFX] Iniciando engine...\n");
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        printf("[ERRO] Falha ao inicializar o engine!\n");
        return false;
    }

    initialized = true;
    printf("[SFX] Engine iniciado com sucesso!\n");
    return true;
}

void Sfx::stop() {
    if (!initialized) {
        printf("[SFX] Engine já estava parado!\n");
        return;
    }

    printf("[SFX] Finalizando engine...\n");
    ma_engine_uninit(&engine);

    initialized = false;
    printf("[SFX] Engine finalizado!\n");
}

void Sfx::morte_monstro() {
    if (!initialized) {
        printf("[ERRO] Engine não inicializado! Use s.start() antes!\n");
        return;
    }

    printf("[SFX] Tentando tocar o som 'morte_monstro'\n");
    ma_result result = ma_engine_play_sound(&engine, "sfx/morte_monstro.mp3", NULL);
    if (result != MA_SUCCESS) {
        printf("[ERRO] Falha ao tocar som! Código: %d\n", result);
    } else {
        printf("[SFX] Som tocando!\n");
    }
}

void Sfx::hit_monstro() {
    if (!initialized) {
        printf("[ERRO] Engine não inicializado! Use s.start() antes!\n");
        return;
    }

    printf("[SFX] Tentando tocar o som 'hit_monstro'\n");
    ma_result result = ma_engine_play_sound(&engine, "sfx/hit_monstro.mp3", NULL);
    if (result != MA_SUCCESS) {
        printf("[ERRO] Falha ao tocar som! Código: %d\n", result);
    } else {
        printf("[SFX] Som tocando!\n");
    }
}

void Sfx::hit_player() {
    if (!initialized) {
        printf("[ERRO] Engine não inicializado! Use s.start() antes!\n");
        return;
    }

    printf("[SFX] Tentando tocar o som 'hit_player'\n");
    ma_result result = ma_engine_play_sound(&engine, "sfx/hit_player.mp3", NULL);
    if (result != MA_SUCCESS) {
        printf("[ERRO] Falha ao tocar som! Código: %d\n", result);
    } else {
        printf("[SFX] Som tocando!\n");
    }
}


void Sfx::tiro_player() {
    if (!initialized) {
        printf("[ERRO] Engine não inicializado! Use s.start() antes!\n");
        return;
    }

    printf("[SFX] Tentando tocar o som 'tiro_player'\n");
    ma_result result = ma_engine_play_sound(&engine, "sfx/tiro_player.mp3", NULL);
    if (result != MA_SUCCESS) {
        printf("[ERRO] Falha ao tocar som! Código: %d\n", result);
    } else {
        printf("[SFX] Som tocando!\n");
    }
}

void Sfx::fireball() {
    if (!initialized) {
        printf("[ERRO] Engine não inicializado! Use s.start() antes!\n");
        return;
    }

    printf("[SFX] Tentando tocar o som 'fireball'\n");
    ma_result result = ma_engine_play_sound(&engine, "sfx/fireball.mp3", NULL);
    if (result != MA_SUCCESS) {
        printf("[ERRO] Falha ao tocar som! Código: %d\n", result);
    } else {
        printf("[SFX] Som tocando!\n");
    }
}

void Sfx::cura() {
    if (!initialized) {
        printf("[ERRO] Engine não inicializado! Use s.start() antes!\n");
        return;
    }

    printf("[SFX] Tentando tocar o som 'cura'\n");
    ma_result result = ma_engine_play_sound(&engine, "sfx/cura.mp3", NULL);
    if (result != MA_SUCCESS) {
        printf("[ERRO] Falha ao tocar som! Código: %d\n", result);
    } else {
        printf("[SFX] Som tocando!\n");
    }
}

void Sfx::game_over() {
    if (!initialized) {
        printf("[ERRO] Engine não inicializado! Use s.start() antes!\n");
        return;
    }

    printf("[SFX] Tentando tocar o som 'game_over'\n");
    ma_result result = ma_engine_play_sound(&engine, "sfx/game_over.mp3", NULL);
    if (result != MA_SUCCESS) {
        printf("[ERRO] Falha ao tocar som! Código: %d\n", result);
    } else {
        printf("[SFX] Som tocando!\n");
    }
}

void Sfx::vitoria() {
    if (!initialized) {
        printf("[ERRO] Engine não inicializado! Use s.start() antes!\n");
        return;
    }

    printf("[SFX] Tentando tocar o som 'vitoria'\n");
    ma_result result = ma_engine_play_sound(&engine, "sfx/vitoria.mp3", NULL);
    if (result != MA_SUCCESS) {
        printf("[ERRO] Falha ao tocar som! Código: %d\n", result);
    } else {
        printf("[SFX] Som tocando!\n");
    }
}

void Sfx::musicaPrincipalStart(const char* filename, bool loop) {
    if (!initialized) {
        printf("[ERRO] Engine não inicializado!\n");
        return;
    }

    if (musicLoaded) {
        musicaPrincipalStop();
    }

    printf("[SFX] Carregando música: %s\n", filename);

    if (ma_sound_init_from_file(&engine, filename, 0, NULL, NULL, &musica_principal) != MA_SUCCESS) {
        printf("[ERRO] Falha ao carregar música!\n");
        return;
    }

    musicLoaded = true;
    ma_sound_set_looping(&musica_principal, loop);

    ma_sound_start(&musica_principal);
    printf("[SFX] Música tocando! Loop: %s\n", loop ? "Sim" : "Não");
}

void Sfx::musicaPrincipalStop(){
    if (!musicLoaded)
        return;

    ma_sound_stop(&musica_principal);
    ma_sound_uninit(&musica_principal);
    musicLoaded = false;
    printf("[SFX] Música parada!\n");
}
