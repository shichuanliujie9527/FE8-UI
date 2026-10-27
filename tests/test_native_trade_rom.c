/* Private local integration test. No ROM/state is a CI fixture. Only the
 * game's own button-driven trade code may change the emulated inventory. */
#include <mgba/flags.h>
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba/core/serialize.h>
#include <mgba-util/vfs.h>
#include "address_space.h"
#include "native_trade.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif
static uint8_t read8(void *c,uint32_t a){struct mCore *core=c;return core->busRead8(core,a);}
static void wait_tutorial(struct mCore *core,const Fe8MemoryReader *r,const Fe8Profile *p,int tutorial) {
    for(int i=0;i<2400;++i) {
        Fe8NativeTrade s;int valid=fe8_native_trade_read(r,p,&s);
        if(valid && !s.blocked && s.tutorial==tutorial) {core->setKeys(core,0);return;}
        /* After the transfer, the game clears the selection callback while
         * its completion dialogue blocks the parent. The production adapter
         * deliberately rejects that transient state; ordinary A still advances
         * dialogue, just as the frontend's generic click fallback does. */
        uint32_t proc=core->busRead32(core,p->trade_proc_pointer);
        int dialogue=proc>=0x02000000 && proc<=0x0203FF94 && !(proc&3) &&
            core->busRead32(core,proc)==p->trade_script && core->busRead8(core,proc+0x28);
        core->setKeys(core,dialogue && i%40<3?1:0);core->runFrame(core);
    }
    fprintf(stderr,"Tutorial %d did not become ready\n",tutorial);assert(0);
}
static void click(struct mCore *core,const Fe8MemoryReader *r,const Fe8Profile *p,int x,int y) {
    Fe8NativeTrade s;Fe8TradeMouse mouse={0};assert(fe8_native_trade_read(r,p,&s));
    assert(fe8_trade_mouse_click(&mouse,&s,x,y));
    for(int i=0;i<180;++i) {
        int valid=fe8_native_trade_read(r,p,&s);
        core->setKeys(core,fe8_trade_mouse_update(&mouse,&s,valid));core->runFrame(core);
    }
    assert(!mouse.active);core->setKeys(core,0);
}
static int run(int argc,char **argv) {
    assert(argc==3 || argc==4);
    struct VFile *rom=VFileOpen(argv[1],O_RDONLY);assert(rom);
    struct mCore *core=mCoreFindVF(rom);rom->close(rom);assert(core && core->init(core));
    mCoreInitConfig(core,"native-trade-test");rom=VFileOpen(argv[1],O_RDONLY);
    struct mStandardLogger logger; mStandardLoggerInit(&logger);
    logger.d.filter->defaultLevels=mLOG_FATAL|mLOG_ERROR; mLogSetDefaultLogger(&logger.d);
    assert(rom && core->loadROM(core,rom));
    mColor *pixels=calloc(240*160,sizeof(*pixels));assert(pixels);core->setVideoBuffer(core,pixels,240);core->reset(core);
    struct VFile *state=VFileOpen(argv[2],O_RDONLY);assert(state);
    assert(mCoreLoadStateNamed(core,state,SAVESTATE_ALL));state->close(state);
    Fe8AddressSpace space;fe8_address_space_init(&space,core,read8);
    const uint32_t bases[]={0x02000000,0x03000000,0x05000000,0x06000000,0x08000000};
    for(unsigned i=0;i<5;++i){size_t size;void *data=mCoreGetMemoryBlock(core,bases[i],&size);if(data&&size)assert(fe8_address_space_add(&space,bases[i],data,size));}
    Fe8MemoryReader reader={&space,fe8_address_space_read8};const Fe8Profile *profile=fe8_profile_for_rom(&reader);
    assert(profile->map_only && profile->trade_proc_pointer);
    wait_tutorial(core,&reader,profile,3);
    Fe8NativeTrade before,after;assert(fe8_native_trade_read(&reader,profile,&before));
    assert(before.column==1 && before.row==0 && before.items[1][2]==0x036C);
    click(core,&reader,profile,160,112); /* click first right-side vulnerary */
    wait_tutorial(core,&reader,profile,5);
    assert(fe8_native_trade_read(&reader,profile,&after));
    assert(after.phase==1 && after.column==0 && after.row==1);
    click(core,&reader,profile,32,96); /* click the offered left-side empty slot */
    wait_tutorial(core,&reader,profile,8);
    assert(fe8_native_trade_read(&reader,profile,&after));
    assert(after.items[0][0]==before.items[0][0] && after.items[0][1]==0x036C);
    assert(after.items[1][0]==before.items[1][0] && after.items[1][1]==before.items[1][1]);
    assert(after.items[1][2]==0x036C && after.items[1][3]==0);
    for(int i=0;i<90;++i){core->setKeys(core,i<3?2:0);core->runFrame(core);}
    assert(!fe8_native_trade_read(&reader,profile,&after));
    if(argc==4){state=VFileOpen(argv[3],O_CREAT|O_TRUNC|O_RDWR);assert(state);assert(mCoreSaveStateNamed(core,state,SAVESTATE_ALL));state->close(state);}
    mCoreConfigDeinit(&core->config);core->deinit(core);free(pixels);
    mLogSetDefaultLogger(NULL);mStandardLoggerDeinit(&logger);
    puts("PASS: mouse-target navigation selected vulnerary, native game transferred it, tutorial completed, trade closed.");
    return 0;
}
#ifdef _WIN32
int wmain(int argc,wchar_t **wide) {
    char **args=calloc(argc,sizeof(*args));assert(args);
    for(int i=0;i<argc;++i){int n=WideCharToMultiByte(CP_UTF8,0,wide[i],-1,NULL,0,NULL,NULL);args[i]=malloc(n);assert(args[i]);WideCharToMultiByte(CP_UTF8,0,wide[i],-1,args[i],n,NULL,NULL);}
    int result=run(argc,args);for(int i=0;i<argc;++i)free(args[i]);free(args);return result;
}
#else
int main(int argc,char **argv){return run(argc,argv);}
#endif
