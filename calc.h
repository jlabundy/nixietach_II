#define CALC_RPM_8CYL           30000000
#define CALC_RPM_6CYL           40000000
#define CALC_RPM_4CYL           60000000

#define CALC_THETA_8CYL         45
#define CALC_THETA_6CYL         60
#define CALC_THETA_4CYL         90

#define CALC_DWELL_OFFSET       820

#define CALC_AVG_LEVEL          3
#define CALC_AVG_DEPTH          (1 << CALC_AVG_LEVEL)

void calc_rpm_dwell_upd (uint16_t ICR1p, uint16_t ICR1n);
void calc_rpm_dwell_clear (void);

uint16_t calc_rpm_get (void);
uint8_t calc_dwell_get (void);