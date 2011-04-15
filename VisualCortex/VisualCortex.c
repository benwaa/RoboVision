/*
The Visual Cortex of guarddog
URLs: http://ammarkov.ath.cx
Written by Ammar Qammaz a.k.a. AmmarkoV 2010

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "VisualCortex.h"
#include "VisionMemory.h"
#include "DisparityDepthMap.h"
#include "MovementRegistration.h"
#include "VisCortexFilters.h"
#include "FeatureExtraction.h"
#include "PatchTracking.h"
#include "FaceDetection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * VISCORTEX_VER = "0.477";


char *  VisCortx_Version()
{
	return VISCORTEX_VER;
}

/*
 ----------------- INITIALIZATION ----------------------
*/

unsigned int VisCortx_SetCamerasGeometry(float distance_between_cameras,float diagonal_field_of_view,float horizontal_field_of_view,float vertical_field_of_view)
{
  if ( ( metrics[RESOLUTION_X]==0 ) || ( metrics[RESOLUTION_Y]==0 ) ) { fprintf(stderr,"Resolution not set.. VisCortx_Start needs to be called before VisCortx_SetCamerasGeometry\n");
                                                                        return 0;
                                                                      }

  /* Cameras should be parallel.. */
  camera_distance = distance_between_cameras;
  camera_diagonal_field_of_view = diagonal_field_of_view;
  camera_horizontal_field_of_view = horizontal_field_of_view;
  camera_vertical_field_of_view = vertical_field_of_view;
  if ( (camera_diagonal_field_of_view!=0.0) && (camera_horizontal_field_of_view==0.0) && (camera_vertical_field_of_view==0.0) )
    {
      fprintf(stderr,"We need to calculate horizontal and vertical field of view from diagonal , this ( as someone could expect ) is not very precise \n");
      float abs_diagonal_resolution = sqrt (metrics[RESOLUTION_X]*metrics[RESOLUTION_X] + metrics[RESOLUTION_Y]*metrics[RESOLUTION_Y]);
      if ( abs_diagonal_resolution == 0 ) { fprintf(stderr,"Could not find horizontal/vertical field of view..\nVisCortx_SetCamerasGeometry will have to be run again\n"); }
      camera_horizontal_field_of_view = ( metrics[RESOLUTION_X] / abs_diagonal_resolution ) * diagonal_field_of_view;
      camera_vertical_field_of_view = ( metrics[RESOLUTION_Y] / abs_diagonal_resolution ) * diagonal_field_of_view;
    }


  fprintf(stderr,"Camera Geometry set to : \n");
  fprintf(stderr,"Distance between cameras : %f \n",camera_distance);
  fprintf(stderr,"Field of view ( diagonal : %f , horizontal : %f , vertical : %f)\n",camera_diagonal_field_of_view,camera_horizontal_field_of_view,camera_vertical_field_of_view);

  return 1;
}



unsigned int VisCortx_Start(unsigned int res_x,unsigned int res_y)
{

   fprintf(stderr,"Visual Cortex , Things todo memo : \n");
   fprintf(stderr,"Check if precalculations are correct\n");
   fprintf(stderr,"Improve filters using a single real pointer\n");
   fprintf(stderr,"Fix Feature tracking!\n");
   fprintf(stderr,"\n");
   fprintf(stderr,"---------------------------------------\n");

   //unsigned int MAX_INT=-1;
   //fprintf(stderr,"MAX_Integer is %u",MAX_INT);

       //e(mach) calculation
    float e1=1.0;
    while (1+e1 >1)
     {
        e1 = e1/2;
     }

    double e2=1.0;
    while (1+e2 >1)
     {
        e2 = e2/2;
     }

    fprintf(stderr,"Machine numerical error , single : %e , double : %e \n",e1,e2);

   InitFaceRecognition(res_x,res_y);
   return InitVisionMemory(res_x,res_y);
}

unsigned int VisCortx_Stop()
{
   CloseFaceRecognition();
   return  CloseVisionMemory();
}

