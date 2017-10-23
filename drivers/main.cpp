
#include "frame.h"
#include "core/system.h"

#include "iusb.h"
#include <stdio.h>
#include <time.h>
#include "rtl-sdr.h"
#include <vector>

template<typename T>
void fft(int n, T *ar, T *ai)
{
    int m, mq, irev, i, j, j1, j2, j3, k;
    float theta, w1r, w1i, w3r, w3i;
    float x0r, x0i, x1r, x1i, x3r, x3i;
    
    /* ---- scrambler ---- */
    i = 0;
    for (j = 1; j < n - 1; j++) {
        for (k = n >> 1; k > (i ^= k); k >>= 1);
        if (j < i) {
            x0r = ar[j];
            x0i = ai[j];
            ar[j] = ar[i];
            ai[j] = ai[i];
            ar[i] = x0r;
            ai[i] = x0i;
        }
    }
    /* ---- L shaped butterflies ---- */
    theta = -2 * atanf(1.0) / n;
    for (m = 4; m <= n; m <<= 1) {
        mq = m >> 2;
        /* ---- W == 1 ---- */
        for (k = mq; k >= 1; k >>= 2) {
            for (j = mq - k; j < mq - (k >> 1); j++) {
                j1 = j + mq;
                j2 = j1 + mq;
                j3 = j2 + mq;
                x1r = ar[j] - ar[j1];
                x1i = ai[j] - ai[j1];
                ar[j] += ar[j1];
                ai[j] += ai[j1];
                x3r = ar[j3] - ar[j2];
                x3i = ai[j3] - ai[j2];
                ar[j2] += ar[j3];
                ai[j2] += ai[j3];
                ar[j1] = x1r - x3i;
                ai[j1] = x1i + x3r;
                ar[j3] = x1r + x3i;
                ai[j3] = x1i - x3r;
            }
        }
        if (m == n) continue;
        /* ---- W == exp(-pi*i/4) ---- */
        irev = n >> 1;
        w1r = cosf(theta * irev);
        for (k = mq; k >= 1; k >>= 2) {
            for (j = m + mq - k; j < m + mq - (k >> 1); j++) {
                j1 = j + mq;
                j2 = j1 + mq;
                j3 = j2 + mq;
                x1r = ar[j] - ar[j1];
                x1i = ai[j] - ai[j1];
                ar[j] += ar[j1];
                ai[j] += ai[j1];
                x3r = ar[j3] - ar[j2];
                x3i = ai[j3] - ai[j2];
                ar[j2] += ar[j3];
                ai[j2] += ai[j3];
                x0r = x1r - x3i;
                x0i = x1i + x3r;
                ar[j1] = w1r * (x0r + x0i);
                ai[j1] = w1r * (x0i - x0r);
                x0r = x1r + x3i;
                x0i = x1i - x3r;
                ar[j3] = w1r * (-x0r + x0i);
                ai[j3] = w1r * (-x0i - x0r);
            }
        }
        /* ---- W != 1, exp(-pi*i/4) ---- */
        for (i = 2 * m; i < n; i += m) {
            for (k = n >> 1; k > (irev ^= k); k >>= 1);
            w1r = cosf(theta * irev);
            w1i = sinf(theta * irev);
            w3r = cosf(theta * 3 * irev);
            w3i = sinf(theta * 3 * irev);
            for (k = mq; k >= 1; k >>= 2) {
                for (j = i + mq - k; j < i + mq - (k >> 1); j++) {
                    j1 = j + mq;
                    j2 = j1 + mq;
                    j3 = j2 + mq;
                    x1r = ar[j] - ar[j1];
                    x1i = ai[j] - ai[j1];
                    ar[j] += ar[j1];
                    ai[j] += ai[j1];
                    x3r = ar[j3] - ar[j2];
                    x3i = ai[j3] - ai[j2];
                    ar[j2] += ar[j3];
                    ai[j2] += ai[j3];
                    x0r = x1r - x3i;
                    x0i = x1i + x3r;
                    ar[j1] = w1r * x0r - w1i * x0i;
                    ai[j1] = w1r * x0i + w1i * x0r;
                    x0r = x1r + x3i;
                    x0i = x1i - x3r;
                    ar[j3] = w3r * x0r - w3i * x0i;
                    ai[j3] = w3r * x0i + w3i * x0r;
                }
            }
        }
    }
    /* ---- radix 2 butterflies ---- */
    mq = n >> 1;
    for (k = mq; k >= 1; k >>= 2) {
        for (j = mq - k; j < mq - (k >> 1); j++) {
            j1 = mq + j;
            x0r = ar[j] - ar[j1];
            x0i = ai[j] - ai[j1];
            ar[j] += ar[j1];
            ai[j] += ai[j1];
            ar[j1] = x0r;
            ai[j1] = x0i;
        }
    }
}

