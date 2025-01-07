#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <uv.h>

uv_loop_t *loop;
uint8_t player_count = 0, RAM[16000] = {0};

typedef struct {
 uint8_t r, g, b;
} pixel_t;

typedef struct {
 uint8_t REG[8], flags, tick;
 uint16_t PC, SP[2], MEM_START;
 pixel_t screen[2][64][64];
} CPU_t;
CPU_t CPU[16];

struct sockaddr_in addr;

enum {
 FLAG_OVERFLOW=0,
 FLAG_DIV0,
 FLAG_UNDERSTACK,
};

enum {
 _A=0,
 _B,
 _C,
 _D,
 _E,
 _F,
 _G,
 _H,
};

typedef enum {
 _NOP=0,
 _MVA,//X = byte
 _MVB,//
 _MVC,//
 _MVD,//
 _MVE,//
 _MVF,//
 _MVG,//
 _MVH,//
 _CPY,//A..B
 _ADA,//A..B = X
 _ADB,//
 _ADC,//
 _ADD,//
 _ADE,//
 _ADF,//
 _ADG,//
 _ADH,//
 _AIA,//X += byte
 _AIB,//
 _AIC,//
 _AID,//
 _AIE,//
 _AIF,//
 _AIG,//
 _AIH,//
 _SBA,//X = A-B
 _SBB,//
 _SBC,//
 _SBD,//
 _SBE,//
 _SBF,//
 _SBG,//
 _SBH,//
 _SIA,//X -= byte
 _SIB,//
 _SIC,//
 _SID,//
 _SIE,//
 _SIF,//
 _SIG,//
 _SIH,//
 _BSL,//A <<= B
 _BSR,//A >>= B
 _MRA,//X = MEM[A..B]
 _MRB,//
 _MRC,//
 _MRD,//
 _MRE,//
 _MRF,//
 _MRG,//
 _MRH,//
 _MWA,//MEM[A..B] = X
 _MWB,//
 _MWC,//
 _MWD,//
 _MWE,//
 _MWF,//
 _MWG,//
 _MWH,//
 _IGT,//A > B    | skip next if wrong
 _IGA,//X > byte |
 _IGB,//         |
 _IGC,//         |
 _IGD,//         |
 _IGE,//         |
 _IGF,//         |
 _IGG,//         |
 _IGH,//         |
 _ILT,//A < B    |
 _ILA,//X < byte |
 _ILB,//         |
 _ILC,//         |
 _ILD,//         |
 _ILE,//         |
 _ILF,//         |
 _ILG,//         |
 _ILH,//         |
 _JMP,//PC = A..B
 _PSH,//Pushes A to stack
 _POP,//Pops stack to A
 _CAL,//Pushes PC to stack and PC = A..B
 _RET,//Pops stack to PC
} opcode_t;

//#################################//

uint8_t ADD(CPU_t sys, uint8_t A, uint8_t B) {
 if ((int)(A+B)>0xFF) sys.flags|=FLAG_OVERFLOW;
 return A+B;
}

uint8_t SUB(CPU_t sys, uint8_t A, uint8_t B) {
 if ((int)(A-B)<0) sys.flags|=FLAG_OVERFLOW;
 return A-B;
}

void IFT(CPU_t sys, bool statement) {
 if (!statement) { sys.PC += 2; }
}

