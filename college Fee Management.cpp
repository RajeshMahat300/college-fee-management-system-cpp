/*
  COLLEGE FEE MANAGEMENT SYSTEM  (console, single file)
  Login: admin / admin123        Build: g++ -o fee FeeManagement1000.cpp
  Data files (text, '|' separated): students.txt fee_structures.txt payments.txt adjustments.txt
  Balance formula: base fee - discounts + fines - payments
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <cstdlib>
using namespace std;

const string F_STU = "students.txt";
const string F_STR = "fee_structures.txt";
const string F_PAY = "payments.txt";
const string F_ADJ = "adjustments.txt";

// ===================== 1. HELPER FUNCTIONS =====================
string trim(string s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
string upper(string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = toupper(s[i]);
    return s;
}
string lower(string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = tolower(s[i]);
    return s;
}
vector<string> splitLine(string line) {
    vector<string> parts;
    string cur = "";
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '|') { parts.push_back(cur); cur = ""; }
        else cur += line[i];
    }
    parts.push_back(cur);
    return parts;
}
string money(double v) {
    ostringstream o;
    o << fixed << setprecision(2) << v;
    return o.str();
}
string shorten(string s, size_t n) {
    return s.size() <= n ? s : s.substr(0, n - 2) + "..";
}
void line() { cout << string(75, '-') << "\n"; }
void title(string t) { cout << "\n" << string(75, '=') << "\n   " << t << "\n" << string(75, '=') << "\n"; }
void pauseScreen() { string t; cout << "\nPress Enter to continue..."; getline(cin, t); }

// ---- safe input ----
string readLine(string msg) {
    string s;
    cout << msg;
    if (!getline(cin, s)) { cout << "\nInput closed. Goodbye.\n"; exit(0); }
    return s;
}
string clean(string s) {                       // '|' is our file separator, so replace it
    for (size_t i = 0; i < s.size(); i++) if (s[i] == '|') s[i] = '/';
    return s;
}
string readText(string msg) {                  // text that cannot be empty
    while (true) {
        string s = trim(readLine(msg));
        if (s != "") return clean(s);
        cout << "  Input cannot be empty.\n";
    }
}
string readOptional(string msg) {              // empty becomes "-"
    string s = trim(readLine(msg));
    return s == "" ? "-" : clean(s);
}
int readInt(string msg, int lo, int hi) {
    while (true) {
        string s = trim(readLine(msg));
        char* end;
        long v = strtol(s.c_str(), &end, 10);
        if (s == "" || *end != '\0') { cout << "  Enter a whole number.\n"; continue; }
        if (v < lo || v > hi) { cout << "  Number must be between " << lo << " and " << hi << ".\n"; continue; }
        return (int)v;
    }
}
double readDouble(string msg, double lo, double hi) {
    while (true) {
        string s = trim(readLine(msg));
        char* end;
        double v = strtod(s.c_str(), &end);
        if (s == "" || *end != '\0') { cout << "  Enter a valid amount.\n"; continue; }
        if (v < lo || v > hi) { cout << "  Amount must be between " << money(lo) << " and " << money(hi) << ".\n"; continue; }
        return v;
    }
}
bool askYesNo(string msg) {
    while (true) {
        string s = lower(trim(readLine(msg + " (y/n): ")));
        if (s == "y" || s == "yes") return true;
        if (s == "n" || s == "no") return false;
        cout << "  Type y or n.\n";
    }
}
string readPhone(string msg) {
    while (true) {
        string s = trim(readLine(msg));
        bool ok = s.size() >= 7 && s.size() <= 15;
        for (size_t i = 0; i < s.size(); i++) if (!isdigit(s[i])) ok = false;
        if (ok) return s;
        cout << "  Phone must have 7 to 15 digits.\n";
    }
}
string readEmail(string msg) {
    while (true) {
        string s = trim(readLine(msg));
        if (s == "") return "-";
        size_t at = s.find('@');
        if (at != string::npos && at > 0 && s.find('.', at) != string::npos && s.find(' ') == string::npos) return s;
        cout << "  Invalid email.\n";
    }
}

// ---- dates (YYYY-MM-DD) ----
string today() {
    time_t t = time(0);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
    return string(buf);
}
bool isValidDate(string d) {
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    for (int i = 0; i < 10; i++) if (i != 4 && i != 7 && !isdigit(d[i])) return false;
    int y = atoi(d.substr(0, 4).c_str()), m = atoi(d.substr(5, 2).c_str()), day = atoi(d.substr(8, 2).c_str());
    if (y < 1990 || y > 2100 || m < 1 || m > 12 || day < 1) return false;
    int dim[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) dim[1] = 29;
    return day <= dim[m - 1];
}
string readDate(string msg) {                  // Enter = today
    while (true) {
        string s = trim(readLine(msg));
        if (s == "") return today();
        if (isValidDate(s)) return s;
        cout << "  Invalid date. Use YYYY-MM-DD.\n";
    }
}
int daysBetween(string d1, string d2) {
    tm a = tm(), b = tm();
    a.tm_year = atoi(d1.substr(0, 4).c_str()) - 1900;
    a.tm_mon = atoi(d1.substr(5, 2).c_str()) - 1;
    a.tm_mday = atoi(d1.substr(8, 2).c_str());
    a.tm_hour = 12;
    b.tm_year = atoi(d2.substr(0, 4).c_str()) - 1900;
    b.tm_mon = atoi(d2.substr(5, 2).c_str()) - 1;
    b.tm_mday = atoi(d2.substr(8, 2).c_str());
    b.tm_hour = 12;
    return (int)(difftime(mktime(&b), mktime(&a)) / 86400.0 + 0.5);
}

// ---- reading / writing whole text files ----
vector<string> readFile(string name) {
    vector<string> lines;
    ifstream in(name.c_str());
    string l;
    while (getline(in, l)) if (trim(l) != "") lines.push_back(l);
    return lines;
}
void writeFile(string name, vector<string>& lines) {
    ofstream out(name.c_str());
    for (size_t i = 0; i < lines.size(); i++) out << lines[i] << "\n";
}

// ===================== 2. CLASSES =====================
class Person {                                 // base class
protected:
    string name, phone, email;
public:
    Person() {}
    Person(string n, string p, string e) : name(n), phone(p), email(e) {}
    string getName() { return name; }
    string getPhone() { return phone; }
    void setName(string n) { name = n; }
    void setPhone(string p) { phone = p; }
    void setEmail(string e) { email = e; }
};

class Student : public Person {                // Student is a Person
    string id, course, structureId, status, admissionDate;
    int year;
public:
    Student() { year = 1; status = "Active"; }
    Student(string i, string n, string p, string e, string c, int y, string s, string d) : Person(n, p, e) {
        id = i; course = c; year = y; structureId = s; status = "Active"; admissionDate = d;
    }
    string getId() { return id; }
    string getCourse() { return course; }
    int getYear() { return year; }
    string getStructureId() { return structureId; }
    string getStatus() { return status; }
    string getAdmissionDate() { return admissionDate; }
    void setCourse(string c) { course = c; }
    void setYear(int y) { year = y; }
    void setStructureId(string s) { structureId = s; }
    void setStatus(string s) { status = s; }
    void show() {
        cout << "  Student ID     : " << id << "\n  Name           : " << name << "\n  Phone          : " << phone
             << "\n  Email          : " << email << "\n  Course / Year  : " << course << " / " << year
             << "\n  Fee structure  : " << structureId << "\n  Status         : " << status
             << "\n  Admission date : " << admissionDate << "\n";
    }
    string toLine() {
        ostringstream o;
        o << id << "|" << name << "|" << phone << "|" << email << "|" << course << "|" << year << "|"
          << structureId << "|" << status << "|" << admissionDate;
        return o.str();
    }
    bool fromLine(string l) {
        vector<string> p = splitLine(l);
        if (p.size() != 9) return false;
        id = p[0]; name = p[1]; phone = p[2]; email = p[3]; course = p[4];
        year = atoi(p[5].c_str()); structureId = p[6]; status = p[7]; admissionDate = p[8];
        return true;
    }
};

class FeeStructure {                           // fee amounts for a course/year
    string id, course;
    int year;
    double head[6];                            // tuition, lab, library, exam, hostel, other
public:
    static string headName(int i) {
        string names[6] = {"Tuition", "Lab", "Library", "Exam", "Hostel", "Other"};
        return names[i];
    }
    FeeStructure() { year = 1; for (int i = 0; i < 6; i++) head[i] = 0; }
    FeeStructure(string i, string c, int y, double h[6]) {
        id = i; course = c; year = y;
        for (int k = 0; k < 6; k++) head[k] = h[k];
    }
    string getId() { return id; }
    string getCourse() { return course; }
    int getYear() { return year; }
    double getHead(int i) { return head[i]; }
    void setHead(int i, double v) { head[i] = v; }
    void setCourse(string c) { course = c; }
    void setYear(int y) { year = y; }
    double getTotal() {
        double t = 0;
        for (int i = 0; i < 6; i++) t += head[i];
        return t;
    }
    void show() {
        cout << "  Structure ID : " << id << "\n  Course/Year  : " << course << " / " << year << "\n";
        for (int i = 0; i < 6; i++) cout << "  " << left << setw(13) << headName(i) << ": " << money(head[i]) << "\n";
        cout << "  TOTAL FEE    : " << money(getTotal()) << "\n";
    }
    string toLine() {
        ostringstream o;
        o << id << "|" << course << "|" << year;
        for (int i = 0; i < 6; i++) o << "|" << head[i];
        return o.str();
    }
    bool fromLine(string l) {
        vector<string> p = splitLine(l);
        if (p.size() != 9) return false;
        id = p[0]; course = p[1]; year = atoi(p[2].c_str());
        for (int i = 0; i < 6; i++) head[i] = atof(p[3 + i].c_str());
        return true;
    }
};

class Payment {                                // one fee payment
    int receiptNo;
    string studentId, date, mode, remarks;
    double amount;
public:
    Payment() { receiptNo = 0; amount = 0; }
    Payment(int r, string s, string d, double a, string m, string rem) {
        receiptNo = r; studentId = s; date = d; amount = a; mode = m; remarks = rem;
    }
    int getReceiptNo() { return receiptNo; }
    string getStudentId() { return studentId; }
    string getDate() { return date; }
    double getAmount() { return amount; }
    string getMode() { return mode; }
    string getRemarks() { return remarks; }
    void setAmount(double a) { amount = a; }
    void setRemarks(string r) { remarks = r; }
    string toLine() {
        ostringstream o;
        o << receiptNo << "|" << studentId << "|" << date << "|" << amount << "|" << mode << "|" << remarks;
        return o.str();
    }
    bool fromLine(string l) {
        vector<string> p = splitLine(l);
        if (p.size() != 6) return false;
        receiptNo = atoi(p[0].c_str()); studentId = p[1]; date = p[2];
        amount = atof(p[3].c_str()); mode = p[4]; remarks = p[5];
        return true;
    }
};

class Adjustment {                             // a DISCOUNT (less to pay) or a FINE (more to pay)
    int id;
    string studentId, type, reason, date;
    double amount;
public:
    Adjustment() { id = 0; amount = 0; }
    Adjustment(int i, string s, string t, double a, string r, string d) {
        id = i; studentId = s; type = t; amount = a; reason = r; date = d;
    }
    int getId() { return id; }
    string getStudentId() { return studentId; }
    string getType() { return type; }
    double getAmount() { return amount; }
    string getReason() { return reason; }
    string getDate() { return date; }
    string toLine() {
        ostringstream o;
        o << id << "|" << studentId << "|" << type << "|" << amount << "|" << reason << "|" << date;
        return o.str();
    }
    bool fromLine(string l) {
        vector<string> p = splitLine(l);
        if (p.size() != 6) return false;
        id = atoi(p[0].c_str()); studentId = p[1]; type = p[2];
        amount = atof(p[3].c_str()); reason = p[4]; date = p[5];
        return true;
    }
};

// ===================== 3. DATA LISTS, FILES, SEARCH =====================
vector<Student> students;
vector<FeeStructure> structures;
vector<Payment> payments;
vector<Adjustment> adjustments;

void loadAll() {
    vector<string> lines = readFile(F_STU);
    for (size_t i = 0; i < lines.size(); i++) { Student s; if (s.fromLine(lines[i])) students.push_back(s); }
    lines = readFile(F_STR);
    for (size_t i = 0; i < lines.size(); i++) { FeeStructure f; if (f.fromLine(lines[i])) structures.push_back(f); }
    lines = readFile(F_PAY);
    for (size_t i = 0; i < lines.size(); i++) { Payment p; if (p.fromLine(lines[i])) payments.push_back(p); }
    lines = readFile(F_ADJ);
    for (size_t i = 0; i < lines.size(); i++) { Adjustment a; if (a.fromLine(lines[i])) adjustments.push_back(a); }
}
void saveStudents() {
    vector<string> lines;
    for (size_t i = 0; i < students.size(); i++) lines.push_back(students[i].toLine());
    writeFile(F_STU, lines);
}
void saveStructures() {
    vector<string> lines;
    for (size_t i = 0; i < structures.size(); i++) lines.push_back(structures[i].toLine());
    writeFile(F_STR, lines);
}
void savePayments() {
    vector<string> lines;
    for (size_t i = 0; i < payments.size(); i++) lines.push_back(payments[i].toLine());
    writeFile(F_PAY, lines);
}
void saveAdjustments() {
    vector<string> lines;
    for (size_t i = 0; i < adjustments.size(); i++) lines.push_back(adjustments[i].toLine());
    writeFile(F_ADJ, lines);
}

// find functions return the position in the list, or -1 when not found
int findStudent(string id) {
    for (size_t i = 0; i < students.size(); i++) if (students[i].getId() == id) return i;
    return -1;
}
int findStructure(string id) {
    for (size_t i = 0; i < structures.size(); i++) if (structures[i].getId() == id) return i;
    return -1;
}
int findPayment(int no) {
    for (size_t i = 0; i < payments.size(); i++) if (payments[i].getReceiptNo() == no) return i;
    return -1;
}
int findAdjustment(int id) {
    for (size_t i = 0; i < adjustments.size(); i++) if (adjustments[i].getId() == id) return i;
    return -1;
}
int nextReceiptNo() {
    int high = 1000;
    for (size_t i = 0; i < payments.size(); i++) if (payments[i].getReceiptNo() > high) high = payments[i].getReceiptNo();
    return high + 1;
}
int nextAdjustmentId() {
    int high = 0;
    for (size_t i = 0; i < adjustments.size(); i++) if (adjustments[i].getId() > high) high = adjustments[i].getId();
    return high + 1;
}

// ===================== 4. FEE CALCULATION =====================
double getBaseFee(Student& s) {
    int pos = findStructure(s.getStructureId());
    return pos < 0 ? 0 : structures[pos].getTotal();
}
double sumAdjustments(string studentId, string type) {
    double total = 0;
    for (size_t i = 0; i < adjustments.size(); i++)
        if (adjustments[i].getStudentId() == studentId && adjustments[i].getType() == type) total += adjustments[i].getAmount();
    return total;
}
double getTotalPaid(string studentId) {
    double total = 0;
    for (size_t i = 0; i < payments.size(); i++) if (payments[i].getStudentId() == studentId) total += payments[i].getAmount();
    return total;
}
double getNetFee(Student& s) {
    return getBaseFee(s) - sumAdjustments(s.getId(), "DISCOUNT") + sumAdjustments(s.getId(), "FINE");
}
double getBalance(Student& s) {                // outstanding balance
    double b = getNetFee(s) - getTotalPaid(s.getId());
    return (b > -0.005 && b < 0.005) ? 0 : b;
}
string getFeeStatus(Student& s) {
    double b = getBalance(s);
    if (b < 0) return "ADVANCE";
    if (b == 0) return getNetFee(s) > 0 ? "PAID" : "NO DUES";
    return getTotalPaid(s.getId()) < 0.005 ? "UNPAID" : "PARTIAL";
}
void showFeeSummary(Student& s) {
    string id = s.getId();
    cout << right << "  Base fee        : " << setw(12) << money(getBaseFee(s))
         << "\n  (-) Discounts   : " << setw(12) << money(sumAdjustments(id, "DISCOUNT"))
         << "\n  (+) Fines       : " << setw(12) << money(sumAdjustments(id, "FINE"))
         << "\n  Net fee         : " << setw(12) << money(getNetFee(s))
         << "\n  (-) Paid so far : " << setw(12) << money(getTotalPaid(id))
         << "\n  OUTSTANDING     : " << setw(12) << money(getBalance(s)) << "  [" << getFeeStatus(s) << "]\n" << left;
}

// ===================== 5. TABLE DISPLAY =====================
void showStudentTable(vector<Student>& v) {
    if (v.empty()) { cout << "  No students found.\n"; return; }
    line();
    cout << left << setw(8) << "ID" << setw(22) << "Name" << setw(9) << "Course" << setw(5) << "Yr" << setw(10) << "Status"
         << right << setw(11) << "Balance" << "  " << left << "Fee status\n";
    line();
    for (size_t i = 0; i < v.size(); i++)
        cout << left << setw(8) << v[i].getId() << setw(22) << shorten(v[i].getName(), 21) << setw(9) << shorten(v[i].getCourse(), 8)
             << setw(5) << v[i].getYear() << setw(10) << v[i].getStatus() << right << setw(11) << money(getBalance(v[i]))
             << "  " << left << getFeeStatus(v[i]) << "\n";
    line();
}
void showStructureTable(vector<FeeStructure>& v) {
    if (v.empty()) { cout << "  No fee structures found.\n"; return; }
    line();
    cout << left << setw(8) << "ID" << setw(10) << "Course" << setw(5) << "Yr" << right;
    for (int h = 0; h < 6; h++) cout << setw(9) << FeeStructure::headName(h);
    cout << setw(11) << "TOTAL" << "\n";
    line();
    for (size_t i = 0; i < v.size(); i++) {
        cout << left << setw(8) << v[i].getId() << setw(10) << shorten(v[i].getCourse(), 9) << setw(5) << v[i].getYear() << right;
        for (int h = 0; h < 6; h++) cout << setw(9) << (long)v[i].getHead(h);
        cout << setw(11) << money(v[i].getTotal()) << "\n";
    }
    line();
    cout << left;
}
void showPaymentTable(vector<Payment>& v) {
    if (v.empty()) { cout << "  No payments found.\n"; return; }
    line();
    cout << left << setw(9) << "Receipt" << setw(9) << "Student" << setw(12) << "Date" << right << setw(12) << "Amount"
         << "  " << left << setw(9) << "Mode" << "Remarks\n";
    line();
    double total = 0;
    for (size_t i = 0; i < v.size(); i++) {
        cout << left << setw(9) << v[i].getReceiptNo() << setw(9) << v[i].getStudentId() << setw(12) << v[i].getDate()
             << right << setw(12) << money(v[i].getAmount()) << "  " << left << setw(9) << v[i].getMode() << shorten(v[i].getRemarks(), 22) << "\n";
        total += v[i].getAmount();
    }
    line();
    cout << "  Payments: " << v.size() << "     Total amount: " << money(total) << "\n";
}
void showAdjustmentTable(vector<Adjustment>& v) {
    if (v.empty()) { cout << "  No discounts or fines found.\n"; return; }
    line();
    cout << left << setw(6) << "ID" << setw(9) << "Student" << setw(10) << "Type" << right << setw(12) << "Amount"
         << "  " << left << setw(12) << "Date" << "Reason\n";
    line();
    for (size_t i = 0; i < v.size(); i++)
        cout << left << setw(6) << v[i].getId() << setw(9) << v[i].getStudentId() << setw(10) << v[i].getType() << right
             << setw(12) << money(v[i].getAmount()) << "  " << left << setw(12) << v[i].getDate() << shorten(v[i].getReason(), 28) << "\n";
    line();
}

// ===================== 6. FEE STRUCTURE MENU =====================
void addStructure() {
    title("ADD FEE STRUCTURE");
    string id;
    while (true) {
        id = upper(readText("Structure ID (example FS01): "));
        if (id.find(' ') != string::npos) cout << "  ID must not contain spaces.\n";
        else if (findStructure(id) >= 0) cout << "  Duplicate ID! This structure already exists.\n";
        else break;
    }
    string course = upper(readText("Course (example BCA): "));
    int year = readInt("Year (1-8): ", 1, 8);
    double h[6];
    cout << "Enter each fee amount (0 if not needed):\n";
    for (int i = 0; i < 6; i++) h[i] = readDouble("  " + FeeStructure::headName(i) + " fee: ", 0, 10000000);
    FeeStructure f(id, course, year, h);
    if (f.getTotal() <= 0) { cout << "  Total fee must be more than zero. Not saved.\n"; return; }
    structures.push_back(f);
    saveStructures();
    cout << "  Saved. Total fee = " << money(f.getTotal()) << "\n";
}
void updateStructure() {
    title("UPDATE FEE STRUCTURE");
    int pos = findStructure(upper(readText("Structure ID to update: ")));
    if (pos < 0) { cout << "  Fee structure not found.\n"; return; }
    structures[pos].show();
    cout << "\n  1. Course\n  2. Year\n  3. Tuition\n  4. Lab\n  5. Library\n  6. Exam\n  7. Hostel\n  8. Other\n  0. Cancel\n";
    int c = readInt("Field to change: ", 0, 8);
    if (c == 0) return;
    if (c == 1) structures[pos].setCourse(upper(readText("New course: ")));
    else if (c == 2) structures[pos].setYear(readInt("New year (1-8): ", 1, 8));
    else structures[pos].setHead(c - 3, readDouble("New amount: ", 0, 10000000));
    saveStructures();
    cout << "  Updated. New total = " << money(structures[pos].getTotal()) << "\n";
}
void deleteStructure() {
    title("DELETE FEE STRUCTURE");
    string id = upper(readText("Structure ID to delete: "));
    int pos = findStructure(id);
    if (pos < 0) { cout << "  Fee structure not found.\n"; return; }
    for (size_t i = 0; i < students.size(); i++)
        if (students[i].getStructureId() == id) { cout << "  Cannot delete: students are using this structure.\n"; return; }
    if (askYesNo("Delete structure " + id + "?")) {
        structures.erase(structures.begin() + pos);
        saveStructures();
        cout << "  Deleted.\n";
    }
}
void structureMenu() {
    while (true) {
        title("FEE STRUCTURE MANAGEMENT");
        cout << "  1. Add structure\n  2. View all structures\n  3. Update structure\n  4. Delete structure\n  0. Back\n";
        int c = readInt("Enter choice: ", 0, 4);
        if (c == 0) return;
        if (c == 1) addStructure();
        if (c == 2) { title("ALL FEE STRUCTURES"); showStructureTable(structures); }
        if (c == 3) updateStructure();
        if (c == 4) deleteStructure();
        pauseScreen();
    }
}

// ===================== 7. STUDENT MENU =====================
void addStudent() {
    title("ADD NEW STUDENT");
    if (structures.empty()) { cout << "  Add a fee structure first.\n"; return; }
    string id;
    while (true) {
        id = upper(readText("Student ID (example S101): "));
        if (id.find(' ') != string::npos) cout << "  ID must not contain spaces.\n";
        else if (findStudent(id) >= 0) cout << "  Duplicate ID! This student already exists.\n";
        else break;
    }
    string name = readText("Full name: ");
    string phone = readPhone("Phone: ");
    string email = readEmail("Email (Enter to skip): ");
    string course = upper(readText("Course (example BCA): "));
    int year = readInt("Year (1-8): ", 1, 8);
    showStructureTable(structures);
    string sid;
    while (true) {
        sid = upper(readText("Fee structure ID: "));
        if (findStructure(sid) >= 0) break;
        cout << "  Fee structure not found. Pick one from the table.\n";
    }
    string date = readDate("Admission date (YYYY-MM-DD, Enter = today): ");
    students.push_back(Student(id, name, phone, email, course, year, sid, date));
    saveStudents();
    cout << "  Student added.\n";
}
void viewOneStudent() {
    int pos = findStudent(upper(readText("Student ID: ")));
    if (pos < 0) { cout << "  Student not found.\n"; return; }
    title("STUDENT PROFILE");
    students[pos].show();
    cout << "\n  --- Fee summary ---\n";
    showFeeSummary(students[pos]);
}
void updateStudent() {
    title("UPDATE STUDENT");
    int pos = findStudent(upper(readText("Student ID to update: ")));
    if (pos < 0) { cout << "  Student not found.\n"; return; }
    students[pos].show();
    cout << "\n  1. Name\n  2. Phone\n  3. Email\n  4. Course\n  5. Year\n  6. Fee structure\n  7. Status\n  0. Cancel\n";
    int c = readInt("Field to change: ", 0, 7);
    if (c == 0) return;
    if (c == 1) students[pos].setName(readText("New name: "));
    else if (c == 2) students[pos].setPhone(readPhone("New phone: "));
    else if (c == 3) students[pos].setEmail(readEmail("New email: "));
    else if (c == 4) students[pos].setCourse(upper(readText("New course: ")));
    else if (c == 5) students[pos].setYear(readInt("New year (1-8): ", 1, 8));
    else if (c == 6) {
        showStructureTable(structures);
        while (true) {
            string sid = upper(readText("New fee structure ID: "));
            if (findStructure(sid) >= 0) { students[pos].setStructureId(sid); break; }
            cout << "  Fee structure not found.\n";
        }
    } else {
        cout << "  1. Active\n  2. Inactive\n";
        students[pos].setStatus(readInt("Choice: ", 1, 2) == 1 ? "Active" : "Inactive");
    }
    saveStudents();
    cout << "  Student updated.\n";
}
void deleteStudent() {
    title("DELETE STUDENT");
    string id = upper(readText("Student ID to delete: "));
    int pos = findStudent(id);
    if (pos < 0) { cout << "  Student not found.\n"; return; }
    students[pos].show();
    if (getTotalPaid(id) > 0) cout << "\n  WARNING: this student's payments will also be deleted.\n";
    if (!askYesNo("Delete this student and all fee records?")) return;
    for (int i = (int)payments.size() - 1; i >= 0; i--)
        if (payments[i].getStudentId() == id) payments.erase(payments.begin() + i);
    for (int i = (int)adjustments.size() - 1; i >= 0; i--)
        if (adjustments[i].getStudentId() == id) adjustments.erase(adjustments.begin() + i);
    students.erase(students.begin() + pos);
    saveStudents(); savePayments(); saveAdjustments();
    cout << "  Student deleted.\n";
}
void searchStudent() {
    title("SEARCH STUDENT");
    string key = lower(readText("Keyword (ID, name, course or phone): "));
    vector<Student> found;
    for (size_t i = 0; i < students.size(); i++) {
        string all = lower(students[i].getId() + " " + students[i].getName() + " " + students[i].getCourse() + " " + students[i].getPhone());
        if (all.find(key) != string::npos) found.push_back(students[i]);
    }
    showStudentTable(found);
}
// simple bubble sort: by 1 = name, 2 = ID, 3 = balance (highest first)
void sortStudentList(vector<Student>& v, int by) {
    for (int i = 0; i < (int)v.size() - 1; i++) {
        for (int j = 0; j < (int)v.size() - 1 - i; j++) {
            bool swapIt = false;
            if (by == 1) swapIt = lower(v[j].getName()) > lower(v[j + 1].getName());
            if (by == 2) swapIt = v[j].getId() > v[j + 1].getId();
            if (by == 3) swapIt = getBalance(v[j]) < getBalance(v[j + 1]);
            if (swapIt) { Student t = v[j]; v[j] = v[j + 1]; v[j + 1] = t; }
        }
    }
}
void sortStudents() {
    title("SORT STUDENTS");
    cout << "  1. By name\n  2. By ID\n  3. By balance (highest first)\n";
    vector<Student> copy = students;
    sortStudentList(copy, readInt("Sort by: ", 1, 3));
    showStudentTable(copy);
}
void filterStudents() {
    title("FILTER STUDENTS");
    cout << "  1. By course\n  2. By year\n  3. By status (Active/Inactive)\n  4. By fee status (PAID/PARTIAL/UNPAID)\n";
    int c = readInt("Filter by: ", 1, 4);
    string course, wanted;
    int year = 0;
    if (c == 1) course = upper(readText("Course: "));
    if (c == 2) year = readInt("Year (1-8): ", 1, 8);
    if (c == 3) wanted = readInt("1 = Active, 2 = Inactive: ", 1, 2) == 1 ? "Active" : "Inactive";
    if (c == 4) { int k = readInt("1 = PAID, 2 = PARTIAL, 3 = UNPAID: ", 1, 3); wanted = k == 1 ? "PAID" : (k == 2 ? "PARTIAL" : "UNPAID"); }
    vector<Student> found;
    for (size_t i = 0; i < students.size(); i++) {
        bool match = false;
        if (c == 1) match = students[i].getCourse() == course;
        if (c == 2) match = students[i].getYear() == year;
        if (c == 3) match = students[i].getStatus() == wanted;
        if (c == 4) match = getFeeStatus(students[i]) == wanted;
        if (match) found.push_back(students[i]);
    }
    showStudentTable(found);
}
void studentMenu() {
    while (true) {
        title("STUDENT MANAGEMENT");
        cout << "  1. Add student\n  2. View all students\n  3. View student profile\n  4. Update student\n  5. Delete student\n"
             << "  6. Search students\n  7. Sort students\n  8. Filter students\n  0. Back\n";
        int c = readInt("Enter choice: ", 0, 8);
        if (c == 0) return;
        if (c == 1) addStudent();
        if (c == 2) { title("ALL STUDENTS"); showStudentTable(students); }
        if (c == 3) viewOneStudent();
        if (c == 4) updateStudent();
        if (c == 5) deleteStudent();
        if (c == 6) searchStudent();
        if (c == 7) sortStudents();
        if (c == 8) filterStudents();
        pauseScreen();
    }
}

// ===================== 8. PAYMENTS AND RECEIPTS =====================
string chooseMode() {
    string modes[4] = {"Cash", "Cheque", "Card", "Online"};
    cout << "  1. Cash\n  2. Cheque\n  3. Card\n  4. Online\n";
    return modes[readInt("Payment mode: ", 1, 4) - 1];
}
// 'out' can be the screen (cout) or a file, so one function prints both
void writeReceipt(ostream& out, Payment& p) {
    string name = "(student deleted)", course = "-";
    double balance = 0;
    int pos = findStudent(p.getStudentId());
    if (pos >= 0) { name = students[pos].getName(); course = students[pos].getCourse(); balance = getBalance(students[pos]); }
    out << "\n" << string(50, '=') << "\n            COLLEGE FEE RECEIPT\n" << string(50, '=') << "\n"
        << "  Receipt No   : " << p.getReceiptNo() << "\n  Date         : " << p.getDate()
        << "\n  Student ID   : " << p.getStudentId() << "\n  Student Name : " << name << "\n  Course       : " << course
        << "\n  Payment Mode : " << p.getMode() << "\n  Remarks      : " << p.getRemarks() << "\n" << string(50, '-')
        << "\n  AMOUNT PAID  : " << money(p.getAmount()) << "\n  BALANCE DUE  : " << money(balance) << "\n" << string(50, '-')
        << "\n  Thank you. Computer generated receipt.\n" << string(50, '=') << "\n";
}
void saveReceiptFile(Payment& p) {
    ostringstream name;
    name << "receipt_" << p.getReceiptNo() << ".txt";
    ofstream file(name.str().c_str());
    writeReceipt(file, p);
    cout << "  Receipt saved in file: " << name.str() << "\n";
}
void makePayment() {
    title("MAKE A FEE PAYMENT");
    string id = upper(readText("Student ID: "));
    int pos = findStudent(id);
    if (pos < 0) { cout << "  Student not found.\n"; return; }
    cout << "  Student: " << students[pos].getName() << " (" << students[pos].getCourse() << ")\n";
    if (students[pos].getStatus() != "Active") { cout << "  Student is Inactive. Payment not allowed.\n"; return; }
    showFeeSummary(students[pos]);
    double balance = getBalance(students[pos]);
    if (balance <= 0) { cout << "\n  No dues to pay.\n"; return; }
    double amount = readDouble("\nAmount to pay: ", 0.01, balance);       // cannot pay more than the balance
    string mode = chooseMode();
    string date;
    while (true) {
        date = readDate("Payment date (YYYY-MM-DD, Enter = today): ");
        if (date > today()) cout << "  Date cannot be in the future.\n";
        else if (date < students[pos].getAdmissionDate()) cout << "  Date cannot be before admission (" << students[pos].getAdmissionDate() << ").\n";
        else break;
    }
    Payment p(nextReceiptNo(), id, date, amount, mode, readOptional("Remarks (Enter to skip): "));
    payments.push_back(p);
    savePayments();
    cout << "  Payment saved.\n";
    writeReceipt(cout, p);
    saveReceiptFile(p);
}
void viewReceipt() {
    int pos = findPayment(readInt("Receipt number: ", 1, 999999999));
    if (pos < 0) { cout << "  Receipt not found.\n"; return; }
    writeReceipt(cout, payments[pos]);
    if (askYesNo("Save this receipt to a file?")) saveReceiptFile(payments[pos]);
}
void searchPayments() {
    title("SEARCH PAYMENTS");
    cout << "  1. By date\n  2. By payment mode\n";
    int c = readInt("Enter choice: ", 1, 2);
    string date, mode;
    if (c == 1) date = readDate("Date (YYYY-MM-DD): ");
    if (c == 2) mode = chooseMode();
    vector<Payment> found;
    for (size_t i = 0; i < payments.size(); i++) {
        bool match = false;
        if (c == 1) match = payments[i].getDate() == date;
        if (c == 2) match = payments[i].getMode() == mode;
        if (match) found.push_back(payments[i]);
    }
    showPaymentTable(found);
}
void editPayment() {
    title("EDIT A PAYMENT");
    int pos = findPayment(readInt("Receipt number to edit: ", 1, 999999999));
    if (pos < 0) { cout << "  Receipt not found.\n"; return; }
    cout << "  1. Amount (now " << money(payments[pos].getAmount()) << ")\n  2. Remarks\n  0. Cancel\n";
    int c = readInt("Field to change: ", 0, 2);
    if (c == 0) return;
    if (c == 1) {
        double limit = payments[pos].getAmount();          // old amount + what is still due
        int sp = findStudent(payments[pos].getStudentId());
        if (sp >= 0 && getBalance(students[sp]) > 0) limit += getBalance(students[sp]);
        payments[pos].setAmount(readDouble("New amount: ", 0.01, limit));
    }
    else payments[pos].setRemarks(readOptional("New remarks: "));
    savePayments();
    cout << "  Payment updated.\n";
}
void cancelPayment() {
    title("CANCEL A PAYMENT");
    int pos = findPayment(readInt("Receipt number to cancel: ", 1, 999999999));
    if (pos < 0) { cout << "  Receipt not found.\n"; return; }
    vector<Payment> one(1, payments[pos]);
    showPaymentTable(one);
    if (askYesNo("Cancel this payment? The balance will increase")) {
        payments.erase(payments.begin() + pos);
        savePayments();
        cout << "  Payment cancelled.\n";
    }
}
void paymentMenu() {
    while (true) {
        title("PAYMENTS AND RECEIPTS");
        cout << "  1. Make payment\n  2. View all payments\n  3. View / save a receipt\n"
             << "  4. Search payments\n  5. Edit a payment\n  6. Cancel a payment\n  0. Back\n";
        int c = readInt("Enter choice: ", 0, 6);
        if (c == 0) return;
        if (c == 1) makePayment();
        if (c == 2) { title("ALL PAYMENTS"); showPaymentTable(payments); }
        if (c == 3) viewReceipt();
        if (c == 4) searchPayments();
        if (c == 5) editPayment();
        if (c == 6) cancelPayment();
        pauseScreen();
    }
}

// ===================== 9. DISCOUNTS AND FINES =====================
void saveNewAdjustment(string studentId, string type, double amount, string reason) {
    adjustments.push_back(Adjustment(nextAdjustmentId(), studentId, type, amount, reason, today()));
    saveAdjustments();
    cout << "  " << type << " of " << money(amount) << " saved.\n";
}
void addDiscount() {
    title("GIVE DISCOUNT / SCHOLARSHIP");
    string id = upper(readText("Student ID: "));
    int pos = findStudent(id);
    if (pos < 0) { cout << "  Student not found.\n"; return; }
    double base = getBaseFee(students[pos]);
    double remaining = base - sumAdjustments(id, "DISCOUNT");     // total discount cannot pass the base fee
    cout << "  Base fee: " << money(base) << "    Discount still possible: " << money(remaining) << "\n";
    if (remaining <= 0.005) { cout << "  No more discount can be given.\n"; return; }
    cout << "  1. Percentage of base fee\n  2. Fixed amount\n";
    double amount;
    if (readInt("Discount type: ", 1, 2) == 1) {
        amount = base * readDouble("Percentage (1-100): ", 1, 100) / 100.0;
        cout << "  Discount amount = " << money(amount) << "\n";
    } else amount = readDouble("Discount amount: ", 0.01, 100000000);
    if (amount > remaining + 0.005) { cout << "  Discount is more than allowed. Not saved.\n"; return; }
    saveNewAdjustment(id, "DISCOUNT", amount, readText("Reason (example Merit scholarship): "));
}
void addFine() {
    title("ADD A FINE");
    string id = upper(readText("Student ID: "));
    if (findStudent(id) < 0) { cout << "  Student not found.\n"; return; }
    cout << "  1. Fixed fine\n  2. Late fee fine (per day)\n";
    if (readInt("Fine type: ", 1, 2) == 1) {
        double amount = readDouble("Fine amount: ", 0.01, 1000000);
        saveNewAdjustment(id, "FINE", amount, readText("Reason (example Library book lost): "));
        return;
    }
    string due = readDate("Fee due date (YYYY-MM-DD): ");
    int lateDays = daysBetween(due, today());
    if (lateDays <= 0) { cout << "  Due date has not passed. No fine.\n"; return; }
    double amount = lateDays * readDouble("Fine per day: ", 0.01, 10000);
    cout << "  Days late: " << lateDays << "    Fine = " << money(amount) << "\n";
    ostringstream reason;
    reason << "Late fee (" << lateDays << " days)";
    if (askYesNo("Apply this fine?")) saveNewAdjustment(id, "FINE", amount, reason.str());
}
void deleteAdjustment() {
    title("DELETE A DISCOUNT / FINE");
    int pos = findAdjustment(readInt("Record ID: ", 1, 999999999));
    if (pos < 0) { cout << "  Record not found.\n"; return; }
    vector<Adjustment> one(1, adjustments[pos]);
    showAdjustmentTable(one);
    if (askYesNo("Delete this record?")) {
        adjustments.erase(adjustments.begin() + pos);
        saveAdjustments();
        cout << "  Record deleted.\n";
    }
}
void adjustmentMenu() {
    while (true) {
        title("DISCOUNTS AND FINES");
        cout << "  1. Give discount\n  2. Add fine (fixed or late fee)\n  3. View all records\n  4. Delete a record\n  0. Back\n";
        int c = readInt("Enter choice: ", 0, 4);
        if (c == 0) return;
        if (c == 1) addDiscount();
        if (c == 2) addFine();
        if (c == 3) { title("ALL DISCOUNTS AND FINES"); showAdjustmentTable(adjustments); }
        if (c == 4) deleteAdjustment();
        pauseScreen();
    }
}

// ===================== 10. BALANCE CHECK AND REPORTS =====================
void checkBalance() {
    title("OUTSTANDING BALANCE");
    int pos = findStudent(upper(readText("Student ID: ")));
    if (pos < 0) { cout << "  Student not found.\n"; return; }
    cout << "  " << students[pos].getName() << " - " << students[pos].getCourse() << " Year " << students[pos].getYear() << "\n\n";
    showFeeSummary(students[pos]);
}
double totalCollected() {
    double t = 0;
    for (size_t i = 0; i < payments.size(); i++) t += payments[i].getAmount();
    return t;
}
void dashboardReport() {
    title("SUMMARY DASHBOARD");
    double expected = 0, discounts = 0, fines = 0, outstanding = 0;
    int active = 0;
    for (size_t i = 0; i < students.size(); i++) {
        string id = students[i].getId();
        expected += getNetFee(students[i]);
        discounts += sumAdjustments(id, "DISCOUNT");
        fines += sumAdjustments(id, "FINE");
        if (getBalance(students[i]) > 0) outstanding += getBalance(students[i]);
        if (students[i].getStatus() == "Active") active++;
    }
    cout << "  Total students           : " << students.size() << " (Active: " << active << ")\n"
         << "  Fee structures           : " << structures.size() << "\n  Payments recorded        : " << payments.size()
         << "\n";
    line();
    cout << "  Total expected fee       : " << money(expected) << "\n  Total discounts given    : " << money(discounts)
         << "\n  Total fines charged      : " << money(fines) << "\n  Total collected          : " << money(totalCollected())
         << "\n  Total outstanding        : " << money(outstanding) << "\n";
}
void writeOutstandingReport(ostream& out) {             // used for the screen and for the text file
    vector<Student> due;
    for (size_t i = 0; i < students.size(); i++) if (getBalance(students[i]) > 0) due.push_back(students[i]);
    sortStudentList(due, 3);
    out << "OUTSTANDING FEE REPORT (" << today() << ")\n" << string(70, '-') << "\n"
        << left << setw(8) << "ID" << setw(22) << "Name" << setw(9) << "Course" << right << setw(11) << "Net fee"
        << setw(11) << "Paid" << setw(11) << "Balance" << "\n" << string(70, '-') << "\n";
    double total = 0;
    for (size_t i = 0; i < due.size(); i++) {
        out << left << setw(8) << due[i].getId() << setw(22) << shorten(due[i].getName(), 21) << setw(9) << shorten(due[i].getCourse(), 8)
            << right << setw(11) << money(getNetFee(due[i])) << setw(11) << money(getTotalPaid(due[i].getId()))
            << setw(11) << money(getBalance(due[i])) << "\n";
        total += getBalance(due[i]);
    }
    out << string(70, '-') << "\n" << left << "Students with dues: " << due.size() << "    Total outstanding: " << money(total) << "\n";
}
void outstandingReport() {
    title("OUTSTANDING BALANCE REPORT");
    writeOutstandingReport(cout);
    if (askYesNo("Save this report to outstanding_report.txt?")) {
        ofstream file("outstanding_report.txt");
        writeOutstandingReport(file);
        cout << "  Report saved.\n";
    }
}
void dateRangeReport() {
    title("COLLECTION BETWEEN TWO DATES");
    string from = readDate("From date (YYYY-MM-DD): "), to;
    while (true) {
        to = readDate("To date (YYYY-MM-DD, Enter = today): ");
        if (to >= from) break;
        cout << "  'To' date cannot be before 'From' date.\n";
    }
    vector<Payment> found;
    for (size_t i = 0; i < payments.size(); i++)
        if (payments[i].getDate() >= from && payments[i].getDate() <= to) found.push_back(payments[i]);
    showPaymentTable(found);
}
void courseReport() {
    title("COURSE-WISE COLLECTION");
    vector<string> courses;                             // parallel lists: course name and money collected
    vector<double> amounts;
    for (size_t i = 0; i < payments.size(); i++) {
        string course = "Unknown";
        int sp = findStudent(payments[i].getStudentId());
        if (sp >= 0) course = students[sp].getCourse();
        int idx = -1;
        for (size_t j = 0; j < courses.size(); j++) if (courses[j] == course) idx = j;
        if (idx == -1) { courses.push_back(course); amounts.push_back(0); idx = courses.size() - 1; }
        amounts[idx] += payments[i].getAmount();
    }
    if (courses.empty()) { cout << "  No payments recorded yet.\n"; return; }
    for (size_t i = 0; i < courses.size(); i++) cout << "  " << left << setw(14) << courses[i] << right << setw(14) << money(amounts[i]) << "\n";
}
void studentLedger() {
    title("STUDENT FEE LEDGER");
    string id = upper(readText("Student ID: "));
    int pos = findStudent(id);
    if (pos < 0) { cout << "  Student not found.\n"; return; }
    students[pos].show();
    vector<Adjustment> adj;
    for (size_t i = 0; i < adjustments.size(); i++) if (adjustments[i].getStudentId() == id) adj.push_back(adjustments[i]);
    cout << "\n  -- Discounts and fines --\n";
    showAdjustmentTable(adj);
    vector<Payment> pay;
    for (size_t i = 0; i < payments.size(); i++) if (payments[i].getStudentId() == id) pay.push_back(payments[i]);
    cout << "\n  -- Payments --\n";
    showPaymentTable(pay);
    cout << "\n";
    showFeeSummary(students[pos]);
}
void reportMenu() {
    while (true) {
        title("REPORTS AND SUMMARIES");
        cout << "  1. Summary dashboard\n  2. Outstanding balance report (save option)\n  3. Collection between two dates\n"
             << "  4. Course-wise collection\n  5. Student ledger\n  0. Back\n";
        int c = readInt("Enter choice: ", 0, 5);
        if (c == 0) return;
        if (c == 1) dashboardReport();
        if (c == 2) outstandingReport();
        if (c == 3) dateRangeReport();
        if (c == 4) courseReport();
        if (c == 5) studentLedger();
        pauseScreen();
    }
}

// ===================== 11. LOGIN AND MAIN =====================
bool login() {
    title("COLLEGE FEE MANAGEMENT SYSTEM - LOGIN");
    for (int attempt = 3; attempt >= 1; attempt--) {
        string user = trim(readLine("Username: "));
        string pass = trim(readLine("Password: "));
        if (user == "admin" && pass == "admin123") { cout << "  Login successful.\n"; return true; }
        cout << "  Wrong username or password. Attempts left: " << attempt - 1 << "\n";
    }
    return false;
}
int main() {
    loadAll();
    if (!login()) { cout << "  Too many wrong attempts. Program closed.\n"; return 0; }
    while (true) {
        title("MAIN MENU - COLLEGE FEE MANAGEMENT");
        cout << "  1. Student management\n  2. Fee structure management\n  3. Payments and receipts\n  4. Discounts and fines\n"
             << "  5. Check outstanding balance\n  6. Reports and summaries\n  0. Exit\n";
        int c = readInt("Enter choice: ", 0, 6);
        if (c == 0) break;
        if (c == 1) studentMenu();
        if (c == 2) structureMenu();
        if (c == 3) paymentMenu();
        if (c == 4) adjustmentMenu();
        if (c == 5) { checkBalance(); pauseScreen(); }
        if (c == 6) reportMenu();
    }
    cout << "\n  All data is saved. Goodbye!\n";
    return 0;
}
