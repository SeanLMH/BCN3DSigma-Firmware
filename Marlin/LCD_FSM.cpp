/*
- LCD_FSM.cpp - A class that manages the FSM related with some printer process
Last Update: 02/01/2019
Author: Alejandro Garcia (S3mt0x)
*/
#include "LCD_FSM.h"
#include "Build_defs.h"

#ifdef SIGMA_TOUCH_SCREEN

#ifndef LCD_HANDLER_H_
#define LCD_HANDLER_H_

#include "genieArduino.h"
#include "Configuration.h"
#include "Touch_Screen_Definitions.h"
#include "Marlin.h"
#include "stepper.h"
#include "temperature.h"
#include "SD_ListFiles.h"
#include "LCD_Handler.h"
#include "ConfigurationStore.h"
#include "BCN3D_customregisters.h"
#include "register_codes.h"
#include "StatusPrintBCN3D.h"

/*
*
*

Global variables definitions

*
*
*/

#ifdef DEV_BER
bool print_feedback_fail_flag = false;
#endif

//Temperatures animation gif flags:
bool flag_temp_gifhotent0 = false;
bool flag_temp_gifhotent1 = false;
bool flag_temp_gifbed = false;

// Register flags:
uint16_t flag_sdprinting_register = 0;
uint8_t flag_utilities_filament_register = 0;
uint8_t flag_utilities_maintenance_register = 0;
uint16_t flag_sdlist_resgiter = 0x01;
uint8_t flag_utilities_calibration_register = 0;
#ifdef DEV_BER
uint8_t flag_serialprint_register = 0;
#endif

// LCD processing flag:
bool lcd_busy = false;

// raft detection flag:
int raft_advise_accept_cancel = -1;	// 0 cancel ; 1 accept ; -1 none

int Temp_ChangeFilament_Saved = 0;

// Change Filament control temperatures:
int Tref1 = 0; // Init temperature
int Tfinal1 = 0; // Target temperature
int Tpercentage_old = 0; // Temperature Percentage

int RegID = 0; // Register input code
int RegID_digit_count = 0; // Register digit count up to 4 digits

float offset_calib_manu[4] = {0.0,0.0,0.0,0.0};	// Manual Fine Calib axis offset to be modified
unsigned int calib_value_selected;// Manual Fine Calib axis id

int8_t calib_zxy_id = 0; // Calib global variable id, used in redo process: 1 X , 2 Y , ZL 3, ZR 4

int calib_confirmation = 1888;// Calib line selected id: for ZL and ZR(if line gap is 0.05): -3(-0.15) to 1(0.05); for X and Y: -5(-0.5) to 4(0.4);

// Load Custom Filament temperatures:
int custom_load_temp = 220;
int custom_unload_temp = 210;
int custom_print_temp = 210;
int custom_bed_temp = 60;

int cycle_filament = 0; // Filament list number: 0 PLA 1 PVA 2 PET-G 3 ABS 4 Nylon 5 TPU 6 Custom


long LCD_FSM_input_buton_flag = -1; // Button Pressed id

int redo_source; // Redo global variable id, used in redo process: 1 X , 2 Y , ZL 3, ZR 4


/*
*
*

Functions definitions

*
*
*/
#ifdef DEV_BER

void copy_message(uint8_t id);

#endif

void setfilenames(int jint); // Get filename
void setfoldernames(int jint); // Get foldername

void insertmetod(); // Init Load filament

void ListFilesParsingProcedure(uint16_t vecto, int jint); // SD list get info from file selected
void ListFilesUpfunc(); // SD list scroll up
void ListFilesDownfunc(); // SD list Open down
void ListFileListINITSD(); // SD list go/back to init
void ListFileListENTERBACKFORLDERSD(); // SD list back
void ListFileSelect_open(); // SD list Open selected

// SD list Select first to fifth:
void ListFileSelect0();
void ListFileSelect1();
void ListFileSelect2();
void ListFileSelect3();
void ListFileSelect4();

void setsdlisthours(int jint, int time_hour); // Set hours from gcode info
void setsdlistmin(int jint, int time_min); // Set min from gcode info
void setsdlistg(int jint, int weight); // Set weight from gcode info

void lcd_animation_handler(); // Animation handler
void update_screen_printing(); // Printing State handler
void update_screen_sdcard(); // SD State handler
void update_screen_noprinting(); // No printing State handler
void update_screen_endinggcode(); // Ending print State handler
#ifdef DEV_BER
void update_screen_serial_printing(); // Serial Printing State handler
#endif


void folder_navigation_register(bool upchdir); // Folder depth count

inline void Calib_check_temps(void); // Calib check temperatures and wait until is reached

inline void Z_compensation_decisor(void); // Decide it shims is needed
void go_loadfilament(uint8_t idbutton); // load filament by id
void go_loadfilament_next(void); // filament list next page
void go_loadfilament_back(void); // filament list back page

#if PATTERN_Z_CALIB == 0 // 0 default calib procedure
inline void Full_calibration_ZL_set(float offset); // Set ZL offset
inline void Full_calibration_ZR_set(float offset); // Set ZR offset
#else // 1 auto calib
void Full_calibration_ZL_set(float offset); // Set ZL offset
void Full_calibration_ZR_set(float offset); // Set ZR offset
#endif
void inline Full_calibration_X_set(float offset); // Set X offset
void inline Full_calibration_Y_set(float offset); // Set Y offset

void turnoff_buttons_xycalib(void); // Set X & Y calib select line buttons to default state
void turnoff_buttons_zcalib(void); // Set ZL & ZR calib select line buttons to default state

void unload_get_ready(void); // Go to unload filament position
inline void unloadfilament_procedure(void); // Proceed to unload filament

void show_data_printconfig(void); // Display printer config, function compatible with different screens

// Bed compensation:
inline void Bed_Compensation_Set_Lines(int jint); // set due selected line
void Bed_Compensation_Redo_Lines(int jint); // redo from selected best line
int Bed_compensation_redo_offset = 0; // offset accumulated
int8_t Bed_Compensation_Lines_Selected[3] = {0,0,0}; // offset measured vector
uint8_t Bed_Compensation_state = 0; // state 0: First Bed Calib, state 1: ZL Calib, state 2: Bed Compensation Back, state 3: Bed Compensation Front Left, state 4: Bed Compensation Front Right

int get_nummaxchars(bool isfilegcode, unsigned int totalpixels); // get number of chars available


void setregiter_num(int n); // introduce register digit
bool check_regiter_num(unsigned int n); // Verify if code is genuine

void lcd_fsm_lcd_input_logic(); // do a task according to input button pressed
void lcd_fsm_output_logic(); // do a task depends of the state

/*
*
*

Constants definitions

*
*
*/

// User image lcd id vector:
const unsigned int userimagesdlist[5] = {USERIMAGE_SDLIST_FILE_0,USERIMAGE_SDLIST_FILE_1,USERIMAGE_SDLIST_FILE_2,USERIMAGE_SDLIST_FILE_3,USERIMAGE_SDLIST_FILE_4};
// Button lcd id vector
const unsigned int buttonsdselected[6] = {BUTTON_SDLIST_SELECT0, BUTTON_SDLIST_SELECT1, BUTTON_SDLIST_SELECT2, BUTTON_SDLIST_SELECT3, BUTTON_SDLIST_SELECT4, 255};
// String lcd id vector
const unsigned int stringfilename[8] = {STRING_SDLIST_NAMEFILE0, STRING_SDLIST_NAMEFILE1, STRING_SDLIST_NAMEFILE2, STRING_SDLIST_NAMEFILE3, STRING_SDLIST_NAMEFILE4, 255,STRING_SDLIST_CONFIRMATION_NAMEFILE,STRING_RECOVERY_PRINT_ASK}; // String lcd id vector
// Custom digits for jint 6 and 7
const unsigned int stringfiledur[3][2]=
{
	{CUSTOMDIGITS_SDLIST_CONFIRMATION_DURFILE_HOURS	, CUSTOMDIGITS_RECOVERY_PRINT_ASK_DUR_HOURS	},
	{CUSTOMDIGITS_SDLIST_CONFIRMATION_DURFILE_MIN	, CUSTOMDIGITS_RECOVERY_PRINT_ASK_DUR_MIN	},
	{CUSTOMDIGITS_SDLIST_CONFIRMATION_DURFILE_G		, CUSTOMDIGITS_RECOVERY_PRINT_ASK_DUR_G		}
};
const unsigned int sdfiledurhor[3][5]= // Custom digits lcd id for hours
{
	#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
	{33, 37, 45, 54, 64},
	{32, 36, 44, 51, 63},
	{30, 35, 52, 50, 62}
	#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
	{24, 34, 45, 41, 61},
	{23, 33, 44, 51, 60},
	{30, 32, 52, 50, 59}
	#endif
};
const unsigned int sdfiledurmin[2][5]= // Custom digits lcd id for min
{
	#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
	{26, 39, 53, 56, 66},
	{34, 38, 46, 55, 65}
	#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
	{26, 36, 53, 54, 63},
	{25, 35, 43, 42, 62}
	#endif
};
const unsigned int sdfilepes[4][5]= // Custom digits lcd id for weight
{
	#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
	{31, 43, 58, 61, 70},
	{29, 42, 49, 60, 69},
	{28, 41, 48, 59, 68},
	{27, 40, 47, 57, 67}
	#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
	{31, 40, 58, 49, 67},
	{29, 39, 48, 57, 66},
	{28, 38, 47, 56, 65},
	{27, 37, 46, 55, 64}
	#endif
};
const unsigned int register_codes[20]= // Register codes vector
{
	REGIST_CODE_0, REGIST_CODE_1, REGIST_CODE_2, REGIST_CODE_3, REGIST_CODE_4,
	REGIST_CODE_5, REGIST_CODE_6, REGIST_CODE_7, REGIST_CODE_8, REGIST_CODE_9,
	REGIST_CODE_10, REGIST_CODE_11, REGIST_CODE_12, REGIST_CODE_13, REGIST_CODE_14,
	REGIST_CODE_15, REGIST_CODE_16, REGIST_CODE_17, REGIST_CODE_18, REGIST_CODE_19
	
};
const unsigned int pixelsize_char_uppercase[26]= // Upper case dimensionless value size
{
	21,16,16,18,14,13,17,18,7,12,17,13,22,
	19,19,16,19,17,14,13,18,16,25,16,14,16

};
const unsigned int pixelsize_char_lowercase[26]= // Lower case dimensionless value size
{
	14,16,14,17,14,8,15,16,7,8,15,7,25,
	16,15,16,16,16,9,12,15,12,21,13,11,12
	
};
const unsigned int pixelsize_char_symbols[20]= // Symbols case dimensionless value size
{
	7,7,12,12,7,9,7,8,17,9,13,
	13,14,13,15,13,15,15,7,7
	
};