unsigned int VisCortx_SelfCheck()
{
    fprintf(stderr,"VisCortx_SelfCheck starting!\n");
    fprintf(stderr,"GenerateCompressHistogramOfImage Beeing called!\n");
    GenerateCompressHistogramOfImage(video_register[LEFT_EYE].pixels,l_video_register[HISTOGRAM_COMPRESSED_LEFT].pixels,30,30);
    fprintf(stderr,"GenerateCompressHistogramOfImage is done!\n");
    int x=0,y=0;


    for (y=0; y<210; y++)
    {
     for (x=0; x<290; x++)
     {
      fprintf(stderr,"X/Y %u / %u ",x,y);
      struct Histogram hist1={0},hist1_old={0};
      HistogramPatch(&hist1_old,video_register[LEFT_EYE].pixels,x,y,metrics[HORIZONTAL_BUFFER],metrics[VERTICAL_BUFFER]);
      CompressedHistogramPatch(l_video_register[HISTOGRAM_COMPRESSED_LEFT].pixels,&hist1,x,y);

     if (
        (hist1.median_r!=hist1_old.median_r)||
        (hist1.median_g!=hist1_old.median_g)||
        (hist1.median_b!=hist1_old.median_b)
        )
    {
      fprintf(stderr," Miss NEW %u %u %u / OLD %u %u %u  \n",hist1.median_r,hist1.median_g,hist1.median_b,hist1_old.median_r,hist1_old.median_g,hist1_old.median_b);
     } else
     fprintf(stderr,"Success\n ");
    }
   }
   fprintf(stderr,"VisCortx_SelfCheck is done!\n");
   return 0;
}

void VisCortx_SetSetting(unsigned int set_num,unsigned int set_val)
{
    if (set_num>=SETTINGS_COUNT) {return;}
    settings[set_num]=set_val;
}

void VisCortx_SetDefaultSettings()
{
   DefaultSettings();
}

unsigned int VisCortx_GetSetting(unsigned int get_num)
{
    if (get_num>=SETTINGS_COUNT) {return 0;}
    return settings[get_num];
}



unsigned int VisCortx_GetMetric(unsigned int get_num)
{
    if (get_num>=METRICS_COUNT) {return 0;}
    return metrics[get_num];
}


unsigned int VisCortx_GetVideoRegisterStats(unsigned int metric_num)
{
   if (metric_num>=METRICS_COUNT) { fprintf(stderr,"Metric does not exist! \n "); return 0; }

   return metrics[metric_num];

}
/*
 ----------------- INITIALIZATION ----------------------
*/



/*
 ----------------- VIDEO INPUT/OUTPUT ----------------------
*/


void VisCortX_CopyFromVideoToVideoRegister(unsigned int input_img_regnum,unsigned int output_img_regnum)
{
  unsigned long syst_mem_end=metrics[RESOLUTION_X]*metrics[RESOLUTION_Y]*3;
  memcpy(video_register[output_img_regnum].pixels,video_register[input_img_regnum].pixels,syst_mem_end);
  return;
}

void VisCortX_BitBltVideoRegister(unsigned int input_img_regnum,unsigned int output_img_regnum,unsigned int px,unsigned int py,unsigned int tx,unsigned int ty,unsigned int size_x,unsigned int size_y)
{
 CopyPartOfImageToImage(video_register[input_img_regnum].pixels,video_register[output_img_regnum].pixels,px,py,tx,ty,size_x,size_y);
}

unsigned int VisCortX_LoadVideoRegisterFromFile(unsigned int reg_num,char * filename)
{
   return LoadRegisterFromFile(filename,reg_num);
}

unsigned int VisCortX_SaveVideoRegisterToFile(unsigned int reg_num,char * filename)
{
   return SaveRegisterToFile(filename,reg_num);
}

unsigned int VisCortx_WriteToVideoRegister(unsigned int reg_num,unsigned int size_x,unsigned int size_y,unsigned int depth,unsigned char * rgbdata)
{
	if (  VideoRegisterRequestIsOk(reg_num,size_x,size_y,depth)!=0 ) { return 1; }

    unsigned long syst_mem_end=metrics[RESOLUTION_X]*metrics[RESOLUTION_Y]*3;
    unsigned long pic_mem_end=size_x*size_y*depth;

    unsigned long mem_end=syst_mem_end;
    if ( pic_mem_end > mem_end ) { fprintf(stderr,"Register is not big enough to accomodate data! \n "); return 1; } else
    if ( pic_mem_end < mem_end ) { mem_end = pic_mem_end; }


    memcpy(video_register[reg_num].pixels,rgbdata,mem_end);

	return 0;
}

