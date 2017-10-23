
class Webcam
{
public:
	virtual void Release()=0;
	virtual void Start()=0;
	virtual void Stop()=0;
	virtual int Width()=0;
	virtual int Height()=0;
	virtual unsigned char *Buffer()=0;

  static Webcam* Create();
  virtual ~Webcam(){}
};

