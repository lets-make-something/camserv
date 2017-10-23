#include "webcam.h"
#include <dshow.h>
#include "qedit.h"

#pragma comment(lib,"strmiids.lib")

class WebcamImpl
  :public Webcam
{
  IGraphBuilder *pGraph;
  IMediaControl *pMC;
  ICaptureGraphBuilder2 *pCapture;
  IBaseFilter *pFilter;
  ISampleGrabber *pGrab;

  int width, height;
  unsigned char *buf;
public:
  WebcamImpl();
  ~WebcamImpl();
  bool Initialize();
  void Release();
  void Start();
  void Stop();
  int Width();
  int Height();
  unsigned char *Buffer();
};

Webcam* Webcam::Create()
{
  WebcamImpl *cam = new WebcamImpl;
  if (cam->Initialize())
  {
    return cam;
  }
  cam->Release();
  delete cam;
  return false;
}

WebcamImpl::WebcamImpl()
{
  pGraph = 0;
  pMC = 0;
  pCapture = 0;
  pFilter = 0;
  pGrab = 0;
  buf = 0;
}

WebcamImpl::~WebcamImpl()
{
}

void WebcamImpl::Release()
{
  if (pGraph)
    pGraph->Release();
  if (pMC)
    pMC->Release();
  if (pCapture)
    pCapture->Release();
  if (pFilter)
    pFilter->Release();
  if (pGrab)
    pGrab->Release();
  if (buf)
    delete[]buf;
  delete (WebcamImpl*)this;
}

int WebcamImpl::Width()
{
  return width;
}

int WebcamImpl::Height()
{
  return height;
}

unsigned char*WebcamImpl::Buffer()
{
  if (pGrab)
  {
    unsigned char *temp = new unsigned char[width*height * 3];
    long sz = width*height * 3;
    pGrab->GetCurrentBuffer(&sz, (long*)temp);
    for (int y = 0; y < height; y++){
      memcpy(buf + width*y * 3, temp + width*(height - y) * 3, width * 3);
    }
  }
  return buf;
}

void WebcamImpl::Start()
{
  if (pGrab&&pMC)
  {
    pGrab->SetBufferSamples(TRUE);
    pMC->Run();
  }
}

void WebcamImpl::Stop()
{
  if (pMC&&pGrab)
  {
    pMC->Stop();
    pGrab->SetBufferSamples(FALSE);
  }
}

bool WebcamImpl::Initialize()
{
  CoInitialize(0);
  IBaseFilter  *pbf = NULL;
  IMoniker * pMoniker = NULL;
  ULONG cFetched;
  ICreateDevEnum * pDevEnum = NULL;
  CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
    IID_ICreateDevEnum, (void **)&pDevEnum);
  IEnumMoniker * pClassEnum = NULL;
  pDevEnum->CreateClassEnumerator(
    CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
  if (pClassEnum == NULL){
    pDevEnum->Release();
    return false;
  }
  pClassEnum->Next(1, &pMoniker, &cFetched);
  pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pbf);
  CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
    IID_IGraphBuilder, (void **)&pGraph);
  pGraph->QueryInterface(IID_IMediaControl, (LPVOID *)&pMC);
  pGraph->AddFilter(pbf, L"Video Capture");
  pbf->Release();

  CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
    IID_IBaseFilter, (LPVOID *)&pFilter);
  pFilter->QueryInterface(IID_ISampleGrabber, (void **)&pGrab);
  AM_MEDIA_TYPE amt;
  ZeroMemory(&amt, sizeof(AM_MEDIA_TYPE));
  amt.majortype = MEDIATYPE_Video;
  amt.subtype = MEDIASUBTYPE_RGB24;
  amt.formattype = FORMAT_VideoInfo;
  pGrab->SetMediaType(&amt);
  pGraph->AddFilter(pFilter, L"SamGra");

  CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
    IID_ICaptureGraphBuilder2, (void **)&pCapture);
  pCapture->SetFiltergraph(pGraph);
  pCapture->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
    pbf, NULL, pFilter);
  pGrab->GetConnectedMediaType(&amt);
  VIDEOINFOHEADER &vh = *(VIDEOINFOHEADER*)amt.pbFormat;
  width = vh.bmiHeader.biWidth;
  height = vh.bmiHeader.biHeight;
  buf = new BYTE[width*height * 3];
  ZeroMemory(buf, width*height * 3);
  return true;
}
