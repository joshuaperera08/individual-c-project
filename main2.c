#include <stdio.h>
#include <string.h>

//constants
#define MAX_PATIENTS         200
#define NUM_SPECIALTIES      4
#define NUM_WARDS            4
#define MAX_BEDS             20
#define NAME_LEN             50
#define FIRST_PATIENT_NUMBER 1001

#define BEDS_FILE    "beds_status.txt"
#define RECORDS_FILE "patient_records.txt"
//lookup tables
const char  *specialtyName[NUM_SPECIALTIES] = {
    "General Practice (OPD)", "Paediatrics", "Cardiology", "Neurology"
};
const double specialtyFee[NUM_SPECIALTIES]  = {1500.00, 2500.00, 4500.00, 5000.00};
const int    specialtyTime[NUM_SPECIALTIES] = {15, 20, 30, 30};   /* mins / patient */
const int    specialtyCap[NUM_SPECIALTIES]  = {30, 20, 12, 10};   /* patients / day */

const char  *wardName[NUM_WARDS] = {
    "General Ward", "Paediatric Ward", "Surgical Ward", "ICU (Intensive Care Unit)"
};
const double wardRate[NUM_WARDS]     = {3000.00, 6000.00, 12000.00, 25000.00};
const int    wardCapacity[NUM_WARDS] = {20, 10, 10, 5};

const char *urgencyLabel[4] = {"", "Normal", "Urgent", "Critical"};

int bedOccupancy[NUM_WARDS][MAX_BEDS];

//global variables..
char   pName[MAX_PATIENTS][NAME_LEN];
int    pAge[MAX_PATIENTS];
int    pUrgency[MAX_PATIENTS];
int    pSpecialty[MAX_PATIENTS];
int    pWard[MAX_PATIENTS];
int    pBed[MAX_PATIENTS];
int    pDays[MAX_PATIENTS];
double pWait[MAX_PATIENTS];
double pBase[MAX_PATIENTS];
double pSurcharge[MAX_PATIENTS];
double pWardCost[MAX_PATIENTS];
double pGross[MAX_PATIENTS];
double pDiscount[MAX_PATIENTS];
double pFinal[MAX_PATIENTS];

int queueCount[NUM_SPECIALTIES];
int patientCount = 0;
int idBase = FIRST_PATIENT_NUMBER;
int inputClosed = 0;



int  readInt(const char *prompt, int min, int max);
void readString(const char *prompt, char *dest, int size);

//formatting
void formatMoney(double amount, char *out);

//bed handling
void initBeds(void);
void loadBeds(void);
void saveBeds(void);
int  findFreeBed(int ward);
void displayBedStatus(void);
void freeBed(void);

//calculations
double calcWaitTime(int spec);
double calcSurcharge(int urgency, double baseFee);
double calcWardCost(int admitted, int ward, int days);
double calcGrossTotal(double base, double surcharge, double wardCost);
double calcDiscount(int age, double gross);
double calcFinalAmount(double gross, double discount);

// main features
void registerPatient(void);
void printBill(int i);
void displayPriorityList(void);
void generateReports(void);

// file handling
void appendRecord(int i);
int  countSavedRecords(void);



//asking for user inputs

int readInt(const char *prompt, int min, int max)
{
    char line[64];
    char extra;
    int value;

    while (1) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            inputClosed = 1;
            return min;   /* callers must check inputClosed */
        }
        /* exactly one integer and nothing else on the line */
        if (sscanf(line, "%d %c", &value, &extra) == 1 &&
            value >= min && value <= max) {
            return value;
        }
        printf("  Invalid input. Please enter a whole number from %d to %d.\n", min, max);
    }
}

void readString(const char *prompt, char *dest, int size)
{
    while (1) {
        printf("%s", prompt);
        if (fgets(dest, size, stdin) == NULL) {
            dest[0] = '\0';
            inputClosed = 1;
            return;       /* callers must check inputClosed */
        }
        dest[strcspn(dest, "\n")] = '\0';
        if (strlen(dest) > 0) {
            return;
        }
        printf("  Name cannot be empty.\n");
    }
}
//formatting
void readString(const char *prompt, char *dest, int size)
{
    while (1) {
        printf("%s", prompt);
        if (fgets(dest, size, stdin) == NULL) {
            dest[0] = '\0';
            inputClosed = 1;
            return;       /* callers must check inputClosed */
        }
        dest[strcspn(dest, "\n")] = '\0';
        if (strlen(dest) > 0) {
            return;
        }
        printf("  Name cannot be empty.\n");
    }
}


