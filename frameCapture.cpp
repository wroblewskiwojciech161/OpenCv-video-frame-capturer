#include <thread>
#include <atomic>
#include <string>  
#include "opencv2/opencv.hpp"
#include<iostream>
#include<unistd.h>
#include <chrono>
#include<string>
#include <algorithm>
using namespace std;
using namespace cv;

//klasa odpoeidzialana za zapis i generowanie datasetu z video
class DataGenerator
{ public:
   void extract_frames(const string &videoFilePath,vector<Mat>& frames,const int rate);
   void save_frames(vector<Mat>& frames, const string& outputDir);
   void convert(vector <Mat> frame);

};
//klasa odpowiedzialana za nagranie video 
class Record
{
    public:
      vector<Mat> frames_vector;
      std::atomic<int> state{1};
      cv::VideoWriter rec;
      bool connection(cv::VideoCapture video);
      void start(cv::VideoCapture cap);
      void stop();
      
      std::thread member1Thread(cv::VideoCapture cap);
      std::thread member2Thread();

};
std::thread Record :: member1Thread(cv::VideoCapture cap){
  return std::thread([=] {start(cap); });
}
std::thread Record :: member2Thread(){

  return std::thread([=] {stop(); });
}
//sprawdzanie połączenia z kamerą
bool Record :: connection(cv::VideoCapture video){
    if(!video.isOpened()){
    std::cout<<"succesfully connected with camera";
    return 0;
    }
    else{
      std::cout<<"succesfully connected with camera";
       return 1;
    }

}
void Record :: start(cv::VideoCapture cap){
   
   cv::Mat frame;
   int frame_width=cap.get(CV_CAP_PROP_FRAME_WIDTH);
   int frame_height=cap.get(CV_CAP_PROP_FRAME_HEIGHT);
   VideoWriter video("out.avi",CV_FOURCC('M','J','P','G'),10, Size(frame_width,frame_height),true);

   while(state != 0){

       cap >> frame;
       video.write(frame);
       //podglad z kamery w czasie rzeczywistym
       imshow( "Frame", frame );
       //test zapisu ostatniej klatki
       imwrite("frame.png", frame);
       frames_vector.push_back(frame);
      //tego trzeba bd sie pozbyc
       char c = (char)waitKey(33);
       if( c == 27 ) 
              break;
       
   }
   //po nagraniu realease
   video.release();
   cap.release();

}
//metoda zmienia zmienna decydujaca o stanie nagrywnia na 0
//koniec nagrywania
void Record :: stop(){
  //test watkow i metody stop
  std::this_thread::sleep_for(2s);
  state=0;
}

 void convert(vector<Mat> frames){

  for(int i=0;i<frames.size();i++){
  Mat frame=frames[i];
  cv::Size sz = frame.size();
  int imageWidth = sz.width;
  int imageHeight = sz.height;
	Mat resized;
	resize(frame, resized, cv::Size(100, 100));
  resize(resized, frame, cv::Size(sz.width, sz.height));
  frames[i]=frame;
  }
 
}
// rate sugeruja co kotra klatke wycinamy
void  extract_frames(const string &videoFilePath,vector<Mat>& frames,const int rate){
	
	try{
		//otworz swtorozny wczesniej plik video
  	VideoCapture cap(videoFilePath); //otworz
  	if(!cap.isOpened())  // sprawdz czy sie powiodlo
  		CV_Error(CV_StsError, "błąd przy otwieraniu pliku video");
	
  	//cap.get(CV_CAP_PROP_FRAME_COUNT) -> zawiera liczbe klatek
  	for(int frameNum = 0; frameNum < cap.get(CV_CAP_PROP_FRAME_COUNT);frameNum+=rate)
  	{
  		Mat frame;
  		cap >> frame; // nastepna klatka
  		frames.push_back(frame);
  	}
  }
  catch( cv::Exception& e ){
    cerr << e.msg << endl;
    exit(1);
  }
	
}

int main(){
  VideoCapture cap(0);
  Record *record= new Record();
  Record rec;
  //rzucanie wątków/ lambda
  //póki co spr czy metoda stop działa
  std::cout<<rec.connection(cap);
  std::thread thread1 = record->member1Thread(cap);
  std::thread thread2 = record->member2Thread();
  
  thread1.join();
  thread2.join();

  //zapisz klatki do vektora zdjec
  vector<Mat> frames,framesConv;
  extract_frames("out.avi",frames,10);
  
  //zapis zdjec do wektora zdjec
  extract_frames("out.avi",framesConv,10);
  //konwersja vektroa zdjec do gorszej jakosci
  convert(framesConv);
  //przykladowy zapis pary zdjec
  imwrite("koronawirusLowRes.jpg",framesConv[0]);
  imwrite("koronawirus.jpg",frames[0]);

  return 0;
}
