typedef struct obd_info {
    unsigned char s1pid00[4]; // Supported PIDs
    unsigned char load; // Engine load
    signed char temperature; // Engine coolant temperature
    unsigned short rpm; // Instantaneous engine RPM
    unsigned char speed; // Instantaneous vehicle speed
} obd_info_t;

void obd_init(void); // OBDII ISO 9141-2 initialization sequence
int get_service1_supported_pids(obd_info_t *obd_info);
int get_obd_data(obd_info_t *obd_info);