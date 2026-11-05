// Exercita o mesmo filtro usado pela ISR, sem hardware.
#include "sensor_ir_eventos.h"
constexpr ir_eventos_t iniciar(int64_t t=0) {
    ir_eventos_t e={};ir_eventos_observar(&e);ir_eventos_emitir(&e,true,t);return e;
}
constexpr void cookie(ir_eventos_t *e, int64_t t=0, int64_t bloqueio=2000) {
    ir_eventos_amostrar(e,0,t+1000);
    ir_eventos_amostrar(e,1,t+3000);
    ir_eventos_amostrar(e,0,t+3000+bloqueio);
    ir_eventos_amostrar(e,0,t+5000+bloqueio);
}
constexpr bool passagem_filtrada() {
    auto e=iniciar();cookie(&e);
    return e.passagem && e.passagens==1 && e.ultimo_bloqueio_us==2000;
}
static_assert(passagem_filtrada(), "Aceitar limiares exatos de 2 ms");
constexpr bool tres_cookies() {
    ir_eventos_t e={};
    for(int i=0;i<3;i++) {
        int64_t t=i*200000;ir_eventos_observar(&e);ir_eventos_emitir(&e,true,t);
        if(e.passagem || e.referencia_livre || e.bloqueado) return false;
        cookie(&e,t);
        if(!e.passagem || e.passagens!=(uint32_t)(i+1)) return false;
        e.observando=false;ir_eventos_emitir(&e,false,t+100000);
    }
    return true;
}
static_assert(tres_cookies(), "Filtro identico para cada cookie");
constexpr bool confirmacao_sem_timeout() {
    auto e=iniciar();ir_eventos_amostrar(&e,0,1000);ir_eventos_amostrar(&e,1,3000);
    ir_eventos_amostrar(&e,0,60000000);ir_eventos_amostrar(&e,0,60002000);
    return e.passagem;
}
static_assert(confirmacao_sem_timeout(), "Sem erro ou limite de espera de IR");
constexpr bool ciclos_sem_cookie() {
    auto e=iniciar();
    for(int i=0;i<20;i++) {
        int64_t t=i*200000;ir_eventos_emitir(&e,true,t);
        ir_eventos_amostrar(&e,1,t+100);ir_eventos_amostrar(&e,0,t+500);
        ir_eventos_amostrar(&e,0,t+1000);ir_eventos_amostrar(&e,0,t+99000);
        ir_eventos_emitir(&e,false,t+100000);ir_eventos_amostrar(&e,1,t+100300);
    }
    return !e.passagem && !e.bloqueado && !e.referencia_livre;
}
static_assert(ciclos_sem_cookie(), "100/100 ms sem objeto nao confirma entrega");
constexpr bool referencia() {
    auto e=iniciar();ir_eventos_amostrar(&e,1,1000);ir_eventos_amostrar(&e,0,5000);
    ir_eventos_amostrar(&e,0,7000);
    return !e.passagem && e.referencia_livre;
}
static_assert(referencia(), "Exigir feixe livre estavel antes do bloqueio");
constexpr bool uma_unidade() {
    auto e=iniciar();
    for(int i=0;i<4;i++) cookie(&e,i*10000);
    return e.passagens==1 && e.passagem;
}
static_assert(uma_unidade(), "Uma confirmacao por observacao");
constexpr bool acomodacao() {
    auto e=iniciar();ir_eventos_amostrar(&e,0,100);ir_eventos_amostrar(&e,1,200);ir_eventos_amostrar(&e,0,300);
    return !e.passagem && !e.amostra_valida;
}
static_assert(acomodacao(), "Ignorar acomodacao inicial");
constexpr bool cancelar() {
    auto e=iniciar();ir_eventos_amostrar(&e,0,1000);ir_eventos_amostrar(&e,1,3000);
    e.observando=false;ir_eventos_amostrar(&e,0,6000);ir_eventos_amostrar(&e,0,8000);
    return !e.passagem;
}
static_assert(cancelar(), "Nao confirmar apos cancelar");
constexpr bool antecedencia_nao_confirma() {
    ir_eventos_t e={};ir_eventos_emitir(&e,true,0);cookie(&e);
    if(e.passagem) return false;
    ir_eventos_observar(&e);ir_eventos_amostrar(&e,0,8000);ir_eventos_amostrar(&e,0,10000);
    return !e.passagem && e.passagens==0;
}
static_assert(antecedencia_nao_confirma(), "Preparo nao confirma cookie");
constexpr bool rejeitar_ruido(int64_t largura) {
    auto e=iniciar();
    ir_eventos_amostrar(&e,0,1000);
    for(int i=0;i<10;i++) {
        int64_t t=3000+i*5000;ir_eventos_amostrar(&e,1,t);
        ir_eventos_amostrar(&e,0,t+largura);ir_eventos_amostrar(&e,0,t+largura+2000);
    }
    return !e.passagem && e.pulsos_rejeitados==10;
}
static_assert(rejeitar_ruido(100), "Rejeitar 10 picos de 100 us");
static_assert(rejeitar_ruido(500), "Rejeitar 10 picos de 500 us");
static_assert(rejeitar_ruido(1999), "Rejeitar bloqueio abaixo de 2 ms");
constexpr bool retorno_curto() {
    auto e=iniciar();ir_eventos_amostrar(&e,0,1000);ir_eventos_amostrar(&e,1,3000);
    ir_eventos_amostrar(&e,0,6000);ir_eventos_amostrar(&e,1,7999);
    ir_eventos_amostrar(&e,0,10000);ir_eventos_amostrar(&e,0,12000);
    return !e.passagem && e.pulsos_rejeitados==1;
}
static_assert(retorno_curto(), "Retorno curto nao deve acumular em outro pulso");
constexpr bool referencia_curta() {
    auto e=iniciar();ir_eventos_amostrar(&e,0,1000);ir_eventos_amostrar(&e,1,2999);
    ir_eventos_amostrar(&e,0,6000);ir_eventos_amostrar(&e,0,8000);
    return !e.passagem;
}
static_assert(referencia_curta(), "Referencia curta nao arma bloqueio");
constexpr bool pausa_descarta_pendente() {
    auto e=iniciar();ir_eventos_amostrar(&e,0,90000);ir_eventos_amostrar(&e,1,93000);
    ir_eventos_amostrar(&e,0,99000);ir_eventos_emitir(&e,false,100000);
    ir_eventos_emitir(&e,true,200000);ir_eventos_amostrar(&e,0,201000);ir_eventos_amostrar(&e,0,203000);
    return !e.passagem;
}
static_assert(pausa_descarta_pendente(), "Pausa nao completa retorno pendente");
constexpr bool entre_ticks() {
    auto e=iniciar();ir_eventos_amostrar(&e,0,1000);
    // Somente bordas da ISR entre amostragens da tarefa.
    ir_eventos_amostrar(&e,1,3000);ir_eventos_amostrar(&e,0,5000);
    ir_eventos_amostrar(&e,1,7000);
    return e.passagem && e.passagens==1;
}
static_assert(entre_ticks(), "Bordas validam nivel anterior entre ticks");
constexpr bool passagem_retida_na_pausa() {
    auto e=iniciar();cookie(&e);ir_eventos_emitir(&e,false,100000);
    ir_eventos_emitir(&e,true,200000);
    return e.passagem && e.passagens==1;
}
static_assert(passagem_retida_na_pausa(), "Pausa preserva passagem ja validada");
