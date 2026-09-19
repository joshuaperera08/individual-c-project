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
const int    specialtyTime[NUM_SPECIALTIES] = {15, 20, 30, 30};
const int    specialtyCap[NUM_SPECIALTIES]  = {30, 20, 12, 10};

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


//user inputs
//asking for user input until user inputs an integer between min and max

int readInt(const char *prompt, int min, int max)
{
    char line[64];
    char extra;
    int value;

    while (1) {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            inputClosed = 1;
            return min;
        }
        // exactly one integer and nothing else on the line
        if (sscanf(line, "%d %c", &value, &extra) == 1 &&
            value >= min && value <= max) {
            return value;
        }
        printf("  Invalid input. Please enter a whole number from %d to %d.\n", min, max);
    }
}
// Reads a non-empty line of text and removes the trailing newline
void readString(const char *prompt, char *dest, int size)
{
    while (1) {
        printf("%s", prompt);
        if (fgets(dest, size, stdin) == NULL) {
            dest[0] = '\0';
            inputClosed = 1;
            return;       // callers must check inputClosed
        }
        dest[strcspn(dest, "\n")] = '\0';
        if (strlen(dest) > 0) {
            return;
        }
        printf("  Name cannot be empty.\n");
    }
}
//formatting
//turns for eg;- 56500.5 into 56,500.50
void formatMoney(double amount, char *out)
{
    long long cents = (long long)(amount * 100.0 + 0.5);
    long long whole = cents / 100;
    int frac = (int)(cents % 100);
    char digits[32];
    int len, pos = 0, i;

    sprintf(digits, "%lld", whole);
    len = (int)strlen(digits);
    for (i = 0; i < len; i++) {
        if (i > 0 && (len - i) % 3 == 0) {
            out[pos++] = ',';
        }
        out[pos++] = digits[i];
    }
    sprintf(out + pos, ".%02d", frac);
}

//bed handling
void initBeds(void)
{
    int w, b;
    for (w = 0; w < NUM_WARDS; w++) {
        for (b = 0; b < MAX_BEDS; b++) {
            bedOccupancy[w][b] = 0;
        }
    }
}

// Loads beds_status.txt if it exists; otherwise all beds stay available.
void loadBeds(void)
{
    FILE *fp = fopen(BEDS_FILE, "r");
    int w, b, value;

    if (fp == NULL) {
        return;
    }
    for (w = 0; w < NUM_WARDS; w++) {
        for (b = 0; b < wardCapacity[w]; b++) {
            if (fscanf(fp, "%d", &value) != 1) {
                fclose(fp);
                return;
            }
            bedOccupancy[w][b] = (value != 0) ? 1 : 0;
        }
    }
    fclose(fp);
}

