#ifndef SFX_H
#define SFX_H
#include "miniaudio.h"
class Sfx {
public:
    public:
    bool start();   // Inicializa engine
    void stop();    // Finaliza engine
    void morte_monstro();
    void hit_monstro();
    void tiro_player();
    void fireball();
    void cura();
    void hit_player();
    void game_over();
    void vitoria();
    void musicaPrincipalStart(const char* filename, bool loop);
    void musicaPrincipalStop();
private:
    ma_engine engine;
    ma_sound musica_principal;
    bool initialized = false;
    bool musicLoaded = false;
};
extern Sfx sfx;
#endif // SFX_H
