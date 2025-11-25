#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <vector>
#include <cstdlib>

using namespace std;

// ---------- BASE64 ENCODE/DECODE (C++98 Compatible) ----------
const string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

string base64_encode(const string &in) {
    string out;
    int val = 0, valb = -6;
    for (size_t i = 0; i < in.size(); i++) {
        unsigned char c = in[i];
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

string base64_decode(const string &in) {
    string out;
    vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[(int)base64_chars[i]] = i;
    int val = 0, valb = -8;
    for (size_t i = 0; i < in.size(); i++) {
        unsigned char c = in[i];
        if (T[(int)c] == -1) break;
        val = (val << 6) + T[(int)c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

// ---------------------- HELPERS ----------------------
void flushInput() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

struct Admin {
    string username;
    string password; // decoded
};

vector<Admin> admins;

void saveAdmins() {
    ofstream file("admin.txt");
    for (size_t i = 0; i < admins.size(); ++i) {
        file << admins[i].username << '|' << base64_encode(admins[i].password) << '\n';
    }
    file.close();
}

void loadAdmins() {
    admins.clear();
    ifstream file("admin.txt");
    if (!file) return;

    string line;
    vector<string> rawLines;
    while (getline(file, line)) {
        rawLines.push_back(line);
        size_t delimPos = line.find('|');
        if (delimPos != string::npos) {
            string user = line.substr(0, delimPos);
            string passEncoded = line.substr(delimPos + 1);
            Admin a;
            a.username = user;
            a.password = base64_decode(passEncoded);
            admins.push_back(a);
        }
    }
    file.close();

    if (!admins.empty()) return;

    if (rawLines.size() >= 2) {
        Admin legacy;
        legacy.username = rawLines[0];
        legacy.password = base64_decode(rawLines[1]);
        admins.push_back(legacy);
        saveAdmins(); // convert legacy file to the new pipe-delimited format
    }
}

bool usernameExists(const string &user) {
    for (size_t i = 0; i < admins.size(); ++i) {
        if (admins[i].username == user) return true;
    }
    return false;
}

void addAdminInteractive() {
    flushInput();
    string user, pass;
    cout << "\n===== ADD NEW ADMIN =====\n";
    cout << "Enter username: ";
    getline(cin, user);

    if (user.empty()) {
        cout << "Username cannot be empty.\n";
        return;
    }
    if (usernameExists(user)) {
        cout << "Username already exists.\n";
        return;
    }

    cout << "Enter password: ";
    getline(cin, pass);
    if (pass.empty()) {
        cout << "Password cannot be empty.\n";
        return;
    }

    Admin a;
    a.username = user;
    a.password = pass;
    admins.push_back(a);
    saveAdmins();
    cout << "Admin added successfully.\n";
}

void ensureAdminSetup() {
    if (!admins.empty()) return;

    cout << "\n===== INITIAL ADMIN SETUP =====";
    flushInput();
    while (true) {
        string user, pass;
        cout << "Create username: ";
        getline(cin, user);
        cout << "Create password: ";
        getline(cin, pass);

        if (user.empty() || pass.empty()) {
            cout << "Username and password must not be empty.\n";
            continue;
        }

        Admin a;
        a.username = user;
        a.password = pass;
        admins.push_back(a);
        saveAdmins();

        char choice;
        cout << "Add another admin? (y/n): ";
        cin >> choice;
        flushInput();
        if (choice != 'y' && choice != 'Y') break;
    }
}

// ---------------------- ROOM & BOOKING STRUCTS ----------------------
struct Room {
    int roomNo;
    string type;
    double price;
    bool isBooked;
};

struct Booking {
    string customerName;
    string phone;
    int roomNo;
    string checkIn;
    string checkOut;
    string paymentMethod;
};

Room rooms[10];
Booking bookings[10];
int bookingCount = 0;

// ---------------------- ADMIN LOGIN ----------------------
bool validateCredentials(const string &user, const string &pass) {
    for (size_t i = 0; i < admins.size(); ++i) {
        if (admins[i].username == user && admins[i].password == pass) return true;
    }
    return false;
}

bool adminLogin() {
    ensureAdminSetup();
    loadAdmins();
    if (admins.empty()) {
        cout << "Admin setup failed.\n";
        return false;
    }

    for (int attempt = 1; attempt <= 3; ++attempt) {
        clearScreen();
        cout << "\n===== ADMIN LOGIN ===== (Attempt " << attempt << " of 3)";
        flushInput();
        string user, pass;
        cout << "Username: ";
        getline(cin, user);
        cout << "Password: ";
        getline(cin, pass);

        if (validateCredentials(user, pass)) {
            cout << "Login successful!\n";
            return true;
        }

        cout << "Invalid username or password.\n";
        if (attempt < 3) {
            cout << "Press Enter to retry...";
            cin.get();
        }
    }

    cout << "Too many failed attempts. Exiting.\n";
    return false;
}

// ---------------------- CREATE ROOMS ----------------------
void createRooms() {
    string types[3] = {"Standard", "Deluxe", "Suite"};
    double prices[3] = {2000, 3500, 5000};
    for (int i = 0; i < 10; i++) {
        rooms[i].roomNo = 101 + i;
        rooms[i].type = types[i % 3];
        rooms[i].price = prices[i % 3];
        rooms[i].isBooked = false;
    }
}

// ---------------------- SAVE TO FILE ----------------------
void saveToFile() {
    ofstream file("bookings.txt");
    file << bookingCount << endl;
    for (int i = 0; i < bookingCount; i++) {
        file << bookings[i].customerName << endl;
        file << bookings[i].phone << endl;
        file << bookings[i].roomNo << endl;
        file << bookings[i].checkIn << endl;
        file << bookings[i].checkOut << endl;
        file << bookings[i].paymentMethod << endl;
    }
    for (int i = 0; i < 10; i++) {
        file << rooms[i].isBooked << endl;
    }
    file.close();
}

// ---------------------- LOAD FROM FILE ----------------------
void loadFromFile() {
    ifstream file("bookings.txt");
    if (!file) return;

    file >> bookingCount;
    file.ignore();
    for (int i = 0; i < bookingCount; i++) {
        getline(file, bookings[i].customerName);
        getline(file, bookings[i].phone);
        file >> bookings[i].roomNo;
        file.ignore();
        getline(file, bookings[i].checkIn);
        getline(file, bookings[i].checkOut);
        getline(file, bookings[i].paymentMethod);
    }
    for (int i = 0; i < 10; i++) {
        file >> rooms[i].isBooked;
    }
    file.close();
}

// ---------------------- SORT & VIEW ROOMS ----------------------
void sortRooms() {
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (rooms[i].price > rooms[j].price) {
                Room temp = rooms[i];
                rooms[i] = rooms[j];
                rooms[j] = temp;
            }
        }
    }
}

void viewRooms() {
    clearScreen();
    cout << "\n===== ROOM LIST (Sorted by Price) =====\n";
    sortRooms();
    for (int i = 0; i < 10; i++) {
        cout << "Room: " << rooms[i].roomNo
             << " | Type: " << rooms[i].type
             << " | Price: Rs. " << rooms[i].price
             << " | Status: " << (rooms[i].isBooked ? "Booked" : "Available")
             << endl;
    }
}

// ---------------------- ADD BOOKING ----------------------
void addBooking() {
    if (bookingCount >= 10) {
        cout << "Booking full!\n";
        return;
    }

    Booking b;
    flushInput();
    cout << "\nEnter Customer Name: ";
    getline(cin, b.customerName);
    cout << "Phone: ";
    getline(cin, b.phone);

    viewRooms();
    cout << "Enter Room Number to Book: ";
    cin >> b.roomNo;
    flushInput();

    bool found = false;
    for (int i = 0; i < 10; i++) {
        if (rooms[i].roomNo == b.roomNo && !rooms[i].isBooked) {
            rooms[i].isBooked = true;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Room not available!\n";
        return;
    }

    cout << "Check-in Date (DD/MM/YYYY): ";
    getline(cin, b.checkIn);
    cout << "Check-out Date (DD/MM/YYYY): ";
    getline(cin, b.checkOut);
    cout << "Payment Method (Cash/Online/Card): ";
    getline(cin, b.paymentMethod);

    bookings[bookingCount++] = b;
    saveToFile();
    cout << "Booking successful!\n";
}

// ---------------------- SEARCH & DELETE ----------------------
void searchCustomer() {
    flushInput();
    string name;
    cout << "\nEnter customer name to search: ";
    getline(cin, name);

    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].customerName == name) {
            cout << "\nCustomer Found!\n";
            cout << "Room: " << bookings[i].roomNo << endl;
            cout << "Phone: " << bookings[i].phone << endl;
            cout << "Check-In: " << bookings[i].checkIn << endl;
            cout << "Check-Out: " << bookings[i].checkOut << endl;
            return;
        }
    }
    cout << "Customer not found.\n";
}

void deleteBooking() {
    int room;
    cout << "\nEnter room number to delete booking: ";
    cin >> room;
    flushInput();

    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].roomNo == room) {
            for (int j = 0; j < 10; j++)
                if (rooms[j].roomNo == room)
                    rooms[j].isBooked = false;
            for (int k = i; k < bookingCount - 1; k++)
                bookings[k] = bookings[k + 1];
            bookingCount--;
            saveToFile();
            cout << "Booking deleted.\n";
            return;
        }
    }
    cout << "Booking not found.\n";
}

