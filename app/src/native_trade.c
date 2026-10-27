#include "native_trade.h"
#include <string.h>

static uint32_t read32(const Fe8MemoryReader *m, uint32_t a) {
    uint32_t value=0;
    for(unsigned i=0;i<4;++i) value|=(uint32_t)m->read8(m->context,a+i)<<(8*i);
    return value;
}
static unsigned read16(const Fe8MemoryReader *m, uint32_t a) {
    return m->read8(m->context,a) | (unsigned)m->read8(m->context,a+1)<<8;
}
int fe8_native_trade_read(const Fe8MemoryReader *m, const Fe8Profile *p, Fe8NativeTrade *out) {
    if(!out) return 0;
    memset(out,0,sizeof(*out));
    if(!m || !m->read8 || !p || !p->trade_proc_pointer || !p->trade_script) return 0;
    uint32_t proc=read32(m,p->trade_proc_pointer);
    if(proc<0x02000000 || proc>0x0203FF94 || (proc&3) || read32(m,proc)!=p->trade_script) return 0;
    uint32_t idle=read32(m,proc+12), pc=read32(m,proc+4);
    if(idle!=p->trade_idle_unselected && idle!=p->trade_idle_selected) return 0;
    if(pc<p->trade_script || pc>=p->trade_script+0x200 || ((pc-p->trade_script)&7)) return 0;
    out->proc=proc;out->phase=idle==p->trade_idle_selected;
    out->blocked=m->read8(m->context,proc+0x28)!=0;
    out->column=m->read8(m->context,proc+0x41);out->row=m->read8(m->context,proc+0x42);
    out->selected_column=m->read8(m->context,proc+0x43);out->selected_row=m->read8(m->context,proc+0x44);
    out->tutorial=m->read8(m->context,proc+0x48);
    if(out->column>1 || out->row>4 || out->tutorial>8 ||
            (out->phase && (out->selected_column>1 || out->selected_row>4))) return 0;
    for(unsigned col=0;col<2;++col) {
        uint32_t unit=read32(m,proc+0x2C+4*col);
        if(unit<p->blue_units || unit>=p->blue_units+62*0x48 || (unit-p->blue_units)%0x48) return 0;
        out->units[col]=unit;
        for(unsigned row=0;row<5;++row) {
            unsigned selectable=m->read8(m->context,proc+0x34+col*6+row);
            if(selectable>1) return 0;
            out->selectable[col][row]=(uint8_t)selectable;
            out->items[col][row]=(uint16_t)read16(m,unit+0x1E+row*2);
        }
    }
    return out->units[0]!=out->units[1] && out->selectable[out->column][out->row];
}
void fe8_trade_mouse_cancel(Fe8TradeMouse *mouse) { memset(mouse,0,sizeof(*mouse)); }
int fe8_trade_mouse_click(Fe8TradeMouse *mouse,const Fe8NativeTrade *s,int x,int y) {
    fe8_trade_mouse_cancel(mouse);
    if(!s || s->blocked || y<72 || y>=152) return 0;
    int col=x>=8 && x<116?0:x>=120 && x<232?1:-1, row=(y-72)/16;
    if(col<0 || !s->selectable[col][row]) return 0;
    /* The tutorial permits only Right, then a vulnerary, then its prescribed
       destination. Refuse other requests; do not bypass its event/key masks. */
    if(s->tutorial && s->tutorial!=2 && s->tutorial!=3 && s->tutorial!=5) return 0;
    if(s->tutorial==2 && (col!=1 || row!=0)) return 0;
    if(s->tutorial==3 && (col!=1 || (s->items[col][row]&255)!=0x6C)) return 0;
    if(s->tutorial==5 && (col!=s->column || row!=s->row)) return 0;
    mouse->initial=*s;mouse->active=1;mouse->column=col;mouse->row=row;
    mouse->release=3;
    return 1;
}
uint32_t fe8_trade_mouse_update(Fe8TradeMouse *m,const Fe8NativeTrade *s,int valid) {
    if(!m->active) return 0;
    if(!valid || !s || s->blocked || s->proc!=m->initial.proc || s->phase!=m->initial.phase ||
            (s->phase && (s->selected_column!=m->initial.selected_column || s->selected_row!=m->initial.selected_row)) ||
            s->tutorial!=m->initial.tutorial || memcmp(s->units,m->initial.units,sizeof(s->units)) ||
            memcmp(s->items,m->initial.items,sizeof(s->items)) ||
            !s->selectable[m->column][m->row] || ++m->frames>120) {
        fe8_trade_mouse_cancel(m);return 0;
    }
    if(m->press) { --m->press;return m->key; }
    if(m->release) { --m->release;return 0; }
    if(m->key==1) { fe8_trade_mouse_cancel(m);return 0; }
    if(s->column!=m->column) m->key=1u<<(m->column?4:5);
    else if(s->row!=m->row) m->key=1u<<(m->row>s->row?7:6);
    else {
        if(s->tutorial==2) { fe8_trade_mouse_cancel(m);return 0; }
        m->key=1;
    }
    m->press=2;m->release=3;return m->key;
}
