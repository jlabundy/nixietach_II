#define DISPLAY_WIDTH           4
#define DISPLAY_MODE_NORM       0
#define DISPLAY_MODE_DEMO       1

void display_init (void);

void display_print_num (uint16_t print_num);
void display_print_pair (uint8_t print_upper, uint8_t print_lower);

void display_update (void);

uint8_t display_mode_get (void);
void display_mode_set (uint8_t mode);

void display_force_on (void);
void display_force_off (void);