// One line per ward, one 0/1 per bed
void saveBeds(void)
{
    FILE *fp = fopen(BEDS_FILE, "w");
    int w, b;

    if (fp == NULL) {
        printf("Warning: could not write %s\n", BEDS_FILE);
        return;
    }
    for (w = 0; w < NUM_WARDS; w++) {
        for (b = 0; b < wardCapacity[w]; b++) {
            fprintf(fp, "%d ", bedOccupancy[w][b]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}
// Returns the first free bed index in a ward, or -1 if the ward is full
int findFreeBed(int ward)
{
    int b;
    for (b = 0; b < wardCapacity[ward]; b++) {
        if (bedOccupancy[ward][b] == 0) {
            return b;
        }
    }
    return -1;
}

//displaying bed availability
void displayBedStatus(void)
{
    int w, b, occupied;
 
    printf("\n==================== BED STATUS ====================\n");
    printf("  ( - = Available, X = Occupied )\n");
    for (w = 0; w < NUM_WARDS; w++) {
        occupied = 0;
        printf("\nWard %d: %s\n  ", w + 1, wardName[w]);
        for (b = 0; b < wardCapacity[w]; b++) {
            printf("[%02d:%c] ", b + 1, bedOccupancy[w][b] ? 'X' : '-');
            if (bedOccupancy[w][b]) {
                occupied++;
            }
            if ((b + 1) % 10 == 0 && b + 1 < wardCapacity[w]) {
                printf("\n  ");
            }
        }
        printf("\n  Occupied: %d / %d\n", occupied, wardCapacity[w]);
    }
    printf("====================================================\n");
}
//Discharge patients,make beds free
void freeBed(void)
{
    int ward, bed;
 
    printf("\n--- Free a Bed (Discharge) ---\n");
    ward = readInt("Ward ID (1-4): ", 1, NUM_WARDS) - 1;
    bed  = readInt("Bed number: ", 1, wardCapacity[ward]) - 1;
 
    if (inputClosed) {
        return;
    }
 
    if (bedOccupancy[ward][bed] == 0) {
        printf("That bed is already available.\n");
        return;
    }
    bedOccupancy[ward][bed] = 0;
    saveBeds();
    printf("Bed #%02d in %s is now available.\n", bed + 1, wardName[ward]);
}

//calculations
//waiting time of a patient
double calcWaitTime(int spec)
{
    return queueCount[spec] * specialtyTime[spec];
}

//charge for patients of urgency 3
double calcSurcharge(int urgency, double baseFee)
{
    if (urgency == 2) {
        return baseFee * 0.20;
    }
    if (urgency == 3) {
        return baseFee * 0.50;
    }
    return 0.0;
}

//charge if admitted in a ward
double calcWardCost(int admitted, int ward, int days)
{
    if (!admitted) {
        return 0.0;
    }
    return days * wardRate[ward];
}

//total gross bill
double calcGrossTotal(double base, double surcharge, double wardCost)
{
    return base + surcharge + wardCost;
}

//check discount availability
double calcDiscount(int age, double gross)
{
    if (age < 5 || age > 65) {
        return gross * 0.15;
    }
    return 0.0;
}

//final bill
double calcFinalAmount(double gross, double discount)
{
 return gross - discount;
}

//patient intake
void registerPatient(void)
{
    int i = patientCount;
    int spec, admitted, ward = -1, bed = -1, days = 0, s;
 
    if (patientCount >= MAX_PATIENTS) {
        printf("\nPatient list is full (%d). Cannot register more today.\n", MAX_PATIENTS);
        return;
    }
 
    printf("\n--- New Patient Registration ---\n");
    readString("Patient Name: ", pName[i], NAME_LEN);
    pAge[i]     = readInt("Patient Age (0-120): ", 0, 120);
    pUrgency[i] = readInt("Triage Level (1 = Normal, 2 = Urgent, 3 = Critical): ", 1, 3);
 
    printf("\nSpecialties:\n");
    for (s = 0; s < NUM_SPECIALTIES; s++) {
        printf("  %d. %s (%d/%d booked today)\n",
               s + 1, specialtyName[s], queueCount[s], specialtyCap[s]);
    }
    spec = readInt("Specialty ID (1-4): ", 1, NUM_SPECIALTIES) - 1;
 
    if (queueCount[spec] >= specialtyCap[spec]) {
        printf("\nSorry, %s has reached its daily cap of %d patients.\n",
               specialtyName[spec], specialtyCap[spec]);
        return;
    }
 
    admitted = readInt("Admit to a ward? (1 = Yes, 0 = No): ", 0, 1);
    if (admitted) {
        while (1) {
            ward = readInt("Ward ID (1-4, or 0 to register as outpatient): ", 0, NUM_WARDS) - 1;
            if (ward < 0) {
                admitted = 0;
                break;
            }
            bed = findFreeBed(ward);
            if (bed >= 0) {
                break;
            }
            printf("  %s is full. Choose another ward.\n", wardName[ward]);
        }
        if (admitted) {
            days = readInt("Days Admitted (1-365): ", 1, 365);
        }
    }
 
    if (inputClosed) {
        return;   // input ended mid-registration: discard it
    }
 
    // everything valid: store the record
    pSpecialty[i] = spec;
    pWard[i]      = admitted ? ward : -1;
    pBed[i]       = admitted ? bed : -1;
    pDays[i]      = admitted ? days : 0;
 
    pWait[i] = calcWaitTime(spec);      // uses the queue BEFORE this patient
    queueCount[spec]++;
 
    if (admitted) {
        bedOccupancy[ward][bed] = 1;
    }
 
    pBase[i]      = specialtyFee[spec];
    pSurcharge[i] = calcSurcharge(pUrgency[i], pBase[i]);
    pWardCost[i]  = calcWardCost(admitted, ward, days);
    pGross[i]     = calcGrossTotal(pBase[i], pSurcharge[i], pWardCost[i]);
    pDiscount[i]  = calcDiscount(pAge[i], pGross[i]);
    pFinal[i]     = calcFinalAmount(pGross[i], pDiscount[i]);
 
    patientCount++;
 
    printBill(i);
    appendRecord(i);
    saveBeds();
}