unsigned char * VisCortx_ReadFromVideoRegister(unsigned int reg_num,unsigned int size_x,unsigned int size_y,unsigned int depth)
{
   if (  VideoRegisterRequestIsOk(reg_num,size_x,size_y,depth)!=0 ) { return 0; }

   if ( (size_x!=video_register[reg_num].size_x) ||
        (size_y!=video_register[reg_num].size_y) ||
        (depth !=video_register[reg_num].depth)
      )
      { fprintf(stderr,"Wrong Register Request Size :S \n ");
        fprintf(stderr,"Register ( %ux%u:%u ) , Request ( %ux%u:%u )\n ",size_x,size_y,depth,video_register[reg_num].size_x,video_register[reg_num].size_y,video_register[reg_num].depth);

        return 0; }


   return video_register[reg_num].pixels;
}


/*
 ----------------- VIDEO INPUT/OUTPUT ----------------------
*/


/*
 ----------------- DEPTH MAPPING FUNCTIONS ----------------------
*/
void  VisCortx_FullDepthMap()
{
  unsigned int edgepercent=settings[PATCH_COMPARISON_EDGES_PERCENT_REQUIRED],patch_x=metrics[HORIZONTAL_BUFFER],patch_y=metrics[VERTICAL_BUFFER];
  unsigned int threshold=settings[DEPTHMAP_COMPARISON_THRESHOLD];
  unsigned int threshold_added=settings[DEPTHMAP_COMPARISON_THRESHOLD_ADDED];

  if ( threshold != 0 )
   {
    settings[DEPTHMAP_COMPARISON_THRESHOLD_LARGE_PATCH]=  (unsigned int) ( (threshold * metrics[VERTICAL_BUFFER_LARGE] * metrics[HORIZONTAL_BUFFER_LARGE] ) / (metrics[HORIZONTAL_BUFFER]*metrics[VERTICAL_BUFFER]) ); //16000;
    settings[DEPTHMAP_COMPARISON_THRESHOLD_EXTRALARGE_PATCH]=  (unsigned int) ( (threshold * metrics[VERTICAL_BUFFER_EXTRALARGE] * metrics[HORIZONTAL_BUFFER_EXTRALARGE] ) / (metrics[HORIZONTAL_BUFFER]*metrics[VERTICAL_BUFFER]) ); //16000;
   }
  /*
     WE COMPARE PATCHES ON 3 DIFFERENT LEVELS , EXTRA LARGE PATCHES , LARGE PATCHES , NORMAL PATCHES
   */

  /*
    CALCULATION OF EXTRA LARGE PATCHES FOLLOWS
   */

  settings[DEPTHMAP_COMPARISON_THRESHOLD]=settings[DEPTHMAP_COMPARISON_THRESHOLD_EXTRALARGE_PATCH];
  settings[PATCH_COMPARISON_EDGES_PERCENT_REQUIRED]=settings[PATCH_COMPARISON_EDGES_PERCENT_REQUIRED_EXTRALARGE_PATCH];
  metrics[VERTICAL_BUFFER]=metrics[VERTICAL_BUFFER_EXTRALARGE];
  metrics[HORIZONTAL_BUFFER]=metrics[HORIZONTAL_BUFFER_EXTRALARGE];

  DepthMapFull(
                video_register[LEFT_EYE].pixels ,
                video_register[RIGHT_EYE].pixels ,
                l_video_register[DEPTH_LEFT].pixels ,
                l_video_register[DEPTH_RIGHT].pixels ,
                metrics[RESOLUTION_X],
                metrics[RESOLUTION_Y],
                1
             );

  /*
    CALCULATION OF LARGE PATCHES FOLLOWS
   */

if ( settings[PATCH_COMPARISON_LEVELS] >= 2 )
{
  settings[DEPTHMAP_COMPARISON_THRESHOLD]=settings[DEPTHMAP_COMPARISON_THRESHOLD_LARGE_PATCH];
  settings[PATCH_COMPARISON_EDGES_PERCENT_REQUIRED]=settings[PATCH_COMPARISON_EDGES_PERCENT_REQUIRED_LARGE_PATCH];
  metrics[VERTICAL_BUFFER]=metrics[VERTICAL_BUFFER_LARGE];
  metrics[HORIZONTAL_BUFFER]=metrics[HORIZONTAL_BUFFER_LARGE];

  DepthMapFull(
                video_register[LEFT_EYE].pixels ,
                video_register[RIGHT_EYE].pixels ,
                l_video_register[DEPTH_LEFT].pixels ,
                l_video_register[DEPTH_RIGHT].pixels ,
                metrics[RESOLUTION_X],
                metrics[RESOLUTION_Y],
                0
             );


}


  /*
    CALCULATION OF NORMAL PATCHES FOLLOWS
   */
   settings[DEPTHMAP_COMPARISON_THRESHOLD_ADDED]=threshold_added;
   settings[DEPTHMAP_COMPARISON_THRESHOLD]=threshold;
   settings[PATCH_COMPARISON_EDGES_PERCENT_REQUIRED]=edgepercent;
   metrics[VERTICAL_BUFFER]=patch_y;
   metrics[HORIZONTAL_BUFFER]=patch_x;
   /* THESE 3 LINES ARE DELIBERATELY OUT OF THE IF CONTROL BECAUSE THESE VALUES ARE DEFAULT*/
if ( settings[PATCH_COMPARISON_LEVELS] >= 3 )
{
  DepthMapFull(
                video_register[LEFT_EYE].pixels ,
                video_register[RIGHT_EYE].pixels ,
                l_video_register[DEPTH_LEFT].pixels ,
                l_video_register[DEPTH_RIGHT].pixels ,
                metrics[RESOLUTION_X],
                metrics[RESOLUTION_Y],
                0
             );
}
  /*
    CONVERTING DEPTH DATA TO RGB VIDEO FORMAT ( FOR USER VIEWING )
   */
  DepthMapToVideo(
                  l_video_register[DEPTH_LEFT].pixels ,
                  video_register[DEPTH_LEFT_VIDEO].pixels ,
                  video_register[DEPTH_LEFT_VIDEO].size_x ,
                  video_register[DEPTH_LEFT_VIDEO].size_y
                 );

}