void check(std::vector<uint8_t> &wave){
    std::vector<uint8_t> ai;
    ai.resize(wave.size());
    memset(&*ai.begin(),0,ai.size());
    fft(wave.size(),&*wave.begin(),&*ai.begin());
    const int S=64;
    int s[S];
    int ave=0;
    memset(s,0,sizeof(s));
    for(int i=0;i<S;i++){
        int offset=wave.size()/S*i;
        for(int j=0;j<wave.size()/S;j++){
            s[i]+=sqrtf((int)wave[i+j]*wave[i+j]+(int)ai[i+j]*ai[i+j]);
        }
        ave+=s[i];
    }
    ave/=S;
    ave=100000;
    printf("%6d: ",ave);
    for(int i=0;i<S;i++){
        s[i]-=ave;
        //int P=(int)(fabsf(powf(s[i]/100.0f,5)/1000));
        int P=abs(s[i]);
        if(P)
            printf("%4d ",P);
        else
            printf("     ");
    }   
    printf("\n");
    wave.clear();
}

#include <deque>
volatile bool running=true;
volatile bool endthread=false;
std::vector<uint8_t> deq;
wts::CriticalSection *deq_cs=NULL;
volatile int tune_hz=0;
static rtlsdr_dev_t *dev = NULL;
void writeout(const uint8_t *data,int size){
    wts::CriticalSectionBlock blk(deq_cs);
    deq.insert(deq.end(),data,data+size);
    if(1024*1024<deq.size()){
        deq.erase(deq.begin(),deq.begin()+deq.size()-1024*1024);
    }
    if(tune_hz){
        printf("Tune : %.2fMhz\n",tune_hz/1000000.0f);
        rtlsdr_set_center_freq(dev, tune_hz);
        tune_hz=0;
    }
}
void peek(uint8_t *data,int size){
    wts::CriticalSectionBlock blk(deq_cs);
    if(0==deq.size()){
        memset(data,0x80,size);
    }else if(deq.size()<size){
        memcpy(data,&*deq.begin(),deq.size());
        memset(data+deq.size(),0x80,size-deq.size());
    }else{
        memcpy(data,&*deq.begin(),size);
    }
}

void receiver(void*){
    #define DEFAULT_SAMPLE_RATE		2048000
    static uint32_t samp_rate = DEFAULT_SAMPLE_RATE;

    #define MHZ(x)	(int)((x)*1000*1000)

	int r = rtlsdr_open(&dev,0);
	int gains[100];
	int count = rtlsdr_get_tuner_gains(dev, NULL);
	fprintf(stderr, "Supported gain values (%d): ", count);
	count = rtlsdr_get_tuner_gains(dev, gains);
	for (int i = 0; i < count; i++)
		fprintf(stderr, "%.1f ", gains[i] / 10.0);
	fprintf(stderr, "\n");

    r = rtlsdr_set_sample_rate(dev,samp_rate);
	r = rtlsdr_set_center_freq(dev, MHZ(81.3));
    r = rtlsdr_set_tuner_gain_mode(dev, 0);
    //r = rtlsdr_set_direct_sampling(dev,1);
	r = rtlsdr_reset_buffer(dev);

    time_t t=time(0);
    uint8_t buffer[512*32];
    int frames=0;
    int sum_read=0;
    int speed=0;
    while(running){
        int n_read=0;
        rtlsdr_read_sync(dev, buffer+sum_read, sizeof(buffer)-sum_read, &n_read);
        sum_read+=n_read;
        if(sum_read>sizeof(buffer)-512)
        {
            writeout(buffer,sum_read);
            sum_read=0;
        }
        speed+=n_read;
        if(t!=time(0)){
            printf("%d bytes/sec\n",speed);
            t=time(0);
            speed=0;
        }
    }

	rtlsdr_close(dev);
    endthread=true;
}