void CPU_exec(CPU_t sys) {
 if(sys.tick-- == 0) {
  sys.tick = 4;
  uint8_t args[3] = {RAM[sys.PC+1], (RAM[sys.PC+1]>>8)&0b111, RAM[sys.PC+1]&0b111};
  switch(RAM[sys.PC]) {
   //case _NOP:
   case _MVA://X = byte
    sys.REG[_A] = args[0];
   case _MVB://
    sys.REG[_B] = args[0];
   case _MVC://
    sys.REG[_C] = args[0];
   case _MVD://
    sys.REG[_D] = args[0];
   case _MVE://
    sys.REG[_E] = args[0];
   case _MVF://
    sys.REG[_F] = args[0];
   case _MVG://
    sys.REG[_G] = args[0];
   case _MVH://
    sys.REG[_H] = args[0];
   case _CPY://A = B
    sys.REG[args[1]] = sys.REG[args[2]];
   case _ADA://X = A+B
    sys.REG[_A] = ADD(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _ADB://
    sys.REG[_B] = ADD(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _ADC://
    sys.REG[_C] = ADD(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _ADD://
    sys.REG[_D] = ADD(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _ADE://
    sys.REG[_E] = ADD(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _ADF://
    sys.REG[_F] = ADD(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _ADG://
    sys.REG[_G] = ADD(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _ADH://
    sys.REG[_H] = ADD(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _AIA://X =+ byte
    sys.REG[_A] = ADD(sys, sys.REG[_A], sys.REG[args[0]]);
   case _AIB://
    sys.REG[_B] = ADD(sys, sys.REG[_B], sys.REG[args[0]]);
   case _AIC://
    sys.REG[_C] = ADD(sys, sys.REG[_C], sys.REG[args[0]]);
   case _AID://
    sys.REG[_D] = ADD(sys, sys.REG[_D], sys.REG[args[0]]);
   case _AIE://
    sys.REG[_E] = ADD(sys, sys.REG[_E], sys.REG[args[0]]);
   case _AIF://
    sys.REG[_F] = ADD(sys, sys.REG[_F], sys.REG[args[0]]);
   case _AIG://
    sys.REG[_G] = ADD(sys, sys.REG[_G], sys.REG[args[0]]);
   case _AIH://
    sys.REG[_H] = ADD(sys, sys.REG[_H], sys.REG[args[0]]);
   case _SBA://X = A-B
    sys.REG[_A] = SUB(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _SBB://
    sys.REG[_B] = SUB(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _SBC://
    sys.REG[_C] = SUB(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _SBD://
    sys.REG[_D] = SUB(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _SBE://
    sys.REG[_E] = SUB(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _SBF://
    sys.REG[_F] = SUB(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _SBG://
    sys.REG[_G] = SUB(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _SBH://
    sys.REG[_H] = SUB(sys, sys.REG[args[1]], sys.REG[args[2]]);
   case _SIA://X =- byte
    sys.REG[_A] = SUB(sys, sys.REG[_A], sys.REG[args[0]]);
   case _SIB://
    sys.REG[_B] = SUB(sys, sys.REG[_B], sys.REG[args[0]]);
   case _SIC://
    sys.REG[_C] = SUB(sys, sys.REG[_C], sys.REG[args[0]]);
   case _SID://
    sys.REG[_D] = SUB(sys, sys.REG[_D], sys.REG[args[0]]);
   case _SIE://
    sys.REG[_E] = SUB(sys, sys.REG[_E], sys.REG[args[0]]);
   case _SIF://
    sys.REG[_F] = SUB(sys, sys.REG[_F], sys.REG[args[0]]);
   case _SIG://
    sys.REG[_G] = SUB(sys, sys.REG[_G], sys.REG[args[0]]);
   case _SIH://
    sys.REG[_H] = SUB(sys, sys.REG[_H], sys.REG[args[0]]);
   case _BSL://A <<= B
    sys.REG[args[1]] <<= sys.REG[args[2]];
   case _BSR://A >>= B
    sys.REG[args[1]] >>= sys.REG[args[2]];
   case _MRA://X = MEM[A..B]
    sys.REG[_A] = RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]];
   case _MRB://
    sys.REG[_B] = RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]];
   case _MRC://
    sys.REG[_C] = RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]];
   case _MRD://
    sys.REG[_D] = RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]];
   case _MRE://
    sys.REG[_E] = RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]];
   case _MRF://
    sys.REG[_F] = RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]];
   case _MRG://
    sys.REG[_G] = RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]];
   case _MRH://
    sys.REG[_H] = RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]];
   case _MWA://MEM[A..B] = X
    RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]] = sys.REG[_A];
   case _MWB://
    RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]] = sys.REG[_B];
   case _MWC://
    RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]] = sys.REG[_C];
   case _MWD://
    RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]] = sys.REG[_D];
   case _MWE://
    RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]] = sys.REG[_E];
   case _MWF://
    RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]] = sys.REG[_F];
   case _MWG://
    RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]] = sys.REG[_G];
   case _MWH://
    RAM[(sys.REG[args[1]]<<8)|sys.REG[args[2]]] = sys.REG[_H];
   case _IGT://A > B    | skip next if wrong
    IFT(sys, sys.REG[args[1]] > sys.REG[args[2]]);
   case _IGA://X > byte |
    IFT(sys, sys.REG[_A] > args[0]);
   case _IGB://         |
    IFT(sys, sys.REG[_B] > args[0]);
   case _IGC://         |
    IFT(sys, sys.REG[_C] > args[0]);
   case _IGD://         |
    IFT(sys, sys.REG[_D] > args[0]);
   case _IGE://         |
    IFT(sys, sys.REG[_E] > args[0]);
   case _IGF://         |
    IFT(sys, sys.REG[_F] > args[0]);
   case _IGG://         |
    IFT(sys, sys.REG[_G] > args[0]);
   case _IGH://         |
    IFT(sys, sys.REG[_H] > args[0]);
   case _ILT://A < B    |
    IFT(sys, sys.REG[args[1]] < sys.REG[args[2]]);
   case _ILA://X < byte |
    IFT(sys, sys.REG[_A] < args[0]);
   case _ILB://         |
    IFT(sys, sys.REG[_B] < args[0]);
   case _ILC://         |
    IFT(sys, sys.REG[_C] < args[0]);
   case _ILD://         |
    IFT(sys, sys.REG[_D] < args[0]);
   case _ILE://         |
    IFT(sys, sys.REG[_E] < args[0]);
   case _ILF://         |
    IFT(sys, sys.REG[_F] < args[0]);
   case _ILG://         |
    IFT(sys, sys.REG[_G] < args[0]);
   case _ILH://         |
    IFT(sys, sys.REG[_H] < args[0]);
   case _JMP://PC = A..B
    sys.PC = ((sys.REG[args[1]]<<8)|sys.REG[args[2]])-2;
   case _PSH://Pushes A to stack
    RAM[sys.SP[0]--] = sys.REG[args[1]];
   case _POP://Pops stack to A
    if (sys.SP [0] < sys.SP[1]) {
     RAM[sys.SP[0]++] = sys.REG[args[1]];
    } else { sys.flags|=FLAG_UNDERSTACK; }
   case _CAL://Pushes PC to stack and PC = A..B
    RAM[sys.SP[0]--] = sys.PC>>8;
    RAM[sys.SP[0]--] = sys.PC&0xFF;
    sys.PC = ((sys.REG[args[1]]<<8)|sys.REG[args[2]])-2;
   case _RET://Pops stack to PC
    if (sys.SP[0]+1 < sys.SP[1]) {
     sys.PC = (RAM[sys.SP[0]])|RAM[sys.SP[0]++]<<8;
    }
   //case default:
    //Do Nothing
  }
 }
 sys.PC+=2;
}


