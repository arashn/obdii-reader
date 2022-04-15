unsigned char s1pid00[4]; // Supported PIDs

unsigned char load; // Engine load
signed char temperature; // Engine coolant temperature
unsigned short rpm; // Instantaneous engine RPM
unsigned char speed; // Instantaneous vehicle speed

void obd_init(void); // OBDII ISO 9141-2 initialization sequence
int get_service1_supported_pids(void);
int get_engine_load(void);
int get_engine_coolant_temp(void);
int get_engine_rpm(void);
int get_vehicle_speed(void);