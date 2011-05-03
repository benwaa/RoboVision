#include "visual_system.h"
#include "configuration.h"
#include <stdio.h>
#include <stdlib.h>
#include "webinterface.h"

// TO SET VIDEO MODE V4L2_PIX_FMT_RGB24
#include <linux/videodev2.h>

int width=320,height=240;
int has_init = 0;

int wait_for_cameras_to_init()
{
        int waittime=0;
        while ( !FeedReceiveLoopAlive(0) ) { printf(".1."); ++waittime; }
        waittime=0;
        while ( !FeedReceiveLoopAlive(1) ) { printf(".2."); ++waittime; }
        fprintf(stderr,"Video loops running!\n");
   return 1;
}

int InitVisualSystem()
{

    LoadConfiguration(); // Pull Devices :P

    InitVideoInputs(2);
    int camerasok=0;

    struct VideoFeedSettings feedsettings={0};
    feedsettings.PixelFormat=V4L2_PIX_FMT_RGB24;


    camerasok+=InitVideoFeed(0,video_device_1,width,height,24,1,feedsettings);
    camerasok+=InitVideoFeed(1,video_device_2,width,height,24,1,feedsettings);
    if ( (camerasok==2) )
      {
          if ( !wait_for_cameras_to_init() ) { fprintf(stderr,"CamerasTimed out!!\n"); return 0; }
      } else
      {
       fprintf(stderr,"Failed to init Video Feeds!\n");
       return 0;
      }


    VisCortx_Start(width,height);
    VisCortx_SetCamerasGeometry(6.0,72.0,0.0,0.0);

    OpenWebInterface();

    has_init=1;
    return 1;
}

int PassVideoInputToCortex(unsigned int clock_time)
{
 void *frame1=0 ,*frame2 = 0;


 // GET INPUT  ( SWAPED OR NOT! )
 if ( swap_inputs == 0 )
 {
    frame1=GetFrame(0); frame2=GetFrame(1);
 } else
 {
    frame1=GetFrame(1); frame2=GetFrame(0);
 }

  if ( ( !NewFrameAvailiable(0) ) || ( !NewFrameAvailiable(1) ) )
   {
     /*No New frames at all , skipping */
     return 0;
   }

  if (( frame1 == 0 ) || ( frame2 == 0 ))
   {
     /*Video Input not returning frames , kernel skipping frame */
     //fprintf(stderr,"Video Input not returning frames , kernel skipping frame \n");
     return 0;
   }

  if ( NewFrameAvailiable(0) )
   {
       VisCortX_CopyVideoRegister(LEFT_EYE,LAST_LEFT_EYE);
       VisCortx_WriteToVideoRegister(LEFT_EYE,width,height,3,(unsigned char *)frame1);
   }

  if ( NewFrameAvailiable(1) )
  {
      VisCortX_CopyVideoRegister(RIGHT_EYE,LAST_RIGHT_EYE);
      VisCortx_WriteToVideoRegister(RIGHT_EYE,width,height,3,(unsigned char *)frame2);
  }

 /*
  * Frames should be signaled processed AFTER they have been passed to Visual Cortex :P
  */
 SignalFrameProcessed(0);
 SignalFrameProcessed(1);


 VisCortx_SetTime(clock_time);
 return 1;
}


int CloseVisualSystem()
{
    CloseWebInterface();

    CloseVideoInputs();
    VisCortx_Stop();
  return 1;
}
