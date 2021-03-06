#ifndef CONFIGURATION_H_INCLUDED
#define CONFIGURATION_H_INCLUDED

#define MAX_STR 256

extern int system_autonomous;


extern char video_device_1[MAX_STR];
extern char video_device_2[MAX_STR];
extern char joystick_device[MAX_STR];
extern char rd01_device[MAX_STR];
extern char arduino_device[MAX_STR];
extern char user[MAX_STR];
extern char group[MAX_STR];
extern char parentdir[MAX_STR];
extern double camera_params_1[12];
extern double camera_params_2[12];

extern unsigned int IRC_Interface_Enabled;

extern char sound_play_command[MAX_STR];

extern char sound_record_command[MAX_STR];
extern char sound_record_parameter[MAX_STR];


extern char tts_app[MAX_STR];
extern char tts_pre_command[MAX_STR];
extern char tts_command[MAX_STR];
extern char tts_command_parameter[MAX_STR];

extern unsigned int fps;
extern unsigned int draw_on;
extern unsigned int web_interface;
extern unsigned int web_interface_snaptime;
extern unsigned int motion_lock_on;
extern unsigned int swap_inputs;

int filename_stripper_found_attack(char * filename);
void LoadConfiguration();
int RefreshDeviceNumbering();

#endif // CONFIGURATION_H_INCLUDED