unsigned int  VisCortx_Get_DepthMapData(unsigned int typeofdata,unsigned int px,unsigned int py)
{
  //if ((px>=settings[RESOLUTION_X])||(py>=settings[RESOLUTION_Y])) { return 0; }
  unsigned int mem_place=( px + py * metrics[RESOLUTION_X] );

  unsigned int retval = 0;
     switch ( typeofdata )
     {
     case 1 : retval= (unsigned int) l_video_register[DEPTH_LEFT].pixels[mem_place]; break;
	 case 2 : retval= (unsigned int) depth_data_array[mem_place].score; break;
	 case 3 : retval= (unsigned int) depth_data_array[mem_place].edge_count; break;
	 case 4 : retval= (unsigned int) depth_data_array[mem_place].x1_patch; break;
	 case 5 : retval= (unsigned int) depth_data_array[mem_place].y1_patch; break;
	 case 6 : retval= (unsigned int) depth_data_array[mem_place].x2_patch; break;
	 case 7 : retval= (unsigned int) depth_data_array[mem_place].y2_patch; break;
	 case 8 : retval= (unsigned int) depth_data_array[mem_place].patch_size_x; break;
	 case 9 : retval= (unsigned int) depth_data_array[mem_place].patch_size_y;  break;
     case 10 : retval= metrics[VERTICAL_BUFFER]; break;
	 case 11 : retval= metrics[HORIZONTAL_BUFFER]; break;
     case 12 : retval= mem_place; break;
     };
  return retval;
}
/*
 ----------------- DEPTH MAPPING FUNCTIONS ----------------------
*/


/*
 ----------------- MOVEMENT REGISTRATION ----------------------
*/
int VisCortx_Movement_Detection(unsigned int left_cam,unsigned int right_cam)
{
   if ( left_cam == 1 )  metrics[CHANGES_LEFT]=RegisterMovements(1,
                                                                 video_register[LAST_LEFT_EYE].pixels,
                                                                 video_register[LEFT_EYE].pixels,
                                                                 video_register[BACKGROUND_LEFT].pixels,
                                                                 video_register[DIFFERENCE_LEFT].pixels,
                                                                 video_register[MOVEMENT_LEFT].pixels
                                                                );


   if (right_cam == 1 ) metrics[CHANGES_RIGHT]=RegisterMovements(0,
                                                                 video_register[LAST_RIGHT_EYE].pixels,
                                                                 video_register[RIGHT_EYE].pixels,
                                                                 video_register[BACKGROUND_RIGHT].pixels,
                                                                 video_register[DIFFERENCE_RIGHT].pixels,
                                                                 video_register[MOVEMENT_RIGHT].pixels
                                                                );
  return 0;
}
/*
 ----------------- MOVEMENT REGISTRATION ----------------------
*/

void VisCortx_SetTime(unsigned int thetime)
{
    if ( thetime<TIME_INC) { fprintf(stderr,"VisCortex Clock truncated"); }
	TIME_INC=thetime;
}

unsigned int VisCortx_GetTime()
{
    return TIME_INC;
}