// ---------------------- CHANGE PASSWORD ----------------------
void changePassword() {
    loadAdmins();
    if (admins.empty()) {
        cout << "No admin accounts available.\n";
        return;
    }

    flushInput();
    cout << "\n===== CHANGE ADMIN PASSWORD =====\n";
    string u, p;
    cout << "Enter current username: ";
    getline(cin, u);
    cout << "Enter current password: ";
    getline(cin, p);

    for (size_t i = 0; i < admins.size(); ++i) {
        if (admins[i].username == u && admins[i].password == p) {
            string newUser, newPass;
            cout << "Enter new username: ";
            getline(cin, newUser);
            cout << "Enter new password: ";
            getline(cin, newPass);

            if (newUser.empty() || newPass.empty()) {
                cout << "Username or password cannot be empty.\n";
                return;
            }

            admins[i].username = newUser;
            admins[i].password = newPass;
            saveAdmins();
            cout << "Password changed successfully!\n";
            return;
        }
    }
    cout << "Incorrect username or password!\n";
}

// ---------------------- MAIN MENU ----------------------
void addAdminAccount() {
    loadAdmins();
    addAdminInteractive();
}

void menu() {
    int ch;
    do {
        clearScreen();
        cout << "\n===== HOTEL BOOKING SYSTEM =====\n";
        cout << "1. View Rooms\n";
        cout << "2. Add Booking\n";
        cout << "3. Search Customer\n";
        cout << "4. Delete Booking\n";
        cout << "5. Change Admin Password\n";
        cout << "6. Add Admin Account\n";
        cout << "7. Exit\n";
        cout << "Choose option: ";
        cin >> ch;

        switch (ch) {
            case 1: viewRooms(); break;
            case 2: addBooking(); break;
            case 3: searchCustomer(); break;
            case 4: deleteBooking(); break;
            case 5: changePassword(); break;
            case 6: addAdminAccount(); break;
            case 7: cout << "Goodbye!\n"; break;
            default: cout << "Invalid option.\n";
        }

        if (ch != 7) {
            cout << "\nPress Enter to continue...";
            flushInput();
        }
    } while (ch != 7);
}

int main() {
    loadAdmins();
    if (!adminLogin()) return 0;

    createRooms();
    loadFromFile();
    menu();
    return 0;
}