//#################################//

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
 buf->base = (char*)malloc(suggested_size);
 buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status) {
 if (status) { fprintf(stderr, "Write error %s\n", uv_strerror(status)); free(req); }
}

void client_loop(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
 if (nread < 0) {
  if (nread != UV_EOF) {
   fprintf(stderr, "Read error %s\n", uv_err_name(nread));
   uv_close((uv_handle_t*) client, NULL);
  }
 } else if (nread > 0) {
  uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
  uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
  uv_write(req, client, &wrbuf, 1, echo_write);
 }

 if (buf->base) {
  free(buf->base);
 }
}

void on_new_connection(uv_stream_t *server, int status) {
 if (status < 0) { fprintf(stderr, "New connection error %s\n", uv_strerror(status)); return; }
 
 uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
 uv_tcp_init(loop, client);
 if (uv_accept(server, (uv_stream_t*) client) == 0 && player_count<16) {
  player_count++;
  uv_read_start((uv_stream_t*)client, alloc_buffer, client_loop);
 } else {
  uv_close((uv_handle_t*) client, NULL);
 }
}

int main() {
 loop = uv_default_loop();
 
 uv_tcp_t server;
 uv_tcp_init(loop, &server);
 
 uv_ip4_addr("0.0.0.0", 7000, &addr);
 
 uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
 int r = uv_listen((uv_stream_t*)&server, 128, on_new_connection);
 if (r) { fprintf(stderr, "Listen error %s\n", uv_strerror(r)); return 1; }
 return uv_run(loop, UV_RUN_DEFAULT);
}