/*
 ----------------- FEATURE TRACKING ----------------------
*/
unsigned int  VisCortx_GetTrackedPoints()
{
  return GetPointTrackList();
}

void  VisCortx_AddTrackPoint(unsigned int cam,unsigned int x,unsigned int y,unsigned int group)
{
  fprintf(stderr,"VisCortx_AddTrackPoint %u %u,%u : %u\n",cam,x,y,group);
  AddPointToTrackList(cam,x-1,y-1,group);
}

void  VisCortx_AutoAddTrackPoints(unsigned int cam)
{
 if (cam==0)
  {
      PrepareCleanSobeledGaussian(video_register[LEFT_EYE].pixels,video_register[EDGES_LEFT].pixels,settings[DEPTHMAP_EDGE_STRICTNESS]);
      ClearVideoRegister(GENERAL_1);
      ExtractFeatures(video_register[EDGES_LEFT].pixels,video_register[GENERAL_1].pixels,metrics[RESOLUTION_X],metrics[RESOLUTION_Y],0);
  }
  //fprintf(stderr,"VisCortx_AutoAddingTrackPoint %u %u,%u : %u\n",cam,x,y,group);
 //
 fprintf(stderr,"VisCortx_AutoAddTrackPoints ok\n");
 return;
}


void  VisCortx_RemoveTimedoutTrackPoints(unsigned int timeout)
{
   RemoveTrackPointIfTimedOut(timeout);
}

unsigned int  VisCortx_GetTrackPoint(unsigned int dat,unsigned int trackpoint)
{

  char cam;
  unsigned int x,y;
  if (dat<4) { GetTrackPoint(trackpoint,1,&cam,&x,&y); } else
             { GetTrackPoint(trackpoint,2,&cam,&x,&y); }
  switch (dat)
  {
   case 1: { return x; break; }
   case 2: { return y; break; }
   case 3: { return cam; break; }
   case 4: { return x; break; }
   case 5: { return y; break; }
   case 6: { return cam; break; }
   case 7: { return GetTrackData(trackpoint,1); break; }
   case 8: { return GetTrackData(trackpoint,2); break; }
   case 9: { return GetTrackData(trackpoint,3); break; }
   default : return 0;
  };
 return 0;
}


void  VisCortx_TrackPoints()
{
  unsigned int i=0;
  for (i=0; i<GetPointTrackList(); i++)
   {
	   ExecuteTrackPoint(i);
   }
}

void  VisCortx_DrawTrackPoints()
{ fprintf(stderr,"Draw Track Points not implemented ok\n");

  unsigned int i=0;
  for (i=0; i<GetPointTrackList(); i++)
   {
	 BitBltTrackPointMemToRegister(i,LAST_LEFT_OPERATION,i*30,0);
   }
}

void  VisCortx_RenewTrackPoint(unsigned int tpoint)
{
  ApplyTrackPositionAsOriginal(tpoint);
}


void  VisCortx_RenewAllTrackPoints()
{
  if ( GetPointTrackList() == 0 ) { return; }

  fprintf(stderr,"TODO : Add different levels of gaussian blur to track points! :P\n");
  unsigned int i=0;
  for (i=0; i<GetPointTrackList(); i++)
   {
	   ApplyTrackPositionAsOriginal(i);
   }
}
/*
 ----------------- FEATURE TRACKING ----------------------
*/

/*
 ----------------- IMAGE PROCESSING ----------------------
*/

void KeepOnlyPixelsClosetoColor(unsigned char R,unsigned char G,unsigned char B,unsigned char howclose)
{
 //   VisCortX_BitBltVideoRegister(LEFT_EYE,GENERAL_1,0,0,0,0,metrics[RESOLUTION_X],metrics[RESOLUTION_Y]);
    memcpy(video_register[GENERAL_1].pixels,video_register[LEFT_EYE].pixels,metrics[RESOLUTION_X]*metrics[RESOLUTION_Y]*3*sizeof(BYTE));
    KillDifferentPixels(video_register[GENERAL_1].pixels ,metrics[RESOLUTION_X],metrics[RESOLUTION_Y],R,G,B,howclose);
}

int SobelNDerivative(int n)
{
    return SobelNDegreeDerivative(n,video_register[LEFT_EYE].pixels,video_register[LAST_LEFT_OPERATION].pixels,metrics[RESOLUTION_X],metrics[RESOLUTION_Y]);
}

