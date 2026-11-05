#pragma once
#include <stdbool.h>
#include <stdint.h>

// Valores iniciais para teste fisico, nao especificacoes do receptor.
#define IR_FILTRO_LIVRE_US 2000
#define IR_FILTRO_BLOQUEIO_US 2000
#define IR_FILTRO_RETORNO_US 2000

#ifdef __cplusplus
#define IR_INLINE constexpr
#else
#define IR_INLINE static inline
#endif

typedef struct {
    bool emitindo, observando, livre, referencia_livre, bloqueado, passagem;
    int64_t valido_apos_us;
    uint32_t passagens;
    bool amostra_valida, nivel_alto;
    int64_t nivel_desde_us, bloqueio_us, ultimo_bloqueio_us;
    uint32_t pulsos_rejeitados;
} ir_eventos_t;

IR_INLINE void ir_eventos_limpar_filtro(ir_eventos_t *e) {
    e->amostra_valida=false;
    e->referencia_livre=false;
    e->bloqueado=false;
    e->bloqueio_us=0;
}

IR_INLINE void ir_eventos_emitir(ir_eventos_t *e, bool ligado, int64_t agora_us) {
    e->emitindo=ligado;
    // A pausa de 100 ms nunca pode completar bloqueio ou retorno pendente.
    ir_eventos_limpar_filtro(e);
    if(ligado) e->valido_apos_us=agora_us+1000;
}

IR_INLINE void ir_eventos_observar(ir_eventos_t *e) {
    e->observando=true;
    ir_eventos_limpar_filtro(e);
    e->passagem=false;
}

IR_INLINE void ir_eventos_validar_nivel(ir_eventos_t *e, int64_t duracao_us) {
    if(e->nivel_alto) {
        if(e->referencia_livre && duracao_us>=IR_FILTRO_BLOQUEIO_US) {
            e->bloqueado=true;
            e->bloqueio_us=duracao_us;
        }
    } else {
        if(e->bloqueado && duracao_us>=IR_FILTRO_RETORNO_US && !e->passagem) {
            e->passagem=true;
            e->passagens++;
            e->ultimo_bloqueio_us=e->bloqueio_us;
        }
        if(duracao_us>=IR_FILTRO_LIVRE_US) e->referencia_livre=true;
    }
}

IR_INLINE void ir_eventos_amostrar(ir_eventos_t *e, int nivel, int64_t agora_us) {
    if(!e->emitindo || agora_us<e->valido_apos_us) return;
    e->livre=nivel==0;
    if(!e->observando) return;
    bool alto=nivel!=0;
    if(!e->amostra_valida) {
        e->amostra_valida=true;
        e->nivel_alto=alto;
        e->nivel_desde_us=agora_us;
        return;
    }
    int64_t duracao_us=agora_us-e->nivel_desde_us;
    // Valida o nivel anterior ANTES de registrar a borda. Assim um ciclo
    // completo entre dois ticks do controlador tambem pode ser capturado.
    ir_eventos_validar_nivel(e,duracao_us);
    if(alto!=e->nivel_alto) {
        if(e->nivel_alto) {
            if(e->referencia_livre && !e->bloqueado && !e->passagem)
                e->pulsos_rejeitados++;
        } else {
            if(e->bloqueado && duracao_us<IR_FILTRO_RETORNO_US && !e->passagem) {
                e->pulsos_rejeitados++;
                e->bloqueado=false;
            }
            e->referencia_livre=duracao_us>=IR_FILTRO_LIVRE_US;
        }
        e->nivel_alto=alto;
        e->nivel_desde_us=agora_us;
    }
}

#undef IR_INLINE