void lcd_fsm_lcd_input_logic(){//We process tasks according to the "LCD_FSM_input_buton_flag" value
	/*
	*
	*

	Local variables definitions

	*
	*
	*/
	int updir;
	int tHotend;
	int tHotend1;
	int tBed;
	int percentage;
	int Tref;
	int Tfinal;
	float calculus;
	int j = 0;
	#if PATTERN_Z_CALIB == 1
	int result=0;
	#endif
	static uint32_t waitPeriod = millis();
	static uint32_t waitPeriod_s = millis();
	
	lcd_busy = true; // MCU is busy
	
	if (card.sdprinting || card.sdispaused){ // Printer is printing or paused
		
		switch(LCD_FSM_input_buton_flag) {
			
			#ifdef ENCLOSURE_SAFETY_STOP
			case BUTTON_PRINT_PAUSE_NOTIFICATION_ENCLOSURE_TOCLOSE_BACK: //Go back Pause
			case BUTTON_PRINT_PAUSE_NOTIFICATION_ENCLOSURE_ISOPEN_NEXT:  // Go back Pause

			display_ChangeForm(FORM_SDPRINTING_PAUSE,0);
			screen_printing_pause_form = screen_printing_pause_form1;
			display.WriteStr(STRING_SDPRINTING_PAUSE_GCODE,namefilegcode);
			bitSet(flag_sdprinting_register,flag_sdprinting_register_datarefresh);

			break;

			#endif
			
			case BUTTON_SDPRINTING_SETTINGS: // Go to Temperatures Window
			
			if(screen_printing_pause_form == screen_printing_pause_form0){
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_SDPRINTING,1);
				display_ButtonState(BUTTON_SDPRINTING_SETTINGS,1);
				display_ButtonState(BUTTON_SDPRINTING_PAUSE,1);
				display_ButtonState(BUTTON_SDPRINTING_STOP,1);
				display_ButtonState(BUTTON_SDPRINTING_BACKSTATE,1);
				screen_printing_pause_form = screen_printing_pause_form3;
				
				}else if(screen_printing_pause_form == screen_printing_pause_form3){
				bitSet(flag_sdprinting_register,flag_sdprinting_register_temps);
				screen_printing_pause_form = screen_printing_pause_form4; // Set Temps State
			}
			
			break;
			
			case BUTTON_SDPRINTING_BACKSTATE: // Back to Printer Screen "screen_printing_pause_form0"
			
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_SDPRINTING,0);
			display_ButtonState(BUTTON_SDPRINTING_SETTINGS,0);
			display_ButtonState(BUTTON_SDPRINTING_PAUSE,0);
			display_ButtonState(BUTTON_SDPRINTING_STOP,0);
			display_ButtonState(BUTTON_SDPRINTING_BACKSTATE,0);
			screen_printing_pause_form = screen_printing_pause_form0;
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_BACK: // Back to Printer Screen or Pause Screen
			case BUTTON_SDPRINTTING_CONTROL_BACK:
			
			bitSet(flag_sdprinting_register,flag_sdprinting_register_showdata);
			screen_printing_pause_form = screen_printing_pause_form3;
			break;
			
			
			case BUTTON_SDPRINTING_PAUSE_UTILITIES: //if screen_printing_pause_form1, Swap Pause form to Utilities; if screen_printing_pause_form2 go to Purge
			
			if(screen_printing_pause_form == screen_printing_pause_form1){
				screen_printing_pause_form = screen_printing_pause_form2;
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_SDPRINTING_PAUSE,1);
				display_ButtonState(BUTTON_SDPRINTING_PAUSE_STOP,1);
				display_ButtonState(BUTTON_SDPRINTING_PAUSE_RESUME,1);
				display_ButtonState(BUTTON_SDPRINTING_PAUSE_UTILITIES,1);
				display_ButtonState( BUTTON_SDPRINTING_PAUSE_BACKSTATE, 1);
				}else if(screen_printing_pause_form == screen_printing_pause_form2){
				is_purging_filament = true;
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_LOAD,0);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD,0);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPUP,0);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPDOWN,0);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_MENU,1);
				display_ChangeForm(FORM_UTILITIES_FILAMENT_PURGE,0);
				surfing_utilities = true;
				purge_extruder_selected = -1;
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0,0);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1,0);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_LEFTTARGET,int(degHotend(0)));
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_RIGHTTARGET,int(degHotend(1)));
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,0);
				is_printing_screen = false;
			}
			
			break;
			
			case BUTTON_PRINTERSETUP_LED_SAVE: // Go to Lighting control screen
			
			display_ButtonState(BUTTON_PRINTERSETUP_LED_MENU,0);
			bitSet(flag_sdprinting_register,flag_sdprinting_register_showdata);
			
			break;
			
			//case BUTTON_PRINTERSETUP_LED_BACK:
			//display_ButtonState(BUTTON_PRINTERSETUP_LED_MENU,0);
			//bitSet(flag_sdprinting_register,flag_sdprinting_register_showdata);
			//break;
			
			case BUTTON_SDPRINTING_PAUSE_BACKSTATE: // Back to Pause Form
			
			if(screen_printing_pause_form == screen_printing_pause_form2){
				screen_printing_pause_form = screen_printing_pause_form1;
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_SDPRINTING_PAUSE,0);
				display_ButtonState( BUTTON_SDPRINTING_PAUSE_STOP, 0);
				display_ButtonState( BUTTON_SDPRINTING_PAUSE_RESUME, 0);
				display_ButtonState( BUTTON_SDPRINTING_PAUSE_UTILITIES, 0);
				display_ButtonState( BUTTON_SDPRINTING_PAUSE_BACKSTATE, 0);
				surfing_utilities = false;
			}
			
			break;
			
			case BUTTON_FILAMENTDETECTOR_NOTICE_ABORT: // Go to after filament runout sensor notification Abort Print
			
			display_ChangeForm(FORM_FILAMENTDETECTOR_ABORT,0);
			
			break;
			
			case BUTTON_FILAMENTDETECTOR_ABORT_YES: // Execute Abort print
			case BUTTON_SDPRINTING_STOPPRINT_YES:
			
			card.closefile();
			bitSet(flag_sdprinting_register,flag_sdprinting_register_printstop);
			cancel_heatup = true;
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			
			break;
			
			case BUTTON_FILAMENTDETECTOR_ABORT_NO: // Back to FRS window
			case BUTTON_UTILITIES_FILAMENT_LOAD_BACK:
			
			if(Flag_checkfil){
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_FILAMENTDETECTOR_NOTICE,(which_extruder_needs_fil== 10)?0:1);
				display_ChangeForm(FORM_FILAMENTDETECTOR_NOTICE,0);
			}
			
			break;
			
			case BUTTON_FILAMENTDETECTOR_NOTICE_CHANGEFILAMENT: // FRS, Reload filament to continue with the print job
			
			filament_mode = 'I';
			insertmetod();
			
			break;
			
			case BUTTON_FILAMENTDETECTOR_NOTICE_UNLOAD_NEXT: // FRS, Unload the remnant filament
			
			setTargetHotend0(target_temperature_check_filament_cooldown_save[0]);
			setTargetHotend1(target_temperature_check_filament_cooldown_save[1]);
			screen_printing_pause_form = screen_printing_pause_form1;
			filament_mode = 'R';
			surfing_utilities = true;
			which_extruder = (which_extruder_needs_fil==10) ? 0:1;
			
			Temp_ChangeFilament_Saved = target_temperature[which_extruder];
			if(which_extruder == 0) setTargetHotend(unload_temp_l,which_extruder);
			else setTargetHotend(unload_temp_r,which_extruder);
			
			gif_processing_state = PROCESSING_DEFAULT;
			display_ChangeForm(FORM_PROCESSING,0);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_UNLOAD_MENU,0);
			
			current_position[Z_AXIS]=Z_MAX_POS-15;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder); //check speed
			st_synchronize();
			current_position[Y_AXIS]=10;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Y_AXIS], active_extruder); //check speed
			st_synchronize();
			ERROR_SCREEN_WARNING;
			
			gif_processing_state = PROCESSING_STOP;
			
			touchscreen_update();
			
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS, CUSTOMDIGITS_ADJUSTING_TEMPERATURES, 0);
			display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
			
			Tref1 = (int)degHotend(which_extruder);
			Tfinal1 = (int)degTargetHotend(which_extruder)-CHANGE_FIL_TEMP_HYSTERESIS;
			Tpercentage_old = 0;
			
			gif_processing_state = PROCESSING_ADJUSTING;
			touchscreen_update();
			is_changing_filament=true; //We are changing filament
			
			break;
			
			case BUTTON_SDPRINTING_STOPPRINT_NO: // Cancel Abort print
			
			if(screen_printing_pause_form == screen_printing_pause_form0){
				
				
				display_ChangeForm(FORM_SDPRINTING,0);
				display.WriteStr(STRING_SDPRINTING_GCODE,namefilegcode);
				bitSet(flag_sdprinting_register,flag_sdprinting_register_datarefresh);
				surfing_utilities = false;
				}else{
				display_ChangeForm(FORM_SDPRINTING_PAUSE,0);
				display.WriteStr(STRING_SDPRINTING_PAUSE_GCODE,namefilegcode);
				bitSet(flag_sdprinting_register,flag_sdprinting_register_datarefresh);
				surfing_utilities = false;
			}
			
			break;
			
			case BUTTON_SDPRINTING_STOPPRINT_SAVE: // Stop Print job and recover it later
			
			if(!waiting_temps && !card.sdispaused){
				
				display_ChangeForm(FORM_SDPRINTING_SAVEPRINT_SURE,0); // Go to Confirmation Window
				
			}
			
			break;
			
			case BUTTON_SDPRINTING_SAVEPRINT_SURE_NOT: // Cancel Save print job
			
			if(!waiting_temps){
				
				
				display_ChangeForm(FORM_SDPRINTING_STOPPRINT,0);
				
				
			}
			
			break;
			
			case BUTTON_SDPRINTING_SAVEPRINT_SURE_OK: // Confirm Save print job
			
			if(!waiting_temps){
				
				
				
				if(screen_printing_pause_form == screen_printing_pause_form0){
					
					card.sdprinting = false;
					card.sdispaused = true;
					display_ChangeForm(FORM_PROCESSING,0);
					gif_processing_state = PROCESSING_DEFAULT;
					bitSet(flag_sdprinting_register,flag_sdprinting_register_printsavejobcommand);
					
				}
				
				
			}
			
			break;
			
			case BUTTON_SDPRINTING_STOP: // Go to Confirmation abort print job, from printing screen
			
			if(screen_printing_pause_form == screen_printing_pause_form0){
				
				display_ChangeForm(FORM_SDPRINTING_STOPPRINT,0);
				
				
				}else if(screen_printing_pause_form == screen_printing_pause_form3){
				bitSet(flag_sdprinting_register,flag_sdprinting_register_light);
			}
			is_printing_screen = false;
			
			break;
			
			case BUTTON_SDPRINTING_PAUSE_STOP: // Go to Confirmation abort print job, from pause screen
			
			if(screen_printing_pause_form == screen_printing_pause_form1){
				
				display_ChangeForm(FORM_SDPRINTING_STOPPRINT,0);
				
				
				}else if(screen_printing_pause_form == screen_printing_pause_form2){
				
				bitSet(flag_sdprinting_register,flag_sdprinting_register_light);
				
				
			}
			is_printing_screen = false;
			
			break;
			
			case BUTTON_SDPRINTING_PAUSE: // Pause print job if "screen_printing_pause_form0", "screen_printing_pause_form3" open to advanced settings
			
			if(card.sdprinting && screen_printing_pause_form == screen_printing_pause_form0){
				
				bitSet(flag_sdprinting_register,flag_sdprinting_register_printpause);
				
				}else if(screen_printing_pause_form == screen_printing_pause_form3){
				bitSet(flag_sdprinting_register,flag_sdprinting_register_control);
				screen_printing_pause_form = screen_printing_pause_form5; // Set Advanced Settins state
			}
			
			break;
			
			case BUTTON_SDPRINTING_PAUSE_RESUME: // Resume print job if "screen_printing_pause_form1", "screen_printing_pause_form2" open Change filament during printing
			
			if(card.sdispaused){
				if(screen_printing_pause_form == screen_printing_pause_form1){
					#ifdef ENCLOSURE_SAFETY_STOP
					if(CHECK_ENCLOSURE_IS_CLOSED && Flag_enclosure_enabled){
						display_ChangeForm(FORM_PRINT_PAUSE_NOTIFICATION_ENCLOSURE_TOCLOSE,0);
						is_printing_screen = false;
						}else{
						bitSet(flag_sdprinting_register,flag_sdprinting_register_printresume);
					}


					#else
					bitSet(flag_sdprinting_register,flag_sdprinting_register_printresume);
					#endif
				}
				else if(screen_printing_pause_form == screen_printing_pause_form2){
					display_ChangeForm(FORM_UTILITIES_FILAMENT_UNLOAD,0);
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_UNLOAD_MENU,1);
					filament_mode = 'R';
					surfing_utilities = true;
					is_printing_screen = false;
				}
			}
			
			break;
			
			// Printing settings:
			case  BUTTON_SDPRINTTING_SETINGS_FAN_LEFT_UP:
			
			bitSet(screen_change_register,screen_change_register_fanup);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_FAN_LEFT_DOWN:
			
			bitSet(screen_change_register,screen_change_register_fandown);
			
			break;
			
			case  BUTTON_SDPRINTTING_SETINGS_FAN_RIGHT_UP:
			
			if(get_dual_x_carriage_mode()==DXC_FULL_SIGMA_MODE)bitSet(screen_change_register,screen_change_register_fanupr);
			else bitSet(screen_change_register,screen_change_register_fanup);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_FAN_RIGHT_DOWN:
			
			if(get_dual_x_carriage_mode()==DXC_FULL_SIGMA_MODE)bitSet(screen_change_register,screen_change_register_fandownr);
			else bitSet(screen_change_register,screen_change_register_fandown);
			
			break;
			
			case  BUTTON_SDPRINTTING_SETINGS_FLOW_LEFT_UP:
			bitSet(screen_change_register,screen_change_register_flowup);
			break;
			
			case  BUTTON_SDPRINTTING_SETINGS_FLOW_RIGHT_UP:
			
			if(get_dual_x_carriage_mode()==DXC_FULL_SIGMA_MODE)bitSet(screen_change_register,screen_change_register_flowupr);
			else bitSet(screen_change_register,screen_change_register_flowup);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_FLOW_LEFT_DOWN:
			
			bitSet(screen_change_register,screen_change_register_flowdown);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_FLOW_RIGHT_DOWN:
			
			if(get_dual_x_carriage_mode()==DXC_FULL_SIGMA_MODE)bitSet(screen_change_register,screen_change_register_flowdownr);
			else bitSet(screen_change_register,screen_change_register_flowdown);
			
			break;
			
			case  BUTTON_SDPRINTTING_SETINGS_SPEED_LEFT_UP:
			
			bitSet(screen_change_register,screen_change_register_speedup);
			
			break;
			
			case  BUTTON_SDPRINTTING_SETINGS_SPEED_RIGHT_UP:
			
			if(get_dual_x_carriage_mode()==DXC_FULL_SIGMA_MODE)bitSet(screen_change_register,screen_change_register_speedupr);
			else bitSet(screen_change_register,screen_change_register_speedup);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_SPEED_LEFT_DOWN:
			
			bitSet(screen_change_register,screen_change_register_speeddown);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_SPEED_RIGHT_DOWN:
			
			if(get_dual_x_carriage_mode()==DXC_FULL_SIGMA_MODE)bitSet(screen_change_register,screen_change_register_speeddownr);
			else bitSet(screen_change_register,screen_change_register_speeddown);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_LEFT_UP:
			
			bitSet(screen_change_register,screen_change_register_nozz1up);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_LEFT_DOWN:
			
			bitSet(screen_change_register,screen_change_register_nozz1down);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_RIGHT_UP:
			
			bitSet(screen_change_register,screen_change_register_nozz2up);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_RIGHT_DOWN:
			
			bitSet(screen_change_register,screen_change_register_nozz2down);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_BED_UP:
			
			bitSet(screen_change_register,screen_change_register_bedup);
			
			break;
			
			case BUTTON_SDPRINTTING_SETINGS_BED_DOWN:
			
			bitSet(screen_change_register,screen_change_register_beddown);
			
			break;
			
			// end Print settings
			
			case BUTTON_UTILITIES_FILAMENT_UNLOAD_BACK: // Back from change filament during print job
			
			display_ChangeForm(FORM_SDPRINTING_PAUSE,0);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_UNLOAD_MENU,0);
			surfing_utilities = false;
			display.WriteStr(STRING_SDPRINTING_PAUSE_GCODE,namefilegcode);
			bitSet(flag_sdprinting_register,flag_sdprinting_register_datarefresh);
			
			break;
			
			
			case BUTTON_UTILITIES_FILAMENT_UNLOAD_SELECTLEFT:
			case BUTTON_UTILITIES_FILAMENT_UNLOAD_SELECTRIGHT:
			
			if (LCD_FSM_input_buton_flag == BUTTON_UTILITIES_FILAMENT_UNLOAD_SELECTLEFT) //Left Nozzle
			{
				
				which_extruder=0;
				
			}
			else //Right Nozzle
			{
				
				which_extruder=1;
			}
			Temp_ChangeFilament_Saved = target_temperature[which_extruder];
			if(which_extruder == 0) setTargetHotend(unload_temp_l,which_extruder);
			else setTargetHotend(unload_temp_r,which_extruder);
			
			gif_processing_state = PROCESSING_DEFAULT;
			display_ChangeForm(FORM_PROCESSING,0);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_UNLOAD_MENU,0);
			
			current_position[Z_AXIS]=Z_MAX_POS-15;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder); //check speed
			st_synchronize();
			
			current_position[Y_AXIS]=10;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Y_AXIS], active_extruder); //check speed
			st_synchronize();
			ERROR_SCREEN_WARNING;
			
			gif_processing_state = PROCESSING_STOP;
			
			touchscreen_update();
			
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS, CUSTOMDIGITS_ADJUSTING_TEMPERATURES, 0);
			display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
			Tref1 = (int)degHotend(which_extruder);
			Tfinal1 = (int)degTargetHotend(which_extruder)-CHANGE_FIL_TEMP_HYSTERESIS;
			Tpercentage_old = 0;
			
			gif_processing_state = PROCESSING_ADJUSTING;
			touchscreen_update();
			is_changing_filament=true;
			
			break;
			
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_FEELSTOPS: // The user pushes the filament until reaches the extruder; then the button should be pressed; heat the heater and wait.
			
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, 0);
			display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
			gif_processing_state = PROCESSING_ADJUSTING;
			
			touchscreen_update();
			
			if (which_extruder==0) setTargetHotend(max(load_temp_l,old_load_temp_l),which_extruder);
			else setTargetHotend(max(load_temp_r,old_load_temp_r),which_extruder);
			
			Tref1 = (int)degHotend(which_extruder);
			Tfinal1 = (int)degTargetHotend(which_extruder)-CHANGE_FIL_TEMP_HYSTERESIS;
			Tpercentage_old = 0;
			
			touchscreen_update();
			
			is_changing_filament=true;
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_KEEPPUSHING_NEXT: // Start loading the filament; if the FRS is enabled, check if there is filament on the detected before start pushing
			
			if (!Flag_FRS_enabled || (Flag_FRS_enabled && CHECK_FRS(which_extruder)))
			{ //Inserting...
				is_checking_filament = false;
				display_ChangeForm(FORM_PROCESSING,0);
				gif_processing_state = PROCESSING_DEFAULT;
				//current_position[X_AXIS] = extruder_offset[X_AXIS][which_extruder];
				//plan_set_position(extruder_offset[X_AXIS][which_extruder], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
				/*#if BCN3D_PRINTER_SETUP == BCN3D_SIGMA_PRINTER_SIGMA
				current_position[X_AXIS] = 155;
				#else
				current_position[X_AXIS] = 155 + X_OFFSET_CALIB_PROCEDURES;
				#endif*/
				plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], XY_TRAVEL_SPEED15/60,which_extruder);
				st_synchronize();
				current_position[E_AXIS] += 30;//Extra extrusion at low feedrate
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS],  700/60, which_extruder); //850/60
				st_synchronize();
				ERROR_SCREEN_WARNING;
				current_position[E_AXIS] += ((BOWDEN_LENGTH-EXTRUDER_LENGTH)-15);//BOWDEN_LENGTH-300+340);
				if (axis_steps_per_unit[E_AXIS]<=512 && axis_steps_per_unit[E_AXIS]>=492){
					plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_FAST_R19_SPEED/60, which_extruder);
					}else{
					plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_FAST_SPEED/60, which_extruder);
				}
				st_synchronize();
				ERROR_SCREEN_WARNING;
				current_position[E_AXIS] += EXTRUDER_LENGTH;//Extra extrusion at low feedrate
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS],  INSERT_SLOW_SPEED/60, which_extruder);
				st_synchronize();
				ERROR_SCREEN_WARNING;
				gif_processing_state = PROCESSING_STOP;
				display_ChangeForm(FORM_UTILITIES_FILAMENT_ADJUST,0);
								
			}
			
			break;
			
			
			case BUTTON_UTILITIES_FILAMENT_UNLOAD_ROLLTHESPOOL_NEXT: // Unload the filament
			
			unloadfilament_procedure();
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_ADJUST_ACCEPT:
			
			if(!bitRead(flag_utilities_filament_register,flag_utilities_filament_register_acceptok))
			{
				
				if(!blocks_queued()){
					display_SetFrame(GIF_UTILITIES_FILAMENT_SUCCESS,0);
					display_ChangeForm(FORM_UTILITIES_FILAMENT_SUCCESS,0);
					gif_processing_state = PROCESSING_SUCCESS;
					printer_state = STATE_LOADUNLOAD_FILAMENT;
					setTargetHotend((float)Temp_ChangeFilament_Saved, which_extruder);
					Flag_checkfil = false;
					st_synchronize();
					}else{
					quickStop();
				}
				
			}
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_ADJUST_LOAD: // Spool is loaded, purge a bit more
			
			if(!bitRead(flag_utilities_filament_register,flag_utilities_filament_register_acceptok)){
				if(!blocks_queued()){
					bitSet(flag_utilities_filament_register,flag_utilities_filament_register_purgeload);
					}else{
					quickStop();
				}
				
			}
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_ADJUST_UNLOAD: // Spool is loaded, retract a bit
			
			if(!bitRead(flag_utilities_filament_register,flag_utilities_filament_register_acceptok)){
				if(!blocks_queued()){
					bitSet(flag_utilities_filament_register,flag_utilities_filament_register_purgeunload);
					}else{
					quickStop();
				}
			}
			
			break;
			
			// Purge Buttons:
			case BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0:
			
			if (!blocks_queued()){
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_LOAD,1);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD,1);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPUP,1);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPDOWN,1);
				if (purge_extruder_selected == 1){
					purge_extruder_selected = 0;
					if(target_temperature[0] == 0){
						setTargetHotend0(print_temp_l);
					}
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0,1);
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1,0);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,target_temperature[0]);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_LEFTTARGET,int(degHotend(0)));
				}
				else{
					purge_extruder_selected = 0;
					if(target_temperature[0] == 0){
						setTargetHotend0(print_temp_l);
					}
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0,1);
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1,0);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,target_temperature[0]);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_LEFTTARGET,int(degHotend(0)));
				}
			}
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1:
			
			if (!blocks_queued()){
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_LOAD,1);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD,1);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPUP,1);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPDOWN,1);
				if (purge_extruder_selected == 0){
					purge_extruder_selected = 1;
					if(target_temperature[1] == 0){
						setTargetHotend1(print_temp_r);
					}
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0,0);
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1,1);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,target_temperature[1]);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_RIGHTTARGET,int(degHotend(1)));
				}
				else{
					purge_extruder_selected = 1;
					if(target_temperature[1] == 0){
						setTargetHotend1(print_temp_r);
					}
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0,0);
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1,1);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,target_temperature[1]);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_RIGHTTARGET,int(degHotend(1)));
				}
			}
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_TEMPUP:
			
			if(purge_extruder_selected != -1){
				
				if (target_temperature[purge_extruder_selected] < HEATER_0_MAXTEMP){
					target_temperature[purge_extruder_selected] += 5;
					setTargetHotend0(target_temperature[0]);
					setTargetHotend1(target_temperature[1]);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,int(target_temperature[purge_extruder_selected]));
				}
			}
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_TEMPDOWN:
			
			if(purge_extruder_selected != -1){
				if (target_temperature[purge_extruder_selected] > 0){
					target_temperature[purge_extruder_selected] -= 5;
					setTargetHotend0(target_temperature[0]);
					setTargetHotend1(target_temperature[1]);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,int(target_temperature[purge_extruder_selected]));
				}
			}
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_LOAD:
			
			if(purge_extruder_selected != -1){
				if(!blocks_queued()){
					bitSet(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect0);
					}else{
					quickStop();
				}
			}
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD:
			
			if(purge_extruder_selected != -1){
				if(!blocks_queued()){
					bitSet(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect1);
					}else{
					quickStop();
				}
			}
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_BACK:
			
			if(!blocks_queued()){
				//quickStop();
				
				is_purging_filament = false;
				display_ChangeForm(FORM_SDPRINTING_PAUSE,0);
				touchscreen_update();
				surfing_utilities = false;
				display.WriteStr(STRING_SDPRINTING_PAUSE_GCODE,namefilegcode);
				bitSet(flag_sdprinting_register,flag_sdprinting_register_datarefresh);
				
				//setTargetHotend0(0);
				//setTargetHotend1(0);
				
			}
			
			break;
			// end Purge Buttons
			
			case BUTTON_UTILITIES_FILAMENT_SUCCESS: // Ending process
			
			if (printer_state == STATE_LOADUNLOAD_FILAMENT)
			{
				
				gif_processing_state = PROCESSING_STOP;
				bitClear(flag_sdprinting_register,flag_utilities_filament_register_acceptok);
				if (filament_mode == 'R')
				{
					
					filament_mode = 'I';
					insertmetod();
					
					//which_extruder = -1;
				}
				else if (filament_mode == 'I')
				{
					
					gif_processing_state = PROCESSING_DEFAULT;
					display_ChangeForm(FORM_PROCESSING,0);
					current_position[Y_AXIS] = saved_position[Y_AXIS];
					plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], homing_feedrate[Y_AXIS]/60, active_extruder);//Purge
					st_synchronize();
					ERROR_SCREEN_WARNING;
					
					//home_axis_from_code(true, true, false);
					
					current_position[Z_AXIS] = saved_position[Z_AXIS] + 10;
					plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS],homing_feedrate[Z_AXIS] ,saved_active_extruder);
					st_synchronize();
					ERROR_SCREEN_WARNING;
					gif_processing_state = PROCESSING_STOP;
					
					screen_printing_pause_form = screen_printing_pause_form1;
					display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_SDPRINTING_PAUSE,0);
					display_ButtonState( BUTTON_SDPRINTING_PAUSE_STOP, 0);
					display_ButtonState( BUTTON_SDPRINTING_PAUSE_RESUME, 0);
					display_ButtonState( BUTTON_SDPRINTING_PAUSE_UTILITIES, 0);
					display_ButtonState( BUTTON_SDPRINTING_PAUSE_BACKSTATE, 0);
					
					display_ChangeForm(FORM_SDPRINTING_PAUSE,0);
					surfing_utilities = false;
					display.WriteStr(STRING_SDPRINTING_PAUSE_GCODE,namefilegcode);
					bitSet(flag_sdprinting_register,flag_sdprinting_register_datarefresh);
				}
				#ifdef DEV_BER
				enquecommand_P(PSTR("M841"));
				Config_StoreSettings();
				#endif
				printer_state = STATE_NONE;
				//doblocking =true;
			}
			
			break;
			
			case BUTTON_ERROR_OK: // Error
			
			if(FLAG_thermal_runaway_screen){
				gif_processing_state = PROCESSING_STOP;
				FLAG_thermal_runaway_screen = false;
				bitSet(flag_sdprinting_register,flag_sdprinting_register_datarefresh);
				if(screen_printing_pause_form == screen_printing_pause_form0){
					display_ChangeForm(FORM_SDPRINTING,0);
					}else{
					display_ChangeForm(FORM_SDPRINTING_PAUSE,0);
				}
				
				}else{
				gif_processing_state = PROCESSING_STOP;
				screen_sdcard = false;
				surfing_utilities=false;
				
				SERIAL_PROTOCOLLNPGM("Main");
				
				surfing_temps = false;
				HeaterCooldownInactivity(true);
				display_ChangeForm( FORM_MAIN, 0);
			}
			
			break;
			
			default:
			break;
			//endswitch
		}
	}
	else if(bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode) && (Bed_Compensation_state > 1)){ // G36 executed
		/*SERIAL_PROTOCOLPGM("Bed_Compensation_Lines_Selected[0]");
		Serial.println(Bed_Compensation_Lines_Selected[0]);
		SERIAL_PROTOCOLPGM("Bed_Compensation_Lines_Selected[1]");
		Serial.println(Bed_Compensation_Lines_Selected[1]);
		SERIAL_PROTOCOLPGM("Bed_Compensation_Lines_Selected[2]");
		Serial.println(Bed_Compensation_Lines_Selected[2]);*/
		int8_t vuitensL = 0;
		int8_t vuitensR = 0;
		switch(LCD_FSM_input_buton_flag){
			
			// Line selection:
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT1:
			
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT1,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,1);
			calib_confirmation = -2;
			
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT2:
			
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT2,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,1);
			calib_confirmation = -1;
			
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT3:
			
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT3,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,1);
			calib_confirmation = 0;
			
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT4:
			
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT4,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,1);
			calib_confirmation = 1;
			
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT5:
			
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT5,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,1);
			calib_confirmation = 2;
			
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM:
			
			if(calib_confirmation <= 2 && calib_confirmation >= -2 && calib_confirmation!=1888 ){
				Bed_Compensation_Set_Lines(calib_confirmation);
			}
			
			break;
			// end Line selection
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOG36_BACK: // Cancel redo
			
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,0);
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,0);
			calib_confirmation = 1888;
			
			break;
			
			// Choose redo from best line printed:
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOG36_BEST1:
			
			Bed_compensation_redo_offset = -3;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);
			
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOG36_BEST5:
			
			Bed_compensation_redo_offset = 3;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);
			
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOG36:
			
			Bed_compensation_redo_offset = 0;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);
			
			break;
			// end Choose redo from best line printed
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_REDO: // go redo
			
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_REDOG36,0);
			
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED: // Clean Bed before print
			
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			home_axis_from_code(true,true,true);
			gif_processing_state = PROCESSING_STOP;
			st_synchronize();
			Calib_check_temps();
			Bed_Compensation_Redo_Lines(Bed_compensation_redo_offset);
			
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBBED_ADJUSTSCREWASKFIRST_NEXT: // Screw adjust to flat the heated bed
			
			gif_processing_state = PROCESSING_STOP;
			vuitensL = Bed_Compensation_Lines_Selected[1]-Bed_Compensation_Lines_Selected[0];
			vuitensR = Bed_Compensation_Lines_Selected[2]-Bed_Compensation_Lines_Selected[0];
			bed_offset_left_screw = -0.1*vuitensL;
			bed_offset_right_screw = -0.1*vuitensR;
			if(vuitensL != 0){
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBBED_SCREW2,0);
				if(vuitensL < 0){
					vuitensL = abs(vuitensL) + 8;
				}
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBBED_SCREW2,vuitensL);
				
			}
			else if(vuitensR != 0){
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBBED_SCREW3,0);
				if(vuitensR < 0){
					vuitensR = abs(vuitensR) + 8;
				}
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBBED_SCREW3,vuitensR);
			}
			else{
				printer_state = STATE_CALIBRATION;
				display_SetFrame(GIF_UTILITIES_CALIBRATION_SUCCESS,0);
				display_ChangeForm( FORM_UTILITIES_CALIBRATION_SUCCESS, 0);
				gif_processing_state = PROCESSING_BED_SUCCESS;
			}
			bed_offset_version = VERSION_NUMBER;
			Config_StoreSettings();
			
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBBED_SCREW2_NEXT: // Screw adjust next step
			
			vuitensR = 0;
			vuitensR = Bed_Compensation_Lines_Selected[2]-Bed_Compensation_Lines_Selected[0];
			if(vuitensR != 0){
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBBED_SCREW3,0);
				if(vuitensR < 0){
					vuitensR = abs(vuitensR) + 8;
				}
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBBED_SCREW3,vuitensR);
			}
			else{
				printer_state = STATE_CALIBRATION;
				display_SetFrame(GIF_UTILITIES_CALIBRATION_SUCCESS,0);
				display_ChangeForm( FORM_UTILITIES_CALIBRATION_SUCCESS, 0);
				gif_processing_state = PROCESSING_BED_SUCCESS;
			}
			
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBBED_SCREW3_NEXT: // Screw adjust final step
			
			printer_state = STATE_CALIBRATION;
			display_SetFrame(GIF_UTILITIES_CALIBRATION_SUCCESS,0);
			display_ChangeForm( FORM_UTILITIES_CALIBRATION_SUCCESS, 0);
			gif_processing_state = PROCESSING_BED_SUCCESS;
			
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_SUCCESS: // Ending G36 bed compensation process
			
			if(printer_state == STATE_CALIBRATION)
			{
				Bed_Compensation_state = 0;
				bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
				gif_processing_state = PROCESSING_STOP;
				setTargetBed(0);
				setTargetHotend0(0);
				setTargetHotend1(0);
				screen_sdcard = false;
				surfing_utilities=false;
				
				SERIAL_PROTOCOLLNPGM("Main");
				
				surfing_temps = false;
				HeaterCooldownInactivity(false);
				display_ChangeForm( FORM_MAIN, 0);
				printer_state = STATE_NONE;
			}
			
			break;
			default:
			break;
		}//endswitch
		
	}
	else{ // No print section, all the process are done at this part of the code
		char buffer[256];
		float feedrate;
		float value;
		switch(LCD_FSM_input_buton_flag){
			
			
			// SD List Selection
			
			case BUTTON_SDLIST_SELECT0:
			
			bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_select0);
			
			break;
			
			case BUTTON_SDLIST_SELECT1:
			
			bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_select1);
			
			break;
			
			case BUTTON_SDLIST_SELECT2:
			
			bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_select2);
			
			break;
			
			case BUTTON_SDLIST_SELECT3:
			
			bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_select3);
			
			break;
			
			case BUTTON_SDLIST_SELECT4:
			
			bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_select4);
			
			break;
			
			// end SD List Selection
			
			case BUTTON_SDLIST_FOLDERBACK: // Folder back
			
			updir = card.updir();
			if (updir==0){
				bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_gofolderback);
				folder_navigation_register(false);
			}
			else if(updir==1){
				bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_gofolderback);
				display_ButtonState( BUTTON_SDLIST_FOLDERBACK,0);
				display.WriteObject(GENIE_OBJ_USERIMAGES, USERIMAGE_SDLIST_FOLDERFILE,0);
				folder_navigation_register(false);
			}
			
			break;
			
			case BUTTON_INSERT_SD_CARD: // Back to Main Menu, sd init has failed
			
			screen_sdcard = false;
			surfing_utilities=false;
			
			SERIAL_PROTOCOLLNPGM("Main");
			
			surfing_temps = false;
			HeaterCooldownInactivity(true);
			display_ChangeForm( FORM_MAIN, 0);
			
			break;
			
			#ifdef ENCLOSURE_SAFETY_STOP
			case BUTTON_PRINT_START_NOTIFICATION_ENCLOSURE_ISOPEN_BACK:
			ListFileSelect_open();
			//display_ChangeForm(FORM_MAIN,0);
			//ListFileListINITSD();
			break;
			#endif
			
			
			case BUTTON_SDLIST_CONFIRMATION_YES: // Confirm to stat printing
			
			if(card.cardOK)
			{
				#ifdef ENCLOSURE_SAFETY_STOP
				if(CHECK_ENCLOSURE_IS_CLOSED && Flag_enclosure_enabled){
					 display_ChangeForm(FORM_PRINT_START_NOTIFICATION_ENCLOSURE_ISOPEN,0);
					 LCD_FSM_input_buton_flag = -1;
					 lcd_busy = false;
					 return;
				}
				#endif
				
				if(!listsd.check_extract_match_hotendsize_print()){ // check if the cfg hotend is the same like the gcode
					display_ChangeForm(FORM_SDLIST_CONFIRMATION_MATCHHOTEND, 0);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDLIST_CONFIRMATION_MATCHHOTEND_P_NOZZLE_SIZE_L_DIGIT1,(int)hotend_size_setup[LEFT_EXTRUDER]);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDLIST_CONFIRMATION_MATCHHOTEND_P_NOZZLE_SIZE_L_DIGIT2,(int)(hotend_size_setup[LEFT_EXTRUDER]*10)%10);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDLIST_CONFIRMATION_MATCHHOTEND_P_NOZZLE_SIZE_R_DIGIT1,(int)hotend_size_setup[RIGHT_EXTRUDER]);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDLIST_CONFIRMATION_MATCHHOTEND_P_NOZZLE_SIZE_R_DIGIT2,(int)(hotend_size_setup[RIGHT_EXTRUDER]*10)%10);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDLIST_CONFIRMATION_MATCHHOTEND_G_NOZZLE_SIZE_L_DIGIT1,(int)which_hotend_setup[LEFT_EXTRUDER]/10);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDLIST_CONFIRMATION_MATCHHOTEND_G_NOZZLE_SIZE_L_DIGIT2,(int)which_hotend_setup[LEFT_EXTRUDER]%10);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDLIST_CONFIRMATION_MATCHHOTEND_G_NOZZLE_SIZE_R_DIGIT1,(int)which_hotend_setup[RIGHT_EXTRUDER]/10);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDLIST_CONFIRMATION_MATCHHOTEND_G_NOZZLE_SIZE_R_DIGIT2,(int)which_hotend_setup[RIGHT_EXTRUDER]%10);
					LCD_FSM_input_buton_flag = -1;
					lcd_busy = false;
					return;
					
				}
				if(listsd.check_extract_ensure_duplication_print()){ // check if the gcode is a duplication or mirror, and verify if the offset are compensated
					
					if(abs(extruder_offset[Z_AXIS][RIGHT_EXTRUDER]) > RAFT_Z_THRESHOLD){
						display_ChangeForm(FORM_RAFT_ADVISE, 0);
						sprintf_P(buffer, PSTR("The first layer printed with the %s hotend"),
						((extruder_offset[Z_AXIS][RIGHT_EXTRUDER] < 0)?"left":"right"));
						//sprintf_P(buffer, PSTR("Info: First layer printed with"));
						display.WriteStr(STRING_RAFT_ADVISE_Z_OFFSET,buffer);
						memset(buffer,'\0',sizeof(buffer));
						sprintf_P(buffer, PSTR("will be distorted by %d.%1d%1d mm."),(int)abs(extruder_offset[Z_AXIS][RIGHT_EXTRUDER]),(int)(abs(extruder_offset[Z_AXIS][RIGHT_EXTRUDER])*10)%10, (int)(abs(extruder_offset[Z_AXIS][RIGHT_EXTRUDER])*100)%10);
						display.WriteStr(STRING_RAFT_ADVISE_Z_OFFSET2,buffer);
						LCD_FSM_input_buton_flag = -1;
						lcd_busy = false;
						return;
					}
				}
				if (!card.filenameIsDir){ //If the filename is a gcode we start printing
					card.getfilename(filepointer);
					char cmd[4 + strlen(card.filename) + 1]; // Room for "M23 ", filename, and null
					sprintf_P(cmd, PSTR("M23 %s"), card.filename);
					for (char *c = &cmd[4]; *c; c++) *c = tolower(*c);
					enquecommand(cmd);
					enquecommand_P(PSTR("M24")); // It also sends you to PRINTING screen
				}
				
			}
			
			break;
			
			case BUTTON_RAFT_ADVISE_ACCEPT: // Continue without shims
			if(card.cardOK)
			{
				
				if (!card.filenameIsDir){ //If the filename is a gcode we start printing
					card.getfilename(filepointer);
					char cmd[4 + strlen(card.filename) + 1]; // Room for "M23 ", filename, and null
					sprintf_P(cmd, PSTR("M23 %s"), card.filename);
					for (char *c = &cmd[4]; *c; c++) *c = tolower(*c);
					enquecommand(cmd);
					enquecommand_P(PSTR("M24")); // It also sends you to PRINTING screen
				}
				
			}
			
			break;
			
			case BUTTON_SDLIST_CONFIRMATION_MATCHHOTEND_NO: // not print
			case BUTTON_RAFT_ADVISE_INSTALL_CANCEL:
			case BUTTON_SDLIST_CONFIRMATION_NO:
			case BUTTON_MAIN_SDLIST:
			
			screen_sdcard = true;
			bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_goinit);
			SERIAL_PROTOCOLLNPGM("Busy");
			break;
			
			
			case BUTTON_SDLIST_GOUP: // sd list scroll up
			
			bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_goup);
			
			break;
			
			case BUTTON_SDLIST_GODOWN: // sd list scroll down
			
			bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_godown);
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE: // Move from Utilities - > Maintenance
			
			display_ChangeForm( FORM_UTILITIES_MAINTENANCE, 0);
			
			break;
			
			case BUTTON_MAIN_TEMPS: // Move from Main Menu - > Temperatures
			SERIAL_PROTOCOLLNPGM("Busy");
			display_ChangeForm( FORM_TEMP, 0);
			HeaterCooldownInactivity(false);
			tHotend=target_temperature[0];
			tHotend1=target_temperature[1];
			tBed=target_temperature_bed;
			
			
			flag_temp_gifhotent0 = false;
			flag_temp_gifhotent1 = false;
			flag_temp_gifbed = false;
			surfing_temps = true;
			
			break;
			
			case BUTTON_TEMP_BACK: // Move from Temperatures - > Main Menu
			
			screen_sdcard = false;
			surfing_utilities = false;
			
			SERIAL_PROTOCOLLNPGM("Main");
						
			surfing_temps = false;
			touchscreen_update();
			HeaterCooldownInactivity(true);
			display_ChangeForm( FORM_MAIN, 0);
			
			break;
			
			case BUTTON_TEMP_LEXTR: // Heat Left Hotend , Cool down Left Hotend
			
			tHotend=target_temperature[0];
			if(tHotend != 0){
				setTargetHotend0(0);
			}
			else{
				setTargetHotend0(print_temp_l);
			}
			flag_temp_gifhotent0 = false;
			
			break;
			
			case BUTTON_TEMP_REXTR: // Heat Right Hotend , Cool down Right Hotend
			
			tHotend1=target_temperature[1];
			if(tHotend1 != 0)
			{
				setTargetHotend1(0);
			}
			else{
				setTargetHotend1(print_temp_r);
			}
			flag_temp_gifhotent1 = false;
			
			break;
			
			case BUTTON_TEMP_BED: // Heat Bed , Cool down Bed
			
			tBed=target_temperature_bed;
			if(tBed != 0){
				setTargetBed(0);
			}
			else {
				setTargetBed( max(bed_temp_l,bed_temp_r));
			}
			flag_temp_gifbed = false;
			
			break;
			
			case BUTTON_RECOVERY_PRINT_ASK_ACCEPT: // Restore print job
			
			enquecommand_P(PSTR("M34"));
			
			break;
			
			case BUTTON_RECOVERY_PRINT_ASK_CANCEL: // Let's skip saved print job
			
			display_ChangeForm(FORM_RECOVERYPRINT_TOBELOST,0);
			
			break;
			
			case BUTTON_RECOVERYPRINT_TOBELOST_ACCEPT: // Confirm to Skip saved print job
			
			display_ChangeForm(FORM_MAIN,0);
			screen_sdcard = false;
			surfing_utilities=false;
			
			SERIAL_PROTOCOLLNPGM("Main");
			
			surfing_temps = false;
			saved_print_flag = 888;
			Config_StoreSettings();
			
			break;
			
			case BUTTON_RECOVERYPRINT_TOBELOST_BACK: // Do not to skip saved print job
			
			display_ChangeForm(FORM_RECOVERY_PRINT_ASK,0);
			card.initsd();
			if (card.cardOK){
				
				workDir_vector_lenght=saved_workDir_vector_lenght;
				for(int i=0; i<saved_workDir_vector_lenght;i++){
					card.getWorkDirName();
					card.getfilename(saved_workDir_vector[i]);
					workDir_vector[i]=saved_workDir_vector[i];
					if (!card.filenameIsDir){
						}else{
						if (card.chdir(card.filename)!=-1){
						}
					}
				}
				setfilenames(7);
			}
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_ZADJUST: // Z leveling
			
			if(saved_print_flag==1888){
				saved_print_flag = 888;
				Config_StoreSettings();
			}
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			if(home_made_Z){
				home_axis_from_code(true,true,false);
				st_synchronize();
				ERROR_SCREEN_WARNING;
			}
			else{
				home_axis_from_code(true,true,true);
				st_synchronize();
				ERROR_SCREEN_WARNING;
			}
			HeaterCooldownInactivity(true);
			gif_processing_state = PROCESSING_STOP;
			enquecommand_P((PSTR("T0")));
			st_synchronize();
			ERROR_SCREEN_WARNING;
			display_ChangeForm( FORM_MAINTENANCE_ZADJUST, 0);
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_BACK: // Move from Maintenance - > Utilities
			
			display_ChangeForm( FORM_UTILITIES, 0);
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_AUTOTUNEHOTENDS_NOTICE_NEXT: // Execute autotune hotends
			
			display_ChangeForm( FORM_ADJUSTING_TEMPERATURES, 0);
			bitSet(flag_utilities_maintenance_register,flag_utilities_maintenance_register_autotune);
			gif_processing_state = PROCESSING_ADJUSTING;
			percentage = 0;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, percentage);
			PID_autotune_Save(print_temp_l, 0, AUTOTUNE_ITERATIONS, 25.0); // Autotune left hotend
			Config_StoreSettings();
			SERIAL_PROTOCOL(MSG_OK);
			SERIAL_PROTOCOL(" p:");
			SERIAL_PROTOCOL(Kp[0]);
			SERIAL_PROTOCOL(" i:");
			SERIAL_PROTOCOL(unscalePID_i(Ki[0]));
			SERIAL_PROTOCOL(" d:");
			SERIAL_PROTOCOL(unscalePID_d(Kd[0]));
			PID_autotune_Save(print_temp_r, 1, AUTOTUNE_ITERATIONS, 25.0); // Autotune right hotend
			Config_StoreSettings();
			SERIAL_PROTOCOL(MSG_OK);
			SERIAL_PROTOCOL(" p:");
			SERIAL_PROTOCOL(Kp[1]);
			SERIAL_PROTOCOL(" i:");
			SERIAL_PROTOCOL(unscalePID_i(Ki[1]));
			SERIAL_PROTOCOL(" d:");
			SERIAL_PROTOCOL(unscalePID_d(Kd[1]));
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			display_SetFrame(GIF_UTILITIES_CALIBRATION_SUCCESS,0);
			display_ChangeForm( FORM_UTILITIES_CALIBRATION_SUCCESS, 0);
			HeaterCooldownInactivity(true);
			printer_state = STATE_CALIBRATION;
			gif_processing_state = PROCESSING_BED_SUCCESS;
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_AUTOTUNEHOTENDS: // Move from Maintenance - > Autotune
			
			display_ChangeForm( FORM_UTILITIES_MAINTENANCE_AUTOTUNEHOTENDS_NOTICE, 0);
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_AUTOTUNEHOTENDS_NOTICE_BACK: // Move from Autotune - > Maintenance
			
			display_ChangeForm( FORM_UTILITIES_MAINTENANCE, 0);
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_MENU: // Move from Maintenance - > Main Menu
			
			screen_sdcard = false;
			surfing_utilities=false;
			
			SERIAL_PROTOCOLLNPGM("Main");
			
			surfing_temps = false;
			touchscreen_update();
			display_ChangeForm( FORM_MAIN, 0);
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_NOTICE_BACK: // Move from Nylon Cleaning - > Maintenance
			
			display_ChangeForm( FORM_UTILITIES_MAINTENANCE, 0);
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_NOTICE_NEXT: // Go Nylon Cleaning
			
			filament_mode = 'R';
			bitSet(flag_utilities_maintenance_register,flag_utilities_maintenance_register_nyloncleaning);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTLEFT, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTRIGHT, 0);
			display.WriteObject(GENIE_OBJ_USERIMAGES, USERIMAGE_UTILITIES_MAINTENANCE_NYLONCLEANING_TEXTBUTTON, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADSKIP, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADGO, 0);
			display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING,0);
			which_extruder = 255;
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING: // Move from Maintenance - > Nylon Cleaning
			
			display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING_NOTICE,0);
			
			break;
			
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_BACK: // Move from Nylon Cleaning - > Maintenance
			
			display_ChangeForm( FORM_UTILITIES_MAINTENANCE, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTLEFT, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTRIGHT, 0);
			display.WriteObject(GENIE_OBJ_USERIMAGES, USERIMAGE_UTILITIES_MAINTENANCE_NYLONCLEANING_TEXTBUTTON, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADSKIP, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADGO, 0);
			bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_nyloncleaning);
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_MENU: // Move from Nylon Cleaning - > Main Menu
			screen_sdcard = false;
			surfing_utilities=false;
			
			SERIAL_PROTOCOLLNPGM("Main");
			
			surfing_temps = false;
			display_ChangeForm( FORM_MAIN, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTLEFT, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTRIGHT, 0);
			display.WriteObject(GENIE_OBJ_USERIMAGES, USERIMAGE_UTILITIES_MAINTENANCE_NYLONCLEANING_TEXTBUTTON, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADSKIP, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADGO, 0);
			bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_nyloncleaning);
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTLEFT: // Select Left Hotend
			
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTLEFT, 1);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTRIGHT, 0);
			display.WriteObject(GENIE_OBJ_USERIMAGES, USERIMAGE_UTILITIES_MAINTENANCE_NYLONCLEANING_TEXTBUTTON, 1);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADSKIP, 1);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADGO, 1);
			which_extruder = 0;
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTRIGHT: // Select Right Hotend
			
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTLEFT, 0);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SELECTRIGHT, 1);
			display.WriteObject(GENIE_OBJ_USERIMAGES, USERIMAGE_UTILITIES_MAINTENANCE_NYLONCLEANING_TEXTBUTTON, 1);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADSKIP, 1);
			display_ButtonState( BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADGO, 1);
			which_extruder = 1;
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADGO: // Start Nylon Cleaning;First, Unload Filament
			
			if(saved_print_flag==1888){
				saved_print_flag = 888;
				Config_StoreSettings();
			}
			if(which_extruder != 255){
				unload_get_ready();
			}
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_UNLOADSKIP: // Start Nylon Cleaning
			
			if(saved_print_flag==1888){
				saved_print_flag = 888;
				Config_StoreSettings();
			}
			if(which_extruder != 255){
				doblocking = true;
				setTargetHotend(NYLON_TEMP_HEATUP_THRESHOLD,which_extruder);
				gif_processing_state = PROCESSING_DEFAULT;
				display_ChangeForm(FORM_PROCESSING,0);
				
				if (home_made_Z){
					home_axis_from_code(true,true,false);
				}
				else{
					home_axis_from_code(true,true,true);
				}
				st_synchronize();
				ERROR_SCREEN_WARNING;
				current_position[Z_AXIS]=Z_MAX_POS-15;
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder); //check speed
				st_synchronize();
				ERROR_SCREEN_WARNING;
				current_position[Y_AXIS]=10;
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Y_AXIS], active_extruder); //check speed
				st_synchronize();
				ERROR_SCREEN_WARNING;
				
				if (which_extruder == 0) {
					changeTool(0);
					current_position[X_AXIS] = NOZZLE_PARK_DISTANCE_BED_X0 + 25;
				}
				else{
					changeTool(1);
					current_position[X_AXIS] = extruder_offset[X_AXIS][RIGHT_EXTRUDER] - 25 - NOZZLE_PARK_DISTANCE_BED_X0;
				}
				
				
				plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], XY_TRAVEL_SPEED15/60,which_extruder);
				st_synchronize();
				ERROR_SCREEN_WARNING;
				gif_processing_state = PROCESSING_STOP;
				touchscreen_update();
				display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP0,0);
				
			}
			
			break;
			
			case BUTTON_MAINTENANCE_ZADJUST_TOP: // Lift up Z 50mm
			if(!blocks_queued()){
				bitSet(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100up);
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10up);
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100down);
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10down);
			}
			break;
			
			case BUTTON_MAINTENANCE_ZADJUST_BOT: // Lift down Z 50mm
			if(!blocks_queued()){
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100up);
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10up);
				bitSet(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100down);
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10down);
			}
			break;
			
			case BUTTON_MAINTENANCE_ZADJUST_DOWN: // Lift down Z 10mm
			if(!blocks_queued()){
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100up);
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10up);
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100down);
				bitSet(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10down);
			}
			break;
			
			case BUTTON_MAINTENANCE_ZADJUST_UP: // Lift up Z 10mm
			if(!blocks_queued()){
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100up);
				bitSet(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10up);
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100down);
				bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10down);
			}
			break;
			
			case BUTTON_MAINTENANCE_ZADJUST_BACK:
			case BUTTON_MAINTENANCE_ZADJUST_ACCEPT:
			if(!blocks_queued()){
				processing_z_set = 255;
				touchscreen_update();
				display_ChangeForm( FORM_UTILITIES_MAINTENANCE, 0);
			}
			
			break;
			
			
			// Purge Buttons:
			case BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0:
			
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPUP,1);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPDOWN,1);
			if (purge_extruder_selected == 1){
				purge_extruder_selected = 0;
				if(target_temperature[0] == 0){
					setTargetHotend0(print_temp_l);
				}
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0,1);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1,0);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,target_temperature[0]);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_LEFTTARGET,int(degHotend(0)));
			}
			else{
				purge_extruder_selected = 0;
				if(target_temperature[0] == 0){
					setTargetHotend0(print_temp_l);
				}
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0,1);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1,0);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,target_temperature[0]);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_LEFTTARGET,int(degHotend(0)));
			}
			
			break;
			case BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1:
			
			
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_PURGE_INSERT,1);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_PURGE_Retract,1);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPUP,1);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPDOWN,1);
			if (purge_extruder_selected == 0){
				purge_extruder_selected = 1;
				
				if(target_temperature[1] == 0){
					setTargetHotend1(print_temp_r);
				}
				
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0,0);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1,1);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,target_temperature[1]);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_RIGHTTARGET,int(degHotend(1)));
			}
			else{
				purge_extruder_selected = 1;
				if(target_temperature[1] == 0){
					setTargetHotend1(print_temp_r);
				}
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0,0);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1,1);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,target_temperature[1]);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_RIGHTTARGET,int(degHotend(1)));
			}
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_TEMPUP:
			if(purge_extruder_selected != -1){
				if (target_temperature[purge_extruder_selected] < HEATER_0_MAXTEMP){
					target_temperature[purge_extruder_selected] += 5;
					setTargetHotend0(target_temperature[0]);
					setTargetHotend1(target_temperature[1]);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,int(target_temperature[purge_extruder_selected]));
				}
			}
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_TEMPDOWN:
			if(purge_extruder_selected != -1){
				if (target_temperature[purge_extruder_selected] > 0){
					target_temperature[purge_extruder_selected] -= 5;
					setTargetHotend0(target_temperature[0]);
					setTargetHotend1(target_temperature[1]);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,int(target_temperature[purge_extruder_selected]));
				}
			}
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_LOAD:
			if(purge_extruder_selected != -1){
				if(!blocks_queued()){
					bitSet(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect0);
					}else{
					quickStop();
				}
			}
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD:
			if(purge_extruder_selected != -1){
				if(!blocks_queued()){
					bitSet(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect1);
					}else{
					quickStop();
				}
			}
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE:
			
			is_purging_filament = true;
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_LOAD,0);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD,0);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPUP,0);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_TEMPDOWN,0);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_MENU,0);
			display_ChangeForm(FORM_UTILITIES_FILAMENT_PURGE,0);
			
			purge_extruder_selected = -1;
			/*setTargetHotend0(print_temp_l);
			setTargetHotend1(print_temp_r);*/
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT0,0);
			display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_SELECT1,0);
			
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_RIGHTTARGET,int(degHotend(1)));
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_LEFTTARGET,int(degHotend(0)));
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_SELECTEDTEMP,0);
			
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_BACK:
			
			is_purging_filament = false;
			quickStop();
			HeaterCooldownInactivity(true);
			touchscreen_update();
			display_ChangeForm(FORM_UTILITIES_FILAMENT,0);
			
			//setTargetHotend0(0);
			//setTargetHotend1(0);
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_PURGE_MENU:
			
			is_purging_filament = false;
			quickStop();
			screen_sdcard = false;
			surfing_utilities=false;
			surfing_temps = false;
			
			SERIAL_PROTOCOLLNPGM("Main");
			
			HeaterCooldownInactivity(true);
			touchscreen_update();
			display_ChangeForm(FORM_MAIN,0);
			
			break;
			
			// end Purge Buttons
			
			
			//*****INSERT/REMOVE FILAMENT*****
			
			
			case BUTTON_UTILITIES_FILAMENT_BACK:
			bitClear(flag_utilities_maintenance_register,flag_utilities_filament_register_home);
			display_ChangeForm(FORM_UTILITIES,0);
			
			break;
			case BUTTON_UTILITIES_FILAMENT_UNLOAD:
			
			filament_mode = 'R';//Remove Mode
			which_extruder = 255;
			display_ChangeForm(FORM_UTILITIES_FILAMENT_UNLOAD,0);
			
			break;
			case BUTTON_UTILITIES_FILAMENT_LOAD:
			
			filament_mode = 'I'; //Insert Mode
			
			/*if (!FLAG_FilamentHome){
			genie.WriteObject(GENIE_OBJ_FORM,FORM_WAITING_ROOM,0);
			
			}*/
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1, 0);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0, 0);
			
			
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 0);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 0);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 0);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILNEXT, 0);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILBACK, 0);
			display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD,0);
			//display.WriteObject(GENIE_OBJ_USERIMAGES,USERBUTTON_UTILITIES_FILAMENT_LOAD_FIL1,0);
			//display.WriteObject(GENIE_OBJ_USERIMAGES,USERBUTTON_UTILITIES_FILAMENT_LOAD_FIL2,0);
			//display.WriteObject(GENIE_OBJ_USERIMAGES,USERBUTTON_UTILITIES_FILAMENT_LOAD_FIL3,0);
			//display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"PLA");
			//display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL2,"PVA");
			//display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL3,"PET-G");
			
			which_extruder = 255;
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_UNLOAD_SELECTLEFT:
			case BUTTON_UTILITIES_FILAMENT_UNLOAD_SELECTRIGHT:
			
			if (LCD_FSM_input_buton_flag == BUTTON_UTILITIES_FILAMENT_UNLOAD_SELECTLEFT) //Left Nozzle
			{
				
				which_extruder=0;
				
			}
			else //Right Nozzle
			{
				
				which_extruder=1;
			}
			unload_get_ready();
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_BACK:
			if (!Step_First_Start_Wizard){
				
				if(flag_utilities_maintenance_changehotend == 888)display_ChangeForm(FORM_UTILITIES_FILAMENT,0);
				else display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_LOADFILAMENT,0);
				
			}
			break;
			
			case BUTTON_UTILITIES_FILAMENT_UNLOAD_BACK:
			
			
			display_ChangeForm(FORM_UTILITIES_FILAMENT,0);
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_UNLOAD_MENU:
			
			screen_sdcard = false;
			surfing_utilities=false;
			surfing_temps = false;
			
			SERIAL_PROTOCOLLNPGM("Main");
			
			display_ChangeForm(FORM_MAIN,0);			

			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_MENU:
			if(!Step_First_Start_Wizard){
				
				if(flag_utilities_maintenance_changehotend == 888){
					screen_sdcard = false;
					surfing_utilities=false;
					surfing_temps = false;
					
					SERIAL_PROTOCOLLNPGM("Main");
					
					display_ChangeForm(FORM_MAIN,0);
				}
			}
			break;

			case BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0:
			case BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1:
			if(!Step_First_Start_Wizard && flag_utilities_maintenance_changehotend != 1888 && flag_utilities_maintenance_changehotend != 2888 ){
				if (LCD_FSM_input_buton_flag == BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0) //Left Nozzle
				{
					which_extruder=0;
				}
				else //Right Nozzle
				{
					which_extruder=1;
				}
				if (filament_mode == 'I') {
					if (which_extruder == 0){
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0, 1);
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1, 0);
					}
					else if(which_extruder == 1){
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1, 1);
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0, 0);
						
					}
					cycle_filament = 0;
					display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_FILAMENT_LOAD_FIL1,0);
					display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 1);
					display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 1);
					display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 1);
					
					//display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_FILAMENT_LOAD_FIL1,0);
					//display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_FILAMENT_LOAD_FIL2,0);
					//display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_FILAMENT_LOAD_FIL3,0);
					display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILNEXT, 1);
					display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILBACK, 1);
					display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"    PLA");
					display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL2,"    PVA");
					display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL3,"  PET-G");
					
					
					}else{
					//*********Move the bed down and the extruders inside
					if(which_extruder == 0) setTargetHotend(unload_temp_l,which_extruder);
					else setTargetHotend(unload_temp_r,which_extruder);
					gif_processing_state = PROCESSING_DEFAULT;
					display_ChangeForm(FORM_PROCESSING,0);
					
					if (home_made_Z){
						home_axis_from_code(true,true,false);
					}
					else{
						home_axis_from_code(true,true,true);
					}
					
					
					current_position[Z_AXIS]=Z_MAX_POS-15;
					plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder); //check speed
					
					current_position[Y_AXIS]=10;
					plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Y_AXIS], active_extruder); //check speed
					st_synchronize();
					ERROR_SCREEN_WARNING;
					
					gif_processing_state = PROCESSING_STOP;
					touchscreen_update();
					//genie.WriteObject(GENIE_OBJ_USERIMAGES,10,1);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, 0);
					display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
					Tref1 = (int)degHotend(which_extruder);
					Tfinal1 = (int)degTargetHotend(which_extruder)-CHANGE_FIL_TEMP_HYSTERESIS;
					Tpercentage_old = 0;
					/****************************************************/
					
					//ATTENTION : Order here is important
					
					
					//Serial.println("REMOVING");
					//genie.WriteStr(STRING_ADVISE_FILAMENT,"");
					/*if (filament_mode == 'I') genie.WriteObject(GENIE_OBJ_USERIMAGES,10,0);
					else if (filament_mode == 'R') genie.WriteObject(GENIE_OBJ_USERIMAGES,10,1);
					else genie.WriteObject(GENIE_OBJ_USERIMAGES,10,1);*/
					gif_processing_state = PROCESSING_ADJUSTING;
					//delay(3500);
					/*if(which_extruder == 0) setTargetHotend(max(remove_temp_l,old_remove_temp_l),which_extruder);
					else setTargetHotend(max(remove_temp_r,old_remove_temp_r),which_extruder);*/
					touchscreen_update();
					is_changing_filament=true; //We are changing filament
					
				}
				
				
			}
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_FILNEXT:
			go_loadfilament_next();
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_FILBACK:
			go_loadfilament_back();
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_FIL1:
			go_loadfilament(1);
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_FIL2:
			go_loadfilament(2);
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_FIL3:
			go_loadfilament(3);
			break;
			
			////CUSTOM MATERIAL BUTTONS
			//case BUTTON_UTILITIES_FILAMENT_LOAD_CUSTOM:
			//
			//
			//if (which_extruder == 1 || which_extruder == 0) // Need to pause
			//{
			//if(Step_First_Start_Wizard){
			////genie.WriteObject(GENIE_OBJ_USERBUTTON, BUTTON_CUSTOM_MENU, 1);
			//}
			//
			//display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD_CUSTOM,0);
			//
			//
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_LOAD,custom_load_temp);
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_UNLOAD,custom_unload_temp);
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_PRINT,custom_print_temp);
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_BED,custom_bed_temp);
			//}
			//
			//break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_CUSTOM_BACK:
			
			
			if(Step_First_Start_Wizard || flag_utilities_maintenance_changehotend == 1888 || flag_utilities_maintenance_changehotend == 2888){
				if(which_extruder == 0){
					display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1, 0);
					display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0, 1);
					}else if (which_extruder == 1){
					display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1, 1);
					display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0, 0);
				}
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_FILAMENT_LOAD_FIL1,0);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILNEXT, 1);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILBACK, 1);
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_MENU,1);
				if(flag_utilities_maintenance_changehotend == 1888 || flag_utilities_maintenance_changehotend == 2888){
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_BACK,0);
					}else{
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_BACK,1);
				}
				
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 1);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 1);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 1);
				display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD,0);
				cycle_filament = 0;
				display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"    PLA");
				display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL2,"    PVA");
				display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL3,"  PET-G");
				}else{
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1, 0);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0, 0);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_FILAMENT_LOAD_FIL1,0);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 0);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 0);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 0);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILNEXT, 0);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILBACK, 0);
				display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD,0);
				which_extruder = 255;
			}
			
			
			
			
			
			break;
			
			
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_CUSTOM_PRINT_LESS:
			if (custom_print_temp > 0){
				custom_print_temp -= 5;
				custom_unload_temp -= 5;
				custom_load_temp -= 5;
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_PRINT,custom_print_temp);
			}
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_CUSTOM_PRINT_MORE:
			if (custom_print_temp < HEATER_0_MAXTEMP-5){
				custom_print_temp += 5;
				custom_unload_temp += 5;
				custom_load_temp += 5;
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_PRINT,custom_print_temp);
			}
			break;
			case BUTTON_UTILITIES_FILAMENT_LOAD_CUSTOM_BED_LESS:
			if (custom_bed_temp > 0){
				custom_bed_temp -= 5;
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_BED,custom_bed_temp);
			}
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_CUSTOM_BED_MORE:
			if (custom_bed_temp < BED_MAXTEMP -5){
				custom_bed_temp += 5;
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_BED,custom_bed_temp);
			}
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_CUSTOM_ACCEPT:
			display_ChangeForm(FORM_PROCESSING,0);
			saved_print_flag = 888;
			if(which_extruder == 0){
				if (custom_print_temp <= HEATER_0_MAXTEMP) print_temp_l = custom_print_temp;
				else print_temp_l = HEATER_0_MAXTEMP;
				if (custom_load_temp <= HEATER_0_MAXTEMP) load_temp_l = custom_load_temp;
				else load_temp_l = HEATER_0_MAXTEMP;
				if (custom_unload_temp <= HEATER_0_MAXTEMP) unload_temp_l = custom_unload_temp;
				else unload_temp_l = HEATER_0_MAXTEMP;
				if (custom_bed_temp <= BED_MAXTEMP) bed_temp_l = custom_bed_temp;
				else bed_temp_l = BED_MAXTEMP;
				setTargetHotend0(load_temp_l);
			}
			else{
				if (custom_print_temp <= HEATER_1_MAXTEMP)print_temp_r = custom_print_temp;
				else print_temp_r = HEATER_1_MAXTEMP;
				if (custom_load_temp <= HEATER_1_MAXTEMP)load_temp_r = custom_load_temp;
				else print_temp_r = HEATER_1_MAXTEMP;
				if (custom_unload_temp <= HEATER_1_MAXTEMP)unload_temp_r = custom_unload_temp;
				else unload_temp_r = HEATER_1_MAXTEMP;
				if (custom_bed_temp <= BED_MAXTEMP)bed_temp_r = custom_bed_temp;
				else bed_temp_r = BED_MAXTEMP;
				setTargetHotend1(load_temp_r);
			}
			#ifdef DEV_BER
			Config_StoreSettings();
			#endif
			insertmetod();
			
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_FEELSTOPS:
			display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD_HANDS,0);
			break;
			
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_HANDS:
			//ATTENTION : Order here is important
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, 0);
			display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
			Tref1 = (int)degHotend(which_extruder);
			Tfinal1 = (int)degTargetHotend(which_extruder)-CHANGE_FIL_TEMP_HYSTERESIS;
			Tpercentage_old = 0;
			//genie.WriteStr(STRING_ADVISE_FILAMENT,"");
			//genie.WriteStr(STRING_ADVISE_FILAMENT,"Insert the filament until you feel it stops, \n then while you keep inserting around \n 10 mm of filament, press the clip");
			gif_processing_state = PROCESSING_ADJUSTING;
			
			if (which_extruder==0) setTargetHotend(max(load_temp_l,old_load_temp_l),which_extruder);
			else setTargetHotend(max(load_temp_r,old_load_temp_r),which_extruder);
			//delay(3500);
			
			
			if (which_extruder == 0) changeTool(0);
			else changeTool(1);
			
			current_position[Y_AXIS] = 100;
			plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], XY_TRAVEL_SPEED15/60,which_extruder);
			st_synchronize();
			ERROR_SCREEN_WARNING;
			touchscreen_update();
			is_changing_filament=true;
			break;
			
			case BUTTON_UTILITIES_FILAMENT_LOAD_KEEPPUSHING_NEXT:
			if (!Flag_FRS_enabled || (Flag_FRS_enabled && CHECK_FRS(which_extruder)))
			{ //Inserting...
				is_checking_filament = false;
				display_ChangeForm(FORM_PROCESSING,0);
				gif_processing_state = PROCESSING_DEFAULT;
				delay(550);
				#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
				current_position[X_AXIS] = 155;
				#else
				current_position[X_AXIS] = 155 + X_OFFSET_CALIB_PROCEDURES;
				#endif
				plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], XY_TRAVEL_SPEED15/60,which_extruder);
				st_synchronize();
				//SERIAL_PROTOCOLPGM("Loading\n");
				doblocking = false;
				current_position[E_AXIS] += 30;//Extra extrusion at low feedrate
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS],  700/60, which_extruder); //850/60
				current_position[E_AXIS] += ((BOWDEN_LENGTH-EXTRUDER_LENGTH)-15);//BOWDEN_LENGTH-300+340);
				st_synchronize();
				Serial.println(current_position[E_AXIS]);
				if (axis_steps_per_unit[E_AXIS]<=512 && axis_steps_per_unit[E_AXIS]>=492){
					plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_FAST_R19_SPEED/60, which_extruder);
					}else{
					plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_FAST_SPEED/60, which_extruder);
				}
				st_synchronize();
				current_position[E_AXIS] += EXTRUDER_LENGTH;//Extra extrusion at low feedrate
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS],  INSERT_SLOW_SPEED/60, which_extruder);
				st_synchronize();
				
				ERROR_SCREEN_WARNING;
				gif_processing_state = PROCESSING_STOP;
				display_ChangeForm(FORM_UTILITIES_FILAMENT_ADJUST,0);
			}
			
			break;
			
			
			case BUTTON_UTILITIES_FILAMENT_UNLOAD_ROLLTHESPOOL_NEXT:
			unloadfilament_procedure();
			
			break;
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP0:
			display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
			gif_processing_state = PROCESSING_ADJUSTING;
			Tref = (int)degHotend(which_extruder);
			Tfinal = (int)(degTargetHotend(which_extruder)-NYLON_TEMP_HYSTERESIS);
			percentage = 0;
			while (degHotend(which_extruder)<(degTargetHotend(which_extruder)-NYLON_TEMP_HYSTERESIS)){ //Waiting to heat the extruder
				if (millis() >= waitPeriod_s){
					int Tinstant;
					memset(buffer, '\0', sizeof(buffer) );
					Tinstant = constrain((int)degHotend(which_extruder), Tref, Tfinal);
					percentage = Tfinal-Tref;
					percentage = 100*(Tinstant-Tref)/percentage;
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, percentage);
					waitPeriod_s=500+millis();
				}
				manage_heater();
				touchscreen_update();
				ERROR_SCREEN_WARNING;
			}
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP2,0);
			
			
			break;
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP2:
			setTargetHotend(0.0,which_extruder);
			if(which_extruder == 0)digitalWrite(FAN_PIN, 1);
			else digitalWrite(FAN2_PIN, 1);
			display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP3,0);
			//gif_processing_state = PROCESSING_UTILITIES_MAINTENANCE_NYLONCLEANING_TEMPS;
			gif_processing_state = PROCESSING_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP3;
			Tref = (int)degHotend(which_extruder);
			Tfinal = 160;
			percentage = 0;
			while (degHotend(which_extruder)>160.0){ //Waiting to heat the extruder
				//previous_millis_cmd = millis();
				memset(buffer, '\0', sizeof(buffer) );
				if (millis() >= waitPeriod_s){
					int Tinstant;
					Tinstant = constrain((int)degHotend(which_extruder), Tfinal, Tref);
					percentage = ((Tref-Tfinal)-(Tinstant-Tfinal))*100;
					percentage = percentage/(Tref-Tfinal);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP3, percentage);
					waitPeriod_s=500+millis();
				}
				manage_heater();
				touchscreen_update();
				ERROR_SCREEN_WARNING;
			}
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			fanSpeed=255;
			printer_state = STATE_NYLONCLEANING;
			
			display_ButtonState(BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP4,1);
			
			
			
			display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP4,0);
			gif_processing_state = PROCESSING_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP4;
			
			break;
			
			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP5:


			display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP6,0);

			break;

			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_REPEAT:


			setTargetHotend(NYLON_TEMP_HEATUP_THRESHOLD,which_extruder);

			display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
			gif_processing_state = PROCESSING_ADJUSTING;
			Tref = (int)degHotend(which_extruder);
			Tfinal = (int)(degTargetHotend(which_extruder)-NYLON_TEMP_HYSTERESIS);
			percentage = 0;
			while (degHotend(which_extruder)<(degTargetHotend(which_extruder)-NYLON_TEMP_HYSTERESIS)){ //Waiting to heat the extruder
				if (millis() >= waitPeriod_s){
					memset(buffer, '\0', sizeof(buffer) );
					int Tinstant;
					Tinstant = constrain((int)degHotend(which_extruder), Tref, Tfinal);
					percentage = Tfinal-Tref;
					percentage = 100*(Tinstant-Tref)/percentage;
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, percentage);
					waitPeriod_s=500+millis();
				}
				manage_heater();
				touchscreen_update();
				ERROR_SCREEN_WARNING;
			}
			gif_processing_state = PROCESSING_STOP;

			display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP2,0);

			break;

			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP7_SUCCESS:
			doblocking = false;
			setTargetHotend0(0);
			setTargetHotend1(0);
			HeaterCooldownInactivity(true);
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			home_axis_from_code(true, true, false);
			gif_processing_state = PROCESSING_STOP;
			display_ChangeForm(FORM_UTILITIES_MAINTENANCE,0);
			bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_nyloncleaning);
			break;

			
			case BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_SUCCESS:
			display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP7,0);
			break;



			//*****AdjustFilament******

			case BUTTON_UTILITIES_FILAMENT_ADJUST_ACCEPT:
			if(!bitRead(flag_utilities_filament_register,flag_utilities_filament_register_acceptok)){
				
				
				if(!blocks_queued()){
					bitSet(flag_utilities_filament_register,flag_utilities_filament_register_acceptok);
					if(which_extruder == 0){
						setTargetHotend0(print_temp_l);
					}
					else{
						setTargetHotend1(print_temp_r);
					}
					display_ChangeForm(FORM_PROCESSING,0);
					home_made = false;
					gif_processing_state = PROCESSING_DEFAULT;
					home_axis_from_code(true,true,false);
					//genie.WriteObject(GENIE_OBJ_FORM,FORM_SUCCESS_FILAMENT,0);
					}else{
					quickStop();
				}
				
				
				
			}
			break;

			case BUTTON_UTILITIES_FILAMENT_ADJUST_LOAD:
			if(!bitRead(flag_utilities_filament_register,flag_utilities_filament_register_acceptok)){
				if(!blocks_queued()){
					bitSet(flag_utilities_filament_register,flag_utilities_filament_register_purgeload);
					}else{
					quickStop();
				}
				
			}
			break;

			case BUTTON_UTILITIES_FILAMENT_ADJUST_UNLOAD:
			if(!bitRead(flag_utilities_filament_register,flag_utilities_filament_register_acceptok)){
				if(!blocks_queued()){
					bitSet(flag_utilities_filament_register,flag_utilities_filament_register_purgeunload);
					}else{
					quickStop();
				}
			}
			break;


			//Extruder Calibrations-------------------------------------------------
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_NOTICE_BACK:
			case BUTTON_UTILITIES_CALIBRATION_CALIBBED_NOTICE_BACK:
			case BUTTON_UTILITIES_CALIBRATION_MANUALFINE_NOTICE_BACK:
			if(!Step_First_Start_Wizard){
				display_ChangeForm(FORM_UTILITIES_CALIBRATION,0);
			}
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_CALIBRATION_GO:
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_NOTICE_NEXT:
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_NOTICE_BACK,0);
			bed_calibration_times = 0;
			if(saved_print_flag==1888){
				saved_print_flag = 888;
				Config_StoreSettings();
			}
			bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_calibfull);
			//enquecommand_P(PSTR("T0"));
			if(!bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_calibbeddone)){  //Do g34
				display_ChangeForm(FORM_PROCESSING,0);
				gif_processing_state = PROCESSING_DEFAULT;
				doblocking= true;
				home_axis_from_code(true,true,true);
				st_synchronize();
				ERROR_SCREEN_WARNING;
				enquecommand_P(PSTR("G34"));	//Start BED Calibration Wizard
				changeTool(0);
				
				
			}
			else{
				
				active_extruder = LEFT_EXTRUDER;
				display_ChangeForm(FORM_PROCESSING,0);
				gif_processing_state = PROCESSING_DEFAULT;
				setTargetHotend0(CALIBFULL_HOTEND_STANDBY_TEMP);
				setTargetHotend1(CALIBFULL_HOTEND_STANDBY_TEMP);
				setTargetBed(max(bed_temp_l,bed_temp_r));
				
				
				home_axis_from_code(true,true,false);
				st_synchronize();
				ERROR_SCREEN_WARNING;
				enquecommand_P(PSTR("T0"));
				gif_processing_state = PROCESSING_STOP;
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZL,0);
				if(Step_First_Start_Wizard){
					display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZL_SKIP,1);
				}
				
			}
			break;
			
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL:
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_NOTICE,0);
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBBED:
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBBED_NOTICE,0);
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBMANUALFINE:
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_MANUALFINE_NOTICE,0);
			break;
			
			
			case BUTTON_UTILITIES_CALIBRATION_MANUALFINE_NOTICE_NEXT:

			offset_calib_manu[0]=0.0;
			offset_calib_manu[1]=0.0;
			offset_calib_manu[2]=0.0;
			offset_calib_manu[3]=0.0;
			calib_value_selected = 0;
			sprintf(buffer, "0.000");

			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_MANUAL,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_ZR,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_ZL,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_X,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_Y,0);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_UTILITIES_CALIBRATION_MANUAL_RIGHT,0);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_UTILITIES_CALIBRATION_MANUAL_LEFT,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_UP,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_DOWN,1);
			display_ChangeForm( FORM_UTILITIES_CALIBRATION_MANUAL,0);
			display.WriteStr(STRING_UTILITIES_CALIBRATION_MANUAL,manual_fine_calib_offset[0],3);
			touchscreen_update();
			break;


			//*****Bed Calibration*****

			case BUTTON_UTILITIES_CALIBRATION_CALIBBED_NOTICE_NEXT:

			if(saved_print_flag==1888){
				saved_print_flag = 888;
				Config_StoreSettings();
			}
			doblocking = true;
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			bed_calibration_times = 0;
			bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_calibfull);
			home_axis_from_code(true,true,true);
			enquecommand_P(PSTR("G34"));	//Start BED Calibration Wizard
			changeTool(0);

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBBED_SCREW3_NEXT:

			doblocking = true;
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			home_axis_from_code(true,true,false);
			changeTool(0);
			enquecommand_P((PSTR("G34")));
			bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_calibbeddone);

			break;


			case BUTTON_UTILITIES_CALIBRATION_CALIBBED_SCREW2_NEXT:
			if (vuitens3!=0){
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBBED_SCREW3,0);
				sprintf(buffer, " %d / 8",vuitens3); //Printing how to calibrate on screen
				//genie.WriteStr(STRING_BED_SCREW3,buffer);
				if (sentit3>0){display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBBED_SCREW3,vuitens3);} //The direction is inverted in Sigma's bed screws
				else{display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBBED_SCREW3,vuitens3+8);}
				}else{
				doblocking = true;
				display_ChangeForm(FORM_PROCESSING,0);
				gif_processing_state = PROCESSING_DEFAULT;
				home_axis_from_code(true,true,false);
				changeTool(0);
				enquecommand_P((PSTR("G34")));
				bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_calibbeddone);
			}

			break;
			case BUTTON_UTILITIES_CALIBRATION_CALIBBED_ADJUSTSCREWASK_NEXT:
			case BUTTON_UTILITIES_CALIBRATION_CALIBBED_ADJUSTSCREWASKFIRST_NEXT:


			gif_processing_state = PROCESSING_STOP;
			/*if (vuitens1!= 0){
			genie.WriteObject(GENIE_OBJ_FORM,FORM_CALIB_BED_SCREW1,0);
			sprintf(buffer, " %d / 8",vuitens1); //Printing how to calibrate on screen
			//genie.WriteStr(STRING_BED_SCREW1,buffer);
			if (vuitens2==0 && vuitens3==0) {genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_BED_CALIB_SW2,0);}
			else{genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_BED_CALIB_SW2,0);}
			if (sentit1>0){genie.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_SCREW1,vuitens1);} //The direction is inverted in Sigma's bed screws
			else{genie.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_SCREW1,vuitens1+8);}
			}*/
			if (vuitens2!= 0){
				SERIAL_PROTOCOLPGM("Jump over screw1 \n");
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBBED_SCREW2,0);
				sprintf(buffer, " %d / 8",vuitens2); //Printing how to calibrate on screen
				//genie.WriteStr(STRING_BED_SCREW2,buffer);
				if (vuitens3==0) display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBBED_SCREW2_NEXT,0);
				if (sentit2>0){display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBBED_SCREW2,vuitens2);} //The direction is inverted in Sigma's bed screws
				else{display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBBED_SCREW2,vuitens2+8);}
			}
			else if (vuitens3!= 0){
				SERIAL_PROTOCOLPGM("Jump over screw1 and screw2 \n");
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBBED_SCREW3,0);
				sprintf(buffer, " %d / 8",vuitens3); //Printing how to calibrate on screen
				//genie.WriteStr(STRING_BED_SCREW3,buffer);
				if (sentit3>0){display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBBED_SCREW3,vuitens3);} //The direction is inverted in Sigma's bed screws
				else{display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBBED_SCREW3,vuitens3+8);}
			}

			break;


			//*****Success Screens*****

			case BUTTON_UTILITIES_CALIBRATION_SUCCESS: // or BUTTON_SUCCESS_FILAMENT_OK
			if (printer_state == STATE_CALIBRATION){
				
				gif_processing_state = PROCESSING_STOP;
				if(bitRead(flag_utilities_maintenance_register,flag_utilities_maintenance_register_autotune)){
					display_ChangeForm(FORM_UTILITIES_MAINTENANCE,0);
					bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_autotune);
				}
				else{
					
					//enquecommand_P((PSTR("G28 X0 Y0")));
					display_ChangeForm(FORM_PROCESSING,0);
					gif_processing_state = PROCESSING_DEFAULT;
					setTargetHotend0(0);
					setTargetHotend1(0);
					setTargetBed(0);
					home_axis_from_code(true, true, false);
					enquecommand_P((PSTR("T0")));
					st_synchronize();
					ERROR_SCREEN_WARNING;
					SERIAL_PROTOCOLPGM("Calibration Successful\n");
					gif_processing_state = PROCESSING_STOP;
					
					display_ChangeForm(FORM_UTILITIES_CALIBRATION,0);
					bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_calibbeddone);
					doblocking=false;
				}
				printer_state = STATE_NONE;
				
				
			}
			else if(printer_state == STATE_LOADUNLOAD_FILAMENT){
				
				gif_processing_state = PROCESSING_STOP;
				if(Step_First_Start_Wizard){
					if(which_extruder == 0){
						enquecommand_P((PSTR("T0")));
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1, 1);
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0, 0);
						display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_MENU,1);
						display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_BACK,1);
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 1);
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 1);
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 1);
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILNEXT, 1);
						display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILBACK, 1);
						display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD,0);
						display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"    PLA");
						display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL2,"    PVA");
						display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL3,"  PET-G");
						
						which_extruder = 1;
					}
					else if (which_extruder == 1){
						enquecommand_P((PSTR("T0")));
						display_ChangeForm(FORM_SETUPASSISTANT_STEP_2,0);
						which_extruder = 0;
					}
				}
				else if(bitRead(flag_utilities_maintenance_register,flag_utilities_maintenance_register_changehotend)){
					if(which_extruder == LEFT_EXTRUDER){
						flag_utilities_maintenance_changehotend = 1888;
						}else if(which_extruder == RIGHT_EXTRUDER){
						flag_utilities_maintenance_changehotend = 2888;
					}
					bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_cooldown_mode1);
					
					bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_changehotend);
				}
				else if(!bitRead(flag_utilities_maintenance_register,flag_utilities_maintenance_register_nyloncleaning)){
					enquecommand_P((PSTR("T0")));
					if(flag_utilities_maintenance_changehotend == 1888 || flag_utilities_maintenance_changehotend == 2888){
						display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_AUTOTUNE,0);
						}else{
						display_ChangeForm(FORM_UTILITIES_FILAMENT,0);
						#ifdef DEV_BER
						enquecommand_P(PSTR("M841"));
						Config_StoreSettings();
						#endif
						HeaterCooldownInactivity(true);
					}
					
				}
				else{
					display_ChangeForm(FORM_PROCESSING,0);
					gif_processing_state = PROCESSING_DEFAULT;
					setTargetHotend0(0);
					setTargetHotend1(0);
					home_axis_from_code(true, true, false);
					setTargetHotend(NYLON_TEMP_HEATUP_THRESHOLD,which_extruder);
					
					current_position[Y_AXIS] = 10;
					
					if (which_extruder == 0) {
						changeTool(0);
						current_position[X_AXIS] = NOZZLE_PARK_DISTANCE_BED_X0 + 25;
					}
					else{
						changeTool(1);
						current_position[X_AXIS] = extruder_offset[X_AXIS][RIGHT_EXTRUDER] - 25 - NOZZLE_PARK_DISTANCE_BED_X0;
					}
					doblocking = true;
					plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], XY_TRAVEL_SPEED15/60,which_extruder);
					st_synchronize();
					ERROR_SCREEN_WARNING;
					gif_processing_state = PROCESSING_STOP;
					display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP0,0);
				}
				bitClear(flag_utilities_filament_register,flag_utilities_filament_register_acceptok);
				printer_state = STATE_NONE;
				
			}
			
			else if(printer_state == STATE_NYLONCLEANING){
				
				display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
				
				display_ButtonState(BUTTON_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP4,0);
				
				if(which_extruder == 0)digitalWrite(FAN_PIN, 1);
				else digitalWrite(FAN2_PIN, 1);
				gif_processing_state = PROCESSING_ADJUSTING;
				Tref = (int)degHotend(which_extruder);
				Tfinal = NYLON_TEMP_COOLDOWN_THRESHOLD;
				percentage = 0;
				while (degHotend(which_extruder)>Tfinal){ //Waiting to heat the extruder
					if (millis() >= waitPeriod_s){
						memset(buffer, '\0', sizeof(buffer) );
						int Tinstant;
						if(Tref < (int)degHotend(which_extruder)){
							Tinstant = Tref;
							}else if((int)degHotend(which_extruder) < Tfinal){
							Tinstant = Tfinal;
							}else{
							Tinstant = (int)degHotend(which_extruder);
						}
						
						percentage = ((Tref-Tfinal)-(Tinstant-Tfinal))*90; //<<<<<<<<<<<<<  0% TO 90%
						percentage = percentage/(Tref-Tfinal);
						display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES,percentage);
						waitPeriod_s=500+millis();
					}
					//previous_millis_cmd = millis();
					manage_heater();
					touchscreen_update();
					ERROR_SCREEN_WARNING;
					
				}
				SERIAL_PROTOCOLPGM("60 degrees \n");
				fanSpeed=0;
				if(which_extruder == 0)digitalWrite(FAN_PIN, 0);
				else digitalWrite(FAN2_PIN, 0);
				setTargetHotend(105.0,which_extruder);
				Tref = (int)degHotend(which_extruder);
				Tfinal = 105-NYLON_TEMP_HYSTERESIS;
				percentage = 0;
				while (degHotend(which_extruder)<(degTargetHotend(which_extruder)-NYLON_TEMP_HYSTERESIS)){ //Waiting to heat the extruder
					
					if (millis() >= waitPeriod_s){
						char buffer[25];
						memset(buffer, '\0', sizeof(buffer) );
						
						int Tinstant;
						if(Tref > (int)degHotend(which_extruder)){
							Tinstant = Tref;
							}else if((int)degHotend(which_extruder) > Tfinal){
							Tinstant = Tfinal;
							}else{
							Tinstant = (int)degHotend(which_extruder);
						}
						percentage = Tfinal-Tref;
						percentage = 90+ 10*(Tinstant-Tref)/percentage;//<<<<<<<<<<<<<  90% TO 100%
						display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES,percentage);
						waitPeriod_s=500+millis();
					}
					manage_heater();
					touchscreen_update();
					ERROR_SCREEN_WARNING;
					
				}
				gif_processing_state = PROCESSING_STOP;
				display_ChangeForm(FORM_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP5,0);
				printer_state = STATE_NONE;
				
			}
			else if(printer_state == STATE_AUTOTUNEPID){
				if(flag_utilities_maintenance_changehotend == 3888){
					display_ChangeForm( FORM_PRINTERSETUP_CHANGEHOTEND_CALIBRATION, 0);
					flag_utilities_maintenance_changehotend = 888;
					Config_StoreSettings();
					printer_state = STATE_NONE;
				}
			}
			
			break;


			//***** Calibration XYZ *****
			
			

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDO_X_LEFT:
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);
			redo_source = 1;
			calculus = extruder_offset[X_AXIS][1] + 0.5;
			SERIAL_PROTOCOLPGM("Calculus:  ");
			Serial.println(calculus);
			extruder_offset[X_AXIS][RIGHT_EXTRUDER]=calculus;
			Config_StoreSettings(); //Store changes
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDO_X_RIGHT:
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);
			redo_source = 1;
			calculus = extruder_offset[X_AXIS][1] -0.4;
			SERIAL_PROTOCOLPGM("Calculus:  ");
			Serial.println(calculus);
			extruder_offset[X_AXIS][RIGHT_EXTRUDER]=calculus;
			Config_StoreSettings(); //Store changes
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDO_X:
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);
			redo_source = 1;
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT1:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT1,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,1);
			calib_confirmation = 5;
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT2:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT2,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,1);
			calib_confirmation = 4;
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT3:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT3,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,1);
			calib_confirmation = 3;
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT4:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT4,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,1);
			calib_confirmation = 2;

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT5:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT5,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,1);
			calib_confirmation = 1;

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT6:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT6,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,1);
			calib_confirmation = 0;

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT7:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT7,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,1);
			calib_confirmation = -1;

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT8:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT8,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,1);
			calib_confirmation = -2;

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT9:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT9,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,1);
			calib_confirmation = -3;
			
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT10:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT10,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,1);
			calib_confirmation = -4;
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_REDO:
			if(calib_zxy_id == 1){
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_REDO_X,0);
				}else if(calib_zxy_id == 2){
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_REDO_Y,0);
			}
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDO_X_BACK:
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDO_Y_BACK:
			turnoff_buttons_xycalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM,0);
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY,0);
			calib_confirmation = 1888;
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOZL_BACK:
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOZR_BACK:
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,0);
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,0);
			calib_confirmation = 1888;
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_CONFIRM:
			if(calib_confirmation <= 5 && calib_confirmation >= -4){
				if(calib_zxy_id == 1){
					Full_calibration_X_set((float)calib_confirmation/10.0);
					}else if(calib_zxy_id == 2){
					Full_calibration_Y_set((float)calib_confirmation/10.0);
				}
			}
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOG36_BACK:
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,0);
			calib_confirmation = 1888;
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDO_Y_UP:

			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);
			redo_source = 2;
			calculus = extruder_offset[Y_AXIS][1] + 0.5;
			SERIAL_PROTOCOLPGM("Calculus:  ");
			Serial.println(calculus);
			extruder_offset[Y_AXIS][RIGHT_EXTRUDER]=calculus;
			Config_StoreSettings(); //Store changes

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDO_Y_DOWN:

			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);
			redo_source = 2;
			calculus = extruder_offset[Y_AXIS][1] -0.4;
			SERIAL_PROTOCOLPGM("Calculus:  ");
			Serial.println(calculus);
			extruder_offset[Y_AXIS][RIGHT_EXTRUDER]=calculus;
			Config_StoreSettings(); //Store changes

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDO_Y:

			redo_source = 2;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CALIBZL_DOWN:

			feedrate = homing_feedrate[Z_AXIS];
			if (current_position[Z_AXIS]<1.25) current_position[Z_AXIS] += 0.05;  //Max down is Z=0.5
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], feedrate/60, active_extruder);
			SERIAL_PROTOCOLPGM("Z position: ");
			Serial.println(current_position[Z_AXIS]);

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CALIBZL_UP:

			feedrate = homing_feedrate[Z_AXIS];
			if (current_position[Z_AXIS]>-5.0) current_position[Z_AXIS] -= 0.05;  //Max up is Z=-5.0
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], feedrate/60, active_extruder);
			SERIAL_PROTOCOLPGM("Z position: ");
			Serial.println(current_position[Z_AXIS]);

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CALIBZL_ACCEPT:
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			setTargetHotend0(print_temp_l);
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;

			SERIAL_PROTOCOLPGM("OK first Extruder! \n");
			//We have to override z_prove_offset
			//zprobe_zoffset-=(current_position[Z_AXIS]); //We are putting more offset if needed
			zprobe_zoffset-=(current_position[Z_AXIS]-0.05); //We are putting more offset if needed
			extruder_offset[Z_AXIS][LEFT_EXTRUDER]=0.0;//It is always the reference
			SERIAL_PROTOCOLPGM("Z1 Probe offset: ");
			Serial.println(zprobe_zoffset);
			Config_StoreSettings(); //Store changes

			current_position[Z_AXIS] += 2;
			plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],homing_feedrate[Z_AXIS]/60,active_extruder);
			st_synchronize();
			ERROR_SCREEN_WARNING;
			home_axis_from_code(true,false,false);
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
			Calib_check_temps();
			bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
			gif_processing_state = PROCESSING_TEST;


			home_axis_from_code(false,true,true);
			ERROR_SCREEN_WARNING;
			
			#if PATTERN_Z_CALIB == 0
			z_test_print_code(LEFT_EXTRUDER,0);
			#else
			result = 0;
			result = z_test_print_code(LEFT_EXTRUDER,0,false);
			if(abs(result) >= 0 && abs(result) < 5){
				gif_processing_state = PROCESSING_TEST;
				home_axis_from_code(false,false,true);
				result = z_test_print_code(LEFT_EXTRUDER,result,true);
			}
			if(result == 1999){
				setTargetHotend0(CALIBFULL_HOTEND_STANDBY_TEMP);
				display_ChangeForm( FORM_UTILITIES_CALIBRATION_CALIBFULL_REDOZL,0);
			}
			#endif
			
			enquecommand_P(PSTR("M84"));
			ERROR_SCREEN_WARNING;

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CALIBZR_DOWN:
			feedrate = homing_feedrate[Z_AXIS];
			if (current_position[Z_AXIS]<1.25) current_position[Z_AXIS] += 0.05;  //Max down is Z=0.5
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], feedrate/60, active_extruder);
			SERIAL_PROTOCOLPGM("Z position: ");
			Serial.println(current_position[Z_AXIS]);
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CALIBZR_UP:
			feedrate = homing_feedrate[Z_AXIS];
			if (current_position[Z_AXIS]>-5.0) current_position[Z_AXIS] -= 0.05;  //Max up is Z=-5.0
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], feedrate/60, active_extruder);
			SERIAL_PROTOCOLPGM("Z position: ");
			Serial.println(current_position[Z_AXIS]);
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CALIBZL_TIP:
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CALIBZR_TIP:
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CALIBZ_TIP,0);
			gif_processing_state = PROCESSING_CALIB_Z_GAUGE;
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CALIBZ_TIP_BACK:
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			if(active_extruder == LEFT_EXTRUDER){
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CALIBZL,0);
				gif_processing_state = PROCESSING_CALIB_ZL;
				}else if(active_extruder == RIGHT_EXTRUDER){
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CALIBZR,0);
				gif_processing_state = PROCESSING_CALIB_ZR;
			}
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CALIBZR_ACCEPT:

			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			setTargetHotend1(print_temp_r);
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;


			//SERIAL_PROTOCOLLNPGM("OK second Extruder!");
			//extruder_offset[Z_AXIS][RIGHT_EXTRUDER]-=(current_position[Z_AXIS]);//Add the difference to the current offset value
			extruder_offset[Z_AXIS][RIGHT_EXTRUDER]-=(current_position[Z_AXIS]-0.05);//Add the difference to the current offset value
			//SERIAL_PROTOCOLPGM("Z2 Offset: ");
			//Serial.println(extruder_offset[Z_AXIS][RIGHT_EXTRUDER]);
			Config_StoreSettings(); //Store changes

			current_position[Z_AXIS] += 2;
			plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],homing_feedrate[Z_AXIS]/60,active_extruder);
			st_synchronize();
			ERROR_SCREEN_WARNING;
			home_axis_from_code(true,false,false);
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
			Calib_check_temps();
			bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
			gif_processing_state = PROCESSING_TEST;



			st_synchronize();
			ERROR_SCREEN_WARNING;

			home_axis_from_code(false,true,true);
			ERROR_SCREEN_WARNING;
			
			#if PATTERN_Z_CALIB == 0
			z_test_print_code(RIGHT_EXTRUDER,32);
			#else
			
			result = 0;
			result = z_test_print_code(RIGHT_EXTRUDER,0, false);
			if(abs(result) >= 0 && abs(result) < 5){
				gif_processing_state = PROCESSING_TEST;
				home_axis_from_code(false,false,true);
				result = z_test_print_code(RIGHT_EXTRUDER,result,true);
			}
			if(result == 1999){
				setTargetHotend1(CALIBFULL_HOTEND_STANDBY_TEMP);
				display_ChangeForm( FORM_UTILITIES_CALIBRATION_CALIBFULL_REDOZR,0);
			}
			#endif
			enquecommand_P(PSTR("M84"));
			ERROR_SCREEN_WARNING;

			break;
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_REDO:
			if(calib_zxy_id == 3){
				display_ChangeForm( FORM_UTILITIES_CALIBRATION_CALIBFULL_REDOZL,0);
				}else if(calib_zxy_id == 4){
				display_ChangeForm( FORM_UTILITIES_CALIBRATION_CALIBFULL_REDOZR,0);
			}
			break;

			
			#if PATTERN_Z_CALIB == 0
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT1:
			
			
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT1,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,1);
			calib_confirmation = LINES_GAP*1000;
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT2:
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT2,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,1);
			calib_confirmation = 0;
			
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT3:
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT3,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,1);
			calib_confirmation = -LINES_GAP*1000;
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT4:
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT4,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,1);
			calib_confirmation = -2*LINES_GAP*1000;
			
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT5:
			turnoff_buttons_zcalib();
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT5,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,1);
			calib_confirmation = -3*LINES_GAP*1000;
			
			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM:
			if(calib_confirmation <= LINES_GAP*1000 && calib_confirmation >= -3*LINES_GAP*1000 && calib_confirmation!=1888 ){
				if(calib_zxy_id == 3){
					Full_calibration_ZL_set((float)calib_confirmation/1000.0);
					}else if(calib_zxy_id == 4){
					Full_calibration_ZR_set((float)calib_confirmation/1000.0);
				}
			}
			break;
			#endif

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOZL_RECALIBRATE:
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOZR_RECALIBRATE:
			redo_source = 3;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);
			current_position[E_AXIS] -= 4;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], 15, active_extruder);//move first extruder
			st_synchronize();

			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED:
			doblocking = true;
			if(redo_source == 0){		 //redo z test print
				if (active_extruder==0){
					setTargetHotend0(print_temp_l);
					bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
					Calib_check_temps();
					bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
					gif_processing_state = PROCESSING_STOP;
					touchscreen_update();
					display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
					gif_processing_state = PROCESSING_TEST;
					home_axis_from_code(true,true,true);
					ERROR_SCREEN_WARNING;
					#if PATTERN_Z_CALIB == 0
					z_test_print_code(LEFT_EXTRUDER,0);
					#else
					
					result = 0;
					result = z_test_print_code(LEFT_EXTRUDER,0,false);
					if(abs(result) >= 0 && abs(result) < 5) {
						gif_processing_state = PROCESSING_TEST;
						home_axis_from_code(false,false,true);
						result = z_test_print_code(LEFT_EXTRUDER,result,true);
					}
					if(result == 1999){
						setTargetHotend0(CALIBFULL_HOTEND_STANDBY_TEMP);
						display_ChangeForm( FORM_UTILITIES_CALIBRATION_CALIBFULL_REDOZL,0);
					}
					#endif
					
					enquecommand_P(PSTR("M84"));
					ERROR_SCREEN_WARNING;
				}
				else{
					setTargetHotend1(print_temp_r);
					bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
					Calib_check_temps();
					bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
					gif_processing_state = PROCESSING_STOP;
					touchscreen_update();
					display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
					gif_processing_state = PROCESSING_TEST;
					home_axis_from_code(true,true,true);
					ERROR_SCREEN_WARNING;
					
					#if PATTERN_Z_CALIB == 0
					z_test_print_code(RIGHT_EXTRUDER,32);
					#else
					
					result = 0;
					result = z_test_print_code(RIGHT_EXTRUDER,0,false);
					if(abs(result) >= 0 && abs(result) < 5) {
						gif_processing_state = PROCESSING_TEST;
						home_axis_from_code(false,false,true);
						result = z_test_print_code(RIGHT_EXTRUDER,result,true);
					}
					if(result == 1999){
						setTargetHotend1(CALIBFULL_HOTEND_STANDBY_TEMP);
						display_ChangeForm( FORM_UTILITIES_CALIBRATION_CALIBFULL_REDOZR,0);
					}
					#endif
					enquecommand_P(PSTR("M84"));
					ERROR_SCREEN_WARNING;
					
				}
			}
			else if(redo_source == 1){ //redo x test print
				setTargetHotend1(print_temp_r);
				setTargetHotend0(print_temp_l);
				bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
				Calib_check_temps();
				bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
				gif_processing_state = PROCESSING_STOP;
				touchscreen_update();
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
				gif_processing_state = PROCESSING_TEST;
				home_axis_from_code(true,true,false);
				current_position[Z_AXIS] = 0.2;
				plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],1500/60,active_extruder);
				enquecommand_P(PSTR("G40"));
			}
			else if(redo_source == 2){ //redo y test print
				setTargetHotend1(print_temp_r);
				setTargetHotend0(print_temp_l);
				bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
				Calib_check_temps();
				bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode);
				gif_processing_state = PROCESSING_STOP;
				touchscreen_update();
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
				gif_processing_state = PROCESSING_TEST;
				home_axis_from_code(true,true,false);
				current_position[Z_AXIS] = 0.3;
				plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],1500/60,active_extruder);
				enquecommand_P(PSTR("G41"));
				
			}
			else if(redo_source == 3){	//recalibrate
				if (active_extruder == 0){
					gif_processing_state = PROCESSING_DEFAULT;
					display_ChangeForm(FORM_PROCESSING,0);
					current_position[E_AXIS]-=4;
					plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],7.5,LEFT_EXTRUDER);
					st_synchronize();
					home_axis_from_code(true,true,true);
					ERROR_SCREEN_WARNING;
					enquecommand_P(PSTR("G43"));
					gif_processing_state = PROCESSING_STOP;
				}
				else{
					gif_processing_state = PROCESSING_DEFAULT;
					display_ChangeForm(FORM_PROCESSING,0);
					current_position[E_AXIS]-=4;
					plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],7.5,RIGHT_EXTRUDER);
					st_synchronize();
					ERROR_SCREEN_WARNING;
					home_axis_from_code(true,true,true);
					ERROR_SCREEN_WARNING;
					enquecommand_P(PSTR("G43"));
					gif_processing_state = PROCESSING_STOP;
				}
			}

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOZL_BEST1:
			redo_source = 0;
			zprobe_zoffset+=0.1;
			Config_StoreSettings(); //Store changes
			gcode_T0_T1_auto(0);
			st_synchronize();
			ERROR_SCREEN_WARNING;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOZR_BEST1:
			redo_source = 0;
			extruder_offset[Z_AXIS][RIGHT_EXTRUDER]+=0.1;
			Config_StoreSettings(); //Store changes
			gcode_T0_T1_auto(1);
			st_synchronize();
			ERROR_SCREEN_WARNING;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOZL_BEST5:
			redo_source = 0;
			zprobe_zoffset-=0.1;
			Config_StoreSettings(); //Store changes
			gcode_T0_T1_auto(0);
			st_synchronize();
			ERROR_SCREEN_WARNING;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOZR_BEST5:
			redo_source = 0;
			extruder_offset[Z_AXIS][RIGHT_EXTRUDER]-=0.1;
			Config_StoreSettings(); //Store changes
			gcode_T0_T1_auto(1);
			st_synchronize();
			ERROR_SCREEN_WARNING;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOZL:
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_REDOZR:
			redo_source = 0;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);

			break;
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZL_GO:

			doblocking=true;

			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			active_extruder = LEFT_EXTRUDER;
			//Wait until temperature it's okey

			changeTool(0);
			home_axis_from_code(true,true,true);
			//changeTool(LEFT_EXTRUDER);
			gif_processing_state = PROCESSING_STOP;
			st_synchronize();
			setTargetHotend0(CALIBFULL_HOTEND_STANDBY_TEMP);
			setTargetHotend1(CALIBFULL_HOTEND_STANDBY_TEMP);
			setTargetBed(max(bed_temp_l,bed_temp_r));
			
			
			Calib_check_temps();
			gif_processing_state = PROCESSING_DEFAULT;
			
			current_position[Z_AXIS] = 60;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], 15, LEFT_EXTRUDER);//move bed
			st_synchronize();
			ERROR_SCREEN_WARNING;

			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			current_position[X_AXIS] = 150;
			#else
			current_position[X_AXIS] = 150 + 100;
			#endif

			current_position[Y_AXIS] = 0;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], 200, LEFT_EXTRUDER);//move first extruder
			st_synchronize();
			current_position[E_AXIS] -= 4;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], 7.5, LEFT_EXTRUDER);//move first extruder
			st_synchronize();
			ERROR_SCREEN_WARNING;
			gif_processing_state = PROCESSING_STOP;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANNOZZLE0,0);


			break;
			
			/*
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZ_GO:
			
			if(Step_First_Start_Wizard){
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZL_SKIP,1);
			}else{
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZL_SKIP,0);
			}
			
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZL,0);
			
			break;
			
			
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZ_SKIP:
			if(!Step_First_Start_Wizard && !bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode)){
			
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX,0);
			}
			break;
			*/
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZL_SKIP:
			if(!Step_First_Start_Wizard && !bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode)){
				
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZR,0);
			}
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZR_GO:

			doblocking=true;
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			active_extruder = RIGHT_EXTRUDER;
			//Wait until temperature it's okey
			changeTool(1);
			home_axis_from_code(true,true,true);
			//changeTool(LEFT_EXTRUDER);
			gif_processing_state = PROCESSING_STOP;

			st_synchronize();
			setTargetHotend0(CALIBFULL_HOTEND_STANDBY_TEMP);
			setTargetHotend1(CALIBFULL_HOTEND_STANDBY_TEMP);
			setTargetBed(max(bed_temp_l,bed_temp_r));
			Calib_check_temps();

			gif_processing_state = PROCESSING_DEFAULT;
			
			current_position[Z_AXIS] = 60;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], 15, RIGHT_EXTRUDER);//move bed
			st_synchronize();
			ERROR_SCREEN_WARNING;
			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			current_position[X_AXIS] = 170;
			#else
			current_position[X_AXIS] = 170 + 100;
			#endif

			current_position[Y_AXIS] = 0;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], 200, RIGHT_EXTRUDER);//move first extruder
			st_synchronize();
			current_position[E_AXIS] -= 4;
			plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], 7.5, RIGHT_EXTRUDER);//move first extruder
			st_synchronize();
			ERROR_SCREEN_WARNING;
			gif_processing_state = PROCESSING_STOP;

			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANNOZZLE1,0);


			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZR_SKIP:
			if(!Step_First_Start_Wizard){
				Z_compensation_decisor();
			}
			break;

			
			case BUTTON_Z_COMPENSATION_SKIP:
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX,0);
			if(Step_First_Start_Wizard){
				display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX_SKIP,1);
			}

			break;

			case BUTTON_Z_COMPENSATION_INSTALL:
			flag_utilities_calibration_zcomensationmode_gauges = 1888;
			
			bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_cooldown_mode0);
			break;

			case BUTTON_Z_COMPENSATION_COMFIRMATION_YES:
			setTargetHotend0(CALIBFULL_HOTEND_STANDBY_TEMP);
			setTargetHotend1(CALIBFULL_HOTEND_STANDBY_TEMP);
			setTargetBed(max(bed_temp_l,bed_temp_r));
			
			
			if(extruder_offset[Z_AXIS][RIGHT_EXTRUDER]<0){
				extruder_offset[Z_AXIS][RIGHT_EXTRUDER] = 0;
				}else{
				zprobe_zoffset +=extruder_offset[Z_AXIS][RIGHT_EXTRUDER];
				extruder_offset[Z_AXIS][RIGHT_EXTRUDER] = 0;
			}

			surfing_utilities = true;

			if(flag_utilities_calibration_zcomensationmode_gauges == 1888){
				display_ChangeForm(FORM_PROCESSING,0);
				gif_processing_state = PROCESSING_DEFAULT;
				doblocking= true;
				home_axis_from_code(true,true,true);
				gif_processing_state = PROCESSING_STOP;
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX,0);
				if(Step_First_Start_Wizard){
					display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX_SKIP,1);
				}
				
				}else if(flag_utilities_calibration_zcomensationmode_gauges == 2888){
				
				bed_calibration_times = 0;
				if(saved_print_flag==1888){
					saved_print_flag = 888;
					Config_StoreSettings();
				}
				bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_calibfull_skipZcalib);
				display_ChangeForm(FORM_Z_COMPENSATION_CONFIRMATION_CALIB_NOTICE,0);
			}
			flag_utilities_calibration_zcomensationmode_gauges = 888;
			Config_StoreSettings();




			break;
			
			case BUTTON_Z_COMPENSATION_CONFIRMATION_CALIB_NOTICE:
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			doblocking= true;
			home_axis_from_code(true,true,true);
			st_synchronize();
			ERROR_SCREEN_WARNING;
			enquecommand_P(PSTR("G34"));	//Start BED Calibration Wizard
			changeTool(0);
			break;

			case BUTTON_Z_COMPENSATION_COMFIRMATION_NOT:
			display_ChangeForm(FORM_Z_COMPENSATION_COMFIRMATION_SURECANCEL,0);

			break;

			case BUTTON_Z_COMPENSATION_COMFIRMATION_SURECANCEL_YES:


			if(flag_utilities_calibration_zcomensationmode_gauges == 1888){
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX,0);
				if(Step_First_Start_Wizard){
					display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX_SKIP,1);
				}
				setTargetHotend0(print_temp_l);
				setTargetHotend1(print_temp_r);
				setTargetBed(max(bed_temp_l,bed_temp_r));
				
				
				flag_utilities_calibration_zcomensationmode_gauges = 888;
				Config_StoreSettings();
				surfing_utilities = true;
				}else if(flag_utilities_calibration_zcomensationmode_gauges == 2888){
				display_ChangeForm(FORM_MAIN,0);
				flag_utilities_calibration_zcomensationmode_gauges = 888;
				Config_StoreSettings();
				
			}


			break;

			case BUTTON_RAFT_ADVISE_CANCEL:
			display_ChangeForm(FORM_RAFT_ADVISE_INSTALL,0);

			break;

			case BUTTON_RAFT_ADVISE_INSTALL_ACCEPT:
			flag_utilities_calibration_zcomensationmode_gauges = 2888;
			screen_sdcard = false;
			bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_cooldown_mode0);
			break;

			case BUTTON_Z_COMPENSATION_COMFIRMATION_SURECANCEL_NOT:
			bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_cooldown_mode0);
			break;

			

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX_GO:
			doblocking=true;
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			home_axis_from_code(true,true,true);
			gif_processing_state = PROCESSING_STOP;
			st_synchronize();
			setTargetHotend0(print_temp_l);
			setTargetHotend1(print_temp_r);
			setTargetBed(max(bed_temp_l,bed_temp_r));
			
			
			Calib_check_temps();
			gif_processing_state = PROCESSING_DEFAULT;
			ERROR_SCREEN_WARNING;
			changeTool(0);
			enquecommand_P(PSTR("G40"));
			ERROR_SCREEN_WARNING;

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX_SKIP:
			if(!Step_First_Start_Wizard){
				
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBY,0);
				if(Step_First_Start_Wizard){
					display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBY_SKIP,1);
				}
			}
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBY_GO:
			doblocking=true;
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			home_axis_from_code(true,true,true);
			gif_processing_state = PROCESSING_STOP;
			setTargetHotend0(print_temp_l);
			setTargetHotend1(print_temp_r);
			setTargetBed(max(bed_temp_l,bed_temp_r));
			
			
			st_synchronize();
			Calib_check_temps();
			gif_processing_state = PROCESSING_DEFAULT;
			changeTool(0);
			ERROR_SCREEN_WARNING;
			enquecommand_P(PSTR("G41"));
			st_synchronize();
			ERROR_SCREEN_WARNING;

			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBY_SKIP:
			if(!Step_First_Start_Wizard){
				
				if(Step_First_Start_Wizard){
					display_ChangeForm(FORM_SETUPASSISTANT_SUCCESS,0);
					gif_processing_state = PROCESSING_SUCCESS_FIRST_RUN;
					FLAG_First_Start_Wizard = 888;
					Step_First_Start_Wizard = false;
					
					}else{
					printer_state = STATE_CALIBRATION;
					display_SetFrame(GIF_UTILITIES_CALIBRATION_SUCCESS,0);
					display_ChangeForm(FORM_UTILITIES_CALIBRATION_SUCCESS,0);
					HeaterCooldownInactivity(true);
					gif_processing_state = PROCESSING_BED_SUCCESS;
				}
				bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_calibfull);
			}
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CLEANNOZZLE0:
			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_CLEANNOZZLE1:
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			current_position[X_AXIS]=x_home_pos(active_extruder);
			current_position[Y_AXIS]=Y_MAX_POS;
			plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],XY_TRAVEL_SPEED/60,active_extruder);
			st_synchronize();
			gif_processing_state = PROCESSING_STOP;
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GLUEBED,0);
			break;

			case BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GLUEBED:

			display_ChangeForm(FORM_PROCESSING,0);

			//home_axis_from_code(true, true , false);
			enquecommand_P(PSTR("G43"));
			flag_continue_calib = false;
			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_BACK:
			display_ChangeForm(FORM_UTILITIES_CALIBRATION,0);
			offset_calib_manu[0]=0.0;
			offset_calib_manu[1]=0.0;
			offset_calib_manu[2]=0.0;
			offset_calib_manu[3]=0.0;
			calib_value_selected = 0;
			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_MENU:
			display_ChangeForm(FORM_MAIN,0);
			offset_calib_manu[0]=0.0;
			offset_calib_manu[1]=0.0;
			offset_calib_manu[2]=0.0;
			offset_calib_manu[3]=0.0;
			calib_value_selected = 0;
			screen_sdcard = false;
			surfing_utilities=false;
			surfing_temps = false;
		
			SERIAL_PROTOCOLLNPGM("Main");
			
			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_OK:
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_MANUAL_SAVE,0);
			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_X:

			calib_value_selected = 0;
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_MANUAL,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_ZR,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_ZL,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_X,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_Y,0);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_UTILITIES_CALIBRATION_MANUAL_RIGHT,0);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_UTILITIES_CALIBRATION_MANUAL_LEFT,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_UP,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_DOWN,1);
			value = 0.0;
			value = manual_fine_calib_offset[calib_value_selected] + offset_calib_manu[calib_value_selected];
			display.WriteStr(STRING_UTILITIES_CALIBRATION_MANUAL,value,3);

			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_Y:

			calib_value_selected = 1;
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_MANUAL,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_ZR,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_ZL,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_X,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_Y,1);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_UTILITIES_CALIBRATION_MANUAL_RIGHT,1);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_UTILITIES_CALIBRATION_MANUAL_LEFT,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_UP,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_DOWN,0);
			value = 0.0;
			value = manual_fine_calib_offset[calib_value_selected] + offset_calib_manu[calib_value_selected];
			display.WriteStr(STRING_UTILITIES_CALIBRATION_MANUAL,value,3);
			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_ZL:

			calib_value_selected = 2;
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_MANUAL,2);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_ZR,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_ZL,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_X,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_Y,0);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_UTILITIES_CALIBRATION_MANUAL_RIGHT,1);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_UTILITIES_CALIBRATION_MANUAL_LEFT,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_UP,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_DOWN,0);
			value = 0.0;
			value = manual_fine_calib_offset[calib_value_selected] + offset_calib_manu[calib_value_selected];
			display.WriteStr(STRING_UTILITIES_CALIBRATION_MANUAL,value,3);
			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_ZR:
			calib_value_selected = 3;
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_MANUAL,2);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_ZR,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_ZL,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_X,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_Y,0);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_UTILITIES_CALIBRATION_MANUAL_RIGHT,1);
			//genie.WriteObject(GENIE_OBJ_USERBUTTON,BUTTON_UTILITIES_CALIBRATION_MANUAL_LEFT,1);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_UP,0);
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_MANUAL_DOWN,0);
			value = 0.0;
			value = manual_fine_calib_offset[calib_value_selected] + offset_calib_manu[calib_value_selected];
			display.WriteStr(STRING_UTILITIES_CALIBRATION_MANUAL,value,3);
			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_UP:

			value = 0.0;

			if(calib_value_selected == 1 || calib_value_selected == 0){
				if(offset_calib_manu[calib_value_selected] < 1.975) offset_calib_manu[calib_value_selected] += 0.025;
			}
			else{
				if(offset_calib_manu[calib_value_selected] < 0.200) offset_calib_manu[calib_value_selected] += 0.025;
			}
			value = offset_calib_manu[calib_value_selected]+manual_fine_calib_offset[calib_value_selected];
			display.WriteStr(STRING_UTILITIES_CALIBRATION_MANUAL,value,3);
			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_DOWN:
			value = 0.0;

			if(calib_value_selected == 1 || calib_value_selected == 0){
				if(offset_calib_manu[calib_value_selected] > -1.975) offset_calib_manu[calib_value_selected] -= 0.025;
			}
			else{
				if(offset_calib_manu[calib_value_selected] > -0.200) offset_calib_manu[calib_value_selected] -= 0.025;
			}
			value = offset_calib_manu[calib_value_selected]+manual_fine_calib_offset[calib_value_selected];

			display.WriteStr(STRING_UTILITIES_CALIBRATION_MANUAL,value,3);
			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_SAVE_OK:
			manual_fine_calib_offset[0] += offset_calib_manu[0];
			manual_fine_calib_offset[1] += offset_calib_manu[1];
			manual_fine_calib_offset[2] += offset_calib_manu[2];
			manual_fine_calib_offset[3] += offset_calib_manu[3];

			extruder_offset[X_AXIS][RIGHT_EXTRUDER]+= offset_calib_manu[0];
			extruder_offset[Y_AXIS][RIGHT_EXTRUDER]+= offset_calib_manu[1];
			zprobe_zoffset += offset_calib_manu[2];
			extruder_offset[Z_AXIS][RIGHT_EXTRUDER]+= offset_calib_manu[3] - offset_calib_manu[2];
			display_ChangeForm(FORM_UTILITIES_CALIBRATION,0);
			offset_calib_manu[0]=0.0;
			offset_calib_manu[1]=0.0;
			offset_calib_manu[2]=0.0;
			offset_calib_manu[3]=0.0;
			calib_value_selected = 0;
			Config_StoreSettings();
			Config_PrintSettings();
			break;

			case BUTTON_UTILITIES_CALIBRATION_MANUAL_SAVE_NOT:

			display_ChangeForm(FORM_UTILITIES_CALIBRATION,0);
			offset_calib_manu[0]=0.0;
			offset_calib_manu[1]=0.0;
			offset_calib_manu[2]=0.0;
			offset_calib_manu[3]=0.0;
			calib_value_selected = 0;
			break;
			
			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			case BUTTON_PRINTERSETUP_EXTRUDERCONFIG_SAVE:
			if (which_extruder_setup == 1){
				axis_steps_per_unit[E_AXIS]=152.0;
				max_feedrate[E_AXIS]=60;
			}
			else if (which_extruder_setup == 2){
				axis_steps_per_unit[E_AXIS]=510.9;
				max_feedrate[E_AXIS]=40;
			}
			if(FLAG_Printer_Setup_Assistant == 888){
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,0);
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_BACK,0);
				FLAG_Printer_Setup_Assistant = 1888;
				display_ChangeForm(FORM_SETUPASSISTANT_SUCCESS,0);
				gif_processing_state = PROCESSING_SUCCESS_FIRST_RUN;
				
				}else{
				show_data_printconfig();
			}
			Config_StoreSettings();
			break;
			
			case BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_1:
			
			display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_2,0);
			display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_1,1);
			which_extruder_setup = 1;
			break;
			
			case BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_2:
			
			display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_2,1);
			display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_1,0);
			which_extruder_setup = 2;
			break;
			#endif
			
			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_MENU_2:
			#endif
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_MENU:
			display_ChangeForm(FORM_MAIN,0);
			which_extruder_setup = -1;
			which_hotend_setup[0] = -1;
			which_hotend_setup[1] = -1;
			break;
			



			//***** Info Screens *****


			//Backing from INFO SCREENS
			case BUTTON_UTILITIES_CALIBRATION_BACK:

			display_ChangeForm(FORM_UTILITIES,0);
			break;


			//SKIP BED CALIBRATION
			case BUTTON_UTILITIES_CALIBRATION_CALIBBED_ADJUSTSCREWASK_SKIP:

			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			if (bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_calibfull)){
				bed_calibration_times = 0;
				
				active_extruder = LEFT_EXTRUDER;
				display_ChangeForm(FORM_PROCESSING,0);
				gif_processing_state = PROCESSING_DEFAULT;
				setTargetHotend0(print_temp_l);
				setTargetHotend1(print_temp_r);
				setTargetBed(max(bed_temp_l,bed_temp_r));
				
				
				
				home_axis_from_code(true,true,false);
				st_synchronize();
				ERROR_SCREEN_WARNING;
				enquecommand_P(PSTR("T0"));
				gif_processing_state = PROCESSING_STOP;
				touchscreen_update();
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZL,0);
				display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZL_SKIP,0);
				if(Step_First_Start_Wizard){
					display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZL_SKIP,1);
				}
				
			}
			
			else if(bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode)){
				active_extruder = which_extruder;
				//genie.WriteObject(GENIE_OBJ_FORM,FORM_WAITING_ROOM,0);
				//gif_processing_state = PROCESSING_DEFAULT;
				
				st_synchronize();
				
				Bed_Compensation_state = 2;
				Bed_compensation_redo_offset = 0;
				ERROR_SCREEN_WARNING;
				if(which_extruder==0){
					enquecommand_P(PSTR("T0"));
					}else{
					enquecommand_P(PSTR("T1"));
				}
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_CLEANBED,0);
			}
			else if(bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_calibfull_skipZcalib)){
				bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_calibfull_skipZcalib);
				active_extruder = LEFT_EXTRUDER;
				setTargetHotend0(CALIBFULL_HOTEND_STANDBY_TEMP);
				setTargetHotend1(CALIBFULL_HOTEND_STANDBY_TEMP);
				setTargetBed(max(bed_temp_l,bed_temp_r));
				
				st_synchronize();
				ERROR_SCREEN_WARNING;
				enquecommand_P(PSTR("T0"));
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX,0);
				
			}
			else{
				printer_state = STATE_CALIBRATION;
				display_SetFrame(GIF_UTILITIES_CALIBRATION_SUCCESS,0);
				display_ChangeForm(FORM_UTILITIES_CALIBRATION_SUCCESS,0);
				HeaterCooldownInactivity(true);
				gif_processing_state = PROCESSING_BED_SUCCESS;
			}

			break;

			case BUTTON_ERROR_OK:

			if(printing_error_temps){
				gif_processing_state = PROCESSING_STOP;
				touchscreen_update();
				display_ChangeForm(FORM_PROCESSING,0);
				enquecommand_P(PSTR("G28 X0 Y0")); //Home X and Y
				back_home = true;
				home_made = false;
				screen_sdcard = false;
				surfing_utilities=false;
				surfing_temps = false;
				
				card.sdprinting = false;
				card.sdispaused = false;
				
				gif_processing_state = PROCESSING_DEFAULT;
				printing_error_temps = false;
			}
			else{
				gif_processing_state = PROCESSING_STOP;
				screen_sdcard = false;
				surfing_utilities=false;
				
				SERIAL_PROTOCOLLNPGM("Main");
				
				surfing_temps = false;
				HeaterCooldownInactivity(true);
				display_ChangeForm( FORM_MAIN, 0);
			}
			doblocking = false;

			break;


			case BUTTON_SETUPASSISTANT_YES:

			surfing_utilities = true;
			Step_First_Start_Wizard = true;
			display_ChangeForm(FORM_SETUPASSISTANT_STEP_1,0);
			Config_ResetDefault();
			Config_StoreSettings();
			break;

			case BUTTON_SETUPASSISTANT_SKIP:
			FLAG_First_Start_Wizard = 888;
			Step_First_Start_Wizard = false;
			display_ChangeForm(FORM_MAIN,0);
			Config_StoreSettings();
			break;

			case BUTTON_SETUPASSISTANT_STEP_NEXT_1:
			which_extruder = 0;
			filament_mode = 'I';
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1, 0);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0, 1);

			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_MENU, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_BACK, 1);
			display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD,0);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILNEXT, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILBACK, 1);
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"    PLA");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL2,"    PVA");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL3,"  PET-G");
			break;

			case BUTTON_SETUPASSISTANT_STEP_NEXT_2:

			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_NOTICE_BACK,1);
			display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_NOTICE,0);

			break;
			
			case BUTTON_INFO_STATS:
			display_ChangeForm(FORM_INFO_STATS,0);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_STATS_HOURS,log_hours_print);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_STATS_COMPLETED,log_prints_finished);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_STATS_XY,((long)((log_X0_mmdone+log_X1_mmdone+log_Y_mmdone)/1000000)));
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_STATS_FILAMENT,(long)((0.0079*(log_E0_mmdone+log_E1_mmdone)/1000)));  //0.0079 Kg/m PLA
			break;
			
			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			case BUTTON_INFO_UI_BACK:
			case BUTTON_INFO_UI_BACK_2:
			display_ChangeForm(FORM_INFO_PRINTER,0);
			break;
			
			case BUTTON_INFO_UI_MENU:
			case BUTTON_INFO_UI_MENU_2:
			display_ChangeForm(FORM_MAIN,0);
			break;
			#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
			
			case BUTTON_INFO_UI_BACK:
			display_ChangeForm(FORM_INFO_PRINTER,0);
			break;
			
			case BUTTON_INFO_UI_MENU:
			display_ChangeForm(FORM_MAIN,0);
			break;
			#endif
			
			case BUTTON_INFO_UNIT:
			
			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			#ifdef ENCLOSURE_SAFETY_STOP
			display_ChangeForm(FORM_INFO_UI_2,0);
			#ifndef BUILD_DATE
			sprintf_P(buffer, PSTR("%s"),VERSION_STRING);
			#else
			sprintf_P(buffer, PSTR("%s|M%02d.%02d"),VERSION_STRING,BUILDTM_MONTH,BUILDTM_DAY);
			#endif
			display.WriteStr(STRING_INFO_UI_VERSION_2,buffer);
			if(UI_SerialID0 || UI_SerialID1 || UI_SerialID2){
				sprintf_P(buffer, PSTR("%03d.%03d%03d.%04d"),UI_SerialID0, (int)(UI_SerialID1/1000),(int)(UI_SerialID1%1000), UI_SerialID2);
				//sprintf(buffer, "%03d.%03d%03d.%04d",1020, 1151,1021, 10002);
				display.WriteStr(STRING_INFO_UI_SERIALID_2,buffer);
				}else{
				strcpy_P(buffer, PSTR(UI_SerialID));
				display.WriteStr(STRING_INFO_UI_SERIALID_2,buffer);
			}


			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_L_DIGIT1_2,(int)hotend_size_setup[LEFT_EXTRUDER]/1);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_L_DIGIT2_2,(int)(hotend_size_setup[LEFT_EXTRUDER]*10)%10);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_R_DIGIT1_2,(int)hotend_size_setup[RIGHT_EXTRUDER]/1);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_R_DIGIT2_2,(int)(hotend_size_setup[RIGHT_EXTRUDER]*10)%10);
			#else
			if(UI_SerialID2<SERIAL_ID_THRESHOLD || (axis_steps_per_unit[E_AXIS]>=512 && axis_steps_per_unit[E_AXIS]<=492) ){
				
				
				display_ChangeForm(FORM_INFO_UI,0);
				#ifndef BUILD_DATE
				sprintf_P(buffer, PSTR("%s"),VERSION_STRING);
				#else
				sprintf_P(buffer, PSTR("%s|M%02d.%02d"),VERSION_STRING,BUILDTM_MONTH,BUILDTM_DAY);
				#endif
				display.WriteStr(STRING_INFO_UI_VERSION,buffer);
				if(UI_SerialID0 || UI_SerialID1 || UI_SerialID2){
					sprintf_P(buffer, PSTR("%03d.%03d%03d.%04d"),UI_SerialID0, (int)(UI_SerialID1/1000),(int)(UI_SerialID1%1000), UI_SerialID2);
					//sprintf(buffer, "%03d.%03d%03d.%04d",1020, 1151,1021, 10002);
					display.WriteStr(STRING_INFO_UI_SERIALID,buffer);
					}else{
					strcpy_P(buffer, PSTR(UI_SerialID));
					display.WriteStr(STRING_INFO_UI_SERIALID,buffer);
				}
				
				
				if (axis_steps_per_unit[E_AXIS]<=152.5 && axis_steps_per_unit[E_AXIS]>=151.5){
					sprintf_P(buffer, PSTR("R16/R17"));
					display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_INFO_EXTRUDER_SETUP,1);
					}else if (axis_steps_per_unit[E_AXIS]<=512 && axis_steps_per_unit[E_AXIS]>=492){
					sprintf_P(buffer, PSTR("R19"));
					display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_INFO_EXTRUDER_SETUP,0);
					}else if (axis_steps_per_unit[E_AXIS]<=110 && axis_steps_per_unit[E_AXIS]>=90){
					sprintf_P(buffer, PSTR("RCustom"));
					}else{
					display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_INFO_EXTRUDER_SETUP,1);
					sprintf_P(buffer, PSTR("%u.%02u"),(unsigned int)axis_steps_per_unit[E_AXIS],(unsigned int)(axis_steps_per_unit[E_AXIS]*100)%100);
				}
				
				display.WriteStr(STRING_INFO_EXTRUDER_SETUP,buffer);
				
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_L_DIGIT1,(int)hotend_size_setup[LEFT_EXTRUDER]/1);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_L_DIGIT2,(int)(hotend_size_setup[LEFT_EXTRUDER]*10)%10);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_R_DIGIT1,(int)hotend_size_setup[RIGHT_EXTRUDER]/1);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_R_DIGIT2,(int)(hotend_size_setup[RIGHT_EXTRUDER]*10)%10);
				
				}else{
				display_ChangeForm(FORM_INFO_UI_2,0);
				#ifndef BUILD_DATE
				sprintf_P(buffer, PSTR("%s"),VERSION_STRING);
				#else
				sprintf_P(buffer, PSTR("%s|M%02d.%02d"),VERSION_STRING,BUILDTM_MONTH,BUILDTM_DAY);
				#endif
				display.WriteStr(STRING_INFO_UI_VERSION_2,buffer);
				if(UI_SerialID0 || UI_SerialID1 || UI_SerialID2){
					sprintf_P(buffer, PSTR("%03d.%03d%03d.%04d"),UI_SerialID0, (int)(UI_SerialID1/1000),(int)(UI_SerialID1%1000), UI_SerialID2);
					//sprintf(buffer, "%03d.%03d%03d.%04d",1020, 1151,1021, 10002);
					display.WriteStr(STRING_INFO_UI_SERIALID_2,buffer);
					}else{
					strcpy_P(buffer, PSTR(UI_SerialID));
					display.WriteStr(STRING_INFO_UI_SERIALID_2,buffer);
				}
				
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_L_DIGIT1_2,(int)hotend_size_setup[LEFT_EXTRUDER]/1);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_L_DIGIT2_2,(int)(hotend_size_setup[LEFT_EXTRUDER]*10)%10);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_R_DIGIT1_2,(int)hotend_size_setup[RIGHT_EXTRUDER]/1);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_R_DIGIT2_2,(int)(hotend_size_setup[RIGHT_EXTRUDER]*10)%10);
			}
			
			#endif
			
			#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
			
			display_ChangeForm(FORM_INFO_UI,0);
			#ifndef BUILD_DATE
			sprintf_P(buffer, PSTR("%s"),VERSION_STRING);
			#else
			sprintf_P(buffer, PSTR("%s|M%02d.%02d"),VERSION_STRING,BUILDTM_MONTH,BUILDTM_DAY);
			#endif
			display.WriteStr(STRING_INFO_UI_VERSION,buffer);
			if(UI_SerialID0 || UI_SerialID1 || UI_SerialID2){
				sprintf_P(buffer, PSTR("%03d.%03d%03d.%04d"),UI_SerialID0, (int)(UI_SerialID1/1000),(int)(UI_SerialID1%1000), UI_SerialID2);
				//sprintf(buffer, "%03d.%03d%03d.%04d",1020, 1151,1021, 10002);
				display.WriteStr(STRING_INFO_UI_SERIALID,buffer);
				}else{
				strcpy_P(buffer, PSTR(UI_SerialID));
				display.WriteStr(STRING_INFO_UI_SERIALID,buffer);
			}
			
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_L_DIGIT1,(int)hotend_size_setup[LEFT_EXTRUDER]/1);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_L_DIGIT2,(int)(hotend_size_setup[LEFT_EXTRUDER]*10)%10);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_R_DIGIT1,(int)hotend_size_setup[RIGHT_EXTRUDER]/1);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_INFO_NOZZLE_SIZE_R_DIGIT2,(int)(hotend_size_setup[RIGHT_EXTRUDER]*10)%10);
			
			#endif
			
			break;
			
			case BUTTON_PRINTERSETUP_SETUPASSISTANT:
			display_SetFrame(GIF_SETUPASSISTANT_INIT,0);
			display_ChangeForm(FORM_SETUPASSISTANT_INIT,0);
			j = 0;
			waitPeriod = millis(); //Processing back home
			while ( j<GIF_FRAMES_INIT_FIRST_RUN){
				if (millis() >= waitPeriod){
					
					display_SetFrame(GIF_SETUPASSISTANT_INIT,j);
					j+=1;
					waitPeriod = GIF_FRAMERATE+millis();	//Every 5s
				}
				
				
				
			}
			if(!check_regiter_num(UI_registercode) && UI_SerialID2 >= SERIAL_ID_THRESHOLD){
				display_ChangeForm(FORM_PRINTERREGISTER_ASK,0);
				}else{
				display_ChangeForm(FORM_SETUPASSISTANT_YESNOT,0);
			}
			

			break;
			
			case BUTTON_PRINTERREGISTER_ASK_LATER:
			if(notice_registercode){
				display_ChangeForm(FORM_MAIN,0);
				notice_registercode = false;
				Config_StoreSettings();
				}else{
				display_ChangeForm(FORM_SETUPASSISTANT_YESNOT,0);
			}
			
			break;
			
			case BUTTON_PRINTERREGISTER_SUCCESS:
			if(RegID_digit_count == 4){
				
				if(notice_registercode){
					display_ChangeForm(FORM_MAIN,0);
					notice_registercode = false;
					}else{
					display_ChangeForm(FORM_SETUPASSISTANT_YESNOT,0);
				}
				UI_registercode = RegID;
				Config_StoreSettings();
			}
			break;
			
			
			case BUTTON_PRINTERREGISTER_ASK_REGISTER:
			case BUTTON_PRINTERREGISTER_BACK:
			display_ChangeForm(FORM_PRINTERREGISTER_ASK_VISIT,0);
			if(UI_SerialID0 || UI_SerialID1 || UI_SerialID2){
				sprintf_P(buffer, PSTR("%03d.%03d%03d.%04d"),UI_SerialID0, (int)(UI_SerialID1/1000),(int)(UI_SerialID1%1000), UI_SerialID2);
				//sprintf(buffer, "%03d.%03d%03d.%04d",1020, 1151,1021, 10002);
				display.WriteStr(STRING_PRINTERREGISTER_ASK_VISIT,buffer);
				}else{
				strcpy_P(buffer, PSTR(UI_SerialID));
				display.WriteStr(STRING_PRINTERREGISTER_ASK_VISIT,buffer);
			}
			
			break;
			
			case BUTTON_PRINTERREGISTER_ASK_VISIT_BACK:
			display_ChangeForm(FORM_PRINTERREGISTER_ASK,0);
			break;
			
			case BUTTON_PRINTERREGISTER_ASK_VISIT_DONE:
			RegID_digit_count = 0;
			RegID = 0;
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT3,10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT2,10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT1,10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT0,10);
			display_ButtonState(BUTTON_PRINTERREGISTER_SUCCESS,0);
			display_ChangeForm(FORM_PRINTERREGISTER,0);
			break;
			
			case BUTTON_PRINTERREGISTER_DELETE:
			
			if(RegID_digit_count>0){
				
				RegID = (RegID / 10)*10;
				
				switch(RegID_digit_count){
					case 1:
					display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT3,10);
					break;
					
					case 2:
					display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT2,10);
					break;
					
					case 3:
					display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT1,10);
					break;
					
					case 4:
					display_ButtonState(BUTTON_PRINTERREGISTER_SUCCESS,0);
					display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT0,10);
					break;
				}
				
				RegID_digit_count--;
				
			}
			
			break;
			
			case BUTTON_PRINTERREGISTER_0:
			setregiter_num(0);
			break;
			
			case BUTTON_PRINTERREGISTER_1:
			setregiter_num(1);
			break;
			
			case BUTTON_PRINTERREGISTER_2:
			setregiter_num(2);
			break;
			
			case BUTTON_PRINTERREGISTER_3:
			setregiter_num(3);
			break;
			
			case BUTTON_PRINTERREGISTER_4:
			setregiter_num(4);
			break;
			
			case BUTTON_PRINTERREGISTER_5:
			setregiter_num(5);
			break;
			
			case BUTTON_PRINTERREGISTER_6:
			setregiter_num(6);
			break;
			
			case BUTTON_PRINTERREGISTER_7:
			setregiter_num(7);
			break;
			
			case BUTTON_PRINTERREGISTER_8:
			setregiter_num(8);
			break;
			
			case BUTTON_PRINTERREGISTER_9:
			setregiter_num(9);
			break;
			
			case BUTTON_PRINTERSETUP_LED:
			display.WriteObject(GENIE_OBJ_ISMARTSLIDER,SMARTSLIDER_PRINTERSETUP_LED_BRIGHTNESS,led_brightness);
			display_ChangeForm(FORM_PRINTERSETUP_LED,0);
			break;
			#if (BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA && !defined(ENCLOSURE_SAFETY_STOP))
			
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_EXTRUDER:
			display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_SAVE,0);
			display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_BACK,0);
			if (which_extruder_setup == 1){
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_1,1);
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_2,0);
				}else if (which_extruder_setup == 2){
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_2,1);
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_1,0);
			}
			
			else{
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_1,0);
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_2,0);
			}
			display_ChangeForm(FORM_PRINTERSETUP_EXTRUDERCONFIG,0);
			break;
			
			case BUTTON_PRINTERSETUP_EXTRUDERCONFIG_BACK:
			#endif
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK:
			case BUTTON_PRINTERSETUP_PRINTERCONFIG:
			if(FLAG_Printer_Setup_Assistant != 888 && flag_utilities_maintenance_changehotend != 1888 && flag_utilities_maintenance_changehotend != 2888){
				if (axis_steps_per_unit[E_AXIS]<=152.5 && axis_steps_per_unit[E_AXIS]>=151.5){
					which_extruder_setup = 1;
					}else if (axis_steps_per_unit[E_AXIS]<=512 && axis_steps_per_unit[E_AXIS]>=492){
					which_extruder_setup = 2;
					}else{
					which_extruder_setup = -1;
				}
				which_hotend_setup[0] =(uint8_t)(hotend_size_setup[0]*10);
				which_hotend_setup[1] = (uint8_t)(hotend_size_setup[1]*10);
				show_data_printconfig();
			}
			break;
			
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3:
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3,1);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10,0);
			which_hotend_setup[which_extruder]=3;
			break;
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4:
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4,1);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10,0);
			which_hotend_setup[which_extruder]=4;
			break;
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5:
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5,1);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10,0);
			which_hotend_setup[which_extruder]=5;
			break;
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6:
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6,1);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10,0);
			which_hotend_setup[which_extruder]=6;
			break;
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8:
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8,1);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10,0);
			which_hotend_setup[which_extruder]=8;
			break;
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10:
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10,1);
			which_hotend_setup[which_extruder]=10;
			break;
			
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_SAVE:
			switch(which_hotend_setup[0]){
				case 3:
				hotend_size_setup[0] = 0.3;
				break;
				case 4:
				hotend_size_setup[0] = 0.4;
				break;
				case 5:
				hotend_size_setup[0] = 0.5;
				break;
				case 6:
				hotend_size_setup[0] = 0.6;
				break;
				case 8:
				hotend_size_setup[0] = 0.8;
				break;
				case 10:
				hotend_size_setup[0] = 1.0;
				break;
				
				default:
				break;
			}
			switch(which_hotend_setup[1]){
				case 3:
				hotend_size_setup[1] = 0.3;
				break;
				case 4:
				hotend_size_setup[1] = 0.4;
				break;
				case 5:
				hotend_size_setup[1] = 0.5;
				break;
				case 6:
				hotend_size_setup[1] = 0.6;
				break;
				case 8:
				hotend_size_setup[1] = 0.8;
				break;
				case 10:
				hotend_size_setup[1] = 1.0;
				break;
				
				default:
				break;
			}
			if(FLAG_Printer_Setup_Assistant==888){
				#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
				display_ChangeForm((which_extruder==0 ? FORM_PRINTERSETUPASSISTANT_RHOTEND:FORM_PRINTERSETUPASSISTANT_EXTRUDER),0);
				#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
				if(which_extruder == 0){
					display_ChangeForm(FORM_PRINTERSETUPASSISTANT_RHOTEND,0);
					}else{
					display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,0);
					FLAG_Printer_Setup_Assistant = 1888;
					display_ChangeForm(FORM_SETUPASSISTANT_SUCCESS,0);
					gif_processing_state = PROCESSING_SUCCESS_FIRST_RUN;
				}
				#endif
				
				}else if(flag_utilities_maintenance_changehotend == 1888 || flag_utilities_maintenance_changehotend == 2888){
				display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_SUCCESS,0);
			}
			else {
				show_data_printconfig();
			}
			#ifdef DEV_BER
			enquecommand_P(PSTR("M841"));
			#endif			
			Config_StoreSettings();
			break;
			
			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS_2:
			if(Flag_FRS_enabled){
				Flag_FRS_enabled = false;
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS_2,0);
				}else{
				Flag_FRS_enabled = true;
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS_2,1);
			}
			Config_StoreSettings();
			break;
			#endif
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS:
			if(Flag_FRS_enabled){
				Flag_FRS_enabled = false;
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS,0);
				}else{
				Flag_FRS_enabled = true;
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS,1);
			}
			Config_StoreSettings();
			break;
			
			#ifdef ENCLOSURE_SAFETY_STOP
						
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_ENCLOSURE:
			if(Flag_enclosure_enabled){
				Flag_enclosure_enabled = false;
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_ENCLOSURE,0);
				}else{
				Flag_enclosure_enabled = true;
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_ENCLOSURE,1);
			}
			Config_StoreSettings();
			break;
			#endif
			
			//case BUTTON_PRINTERSETUP_PRINTERCONFIG_LHOTEND:
			//case BUTTON_PRINTERSETUP_PRINTERCONFIG_RHOTEND:
			//
			//if(LCD_FSM_input_buton_flag == BUTTON_PRINTERSETUP_PRINTERCONFIG_LHOTEND) which_extruder = 0;
			//else which_extruder = 1;
			//
			////display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_SAVE,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10,0);
			//switch((int)(which_hotend_setup[which_extruder])){
			//case 3:
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3,1);
			//break;
			//case 4:
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4,1);
			//break;
			//case 5:
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5,1);
			//break;
			//case 6:
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6,1);
			//break;
			//case 8:
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8,1);
			//break;
			//case 10:
			//display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10,1);
			//break;
			//default:
			//break;
			//
			//}
			//display_ChangeForm(FORM_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE,0);
			//break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND:
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_HOTEND_LEFT,0);
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_HOTEND_RIGHT,0);
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SKIP,0);
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_UNLOAD,0);
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_CHANGEHOTEND,0);
			display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND,0);
			bitSet(flag_utilities_maintenance_register,flag_utilities_maintenance_register_changehotend);
			which_extruder = 255;
			break;
			
			case BUTTON_PRINTERSETUP_LED_SAVE:
			display_ChangeForm(FORM_PRINTERSETUP,0);
			Config_StoreSettings();
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_MENU:
			case BUTTON_PRINTERSETUP_LED_MENU:
			bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_changehotend);
			display_ChangeForm(FORM_MAIN,0);
			screen_sdcard = false;
			surfing_utilities=false;
			
			SERIAL_PROTOCOLLNPGM("Main");
			
			surfing_temps = false;
			break;
			
			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_BACK_2:
			#endif
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_BACK:
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_BACK:
			//case BUTTON_PRINTERSETUP_LED_BACK:
			bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_changehotend);
			display_ChangeForm(FORM_PRINTERSETUP,0);
			break;
			
			
			//case BUTTON_PRINTERSETUP_CHANGEHOTEND_ASK_YES:
			//display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE,0);
			//if(flag_utilities_maintenance_changehotend == 1888)which_extruder = LEFT_EXTRUDER;
			//else if(flag_utilities_maintenance_changehotend == 2888)which_extruder = RIGHT_EXTRUDER;
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_3,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_4,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_5,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_6,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_8,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_10,0);
			//switch((int)(hotend_size_setup[which_extruder]*10)){
			//case 3:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_3,1);
			//break;
			//case 4:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_4,1);
			//break;
			//case 5:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_5,1);
			//break;
			//case 6:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_6,1);
			//break;
			//case 8:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_8,1);
			//break;
			//case 10:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_10,1);
			//break;
			//default:
			//break;
			//
			//}
			//break;
			//case BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_ACCEPT:
			//display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_SUCCESS,0);
			//break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_SUCCESS_NEXT:
			display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_LOADFILAMENT,0);
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_ASK_NO:
			display_ChangeForm(FORM_MAIN,0);
			flag_utilities_maintenance_changehotend = 888;
			Config_StoreSettings();
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_AUTOTUNE_GO:
			display_ChangeForm( FORM_ADJUSTING_TEMPERATURES, 0);
			gif_processing_state = PROCESSING_ADJUSTING;
			percentage = 0;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, percentage);
			PID_autotune_Save(flag_utilities_maintenance_changehotend==1888?print_temp_l:print_temp_r, flag_utilities_maintenance_changehotend==1888?0:1, AUTOTUNE_ITERATIONS, 25.0);
			flag_utilities_maintenance_changehotend = 3888;
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
			display_ChangeForm( FORM_UTILITIES_CALIBRATION_SUCCESS, 0);
			gif_processing_state = PROCESSING_SUCCESS;
			printer_state = STATE_AUTOTUNEPID;
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_LOADFILAMENT_GO:
			filament_mode = 'I';
			cycle_filament = 0;
			if(which_extruder ==  1){
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1, 1);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0, 0);
			}
			else{
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT1, 0);
				display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_SELECT0, 1);
			}
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_MENU, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_BACK, 0);
			display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD,0);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILNEXT, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FILBACK, 1);
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"    PLA");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL2,"    PVA");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL3,"  PET-G");

			surfing_utilities = true;
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_LOADFILAMENT_SKIP:
			display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_LOADFILAMENT_SURE,0);
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_LOADFILAMENT_SURE_YES:
			display_ChangeForm(FORM_MAIN,0);
			flag_utilities_maintenance_changehotend = 888;
			Config_StoreSettings();
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_LOADFILAMENT_SURE_NO:
			display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_LOADFILAMENT,0);
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_SUCCESS_SKIP:
			display_ChangeForm(FORM_MAIN,0);
			flag_utilities_maintenance_changehotend = 888;
			Config_StoreSettings();
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_AUTOTUNE_SKIP:
			display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_AUTOTUNE_ASK,0);
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_AUTOTUNE_ASK_NO:
			display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_AUTOTUNE,0);
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_AUTOTUNE_ASK_YES:
			display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_CALIBRATION,0);
			flag_utilities_maintenance_changehotend = 888;
			Config_StoreSettings();
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_CALIBRATION_ASK_NO:
			display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_CALIBRATION,0);
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_CALIBRATION_ASK_YES:
			display_ChangeForm(FORM_MAIN,0);
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_CALIBRATION_SKIP:
			display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_CALIBRATION_ASK,0);
			break;
			
			
			
			//case BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_3:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_3,1);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_4,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_5,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_6,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_8,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_10,0);
			//hotend_size_setup[which_extruder]=0.3;
			//break;
			//case BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_4:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_3,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_4,1);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_5,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_6,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_8,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_10,0);
			//hotend_size_setup[which_extruder]=0.4;
			//break;
			//case BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_5:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_3,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_4,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_5,1);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_6,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_8,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_10,0);
			//hotend_size_setup[which_extruder]=0.5;
			//break;
			//case BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_6:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_3,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_4,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_5,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_6,1);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_8,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_10,0);;
			//hotend_size_setup[which_extruder]=0.6;
			//break;
			//case BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_8:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_3,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_4,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_5,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_6,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_8,1);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_10,0);
			//hotend_size_setup[which_extruder]=0.8;
			//break;
			//case BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_10:
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_3,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_4,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_5,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_6,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_8,0);
			//display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SELECTSIZE_10,1);
			//hotend_size_setup[which_extruder]=1.0;
			//break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_HOTEND_LEFT:
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_HOTEND_LEFT,1);
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_HOTEND_RIGHT,0);
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SKIP,1);
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_UNLOAD,1);
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_CHANGEHOTEND,1);
			which_extruder = 0;
			break;
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_HOTEND_RIGHT:
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_HOTEND_LEFT,0);
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_HOTEND_RIGHT,1);
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_SKIP,1);
			display_ButtonState(BUTTON_PRINTERSETUP_CHANGEHOTEND_UNLOAD,1);
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_CHANGEHOTEND,1);
			which_extruder = 1;
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_UNLOAD:
			if(which_extruder == 0 || which_extruder == 1){
				filament_mode = 'R';
				unload_get_ready();
			}
			break;
			
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_SKIP:
			
			if(which_extruder == LEFT_EXTRUDER){
				flag_utilities_maintenance_changehotend = 1888;
				}else if(which_extruder == RIGHT_EXTRUDER){
				flag_utilities_maintenance_changehotend = 2888;
			}
			if(which_extruder == 0 || which_extruder == 1){
				bitSet(flag_utilities_calibration_register,flag_utilities_calibration_register_cooldown_mode1);
			}
			break;
			
			case BUTTON_MAINTENANCENOTICE:
			display_ChangeForm(FORM_MAIN,0);
			Config_StoreSettings();
			break;
			
			
			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_RHOTEND_2:
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_LHOTEND_2:
			#endif
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_RHOTEND:
			case BUTTON_PRINTERSETUP_PRINTERCONFIG_LHOTEND:
			case BUTTON_PRINTERSETUPASSISTANT_LHOTEND:
			case BUTTON_PRINTERSETUPASSISTANT_RHOTEND:
			case BUTTON_PRINTERSETUP_CHANGEHOTEND_ASK_YES:
			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			if(LCD_FSM_input_buton_flag == BUTTON_PRINTERSETUP_PRINTERCONFIG_LHOTEND || LCD_FSM_input_buton_flag == BUTTON_PRINTERSETUP_PRINTERCONFIG_LHOTEND_2){
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_DESCRIPTION,0);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_HEADER,0);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_SAVE,0);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,0);
				which_extruder = LEFT_EXTRUDER;
			}
			else if(LCD_FSM_input_buton_flag == BUTTON_PRINTERSETUP_PRINTERCONFIG_RHOTEND || LCD_FSM_input_buton_flag == BUTTON_PRINTERSETUP_PRINTERCONFIG_RHOTEND_2){
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_DESCRIPTION,0);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_HEADER,1);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_SAVE,0);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,0);
				which_extruder = RIGHT_EXTRUDER;
			}
			#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
			if(LCD_FSM_input_buton_flag == BUTTON_PRINTERSETUP_PRINTERCONFIG_LHOTEND){
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_DESCRIPTION,0);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_HEADER,0);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_SAVE,0);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,0);
				which_extruder = LEFT_EXTRUDER;
			}
			else if(LCD_FSM_input_buton_flag == BUTTON_PRINTERSETUP_PRINTERCONFIG_RHOTEND){
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_DESCRIPTION,0);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_HEADER,1);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_SAVE,0);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,0);
				which_extruder = RIGHT_EXTRUDER;
			}
			#endif
			else if(flag_utilities_maintenance_changehotend == 1888){
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_DESCRIPTION,1);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_HEADER,2);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_SAVE,1);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,1);
				which_extruder = LEFT_EXTRUDER;
			}
			else if(flag_utilities_maintenance_changehotend == 2888){
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_DESCRIPTION,1);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_HEADER,2);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_SAVE,1);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,1);
				which_extruder = RIGHT_EXTRUDER;
			}
			else if(LCD_FSM_input_buton_flag == BUTTON_PRINTERSETUPASSISTANT_LHOTEND){
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_DESCRIPTION,1);
				#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_HEADER,3);
				#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_HEADER,4);
				#endif
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_SAVE,1);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,1);
				which_extruder = LEFT_EXTRUDER;
			}
			else if(LCD_FSM_input_buton_flag == BUTTON_PRINTERSETUPASSISTANT_RHOTEND){
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_DESCRIPTION,1);
				#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_HEADER,3);
				#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_HEADER,4);
				#endif
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_SAVE,1);
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_BACK,1);
				which_extruder = RIGHT_EXTRUDER;
			}
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8,0);
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10,0);
			switch((int)(hotend_size_setup[which_extruder]*10)){
				case 3:
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_3,1);
				break;
				case 4:
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_4,1);
				break;
				case 5:
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_5,1);
				break;
				case 6:
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_6,1);
				break;
				case 8:
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_8,1);
				break;
				case 10:
				display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE_10,1);
				break;
				default:
				break;
				
			}
			display_ChangeForm(FORM_PRINTERSETUP_PRINTERCONFIG_SELECTSIZE,0);
			break;
			#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
			case BUTTON_PRINTERSETUPASSISTANT_EXTRUDER:
			display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_BACK,1);
			display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_SAVE,1);
			if (which_extruder_setup == 1){
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_1,1);
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_2,0);
				}else if (which_extruder_setup == 2){
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_2,1);
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_1,0);
			}
			
			else{
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_1,0);
				display_ButtonState(BUTTON_PRINTERSETUP_EXTRUDERCONFIG_CONFIG_2,0);
			}
			display_ChangeForm(FORM_PRINTERSETUP_EXTRUDERCONFIG,0);
			break;
			#endif
			
			#ifdef DEV_BER
			case BUTTON_PRINT_ASK_FEEDBACK_YES:
			display_ChangeForm(FORM_MAIN,0);
			copy_message(copy_success_true);
			copy_message(copy_status_done);
			Send_feedback_flag = false;
			
			break;
			/*
			case BUTTON_PRINT_ASK_FEEDBACK_REASONS_ACCEPT:
			if(print_feedback_fail_flag){
				Send_feedback_flag = false;
				display_ChangeForm(FORM_MAIN,0);
				Serial.println(F("[M842]{\"status\": \"done\"}"));
				print_feedback_fail_flag = false;
			}
			break;
			*/
			
			case BUTTON_PRINT_ASK_FEEDBACK_NO:
			display_ChangeForm(FORM_PRINT_ASK_FEEDBACK_RETRY,0);
			copy_message(copy_success_false);
			break;
			
			/*display_ChangeForm(FORM_PRINT_ASK_FEEDBACK_REASONS,0);
			Serial.println(F("[M842]{\"success\": false}"));
			print_feedback_fail_flag = false;
					
			break;*/	
			case BUTTON_PRINT_REMOVE_NEXT:
			{
				if(StatusJob.get_status() == 0){
					display_ChangeForm(FORM_PRINT_ASK_FEEDBACK,0);
				}else{
					display_ChangeForm(FORM_PRINT_ASK_FEEDBACK_RETRY,0);
				}
				StatusJob.set_status(0);
				
			}
			break;
			
			case BUTTON_PRINT_ASK_FEEDBACK_RETRY_NOW:
			display_ChangeForm(FORM_MAIN,0);
			copy_message(copy_retry_now);
			copy_message(copy_status_done);
			Send_feedback_flag = false;
			break;
			
			case BUTTON_PRINT_ASK_FEEDBACK_RETRY_LATER:
			display_ChangeForm(FORM_MAIN,0);
			copy_message(copy_retry_later);
			copy_message(copy_status_done);
			Send_feedback_flag = false;
			break;
			
			case BUTTON_PRINT_ASK_FEEDBACK_RETRY_NO:
			display_ChangeForm(FORM_MAIN,0);
			copy_message(copy_retry_no);
			copy_message(copy_status_done);
			Send_feedback_flag = false;
			break;
			
			/*
			case BUTTON_PRINT_ASK_FEEDBACK_REASONS_DOWN:
			Serial.println(F("[M842]{\"pg\": \"down\"}"));
			break;
			
			case BUTTON_PRINT_ASK_FEEDBACK_REASONS_UP:
			Serial.println(F("[M842]{\"pg\": \"up\"}"));
			break;
			
			case BUTTON_PRINT_ASK_FEEDBACK_REASONS_SEL1:
			Serial.println(F("[M842]{\"failreason\": 0}"));
			print_feedback_fail_flag = true;
			break;
			
			case BUTTON_PRINT_ASK_FEEDBACK_REASONS_SEL2:
			Serial.println(F("[M842]{\"failreason\": 1}"));
			print_feedback_fail_flag = true;
			break;
			
			case BUTTON_PRINT_ASK_FEEDBACK_REASONS_SEL3:
			Serial.println(F("[M842]{\"failreason\": 2}"));
			print_feedback_fail_flag = true;
			break;
			
			case BUTTON_PRINT_ASK_FEEDBACK_REASONS_SEL4:
			Serial.println(F("[M842]{\"failreason\": 3}"));
			print_feedback_fail_flag = true;
			break;
			
			case BUTTON_PRINT_ASK_FEEDBACK_REASONS_SEL5:
			Serial.println(F("[M842]{\"failreason\": 4}"));
			print_feedback_fail_flag = true;
			break;
			*/
			case BUTTON_FILAMENTDETECTOR_ABORT_NO:
			StatusJob.set_status(1); // Confirm
			display_ChangeForm(FORM_PRINT_SERIAL,0);
			display.WriteStr(STRING_PRINT_SERIAL_GCODE,StatusJob.printjobname);
			bitSet(flag_serialprint_register,flag_serialprint_register_datarefresh);
			break;
			
			case BUTTON_FILAMENTDETECTOR_ABORT_YES:
			StatusJob.set_status(5); // Confirm
			copy_message(copy_print_cancel);
			display_ChangeForm(FORM_PROCESSING,0);
			break;
			
			case BUTTON_PRINT_SERIAL_CANCEL:
			StatusJob.set_status(0); // Confirm
			display_ChangeForm(FORM_FILAMENTDETECTOR_ABORT,0);
			break;
			
			case BUTTON_PRINT_SERIAL_PAUSERESUME:
			if(StatusJob.get_status() == 1){
				StatusJob.set_status(2);
				copy_message(copy_print_pause);
				display_ChangeForm(FORM_PROCESSING,0);
			}else if(StatusJob.get_status() == 4){
				StatusJob.set_status(3);
				copy_message(copy_print_resume);
				display_ChangeForm(FORM_PROCESSING,0);
			}
			break;
			
			#endif
			
			default:
			break;
			



		}//endswitch

	

	}// else
	
	lcd_busy = false;
	LCD_FSM_input_buton_flag = -1;
	
}
void lcd_fsm_output_logic(){//We process tasks according to the present state
	#ifdef DEV_BER	
	if((card.sdprinting && !card.sdispaused) || (!card.sdprinting && card.sdispaused) )
	{
		update_screen_printing();//STATE PRINTING
		}else if(screen_sdcard){
		update_screen_sdcard();//STATE LIST SDGCODES
		}else if(flag_ending_gcode){
		update_screen_endinggcode();//STATE ENDING PRINTING
		}else if(flag_serial_gcode){
		update_screen_serial_printing();//STATE SERIAL PRINT
		}else{
		update_screen_noprinting();//STATE NO PRINTING
	}	
	#else
	if((card.sdprinting && !card.sdispaused) || (!card.sdprinting && card.sdispaused) )
	{
		update_screen_printing();//STATE PRINTING
		}else if(screen_sdcard){
		update_screen_sdcard();//STATE LIST SDGCODES
		}else if(flag_ending_gcode){
		update_screen_endinggcode();//STATE ENDING PRINTING
		}else{
		update_screen_noprinting();//STATE NO PRINTING
	}
	#endif
	
	
	
}
void update_screen_endinggcode(){
	
	if(buflen < BUFSIZE){
		flag_ending_gcode = false;
		enquecommand_P(PSTR("M990"));
	}
	
}
#ifdef DEV_BER
void update_screen_serial_printing(){
	
	 static uint32_t waitPeriod = millis();
	 static uint32_t waitPeriod_p = millis();
	
	 
	// gif animation
	static int8_t processing_state = 0;
	switch(StatusJob.get_status()){
		case 1:
		case 4:
		{
			if (millis() >= waitPeriod)
			{
				
				
				static unsigned int percentDone = 0;
				static unsigned int minuteremaining = 0;
				
				if (StatusJob.get_temp0() !=(int)(degHotend(0)+0.5) || bitRead(flag_serialprint_register,flag_serialprint_register_datarefresh)){
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINT_SERIAL_HOTEND0,(int)(degHotend(0)+0.5));
				}
				if (StatusJob.get_temp1() !=(int)(degHotend(1)+0.5) || bitRead(flag_serialprint_register,flag_serialprint_register_datarefresh)){
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINT_SERIAL_HOTEND1,(int)(degHotend(1)+0.5));
				}
				if (StatusJob.get_tempbed() !=(int)(degBed()+0.5) || bitRead(flag_serialprint_register,flag_serialprint_register_datarefresh)){
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINT_SERIAL_BED,(int)(degBed()+0.5));
				}
				if (percentDone != StatusJob.get_progress() || bitRead(flag_serialprint_register,flag_serialprint_register_datarefresh)){
					percentDone = StatusJob.get_progress();
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINT_SERIAL_PERCENTAGE,percentDone);
				}
				if ( minuteremaining != StatusJob.get_remaining_min() || bitRead(flag_serialprint_register,flag_serialprint_register_datarefresh)){
					minuteremaining = StatusJob.get_remaining_min();
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINT_SERIAL_TIMEREMAINING_MINS,minuteremaining);
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINT_SERIAL_TIMEREMAINING_HOURS,StatusJob.get_remaining_h());
				}
				bitClear(flag_serialprint_register,flag_serialprint_register_datarefresh);
				
				StatusJob.update_temps();
				//StatusJob.calc_remaining_time(3600*listsd.get_hoursremaining()+60*listsd.get_minutesremaining());
				
				
				waitPeriod=1500+millis();	//Every 1.5s
				
			}
		}
		break;
		
		case 2:
		case 3:
		case 5:
		{
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state<GIF_FRAMES_PROCESSING) ? processing_state + 3 : 0;
				display_SetFrame(GIF_PROCESSING,processing_state);
				
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
		}
		break;
		
		default:
		break;
		
	}
}
#endif
void update_screen_printing(){
	static uint32_t waitPeriod = millis();
	static uint32_t waitPeriod_inactive = millis();
	
	// Save last Temps
	static int last_target_temperature_hot0 = target_temperature[0];
	static int last_target_temperature_hot1 = target_temperature[1];
	static int last_target_temperature_bed = target_temperature_bed;
	
	// Save last Advanced Settings
	static int last_fan0 = (int)(round((100.0*fanSpeed)/255.0)+fanSpeed_offset[LEFT_EXTRUDER]);
	static int last_fan1 = (int)(round((100.0*fanSpeed)/255.0)+fanSpeed_offset[RIGHT_EXTRUDER]);
	static int last_flow0 = extruder_multiply[LEFT_EXTRUDER];
	static int last_flow1 = extruder_multiply[RIGHT_EXTRUDER];
	static int last_speed0 = feedmultiply[LEFT_EXTRUDER];
	static int last_speed1 = feedmultiply[RIGHT_EXTRUDER];
	if (millis() >= waitPeriod_inactive){
		
		time_inactive_extruder[!active_extruder] += 1;// 1 second
		waitPeriod_inactive=1000+millis();
	}
	if(bitRead(flag_sdprinting_register,flag_sdprinting_register_temps)){
		display_ChangeForm(FORM_SDPRINTTING_TEMPS,0);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_LEFT,target_temperature[0]);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_BED,target_temperature_bed);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_RIGHT,target_temperature[1]);
		is_printing_screen = false;
		bitClear(flag_sdprinting_register,flag_sdprinting_register_temps);
	}
	if(screen_printing_pause_form == screen_printing_pause_form4){
		if(last_target_temperature_hot0 != target_temperature[0] ||
		last_target_temperature_hot1 != target_temperature[1] ||
		last_target_temperature_bed != target_temperature_bed){
			last_target_temperature_hot0 = target_temperature[0];
			last_target_temperature_hot1 = target_temperature[1];
			last_target_temperature_bed = target_temperature_bed;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_LEFT,last_target_temperature_hot0);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_BED,last_target_temperature_bed);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_RIGHT,last_target_temperature_hot1);
		}
	}
	if(bitRead(flag_sdprinting_register,flag_sdprinting_register_light)){
		display_ButtonState(BUTTON_PRINTERSETUP_LED_MENU,1);
		display.WriteObject(GENIE_OBJ_ISMARTSLIDER,SMARTSLIDER_PRINTERSETUP_LED_BRIGHTNESS,led_brightness);
		display_ChangeForm(FORM_PRINTERSETUP_LED,0);
		bitClear(flag_sdprinting_register,flag_sdprinting_register_light);
	}
	
	if(bitRead(flag_sdprinting_register,flag_sdprinting_register_control)){
		display_ChangeForm(FORM_SDPRINTTING_CONTROL,0);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_LEFT,(int)(round((100.0*fanSpeed)/255.0)+fanSpeed_offset[LEFT_EXTRUDER]));
		
		if(get_dual_x_carriage_mode() == DXC_FULL_SIGMA_MODE){
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_RIGHT,(int)(round((100.0*fanSpeed)/255.0)+fanSpeed_offset[RIGHT_EXTRUDER]));
			}else{
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_RIGHT,(int)(round((100.0*fanSpeed)/255.0)+fanSpeed_offset[LEFT_EXTRUDER]));
		}
		
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FLOW_LEFT,extruder_multiply[LEFT_EXTRUDER]);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FLOW_RIGHT,extruder_multiply[RIGHT_EXTRUDER]);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_SPEED_LEFT,feedmultiply[LEFT_EXTRUDER]);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_SPEED_RIGHT,feedmultiply[RIGHT_EXTRUDER]);
		
		waitPeriod=1500+millis();
		is_printing_screen = false;
		bitClear(flag_sdprinting_register,flag_sdprinting_register_control);
	}
	if(screen_printing_pause_form == screen_printing_pause_form5){
		if(last_fan0 != (int)(round((100.0*fanSpeed)/255.0)+fanSpeed_offset[LEFT_EXTRUDER]) ||
		last_fan1 != (int)(round((100.0*fanSpeed)/255.0)+fanSpeed_offset[(get_dual_x_carriage_mode() == DXC_FULL_SIGMA_MODE?RIGHT_EXTRUDER:LEFT_EXTRUDER)]) ||
		last_flow0 != extruder_multiply[LEFT_EXTRUDER] ||
		last_flow1 != extruder_multiply[RIGHT_EXTRUDER] ||
		last_speed0 != feedmultiply[LEFT_EXTRUDER] ||
		last_speed1 != feedmultiply[RIGHT_EXTRUDER]){
			last_fan0 = (int)(round((100.0*fanSpeed)/255.0)+fanSpeed_offset[LEFT_EXTRUDER]);
			last_fan1 = (int)(round((100.0*fanSpeed)/255.0)+fanSpeed_offset[(get_dual_x_carriage_mode() == DXC_FULL_SIGMA_MODE?RIGHT_EXTRUDER:LEFT_EXTRUDER)]);
			last_flow0 = extruder_multiply[LEFT_EXTRUDER];
			last_flow1 = extruder_multiply[RIGHT_EXTRUDER];
			last_speed0 = feedmultiply[LEFT_EXTRUDER];
			last_speed1 = feedmultiply[RIGHT_EXTRUDER];
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_LEFT,last_fan0);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_RIGHT,last_fan1);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FLOW_LEFT,last_flow0);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FLOW_RIGHT,last_flow1);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_SPEED_LEFT,last_speed0);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_SPEED_RIGHT,last_speed1);
		}
		
	}
	if(bitRead(flag_sdprinting_register,flag_sdprinting_register_showdata)){
		if (screen_printing_pause_form == screen_printing_pause_form0 || screen_printing_pause_form == screen_printing_pause_form3){
			display_ChangeForm(FORM_SDPRINTING,0);
			surfing_utilities = false;
			display.WriteStr(STRING_SDPRINTING_GCODE,namefilegcode);
		}
		else{
			display_ChangeForm(FORM_SDPRINTING_PAUSE,0);
			surfing_utilities = false;
			display.WriteStr(STRING_SDPRINTING_PAUSE_GCODE,namefilegcode);
		}
		bitSet(flag_sdprinting_register,flag_sdprinting_register_datarefresh);
		//waitPeriod=1500+millis();	//Every 1.5s
		bitClear(flag_sdprinting_register,flag_sdprinting_register_showdata);
		
	}
	if(bitRead(screen_change_register,screen_change_register_nozz1up)){
		if (target_temperature[0] < HEATER_0_MAXTEMP)
		{
			#ifdef RELATIVE_TEMP_PRINT
			if(Flag_hotend0_relative_temp){
				hotend0_relative_temp +=5;
				target_temperature[0]+=5;
				}else{
				target_temperature[0]+=5;
			}
			#else
			target_temperature[0]+=5;
			#endif
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_LEFT,target_temperature[0]);
			
		}
		bitClear(screen_change_register,screen_change_register_nozz1up);
	}
	if(bitRead(screen_change_register,screen_change_register_nozz2up)){
		if (target_temperature[1]<HEATER_1_MAXTEMP)
		{
			#ifdef RELATIVE_TEMP_PRINT
			if(Flag_hotend1_relative_temp){
				hotend1_relative_temp +=5;
				target_temperature[1]+=5;
				}else{
				target_temperature[1]+=5;
			}
			#else
			target_temperature[1]+=5;
			#endif
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_RIGHT,target_temperature[1]);
			
		}
		bitClear(screen_change_register,screen_change_register_nozz2up);
	}
	if(bitRead(screen_change_register,screen_change_register_bedup)){
		if (target_temperature_bed < BED_MAXTEMP)//MaxTemp
		{
			target_temperature_bed+=5;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_BED,target_temperature_bed);
			
		}
		
		bitClear(screen_change_register,screen_change_register_bedup);
	}
	if(bitRead(screen_change_register,screen_change_register_speedup)){
		if (feedmultiply[LEFT_EXTRUDER]<200)
		{
			feedmultiply[LEFT_EXTRUDER]+=5;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_SPEED_LEFT,feedmultiply[LEFT_EXTRUDER]);
			if(get_dual_x_carriage_mode()!=DXC_FULL_SIGMA_MODE){
				feedmultiply[RIGHT_EXTRUDER]=feedmultiply[LEFT_EXTRUDER];
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_SPEED_RIGHT,feedmultiply[LEFT_EXTRUDER]);
			}
		}
		bitClear(screen_change_register,screen_change_register_speedup);
	}
	if(bitRead(screen_change_register,screen_change_register_speedupr)){
		if (feedmultiply[RIGHT_EXTRUDER]<200)
		{
			feedmultiply[RIGHT_EXTRUDER]+=5;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_SPEED_RIGHT,feedmultiply[RIGHT_EXTRUDER]);
			
		}
		bitClear(screen_change_register,screen_change_register_speedupr);
	}
	if(bitRead(screen_change_register,screen_change_register_flowup)){
		if (extruder_multiply[LEFT_EXTRUDER]<150)
		{
			extruder_multiply[LEFT_EXTRUDER]+=1;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FLOW_LEFT,extruder_multiply[LEFT_EXTRUDER]);
			if(get_dual_x_carriage_mode()!=DXC_FULL_SIGMA_MODE){
				extruder_multiply[RIGHT_EXTRUDER]=extruder_multiply[LEFT_EXTRUDER];
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FLOW_RIGHT,extruder_multiply[LEFT_EXTRUDER]);
			}
		}
		bitClear(screen_change_register,screen_change_register_flowup);
	}
	if(bitRead(screen_change_register,screen_change_register_flowupr)){
		if (extruder_multiply[RIGHT_EXTRUDER]<150)
		{
			extruder_multiply[RIGHT_EXTRUDER]+=1;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FLOW_RIGHT,extruder_multiply[RIGHT_EXTRUDER]);
		}
		bitClear(screen_change_register,screen_change_register_flowupr);
	}
	if(bitRead(screen_change_register,screen_change_register_fanup)){
		if ((((int)round((100.0*fanSpeed)/255.0)+fanSpeed_offset[LEFT_EXTRUDER])*255)/100 < 255)
		{
			int fspeed=0;
			fanSpeed_offset[LEFT_EXTRUDER]=constrain(fanSpeed_offset[LEFT_EXTRUDER]+5,-100,100);
			fspeed=constrain((int)((round((100.0*fanSpeed)/255.0)+fanSpeed_offset[LEFT_EXTRUDER])*255)/100,0,255);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_LEFT,(int)round((100.0*fspeed)/255.0));
			if(get_dual_x_carriage_mode()!=DXC_FULL_SIGMA_MODE){
				fanSpeed_offset[RIGHT_EXTRUDER]=fanSpeed_offset[LEFT_EXTRUDER];
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_RIGHT,(int)round((100.0*fspeed)/255.0));
			}
		}
		bitClear(screen_change_register,screen_change_register_fanup);
	}
	if(bitRead(screen_change_register,screen_change_register_fanupr)){
		if ((((int)round((100.0*fanSpeed)/255.0)+fanSpeed_offset[RIGHT_EXTRUDER])*255)/100 < 255)
		{
			int fspeed=0;
			fanSpeed_offset[RIGHT_EXTRUDER]=constrain(fanSpeed_offset[RIGHT_EXTRUDER]+5,-100,100);
			fspeed=constrain((int)((round((100.0*fanSpeed)/255.0)+fanSpeed_offset[RIGHT_EXTRUDER])*255)/100,0,255);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_RIGHT,(int)round((100.0*fspeed)/255.0));
		}
		bitClear(screen_change_register,screen_change_register_fanupr);
	}
	if(bitRead(screen_change_register,screen_change_register_nozz1down)){
		if (target_temperature[0] > HEATER_0_MINTEMP)
		{
			#ifdef RELATIVE_TEMP_PRINT
			if(Flag_hotend0_relative_temp){
				hotend0_relative_temp -=5;
				target_temperature[0]-=5;
				}else{
				target_temperature[0]-=5;
			}
			#else
			target_temperature[0]-=5;
			#endif
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_LEFT,target_temperature[0]);
			
		}
		
		bitClear(screen_change_register,screen_change_register_nozz1down);
	}
	if(bitRead(screen_change_register,screen_change_register_nozz2down)){
		if (target_temperature[1]>HEATER_1_MINTEMP)
		{
			#ifdef RELATIVE_TEMP_PRINT
			if(Flag_hotend1_relative_temp){
				hotend1_relative_temp -=5;
				target_temperature[1]-=5;
				}else{
				target_temperature[1]-=5;
			}
			#else
			target_temperature[1]-=5;
			#endif
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_RIGHT,target_temperature[1]);
			
		}
		
		bitClear(screen_change_register,screen_change_register_nozz2down);
	}
	if(bitRead(screen_change_register,screen_change_register_beddown)){
		if (target_temperature_bed> BED_MINTEMP)//Mintemp
		{
			target_temperature_bed-=5;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTTING_SETINGS_BED,target_temperature_bed);
			
		}
		
		bitClear(screen_change_register,screen_change_register_beddown);
	}
	if(bitRead(screen_change_register,screen_change_register_speeddown)){
		if (feedmultiply[LEFT_EXTRUDER]>50)
		{
			feedmultiply[LEFT_EXTRUDER]-=5;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_SPEED_LEFT,feedmultiply[LEFT_EXTRUDER]);
			if(get_dual_x_carriage_mode()!=DXC_FULL_SIGMA_MODE){
				feedmultiply[RIGHT_EXTRUDER]=feedmultiply[LEFT_EXTRUDER];
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_SPEED_RIGHT,feedmultiply[LEFT_EXTRUDER]);
			}
		}
		bitClear(screen_change_register,screen_change_register_speeddown);
	}
	if(bitRead(screen_change_register,screen_change_register_speeddownr)){
		if (feedmultiply[RIGHT_EXTRUDER]>50)
		{
			feedmultiply[RIGHT_EXTRUDER]-=5;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_SPEED_RIGHT,feedmultiply[RIGHT_EXTRUDER]);
			
		}
		bitClear(screen_change_register,screen_change_register_speeddownr);
	}
	if(bitRead(screen_change_register,screen_change_register_flowdown)){
		if (extruder_multiply[LEFT_EXTRUDER]>50)
		{
			extruder_multiply[LEFT_EXTRUDER]-=1;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FLOW_LEFT,extruder_multiply[LEFT_EXTRUDER]);
			if(get_dual_x_carriage_mode()!=DXC_FULL_SIGMA_MODE){
				extruder_multiply[RIGHT_EXTRUDER]=extruder_multiply[LEFT_EXTRUDER];
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FLOW_RIGHT,extruder_multiply[LEFT_EXTRUDER]);
			}
			
		}
		bitClear(screen_change_register,screen_change_register_flowdown);
	}
	if(bitRead(screen_change_register,screen_change_register_flowdownr)){
		if (extruder_multiply[RIGHT_EXTRUDER]>50)
		{
			extruder_multiply[RIGHT_EXTRUDER]-=1;
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FLOW_RIGHT,extruder_multiply[RIGHT_EXTRUDER]);
			
		}
		bitClear(screen_change_register,screen_change_register_flowdownr);
	}
	if(bitRead(screen_change_register,screen_change_register_fandown)){
		if ((((int)round((100.0*fanSpeed)/255.0)+fanSpeed_offset[LEFT_EXTRUDER])*255)/100 > 0)
		{
			int fspeed=0;
			fanSpeed_offset[LEFT_EXTRUDER]=constrain(fanSpeed_offset[LEFT_EXTRUDER]-5,-100,100);
			fspeed=constrain((int)((round((100.0*fanSpeed)/255.0)+fanSpeed_offset[LEFT_EXTRUDER])*255)/100,0,255);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_LEFT,(int)round((100.0*fspeed)/255.0));
			if(get_dual_x_carriage_mode()!=DXC_FULL_SIGMA_MODE){
				fanSpeed_offset[RIGHT_EXTRUDER]=fanSpeed_offset[LEFT_EXTRUDER];
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_RIGHT,(int)round((100.0*fspeed)/255.0));
			}
		}
		bitClear(screen_change_register,screen_change_register_fandown);
	}
	if(bitRead(screen_change_register,screen_change_register_fandownr)){
		if ((((int)round((100.0*fanSpeed)/255.0)+fanSpeed_offset[RIGHT_EXTRUDER])*255)/100 > 0)
		{
			int fspeed=0;
			fanSpeed_offset[RIGHT_EXTRUDER]=constrain(fanSpeed_offset[RIGHT_EXTRUDER]-5,-100,100);
			fspeed=constrain((int)((round((100.0*fanSpeed)/255.0)+fanSpeed_offset[RIGHT_EXTRUDER])*255)/100,0,255);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_SETTINGS_FAN_RIGHT,(int)round((100.0*fspeed)/255.0));
			
		}
		bitClear(screen_change_register,screen_change_register_fandownr);
	}
	#ifdef ENCLOSURE_SAFETY_STOP
	if(bitRead(flag_sdprinting_register,flag_sdprinting_register_printpause) || Flag_enclosure_is_open == 1){
		if(!waiting_temps){
			card.pauseSDPrint();
			SERIAL_PROTOCOLLNPGM("PAUSE");
			bitSet(flag_sdprinting_register,flag_sdprinting_register_pausepause);
		}
		Flag_enclosure_is_open = 2;
		bitClear(flag_sdprinting_register,flag_sdprinting_register_printpause);
	}
	#else
	if(bitRead(flag_sdprinting_register,flag_sdprinting_register_printpause)){
		if(!waiting_temps){
			card.pauseSDPrint();
			SERIAL_PROTOCOLLNPGM("PAUSE");
			bitSet(flag_sdprinting_register,flag_sdprinting_register_pausepause);
		}
		bitClear(flag_sdprinting_register,flag_sdprinting_register_printpause);
	}
	#endif
	
	
	if(bitRead(flag_sdprinting_register,flag_sdprinting_register_printresume)){
						
		if(!waiting_temps){
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
			card.startFileprint();
			SERIAL_PROTOCOLLNPGM("RESUME");
			bitClear(flag_sdprinting_register,flag_sdprinting_register_pauseresume);
			if(bitRead(flag_sdprinting_register,flag_sdprinting_register_printresume)){
				enquecommand_P((PSTR("G70")));
				bitClear(flag_sdprinting_register,flag_sdprinting_register_printresume);
			}
			is_printing_screen = false;
		}
		bitClear(flag_sdprinting_register,flag_sdprinting_register_printresume);
	}
	
	if(bitRead(flag_sdprinting_register,flag_sdprinting_register_printstop) || bitRead(flag_sdprinting_register,flag_sdprinting_register_printsavejob)){
		
		if(!(card.sdispaused && (screen_printing_pause_form == screen_printing_pause_form1))){
			bufindw = (bufindr + 1)%BUFSIZE;
			buflen = 1;
		}
		
		doblocking =false;
		log_X0_mmdone += x0mmdone/axis_steps_per_unit[X_AXIS];
		log_X1_mmdone += x1mmdone/axis_steps_per_unit[X_AXIS];
		log_Y_mmdone += ymmdone/axis_steps_per_unit[Y_AXIS];
		log_E0_mmdone += e0mmdone/axis_steps_per_unit[E_AXIS];
		log_E1_mmdone += e1mmdone/axis_steps_per_unit[E_AXIS];
		x0mmdone = 0;
		x1mmdone = 0;
		ymmdone = 0;
		e0mmdone = 0;
		e1mmdone = 0;
		if(bitRead(flag_sdprinting_register,flag_sdprinting_register_printstop)){
			enquecommand_P(PSTR("M35"));
			bitClear(flag_sdprinting_register,flag_sdprinting_register_printstop);
			}else if(bitRead(flag_sdprinting_register,flag_sdprinting_register_printsavejob)){
			enquecommand_P(PSTR("G28 X0 Y0")); //Home X and Y
			bitClear(flag_sdprinting_register,flag_sdprinting_register_printsavejob);
		}
		
		acceleration = acceleration_old;
		feedmultiply[LEFT_EXTRUDER]=100;
		feedmultiply[RIGHT_EXTRUDER]=100;
		fanSpeed_offset[LEFT_EXTRUDER]=0;
		fanSpeed_offset[RIGHT_EXTRUDER]=0;
		extruder_multiply[LEFT_EXTRUDER]=100;
		extruder_multiply[RIGHT_EXTRUDER]=100;
		fanSpeed = 0;
		SERIAL_PROTOCOLLNPGM(MSG_STOP_PRINT);
		SERIAL_PROTOCOLLNPGM(MSG_FILE_PRINTED);
		Flag_fanSpeed_mirror = 0;
		cancel_heatup = true;
		#ifdef ENABLE_CURA_COUNTDOWN_TIMER
		flag_is_cura_file = false;
		#endif
		
		back_home = true;
		home_made = false;
		screen_sdcard = false;
		surfing_utilities=false;
		surfing_temps = false;
		card.sdprinting = false;
		card.sdispaused = false;
		Flag_checkfil = false;
		#ifdef RELATIVE_TEMP_PRINT
		Flag_hotend0_relative_temp = false;
		Flag_hotend1_relative_temp = false;
		#endif
		gif_processing_state = PROCESSING_STOP;
	}
	if (surfing_utilities)
	{
		if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect0)){
			bitClear(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect0);
			if(degHotend(purge_extruder_selected) >= target_temperature[purge_extruder_selected]-PURGE_TEMP_HYSTERESIS){
				gif_processing_state = PROCESSING_PURGE_LOAD;
				current_position[E_AXIS]+=PURGE_DISTANCE_INSERTED;
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_SLOW_SPEED/60, purge_extruder_selected);//Purge
				st_synchronize();
				disable_e0();
				disable_e1();
				gif_processing_state = PROCESSING_STOP;
				display_SetFrame(GIF_UTILITIES_FILAMENT_PURGE,0);
			}
		}
		if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect1)){
			bitClear(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect1);
			if(degHotend(purge_extruder_selected) >= target_temperature[purge_extruder_selected]-PURGE_TEMP_HYSTERESIS){
				gif_processing_state = PROCESSING_PURGE_UNLOAD;
				current_position[E_AXIS]-=5;
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_SLOW_SPEED/60, purge_extruder_selected);//Retract
				st_synchronize();
				disable_e0();
				disable_e1();
				gif_processing_state = PROCESSING_STOP;
				display_SetFrame(GIF_UTILITIES_FILAMENT_PURGE,0);
			}
		}
		if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_purgeload)){
			bitClear(flag_utilities_filament_register,flag_utilities_filament_register_purgeload);
			if(degHotend(which_extruder) >= target_temperature[which_extruder]-PURGE_TEMP_HYSTERESIS){
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_ADJUST_LOAD, 1);
				current_position[E_AXIS]+=PURGE_DISTANCE_INSERTED;
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_SLOW_SPEED/60, which_extruder);//Purge
				st_synchronize();
				disable_e0();
				disable_e1();
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_ADJUST_LOAD, 0);
			}
		}
		if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_purgeunload)){
			bitClear(flag_utilities_filament_register,flag_utilities_filament_register_purgeunload);
			if(degHotend(which_extruder) >= target_temperature[which_extruder]-PURGE_TEMP_HYSTERESIS){
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_ADJUST_UNLOAD, 1);
				current_position[E_AXIS]-=5;
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_SLOW_SPEED/60, which_extruder);//Retract
				st_synchronize();
				disable_e0();
				disable_e1();
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_ADJUST_UNLOAD, 0);
			}
		}
		
		if(is_checking_filament){
			
			static bool filament_state_check = true;
			
			if(!Flag_FRS_enabled || (Flag_FRS_enabled && CHECK_FRS(which_extruder))){
				if(!filament_state_check){
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_KEEPPUSHING_NEXT,0);
				}
				filament_state_check = true;
				
				}else{
				if(filament_state_check){
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_KEEPPUSHING_NEXT,1);
				}
				filament_state_check = false;
			}
		}
		
		//static uint32_t waitPeriod = millis();
		if (millis() >= waitPeriod)
		{
			int tHotend=(int)(degHotend(0)+0.5);
			int tHotend1=(int)(degHotend(1)+0.5);
			int percentage = 0;
			int Tinstant = 0;
			if(is_purging_filament)
			{
				
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_LEFTTARGET,tHotend);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_RIGHTTARGET,tHotend1);
				
				if(degHotend(purge_extruder_selected) >= target_temperature[purge_extruder_selected]-PURGE_TEMP_HYSTERESIS && purge_extruder_selected != -1){
					
					if(purge_extruder_selected == 0){
						display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_LOAD,1);
						display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD,1);
						}else if (purge_extruder_selected == 1){
						display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_LOAD,1);
						display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD,1);
					}
					
					
					}else{
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_LOAD, 0);
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD, 0);
				}
				
			}
			
			if(is_changing_filament){
				
				
				if(Tref1<Tfinal1){
					
					Tinstant = constrain((int)degHotend(which_extruder),Tref1,Tfinal1);
				}
				else{
					Tinstant = constrain((int)degHotend(which_extruder),Tfinal1,Tref1);
				}
				percentage = Tfinal1-Tref1;
				percentage = 100*(Tinstant-Tref1)/percentage;
				if(percentage > Tpercentage_old){
					Tpercentage_old = percentage;
				}
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, Tpercentage_old);
				
				if ((abs((int)degHotend(which_extruder)-(int)degTargetHotend(which_extruder)) < CHANGE_FIL_TEMP_HYSTERESIS)){
					// if we want to add user setting temp, we should control if is heating
					//We have preheated correctly
					
					if (filament_mode =='I'){
						is_checking_filament = true;
						heatting = false;
						//genie.WriteStr(STRING_FILAMENT,"Press GO and keep pushing the filament \n until starts being pulled");
						if(!Flag_FRS_enabled || (Flag_FRS_enabled && CHECK_FRS(which_extruder))){
							display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_KEEPPUSHING_NEXT,0);
							}else{
							display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_KEEPPUSHING_NEXT,1);
						}
						display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD_KEEPPUSHING,0);
						//genie.WriteStr(STRING_FILAMENT,"Press GO and keep pushing the filament \n until starts being pulled");
						gif_processing_state = PROCESSING_STOP;
					}
					else if (filament_mode =='R')
					{
						heatting = false;
						
						
						if(Flag_checkfil){
							is_changing_filament=false;
							unloadfilament_procedure();
							
							}else{
							display_ChangeForm(FORM_UTILITIES_FILAMENT_UNLOAD_ROLLTHESPOOL,0);
							//genie.WriteStr(STRING_FILAMENT,"Press GO to Remove Filament, roll\n the spool backwards to save the filament");
						}
					}
					
					is_changing_filament=false; //Reset changing filament control
				}
			}
			
			
			// Check if preheat for insert_FIL is done ////////////////////////////////////////////////////////////////////
			
			
			
			waitPeriod=500+millis(); // Every Second
		}
	}
	if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_acceptok) && !home_made){
		gif_processing_state = PROCESSING_DEFAULT;
	}
	if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_acceptok) && home_made && (gif_processing_state == PROCESSING_DEFAULT)){
		gif_processing_state = PROCESSING_STOP;
		printer_state = STATE_LOADUNLOAD_FILAMENT;
		display_ChangeForm(FORM_UTILITIES_FILAMENT_SUCCESS,0);
		gif_processing_state = PROCESSING_SUCCESS;
	}
	if(bitRead(flag_sdprinting_register,flag_sdprinting_register_datarefresh)){
		is_printing_screen = true;
	}
	if(is_printing_screen){
		
		static int count5s = 0;
		static int count5s1 = 0;
		if (millis() >= waitPeriod)
		{
			
			//static int tHotend = -1;
			//static int tHotend1 = -1;
			//static int tBed = -1;
			//static int percentDone = -1;
			static int feedmultiply1 = -1;
			unsigned int minuteremaining = StatusJob.get_remaining_min();
			StatusJob.calc_remaining_time_from_sd();
			if (StatusJob.get_temp0() !=(int)(degHotend(0)+0.5) || bitRead(flag_sdprinting_register,flag_sdprinting_register_datarefresh)){
				if(!card.sdispaused)display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_HOTEND0,(int)(degHotend(0)+0.5));
				else display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_PAUSE_HOTEND0,(int)(degHotend(0)+0.5));
			}
			if (StatusJob.get_temp1() !=(int)(degHotend(1)+0.5) || bitRead(flag_sdprinting_register,flag_sdprinting_register_datarefresh)){
				if(!card.sdispaused)display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_HOTEND1,(int)(degHotend(1)+0.5));
				else display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_PAUSE_HOTEND1,(int)(degHotend(1)+0.5));
			}
			if (StatusJob.get_tempbed() !=(int)(degBed()+0.5) || bitRead(flag_sdprinting_register,flag_sdprinting_register_datarefresh)){
				if(!card.sdispaused)display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_BED,(int)(degBed()+0.5));
				else display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_PAUSE_BED,(int)(degBed()+0.5));
			}
			if (StatusJob.get_progress() != card.percentDone() || bitRead(flag_sdprinting_register,flag_sdprinting_register_datarefresh)){
				if(!card.sdispaused)display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_PERCENTAGE,card.percentDone());
				else display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_PAUSE_PERCENTAGE,card.percentDone());
			}
			if ( StatusJob.get_remaining_min() != minuteremaining || bitRead(flag_sdprinting_register,flag_sdprinting_register_datarefresh)){
				
				if(!card.sdispaused){
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_TIMEREMAINING_MINS,StatusJob.get_remaining_min());
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_TIMEREMAINING_HOURS,StatusJob.get_remaining_h());
				}
				else {
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_PAUSE_TIMEREMAINING_MINS,StatusJob.get_remaining_min());
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_PAUSE_TIMEREMAINING_HOURS,StatusJob.get_remaining_h());
				}
			}
			
			if(feedmultiply[active_extruder] != feedmultiply1 || bitRead(flag_sdprinting_register,flag_sdprinting_register_datarefresh)){
				feedmultiply1 = feedmultiply[active_extruder];
				if(!card.sdispaused)display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_FEED,feedmultiply1);
				else display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_SDPRINTING_PAUSE_FEED,feedmultiply1);
			}
			
			StatusJob.update_temps();
			StatusJob.set_progress(card.percentDone());
			
			bitClear(flag_sdprinting_register,flag_sdprinting_register_datarefresh);
			count5s++;
			count5s1++;
			if (count5s == 1440){ //2.5s * 1440 = 3600s = 1h
				count5s=0;
				log_hours_print++;
			}
			if (count5s1 == 24){ //2.5s * 34 = 60s = 1min
				count5s1=0;
				log_min_print++;
			}
			waitPeriod=1500+millis();	//Every 2.5s
			
		}
		
	}
	
}
void update_screen_noprinting(){
	static uint32_t waitPeriod_c = millis();
	static uint32_t waitPeriodno = millis();
	static uint32_t waitPeriod_p = millis();
	static int8_t processing_state = 0;
	if (surfing_temps){
		//static uint32_t waitPeriod = millis();
		if (millis() >= waitPeriodno)
		{
						
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_TEMP_LEXTR,(int)(degHotend(0)+0.5));
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_TEMP_LEXTR_TARGET,target_temperature[0]);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_TEMP_REXTR,(int)(degHotend(1)+0.5));
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_TEMP_REXTR_TARGET,target_temperature[1]);
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_TEMP_BED,(int)(degBed()+0.5));
			display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_TEMP_BED_TARGET,target_temperature_bed);
			
			waitPeriodno=1000+millis();
		}
		if (millis() >= waitPeriod_p)
		{
			
			int tHotend = (int)(degHotend(0)+0.5);
			int tHotend1 = (int)(degHotend(1)+0.5);
			int tBed = (int)(degBed()+0.5);
			
			if ((tHotend <= target_temperature[0]-10 || tHotend >= target_temperature[0]+10) && target_temperature[0]!=0) {
				flag_temp_gifhotent0 = true;
				display_ButtonState(BUTTON_TEMP_LEXTR,0);//<GIFF
				
			}
			else if(target_temperature[0]!=0){
				display_SetFrame(GIF_TEMP_LEXTR,GIF_FRAMES_PREHEAT+1);
				flag_temp_gifhotent0 = false;
				display_ButtonState(BUTTON_TEMP_LEXTR,1);//<GIFF
			}
			else{
				flag_temp_gifhotent0 = false;
				display_SetFrame(GIF_TEMP_LEXTR,0);
				display_ButtonState(BUTTON_TEMP_LEXTR,0);//<GIFF
			}
			if ((tHotend1 <= target_temperature[1]-10 || tHotend1 >= target_temperature[1]+10) && target_temperature[1]!=0)  {
				flag_temp_gifhotent1 = true;//<GIFF
				display_ButtonState(BUTTON_TEMP_REXTR,0);//<GIFF
				
			}
			else if(target_temperature[1]!=0){
				display_SetFrame(GIF_TEMP_REXTR,GIF_FRAMES_PREHEAT+1);
				flag_temp_gifhotent1 = false;
				display_ButtonState(BUTTON_TEMP_REXTR,1); //<GIFF
			}
			else{
				display_SetFrame(GIF_TEMP_REXTR,0);
				flag_temp_gifhotent1 = false;
				display_ButtonState(BUTTON_TEMP_REXTR,0);//<GIFF
			}
			if (( tBed <= target_temperature_bed-10 ||  tBed >= target_temperature_bed+10) && target_temperature_bed!=0)  {
				flag_temp_gifbed = true;
				display_ButtonState(BUTTON_TEMP_BED,0);//<GIFF
				
			}
			else if(target_temperature_bed!=0){
				display_SetFrame(GIF_TEMP_BED,GIF_FRAMES_PREHEAT+2);
				flag_temp_gifbed = false;
				display_ButtonState(BUTTON_TEMP_BED,1);//<GIFF
			}
			else{
				display_SetFrame(GIF_TEMP_BED,0);
				flag_temp_gifbed = false;
				display_ButtonState(BUTTON_TEMP_BED,0);//<GIFF
			}
			
			if(flag_temp_gifhotent0 || flag_temp_gifhotent1 || flag_temp_gifbed ){
				
				processing_state  = (processing_state < GIF_FRAMES_PREHEAT) ? processing_state + 1 : 3;
				
				if(flag_temp_gifhotent0){
					display_SetFrame(GIF_TEMP_LEXTR,processing_state);
				}
				if(flag_temp_gifhotent1){
					display_SetFrame(GIF_TEMP_REXTR,processing_state);
				}
				if(flag_temp_gifbed){
					display_SetFrame(GIF_TEMP_BED,processing_state);
				}
			}
			
			waitPeriod_p=GIF_FRAMERATE+millis();
		}
	}
	
	if (surfing_utilities)
	{
		if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect0)){
			bitClear(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect0);
			if(degHotend(purge_extruder_selected) >= target_temperature[purge_extruder_selected]-PURGE_TEMP_HYSTERESIS){
				gif_processing_state = PROCESSING_PURGE_LOAD;
				current_position[E_AXIS]+=PURGE_DISTANCE_INSERTED;
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_SLOW_SPEED/60, purge_extruder_selected);//Purge
				st_synchronize();
				RESET_E_COORDINATES;
				disable_e0();
				disable_e1();
				gif_processing_state = PROCESSING_STOP;
				display_SetFrame(GIF_UTILITIES_FILAMENT_PURGE,0);
			}
		}
		if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect1)){
			bitClear(flag_utilities_filament_register,flag_utilities_filament_register_purgeselect1);
			if(degHotend(purge_extruder_selected) >= target_temperature[purge_extruder_selected]-PURGE_TEMP_HYSTERESIS){
				gif_processing_state = PROCESSING_PURGE_UNLOAD;
				current_position[E_AXIS]-=5;
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_SLOW_SPEED/60, purge_extruder_selected);//Retract
				st_synchronize();
				RESET_E_COORDINATES;
				disable_e0();
				disable_e1();
				gif_processing_state = PROCESSING_STOP;
				display_SetFrame(GIF_UTILITIES_FILAMENT_PURGE,0);
			}
		}
		if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_purgeload)){
			bitClear(flag_utilities_filament_register,flag_utilities_filament_register_purgeload);
			if(degHotend(which_extruder) >= target_temperature[which_extruder]-PURGE_TEMP_HYSTERESIS){
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_ADJUST_LOAD, 1);
				current_position[E_AXIS]+=PURGE_DISTANCE_INSERTED;
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_SLOW_SPEED/60, which_extruder);//Purge
				st_synchronize();
				RESET_E_COORDINATES;
				disable_e0();
				disable_e1();
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_ADJUST_LOAD, 0);
			}
		}
		if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_purgeunload)){
			bitClear(flag_utilities_filament_register,flag_utilities_filament_register_purgeunload);
			if(degHotend(which_extruder) >= target_temperature[which_extruder]-PURGE_TEMP_HYSTERESIS){
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_ADJUST_UNLOAD, 1);
				current_position[E_AXIS]-=5;
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_SLOW_SPEED/60, which_extruder);//Retract
				st_synchronize();
				RESET_E_COORDINATES;
				disable_e0();
				disable_e1();
				display_ButtonState(BUTTON_UTILITIES_FILAMENT_ADJUST_UNLOAD, 0);
			}
		}
		if(is_checking_filament){
			
			static bool filament_state_check = true;
			
			if(!Flag_FRS_enabled || (Flag_FRS_enabled && CHECK_FRS(which_extruder))){
				if(!filament_state_check){
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_KEEPPUSHING_NEXT,0);
				}
				filament_state_check = true;
				
				}else{
				if(filament_state_check){
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_KEEPPUSHING_NEXT,1);
				}
				filament_state_check = false;
			}
		}
		if (millis() >= waitPeriodno)
		{
			int tHotend = (int)(degHotend(0)+0.5);
			int tHotend1 = (int)(degHotend(1)+0.5);
			int Tinstant = 0;
			int percentage = 0;
			
			if(is_purging_filament){
				
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_LEFTTARGET,tHotend);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_PURGE_RIGHTTARGET,tHotend1);
				
				if(degHotend(purge_extruder_selected) >= target_temperature[purge_extruder_selected]-PURGE_TEMP_HYSTERESIS && purge_extruder_selected != -1){
					
					if(purge_extruder_selected == 0){
						display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_LOAD,1);
						display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD,1);
						}else if (purge_extruder_selected == 1){
						display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_LOAD,1);
						display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD,1);
					}
					
					
					}else{
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_LOAD, 0);
					display_ButtonState(BUTTON_UTILITIES_FILAMENT_PURGE_UNLOAD, 0);
				}
				
			}
			if(is_changing_filament){
				
				
				if(Tref1<Tfinal1){
					
					Tinstant = constrain((int)degHotend(which_extruder),Tref1,Tfinal1);
				}
				else{
					Tinstant = constrain((int)degHotend(which_extruder),Tfinal1,Tref1);
				}
				percentage = Tfinal1-Tref1;
				percentage = 100*(Tinstant-Tref1)/percentage;
				if(percentage > Tpercentage_old){
					Tpercentage_old = percentage;
				}
				
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, Tpercentage_old);
				
				// Check if preheat for insert_FIL is done ////////////////////////////////////////////////////////////////////
				if ((abs((int)degHotend(which_extruder)-(int)degTargetHotend(which_extruder)) < CHANGE_FIL_TEMP_HYSTERESIS)){
					// if we want to add user setting temp, we should control if is heating
					
					SERIAL_PROTOCOLPGM("Ready to Load/Unload \n");
					//We have preheated correctly
					if (filament_mode =='I'){
						if(!Flag_FRS_enabled || (Flag_FRS_enabled && CHECK_FRS(which_extruder))){
							display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_KEEPPUSHING_NEXT,0);
							}else{
							display_ButtonState(BUTTON_UTILITIES_FILAMENT_LOAD_KEEPPUSHING_NEXT,1);
						}
						display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD_KEEPPUSHING,0);
						is_checking_filament = true;
					}
					else if (filament_mode =='R')
					{
						
						display_ChangeForm(FORM_UTILITIES_FILAMENT_UNLOAD_ROLLTHESPOOL,0);
						
					}
					gif_processing_state = PROCESSING_STOP;
					is_changing_filament=false; //Reset changing filament control
				}
			}
			
			
			waitPeriodno=1000+millis(); // Every Second
		}
	}
	if(bitRead(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10up)){
		processing_z_set = 0;
		bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10up);
		current_position[Z_AXIS]-=10;
		
		if (home_made_Z){
			if(current_position[Z_AXIS] < Z_MIN_POS){
				current_position[Z_AXIS] = Z_MIN_POS;
			}
		}
		else{
			quickStop();
		}
		plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder); //check speed
		st_synchronize();
		processing_z_set = 255;
	}
	if(bitRead(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100up)){
		processing_z_set = 0;
		current_position[Z_AXIS]-=100;
		bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100up);
		if (home_made_Z){
			if(current_position[Z_AXIS]< Z_MIN_POS){
				current_position[Z_AXIS]= Z_MIN_POS;
			}
		}
		plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder); //check speed
		st_synchronize();
		processing_z_set = 255;
		
	}
	if(bitRead(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10down)){
		processing_z_set = 1;
		current_position[Z_AXIS]+=10;
		bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust10down);
		if (home_made_Z){
			if(current_position[Z_AXIS] > Z_MAX_POS-15){
				current_position[Z_AXIS] = Z_MAX_POS-15;
			}
		}
		plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder); //check speed
		st_synchronize();
		processing_z_set = 255;
		
	}
	if(bitRead(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100down)){
		processing_z_set = 1;
		current_position[Z_AXIS]+=100;
		bitClear(flag_utilities_maintenance_register,flag_utilities_maintenance_register_zdjust100down);
		if (home_made_Z){
			
			if(current_position[Z_AXIS] > Z_MAX_POS-15){
				current_position[Z_AXIS] = Z_MAX_POS-15;
			}
		}
		plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder); //check speed
		st_synchronize();
		processing_z_set = 255;
		
	}
	if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_acceptok) && !home_made){
		gif_processing_state = PROCESSING_DEFAULT;
	}
	if(bitRead(flag_utilities_filament_register,flag_utilities_filament_register_acceptok) && home_made && (gif_processing_state == PROCESSING_DEFAULT)){
		gif_processing_state = PROCESSING_STOP;
		printer_state = STATE_LOADUNLOAD_FILAMENT;
		display_ChangeForm(FORM_UTILITIES_FILAMENT_SUCCESS,0);
		HeaterCooldownInactivity(true);
		gif_processing_state = PROCESSING_SUCCESS;
	}
	if(bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_cooldown_mode0) || bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_cooldown_mode1)){
		
		uint8_t mode = bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_cooldown_mode0)?0:1;
		
		bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_cooldown_mode0);
		bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_cooldown_mode1);
		
		display_ChangeForm(FORM_PROCESSING,0);
		gif_processing_state = PROCESSING_DEFAULT;
		Config_StoreSettings();
		
		setTargetBed(0);
		setTargetHotend0(0);
		setTargetHotend1(0);
		
		if(mode == 0){
			if(extruder_offset[Z_AXIS][RIGHT_EXTRUDER] < 0.0){
				which_extruder = RIGHT_EXTRUDER;
				}else{
				which_extruder = LEFT_EXTRUDER;
			}
		}
		
		
		changeTool(which_extruder);
		home_axis_from_code(true,true,(home_made_Z?false:true));
		
		
		
		current_position[Z_AXIS] = 80;
		plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],12,which_extruder);
		current_position[Y_AXIS] = 100;
		plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],XY_TRAVEL_SPEED15/60.0,which_extruder);
		
		st_synchronize();
		
		gif_processing_state = PROCESSING_STOP;
		
		
		if((int)degHotend(which_extruder)>NYLON_TEMP_COOLDOWN_THRESHOLD){
			display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
			if(which_extruder == 0)digitalWrite(FAN_PIN, 1);
			else digitalWrite(FAN2_PIN, 1);
			gif_processing_state = PROCESSING_ADJUSTING;
			int Tref_c = (int)degHotend(which_extruder);
			int Tfinal_c = NYLON_TEMP_COOLDOWN_THRESHOLD;
			int percentage_c = 0;
			int percentage_old_c = 0;
			int Tinstant_c = 0;
			while ((int)degHotend(which_extruder)>Tfinal_c){ //Waiting to heat the extruder
				
				manage_heater();
				touchscreen_update();
				ERROR_SCREEN_WARNING2;
				
				if (millis() >= waitPeriod_c){
					
					
					Tinstant_c = constrain((int)degHotend(which_extruder),Tfinal_c,Tref_c);
					
					percentage_c = ((Tref_c-Tfinal_c)-(Tinstant_c-Tfinal_c))*100; //<<<<<<<<<<<<<  0% TO 100%
					percentage_c = percentage_c/(Tref_c-Tfinal_c);
					if(percentage_c > percentage_old_c){
						percentage_old_c = percentage_c;
					}
					display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES,percentage_old_c);
					waitPeriod_c = 500 + millis();
				}
				
				
			}
			gif_processing_state = PROCESSING_STOP;
			touchscreen_update();
		}
		
		
		#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
		current_position[X_AXIS] = 155;
		#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
		current_position[X_AXIS] = 155 + X_OFFSET_CALIB_PROCEDURES;
		#endif
		plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],XY_TRAVEL_SPEED15/60.0,which_extruder);

		
		
		if(mode == 0){
			char offset_string[250];
			memset(offset_string, '\0', sizeof(offset_string) );
			int offset_aprox;
			offset_aprox = (int)(round(abs(extruder_offset[Z_AXIS][RIGHT_EXTRUDER])*100.01)/5.0);
			sprintf_P(offset_string, PSTR("Install %d %s on the %s hotend."), offset_aprox, ((offset_aprox > 1)?"shims":"shim") , ((extruder_offset[Z_AXIS][RIGHT_EXTRUDER] < 0)?"right":"left"));
			display_ChangeForm(FORM_Z_COMPENSATION_SHUTDOWN,0);
			display.WriteStr(STRING_Z_COMPENSATION_SHUTDOWN,offset_string);
		}
		else if(mode == 1){
			display_ChangeForm(FORM_PRINTERSETUP_CHANGEHOTEND_SHUTDOWN,0);
		}
		
		
		
	}
	
}
void update_screen_sdcard(){
	static uint32_t waitPeriod_input_button_command = millis();
	
	if(bitRead(flag_sdlist_resgiter,flag_sdlist_resgiter_goup)){
		ListFilesDownfunc();
		bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_goup);
	}
	if(bitRead(flag_sdlist_resgiter,flag_sdlist_resgiter_godown)){
		ListFilesUpfunc();
		bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_godown);
	}
	if(bitRead(flag_sdlist_resgiter,flag_sdlist_resgiter_goinit)){
		ListFileListINITSD();
		bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_goinit);
	}
	if(bitRead(flag_sdlist_resgiter,flag_sdlist_resgiter_gofolderback)){
		ListFileListENTERBACKFORLDERSD();
		waitPeriod_input_button_command = millis();
		bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_gofolderback);
	}
	if(millis() >= waitPeriod_input_button_command && bitRead(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown)){
		if(bitRead(flag_sdlist_resgiter,flag_sdlist_resgiter_select0)){
			ListFileSelect0();
			waitPeriod_input_button_command = 1000 + millis();
		}
		else if(bitRead(flag_sdlist_resgiter,flag_sdlist_resgiter_select1)){
			ListFileSelect1();
			waitPeriod_input_button_command = 1000 + millis();
		}
		else if(bitRead(flag_sdlist_resgiter,flag_sdlist_resgiter_select2)){
			ListFileSelect2();
			waitPeriod_input_button_command = 1000 + millis();
		}
		else if(bitRead(flag_sdlist_resgiter,flag_sdlist_resgiter_select3)){
			ListFileSelect3();
			waitPeriod_input_button_command = 1000 + millis();
		}
		else if(bitRead(flag_sdlist_resgiter,flag_sdlist_resgiter_select4)){
			ListFileSelect4();
			waitPeriod_input_button_command = 1000 + millis();
		}
		
	}
	bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_select0);
	bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_select1);
	bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_select2);
	bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_select3);
	bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_select4);
}
void lcd_animation_handler(){//We process the animations frames
	static uint32_t waitPeriod_pbackhome = millis(); //Processing back home
	static uint32_t waitPeriod_p = millis();
	static int8_t processing_state = 0;
	static int8_t processing_state_z = 0;
	if (processing_z_set == 0 || processing_z_set == 1){
		if (millis() >= waitPeriod_p){
			if (processing_z_set == 0){
				if(processing_state_z<GIF_FRAMES_ZSET){
					processing_state_z++;
				}
				else{
					processing_state_z=0;
				}
			}
			else{
				if(processing_state_z>0){
					processing_state_z--;
				}
				else{
					processing_state_z=GIF_FRAMES_ZSET-1;
				}
			}
			display_SetFrame(GIF_MAINTENANCE_ZADJUST,processing_state_z);
			waitPeriod_p=GIF_FRAMERATE+millis();
		}
	}
	else if(back_home){
		if(home_made == false){
			cancel_heatup = true;
			
			if (millis() >= waitPeriod_pbackhome){
				processing_state = (processing_state<GIF_FRAMES_PROCESSING) ? processing_state + 3 : 0;
				display_SetFrame(GIF_PROCESSING,processing_state);
				waitPeriod_pbackhome=GIF_FRAMERATE+millis();
			}
			
		}
		else{
			back_home = false;
			saved_print_smartpurge_flag = false;
			gcode_T0_T1_auto(0);
			st_synchronize();
			if(SD_FINISHED_STEPPERRELEASE)
			{
				enquecommand_P(PSTR(SD_FINISHED_RELEASECOMMAND));
			}
			quickStop();
			set_dual_x_carriage_mode(DEFAULT_DUAL_X_CARRIAGE_MODE);
			autotempShutdown();
			setTargetHotend0(0);
			setTargetHotend1(0);
			setTargetBed(0);
			log_hours_lastprint = (int)(log_min_print/60);
			log_minutes_lastprint = (int)(log_min_print%60);
			Config_StoreSettings();
			cancel_heatup = false;
			if(current_position[Z_AXIS]>Z_MAX_POS-15){
				current_position[Z_AXIS] -= Z_SIGMA_RAISE_BEFORE_HOMING;
				plan_buffer_line(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS],current_position[E_AXIS],6,active_extruder);
				st_synchronize();
			}
			if(FLAG_thermal_runaway){
				char buffer[80];
				sprintf(buffer, "ERROR(88): Temperature not reached by Heater_ID: %d",ID_thermal_runaway);
				if(!FLAG_thermal_runaway_screen && (screen_printing_pause_form !=screen_printing_pause_form2)){
					display_ChangeForm(FORM_ERROR_SCREEN,0);
					display.WriteStr(STRING_ERROR_MESSAGE,buffer);
					FLAG_thermal_runaway_screen = true;
					gif_processing_state = PROCESSING_ERROR;
				}
				FLAG_thermal_runaway = false;
				return;
			}
			if(saved_print_flag == 1888){
				display_ChangeForm(FORM_SDPRINTING_SAVEJOB_SUCCESS,0);
				gif_processing_state = PROCESSING_SAVE_PRINT_SUCCESS;
				processing_state = 0;
				
				}else{
				#ifdef DEV_BER
				Send_feedback_flag = true;
				//SERIAL_PROTOCOLLNPGM("Not SD printing");
				
				listsd.get_info_from_cura();
				card.feedback_ending();
				card.closefile();
				
				//display_ChangeForm(FORM_PRINT_REMOVE,0);
				display_ChangeForm(FORM_MAIN,0);
				SERIAL_PROTOCOLLNPGM("Main");
				#else
				display_ChangeForm(FORM_MAIN,0);
				#endif
				
			}
			
		}
	}
	else{
		switch(gif_processing_state){
			case PROCESSING_DEFAULT:
			
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state<GIF_FRAMES_PROCESSING) ? processing_state + 3 : 0;
				display_SetFrame(GIF_PROCESSING,processing_state);
				
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			break;
			
			
			case PROCESSING_ADJUSTING:
			
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state<GIF_FRAMES_ADJUSTINGTEMPS) ? processing_state + 1 : 0;
				display_SetFrame(GIF_ADJUSTING_TEMPERATURES,processing_state);
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			break;
			
			
			case PROCESSING_TEST:
			
			if (millis() >= waitPeriod_p){
				
				display_SetFrame(GIF_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,processing_state);
				processing_state = (processing_state<GIF_FRAMES_CALIBTEST) ? processing_state + 1 : 0;
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			
			break;
			
			
			case PROCESSING_BED_FIRST:
			
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state<GIF_FRAMES_BEDSCREW) ? processing_state + 1 : 0;
				display_SetFrame(GIF_UTILITIES_CALIBRATION_CALIBBED_ADJUSTSCREWASKFIRST,processing_state);
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			
			break;
			
			case PROCESSING_SUCCESS:
			
			if (millis() >= waitPeriod_p){
				
				if(processing_state<GIF_FRAMES_SUCCESS){
					processing_state++;
					display_SetFrame(GIF_UTILITIES_FILAMENT_SUCCESS,processing_state);
				}
				else if(processing_state == GIF_FRAMES_SUCCESS){
					display_SetFrame(GIF_UTILITIES_FILAMENT_SUCCESS,processing_state);
					processing_state=0;
					gif_processing_state = PROCESSING_STOP;
				}
				else{
					processing_state=0;
					display_SetFrame(GIF_UTILITIES_FILAMENT_SUCCESS,processing_state);
				}
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			
			break;
			
			
			case PROCESSING_SAVE_PRINT_SUCCESS:
			
			if (millis() >= waitPeriod_p){
				
				if(processing_state<GIF_FRAMES_SUCCESS){
					
					display_SetFrame(GIF_SDPRINTING_SAVEJOB_SUCCESS,processing_state);
					processing_state++;
				}
				else if(processing_state == GIF_FRAMES_SUCCESS){
					
					display_SetFrame(GIF_SDPRINTING_SAVEJOB_SUCCESS,processing_state);
					processing_state=0;
					gif_processing_state = PROCESSING_STOP;
					delay(5000);
					display_ChangeForm(FORM_SDPRINTING_SAVEJOB_SHUTDOWN,0);
					
				}
				else{
					processing_state=0;
					display_SetFrame(GIF_SDPRINTING_SAVEJOB_SUCCESS,processing_state);
				}
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			
			break;
			
			
			case PROCESSING_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP3:
			
			if (millis() >= waitPeriod_p){
				
				
				processing_state = (processing_state<GIF_FRAMES_NYLONSTEP3) ? processing_state + 1 : 0;
				display_SetFrame(GIF_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP3,processing_state);
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			
			
			break;
			
			
			case PROCESSING_PURGE_LOAD:
			
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state<GIF_FRAMES_PURGELOAD) ? processing_state + 1 : 0;
				display_SetFrame(GIF_UTILITIES_FILAMENT_PURGE,processing_state);
				waitPeriod_p=150+millis();
			}
			
			break;
			
			case PROCESSING_PURGE_UNLOAD:
			
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state > 0) ? processing_state - 1 : GIF_FRAMES_PURGEUNLOAD;
				display_SetFrame(GIF_UTILITIES_FILAMENT_PURGE,processing_state);
				waitPeriod_p=150+millis();
			}
			
			break;
			
			
			case PROCESSING_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP4:
			
			if (millis() >= waitPeriod_p){
				
				if(processing_state<GIF_FRAMES_SUCCESS){
					display_SetFrame(GIF_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP4,processing_state);
					processing_state++;
				}
				else if(processing_state == GIF_FRAMES_SUCCESS){
					display_SetFrame(GIF_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP4,processing_state);
					processing_state=0;
					gif_processing_state = PROCESSING_STOP;
				}
				else{
					processing_state=0;
					display_SetFrame(GIF_UTILITIES_MAINTENANCE_NYLONCLEANING_STEP4,processing_state);
				}
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			break;
			
			
			case PROCESSING_BED_SUCCESS:
			
			if (millis() >= waitPeriod_p){
				
				if(processing_state<GIF_FRAMES_SUCCESS){
					
					display_SetFrame(GIF_UTILITIES_CALIBRATION_SUCCESS,processing_state);
					processing_state++;
				}
				else if(processing_state == GIF_FRAMES_SUCCESS){
					display_SetFrame(GIF_UTILITIES_CALIBRATION_SUCCESS,processing_state);
					processing_state=0;
					gif_processing_state = PROCESSING_STOP;
				}
				else{
					processing_state=0;
					display_SetFrame(GIF_UTILITIES_CALIBRATION_SUCCESS,processing_state);
				}
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			break;
			
			
			case PROCESSING_SUCCESS_FIRST_RUN:
			
			if (millis() >= waitPeriod_p){
				
				if(processing_state<GIF_FRAMES_SUCCESS){
					
					display_SetFrame(GIF_SETUPASSISTANT_SUCCESS,processing_state);
					processing_state++;
				}
				else if (processing_state == GIF_FRAMES_SUCCESS){
					
					display_SetFrame(GIF_SETUPASSISTANT_SUCCESS,processing_state);
					processing_state=0;
					gif_processing_state = PROCESSING_STOP;
					delay(5000);
					
					
					setTargetHotend0(0);
					setTargetHotend1(0);
					setTargetBed(0);
					home_axis_from_code(true, true, false);
					enquecommand_P((PSTR("T0")));
					st_synchronize();
					ERROR_SCREEN_WARNING2;
					
					doblocking=false;
					
					screen_sdcard = false;
					surfing_utilities=false;
					
					SERIAL_PROTOCOLLNPGM("Main");
					
					surfing_temps = false;
					display_ChangeForm( FORM_MAIN, 0);
				}
				else{
					processing_state= 0;
					display_SetFrame(GIF_SETUPASSISTANT_SUCCESS,processing_state);
				}
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			
			break;
			
			case PROCESSING_BED:
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state<GIF_FRAMES_BEDSCREW) ? processing_state + 1 : 0;
				display_SetFrame(GIF_UTILITIES_CALIBRATION_CALIBBED_ADJUSTSCREWASK,processing_state);
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			break;
			case PROCESSING_CALIB_ZL:
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state<GIF_FRAMES_ZCALIB) ? processing_state + 1 : 0;
				display_SetFrame(GIF_UTILITIES_CALIBRATION_CALIBFULL_CALIBZL,processing_state);
				waitPeriod_p=100+millis();
			}
			break;
			case PROCESSING_CALIB_ZR:
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state<GIF_FRAMES_ZCALIB) ? processing_state + 1 : 0;
				display_SetFrame(GIF_UTILITIES_CALIBRATION_CALIBFULL_CALIBZR,processing_state);
				waitPeriod_p=100+millis();
			}
			break;
			
			case PROCESSING_CALIB_Z_GAUGE:
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state<GIF_FRAMES_ZCALIB_GAUGE) ? processing_state + 1 : 0;
				display_SetFrame(GIF_UTILITIES_CALIBRATION_CALIBFULL_CALIBZ_TIP_1,processing_state);
				display_SetFrame(GIF_UTILITIES_CALIBRATION_CALIBFULL_CALIBZ_TIP_2,processing_state);
				display_SetFrame(GIF_UTILITIES_CALIBRATION_CALIBFULL_CALIBZ_TIP_3,processing_state);
				waitPeriod_p=GIF_FRAMERATE2+millis();
			}
			break;
			
			case PROCESSING_ERROR:
			
			if (millis() >= waitPeriod_p){
				
				processing_state = (processing_state<GIF_FRAMES_ERROR) ? processing_state + 1 : 0;
				display_SetFrame(GIF_ERROR,processing_state);
				waitPeriod_p=GIF_FRAMERATE+millis();
			}
			
			break;
			
			default:
			processing_state = 0;
			
		}
	}
}
void ListFilesParsingProcedure(int vecto, int jint){
	card.getfilename(vecto);
	//Serial.println(card.longFilename);
	if (card.filenameIsDir)
	{
		display_ButtonState(buttonsdselected[jint],1);
		display.WriteObject(GENIE_OBJ_USERIMAGES,userimagesdlist[jint],2);
		setfoldernames(jint);
		if(card.chdir(card.filename)!= -1){
			uint16_t NUMitems = card.getnrfilenames();
			card.updir();
			card.getWorkDirName();
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][jint],NUMitems);//Printing form
			
			setsdlisthours(jint,NUMitems);
			
		}
		else{
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][jint],0);//Printing form
			
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[jint][0],0);
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[jint][1],10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[jint][2],10);
		}
	}
	else{
		display_ButtonState(buttonsdselected[jint],0);
		display.WriteObject(GENIE_OBJ_USERIMAGES,userimagesdlist[jint],1);
		listsd.get_lineduration(true, NULL);
		setfilenames(jint);
		if(listsd.get_minutes() == -1){
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][jint],0);//Printing form
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[1][jint],0);//Printing form
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[2][jint],0);//Printing form
			
			setsdlisthours(jint,0);
			setsdlistmin(jint,0);
			setsdlistg(jint,0);
			
			}else{
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][jint],listsd.get_hours());//Printing form
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[1][jint],listsd.get_minutes());//Printing form
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[2][jint],listsd.get_filgramos1());//Printing form
			
			setsdlisthours(jint,listsd.get_hours());
			setsdlistmin(jint,listsd.get_minutes());
			setsdlistg(jint,listsd.get_filgramos1());
			
		}
		//Serial.println(listsd.commandline);
	}
}
void ListFilesUpfunc(){
	
	
	int vecto = 0;
	int jint = 0;
	
	
	if (card.cardOK){
		uint16_t fileCnt = card.getnrfilenames();
		//Declare filepointer
		card.getWorkDirName();
		
		if(fileCnt > SDFILES_LIST_NUM){
			filepointer = (filepointer == ((fileCnt-1)/SDFILES_LIST_NUM)*SDFILES_LIST_NUM )? 0 : filepointer + SDFILES_LIST_NUM;
			
			display_SetFrame( GIF_SDLIST_SCROLLBAR,	filepointer*40/(((fileCnt-1)/SDFILES_LIST_NUM)*SDFILES_LIST_NUM));
			
			while(jint < SDFILES_LIST_NUM){
				if(fileCnt > filepointer +  jint){
					
					vecto = filepointer + jint;
					
					ListFilesParsingProcedure(vecto, jint);
					
				}
				else{
					display_ButtonState(buttonsdselected[jint],0);
					//char buffer[8];
					//strcpy_P(buffer,PSTR("        "));
					//display.WriteStr(stringfilename[jint],"        ");//Printing form
					display.WriteObject(GENIE_OBJ_USERIMAGES,userimagesdlist[jint],0);
					//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][jint],0);//Printing form
					//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[1][jint],0);//Printing form
					//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[2][jint],0);//Printing form
					
				}
				jint++;
			}
			
		}
		
		
	}
	
	bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
	
	memset(listsd.commandline2, '\0', sizeof(listsd.commandline2) );
}
void ListFilesDownfunc(){
	
	int vecto = 0;
	int jint = 0;
	
	if (card.cardOK){
		uint16_t fileCnt = card.getnrfilenames();
		//Declare filepointer
		card.getWorkDirName();
		
		if(fileCnt > SDFILES_LIST_NUM){
			filepointer = (filepointer == 0) ? ((fileCnt-1)/SDFILES_LIST_NUM)*SDFILES_LIST_NUM : filepointer-SDFILES_LIST_NUM;
			
			display_SetFrame( GIF_SDLIST_SCROLLBAR,	filepointer*40/(((fileCnt-1)/SDFILES_LIST_NUM)*SDFILES_LIST_NUM));
			
			while(jint < SDFILES_LIST_NUM){
				if(fileCnt > filepointer +  jint){
					
					vecto = filepointer + jint;
					
					ListFilesParsingProcedure(vecto, jint);
					
				}
				else{
					display_ButtonState(buttonsdselected[jint],0);
					//char buffer[8];
					//strcpy_P(buffer,PSTR("        "));
					//display.WriteStr(stringfilename[jint],"        ");//Printing form
					display.WriteObject(GENIE_OBJ_USERIMAGES,userimagesdlist[jint],0);
					//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][jint],0);//Printing form
					//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[1][jint],0);//Printing form
					//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[2][jint],0);//Printing form
					
				}
				
				jint++;
			}
			
		}
		
	}
	bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);

	memset(listsd.commandline2, '\0', sizeof(listsd.commandline2) );
}
void ListFileListINITSD(){
	display_SetFrame( GIF_SDLIST_SCROLLBAR,0);
	
	
	uint32_t waitPeriod_p = millis();
	int8_t processing_state = 0;
	
	
	display_ChangeForm(FORM_PROCESSING,0);
	
	while(processing_state<GIF_FRAMES_PROCESSING){
		if (millis() >= waitPeriod_p){
			
			processing_state = processing_state + 3;
			display_SetFrame(GIF_PROCESSING,processing_state);
			
			waitPeriod_p=GIF_FRAMERATE+millis();
		}
	}
	
	
	////Check sdcardFiles
	filepointer = 0;
	int vecto = 0;
	uint16_t jint = 0;
	card.initsd();
	while(card.updir() != -1);
	
	//{
	//if (millis() >= waitPeriod_p){
	//
	//processing_state = (processing_state<GIF_FRAMES_PROCESSING) ? processing_state + 3 : 0;
	//display_SetFrame(GIF_PROCESSING,processing_state);
	//
	//waitPeriod_p=GIF_FRAMERATE+millis();
	//}
	//}
	
	workDir_vector_lenght = 0;
	if (card.cardOK){
		display_ButtonState( BUTTON_SDLIST_FOLDERBACK,0);
		display.WriteObject(GENIE_OBJ_USERIMAGES, USERIMAGE_SDLIST_FOLDERFILE,0);
		for(int i=0;i<5;i++){
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][i],0);//Printing form
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[1][i],0);//Printing form
			//display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[2][i],0);//Printing form
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[0][i],10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[1][i],10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[2][i],10);
			
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurmin[0][i],10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurmin[1][i],10);
			
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[0][i],10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[1][i],10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[2][i],10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[3][i],10);
			
			display.WriteObject(GENIE_OBJ_USERIMAGES,userimagesdlist[i],0);
		}
		display_ChangeForm(FORM_SDLIST,0);
		uint16_t fileCnt = card.getnrfilenames();
		//Declare filepointer
		card.getWorkDirName();
		
		while(jint < SDFILES_LIST_NUM){
			
			if(jint < fileCnt){
				
				vecto = filepointer + jint;
				ListFilesParsingProcedure(vecto, jint);
				
			}
			else{
				display_ButtonState(buttonsdselected[jint],0);
				//char buffer[8];
				//strcpy_P(buffer,PSTR("        "));
				//display.WriteStr(stringfilename[jint],"        ");//Printing form
				display.WriteObject(GENIE_OBJ_USERIMAGES,userimagesdlist[jint],0);
				//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][jint],0);//Printing form
				//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[1][jint],0);//Printing form
				//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[2][jint],0);//Printing form
				
			}
			
			jint++;
		}
		
		
	}
	else{
		#ifndef ErroWindowEnable
		display_ChangeForm( FORM_INSERT_SD_CARD, 0);
		screen_sdcard = true;
		#else
		display_ChangeForm( FORM_ERROR_SCREEN, 0);
		display.WriteStr(STRING_ERROR_MESSAGE,"ERROR: INSERT SDCARD");//Printing form
		gif_processing_state = PROCESSING_ERROR;
		screen_sdcard = true;
		#endif
	}
	memset(listsd.commandline2, '\0', sizeof(listsd.commandline2) );

}
void ListFileSelect_open(){
	card.getfilename(filepointer);
	if (!card.filenameIsDir){
		folder_navigation_register(true);
		display_ChangeForm( FORM_SDLIST_CONFIRMATION,0);
		listsd.get_lineduration(true, NULL);
		setfilenames(6);
		
	}
	else{
		if (card.chdir(card.filename)!=-1){
			folder_navigation_register(true);
			bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_gofolderback);
			display_ButtonState( BUTTON_SDLIST_FOLDERBACK,1);
			display.WriteObject(GENIE_OBJ_USERIMAGES, USERIMAGE_SDLIST_FOLDERFILE,1);
		}
		
		
	}
}
void ListFileSelect0(){
	if(card.cardOK)
	{
		bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
		ListFileSelect_open();
		bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
	}
}
void ListFileSelect1(){
	if(card.cardOK)
	{
		bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
		uint16_t fileCnt = card.getnrfilenames();
		if(fileCnt > filepointer +  1){
			if (filepointer == card.getnrfilenames()-1)
			{
				filepointer=0; //First SD file
				}else{
				filepointer++;
			}
			ListFileSelect_open();
		}
		bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
	}
}
void ListFileSelect2(){
	if(card.cardOK)
	{
		bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
		uint16_t fileCnt = card.getnrfilenames();
		if(fileCnt >filepointer +   2){
			if (filepointer == card.getnrfilenames()-1)
			{
				filepointer=1; //First SD file
			}
			else if (filepointer == card.getnrfilenames()-2 )
			{
				filepointer=0; //First SD file
			}
			else{
				filepointer+=2;
			}
			ListFileSelect_open();
		}
		bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
	}
}
void ListFileSelect3(){
	if(card.cardOK)
	{
		bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
		uint16_t fileCnt = card.getnrfilenames();
		if(fileCnt >filepointer +   3){
			if (filepointer == card.getnrfilenames()-1)
			{
				filepointer=2; //First SD file
			}
			else if (filepointer == card.getnrfilenames()-2 )
			{
				filepointer=1; //First SD file
			}
			else if (filepointer == card.getnrfilenames()-3 )
			{
				filepointer=0; //First SD file
			}
			else{
				filepointer+=3;
			}
			ListFileSelect_open();
		}
		
		bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
	}
}
void ListFileSelect4(){
	if(card.cardOK)
	{
		bitClear(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
		uint16_t fileCnt = card.getnrfilenames();
		if(fileCnt >filepointer +   4){
			if (filepointer == card.getnrfilenames()-1)
			{
				filepointer=3; //First SD file
			}
			else if (filepointer == card.getnrfilenames()-2 )
			{
				filepointer=2; //First SD file
			}
			else if (filepointer == card.getnrfilenames()-3 )
			{
				filepointer=1; //First SD file
			}
			else if (filepointer == card.getnrfilenames()-4 )
			{
				filepointer=0; //First SD file
			}
			else{
				filepointer+=4;
			}
			ListFileSelect_open();
		}
		bitSet(flag_sdlist_resgiter,flag_sdlist_resgiter_filesupdown);
	}
}
void ListFileListENTERBACKFORLDERSD(){
	display_SetFrame( GIF_SDLIST_SCROLLBAR,0);
	filepointer = 0;
	int vecto = 0;
	uint16_t jint = 0;
	//Declare filepointer
	card.getWorkDirName();
	//Text index starts at 0
	//for(jint = 0; jint < 4; jint++){//
	//genie.WriteStr(STRING_FOLDER_NAME,card.getWorkDirName());//Printing form
	
	
	if (card.cardOK){
		
		uint16_t fileCnt = card.getnrfilenames();
		//Declare filepointer
		card.getWorkDirName();
		
		while(jint < SDFILES_LIST_NUM){
			
			if(jint < fileCnt){
				
				vecto = filepointer + jint;
				
				ListFilesParsingProcedure(vecto, jint);
				
			}
			else{
				display_ButtonState(buttonsdselected[jint],0);
				//char buffer[8];
				//strcpy_P(buffer,PSTR("        "));
				display.WriteStr(stringfilename[jint],"        ");//Printing form
				//display.WriteObject(GENIE_OBJ_USERIMAGES,userimagesdlist[jint],0);
				//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][jint],0);//Printing form
				//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[1][jint],0);//Printing form
				//genie.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[2][jint],0);//Printing form
				
			}
			jint++;
		}
		
		
	}
	else{
		#ifndef ErroWindowEnable
		display_ChangeForm( FORM_INSERT_SD_CARD, 0);
		screen_sdcard = true;
		#else
		display_ChangeForm( FORM_ERROR_SCREEN, 0);
		display.WriteStr(STRING_ERROR_MESSAGE,"ERROR: INSERT SDCARD");//Printing form
		gif_processing_state = PROCESSING_ERROR;
		screen_sdcard = true;
		#endif
	}
	memset(listsd.commandline2, '\0', sizeof(listsd.commandline2) );
	
}
int get_nummaxchars(bool isfilegcode, unsigned int totalpixels){
	unsigned int totalcount=0;
	int nchars=0;
	int length_array = String(card.longFilename).length() - (isfilegcode?6:0); //avoid .gcode
	//Serial.println("length_array: ");
	//Serial.println(length_array);
	unsigned int Char_check = 0;
	
	while(nchars < length_array){
		Char_check= (unsigned int)card.longFilename[nchars];
		if(Char_check>=65 && Char_check<=90){//upper
			totalcount += pixelsize_char_uppercase[Char_check-65];
			}else if(Char_check>=97 && Char_check<=122){//lower
			totalcount += pixelsize_char_lowercase[Char_check-97];
			}else if(Char_check>=40 && Char_check<=59){//lower
			totalcount += pixelsize_char_symbols[Char_check-40];
			}else{
			totalcount += 15;
		}
		if(totalcount > totalpixels)break;
		nchars++;
	}
	/*Serial.println("N chars: ");
	Serial.println(nchars);
	Serial.println("N Prixeles: ");
	Serial.println(totalcount);*/
	return nchars;
}

void setfoldernames(int jint){
	
	unsigned int count = get_nummaxchars(false, 330);
	char buffer[50];
	memset( buffer, '\0', sizeof(buffer));
	
	if (count == 0){
		strcpy(buffer, card.filename);
		display.WriteStr(stringfilename[jint],buffer);//Printing for
		
		}else{
		
		for (unsigned int i = 0; i < count ; i++)
		{
			buffer[i]=card.longFilename[i];
		}
		if (String(card.longFilename).length() > count){
			buffer[count]='.';
			buffer[count+1]='.';
			buffer[count+2]='.';
			buffer[count+3]='\0';
		}
		
		display.WriteStr(stringfilename[jint],buffer);//Printing form
		
	}
	Serial.println(buffer);
	
	
}
void setfilenames(int jint){
	
	unsigned int count = get_nummaxchars(true, ((jint==6 || jint == 7)?260:330));
	
	char buffer[50];
	memset( buffer, '\0', sizeof(buffer));
	
	if (String(card.longFilename).length() == 0){
		strcpy(buffer, card.filename);
		display.WriteStr(stringfilename[jint],buffer);//Printing for
		
		}else{
		
		for (unsigned int i = 0; i < count ; i++)
		{
			buffer[i]=card.longFilename[i];
		}
		if ((String(card.longFilename).length() - 6) > count){
			buffer[count]='.';
			buffer[count+1]='.';
			buffer[count+2]='.';
			buffer[count+3]='\0';
		}
		
		display.WriteStr(stringfilename[jint],buffer);//Printing form
		
	}
	
	if(jint == 6){
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][0],listsd.get_hours());
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[1][0],listsd.get_minutes());
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[2][0],listsd.get_filgramos1());
		}else if(jint == 7){
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[0][1],listsd.get_hours());
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[1][1],listsd.get_minutes());
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,stringfiledur[2][1],listsd.get_filgramos1());
	}
	Serial.println(buffer);
}
void insertmetod(){
	if(!card.sdispaused){
		gif_processing_state = PROCESSING_DEFAULT;
		//genie.WriteObject(GENIE_OBJ_FORM,FORM_WAITING_ROOM,0);
		doblocking = true;
		if(Step_First_Start_Wizard){
			home_axis_from_code(false,true,false);
			home_axis_from_code(true,false,true);
			}else{
			if (!home_made) home_axis_from_code(true,true,true);
			else home_axis_from_code(true,true,false);
		}
		int feedrate;
		
		current_position[Z_AXIS]=Z_MAX_POS-15;
		
		plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], 12, active_extruder); //check speed
		st_synchronize();
		ERROR_SCREEN_WARNING2;
		/****************************************************/
		}else{
		
	}
	gif_processing_state = PROCESSING_STOP;
	display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD_FEELSTOPS,0);
	
		
}
void folder_navigation_register(bool upchdir){

	if(upchdir){
		workDir_vector[workDir_vector_lenght] = filepointer;
		workDir_vector_lenght++;
		
		}else{
		workDir_vector_lenght--;

	}
}
void Bed_Compensation_Set_Lines(int jint){
	if(Bed_Compensation_state == 2){
		Bed_Compensation_state = 3;
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
		gif_processing_state = PROCESSING_TEST;
		Bed_Compensation_Lines_Selected[0]+=jint;
		#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
		bed_test_print_code(-84.0,-220.0, 0);
		#else
		bed_test_print_code(-184.0,-220.0, 0);
		#endif
		turnoff_buttons_zcalib();
		calib_confirmation = 1888;
		display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,0);
		display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,2);
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,0);
		gif_processing_state = PROCESSING_STOP;
	}
	else if(Bed_Compensation_state == 3){
		Bed_Compensation_state = 4;
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
		gif_processing_state = PROCESSING_TEST;
		Bed_Compensation_Lines_Selected[1]+=jint;
		#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
		bed_test_print_code(82.0,-220.0, 0);
		#else
		bed_test_print_code(192.0,-220.0, 0);
		#endif
		turnoff_buttons_zcalib();
		calib_confirmation = 1888;
		display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,0);
		display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,2);
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,0);
		gif_processing_state = PROCESSING_STOP;
	}
	else if(Bed_Compensation_state == 4){
		Bed_Compensation_Lines_Selected[2]+=jint;
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBBED_ADJUSTSCREWASKFIRST,0);
		gif_processing_state = PROCESSING_BED_FIRST;
	}
	
}
void Bed_Compensation_Redo_Lines(int jint){
	if(Bed_Compensation_state == 2){
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
		gif_processing_state = PROCESSING_TEST;
		Bed_Compensation_Lines_Selected[0]+=jint;
		bed_test_print_code(0, 0, Bed_Compensation_Lines_Selected[0]);
		turnoff_buttons_zcalib();
		calib_confirmation = 1888;
		display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,0);
		display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,2);
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,0);
		gif_processing_state = PROCESSING_STOP;
	}
	else if(Bed_Compensation_state == 3){
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
		gif_processing_state = PROCESSING_TEST;
		Bed_Compensation_Lines_Selected[1]+=jint;
		#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
		bed_test_print_code(-84.0,-220.0, Bed_Compensation_Lines_Selected[1]);
		#else
		bed_test_print_code(-184.0,-220.0, Bed_Compensation_Lines_Selected[1]);
		#endif
		turnoff_buttons_zcalib();
		calib_confirmation = 1888;
		display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,0);
		display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,2);
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,0);
		gif_processing_state = PROCESSING_STOP;
	}
	else if(Bed_Compensation_state == 4){
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_PRINTINGTEST,0);
		gif_processing_state = PROCESSING_TEST;
		Bed_Compensation_Lines_Selected[2]+=jint;
		#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
		bed_test_print_code(82.0,-220.0, Bed_Compensation_Lines_Selected[2]);
		#else
		bed_test_print_code(192.0,-220.0, Bed_Compensation_Lines_Selected[2]);
		#endif
		turnoff_buttons_zcalib();
		calib_confirmation = 1888;
		display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_CONFIRM,0);
		display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,2);
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR,0);
		gif_processing_state = PROCESSING_STOP;
	}
	
}
void Z_compensation_decisor(void){
	
	
	if(abs(extruder_offset[Z_AXIS][RIGHT_EXTRUDER]) <=RAFT_Z_THRESHOLD){
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX,0);
		if(Step_First_Start_Wizard){
			display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBX_SKIP,1);
		}
		}else{
		char offset_string[250]="";
		int offset_aprox;
		offset_aprox = (int)(round(abs(extruder_offset[Z_AXIS][RIGHT_EXTRUDER])*100.01)/5.0);
		//char buffer[80];
		//sprintf_P(offset_string, PSTR("Your Sigma Z axis has been calibrated\n\nTo avoid first layer Z compensation in Mirror/Duplication Mode:\n1.Turn off the machine and install gauges \n    on %s Hotend to correct %d.%1d%1dmm\n2. Re-run a Full Calibration\n\nWarning: Hotends may be hot when turning off the machine\n "),
		sprintf_P(offset_string, PSTR("Install %d %s on the %s hotend."), offset_aprox, ((offset_aprox > 1)?"shims":"shim") , ((extruder_offset[Z_AXIS][RIGHT_EXTRUDER] < 0)?"right":"left"));
		/*if ((extruder_offset[Z_AXIS][RIGHT_EXTRUDER])<0){
		sprintf_P(offset_string, PSTR("RIGHT HOTEND %d.%1d%1d"),(int)(5*offset_aprox)/100,(int)((5*offset_aprox)/10)%10,(int)(5*offset_aprox)%10);
		}else{
		sprintf_P(offset_string, PSTR("LEFT HOTEND %d.%1d%1d"),(int)(5*offset_aprox)/100,(int)((5*offset_aprox)/10)%10,(int)(5*offset_aprox)%10);
		}*/
		
		display_ChangeForm(FORM_Z_COMPENSATION,0);
		display.WriteStr(STRING_Z_OFFSET_BETWEEN_NOZZLES,offset_string);
	}
	
}
void Calib_check_temps(void){
	static unsigned long waitPeriod_s = millis();
	if((abs(degHotend(LEFT_EXTRUDER)-degTargetHotend(LEFT_EXTRUDER))>5) || (abs(degHotend(RIGHT_EXTRUDER)-degTargetHotend(RIGHT_EXTRUDER))>5) || ((degTargetBed()-degBed())-degBed()> 2)){
		int Tref0 = (int)degHotend0();
		int Tref1 = (int)degHotend1();
		int Trefbed = (int)degBed();
		int Tfinal0 = (int)degTargetHotend(LEFT_EXTRUDER);
		int Tfinal1 = (int)degTargetHotend(RIGHT_EXTRUDER);
		int Tfinalbed = (int)(degTargetBed()-2);
		long percentage = 0;
		long percentage_old = 0;
		display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
		gif_processing_state = PROCESSING_ADJUSTING;
		while (((abs(degHotend(LEFT_EXTRUDER)-degTargetHotend(LEFT_EXTRUDER))>5) && Tfinal0!=0) || ((abs(degHotend(RIGHT_EXTRUDER)-degTargetHotend(RIGHT_EXTRUDER))>5) && Tfinal1!=0) || ((degTargetBed()-degBed())> 2)){ //Waiting to heat the extruder
			
			manage_heater();
			touchscreen_update();
			ERROR_SCREEN_WARNING2;
			
			if (millis() >= waitPeriod_s){
				char buffer[25];
				memset(buffer, '\0', sizeof(buffer) );
				int Tinstanthot0, Tinstanthot1, Tinstantbed;
				
				if(Tref0<Tfinal0){
					
					Tinstanthot0 = constrain((int)degHotend(LEFT_EXTRUDER),Tref0,Tfinal0);
				}
				else{
					
					Tinstanthot0 = constrain((int)degHotend(LEFT_EXTRUDER),Tfinal0,Tref0);
				}
				if(Tref1<Tfinal1){
					
					
					Tinstanthot1 = constrain((int)degHotend(RIGHT_EXTRUDER),Tref1,Tfinal1);
				}
				else{
					Tinstanthot1 = constrain((int)degHotend(RIGHT_EXTRUDER),Tfinal1,Tref1);
				}
				if(Trefbed < Tfinalbed){
					Tinstantbed = constrain((int)degBed(),Trefbed,Tfinalbed);
					}else{
					Tinstantbed = constrain((int)degBed(),Tfinalbed,Trefbed);
				}
				
				percentage = abs((long)Tfinal0-(long)Tref0)+abs((long)Tfinal1-(long)Tref1)+abs((long)Tfinalbed*5-(long)Trefbed*5);
				percentage= 100*(abs((long)Tinstanthot0-(long)Tref0)+abs((long)Tinstanthot1-(long)Tref1)+abs((long)Tinstantbed*5-(long)Trefbed*5))/percentage;
				if(percentage > percentage_old){
					percentage_old = percentage;
				}
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, percentage_old);
				
				waitPeriod_s=500+millis();
			}
			
			
		}
		if(!bitRead(flag_utilities_calibration_register,flag_utilities_calibration_register_bedcomensationmode)){
			gif_processing_state = PROCESSING_STOP;
			st_synchronize();
			display_ChangeForm(FORM_PROCESSING,0);
			gif_processing_state = PROCESSING_DEFAULT;
		}
	}
}
void Full_calibration_ZL_set(float offset){
	manual_fine_calib_offset[2]=0.0;
	manual_fine_calib_offset[3]=0.0;
	display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZR,0);
	if(Step_First_Start_Wizard){
		display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBZR_SKIP,1);
	}
	
	active_extruder = RIGHT_EXTRUDER;
	zprobe_zoffset+=offset;
	Config_StoreSettings(); //Store changes
}
void Full_calibration_ZR_set(float offset){
	manual_fine_calib_offset[3]=0.0;
	extruder_offset[Z_AXIS][RIGHT_EXTRUDER]+=offset;
	Z_compensation_decisor();
	Config_StoreSettings(); //Store changes
}
void Full_calibration_X_set(float offset){
	manual_fine_calib_offset[0]=0.0;
	display_ChangeForm(FORM_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBY,0);
	if(Step_First_Start_Wizard){
		display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_GOCALIBY_SKIP,1);
	}
	float calculus = extruder_offset[X_AXIS][1]+offset;
	SERIAL_PROTOCOLPGM("Calculus:  ");
	Serial.println(calculus);
	extruder_offset[X_AXIS][RIGHT_EXTRUDER]=calculus;
	Config_StoreSettings(); //Store changes
}
void Full_calibration_Y_set(float offset){
	setTargetHotend0(0);
	setTargetHotend1(0);
	setTargetBed(0);
	manual_fine_calib_offset[1]=0.0;
	if (!Step_First_Start_Wizard){
		printer_state = STATE_CALIBRATION;
		display_SetFrame(GIF_UTILITIES_CALIBRATION_SUCCESS,0);
		display_ChangeForm(FORM_UTILITIES_CALIBRATION_SUCCESS,0);
		gif_processing_state = PROCESSING_BED_SUCCESS;
		
		//char buffer[30];
		float calculus = extruder_offset[Y_AXIS][1] + offset;
		SERIAL_PROTOCOLPGM("Calculus:  ");
		Serial.println(calculus);
		//sprintf(buffer, "M218 T1 X%f",calculus); //
		//sprintf_P(buffer, PSTR("M218 T1 X%s"), String(calculus));
		//enquecommand(buffer);
		extruder_offset[Y_AXIS][RIGHT_EXTRUDER]=calculus;
		//enquecommand_P((PSTR("M218 T1 X-0.5")));
		Config_StoreSettings(); //Store changes
		//genie.WriteObject(GENIE_OBJ_FORM,FORM_MAIN_SCREEN,0);
		bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_calibfull);
		
		}else{
		display_ChangeForm(FORM_SETUPASSISTANT_SUCCESS,0);
		gif_processing_state = PROCESSING_SUCCESS_FIRST_RUN;
		FLAG_First_Start_Wizard = 888;
		Step_First_Start_Wizard = false;
		//char buffer[30];
		float calculus = extruder_offset[Y_AXIS][1] + offset;
		SERIAL_PROTOCOLPGM("Calculus:  ");
		Serial.println(calculus);
		//sprintf(buffer, "M218 T1 X%f",calculus); //
		//sprintf_P(buffer, PSTR("M218 T1 X%s"), String(calculus));
		//enquecommand(buffer);
		extruder_offset[Y_AXIS][RIGHT_EXTRUDER]=calculus;
		//enquecommand_P((PSTR("M218 T1 X-0.5")));
		Config_StoreSettings(); //Store changes
		//genie.WriteObject(GENIE_OBJ_FORM,FORM_MAIN_SCREEN,0);
		bitClear(flag_utilities_calibration_register,flag_utilities_calibration_register_calibfull);
	}
	HeaterCooldownInactivity(true);
}
void unloadfilament_procedure(void){//Removing...
	
	display_ChangeForm(FORM_PROCESSING,0);
	gif_processing_state = PROCESSING_DEFAULT;
	
	
	current_position[E_AXIS] +=PURGE_LENGHT_UNLOAD;
	
	plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], PURGE_SPEED_UNLOAD/60, which_extruder);
	
	//current_position[E_AXIS] -=EXTRUDER_LENGTH+5;
	//
	//if (axis_steps_per_unit[E_AXIS]<=493 && axis_steps_per_unit[E_AXIS]>=492){
	//plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_FAST_R18_SPEED/60, which_extruder);
	//}else{
	//plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_FAST_SPEED/60, which_extruder);
	//}
	
	current_position[E_AXIS] -= (BOWDEN_LENGTH + EXTRUDER_LENGTH + PURGE_LENGHT_UNLOAD + 100);//Extra extrusion at fast feedrate
	if (axis_steps_per_unit[E_AXIS]<=512 && axis_steps_per_unit[E_AXIS]>=492){
		plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_FAST_R19_SPEED/60, which_extruder);
		}else{
		plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], INSERT_FAST_SPEED/60, which_extruder);
	}
	
	//
	//current_position[Y_AXIS] = Y_MAX_POS-5;
	//plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], XY_TRAVEL_SPEED/60, which_extruder);
	
	st_synchronize();
	ERROR_SCREEN_WARNING;
	RESET_E_COORDINATES;
	gif_processing_state = PROCESSING_STOP;
	printer_state = STATE_LOADUNLOAD_FILAMENT;
	
	
	if(Flag_checkfil){
		
		display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_FILAMENTDETECTOR_NOTICE,(which_extruder_needs_fil== 10)?0:1);
		display_ChangeForm(FORM_FILAMENTDETECTOR_NOTICE,0);
		
		
		}else{
		display_SetFrame(GIF_UTILITIES_FILAMENT_SUCCESS,0);
		display_ChangeForm(FORM_UTILITIES_FILAMENT_SUCCESS,0);
		gif_processing_state = PROCESSING_SUCCESS;
		if(!card.sdispaused && !card.sdprinting) HeaterCooldownInactivity(true);
	}
	#ifdef DEV_BER
	if (which_extruder == 0){
		old_load_temp_l = load_temp_l;
		old_unload_temp_l = unload_temp_l;
		old_print_temp_l  = print_temp_l;
		fil_id_l = 0; // set fil id to 0
	}
	else{
		old_load_temp_r = load_temp_r;
		old_unload_temp_r = unload_temp_r;
		old_print_temp_r  = print_temp_r;
		fil_id_r = 0; //set fil id to 0
	}
	#else
	if (which_extruder == 0){
		old_load_temp_l = load_temp_l;
		old_unload_temp_l = unload_temp_l;
		old_print_temp_l  = print_temp_l;
	}
	else{
		old_load_temp_r = load_temp_r;
		old_unload_temp_r = unload_temp_r;
		old_print_temp_r  = print_temp_r;
	}
	#endif
	
	
	
}
void unload_get_ready(){
	if(which_extruder == 0) setTargetHotend(unload_temp_l,which_extruder);
	else setTargetHotend(unload_temp_r,which_extruder);
	
	gif_processing_state = PROCESSING_DEFAULT;
	display_ChangeForm(FORM_PROCESSING,0);
	
	if (home_made_Z){
		home_axis_from_code(true,true,false);
	}
	else{
		home_axis_from_code(true,true,true);
	}
	
	current_position[Z_AXIS]=Z_MAX_POS-15;
	plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder); //check speed
	
	current_position[Y_AXIS]=10;
	plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], max_feedrate[Y_AXIS], active_extruder); //check speed
	st_synchronize();
	ERROR_SCREEN_WARNING;
	
	gif_processing_state = PROCESSING_STOP;
	touchscreen_update();
	display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_ADJUSTING_TEMPERATURES, 0);
	display_ChangeForm(FORM_ADJUSTING_TEMPERATURES,0);
	Tref1 = (int)degHotend(which_extruder);
	Tfinal1 = (int)degTargetHotend(which_extruder)-CHANGE_FIL_TEMP_HYSTERESIS;
	Tpercentage_old = 0;
	touchscreen_update();
	gif_processing_state = PROCESSING_ADJUSTING;
	is_changing_filament=true; //We are changing filament
}