/*
 ----------------- FACE DETECTION ----------------------
*/
unsigned int VisCortx_RecognizeFaces(unsigned int cam)
{
   if ( cam == 0 ) { return RecognizeFaces(video_register[LEFT_EYE].pixels); } else
   if ( cam == 1 ) { return RecognizeFaces(video_register[RIGHT_EYE].pixels); }
   return 0;
}

void VisCortx_GetFaceNumber(char num,unsigned int *pos_x,unsigned int *pos_y,unsigned int *total_size)
{
    GetFaceNumber(num,pos_x,pos_y,total_size);
}

/*
 ----------------- LIDAR DEPTH DATA EMULATION ----------------------
*/

float VisCortx_MinCameraHorizontalAngle()
{
  return -camera_horizontal_field_of_view/2;
}

float VisCortx_MaxCameraHorizontalAngle()
{
  return camera_horizontal_field_of_view/2;
}

float VisCortx_CameraHorizontalAngleStep()
{
  if ( camera_horizontal_field_of_view == 0 ) { return 0; }
  return  metrics[RESOLUTION_X] / camera_horizontal_field_of_view;
}


unsigned short VisCortx_GetDepth_From_Angle(char num,float horizontal_angle,float vertical_angle)
{
 /* horizontal_angle ( left is less , right is more )
    vertical angle ( down is less , up is more )

     When X is 0 horizontal -> angle -field_of_view
     When X is RES_X/2 -> horizontal angle 0
     When X is RES_X -> horizontal angle +field_of_view
  */
  float degree_step_x = 0 , degree_step_y = 0;
  if (camera_horizontal_field_of_view !=0) { degree_step_x = metrics[RESOLUTION_X] / camera_horizontal_field_of_view; } else
                                           { fprintf(stderr,"Camera horizontal field of view NOT set .. , this will probably keep popping up , until you call VisCortx_SetCamerasGeometry\n"); return 0; }
  if ( degree_step_x == 0 ) { fprintf(stderr,"Camera resolution probably not set , cannot continue to get depth \n"); return 0; }


  if (camera_vertical_field_of_view !=0) { degree_step_y = metrics[RESOLUTION_Y] / camera_vertical_field_of_view; } else
                                         { fprintf(stderr,"Camera vertical field of view NOT set .. , this will probably keep popping up , until you call VisCortx_SetCamerasGeometry\n"); return 0; }
  if ( degree_step_y == 0 ) { fprintf(stderr,"Camera resolution probably not set , cannot continue to get depth \n"); return 0; }

  unsigned int uint_pixel_x=metrics[RESOLUTION_X] / 2 , uint_pixel_y=metrics[RESOLUTION_Y] / 2 , ptr = 0;

  float abs_horizontal_angle = horizontal_angle; if ( abs_horizontal_angle < 0 ) { abs_horizontal_angle = abs_horizontal_angle  * (-1); }
  float abs_vertical_angle = vertical_angle;     if ( abs_vertical_angle < 0 ) { abs_vertical_angle = abs_vertical_angle  * (-1); }

  float pixel_x = abs_horizontal_angle / degree_step_x  , pixel_y =abs_vertical_angle / degree_step_y;
  if ( horizontal_angle < 0 ) { pixel_x  = (metrics[RESOLUTION_X] / 2) - abs_horizontal_angle; }
  if ( vertical_angle < 0   ) { pixel_y  = (metrics[RESOLUTION_Y] / 2) - abs_vertical_angle;   }

  if ( ( pixel_x >= 0.0 ) && (pixel_x < metrics[RESOLUTION_X] ) ) { uint_pixel_x = (unsigned int) pixel_x; }
  if ( ( pixel_y >= 0.0 ) && (pixel_y < metrics[RESOLUTION_Y] ) ) { uint_pixel_y = (unsigned int) pixel_y; }

  unsigned int Camera_Selected = DEPTH_LEFT;
  if ( num == 1 ) { Camera_Selected = DEPTH_RIGHT; }

  ptr = uint_pixel_y * metrics[RESOLUTION_X] + uint_pixel_x;

  if ( metrics[RESOLUTION_MEMORY_LIMIT_1BYTE] <= ptr ) { return 0; }

  return l_video_register[Camera_Selected].pixels[ptr];
}

unsigned short VisCortx_SetDepthScale(unsigned short depth_units,float centimeters)
{
  if ( depth_units<255 )
     {
       depth_units_in_cm[depth_units]=centimeters;
     }
  return 0;
}

float VisCortx_DepthUnitsToCM(unsigned short depth_units)
{
  if (depth_units<255)
   {
       return depth_units_in_cm[depth_units];
   }
  return 0.0;
}