void onCreate()
{
    deq_cs=wts::CreateCriticalSection();
    wts::StartThread(receiver,1024*1024,NULL);
}

void onSurfaceCreated()
{
#if defined(ANDROID)
    enum wts::gfx::TextureFormat back_fmt=wts::gfx::TEXTURE_ABGR8;
    gf=wts::gfx::CreateGraphicsEGL();
#elif defined(EMSCRIPTEN)
    enum wts::gfx::TextureFormat back_fmt=wts::gfx::TEXTURE_ABGR8;
    gf=wts::gfx::CreateGraphicsEGL();
#elif defined(WINDOWS)
    
#define D3D
    enum wts::gfx::TextureFormat back_fmt=wts::gfx::TEXTURE_ARGB8;
    gf=wts::gfx::CreateGraphicsD3D();
    /*/
    enum wts::gfx::TextureFormat back_fmt=wts::gfx::TEXTURE_ABGR8;
    gf=wts::gfx::CreateGraphicsEGL();*/

    gf->InitializeFromViewport(vp);
#else
    gf=wts::gfx::CreateGraphicsMock();
#endif
    gf->SetBgColor(0xff000000);

    wts::gfx::CreateManagedGraphics(&mg);
    mg->Initialize(gf);
}

wts::gfx::Texture *spector;
uint32_t *spector_rgb=NULL;
void onSurfaceChanged(int width,int height)
{
#if defined(ANDROID)
    gf->Initialize(width,height);
#elif defined(WINDOWS)
#else
#endif
    mg->directional_light_->Direction(wts::gfx::Vec3(-1.5f,1,-3));
    mg->directional_light_enable_=false;

    gf->SetRenderState(wts::gfx::RENDER_Z,wts::gfx::VALUE_DISABLE);
	gf->SetRenderState(wts::gfx::RENDER_CULLMODE,wts::gfx::CULL_NONE);

    mg->camera_->PerspectiveFov(1,1000,wts::gfx::PI/3,(float)gf->Height()/gf->Width());
    mg->camera_->LookAt(
        wts::gfx::Vec3(0,20,-100),
        wts::gfx::Vec3(0,50,0),
        wts::gfx::Vec3(0,1,0));
    gf->CreateTexture(&spector,1024,256,wts::gfx::TEXTURE_ARGB8);
    spector_rgb=new uint32_t[1024*256];
}

void onDestroy()
{
    running=false;
    while (!endthread)
    {
        wts::Sleep(1);
    }
    wts::DeleteCriticalSection(deq_cs);
    delete[] spector_rgb;
    exit(0);
}

void onResume()
{

}

void onPause()
{
    running=false;
    while (!endthread)
    {
        wts::Sleep(1);
    }
    wts::DeleteCriticalSection(deq_cs);
    exit(0);
}

static wts::gfx::Vector<2,int> last_pos;
static bool is_touch=false;;

void onTouch(int x,int y)
{
    is_touch=true;
    last_pos.x=x;
    last_pos.y=y;
    tune_hz=x*1000000/5;
    if(y>256)
        tune_hz*=5;
}

void onTouchMove(int x,int y)
{
    last_pos.x=x;
    last_pos.y=y;
    tune_hz=x*1000000/5;
    if(y>256)
        tune_hz*=5;
}

void onTouchUp()
{
}

void onZoom(float f)
{
}