void go_loadfilament_next(void){
	if(which_extruder != 255){
		switch(cycle_filament){
			
			case 0:
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 1);
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"    ABS");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL2," Nylon");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL3,"    TPU");
			break;
			
			case 1:
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_FILAMENT_LOAD_FIL1,3);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 0);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 0);
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"Custom");
			
			break;
			
			case 2:
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 1);
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"    PLA");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL2,"    PVA");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL3,"  PET-G");
			break;
		}
		cycle_filament = (cycle_filament + 1)%3;
	}
}
void go_loadfilament_back(void){
	if(which_extruder != 255){
		switch(cycle_filament){
			
			case 0:
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_UTILITIES_FILAMENT_LOAD_FIL1,3);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 0);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 0);
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"Custom");
			break;
			
			case 1:
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 1);
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"    PLA");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL2,"    PVA");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL3,"  PET-G");
			break;
			
			case 2:
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL1, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL2, 1);
			display_ButtonState( BUTTON_UTILITIES_FILAMENT_LOAD_FIL3, 1);
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL1,"    ABS");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL2," Nylon");
			display.WriteStr(STRING_UTILITIES_FILAMENT_LOAD_FIL3,"    TPU");
			break;
			
		}
		cycle_filament = (cycle_filament + 2)%3;
	}
}
void go_loadfilament(uint8_t idbutton){
	
	if(idbutton == 1){
		
		saved_print_flag = 888;
		if (which_extruder == 1) // Need to pause
		{
			
			
			switch(cycle_filament){
				case 0:
				if(!card.sdispaused){
					display_ChangeForm(FORM_PROCESSING,0);
				}
				print_temp_r = PLA_PRINT_TEMP;
				load_temp_r = PLA_LOAD_TEMP;
				unload_temp_r = PLA_UNLOAD_TEMP;
				bed_temp_r = PLA_BED_TEMP;
				#ifdef DEV_BER
				fil_id_r = PLA_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend1(load_temp_r);
				insertmetod();
				break;
				
				case 1:
				if(!card.sdispaused){
					display_ChangeForm(FORM_PROCESSING,0);
				}
				print_temp_r = ABS_PRINT_TEMP;
				load_temp_r = ABS_LOAD_TEMP;
				unload_temp_r = ABS_UNLOAD_TEMP;
				bed_temp_r = ABS_BED_TEMP;
				#ifdef DEV_BER
				fil_id_r = ABS_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend1(load_temp_r);
				insertmetod();
				break;
				
				case 2:
				if(Step_First_Start_Wizard){
					//genie.WriteObject(GENIE_OBJ_USERBUTTON, BUTTON_CUSTOM_MENU, 1);
				}
				
				display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD_CUSTOM,0);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_PRINT,custom_print_temp);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_BED,custom_bed_temp);
				break;
			}
			
			//genie.WriteObject(GENIE_OBJ_FORM,FORM_INFO_FIL_INSERTED,0);
			
		}
		else if(which_extruder == 0){
			
			
			switch(cycle_filament){
				case 0:
				if(!card.sdispaused){
					display_ChangeForm(FORM_PROCESSING,0);
				}
				print_temp_l = PLA_PRINT_TEMP;
				load_temp_l = PLA_LOAD_TEMP;
				unload_temp_l = PLA_UNLOAD_TEMP;
				bed_temp_l = PLA_BED_TEMP;
				#ifdef DEV_BER
				fil_id_l = PLA_ID;
				#else
				Config_StoreSettings();
				#endif				
				setTargetHotend0(load_temp_l);
				insertmetod();
				break;
				
				case 1:
				if(!card.sdispaused){
					display_ChangeForm(FORM_PROCESSING,0);
				}
				print_temp_l = ABS_PRINT_TEMP;
				load_temp_l = ABS_LOAD_TEMP;
				unload_temp_l = ABS_UNLOAD_TEMP;
				bed_temp_l = ABS_BED_TEMP;
				#ifdef DEV_BER
				fil_id_l = ABS_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend0(load_temp_l);
				insertmetod();
				break;
				
				case 2:
				if(Step_First_Start_Wizard){
					//genie.WriteObject(GENIE_OBJ_USERBUTTON, BUTTON_CUSTOM_MENU, 1);
				}
				
				display_ChangeForm(FORM_UTILITIES_FILAMENT_LOAD_CUSTOM,0);
				
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_PRINT,custom_print_temp);
				display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_UTILITIES_FILAMENT_LOAD_CUSTOM_BED,custom_bed_temp);
				break;
			}
			
			
		}
		}else if(idbutton == 2){
		saved_print_flag = 888;
		if (which_extruder == 1) // Need to pause
		{
			
			
			switch(cycle_filament){
				case 0:
				if(!card.sdispaused){
					display_ChangeForm(FORM_PROCESSING,0);
				}
				print_temp_r = PVA_PRINT_TEMP;
				load_temp_r = PVA_LOAD_TEMP;
				unload_temp_r = PVA_UNLOAD_TEMP;
				bed_temp_r = PVA_BED_TEMP;
				#ifdef DEV_BER
				fil_id_r = PVA_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend1(load_temp_r);
				insertmetod();
				break;
				
				case 1:
				display_ChangeForm(FORM_PROCESSING,0);
				print_temp_r = NYLON_PRINT_TEMP;
				load_temp_r = NYLON_LOAD_TEMP;
				unload_temp_r = NYLON_UNLOAD_TEMP;
				bed_temp_r = NYLON_BED_TEMP;
				#ifdef DEV_BER
				fil_id_r = NYLON_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend1(load_temp_r);
				insertmetod();
				break;
				
				case 2:
				//none
				break;
			}
			
			//genie.WriteObject(GENIE_OBJ_FORM,FORM_INFO_FIL_INSERTED,0);
			
		}
		else if(which_extruder == 0){
			
			
			
			switch(cycle_filament){
				case 0:
				if(!card.sdispaused){
					display_ChangeForm(FORM_PROCESSING,0);
				}
				print_temp_l = PVA_PRINT_TEMP;
				load_temp_l = PVA_LOAD_TEMP;
				unload_temp_l = PVA_UNLOAD_TEMP;
				bed_temp_l = PVA_BED_TEMP;
				#ifdef DEV_BER
				fil_id_l = PVA_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend0(load_temp_l);
				insertmetod();
				break;
				
				case 1:
				display_ChangeForm(FORM_PROCESSING,0);
				print_temp_l = NYLON_PRINT_TEMP;
				load_temp_l = NYLON_LOAD_TEMP;
				unload_temp_l = NYLON_UNLOAD_TEMP;
				bed_temp_l = NYLON_BED_TEMP;
				#ifdef DEV_BER
				fil_id_l = NYLON_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend0(load_temp_l);
				insertmetod();
				break;
				
				case 2:
				//none
				break;
			}
			
			
		}
		}else if(idbutton == 3){
		saved_print_flag = 888;
		if (which_extruder == 1) // Need to pause
		{
			
			
			switch(cycle_filament){
				case 0:
				if(!card.sdispaused){
					display_ChangeForm(FORM_PROCESSING,0);
				}
				print_temp_r = PETG_PRINT_TEMP;
				load_temp_r = PETG_LOAD_TEMP;
				unload_temp_r = PETG_UNLOAD_TEMP;
				bed_temp_r = PETG_BED_TEMP;
				#ifdef DEV_BER
				fil_id_r = PETG_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend1(load_temp_r);
				insertmetod();
				break;
				
				case 1:
				display_ChangeForm(FORM_PROCESSING,0);
				print_temp_r = TPU_PRINT_TEMP;
				load_temp_r = TPU_LOAD_TEMP;
				unload_temp_r = TPU_UNLOAD_TEMP;
				bed_temp_r = TPU_BED_TEMP;
				#ifdef DEV_BER
				fil_id_r = TPU_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend1(load_temp_r);
				insertmetod();
				break;
				
				case 2:
				//none
				break;
			}
			
			//genie.WriteObject(GENIE_OBJ_FORM,FORM_INFO_FIL_INSERTED,0);
			
		}
		else if(which_extruder == 0){
			
			
			switch(cycle_filament){
				case 0:
				if(!card.sdispaused){
					display_ChangeForm(FORM_PROCESSING,0);
				}
				print_temp_l = PETG_PRINT_TEMP;
				load_temp_l = PETG_LOAD_TEMP;
				unload_temp_l = PETG_UNLOAD_TEMP;
				bed_temp_l = PETG_BED_TEMP;
				#ifdef DEV_BER
				fil_id_l = PETG_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend0(load_temp_l);
				insertmetod();
				break;
				
				case 1:
				display_ChangeForm(FORM_PROCESSING,0);
				print_temp_l = TPU_PRINT_TEMP;
				load_temp_l = TPU_LOAD_TEMP;
				unload_temp_l = TPU_UNLOAD_TEMP;
				bed_temp_l = TPU_BED_TEMP;
				#ifdef DEV_BER
				fil_id_l = TPU_ID;
				#else
				Config_StoreSettings();
				#endif
				setTargetHotend0(load_temp_l);
				insertmetod();
				break;
				
				case 2:
				//none
				break;
			}
			
			
		}
	}
	
}
void turnoff_buttons_xycalib(void){
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT1,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT2,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT3,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT4,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT5,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT6,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT7,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT8,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT9,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSXY_SELECT10,0);
}
void turnoff_buttons_zcalib(void){
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT1,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT2,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT3,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT4,0);
	display_ButtonState(BUTTON_UTILITIES_CALIBRATION_CALIBFULL_RESULTSZLR_SELECT5,0);
}
void show_data_printconfig(void){
	
	#if BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMA
	#ifdef ENCLOSURE_SAFETY_STOP
	
	display_ChangeForm(FORM_PRINTERSETUP_PRINTERCONFIG,0);
	
	if(Flag_FRS_enabled){
		display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS,1);
		}else{
		display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS,0);
	}
	#ifdef ENCLOSURE_SAFETY_STOP
	if(Flag_enclosure_enabled){
		display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_ENCLOSURE,1);
		}else{
		display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_ENCLOSURE,0);
	}
	#endif
	display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_L_DIGIT1,(int)which_hotend_setup[LEFT_EXTRUDER]/10);
	display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_L_DIGIT2,(int)which_hotend_setup[LEFT_EXTRUDER]%10);
	display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_R_DIGIT1,(int)which_hotend_setup[RIGHT_EXTRUDER]/10);
	display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_R_DIGIT2,(int)which_hotend_setup[RIGHT_EXTRUDER]%10);
	
	#else
	char buffer[25];
	
	if(UI_SerialID2<SERIAL_ID_THRESHOLD || (axis_steps_per_unit[E_AXIS]>=512 && axis_steps_per_unit[E_AXIS]<=492) ){
		
		display_ChangeForm(FORM_PRINTERSETUP_PRINTERCONFIG,0);
		
		if(Flag_FRS_enabled){
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS,1);
			}else{
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS,0);
		}
		
		
		if (which_extruder_setup == 1){
			sprintf_P(buffer, PSTR("R16/R17"));
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_EXTRUDER_SETUP,0);
			}else if (which_extruder_setup == 2){
			sprintf_P(buffer, PSTR("R19"));
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_EXTRUDER_SETUP,1);
			}else{
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERSETUP_EXTRUDER_SETUP,0);
			sprintf_P(buffer, PSTR("%u.%02u"),(unsigned int)axis_steps_per_unit[E_AXIS],(unsigned int)(axis_steps_per_unit[E_AXIS]*100)%100);
		}
		display.WriteStr(STRING_PRINTERSETUP_EXTRUDER_SETUP,buffer);
		
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_L_DIGIT1,(int)which_hotend_setup[LEFT_EXTRUDER]/10);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_L_DIGIT2,(int)which_hotend_setup[LEFT_EXTRUDER]%10);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_R_DIGIT1,(int)which_hotend_setup[RIGHT_EXTRUDER]/10);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_R_DIGIT2,(int)which_hotend_setup[RIGHT_EXTRUDER]%10);
		
		}else{
		display_ChangeForm(FORM_PRINTERSETUP_PRINTERCONFIG_2,0);
		
		if(Flag_FRS_enabled){
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS_2,1);
			}else{
			display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS_2,0);
		}
		
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_L_DIGIT1_2,(int)which_hotend_setup[LEFT_EXTRUDER]/10);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_L_DIGIT2_2,(int)which_hotend_setup[LEFT_EXTRUDER]%10);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_R_DIGIT1_2,(int)which_hotend_setup[RIGHT_EXTRUDER]/10);
		display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_R_DIGIT2_2,(int)which_hotend_setup[RIGHT_EXTRUDER]%10);
	}
	#endif
	#elif BCN3D_PRINTER_SETUP == BCN3D_PRINTER_IS_SIGMAX
	
	display_ChangeForm(FORM_PRINTERSETUP_PRINTERCONFIG,0);
	
	if(Flag_FRS_enabled){
		display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS,1);
		}else{
		display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_FRS,0);
	}
	#ifdef ENCLOSURE_SAFETY_STOP
	if(Flag_enclosure_enabled){
		display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_ENCLOSURE,1);
		}else{
		display_ButtonState(BUTTON_PRINTERSETUP_PRINTERCONFIG_ENCLOSURE,0);
	}
	#endif
	display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_L_DIGIT1,(int)which_hotend_setup[LEFT_EXTRUDER]/10);
	display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_L_DIGIT2,(int)which_hotend_setup[LEFT_EXTRUDER]%10);
	display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_R_DIGIT1,(int)which_hotend_setup[RIGHT_EXTRUDER]/10);
	display.WriteObject(GENIE_OBJ_CUSTOM_DIGITS,CUSTOMDIGITS_PRINTERSETUP_NOZZLE_SIZE_R_DIGIT2,(int)which_hotend_setup[RIGHT_EXTRUDER]%10);
	
	
	#endif
}

