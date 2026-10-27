#include "native_trade.h"
#include <assert.h>
#include <string.h>
static uint8_t ram[4096];
static uint8_t read8(void *unused,uint32_t a) {(void)unused;assert(a>=0x02000000 && a<0x02001000);return ram[a-0x02000000];}
static void put32(unsigned a,uint32_t v) {for(unsigned i=0;i<4;++i)ram[a+i]=(uint8_t)(v>>(8*i));}
int main(void) {
    Fe8Profile p={0};p.blue_units=0x02000200;p.trade_proc_pointer=0x02000000;
    p.trade_script=0x08001000;p.trade_idle_unselected=0x08002001;p.trade_idle_selected=0x08002101;
    put32(0,0x02000100);put32(0x100,p.trade_script);put32(0x104,p.trade_script+0x68);
    put32(0x10C,p.trade_idle_unselected);put32(0x12C,p.blue_units);put32(0x130,p.blue_units+0x48);
    ram[0x134]=1;ram[0x13A]=ram[0x13B]=ram[0x13C]=1;ram[0x141]=1;
    ram[0x21E]=0x14;ram[0x266]=1;ram[0x268]=0x14;ram[0x26A]=0x6C;ram[0x26B]=3;
    Fe8MemoryReader reader={0,read8};Fe8NativeTrade s;Fe8TradeMouse mouse={0};
    assert(fe8_native_trade_read(&reader,&p,&s));
    assert(!fe8_native_trade_read(&reader,NULL,&s));
    assert(fe8_native_trade_read(&reader,&p,&s));
    assert(!fe8_trade_mouse_click(&mouse,&s,110,10));
    assert(!fe8_trade_mouse_click(&mouse,&s,118,110));
    assert(!fe8_trade_mouse_click(&mouse,&s,160,150));
    assert(fe8_trade_mouse_click(&mouse,&s,160,112));
    uint32_t previous=0;int confirmations=0;
    for(int i=0;i<100;++i) {
        uint32_t key=fe8_trade_mouse_update(&mouse,&s,1),edge=key&~previous;previous=key;
        if(edge==128) ++s.row;
        if(edge==1) {assert(s.column==1 && s.row==2);++confirmations;}
    }
    assert(confirmations==1 && !mouse.active);
    s.row=0;s.tutorial=3;
    assert(!fe8_trade_mouse_click(&mouse,&s,160,80)); /* tutorial rejects sword */
    assert(fe8_trade_mouse_click(&mouse,&s,160,112));
    s.blocked=1;assert(!fe8_trade_mouse_update(&mouse,&s,1)&&!mouse.active);
    s.blocked=0;assert(fe8_trade_mouse_click(&mouse,&s,160,112));
    s.items[1][2]=0;assert(!fe8_trade_mouse_update(&mouse,&s,1)&&!mouse.active);
    s.tutorial=0;s.phase=1;s.selectable[0][1]=1;s.column=0;s.row=1;
    assert(fe8_trade_mouse_click(&mouse,&s,32,96));
    s.phase=0;assert(!fe8_trade_mouse_update(&mouse,&s,1)&&!mouse.active);
    s.phase=1;assert(fe8_trade_mouse_click(&mouse,&s,32,96));
    assert(!fe8_trade_mouse_update(&mouse,NULL,0)&&!mouse.active);
    put32(0x100,0x08009000);assert(!fe8_native_trade_read(&reader,&p,&s));
    put32(0x100,p.trade_script);put32(0x12C,0x02000201);assert(!fe8_native_trade_read(&reader,&p,&s));
    return 0;
}