void onDrawFrame()
{
    static int three=0;
    using namespace wts::gfx;
    uint64_t now_time=wts::GetTime();
    int delta_time=(three++)%3?17:16;//now_time-last_time;

    gf->Clear();
    gf->BeginScene();

    static const int S=1024*64;
    uint8_t wave[S];
    peek(wave,sizeof(wave));

    //render wave
    {
        typedef Fertex<FTX_POSITION|FTX_DIFFUSE> line_t;
        line_t *line=new line_t[S*2];
        for(int i=0;i<S;i++){
            line[i*2+0].position.x=(2.0f/S)*i-1.0f;
            line[i*2+0].position.y=0.75f;
            line[i*2+0].position.z=0;
            line[i*2+0].diffuse=0x20ffffff;
            line[i*2+1].position.x=line[i*2+0].position.x;
            line[i*2+1].position.y=wave[i]/512.0f+0.5f;
            line[i*2+1].position.z=0;
            line[i*2+1].diffuse=0x20ffffff;
        }
        gf->SetAlphaBlendMode(ALPHABLEND_ADD);
        gf->SetShader(shader::ShaderEnd(),0);
        gf->DrawPrimitiveUP(PRIMITIVE_LINELIST,S,line,line_t::format);
        delete[]line;
    }
    int ar[S],ai[S];
    memset(ai,0,sizeof(ai));
    for(int j=0;j<S;j++){
        ar[j]=(int)wave[j]-128;
    }
    fft(S,ar,ai);
    for(int j=0;j<S;j++){
        ai[j]=(ar[j]*ar[j]+ai[j]*ai[j])/(65536*128);
    }
    memmove(spector_rgb,spector_rgb+1024,sizeof(uint32_t)*1024*255);
    int power[1024];
    for(int i=0;i<1024;i++){
        power[i]=0;
    }
    //render spector
    {
        typedef Fertex<FTX_POSITION|FTX_DIFFUSE> line_t;
        line_t *line=new line_t[S];
        for(int i=0;i<S/2;i++){
            line[i*2+0].position.x=(4.0f/S)*i-1.0f;
            line[i*2+0].position.y=0;
            line[i*2+0].position.z=0;
            line[i*2+0].diffuse=0x2000ff00;
            line[i*2+0].diffuse.a=ai[i]/256.0f;
            line[i*2+0].diffuse.r=ai[i]/256.0f;
            line[i*2+1].position.x=line[i*2+0].position.x;
            line[i*2+1].position.y=ai[i]/512.0f;
            line[i*2+1].position.z=0;
            line[i*2+1].diffuse=0x80000000;
            line[i*2+1].diffuse.r=ai[i]/256.0f;
            power[i*1024/(S/2)]+=ai[i];
        }
        gf->SetAlphaBlendMode(ALPHABLEND_ADD);
        gf->SetShader(shader::ShaderEnd(),0);
        gf->DrawPrimitiveUP(PRIMITIVE_LINELIST,S/2,line,line_t::format);
        delete[]line;
    }
    uint32_t *ll=spector_rgb+1024*254;
    for(int i=0;i<1024;i++){
        power[i]/=4;
        if(power[i]>255)
            power[i]=255;
        ll[i]=0xff000000;
        ll[i]+=0x010101*power[i];
    }
    {
        wts::gfx::LockInfo li;
        spector->Lock(li,wts::gfx::LOCK_WRITE);
        for(int i=0;i<256;i++){
            memcpy(li.Bits+li.Pitch*i,spector_rgb+1024*i,sizeof(uint32_t)*1024);
        }
        spector->Unlock();
        Fertex<FTX_POSITION|FTX_TEX1> v[4];
        v[0].position=Vec3(-1.0f,-1.0f,0.0f);
        v[0].u=0.0f;
        v[1].position=Vec3(-1.0f, 0.0f,0.0f);
        v[1].u=0.0f;
        v[2].position=Vec3( 1.0f,-1.0f,0.0f);
        v[2].u=1.0f;
        v[3].position=Vec3( 1.0f, 0.0f,0.0f);
        v[3].u=1.0f;
        v[0].v=0.0f;
        v[1].v=1.0f;
        v[2].v=0.0f;
        v[3].v=1.0f;
        gf->SetAlphaBlendMode(ALPHABLEND_NONE);
        mg->decal_texture_->SetTexture(spector);
        gf->SetShader(mg->decal_texture_,0);
        gf->SetShader(shader::ShaderEnd(),1);
        gf->DrawPrimitiveUP(PRIMITIVE_TRIANGLESTRIP,2,v,v[0].format);
    }

    gf->EndScene();
    gf->Flip(vp);

}