void setsdlisthours(int jint, int time_hour){
	display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[0][jint],time_hour%10);
	if(time_hour>=10){
		display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[1][jint],(time_hour/10)%10);
		if(time_hour>=100){
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[2][jint],(time_hour/100)%10);
			}else{
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[2][jint],10);
		}
		
		}else{
		display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[1][jint],10);
		display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurhor[2][jint],10);
	}
}
void setsdlistmin(int jint, int time_min){
	display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurmin[0][jint],time_min%10);
	display.WriteObject(GENIE_OBJ_USERIMAGES,sdfiledurmin[1][jint],(time_min/10)%10);
}
void setsdlistg(int jint, int weight){
	display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[0][jint],weight%10);
	if(weight>=10){
		display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[1][jint],(weight/10)%10);
		if(weight>=100){
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[2][jint],(weight/100)%10);
			
			if(weight>=1000){
				display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[3][jint],(weight/1000)%10);
				
				}else{
				display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[3][jint],10);
			}
			
			}else{
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[2][jint],10);
			display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[3][jint],10);
		}
		
		}else{
		display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[1][jint],10);
		display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[2][jint],10);
		display.WriteObject(GENIE_OBJ_USERIMAGES,sdfilepes[3][jint],10);
	}
}
bool check_regiter_num(unsigned int n){
	for(int i = 0; i<20; i++){
		if(register_codes[i]==n)return 1;
		
	}
	return 0;
}

void setregiter_num(int n){ // n 0 up to 9
	
	if(RegID_digit_count<4){
		
		switch(RegID_digit_count){
			case 0:
			RegID = n*1000;
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT3,n);
			break;
			
			case 1:
			RegID = RegID + n*100;
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT2,n);
			break;
			
			case 2:
			RegID = RegID + n*10;
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT1,n);
			break;
			
			case 3:
			RegID = RegID + n;
			display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT0,n);
			if(check_regiter_num(RegID)){
				display_ButtonState(BUTTON_PRINTERREGISTER_SUCCESS,1);
				}else{
				RegID = 0;
				RegID_digit_count = 0;
				delay(1000);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT3,10);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT2,10);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT1,10);
				display.WriteObject(GENIE_OBJ_USERIMAGES,USERIMAGE_PRINTERREGISTER_DIGIT0,10);
				return;
			}
			
			break;
		}
		
		RegID_digit_count++;
	}
	
	
}
#ifdef DEV_BER
void copy_message(uint8_t id){
	
	switch(id){
		
		case copy_status_done:
		SERIAL_PROTOCOLLNPGM("[M842]{\"status\": \"done\"}");
		break;
		
		case copy_success_false:
		SERIAL_PROTOCOLLNPGM("[M842]{\"success\": false}");
		break;
		
		case copy_success_true:
		SERIAL_PROTOCOLLNPGM("[M842]{\"success\": true}");
		break;
		
		case copy_retry_now:
		SERIAL_PROTOCOLLNPGM("[M842]{\"retry\": \"now\"}");
		break;
		
		case copy_retry_later:
		SERIAL_PROTOCOLLNPGM("[M842]{\"retry\": \"later\"}");
		break;
		
		case copy_retry_no:
		SERIAL_PROTOCOLLNPGM("[M842]{\"retry\": \"no\"}");
		break;
		
		case copy_print_cancel://request cancel
		SERIAL_PROTOCOLLNPGM("[M849]");
		break;
		
		case copy_print_pause://request pause
		SERIAL_PROTOCOLLNPGM("[M848]");
		break;
		
		case copy_print_resume://request resume
		SERIAL_PROTOCOLLNPGM("[M850]");
		break;
						
		default:
		break;
	}	
	
}
#endif
#endif /* INCLUDE */

#endif
